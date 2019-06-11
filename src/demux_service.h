#pragma once

#include "mpegts.h"

#include <memory>
#include <string>

#include <boost/asio/io_context.hpp>

namespace mpegts
{
class demux_service
{
public:
  explicit demux_service(const std::string &file_name,
      boost::asio::io_context &signal_listening_context, packet_received_callback_t callback);
  ~demux_service();
  demux_service(const demux_service &) = delete;
  demux_service &operator=(const demux_service &) = delete;

  void start();
  void stop();
  void join();
  void reset();

private:
  class impl;
  std::unique_ptr<impl> _impl;
};
} // namespace mpegts
