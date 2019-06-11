#pragma once

#include "logger.h"
#include "mpegts.h"
#include "mpegts_detail.h"
#include "utils.hpp"

#include <limits>
#include <sstream>
#include <unordered_map>

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
    // https://ffmpeg.org/doxygen/3.2/mpegts_8c_source.html
    const size_t MAX_PES_PAYLOAD_SIZE = 1024 * 200;
    const size_t MIN_PES_OPT_HEADER_SIZE = 3;

    struct pes_packet_impl_t
    {
      uint16_t stream_id;
      size_t length;

      std::array<uint8_t, MAX_PES_PAYLOAD_SIZE> data;
      size_t cur_data_length;

      pes_packet_impl_t(uint16_t stream_id, size_t length)
      {
        reset(stream_id, length);
      }

      void reset(uint16_t stream_id, size_t length)
      {
        this->stream_id = stream_id;
        this->length = length;
        this->cur_data_length = 0;
      }
    };
  } // namespace

  class pes_parser
  {
  public:
    // flush existing PES packets, assume they are ready by the end of TS stream
    template <typename pes_packet_callback_t>
    void flush(pes_packet_callback_t callback)
    {
      for (auto it = _pid_to_pes_packet.begin(); it != _pid_to_pes_packet.end(); ++it)
      {
        handle_ready_pes_packet(it, callback);
      }
    }

    template <typename pes_packet_callback_t>
    void parse(ts_packet_t &ts_packet, pes_packet_callback_t callback)
    {
      pid_to_pes_packet_map_t::iterator it;

      // start of PES packet
      if (ts_packet.pusi)
      {
        uint32_t start_code = boost::endian::big_to_native(
            *reinterpret_cast<const uint32_t *>(ts_packet.data.data() + ts_packet.pes_offset));
        ts_packet.pes_offset += sizeof(uint32_t);

        // expected start code is 00 00 01 <stream_id byte>
        if (((start_code >> 8) & 0x01) != 0x01)
        {
          // not a PES packet
          return;
        }

        // https://ffmpeg.org/doxygen/3.2/mpegts_8c_source.html
        uint16_t stream_id = (start_code & 0xff) | 0x100;
        if (stream_id == 0x1bc || stream_id == 0x1bf || /* program_stream_map, private_stream_2 */
            stream_id == 0x1f0 || stream_id == 0x1f1 || /* ECM, EMM */
            stream_id == 0x1ff || stream_id == 0x1f2 || /* program_stream_directory, DSMCC_stream */
            stream_id == 0x1f8)
        {
          // PES doesn't contain a media stream
          return;
        }

        uint16_t pes_length = boost::endian::big_to_native(
            *reinterpret_cast<const uint16_t *>(ts_packet.data.data() + ts_packet.pes_offset));
        ts_packet.pes_offset += sizeof(uint16_t);

        it = _pid_to_pes_packet.find(ts_packet.pid);

        if (it != _pid_to_pes_packet.end())
        {
          handle_ready_pes_packet(it, callback);

          // reusing PES packet avoids reallocating of std::array member
          it->second.reset(stream_id, (!pes_length ? MAX_PES_PAYLOAD_SIZE : pes_length));
        }
        else
        {
          bool inserted;
          std::tie(it, inserted) = _pid_to_pes_packet.insert(std::make_pair(ts_packet.pid,
              pes_packet_impl_t{stream_id, (!pes_length ? MAX_PES_PAYLOAD_SIZE : pes_length)}));
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

      const auto ts_pes_length = ts_packet.data.size() - ts_packet.pes_offset;

      memcpy(&it->second.data[0] + it->second.cur_data_length,
          ts_packet.data.data() + ts_packet.pes_offset, ts_pes_length);

      it->second.cur_data_length += ts_pes_length;
    }

  private:
    using pid_to_pes_packet_map_t = std::unordered_map<uint16_t, pes_packet_impl_t>;
    pid_to_pes_packet_map_t _pid_to_pes_packet;
    uint64_t _pes_packet_num = 0;

    template <typename pes_packet_callback_t>
    void handle_ready_pes_packet(
        const pid_to_pes_packet_map_t::iterator &it, pes_packet_callback_t callback)
    {
      uint32_t opt_pes_header =
          boost::endian::big_to_native(*reinterpret_cast<const uint32_t *>(it->second.data.data()));

      uint8_t payload_offset = ((opt_pes_header & 0xff00) >> 8) + MIN_PES_OPT_HEADER_SIZE;

      if (logger::log_pes_packets)
      {
        boost::property_tree::ptree pt;

        pt.put("pes_packet.ts_packet_pid", utils::num_to_hex(it->first, true));
        pt.put("pes_packet.stream_id", utils::num_to_hex(it->second.stream_id, false));
        pt.put("pes_packet.length", it->second.length);

        std::stringstream ss;
        boost::property_tree::json_parser::write_json(ss, pt);
        BOOST_LOG_TRIVIAL(info) << "PES packet #" << _pes_packet_num++ << ": " << ss.str();
      }

      callback(pes_packet_t{it->first,
          buffer_slice{it->second.data.data() + payload_offset,
              it->second.cur_data_length - payload_offset}});
    }
  };

} // namespace detail
} // namespace mpegts
