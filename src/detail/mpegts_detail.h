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

#pragma once

#include <array>

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
    uint16_t pid;
    bool pusi;
    size_t pes_offset;

    ts_packet_t()
    {
      reset();
    }

    void reset()
    {
      header = 0;
      pid = 0;
      pusi = false;
      pes_offset = 0;
    }
  };
} // namespace detail
} // namespace mpegts
