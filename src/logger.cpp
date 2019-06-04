#include "logger.h"

#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/log/attributes/named_scope.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>
namespace logger
{

namespace log = boost::log;

void init(log::trivial::severity_level severity, const std::string& file_name)
{
  log::add_common_attributes();
  log::core::get()->add_global_attribute("Scope", log::attributes::named_scope());
  log::core::get()->set_filter(log::trivial::severity >= severity);

  const auto fmt_timestamp = log::expressions::format_date_time<boost::posix_time::ptime>(
      "TimeStamp", "%Y-%m-%dT%H:%M:%S.%f");
  const auto fmt_thread_id =
      log::expressions::attr<log::attributes::current_thread_id::value_type>("ThreadID");
  const auto fmt_severity = log::expressions::attr<log::trivial::severity_level>("Severity");
  const auto fmt_scope = log::expressions::format_named_scope("Scope", log::keywords::format = "%n");

  const auto log_fmt = log::expressions::format("[%1%] (%2%) [%3%] %4% : %5%") % fmt_timestamp %
      fmt_thread_id % fmt_severity % fmt_scope % log::expressions::smessage;

  auto consoleSink = log::add_console_log(std::clog);
  consoleSink->set_formatter(log_fmt);

  auto fsSink = log::add_file_log(
      log::keywords::file_name = file_name,
      log::keywords::open_mode = std::ios_base::out,
      log::keywords::auto_flush = true);

  fsSink->set_formatter(log_fmt);

  // fsSink->locked_backend()->set_file_collector(log::sinks::file::make_collector(
  //     log::keywords::target = "./",
  //     log::keywords::max_files = 100));
}

} //
