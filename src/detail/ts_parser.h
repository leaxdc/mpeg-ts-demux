#include "mpegts.h"
#include "mpegts_detail.h"

#include <unordered_map>

namespace mpegts
{
namespace detail
{
  class ts_parser
  {
  public:
    bool parse(ts_packet_t &ts_packet);

  private:
    std::unordered_map<uint16_t, int8_t> _pid_to_continuity_cnt;
  };

} // namespace detail
} // namespace mpegts
