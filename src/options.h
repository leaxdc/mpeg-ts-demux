#pragma once

#include <string>

namespace mpegts
{
class options
{
public:
  bool parse(int argc, char *argv[]);

  const std::string &get_input_file_name() const;
  const std::string &get_oputput_directory() const;
  bool is_log_enabled()const;

  void print()const;

private:
  std::string _input_file;
  std::string _output_dir;
  bool _log_enabled;
};
} // namespace mpegts
