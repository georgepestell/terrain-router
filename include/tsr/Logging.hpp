#pragma once

/** Include fmt core library
 * Clangd unused_include ignored because the include is required,
 * but used in #DEFINE TSR_LOG functions, which are injected into the source
 * code when used.
 */
#include "fmt/core.h" // IWYU pragma: keep

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

void log_message(LogLevel logLevel, std::string filename, const int line,
                 const std::string &message);

void log_set_global_logstream(LogStream logstream);

void log_set_global_loglevel(LogLevel level);

LogStream log_get_global_logstream();
LogLevel log_get_global_loglevel();

} // namespace tsr

/**
 * Remove TSR_LOG_TRACE commands in source code for Release builds.
 * Improves performance, but allows TRACE logging for debugging.
 */
#ifdef TSR_DEBUG
#define TSR_LOG_TRACE(fmtString, ...)                                          \
  ::tsr::log_message(::tsr::LogLevel::TRACE, __FILE__, __LINE__,               \
                     ::fmt::format(fmtString, ##__VA_ARGS__))
#else
#define TSR_LOG_TRACE(fmtString, ...)
#endif

/**
 * Defines the normal logging functions which are injected into the source
 * code by the compiler
 */
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