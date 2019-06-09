#pragma once

#include <boost/log/trivial.hpp>
#include <atomic>

namespace logger
{
void init(boost::log::trivial::severity_level severity_level, const std::string &file_name);
extern std::atomic<boost::log::trivial::severity_level> current_severity_level;
} // namespace logger
