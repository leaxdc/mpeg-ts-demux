#pragma once

#include "logger.h"
#include "mpegts.h"
#include "mpegts_detail.h"


#include <boost/endian/conversion.hpp>
#include <boost/log/trivial.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <unordered_map>
#include <sstream>

namespace mpegts
{
namespace detail
{
  const size_t MAX_PES_PACKET_DATA_SIZE = 65536;

  struct pes_packet_details_t : pes_packet_t
  {
    uint8_t *out_ptr;
    pes_packet_details_t(uint16_t pid, size_t data_size)
        : pes_packet_t(pid, data_size), out_ptr(&data[0])
    {
    }
  };

  using pes_packet_details_uptr = std::unique_ptr<pes_packet_details_t>;

  class pes_parser
  {
  private:
    using pid_to_pes_packet_map_t = std::unordered_map<uint16_t, pes_packet_details_uptr>;
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
      for (auto &v: _pid_to_pes_packet)
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
            *reinterpret_cast<const uint32_t *>(ts_packet.pes_offset_ptr));

        std::stringstream ss;
        ss << std::hex << start_code;

        BOOST_LOG_TRIVIAL(trace) << "Maybe PES start code: " << ss.str();

        uint16_t pes_length = boost::endian::big_to_native(
            *reinterpret_cast<const uint16_t *>(ts_packet.pes_offset_ptr + sizeof(uint32_t)));

        if (logger::current_severity_level.load() == boost::log::trivial::severity_level::trace)
        {
          boost::property_tree::ptree pt;

          pt.put("pes_preparsed_header.start_code_hex", ss.str());
          pt.put("pes_preparsed_header.pes_length", pes_length);

          std::stringstream ss;
          boost::property_tree::json_parser::write_json(ss, pt);
          BOOST_LOG_TRIVIAL(trace) << ss.str();
        }

        if (!(start_code & 0x0100))
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
            std::make_unique<pes_packet_details_t>(ts_packet.pid,
                pes_length + sizeof(start_code) + sizeof(pes_length))));
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

      auto ts_packet_used_length = (ts_packet.pes_offset_ptr - ts_packet.data.data());
      auto ts_pes_length = ts_packet.data.size() - ts_packet_used_length;

      memcpy(it->second->out_ptr, ts_packet.pes_offset_ptr, ts_pes_length);
      it->second->out_ptr += ts_pes_length;
    }
  };

} // namespace detail
} // namespace mpegts
