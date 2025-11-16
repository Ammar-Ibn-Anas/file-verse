#ifndef UCONF_PARSER_HPP
#define UCONF_PARSER_HPP

#include "config_types.hpp"
#include <string>

namespace ofs::config
{
    bool load_uconf_or_create_default(const std::string& path, Config& out_cfg,std::string& err);

    bool write_uconf(const std::string& path, const Config& cfg, std::string& err);
}

#endif // UCONF_PARSER_HPP