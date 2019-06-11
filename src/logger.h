#pragma once

#include <atomic>
#include <boost/log/trivial.hpp>

namespace logger
{
void init(boost::log::trivial::severity_level severity_level, const std::string &file_name);
extern std::atomic<boost::log::trivial::severity_level> current_severity_level;
extern std::atomic_bool log_ts_packets;
extern std::atomic_bool log_pes_packets;
} // namespace logger
