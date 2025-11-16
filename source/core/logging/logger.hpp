#pragma once
#include <string>
#include <fstream>
#include <mutex>
#include <chrono>
#include <ctime>

namespace ofs
{

enum class LogLevel
{
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL
};

class Logger
{
private:
    std::ofstream file_stream_;
    std::mutex mtx_;
    std::string log_file_path_;
    size_t max_file_size_bytes_;

    Logger();
    void rotate_if_needed();
    std::string get_timestamp_utc();
    std::string level_to_string(LogLevel level);

public:
    static Logger& get_instance();

    void set_log_file(const std::string& path);
    void set_max_file_size(size_t bytes);

    void log(LogLevel level,
             const std::string& module,
             int code,
             const std::string& msg,
             const std::string& src_file,
             int line);

    Logger(const Logger&) = delete;
    Logger& operator= (const Logger&) = delete;
};

}