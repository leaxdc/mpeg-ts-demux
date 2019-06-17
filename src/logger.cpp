/*

Copyright 2019 Peter Asanov

Permission is hereby granted, free of charge,
to any person obtaining a copy of this software and associated documentation files( the "Software"),
to deal in the Software without restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and / or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#include "logger.h"

#include <boost/log/expressions.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>

namespace logger
{
std::atomic<boost::log::trivial::severity_level> current_severity_level;
std::atomic_bool log_ts_packets;
std::atomic_bool log_pes_packets;

namespace log = boost::log;

void init(log::trivial::severity_level severity_level, const std::string &file_name)
{
  current_severity_level = severity_level;

  log::add_common_attributes();
  log::core::get()->add_global_attribute("Scope", log::attributes::named_scope());
  log::core::get()->set_filter(log::trivial::severity >= severity_level);

  const auto fmt_timestamp = log::expressions::format_date_time<boost::posix_time::ptime>(
      "TimeStamp", "%Y-%m-%dT%H:%M:%S.%f");
  const auto fmt_thread_id =
      log::expressions::attr<log::attributes::current_thread_id::value_type>("ThreadID");
  const auto fmt_severity = log::expressions::attr<log::trivial::severity_level>("Severity");
  const auto fmt_scope =
      log::expressions::format_named_scope("Scope", log::keywords::format = "%n");

  const auto log_fmt = log::expressions::format("[%1%] (%2%) [%3%] %4% : %5%") % fmt_timestamp %
      fmt_thread_id % fmt_severity % fmt_scope % log::expressions::smessage;

  auto consoleSink = log::add_console_log(std::clog);
  consoleSink->set_formatter(log_fmt);

  auto fsSink = log::add_file_log(log::keywords::file_name = file_name,
      log::keywords::open_mode = std::ios_base::out, log::keywords::auto_flush = true);

  fsSink->set_formatter(log_fmt);
}

} // namespace logger
