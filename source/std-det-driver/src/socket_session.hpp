/////////////////////////////////////////////////////////////////////
// Copyright (c) 2024 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_SOCKET_SESSION_HPP
#define STD_DETECTOR_BUFFER_SOCKET_SESSION_HPP

#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include "writer_driver.hpp"

namespace std_driver {

class socket_session : public std::enable_shared_from_this<socket_session>
{
  using tcp = boost::asio::ip::tcp;
  using response_handler = std::function<void(boost::beast::error_code, std::size_t)>;
  static constexpr auto noop_handler = +[](boost::beast::error_code, std::size_t) {};

  boost::beast::websocket::stream<tcp::socket> websocket;
  boost::beast::multi_buffer buffer;
  std::shared_ptr<state_manager> manager;
  std::shared_ptr<writer_driver> writer;

  response_handler close_socket_handler;

public:
  explicit socket_session(tcp::socket socket,
                          std::shared_ptr<state_manager> sm,
                          std::shared_ptr<writer_driver> w);
  void start();

private:
  void initialize();
  void accept_and_process();
  void process_request();
  void start_recording(const std::string& message);
  void stop_recording();
  void get_status();

  void monitor_writer_state();

  void listen_for_stop();

  void reject_and_close();
  void send_response(std::string_view status,
                     response_handler handler = noop_handler,
                     std::optional<std::string_view> reason = std::nullopt);
  void send_recording_response(unsigned int count);
};

} // namespace std_driver

#endif // STD_DETECTOR_BUFFER_SOCKET_SESSION_HPP
