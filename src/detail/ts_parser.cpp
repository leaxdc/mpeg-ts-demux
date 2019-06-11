/*
Copyright 2019 Peter Asanov

Permission is hereby granted, free of charge,
to any person obtaining a copy of this software and associated documentation files(
    the "Software"),
to deal in the Software without restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and / or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so,
subject to the following conditions :

The above copyright notice and this permission notice shall be included in all copies
or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS",
WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "ts_parser.h"
#include "logger.h"
#include "utils.hpp"

#include <sstream>

#include <boost/endian/conversion.hpp>
#include <boost/log/trivial.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

namespace mpegts
{
namespace detail
{
  bool ts_parser::parse(ts_packet_t &ts_packet)
  {
    // converting to big endian because using big endian masks from the documentation:
    // https://en.wikipedia.org/wiki/MPEG_transport_stream#Important_elements_of_a_transport_stream

    auto header = boost::endian::native_to_big(ts_packet.header);

    if ((header & 0xff000000) != 0x47000000)
    {
      // TS packet sync byte is invalid (expected 0x47), skipping
      return false;
    }

    bool transport_error = (header & 0x800000);
    int8_t continuity_cnt = (header & 0xf);

    bool pusi = static_cast<bool>(header & 0x400000);
    ts_packet.pusi = pusi;

    uint16_t pid = (header & 0x1fff00) >> 8;
    ts_packet.pid = pid;

    uint8_t adaptation_field_ctl = (header & 0x30) >> 4;

    if (logger::log_ts_packets)
    {
      boost::property_tree::ptree pt;

      pt.put("ts_packet.header_bytes_hex", utils::num_to_hex(header, false));
      pt.put("ts_packet.fields.transport_error_indicator", transport_error);
      pt.put("ts_packet.fields.PUSI", pusi);
      pt.put("ts_packet.fields.PID", utils::num_to_hex(pid, true));
      pt.put("ts_packet.fields.continuity_cnt", continuity_cnt);
      pt.put("ts_packet.fields.adaptation_field_ctl", adaptation_field_ctl);

      std::stringstream ss;
      boost::property_tree::json_parser::write_json(ss, pt);
      BOOST_LOG_TRIVIAL(info) << "TS packet #" << _ts_packet_num++ << ": " << ss.str();
    }

    if (transport_error)
    {
      BOOST_LOG_TRIVIAL(trace) << "TS packet is corrupt, skipping";
      return false;
    }

    if (adaptation_field_ctl == 0x00 || adaptation_field_ctl == 0x02)
    {
      BOOST_LOG_TRIVIAL(trace) << "TS packet has no payload, skipping";
      return false;
    }

    if (!((pid >= 0x20 && pid <= 0x1FFA) || (pid >= 0x1FFC && pid <= 0x1FFE)))
    {
      BOOST_LOG_TRIVIAL(trace) << "TS packet PID is outside of tables or PES range, skipping";
      return false;
    }

    if (adaptation_field_ctl == 0x3)
    {
      uint8_t adaptaion_field_len = *(ts_packet.data.data() + ts_packet.pes_offset);
      ts_packet.pes_offset += sizeof(uint8_t) + adaptaion_field_len;
    }

    const auto it = _pid_to_continuity_cnt.find(pid);
    if (_pid_to_continuity_cnt.find(pid) != _pid_to_continuity_cnt.end())
    {
      if (continuity_cnt - it->second != 1 && it->second != 15 && continuity_cnt != 0)
      {
        BOOST_LOG_TRIVIAL(warning)
            << "TS packet loss detected, PID: " << utils::num_to_hex(ts_packet.pid, true);
      }
    }

    _pid_to_continuity_cnt[pid] = continuity_cnt;

    return true;
  }
} // namespace detail
} // namespace mpegts
