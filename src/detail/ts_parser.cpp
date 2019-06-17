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
#include "log_utils.h"
#include "utils.hpp"

#include <boost/endian/conversion.hpp>
#include <boost/log/trivial.hpp>

namespace mpegts
{
namespace detail
{
  namespace
  {
    bool do_checks(const ts_packet_t &ts_packet)
    {
      if (ts_packet.sync_byte != 0x47)
      {
        BOOST_LOG_TRIVIAL(trace) << "TS packet sync byte is invalid (expected 0x47), skipping";
        return false;
      }

      if (ts_packet.transport_error)
      {
        BOOST_LOG_TRIVIAL(trace) << "TS packet is corrupt, skipping";
        return {};
      }

      if (ts_packet.adaptation_field_ctl == 0x00 || ts_packet.adaptation_field_ctl == 0x02)
      {
        BOOST_LOG_TRIVIAL(trace) << "TS packet has no payload, skipping";
        return {};
      }

      if (!((ts_packet.pid >= 0x20 && ts_packet.pid <= 0x1FFA) ||
              (ts_packet.pid >= 0x1FFC && ts_packet.pid <= 0x1FFE)))
      {
        BOOST_LOG_TRIVIAL(trace) << "TS packet PID is outside of tables or PES range, skipping";
        return {};
      }

      return true;
    }
  } // namespace

  void ts_parser::handle_continuity_cnt(const ts_packet_t &ts_packet)
  {
    const auto it = _pid_to_continuity_cnt.find(ts_packet.pid);
    if (_pid_to_continuity_cnt.find(ts_packet.pid) != _pid_to_continuity_cnt.end())
    {
      if (ts_packet.continuity_cnt - it->second != 1 && it->second != 15 &&
          ts_packet.continuity_cnt != 0)
      {
        BOOST_LOG_TRIVIAL(warning)
            << "TS packet loss detected, PID: " << utils::num_to_hex(ts_packet.pid, true);
      }
    }

    _pid_to_continuity_cnt[ts_packet.pid] = ts_packet.continuity_cnt;
  }

  ts_packet_opt ts_parser::parse(ts_packet_t ts_packet)
  {
    // converting to big endian because using big endian masks from the documentation:
    // https://en.wikipedia.org/wiki/MPEG_transport_stream#Important_elements_of_a_transport_stream

    auto header = boost::endian::native_to_big(ts_packet.header);

    ts_packet.sync_byte = (header & 0xff000000) >> 24;
    ts_packet.transport_error = (header & 0x800000);
    ts_packet.continuity_cnt = (header & 0xf);
    ts_packet.pusi = static_cast<bool>(header & 0x400000);
    ts_packet.pid = (header & 0x1fff00) >> 8;
    ts_packet.adaptation_field_ctl = (header & 0x30) >> 4;

    if (!do_checks(ts_packet))
    {
      log_utils::log_ts_packet(ts_packet, _ts_packet_num++);
      return {};
    }

    if (ts_packet.adaptation_field_ctl == 0x3)
    {
      uint8_t adaptaion_field_len = ts_packet.data[0];
      ts_packet.pes_offset = sizeof(uint8_t) + adaptaion_field_len;
    }
    else
    {
      ts_packet.pes_offset = 0;
    }

    log_utils::log_ts_packet(ts_packet, _ts_packet_num++);

    handle_continuity_cnt(ts_packet);

    return ts_packet;
  }
} // namespace detail
} // namespace mpegts
