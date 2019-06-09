#pragma once

#include "mpegts.h"

// allow move constructible handlers
#define BOOST_ASIO_DISABLE_HANDLER_TYPE_REQUIREMENTS
#include <boost/asio/io_context.hpp>

#include <memory>
#include <string>

namespace mpegts
{
class demux_service
{
public:
  explicit demux_service(const std::string &file_name, boost::asio::io_context &io_context,
  	packet_received_callback_t callback);
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
