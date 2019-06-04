#include "parser.h"
#include "logger.h"
#include "options.h"

#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>
#include <boost/program_options.hpp>

#include <iostream>

namespace po = boost::program_options;

namespace
{
  const std::string log_file_name = "mpeg-ts-demux_%Y%m%d_%H%M%S.log";
} //

int main(int argc, char *argv[])
{
  try
  {
    mpegts::options options;

    if (!options.parse(argc, argv))
    {
      return 1;
    }

    logger::init(options.get_log_severity_level(), log_file_name);
    options.print();
  }
  catch (const std::exception &e)
  {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  return 0;
}
