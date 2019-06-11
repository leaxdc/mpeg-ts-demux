#pragma once

#include <iomanip>
#include <sstream>
#include <string>

namespace utils
{
template <typename T>
std::string num_to_hex(T num, bool is_0x)
{
  std::stringstream ss;
  ss << std::hex << (is_0x ? "0x" : "") << num;

  return ss.str();
}
} // namespace utils
