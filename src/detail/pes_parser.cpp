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
#include "logger.h"
#include "utils.hpp"

#include <limits>
#include <sstream>

#include <boost/endian/conversion.hpp>
#include <boost/log/trivial.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

namespace mpegts
{
namespace detail
{
  namespace
  {
    const size_t MIN_PES_OPT_HEADER_SIZE = 3;

    bool do_checks(uint32_t start_code, uint16_t stream_id)
    {
      // expected start code is 00 00 01 <stream_id byte>
      if (((start_code >> 8) & 0x01) != 0x01)
      {
        // not a PES packet
        return false;
      }
      // https://ffmpeg.org/doxygen/3.2/mpegts_8c_source.html
      if (stream_id == 0x1bc || stream_id == 0x1bf || /* program_stream_map, private_stream_2 */
          stream_id == 0x1f0 || stream_id == 0x1f1 || /* ECM, EMM */
          stream_id == 0x1ff || stream_id == 0x1f2 || /* program_stream_directory, DSMCC_stream */
          stream_id == 0x1f8)
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
    for (auto it = _pid_to_pes_packet.begin(); it != _pid_to_pes_packet.end(); ++it)
    {
      handle_ready_pes_packet(it);
    }
  }

  void pes_parser::feed_ts_packet(const ts_packet_t &ts_packet)
  {
    pid_to_pes_packet_map_t::iterator it;

    auto offset = ts_packet.pes_offset;

    // start of PES packet
    if (ts_packet.pusi)
    {
      uint32_t start_code = boost::endian::big_to_native(
          *reinterpret_cast<const uint32_t *>(ts_packet.data.data() + offset));
      uint16_t stream_id = (start_code & 0xff) | 0x100;

      if (!do_checks(start_code, stream_id))
      {
        return;
      }

      offset += sizeof(uint32_t);

      uint16_t pes_length = boost::endian::big_to_native(
          *reinterpret_cast<const uint16_t *>(ts_packet.data.data() + offset));
      offset += sizeof(uint16_t);

      it = _pid_to_pes_packet.find(ts_packet.pid);

      if (it != _pid_to_pes_packet.end())
      {
        handle_ready_pes_packet(it);

        // reusing PES packet avoids reallocating of std::array member
        it->second.reset(stream_id, pes_length);
      }
      else
      {
        bool inserted;
        std::tie(it, inserted) = _pid_to_pes_packet.insert(
            std::make_pair(ts_packet.pid, pes_packet_impl_t{stream_id, pes_length}));
      }
    }
    else
    {
      it = _pid_to_pes_packet.find(ts_packet.pid);

      if (it == _pid_to_pes_packet.end())
      {
        // PUSI bit is 0, but PID is not in map, skipping
        return;
      }
    }

    auto ts_pes_length = ts_packet.data.size() - offset;

    memcpy(&it->second.data[0] + it->second.cur_data_length, ts_packet.data.data() + offset,
        ts_pes_length);

    it->second.cur_data_length += ts_pes_length;
  }

  void pes_parser::handle_ready_pes_packet(const pid_to_pes_packet_map_t::iterator &it)
  {
    uint32_t opt_pes_header =
        boost::endian::big_to_native(*reinterpret_cast<const uint32_t *>(it->second.data.data()));

    uint8_t payload_offset = ((opt_pes_header & 0xff00) >> 8) + MIN_PES_OPT_HEADER_SIZE;
    auto payload_length = it->second.cur_data_length - payload_offset;

    if (logger::log_pes_packets)
    {
      boost::property_tree::ptree pt;

      pt.put("pes_packet.ts_packet_pid", utils::num_to_hex(it->first, true));
      pt.put("pes_packet.stream_id", utils::num_to_hex(it->second.stream_id, false));
      pt.put("pes_packet.length", it->second.length);
      pt.put("pes_packet.payload_length", payload_length);

      std::stringstream ss;
      boost::property_tree::json_parser::write_json(ss, pt);
      BOOST_LOG_TRIVIAL(info) << "PES packet #" << _pes_packet_num++ << ": " << ss.str();
    }

    _callback(pes_packet_t{
        it->first, buffer_slice{it->second.data.data() + payload_offset, payload_length}});
  }
} // namespace detail
} // namespace mpegts
