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
#include <unordered_map>

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

    using ofs_map_t = std::unordered_map<uint16_t, std::ofstream>;
    using ofs_map_uptr = std::unique_ptr<ofs_map_t, std::function<void (ofs_map_t*)>>;

    auto  ofs_map = ofs_map_uptr(new ofs_map_t(), [](ofs_map_t *map) {
      for (auto &value : *map) {
          value.second.close();
      }
    });

    mpegts::demux_service svc(
        options.get_input_file_name(), main_context, [&ofs_map, &options](const mpegts::pes_packet_t& packet) {

          auto it = ofs_map->find(packet.pid);
          if (it == ofs_map->end())
          {
            std::ofstream ofs;
            auto exception_mask = ofs.exceptions() | std::ios::failbit;
            ofs.exceptions(exception_mask);
            ofs.open((boost::filesystem::path(options.get_oputput_directory()) / std::to_string(packet.pid)).string(), std::ios::out | std::ios::binary);
            bool inserted;
            std::tie(it, inserted) = ofs_map->insert(std::make_pair(packet.pid, std::move(ofs)));
          }

          BOOST_LOG_TRIVIAL(info) << "Got PES packet with pid: " << packet.pid << "; payload length: " << packet.payload.length;

          it->second.write(reinterpret_cast<const char*>(packet.payload.data), packet.payload.length );
        });

    // asio::signal_set signal_set(main_context, SIGINT, SIGTERM);

    // signal_set.async_wait([&svc](const auto &ec, int sig_code) {
    //   BOOST_LOG_TRIVIAL(trace) << "Got signal: " << sig_code << "stopping...";

    //   if (ec)
    //   {
    //     BOOST_LOG_TRIVIAL(error) << "Error: " << ec.message();
    //   }

    //   svc.stop();
    // });

    svc.start();

    // int ret = main_context.run();

    svc.join();

    BOOST_LOG_TRIVIAL(info) << "Exitting...";

    return 0;
  }
  catch (const std::exception &e)
  {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  return 0;
}
