#pragma once

#include <string>
#include "mpegts.h"

namespace mpegts
{
class parser
{
public:
  explicit parser(const std::string &file_name);
  void parse(data_received_callback callback);
};
} // namespace mpegts
