// server.cpp
#include <ucp/api/ucp.h>
#include <ucs/type/status.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cstring>
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <cstdlib>

// Global container for connected client endpoints and its mutex.
std::vector<ucp_ep_h> connected_clients;
std::mutex clients_mutex;
void connection_handler(ucp_conn_request_h conn_request, void* arg)
{
  auto worker = static_cast<ucp_worker_h>(arg);
  ucp_ep_params_t ep_params{};
  ep_params.field_mask = UCP_EP_PARAM_FIELD_CONN_REQUEST | UCP_EP_PARAM_FIELD_ERR_HANDLER;
  ep_params.conn_request = conn_request;
  ep_params.err_handler.cb = [](void*, ucp_ep_h, ucs_status_t status) {
    std::cerr << "Endpoint error: " << ucs_status_string(status) << "\n";
  };
  ucp_ep_h ep = nullptr;
  ucs_status_t status = ucp_ep_create(worker, &ep_params, &ep);
  if (status != UCS_OK) {
    std::cerr << "Failed to create endpoint: " << ucs_status_string(status) << std::endl;
    return;
  }
  {
    std::lock_guard<std::mutex> lock(clients_mutex);
    connected_clients.push_back(ep);
  }
  std::cout << "New client connected." << std::endl;
}

// Helper: Blocking send of a message using UCX nonblocking API.
void blocking_send(ucp_ep_h ep, const char* buffer, size_t length, ucp_worker_h worker)
{
  // Send with tag 0 and a trivial callback.
  std::cout << "Sending." << std::endl;
  void* request = ucp_tag_send_nb(ep, buffer, length, ucp_dt_make_contig(1), 0,
                                  [](void* req, ucs_status_t /*status*/) { (void)req; });

  if (UCS_PTR_IS_ERR(request)) {
    throw std::runtime_error("ucp_tag_send_nb failed");
  }
  // If request is not immediately complete, poll until it is.
  if (request != nullptr) {
    while (ucp_request_check_status(request) == UCS_INPROGRESS) {
      ucp_worker_progress(worker);
    }
    ucp_request_free(request);
  }
}

// Broadcast message function: sends a 1GB message (with "hello, world" in the first 13 bytes)
// to all connected endpoints.
void broadcast_message(ucp_worker_h worker)
{
  constexpr size_t msg_size = 1024ULL * 1024 * 1024; // 1GB
  // Create and initialize the message.
  std::vector<char> msg(msg_size, 0);
  constexpr char greeting[13] = "hello, world";
  std::memcpy(msg.data(), greeting, sizeof(greeting));

  // Lock the client list and send the message to each endpoint.
  std::lock_guard<std::mutex> lock(clients_mutex);
  for (auto it = connected_clients.begin(); it != connected_clients.end();) {
    try {
      blocking_send(*it, msg.data(), msg.size(), worker);
      ++it;
    }
    catch (const std::exception& e) {
      std::cerr << "Error sending to client, removing endpoint: " << e.what() << std::endl;
      // Close the endpoint and remove it from the list.
      ucp_ep_close_nb(*it, 0);
      it = connected_clients.erase(it);
    }
  }
}

int main()
{
  ucp_params_t ucp_params{};
  ucp_params.field_mask = UCP_PARAM_FIELD_FEATURES;
  ucp_params.features = UCP_FEATURE_TAG;

  ucp_config_t* config = nullptr;
  ucs_status_t status = ucp_config_read(nullptr, nullptr, &config);
  if (status != UCS_OK) {
    std::cerr << "Failed to read UCX config: " << ucs_status_string(status) << std::endl;
    std::exit(EXIT_FAILURE);
  }

  ucp_context_h context = nullptr;
  status = ucp_init(&ucp_params, config, &context);
  ucp_config_release(config);
  if (status != UCS_OK) {
    std::cerr << "Failed to initialize UCX: " << ucs_status_string(status) << std::endl;
    std::exit(EXIT_FAILURE);
  }

  ucp_worker_params_t worker_params{};
  worker_params.field_mask = UCP_WORKER_PARAM_FIELD_THREAD_MODE;
  worker_params.thread_mode = UCS_THREAD_MODE_MULTI;

  ucp_worker_h worker = nullptr;
  status = ucp_worker_create(context, &worker_params, &worker);
  if (status != UCS_OK) {
    std::cerr << "Failed to create UCX worker: " << ucs_status_string(status) << std::endl;
    ucp_cleanup(context);
    std::exit(EXIT_FAILURE);
  }

  // Set up a listening socket address on SERVER_PORT.
  constexpr int SERVER_PORT = 13337;
  struct sockaddr_in listen_addr{};
  listen_addr.sin_family = AF_INET;
  listen_addr.sin_port = htons(SERVER_PORT);
  listen_addr.sin_addr.s_addr = INADDR_ANY;

  // Configure listener parameters, including the connection handler callback.
  ucp_listener_params_t listener_params{};
  listener_params.field_mask =
      UCP_LISTENER_PARAM_FIELD_SOCK_ADDR | UCP_LISTENER_PARAM_FIELD_CONN_HANDLER;
  listener_params.sockaddr.addr = reinterpret_cast<struct sockaddr*>(&listen_addr);
  listener_params.sockaddr.addrlen = sizeof(listen_addr);
  listener_params.conn_handler.cb = connection_handler;
  listener_params.conn_handler.arg = worker; // Pass the worker directly.

  ucp_listener_h listener = nullptr;
  status = ucp_listener_create(worker, &listener_params, &listener);
  if (status != UCS_OK) {
    std::cerr << "Failed to create listener: " << ucs_status_string(status) << std::endl;
    ucp_worker_destroy(worker);
    ucp_cleanup(context);
    std::exit(EXIT_FAILURE);
  }
  std::cout << "Server listening on port " << SERVER_PORT << std::endl;

  // Main progress loop: drive UCX operations and broadcast every second.
  using clock = std::chrono::steady_clock;
  auto next_broadcast = clock::now() + std::chrono::seconds(1);
  while (true) {
    ucp_worker_progress(worker);

    if (clock::now() >= next_broadcast) {
      broadcast_message(worker);
      next_broadcast += std::chrono::seconds(1);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  // Cleanup (this part is never reached in the example).
  ucp_listener_destroy(listener);
  ucp_worker_destroy(worker);
  ucp_cleanup(context);

  return 0;
}
