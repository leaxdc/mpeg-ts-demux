#include "ts_parser.h"
#include "logger.h"

#include <boost/endian/conversion.hpp>
#include <boost/log/trivial.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <sstream>

namespace mpegts
{
namespace detail
{
  parsed_ts_packet_opt ts_parser::parse(const ts_packet_t &ts_packet)
  {
    const auto header = boost::endian::native_to_big(ts_packet.header);

    if ((header & 0xff000000) != 0x47000000)
    {
      BOOST_LOG_TRIVIAL(trace) << "TS packet sync byte is invalid (expected 0x47), skipping";
      return {};
    }

    bool transport_error = (header & 0x800000);
    uint8_t continuity_cnt = (header & 0xf);
    bool pusi = static_cast<bool>(header & 0x400000);
    uint16_t pid = (header & 0x1fff00);
    uint8_t adaptation_field_ctl = (header & 0x30);

    if (logger::current_severity_level.load() == boost::log::trivial::severity_level::trace)
    {
      boost::property_tree::ptree pt;

      auto num_to_hex = [](auto num, bool is_0x) {
        std::stringstream ss;
        ss << std::hex << (is_0x ? "0x" : "") << num;
        return ss.str();
      };

      pt.put("ts_header.bytes_hex", num_to_hex(header,false));
      pt.put("ts_header.fields.transport_error_indicator", transport_error);
      pt.put("ts_header.fields.PUSI", pusi);
      pt.put("ts_header.fields.PID", num_to_hex(pid, true));
      pt.put("ts_header.fields.continuity_cnt", continuity_cnt);
      pt.put("ts_header.fields.adaptation_field_ctl", adaptation_field_ctl);

      std::stringstream ss;
      boost::property_tree::json_parser::write_json(ss, pt);
      BOOST_LOG_TRIVIAL(trace) << ss.str();
    }

    if (transport_error)
    {
      BOOST_LOG_TRIVIAL(trace) << "TS packet is corrupt, skipping";
      return {};
    }

    // if (!((pid >= 0x0020 && pid > 0x1FFA) || (pid >= 0x1FFC && pid <= 0x1FFE)))
    // {
    //   BOOST_LOG_TRIVIAL(trace) << "TS packet PID is outside of tables or ES range, skipping";
    //   return {};
    // }

    if (adaptation_field_ctl == 0x00 || adaptation_field_ctl == 0x02)
    {
      BOOST_LOG_TRIVIAL(trace) << "TS packet has no payload, skipping";
      return {};
    }

    parsed_ts_packet_t parsed_ts_packet{ts_packet.data.begin(), pid, pusi};

    if (adaptation_field_ctl == 0x3)
    {
      uint8_t adaptaion_field_len = *parsed_ts_packet.pes_offset_it;
      parsed_ts_packet.pes_offset_it += adaptaion_field_len;
      BOOST_LOG_TRIVIAL(trace) << "Skipped TS adaptation field of len: " << adaptaion_field_len;
    }

    return parsed_ts_packet;
  }
} // namespace detail
} // namespace mpegts
