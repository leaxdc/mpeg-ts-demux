#include "log_utils.h"
#include "logger.h"
#include "utils.hpp"

#include <boost/log/trivial.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

namespace mpegts
{
namespace detail
{
  namespace log_utils
  {
    void log_ts_packet(const ts_packet_t &ts_packet, uint64_t ts_packet_num)
    {
      if (logger::log_ts_packets)
      {
        boost::property_tree::ptree pt;

        pt.put("ts_packet.header_bytes_hex", utils::num_to_hex(ts_packet.header, false));
        if (ts_packet.pes_offset)
        {
          pt.put("ts_packet.pes_packet.offset", *ts_packet.pes_offset);
        }

        pt.put("ts_packet.transport_error_indicator", ts_packet.transport_error);
        pt.put("ts_packet.PUSI", ts_packet.pusi);
        pt.put("ts_packet.PID", utils::num_to_hex(ts_packet.pid, true));
        pt.put("ts_packet.continuity_cnt", ts_packet.continuity_cnt);
        pt.put("ts_packet.adaptation_field_ctl", ts_packet.adaptation_field_ctl);

        std::stringstream ss;
        boost::property_tree::json_parser::write_json(ss, pt);

        BOOST_LOG_TRIVIAL(info) << "TS packet #" << ts_packet_num << ": " << ss.str();
      }
    }

    void log_pes_packet(const pes_packet_impl_t &pes_packet, size_t pes_packet_num)
    {
      if (logger::log_pes_packets)
      {
        boost::property_tree::ptree pt;

        pt.put("pes_packet.ts_packet.pid", utils::num_to_hex(pes_packet.ts_packet_pid, true));
        pt.put("pes_packet.stream_id", utils::num_to_hex(pes_packet.stream_id, false));
        pt.put("pes_packet.max_length", pes_packet.max_length);
        pt.put("pes_packet.cur_length", pes_packet.cur_length);
        pt.put("pes_packet.payload_offset", pes_packet.payload_offset);
        pt.put("pes_packet.payload_length", pes_packet.payload_length);

        std::stringstream ss;
        boost::property_tree::json_parser::write_json(ss, pt);
        BOOST_LOG_TRIVIAL(info) << ((!pes_packet.payload_offset || !pes_packet.payload_length)
                                           ? "PES packet part #"
                                           : "PES packet #")
                                << pes_packet_num << ": " << ss.str();
      }
    }
  } // namespace log_utils
} // namespace detail
} // namespace mpegts
