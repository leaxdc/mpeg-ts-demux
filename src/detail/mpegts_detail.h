#pragma once

#include <array>

namespace mpegts
{
namespace detail
{
  // https://en.wikipedia.org/wiki/MPEG_transport_stream#Important_elements_of_a_transport_stream
  constexpr const uint8_t TS_PACKET_SIZE = 188;
  using ts_packet_data_t = std::array<uint8_t, TS_PACKET_SIZE - sizeof(uint32_t)>;

  struct ts_packet_t
  {
    uint32_t header;
    ts_packet_data_t data;
    uint16_t pid;
    bool pusi;
    size_t pes_offset;

    ts_packet_t()
    {
      reset();
    }

    void reset()
    {
      header = 0;
      pid = 0;
      pusi = false;
      pes_offset = 0;
    }
  };
} // namespace detail
} // namespace mpegts
