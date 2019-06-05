#include "parser.h"
#include "logger.h"
#include "options.h"

#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>
#include <boost/program_options.hpp>

#include <boost/asio/io_context.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/executor_work_guard.hpp>
#include <boost/asio/signal_set.hpp>

#include <iostream>

namespace po = boost::program_options;
namespace asio = boost::asio;

namespace
{
  const std::string log_file_name = "mpeg-ts-demux_%Y%m%d_%H%M%S.log";
} //

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

    // main context is to wait for Ctrl + C
    asio::io_context main_context;

    asio::signal_set signal_set(main_context, SIGINT, SIGTERM);

    signal_set.async_wait([&svc](const auto &ec, int sig_code) {
      BOOST_LOG_TRIVIAL(trace) << "Got signal: %d";

      if (ec)
      {
        BOOST_LOG_TRIVIAL(error) << "Error: " << ec.message();
      }

      svc.stop();
    });

    int ret = main_context.run();

    // context is finished in 2 cases:
    // 1. got signal
    // 2. no more data

    svc.wait();

    handling_context.stop();
    work.reset();
    handling_thread.join();

    return ret;
  }
  catch (const std::exception &e)
  {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  return 0;
}
