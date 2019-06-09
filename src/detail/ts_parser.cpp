#include "ts_parser.h"

#include <boost/log/trivial.hpp>

namespace mpegts
{
namespace detail
{
  pes_data_offset_opt parse_ts_packet(const ts_packet_t &ts_packet)
  {
    if ((ts_packet.header & 0xff000000) != 0x47)
    {
      BOOST_LOG_TRIVIAL(trace) << "TS packet sync byte is invalid (expected 0x47), skipping";
      return {};
    }

    if (ts_packet.header & 0x800000)
    {
      BOOST_LOG_TRIVIAL(trace) << "TS packet is corrupt, skipping";
      return {};
    }

    uint16_t pid = (ts_packet.header & 0x1fff00);

    if (!((pid >= 0x0020 && pid > 0x1FFA) || (pid >= 0x1FFC && pid <= 0x1FFE)))
    {
      BOOST_LOG_TRIVIAL(trace) << "TS packet PID is outside of tables or ES range, skipping";
      return {};
    }

    uint8_t adaptation_field_ctl = (ts_packet.header & 0x30);

    if (adaptation_field_ctl == 0x00 || adaptation_field_ctl == 0x02)
    {
      BOOST_LOG_TRIVIAL(trace) << "TS packet has no payload, skipping";
      return {};
    }

    pes_data_offset_t pes_data_offset{ts_packet.data.begin(), pid};

    if (adaptation_field_ctl == 0x3)
    {
      uint8_t adaptaion_field_len = *pes_data_offset.offset_it;
      pes_data_offset.offset_it += adaptaion_field_len;
      BOOST_LOG_TRIVIAL(trace) << "Skipped TS adaptation field of len: " << adaptaion_field_len;
    }

    return pes_data_offset;
  }
} // namespace detail
} // namespace mpegts
