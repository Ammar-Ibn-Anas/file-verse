#ifndef LOG_MACROS_HPP
#define LOG_MACROS_HPP

#include "logger.hpp" 

// The core macro that calls the actual log function, passing __FILE__ and __LINE__
#define OFS_LOG(level, module, code, msg) \
    ofs::Logger::get_instance().log(level, module, code, msg, __FILE__, __LINE__)

#define LOG_FATAL(module, code, msg)  OFS_LOG(ofs::LogLevel::fatal, module, code, msg)
#define LOG_ERROR(module, code, msg)  OFS_LOG(ofs::LogLevel::error, module, code, msg)
#define LOG_WARN(module, code, msg)   OFS_LOG(ofs::LogLevel::warn, module, code, msg)
#define LOG_INFO(module, code, msg)   OFS_LOG(ofs::LogLevel::info, module, code, msg)
#define LOG_DEBUG(module, code, msg)  OFS_LOG(ofs::LogLevel::debug, module, code, msg)

#endif // LOG_MACROS_HPP