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

#include <array>
#include <optional>

namespace mpegts
{
namespace detail
{
  // https://en.wikipedia.org/wiki/MPEG_transport_stream#Important_elements_of_a_transport_stream
  constexpr const uint8_t TS_PACKET_SIZE = 188;
  using ts_packet_data_t = std::array<uint8_t, TS_PACKET_SIZE - sizeof(uint32_t)>;

  struct ts_packet_t
  {
    uint32_t header;

    ts_packet_data_t data;

    uint16_t sync_byte;
    bool transport_error;
    int8_t continuity_cnt;
    bool pusi;
    uint16_t pid;
    uint8_t adaptation_field_ctl;

    std::optional<uint8_t> pes_offset;
  };

  using ts_packet_opt = std::optional<ts_packet_t>;

  // https://ffmpeg.org/doxygen/3.2/mpegts_8c_source.html
  const size_t MAX_PES_PAYLOAD_SIZE = 1024 * 200;

  struct pes_packet_impl_t
  {
    uint32_t start_code;
    uint16_t ts_packet_pid;
    uint16_t stream_id;
    size_t max_length;

    std::array<uint8_t, MAX_PES_PAYLOAD_SIZE> data;
    size_t cur_length;

    uint16_t payload_offset;
    uint16_t payload_length;
  };

} // namespace detail
} // namespace mpegts
