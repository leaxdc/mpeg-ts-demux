#pragma once

#include "mpegts_detail.h"

namespace mpegts
{
namespace detail
{
  namespace log_utils
  {
    void log_ts_packet(const ts_packet_t &ts_packet, uint64_t ts_packet_num);
    void log_pes_packet(const pes_packet_impl_t &pes_packet, size_t pes_packet_num);
  } // namespace log_utils
} // namespace detail

} // namespace mpegts
