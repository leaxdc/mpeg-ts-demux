#pragma once

#include <vector>

namespace mpegts
{
using byte_vec = std::vector<uint8_t>;

namespace elementary_stream
{
  enum class packet_type
  {
    video = 0,
    audio
  };

  struct packet
  {
    byte_vec data;
    packet_type type;
  };
} // namespace elementary_stream

using data_received_callback = std::function<void(elementary_stream::packet)>;
} // namespace mpegts
