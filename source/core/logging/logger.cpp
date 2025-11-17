#include "../../include/logger.hpp" // Changed path for new structure
#include <iostream>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <stdexcept>

using namespace ofs;

Logger::Logger()
{
    log_file_path_ = "./logs/ofs.log";
    max_file_size_bytes_ = 1024 * 1024;
    std::filesystem::create_directories ( "./logs" );
    file_stream_.open ( log_file_path_, std::ios::app );
}

Logger& Logger::get_instance()
{
    static Logger instance;
    return instance;
}

void Logger::set_log_file(const std::string& path)
{
    std::lock_guard<std::mutex> lock ( mtx_ );
    log_file_path_ = path;
    if (file_stream_.is_open())
    {
        file_stream_.close();
    }
    std::filesystem::create_directories ( std::filesystem::path ( path ).parent_path() );
    file_stream_.open ( log_file_path_, std::ios::app );
}

void Logger::set_max_file_size(size_t bytes)
{
    std::lock_guard<std::mutex> lock ( mtx_ );
    max_file_size_bytes_ = bytes;
}

std::string Logger::get_timestamp_utc()
{
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t ( now );
    std::tm utc {};
#if defined(_WIN32)
    gmtime_s ( &utc, &time );
#else
    gmtime_r ( &time, &utc );
#endif
    std::ostringstream oss;
    oss << std::put_time ( &utc, "%Y-%m-%dT%H:%M:%SZ" );
    return oss.str();
}

std::string Logger::level_to_string(LogLevel level)
{
    switch (level)
    {
        case LogLevel::debug: return "DEBUG";
        case LogLevel::info:  return "INFO";
        case LogLevel::warn:  return "WARN";
        case LogLevel::error: return "ERROR";
        case LogLevel::fatal: return "FATAL";
    }
    return "UNKNOWN";
}

void Logger::rotate_if_needed()
{
    if ( !std::filesystem::exists ( log_file_path_ ) )
    {
        return;
    }
    std::error_code ec;
    auto size = std::filesystem::file_size ( log_file_path_, ec );
    if (ec)
    {
        return;
    }
    if (size < max_file_size_bytes_)
    {
        return;
    }

    std::string new_path = log_file_path_ + "." + get_timestamp_utc();
    std::filesystem::rename ( log_file_path_, new_path );
    if (file_stream_.is_open())
    {
        file_stream_.close();
    }
    file_stream_.open ( log_file_path_, std::ios::app );
}

void Logger::write_internal(LogLevel level,
                            const std::string& module,
                            int code,
                            const std::string& msg,
                            const std::string& src_file,
                            int line)
{
    std::lock_guard<std::mutex> lock ( mtx_ );
    rotate_if_needed();
    std::string timestamp = get_timestamp_utc();

    if ( !file_stream_.is_open() )
    {
        std::filesystem::create_directories ( std::filesystem::path ( log_file_path_ ).parent_path() );
        file_stream_.open ( log_file_path_, std::ios::app );
    }

    file_stream_ << timestamp
               << " level=" << level_to_string ( level )
               << " module=" << module
               << " code=" << code
               << " msg=\"" << msg << "\""
               << " file=\"" << src_file << "\""
               << " line=" << line
               << "\n";
    file_stream_.flush();

    std::cout << level_to_string ( level ) << ": " << msg << std::endl;

    if (level == LogLevel::fatal)
    {
        throw std::runtime_error ( std::string("Fatal error: ") + msg );
    }
}