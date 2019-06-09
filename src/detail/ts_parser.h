#include "mpegts.h"

#include <boost/optional.hpp>

#include <array>

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
  };

  struct pes_data_offset_t
  {
    ts_packet_data_t::const_iterator offset_it;
    uint16_t pid;
  };

  using pes_data_offset_opt = boost::optional<pes_data_offset_t>;

  pes_data_offset_opt parse_ts_packet(const ts_packet_t &ts_packet);
} // namespace detail
} // namespace mpegts
