/////////////////////////////////////////////////////////////////////
// Copyright (c) 2024 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include "socket_session.hpp"

#include <nlohmann/json.hpp>
#include <utility>
#include <fmt/core.h>
#include <spdlog/spdlog.h>

#include "process_api_message.hpp"

using json = nlohmann::json;

namespace sbr {

socket_session::socket_session(tcp::socket socket,
                               std::shared_ptr<state_manager> sm,
                               std::shared_ptr<writer_driver> w)
    : websocket(std::move(socket))
    , manager(std::move(sm))
    , writer(std::move(w))
{
  manager->add_active_session();
}

socket_session::~socket_session()
{
  manager->remove_active_session();
}

void socket_session::start()
{
  spdlog::info("[event] socket_session started - waiting for user request");
  initialize();
  accept_and_process();
}

void socket_session::initialize()
{
  close_socket_handler = [self = shared_from_this()](boost::beast::error_code ec, std::size_t) {
    if (ec) return;
    self->monitor_thread.request_stop();
    self->websocket.async_close(boost::beast::websocket::close_code::normal,
                                [](boost::beast::error_code) {});
    spdlog::info("[event] socket_session finished - connection closed");
  };
}

void socket_session::accept_and_process()
{
  websocket.async_accept([self = shared_from_this()](boost::beast::error_code ec) {
    if (ec) return;
    self->monitor_writer_state();
    self->process_request();
  });
}

void socket_session::process_request()
{
  websocket.async_read(buffer, [self = shared_from_this()](boost::beast::error_code ec,
                                                           std::size_t bytes_transferred) {
    if (ec) return;

    auto message = boost::beast::buffers_to_string(self->buffer.data());
    self->buffer.consume(bytes_transferred);
    spdlog::info(R"([event] Received request: "{}")", message);

    if (auto command = parse_command(message); command.has_value()) {
      const auto command_string = command->value("command", "");
      if (command_string == "stop")
        self->stop_recording();
      else if (self->manager->get_state() == driver_state::idle)
        self->start_recording(message);
      else
        self->reject();
    }
    else
      self->send_response("error", noop_handler, fmt::format("Invalid command! {}", message));
    self->process_request();
  });
}

void socket_session::start_recording(const std::string& message)
{
  if (auto settings = parse_command(message).and_then(process_start_request); settings.has_value())
  {
    writer->start(settings.value());
  }
  else
    send_response(
        "error", close_socket_handler,
        fmt::format("Invalid command! expected: start with path parameter, received: {}", message));
}

void socket_session::stop_recording()
{
  if (manager->is_recording()) manager->change_state(driver_state::stop);
}

void socket_session::monitor_writer_state()
{
  using namespace std::chrono_literals;
  monitor_thread = std::jthread([self = shared_from_this()](std::stop_token stop_token) {
    while (!stop_token.stop_requested()) {
      auto state = self->manager->wait_for_change_or_timeout(100ms);
      if (stop_token.stop_requested()) break; // ensure to stop after timeout

      switch (state) {
      case driver_state::file_saved:
      case driver_state::error:
        self->send_response(to_string(state));
        self->manager->change_state(driver_state::idle);
        break;
      case driver_state::recording:
      case driver_state::saving_file:
        self->send_response_with_count(to_string(state), self->manager->get_images_processed());
        break;
      default:
        self->send_response(to_string(state));
        break;
      }
    }
  });
}

void socket_session::reject()
{
  spdlog::info("[event] Request rejected - driver is busy.");
  send_response("rejected", noop_handler, "driver is busy!");
}

void socket_session::send_response(std::string_view status,
                                   response_handler handler,
                                   std::optional<std::string_view> reason)
{
  json response = {{"status", status}};
  if (reason.has_value()) response["reason"] = reason.value();
  websocket.async_write(boost::asio::buffer(response.dump()), handler);
}

void socket_session::send_response_with_count(std::string_view status, unsigned int count)
{
  const json response = {{"status", status}, {"count", count}};
  websocket.async_write(boost::asio::buffer(response.dump()), noop_handler);
}

} // namespace std_driver
