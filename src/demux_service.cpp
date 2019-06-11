#include "demux_service.h"
#include "detail/ts_parser.h"
#include "detail/pes_parser.hpp"

#include <boost/log/trivial.hpp>
#include <boost/thread.hpp>

#include <array>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace mpegts
{
class demux_service::impl
{
public:
  impl(const std::string &file_name, boost::asio::io_context &/*io_context*/,
      packet_received_callback_t callback)
      : _file_name(file_name), /*_io_context(io_context),*/ _callback(std::move(callback))
  {
    if (!_callback)
    {
      throw std::runtime_error("callback is not set");
    }
  }

  void start()
  {
    if (_processing_thread)
    {
      throw std::runtime_error("Processing is already started");
    }

    _processing_thread = std::make_unique<boost::thread>([this]() {
      try
      {
        BOOST_LOG_TRIVIAL(trace) << "Starting processing of file: " << _file_name;

        std::ifstream ifs;
        auto exception_mask = ifs.exceptions() | std::ios::failbit;
        ifs.exceptions(exception_mask);
        ifs.open(_file_name, std::ios::in | std::ios::binary);

        detail::ts_parser ts_parser;
        detail::pes_parser pes_parser;

        while (!ifs.eof())
        {
            // boost::this_thread::interruption_point();

            detail::ts_packet_t ts_packet;
            ifs.read(reinterpret_cast<char*>(&ts_packet.header), sizeof(uint32_t));
            ifs.read(reinterpret_cast<char*>(ts_packet.data.data()), ts_packet.data.size());

            auto ts_packet_opt = ts_parser.parse(std::move(ts_packet));
            if (ts_packet_opt)
            {
              pes_parser.parse(std::move(*ts_packet_opt), _callback);
            }
        }

        BOOST_LOG_TRIVIAL(trace) << "Flushing...";

        pes_parser.flush(_callback);
      }
      catch (const boost::thread_interrupted &)
      {
        BOOST_LOG_TRIVIAL(trace) << "Processing tread interrupted";
      }
      catch (const std::exception &e)
      {
        BOOST_LOG_TRIVIAL(error) << e.what();
      }
      // _io_context.stop();
    });
  }
  void stop()
  {
    if (_processing_thread)
    {
      BOOST_LOG_TRIVIAL(trace) << "Interrupting processing thread...";
      _processing_thread->interrupt();
    }
  }

  void join()
  {
    if (_processing_thread)
    {
      BOOST_LOG_TRIVIAL(trace) << "Joining processing thread...";
      _processing_thread->join();
      BOOST_LOG_TRIVIAL(trace) << "Processing thread finished";
    }
  }

  void reset()
  {
    stop();
    join();
    _processing_thread.reset();
  }

private:
  const std::string _file_name;
  // boost::asio::io_context &_io_context;
  packet_received_callback_t _callback;
  std::unique_ptr<boost::thread> _processing_thread;
};

demux_service::demux_service(const std::string &file_name, boost::asio::io_context &io_context,
    packet_received_callback_t callback)
    : _impl(std::make_unique<impl>(file_name, io_context, std::move(callback)))
{
}

demux_service::~demux_service()
{
}

void demux_service::start()
{
  _impl->start();
}

void demux_service::stop()
{
  _impl->stop();
}
void demux_service::join()
{
  _impl->join();
}
void demux_service::reset()
{
  _impl->reset();
}
} // namespace mpegts
