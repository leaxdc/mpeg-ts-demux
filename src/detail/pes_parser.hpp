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

  class pes_parser
  {
  private:
    using pid_to_pes_packet_map_t = std::unordered_map<uint16_t, pes_packet_t>;
    pid_to_pes_packet_map_t _pid_to_pes_packet;

    template <typename pes_packet_callback_t>
    void flush(const pid_to_pes_packet_map_t::iterator &it, pes_packet_callback_t callback)
    {
      BOOST_LOG_TRIVIAL(trace) << "Flushing PES packet with TS pid: " << it->first;

      callback(std::move(it->second));
      _pid_to_pes_packet.erase(it);
    }

  public:
    template <typename pes_packet_callback_t>
    void flush(pes_packet_callback_t callback)
    {
      for (auto &v : _pid_to_pes_packet)
      {
        callback(std::move(v.second));
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

        ts_packet.pes_offset += sizeof(uint32_t);

        uint8_t stream_id = (start_code & 0xff);

        auto num_to_hex = [](uint32_t num, bool is_0x) {
          std::stringstream ss;
          ss << std::hex << (is_0x ? "0x" : "") << num;
          return ss.str();
        };

        BOOST_LOG_TRIVIAL(trace) << "Maybe PES start code: " << num_to_hex(start_code, false);

        uint16_t pes_length = boost::endian::big_to_native(
            *reinterpret_cast<const uint16_t *>(ts_packet.data.data() + ts_packet.pes_offset));

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

        if (((start_code >> 8) & 0x01) != 0x01)
        {
          BOOST_LOG_TRIVIAL(trace) << "Not a PES packet, skipping";
          return;
        }

        it = _pid_to_pes_packet.find(ts_packet.pid);

        if (it != _pid_to_pes_packet.end())
        {
          flush(it, callback);
        }

        bool inserted;
        std::tie(it, inserted) = _pid_to_pes_packet.insert(std::make_pair(ts_packet.pid,
            pes_packet_t{ts_packet.pid, (!pes_length ? MAX_PES_PAYLOAD_SIZE : pes_length)}));
      }
      else
      {
        auto it(_pid_to_pes_packet.find(ts_packet.pid));

        if (it == _pid_to_pes_packet.end())
        {
          BOOST_LOG_TRIVIAL(trace) << "PUSI is 0, PID is not in PES map, skipping";
          return;
        }
      }

      const auto ts_pes_length = ts_packet.data.size() - ts_packet.pes_offset;
      BOOST_LOG_TRIVIAL(trace) << "TS PES length: " << ts_pes_length;

      memcpy(&it->second.data.buffer[0] + it->second.data.length,
          ts_packet.data.data() + ts_packet.pes_offset, ts_pes_length);
      it->second.data.length += ts_pes_length;
    }
  };

} // namespace detail
} // namespace mpegts
