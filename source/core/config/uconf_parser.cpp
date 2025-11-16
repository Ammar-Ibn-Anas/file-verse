#include "uconf_parser.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <filesystem>

using namespace std::literals;
namespace fs = std::filesystem;

namespace ofs::config
{
    static inline std::string trim(const std::string& s)
    {
        size_t a = s.find_first_not_of(" \t\r\n");
        if (a == std::string::npos)
            return "";

        size_t b = s.find_last_not_of(" \t\r\n");

        return s.substr(a, b - a + 1);
    }

    static inline std::string strip_quotes(const std::string& s)
    {
        if (s.size() >= 2 && ((s.front() == '"' && s.back() == '"') ||
                          (s.front() == '\'' && s.back() == '\'')))
            return s.substr(1, s.size() - 2);
        return s;
    }

    static bool parse_bool(const std::string& v,bool& out)
    {
        std::string s = v;
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        if (s == "true" || s == "1" || s == "yes" || s == "y") 
        {
             out = true; return true; 
        }
        if (s == "false" || s == "0" || s == "no" || s == "n") 
        { 
            out = false; return true; 
        }
        return false;
    }

    static bool parse_u64_dec(const std::string& v, uint64_t& out) 
    {
    try 
    {
        size_t idx = 0;
        if (v.empty()) return false;
        out = std::stoull(v, &idx, 10); 
        return idx == v.size();
    } catch (...) 
    {
        return false;
    }
    }
    
    static bool parse_u32_dec(const std::string& v, uint32_t& out) 
    {
        uint64_t tmp;
        if (!parse_u64_dec(v, tmp)) return false;
        if (tmp > UINT32_MAX) return false;
        out = static_cast<uint32_t>(tmp);
        return true;
    }

    static bool parse_u16_dec(const std::string& v, uint16_t& out) 
    {
        uint64_t tmp;
        if (!parse_u64_dec(v, tmp)) return false;
        if (tmp > UINT16_MAX) return false;
        out = static_cast<uint16_t>(tmp);
        return true;
    }

    bool write_uconf(const std::string& path, const Config& cfg, std::string& err) 
    {
        try 
        {
            fs::path p = path;
            fs::path dir = p.parent_path();
            if (!dir.empty() && !fs::exists(dir)) 
            {
                if (!fs::create_directories(dir)) 
                {
                    err = "unable to create directory: " + dir.string();
                    return false;
                }
            }

            std::ofstream os(path, std::ios::out | std::ios::trunc);
            if (!os) 
            { 
                err = "unable to open file for writing: " + path; return false; 
            }

            os << "[filesystem]\n";
            os << "total_size = " << cfg.total_size << "        # Total size in bytes\n";
            os << "header_size = " << cfg.header_size << "             # Header size\n";
            os << "block_size = " << cfg.block_size << "             # Block size\n";
            os << "max_files = " << cfg.max_files << "              # Maximum number of files\n";
            os << "max_filename_length = " << cfg.max_filename_length << "     # Maximum filename length\n\n";

            os << "[security]\n";
            os << "max_users = " << cfg.max_users << "                # Maximum number of users\n";
            os << "admin_username = \"" << cfg.admin_username << "\"      # Default admin username\n";
            os << "admin_password = \"" << cfg.admin_password << "\"      # Default admin password\n";
            os << "require_auth = " << (cfg.require_auth ? "true" : "false") << "           # Require authentication\n\n";

            os << "[server]\n";
            os << "port = " << cfg.port << "                   # Server port\n";
            os << "max_connections = " << cfg.max_connections << "          # Maximum simultaneous connections\n";
            os << "queue_timeout = " << cfg.queue_timeout << "            # Maximum queue wait time (seconds)\n";

            os.close();
            return true;
        } catch (const std::exception& e) 
        {
            err = std::string("exception writing uconf: ") + e.what();
            return false;
        }
}

    bool load_uconf_or_create_default(const std::string& path, Config& out_cfg, std::string& err) 
    {

        if (!fs::exists(path))
        {
            Config def;
            if (!write_uconf(path, def, err)) 
            {
                err = "failed to write default uconf: " + err;
                return false;
            }
            out_cfg = def;
            return true;
        }

        std::ifstream is(path);
        if (!is) 
        { 
            err = "unable to open uconf file: " + path; 
            return false; 
        }

        Config cfg = out_cfg;
        std::string line;
        std::string current_section;
        uint32_t line_no = 0;

        while(std::getline(is,line))
        {
            ++line_no;

            if (!line.empty() && line.back() == '\r') 
            {
                line.pop_back();
            }

            std::string t = trim(line);
            if(t.empty() || t.front() == '#' || t.front() == ';')
                continue;

            if(t.front() == '[' && t.back() == ']')
            {
                current_section = trim(t.substr(1, t.size() - 2));
                std::transform(current_section.begin(), current_section.end(), current_section.begin(), ::tolower);
                continue;
            }

            size_t eq = t.find('=');
            if (eq == std::string::npos) continue; // ignore malformed line
            std::string key = trim(t.substr(0, eq));
            std::string val = trim(t.substr(eq + 1));

            // strip inline comment
            size_t hashpos = val.find('#');
            if (hashpos != std::string::npos) val = trim(val.substr(0, hashpos));
            size_t semip = val.find(';');
            if (semip != std::string::npos) val = trim(val.substr(0, semip));

            // strip surrounding quotes for strings
            std::string sval = strip_quotes(val);

            // route by section + key
            std::string k = key;
            std::transform(k.begin(), k.end(), k.begin(), ::tolower);

            try 
            {
                if (current_section == "filesystem") 
                {
                    if (k == "total_size") 
                    {
                        uint64_t tmp; 
                        if (!parse_u64_dec(sval, tmp)) 
                        { 
                            err = "bad total_size at line " + std::to_string(line_no); 
                            return false; 
                        }
                        cfg.total_size = tmp;
                    } else if (k == "header_size") 
                    {
                        uint32_t tmp; 
                        if (!parse_u32_dec(sval,tmp)) 
                        { 
                            err = "bad header_size at line " + std::to_string(line_no); 
                            return false; 
                        }
                        cfg.header_size = tmp;
                    } else if (k == "block_size") 
                    {
                        uint32_t tmp; 
                        if (!parse_u32_dec(sval,tmp)) 
                        { 
                            err = "bad block_size at line " + std::to_string(line_no); 
                            return false; 
                        }
                        cfg.block_size = tmp;
                    } else if (k == "max_files") 
                    {
                        uint32_t tmp; 
                        if (!parse_u32_dec(sval,tmp)) 
                        { 
                            err = "bad max_files at line " + std::to_string(line_no); 
                            return false; 
                        }
                        cfg.max_files = tmp;
                    } else if (k == "max_filename_length") 
                    {
                        uint32_t tmp; 
                        if (!parse_u32_dec(sval,tmp)) 
                        { 
                            err = "bad max_filename_length at line " + std::to_string(line_no); 
                            return false; 
                        }
                        cfg.max_filename_length = tmp;
                    }
                } else if (current_section == "security") 
                {
                    if (k == "max_users") 
                    {
                        uint32_t tmp; 
                        if (!parse_u32_dec(sval,tmp)) 
                        { 
                            err = "bad max_users at line " + std::to_string(line_no); 
                            return false; 
                        }
                        cfg.max_users = tmp;
                    } else if (k == "admin_username") 
                    {
                        cfg.admin_username = sval;
                    } else if (k == "admin_password") 
                    {
                        cfg.admin_password = sval;
                    } else if (k == "require_auth") 
                    {
                        bool b; 
                        if (!parse_bool(sval,b)) 
                        { 
                            err = "bad require_auth at line " + std::to_string(line_no); 
                            return false; 
                        }
                        cfg.require_auth = b;
                    }
                } else if (current_section == "server") 
                {
                    if (k == "port") 
                    {
                        uint16_t tmp; 
                        if (!parse_u16_dec(sval,tmp)) 
                        { 
                            err = "bad port at line " + std::to_string(line_no); 
                            return false; 
                        }
                        cfg.port = tmp;
                    } else if (k == "max_connections") 
                    {
                        uint16_t tmp; 
                        if (!parse_u16_dec(sval,tmp)) 
                        { 
                            err = "bad max_connections at line " + std::to_string(line_no); 
                            return false; 
                        }
                        cfg.max_connections = tmp;
                    } else if (k == "queue_timeout") 
                    {
                        uint16_t tmp; 
                        if (!parse_u16_dec(sval,tmp)) 
                        { 
                            err = "bad queue_timeout at line " + std::to_string(line_no); 
                            return false; 
                        }
                        cfg.queue_timeout = tmp;
                    }
                } else 
                {
                }
            } catch (const std::exception& ex) 
            {
                err = "exception parsing uconf at line " + std::to_string(line_no) + ": " + ex.what();
                return false;
            }
        } 

        out_cfg = cfg;
        return true;
    }
}