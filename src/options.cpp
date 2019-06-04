#include "options.h"

#include <boost/log/trivial.hpp>
#include <boost/program_options.hpp>
#include <iostream>

namespace mpegts
{
namespace po = boost::program_options;
namespace log = boost::log;

bool options::parse(int argc, char *argv[])
{
  po::options_description desc("Options");

  using log::trivial::severity_level;

  desc.add_options()("help", "produce help message")(
      "output_dir,o", po::value(&_output_dir), "output directory")("log_level,l",
      po::value<severity_level>(&_log_level)->default_value(severity_level::info),
      "log level [trace, debug, info, warning, error, fatal]");

  auto print_help = [&]() {
    std::cout << "Usage: " << argv[0] << " [options] <input_file_name>"
              << "\n"
              << desc;
  };

  po::variables_map vm;
  auto parsed = po::command_line_parser(argc, argv).options(desc).run();
  po::store(parsed, vm);
  po::notify(vm);
  if (vm.count("help"))
  {
    print_help();
    return false;
  }

  po::options_description hidden_desc("Hidden options");
  hidden_desc.add_options()("input", po::value(&_input_file)->required());

  // Desc for parsing
  po::options_description parsing_desc;
  parsing_desc.add(desc).add(hidden_desc);

  po::positional_options_description pos;
  pos.add("input", -1);

  parsed = po::command_line_parser(argc, argv).options(parsing_desc).positional(pos).run();

  po::store(parsed, vm);
  try
  {
    po::notify(vm);
  }
  catch (const po::error &)
  {
    std::cerr << "Error: input is required"
              << "\n";
    print_help();
    return false;
  }

  return true;
}

const std::string &options::get_input_file_name() const
{
  return _input_file;
}
const std::string &options::get_oputput_directory() const
{
  return _output_dir;
}

boost::log::trivial::severity_level options::get_log_severity_level() const
{
  return _log_level;
}

void options::print() const
{
  BOOST_LOG_TRIVIAL(info) << "Input file name: " << _input_file;
  BOOST_LOG_TRIVIAL(info) << "Output directory: " << _output_dir;
  BOOST_LOG_TRIVIAL(info) << "Log level: " << _log_level;
}

} // namespace mpegts
