#include "demux_service.h"
#include "logger.h"
#include "options.h"

#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>
#include <boost/program_options.hpp>

#include <boost/asio/executor_work_guard.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/signal_set.hpp>

#include <iostream>

namespace po = boost::program_options;
namespace asio = boost::asio;

namespace
{
const std::string log_file_name = "mpeg-ts-demux_%Y%m%d_%H%M%S.log";
} // namespace

int main(int argc, char *argv[])
{
  try
  {
    mpegts::options options;

    if (!options.parse(argc, argv))
    {
      return 1;
    }

    logger::init(options.get_log_severity_level(), log_file_name);
    options.print();

    asio::io_context main_context;
    mpegts::demux_service svc(
        options.get_input_file_name(), main_context, [](mpegts::pes_packet_uptr packet) {

        });

    asio::signal_set signal_set(main_context, SIGINT, SIGTERM);

    signal_set.async_wait([&svc](const auto &ec, int sig_code) {
      BOOST_LOG_TRIVIAL(trace) << "Got signal: " << sig_code << "stopping...";

      if (ec)
      {
        BOOST_LOG_TRIVIAL(error) << "Error: " << ec.message();
      }

      svc.stop();
    });

    svc.start();

    int ret = main_context.run();

    svc.join();

    BOOST_LOG_TRIVIAL(info) << "Exitting...";

    return ret;
  }
  catch (const std::exception &e)
  {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  return 0;
}
