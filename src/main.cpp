/*

Copyright 2019 Peter Asanov

Permission is hereby granted, free of charge,
to any person obtaining a copy of this software and associated documentation files( the "Software"),
to deal in the Software without restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and / or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#include "demux_service.h"
#include "logger.h"
#include "options.h"
#include "utils.hpp"

#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>
#include <boost/program_options.hpp>

#include <boost/asio/executor_work_guard.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/signal_set.hpp>

#include <fstream>
#include <iostream>
#include <unordered_map>

namespace asio = boost::asio;

namespace
{
const std::string log_file_name = "mpeg-ts-demux_%Y%m%d_%H%M%S.log";
using ofs_map_t = std::unordered_map<uint16_t, std::ofstream>;
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

    asio::io_context signal_handling_ctx;

    ofs_map_t ofs_map;

    mpegts::demux_service svc(options.get_input_file_name(), signal_handling_ctx,
        [&ofs_map, &options](const mpegts::pes_packet_t &packet) {
          auto it = ofs_map.find(packet.pid);
          if (it == ofs_map.end())
          {
            std::ofstream ofs;
            auto exception_mask = ofs.exceptions() | std::ios::failbit;
            ofs.exceptions(exception_mask);
            ofs.open((boost::filesystem::path(options.get_oputput_directory()) /
                         utils::num_to_hex(packet.pid, true))
                         .string(),
                std::ios::out | std::ios::binary | std::ios::trunc);
            bool inserted;
            std::tie(it, inserted) = ofs_map.emplace(packet.pid, std::move(ofs));
          }

          BOOST_LOG_TRIVIAL(trace)
              << "Got PES packet with PID: " << utils::num_to_hex(packet.pid, true)
              << " and payload length: " << packet.payload.length;

          it->second.write(
              reinterpret_cast<const char *>(packet.payload.data), packet.payload.length);
        });

    asio::signal_set signal_set(signal_handling_ctx, SIGINT, SIGTERM);

    signal_set.async_wait([&svc](const auto &ec, int sig_code) {
      BOOST_LOG_TRIVIAL(trace) << "Got signal: " << sig_code << "; stopping...";
      if (ec)
      {
        BOOST_LOG_TRIVIAL(error) << "Error: " << ec.message();
      }

      svc.stop();
    });

    svc.start();

    int ret = signal_handling_ctx.run();
    svc.join();

    BOOST_LOG_TRIVIAL(info) << "Exiting...";

    return ret;
  }
  catch (const std::exception &e)
  {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  return 0;
}
