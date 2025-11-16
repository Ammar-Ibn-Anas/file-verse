#ifndef CONFIG_TYPES_HPP
#define CONFIG_TYPES_HPP

#include <cstdint>
#include <string>

namespace ofs::config
{
    struct Config
    {
        uint64_t total_size = 104857600ULL;      
        uint32_t header_size = 512u;
        uint32_t block_size = 4096u;             
        uint32_t max_files = 1000u;
        uint32_t max_filename_length = 10u;      

        uint32_t max_users = 50u;
        std::string admin_username = "admin";
        std::string admin_password = "admin123";
        bool require_auth = true;

        // Server
        uint16_t port = 8080u;
        uint16_t max_connections = 20u;
        uint16_t queue_timeout = 30u;            // seconds

        // default ctor uses the inline defaults above
    };
}

#endif // CONFIG_TYPES_HPP