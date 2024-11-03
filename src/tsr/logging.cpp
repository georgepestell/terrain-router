#include "tsr/logging.hpp"

#include <atomic>
#include <cstdio>

namespace tsr {

/** Logging code copied and minimally adapted from tin-terrain 
    https://github.com/heremaps/tin-terrain/blob/master/src/logging.cpp
*/

/* BEGIN COPIED CODE */

#if (defined(DEBUG) || defined(TSR_DEBUG))
constexpr LogLevel GLOBAL_DEFAULT_LOG_LEVEL = LogLevel::DEBUG;
#else
constexpr LogLevel GLOBAL_DEFAULT_LOG_LEVEL = LogLevel::INFO;
#endif

static std::atomic<LogLevel> g_global_log_level = {GLOBAL_DEFAULT_LOG_LEVEL};
static std::atomic<LogStream> g_global_log_stream = {LogStream::STDERR};

void log_set_global_logstream(LogStream logstream) {
  g_global_log_stream = logstream;
}

void log_set_global_loglevel(LogLevel level) {
  g_global_log_level = level;
}

LogStream log_get_global_logstream() {
  return g_global_log_stream;
}

LogLevel log_get_global_loglevel() {
  return g_global_log_level;
}

static const char *loglevel_to_str(const LogLevel level) {
  switch (level) {
  case LogLevel::TRACE:
    return "TRACE";
  case LogLevel::DEBUG:
    return "DEBUG";
  case LogLevel::INFO:
    return "INFO";
  case LogLevel::WARN:
    return "WARN";
  case LogLevel::ERROR:
    return "ERROR";
  case LogLevel::FATAL:
    return "FATAL";
  default:
    return "";
  }
}

void log_message(LogLevel level, const char *filename, const int line,
                 const std::string &message) {

  const LogLevel global_loglevel = g_global_log_level;
  const LogStream global_logstream = g_global_log_stream;

  FILE *log_stream = global_logstream == LogStream::STDOUT ? stdout : (global_logstream == LogStream::STDERR ? stderr : nullptr);


  if (global_loglevel <= level && !message.empty() && log_stream) {
    if (level == LogLevel::INFO) {
      fmt::print(log_stream, "{:s}\n", message.c_str());
    } else {

      const char *filename_last_component = strrchr(filename, '/');
      if (filename_last_component) {
        filename = filename_last_component + 1;
      }
      fmt::print(log_stream, "{:s} {:s}:{:d} {:s}\n", loglevel_to_str(level),
                 filename, line, message.c_str());

    }
    std::fflush(log_stream);
  }
}

/* END COPIED CODE */

} // namespace tsr