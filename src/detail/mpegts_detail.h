#pragma once

#include <array>
#include <boost/optional.hpp>

namespace mpegts
{
namespace detail
{
  constexpr const uint8_t TS_PACKET_SIZE = 188;
  using ts_packet_data_t = std::array<uint8_t, TS_PACKET_SIZE - sizeof(uint32_t)>;

  struct ts_packet_t
  {
    uint32_t header;
    ts_packet_data_t data;
    uint16_t pid;
    bool pusi;
    uint8_t* pes_offset_ptr;

    ts_packet_t() : header(0), pid(0), pusi(false), pes_offset_ptr(data.data())
    {
    }

    ts_packet_t(const ts_packet_t &) = delete;
    ts_packet_t &operator=(const ts_packet_t &) = delete;
  };

  using ts_packet_opt = boost::optional<ts_packet_t>;

} // namespace detail
} // namespace mpegts
