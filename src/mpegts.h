#pragma once

#include <memory>
#include <vector>

namespace mpegts
{
// enum class pes_packet_type
// {
//   video = 0,
//   audio
// };

using buffer_slice_t = std::pair<const uint8_t *, size_t>;

struct pes_packet_t
{
  buffer_slice_t data;
  uint8_t stream_id;

  pes_packet_t(buffer_slice_t data, uint8_t stream_id) : data(data), stream_id(stream_id)
  {
  }

  virtual ~pes_packet_t() = default;
  pes_packet_t(const pes_packet_t &) = delete;
  pes_packet_t &operator=(const pes_packet_t &) = delete;
};

using pes_packet_uptr = std::unique_ptr<pes_packet_t>;
using packet_received_callback_t = std::function<void(pes_packet_uptr)>;

} // namespace mpegts
