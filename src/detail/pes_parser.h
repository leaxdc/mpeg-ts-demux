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

#pragma once

#include "mpegts.h"
#include "mpegts_detail.h"

#include <unordered_map>

namespace mpegts
{
namespace detail
{
  // pes packets builder
  class pes_parser
  {
  public:
    explicit pes_parser(packet_received_callback_t callback);

    void feed_ts_packet(ts_packet_t ts_packet);
    void flush();

  private:
    using pid_to_pes_packet_map_t = std::unordered_map<uint16_t, pes_packet_impl_t>;

    packet_received_callback_t _callback;
    pid_to_pes_packet_map_t _pid_to_pes_packet;
    uint64_t _pes_packet_num = 0;

    void handle_ready_pes_packet(pid_to_pes_packet_map_t::value_type &v);
  };
} // namespace detail
} // namespace mpegts
