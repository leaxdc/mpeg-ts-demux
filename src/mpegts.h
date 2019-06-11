#pragma once

#include <memory>
#include <vector>
#include <functional>

namespace mpegts
{

namespace detail
{
struct pes_packet_details_t;
} // namespace detail

using byte_vec = std::vector<uint8_t>;

struct pes_packet_t
{
protected:
  pes_packet_t(uint16_t pid, size_t data_size) : pid(pid), data(data_size)
  {
  }

public:
  uint16_t pid;
  byte_vec data;

  virtual ~pes_packet_t() = default;
  pes_packet_t(const pes_packet_t &) = delete;
  pes_packet_t &operator=(const pes_packet_t &) = delete;
  pes_packet_t(pes_packet_t &&) = default;
  pes_packet_t &operator=(pes_packet_t &&) = default;

  friend struct pes_packet_details_t;
};

using pes_packet_uptr = std::unique_ptr<pes_packet_t>;
using packet_received_callback_t = std::function<void(pes_packet_uptr)>;

} // namespace mpegts
