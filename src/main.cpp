#include "parser.h"
#include "options.h"

#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>
#include <boost/program_options.hpp>

#include <iostream>

namespace po = boost::program_options;

int main(int argc, char *argv[])
{
  try
  {
    mpegts::options options(argc, argv);
    options.print();

  }
  catch (const std::exception &e)
  {
    std::cerr << e.what() << std::endl;
  }
  return 0;
}
