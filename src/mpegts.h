#pragma once

#include <functional>
#include <vector>

namespace mpegts
{
struct buffer_slice
{
  const uint8_t *data;
  size_t length;
};

struct pes_packet_t
{
  uint16_t pid;
  buffer_slice payload;
};

using packet_received_callback_t = std::function<void(const pes_packet_t &)>;

} // namespace mpegts
