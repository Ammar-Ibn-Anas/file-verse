#include "../../include/logger.hpp" 
#include <iostream>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <stdexcept>
#include <sys/stat.h>

#if defined(_WIN32)
    #include <windows.h>
    #include <processthreadsapi.h>
#else
    #include <unistd.h>
    #include <limits.h>
#endif

using namespace ofs;

Logger::Logger() 
{
    log_file_path_ = "./logs/ofs.log";
    
    initialize_app_identifier();
    
    std::filesystem::create_directories(std::filesystem::path(log_file_path_).parent_path());
    
    file_stream_.open(log_file_path_, std::ios::app);
    if (!file_stream_.is_open())
    {
        std::cerr << "fatal: logger failed to open log file: " << log_file_path_ << std::endl;
    }
}

Logger::~Logger() 
{
    if (file_stream_.is_open()) {
        file_stream_.close();
    }
}

Logger& Logger::get_instance() 
{
    static Logger instance;
    return instance;
}

void Logger::initialize_app_identifier()
{
#if defined(_WIN32)
    process_id_ = GetCurrentProcessId();
    
    char path[MAX_PATH];
    DWORD length = GetModuleFileNameA(NULL, path, MAX_PATH);
    if (length > 0) {
        std::filesystem::path exe_path(path);
        app_identifier_ = exe_path.filename().string();
    } else {
        app_identifier_ = "unknown_app";
    }
#else
    process_id_ = getpid();
    
    char path[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (len != -1) {
        path[len] = '\0';
        std::filesystem::path exe_path(path);
        app_identifier_ = exe_path.filename().string();
    } else {
        app_identifier_ = "unknown_app";
    }
#endif
}

void Logger::set_app_name(const std::string& name)
{
    std::lock_guard<std::mutex> lock(mtx_);
    app_identifier_ = name;
}

void Logger::set_log_file(const std::string& path) 
{
    std::lock_guard<std::mutex> lock(mtx_);
    if (file_stream_.is_open()) {
        file_stream_.close();
    }
    log_file_path_ = path;
    std::filesystem::create_directories(std::filesystem::path(path).parent_path());
    file_stream_.open(log_file_path_, std::ios::app);
    if (!file_stream_.is_open()) {
        std::cerr << "CRITICAL: Failed to open new log file at " << log_file_path_ << std::endl;
    }
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

std::string Logger::get_timestamp_utc() 
{
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm utc {};

#if defined(_WIN32)
    gmtime_s(&utc, &time);
#else
    if (gmtime_r(&time, &utc) == nullptr) {}
#endif
    
    std::ostringstream oss;
    oss << std::put_time(&utc, "%Y-%m-%dT%H:%M:%SZ");
    return oss.str();
}

bool Logger::file_was_rotated() 
{
    if (!std::filesystem::exists(log_file_path_)) {
        return true;
    }
    
#if defined(_WIN32)
    try {
        size_t current_size = std::filesystem::file_size(log_file_path_);
        auto current_pos = file_stream_.tellp();
        if (current_pos > 0 && current_size < (size_t)current_pos / 2) {
            return true;
        }
    } catch (...) {
        return true;
    }
#else
    struct stat current_stat;
    if (stat(log_file_path_.c_str(), &current_stat) != 0) {
        return true;
    }
    
    try {
        size_t disk_size = std::filesystem::file_size(log_file_path_);
        auto stream_pos = file_stream_.tellp();
        
        if (stream_pos > 0 && disk_size < (std::streampos)stream_pos / 2) {
            return true;
        }
    } catch (...) {
        return true;
    }
#endif
    
    return false;
}

void Logger::rotate_if_needed() 
{
    if (!file_stream_.is_open()) {
        return; 
    }

    if (!std::filesystem::exists(log_file_path_)) return;
    size_t current_size = 0;
    try {
        current_size = std::filesystem::file_size(log_file_path_);
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "LOG ERROR: Failed to check file size for rotation: " << e.what() << std::endl;
        return;
    }

    if (current_size < MAX_FILE_SIZE_BYTES) return;

    try {
        std::string new_path = log_file_path_ + "." + get_timestamp_utc() + ".log";
        
        file_stream_.close(); 
        
        std::error_code ec;
        std::filesystem::rename(log_file_path_, new_path, ec);
        
        if (ec) {
            std::cerr << "LOG WARN: Rename failed (code: " << ec.value() << ", msg: " << ec.message() << "). File possibly locked by another process." << std::endl;
        }

        file_stream_.open(log_file_path_, std::ios::app);
    } catch (const std::exception& e) {
        std::cerr << "LOG ERROR: Catastrophic rotation failure: " << e.what() << std::endl;
    }
}

void Logger::write_internal(LogLevel level,
                            const std::string& module,
                            int code,
                            const std::string& msg,
                            const std::string& src_file,
                            int line) 
{
    std::lock_guard<std::mutex> lock(mtx_); 
    
    std::string level_str = level_to_string(level);
    
    if (file_stream_.is_open() && file_was_rotated()) {
        file_stream_.close();
    }
    
    if (!file_stream_.is_open()) {
        file_stream_.open(log_file_path_, std::ios::app);
    }

    if (!file_stream_.is_open()) {
        std::string fallback_msg = "[" + level_str + "] " + msg + " (File stream permanently closed)";
        if (level == LogLevel::error || level == LogLevel::fatal) {
            std::cerr << fallback_msg << std::endl;
        } else {
            std::cout << fallback_msg << std::endl;
        }
        
        if (level == LogLevel::fatal) {
            std::exit(code);
        }
        return;
    }
    
    rotate_if_needed(); 

    std::string timestamp = get_timestamp_utc();

    if (file_stream_.is_open()) {
        file_stream_ << timestamp
                   << " app=\"" << app_identifier_ << "\""
                   << " pid=" << process_id_
                   << " level=" << level_str
                   << " module=" << module
                   << " code=" << code
                   << " msg=\"" << msg << "\""
                   << " file=\"" << src_file << "\""
                   << " line=" << line
                   << "\n";
        file_stream_.flush();
    }

    std::string console_prefix = "[" + app_identifier_ + ":" + std::to_string(process_id_) + "][" + level_str + "] ";
    if (level == LogLevel::error || level == LogLevel::fatal) {
        std::cerr << console_prefix << msg << std::endl;
    } else {
        std::cout << console_prefix << msg << std::endl;
    }

    if (level == LogLevel::fatal)
    {
        if (file_stream_.is_open()) {
            file_stream_.close();
        }
        std::exit(code);
    }
}