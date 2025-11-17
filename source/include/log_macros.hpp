#ifndef LOG_MACROS_HPP
#define LOG_MACROS_HPP

#include "logger.hpp"

#ifndef OFS_LOG_ENABLED
#define OFS_LOG_ENABLED 1
#endif

#if OFS_LOG_ENABLED

// Macro now takes all key logging arguments
#define LOG_INTERNAL(level, module_name, error_code, msg) \
    ofs::Logger::get_instance().log(level, module_name, error_code, msg, __FILE__, __LINE__)

// Updated macro calls to use the new LogLevel names (snake_case) and the full API
#define LOG_FATAL(module, code, msg)   LOG_INTERNAL(ofs::LogLevel::fatal,   module, code, msg)
#define LOG_ERROR(module, code, msg)   LOG_INTERNAL(ofs::LogLevel::error,   module, code, msg)
#define LOG_WARN(module, code, msg)    LOG_INTERNAL(ofs::LogLevel::warn,    module, code, msg)
#define LOG_INFO(module, code, msg)    LOG_INTERNAL(ofs::LogLevel::info,    module, code, msg)
#define LOG_DEBUG(module, code, msg)   LOG_INTERNAL(ofs::LogLevel::debug,   module, code, msg)

#else

// Placeholder with the required arguments for consistency
#define LOG_FATAL(module, code, msg)   ((void)0)
#define LOG_ERROR(module, code, msg)   ((void)0)
#define LOG_WARN(module, code, msg)    ((void)0)
#define LOG_INFO(module, code, msg)    ((void)0)
#define LOG_DEBUG(module, code, msg)   ((void)0)

#endif // OFS_LOG_ENABLED

#endif // LOG_MACROS_HPP