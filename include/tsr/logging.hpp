#pragma once

#include "fmt/core.h"
#include <string>

namespace tsr {

/** Logging code copied and minimally adapted from tin-terrain 
    https://github.com/heremaps/tin-terrain/blob/master/include/logging.h
*/

/* BEGIN COPIED CODE */

enum class LogLevel {
  TRACE = 0,
  DEBUG = 1,
  INFO = 2,
  WARN = 3,
  ERROR = 4,
  FATAL = 5
};

enum class LogStream { NONE, STDOUT, STDERR };

void log_message(LogLevel logLevel, const char *filename, const int line,
                 const std::string &message);

} // namespace tsr

// Compiler-Disable Trace logs when in production
#ifdef TSR_DEBUG
#define TSR_LOG_TRACE(fmtString, ...)                                          \
  ::tsr::log_message(::tsr::LogLevel::TRACE, __FILE__, __LINE__,               \
                     ::fmt::format(fmtString, ##__VA_ARGS__))
#else
#define TSR_LOG_TRACE(fmtString, ...)
#endif

#define TSR_LOG_DEBUG(fmtString, ...)                                          \
  ::tsr::log_message(::tsr::LogLevel::DEBUG, __FILE__, __LINE__,               \
                     ::fmt::format(fmtString, ##__VA_ARGS__))
#define TSR_LOG_INFO(fmtString, ...)                                           \
  ::tsr::log_message(::tsr::LogLevel::INFO, __FILE__, __LINE__,                \
                     ::fmt::format(fmtString, ##__VA_ARGS__))
#define TSR_LOG_WARN(fmtString, ...)                                           \
  ::tsr::log_message(::tsr::LogLevel::WARN, __FILE__, __LINE__,                \
                     ::fmt::format(fmtString, ##__VA_ARGS__))
#define TSR_LOG_ERROR(fmtString, ...)                                          \
  ::tsr::log_message(::tsr::LogLevel::ERROR, __FILE__, __LINE__,               \
                     ::fmt::format(fmtString, ##__VA_ARGS__))
#define TSR_LOG_FATAL(fmtString, ...)                                          \
  ::tsr::log_message(::tsr::LogLevel::FATAL, __FILE__, __LINE__,               \
                     ::fmt::format(fmtString, ##__VA_ARGS__))

/* END COPIED CODE */