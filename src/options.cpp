#include "options.h"


// #include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>

#include <boost/program_options.hpp>
#include <iostream>

namespace po = boost::program_options;

namespace mpegts
{

bool options::parse(int argc, char *argv[])
{
  po::options_description desc("Options");
  desc.add_options()("help", "produce help message")
  		("output_dir,o", po::value(&_output_dir), "output directory")
  		("enable_log,l", po::bool_switch(&_log_enabled), "enable log file");

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

bool options::is_log_enabled()const
{
	return _log_enabled;
}

void options::print()const
{
	std::cout << "Input file name: " << _input_file;
	std::cout << "Output directory: " << _output_dir;
	std::cout << "Log enabled: " << _log_enabled;
}

} //
