#pragma once

#include <boost/log/trivial.hpp>

namespace logger
{
void init(boost::log::trivial::severity_level severity, const std::string &file_name);
} // namespace logger
