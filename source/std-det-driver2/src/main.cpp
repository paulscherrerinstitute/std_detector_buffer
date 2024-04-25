/////////////////////////////////////////////////////////////////////
// Copyright (c) 2024 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <memory>
#include <string>

#include <boost/asio.hpp>

using tcp = boost::asio::ip::tcp;
using namespace std::string_literals;

#include "utils/utils.hpp"
#include "state_manager.hpp"
#include "writer_driver.hpp"
#include "socket_session.hpp"

namespace {

void do_accept(boost::asio::ip::tcp::acceptor& acceptor,
               std::shared_ptr<std_driver::state_manager> manager,
               std::shared_ptr<std_driver::writer_driver> driver)
{
  auto socket = std::make_shared<boost::asio::ip::tcp::socket>(acceptor.get_executor());

  acceptor.async_accept(*socket, [&, socket, manager, driver](boost::system::error_code ec) {
    if (!ec)
      std::make_shared<std_driver::socket_session>(std::move(*socket), manager, driver)->start();

    do_accept(acceptor, manager, driver);
  });
}

std::tuple<utils::DetectorConfig, std::string, std::size_t> read_arguments(int argc, char* argv[])
{
  auto program = utils::create_parser("std_det_driver");
  program->add_argument("-s", "--source_suffix")
      .help("suffix for ipc source for ram_buffer")
      .default_value("image"s);
  program->add_argument("-n", "--number_of_writers")
      .default_value(12ul)
      .scan<'u', std::size_t>()
      .help("Number of parallel writers");

  program = utils::parse_arguments(std::move(program), argc, argv);
  return {utils::read_config_from_json_file(program->get("detector_json_filename")),
          program->get("--source_suffix"), program->get<std::size_t>("--number_of_writers")};
}

} // namespace

int main(int argc, char* argv[])
{
  const auto [config, source_suffix, number_of_writers] = read_arguments(argc, argv);
  [[maybe_unused]] utils::log::logger l{"std_det_driver", config.log_level};
  auto const address = boost::asio::ip::make_address("0.0.0.0");
  auto const port = static_cast<unsigned short>(8080);
  auto sm = std::make_shared<std_driver::state_manager>();
  auto driver = std::make_shared<std_driver::writer_driver>(sm);

  boost::asio::io_context ioc{1};

  tcp::acceptor acceptor{ioc, {address, port}};
  do_accept(acceptor, sm, driver);

  ioc.run();

  return 0;
}
