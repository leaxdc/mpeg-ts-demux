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
#include "detail/pes_parser.h"
#include "detail/ts_parser.h"

#include <array>
#include <fstream>
#include <iomanip>
#include <sstream>

#include <boost/log/trivial.hpp>
#include <boost/thread.hpp>

namespace mpegts
{
class demux_service::impl
{
public:
  impl(const std::string &file_name, boost::asio::io_context &signal_handling_ctx,
      packet_received_callback_t callback)
      : _file_name(file_name), _signal_handling_ctx(signal_handling_ctx),
        _callback(std::move(callback))
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
        BOOST_LOG_TRIVIAL(info) << "Starting processing of file: " << _file_name;

        std::ifstream ifs;
        auto exception_mask = ifs.exceptions() | std::ios::failbit;
        ifs.exceptions(exception_mask);
        ifs.open(_file_name, std::ios::in | std::ios::binary);

        detail::ts_parser ts_parser;
        detail::pes_parser pes_parser(_callback);

        // reusing ts_packet avoids reallocating of std::array member
        detail::ts_packet_t ts_packet;

        while (!ifs.eof())
        {
          boost::this_thread::interruption_point();

          ifs.read(reinterpret_cast<char *>(&ts_packet.header), sizeof(uint32_t));
          ifs.read(reinterpret_cast<char *>(ts_packet.data.data()), ts_packet.data.size());

          if (ts_parser.parse(ts_packet))
          {
            pes_parser.feed_ts_packet(ts_packet);
            ts_packet.reset();
          }
        }

        BOOST_LOG_TRIVIAL(trace) << "Flushing...";
        pes_parser.flush();
      }
      catch (const boost::thread_interrupted &)
      {
        BOOST_LOG_TRIVIAL(trace) << "Processing tread interrupted.";
      }
      catch (const std::ios_base::failure &)
      {
        BOOST_LOG_TRIVIAL(error) << strerror(errno);
      }
      catch (const std::exception &e)
      {
        BOOST_LOG_TRIVIAL(error) << e.what();
      }
      _signal_handling_ctx.stop();
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
      BOOST_LOG_TRIVIAL(info) << "Processing thread finished.";
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
  boost::asio::io_context &_signal_handling_ctx;
  packet_received_callback_t _callback;
  std::unique_ptr<boost::thread> _processing_thread;
};

demux_service::demux_service(const std::string &file_name,
    boost::asio::io_context &signal_handling_ctx, packet_received_callback_t callback)
    : _impl(std::make_unique<impl>(file_name, signal_handling_ctx, std::move(callback)))
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
