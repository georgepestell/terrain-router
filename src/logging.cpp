#include "tsr/logging.hpp"

#include <atomic>
#include <cstdio>

namespace tsr {

#if (defined(DEBUG) || defined(TSR_DEBUG))
constexpr LogLevel GLOBAL_DEFAULT_LOG_LEVEL = LogLevel::DEBUG;
#else
constexpr LogLevel GLOBAL_DEFAULT_LOG_LEVEL = LogLevel::INFO;
#endif

static std::atomic<LogLevel> g_global_log_level = {GLOBAL_DEFAULT_LOG_LEVEL};
static std::atomic<LogStream> g_global_log_stream = {LogStream::STDERR};

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

void log_message(LogLevel logLevel, const char *filename, const int line,
                 const std::string &message) {

  FILE *log_stream = stdout;

  if (!message.empty() && log_stream) {

    if (logLevel == LogLevel::INFO) {
      fmt::print(log_stream, "{:s}\n", message.c_str());
    } else {
      const char *filename_last_component = strrchr(filename, '/');
      if (filename_last_component) {
        filename = filename_last_component + 1;
      }
      fmt::print(log_stream, "{:s} {:s}:{:d} {:s}\n", loglevel_to_str(logLevel),
                 filename, line, message.c_str());
    }
  }

  std::fflush(log_stream);
}

} // namespace tsr