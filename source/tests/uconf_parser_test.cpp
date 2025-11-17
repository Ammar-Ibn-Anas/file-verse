#include "../include/uconf_parser.hpp"
#include "../include/logger.hpp" // Keep logger include for general use if needed

#include <iostream>
#include <string>

// Removed all duplicated parser function implementations (trim, write_uconf, load_uconf_or_create_default, etc.)

int main(int argc, char** argv)
{
    std::string path = "compiled/default.uconf";
    if (argc > 1)
    {
        path = argv[1];
    }

    ofs::config::Config cfg;
    std::string err;
    bool ok = ofs::config::load_uconf_or_create_default ( path, cfg, err );

    if ( !ok )
    {
        std::cerr << "Failed to load uconf: " << err << "\n";
        return 1;
    }

    std::cout << "Loaded config from: " << path << "\n";
    std::cout << " total_size: " << cfg.total_size << "\n";
    std::cout << " header_size: " << cfg.header_size << "\n";
    std::cout << " block_size: " << cfg.block_size << "\n";
    std::cout << " max_files: " << cfg.max_files << "\n";
    std::cout << " max_filename_length: " << cfg.max_filename_length << "\n";
    std::cout << " max_users: " << cfg.max_users << "\n";
    std::cout << " admin_username: " << cfg.admin_username << "\n";
    std::cout << " require_auth: " << (cfg.require_auth ? "true" : "false") << "\n";
    std::cout << " server.port: " << cfg.port << "\n";
    return 0;
}