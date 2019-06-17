/*

Copyright 2019 Peter Asanov

Permission is hereby granted, free of charge,
to any person obtaining a copy of this software and associated documentation files( the "Software"),
to deal in the Software without restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and / or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#include "pes_parser.h"
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
    const size_t MIN_PES_OPT_HEADER_SIZE = 3;

    bool do_checks(const pes_packet_impl_t &pes_packet)
    {
      // expected start code is 00 00 01 <stream_id byte>
      if (((pes_packet.start_code >> 8) & 0x01) != 0x01)
      {
        // not a PES packet
        return false;
      }
      // https://ffmpeg.org/doxygen/3.2/mpegts_8c_source.html
      if (pes_packet.stream_id == 0x1bc ||
          pes_packet.stream_id == 0x1bf || /* program_stream_map, private_stream_2 */
          pes_packet.stream_id == 0x1f0 || pes_packet.stream_id == 0x1f1 || /* ECM, EMM */
          pes_packet.stream_id == 0x1ff ||
          pes_packet.stream_id == 0x1f2 || /* program_stream_directory, DSMCC_stream */
          pes_packet.stream_id == 0x1f8)
      {
        // PES doesn't contain a media stream
        return false;
      }
      return true;
    }
  } // namespace

  pes_parser::pes_parser(packet_received_callback_t callback) : _callback(std::move(callback))
  {
  }

  void pes_parser::flush()
  {
    std::for_each(begin(_pid_to_pes_packet), end(_pid_to_pes_packet),
        std::bind(&pes_parser::handle_ready_pes_packet, this, std::placeholders::_1));
  }

  pid_to_pes_packet_map_t::iterator pes_parser::handle_pusi_packet(ts_packet_t &ts_packet)
  {
    pid_to_pes_packet_map_t::iterator map_it;

    if (!ts_packet.pusi)
    {
      BOOST_LOG_TRIVIAL(warning) << "Not PUSI packet, skipping";
      return map_it;
    }

    pes_packet_impl_t pes_packet{};

    pes_packet.ts_packet_pid = ts_packet.pid;
    pes_packet.start_code = boost::endian::big_to_native(
        *reinterpret_cast<const uint32_t *>(&ts_packet.data[*ts_packet.pes_offset]));
    pes_packet.stream_id = (pes_packet.start_code & 0xff) | 0x100;

    if (!do_checks(pes_packet))
    {
      log_utils::log_pes_packet(pes_packet, _pes_packet_num++);
      return map_it;
    }

    *ts_packet.pes_offset += sizeof(uint32_t);

    pes_packet.max_length = boost::endian::big_to_native(
        *reinterpret_cast<const uint16_t *>(&ts_packet.data[*ts_packet.pes_offset]));
    *ts_packet.pes_offset += sizeof(uint16_t);

    map_it = _pid_to_pes_packet.find(ts_packet.pid);

    if (map_it != _pid_to_pes_packet.end())
    {
      handle_ready_pes_packet(*map_it);
      std::swap(map_it->second, pes_packet);
    }
    else
    {
      map_it = _pid_to_pes_packet.emplace(ts_packet.pid, std::move(pes_packet)).first;
    }

    return map_it;
  }

  void pes_parser::feed_ts_packet(ts_packet_t ts_packet)
  {
    if (!ts_packet.pes_offset)
    {
      BOOST_LOG_TRIVIAL(warning) << "PES offset is not set, skipping";
      return;
    }

    pid_to_pes_packet_map_t::iterator map_it;

    // start of PES packet
    if (ts_packet.pusi)
    {
      map_it = handle_pusi_packet(ts_packet);
      if (map_it == _pid_to_pes_packet.end())
      {
        return;
      }
    }
    else
    {
      map_it = _pid_to_pes_packet.find(ts_packet.pid);

      if (map_it == _pid_to_pes_packet.end())
      {
        // PUSI bit is 0, but PID is not in map, skipping
        return;
      }
    }

    const auto ts_pes_length = ts_packet.data.size() - *ts_packet.pes_offset;

    const auto out_it = begin(map_it->second.data) + map_it->second.cur_length;
    const auto in_it_start = cbegin(ts_packet.data) + *ts_packet.pes_offset;
    const auto in_it_end = in_it_start + ts_pes_length;

    std::copy(in_it_start, in_it_end, out_it);
    map_it->second.cur_length += ts_pes_length;
  }

  void pes_parser::handle_ready_pes_packet(pid_to_pes_packet_map_t::value_type &v)
  {
    auto &pes_packet = v.second;

    const uint32_t opt_pes_header =
        boost::endian::big_to_native(*reinterpret_cast<const uint32_t *>(&pes_packet.data[0]));

    pes_packet.payload_offset = ((opt_pes_header & 0xff00) >> 8) + MIN_PES_OPT_HEADER_SIZE;
    pes_packet.payload_length = pes_packet.cur_length - pes_packet.payload_offset;

    log_utils::log_pes_packet(pes_packet, _pes_packet_num);

    _callback(pes_packet_t{v.first,
        buffer_slice{&pes_packet.data[pes_packet.payload_offset], pes_packet.payload_length}});
  }
} // namespace detail
} // namespace mpegts
