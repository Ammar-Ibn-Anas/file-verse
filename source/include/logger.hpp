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
    debug, 
    info,  
    warn,  
    error, 
    fatal  
};

class Logger
{
private:
    std::ofstream file_stream_;
    std::mutex mtx_;
    std::string log_file_path_;
    static constexpr size_t MAX_FILE_SIZE_BYTES = 10 * 1024 * 1024;
    std::string app_identifier_;
    int process_id_;

    Logger();
    void rotate_if_needed();
    bool file_was_rotated();
    std::string get_timestamp_utc();
    std::string level_to_string(LogLevel level);
    void initialize_app_identifier();

    void write_internal(LogLevel level,
                        const std::string& module,
                        int code,
                        const std::string& msg,
                        const std::string& src_file,
                        int line);

public:

    ~Logger();

    static Logger& get_instance();

    void set_log_file(const std::string& path);
    void set_app_name(const std::string& name);

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

} // namespace ofs

#endif // LOGGER_HPP