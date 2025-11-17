#ifndef LOGGER_HPP
#define LOGGER_HPP
#include <string>
#include <fstream>
#include <mutex>
#include <chrono>
#include <ctime>

namespace ofs
{

enum class LogLevel
{
    debug, // Renamed to snake_case
    info,  // Renamed to snake_case
    warn,  // Renamed to snake_case
    error, // Renamed to snake_case
    fatal  // Renamed to snake_case
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

    // Internal method to handle the actual write operation
    void write_internal(LogLevel level,
                        const std::string& module,
                        int code,
                        const std::string& msg,
                        const std::string& src_file,
                        int line);

public:
    static Logger& get_instance();

    void set_log_file(const std::string& path);
    void set_max_file_size(size_t bytes);

    // Public API, redirects to internal implementation
    void log(LogLevel level,
             const std::string& module,
             int code,
             const std::string& msg,
             const std::string& src_file,
             int line)
    {
        write_internal(level, module, code, msg, src_file, line);
    }

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
};

}

#endif // LOGGER_HPP