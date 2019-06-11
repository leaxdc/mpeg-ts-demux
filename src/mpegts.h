#pragma once

#include <functional>
#include <vector>

namespace mpegts
{
namespace detail
{
  class pes_parser;
} // namespace detail

using byte_vec = std::vector<uint8_t>;
struct pes_data
{
  byte_vec buffer;
  size_t length;
};

struct pes_packet_t
{
  pes_packet_t(uint16_t pid, size_t buffer_size) : pid(pid), data{byte_vec(buffer_size), 0}
  {
  }

  uint16_t pid;
  pes_data data;

  virtual ~pes_packet_t() = default;
  pes_packet_t(const pes_packet_t &) = delete;
  pes_packet_t &operator=(const pes_packet_t &) = delete;
  pes_packet_t(pes_packet_t &&) = default;
  pes_packet_t &operator=(pes_packet_t &&) = default;

  friend class pes_parser;
};

using packet_received_callback_t = std::function<void(const pes_packet_t&)>;

} // namespace mpegts
