#pragma once

#include "logger.h"
#include "mpegts.h"
#include "mpegts_detail.h"

#include <boost/endian/conversion.hpp>
#include <boost/log/trivial.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <limits>
#include <sstream>
#include <unordered_map>

namespace mpegts
{
namespace detail
{
  // https://ffmpeg.org/doxygen/3.2/mpegts_8c_source.html
  const size_t MAX_PES_PAYLOAD_SIZE = 1024 * 200;
  const size_t MIN_PES_OPT_HEADER_SIZE = 3;

  struct pes_packet_impl_t
  {
    uint16_t stream_id;
    size_t pes_length;

    std::array<uint8_t, MAX_PES_PAYLOAD_SIZE> data;
    size_t cur_data_length;

    pes_packet_impl_t(uint16_t stream_id, size_t pes_length)
    {
      reset(stream_id, pes_length);
    }

    void reset(uint16_t stream_id, size_t pes_length)
    {
      this->stream_id = stream_id;
      this->pes_length = pes_length;
      this->cur_data_length = 0;
    }
  };

  class pes_parser
  {
  public:
    template <typename pes_packet_callback_t>
    void flush(pes_packet_callback_t callback)
    {
      for (auto it = _pid_to_pes_packet.begin(); it != _pid_to_pes_packet.end(); ++it)
      {
        parse_ready_pes(it, callback);
      }
    }

    template <typename pes_packet_callback_t>
    void parse(ts_packet_t ts_packet, pes_packet_callback_t callback)
    {
      pid_to_pes_packet_map_t::iterator it;

      if (ts_packet.pusi)
      {
        uint32_t start_code = boost::endian::big_to_native(
            *reinterpret_cast<const uint32_t *>(ts_packet.data.data() + ts_packet.pes_offset));

        BOOST_LOG_TRIVIAL(trace) << "---1";

        ts_packet.pes_offset += sizeof(uint32_t);

        BOOST_LOG_TRIVIAL(trace) << "---2: " << ts_packet.pes_offset;

        if (((start_code >> 8) & 0x01) != 0x01)
        {
          BOOST_LOG_TRIVIAL(trace) << "Not a PES packet, skipping";
          return;
        }

        // https://ffmpeg.org/doxygen/3.2/mpegts_8c_source.html
        uint16_t stream_id = (start_code & 0xff) | 0x100;
        if (stream_id == 0x1bc || stream_id == 0x1bf || /* program_stream_map, private_stream_2 */
            stream_id == 0x1f0 || stream_id == 0x1f1 || /* ECM, EMM */
            stream_id == 0x1ff || stream_id == 0x1f2 || /* program_stream_directory, DSMCC_stream */
            stream_id == 0x1f8)
        {
          BOOST_LOG_TRIVIAL(trace) << "PES doesn't contain a media stream, skipping";
          return;
        }

        auto num_to_hex = [](uint32_t num, bool is_0x) {
          std::stringstream ss;
          ss << std::hex << (is_0x ? "0x" : "") << num;
          return ss.str();
        };

        uint16_t pes_length = boost::endian::big_to_native(
            *reinterpret_cast<const uint16_t *>(ts_packet.data.data() + ts_packet.pes_offset));

        BOOST_LOG_TRIVIAL(trace) << "---3: " << pes_length;

        ts_packet.pes_offset += sizeof(uint16_t);

        if (logger::current_severity_level.load() == boost::log::trivial::severity_level::trace)
        {
          boost::property_tree::ptree pt;

          pt.put("pes_preparsed_header.start_code_hex", num_to_hex(start_code, false));
          pt.put("pes_preparsed_header.stream_id", num_to_hex(stream_id, true));
          pt.put("pes_preparsed_header.pes_length", pes_length);

          std::stringstream ss;
          boost::property_tree::json_parser::write_json(ss, pt);
          BOOST_LOG_TRIVIAL(trace) << ss.str();
        }

        it = _pid_to_pes_packet.find(ts_packet.pid);

        if (it != _pid_to_pes_packet.end())
        {
          parse_ready_pes(it, callback);
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
          BOOST_LOG_TRIVIAL(trace) << "PUSI is 0, PID is not in PES map, skipping";
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

    template <typename pes_packet_callback_t>
    void parse_ready_pes(
        const pid_to_pes_packet_map_t::iterator &it, pes_packet_callback_t callback)
    {
      std::stringstream ss;
      ss << std::hex << "0x" << it->first;

      BOOST_LOG_TRIVIAL(trace) << "Sending PES packet with TS pid: " << ss.str();

      uint32_t opt_pes_header =
          boost::endian::big_to_native(*reinterpret_cast<const uint32_t *>(it->second.data.data()));

      uint8_t payload_offset = ((opt_pes_header & 0xff00) >> 8) + MIN_PES_OPT_HEADER_SIZE;
      BOOST_LOG_TRIVIAL(trace) << "Payload offset: " << (uint32_t)payload_offset;

      callback(pes_packet_t{it->first,
          buffer_slice{it->second.data.data() + payload_offset,
              it->second.cur_data_length - payload_offset}});
    }
  };

} // namespace detail
} // namespace mpegts
