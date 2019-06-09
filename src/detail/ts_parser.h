#include "mpegts.h"

#include <boost/optional.hpp>
#include <unordered_map>

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

  struct parsed_ts_packet_t
  {
    ts_packet_data_t::const_iterator pes_offset_it;
    uint16_t pid;
    bool pusi;
  };

  using parsed_ts_packet_opt = boost::optional<parsed_ts_packet_t>;

  class ts_parser
  {
  public:
    parsed_ts_packet_opt parse(const ts_packet_t &ts_packet);

  private:
    std::unordered_map<uint16_t, uint8_t> _pid_to_continuity_cnt;
  };

} // namespace detail
} // namespace mpegts
