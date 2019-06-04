#pragma once

#include <boost/log/trivial.hpp>
#include <string>

namespace mpegts
{
class options
{
public:
  bool parse(int argc, char *argv[]);

  const std::string &get_input_file_name() const;
  const std::string &get_oputput_directory() const;
  boost::log::trivial::severity_level get_log_severity_level() const;

  void print() const;

private:
  std::string _input_file;
  std::string _output_dir;
  boost::log::trivial::severity_level _log_level;
};
} // namespace mpegts
