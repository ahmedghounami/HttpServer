#include "server.hpp"

int is_digit(std::string str)
{
    for (unsigned int i = 0; i < str.length(); i++)
    {
        if (!isdigit(str[i]))
            return 0;
    }
    return 1;
}

int somthing_after(std::istringstream &ss)
{
    std::string something;
    ss >> something;
    if (something.empty() == false)
        throw std::runtime_error("something wrong after value");
    return 0;
}

void parse_location(std::istringstream &ss, std::string &key, location &loc)
{
    if (key == "path")
    {
        std::string path;
        ss >> path;
        struct stat info;
        if (loc.path != "")
            throw std::runtime_error("Duplicate path");
        if (stat(path.c_str(), &info) != 0)
            throw std::runtime_error("path does not exist");
        else if (access(path.c_str(), R_OK) != 0)
            throw std::runtime_error("path is not readable");
        else if (access(path.c_str(), W_OK) != 0)
            throw std::runtime_error("path is not writable");
        somthing_after(ss);
        loc.path = path;
    }
    else if (key == "index")
    {
        for (std::string index; ss >> index;)
            loc.index.push_back(index);
    }
    else if (key == "autoindex")
    {
        std::string autoindex;
        ss >> autoindex;
        if (autoindex == "on" || autoindex == "on")
            loc.autoindex = true;
        else if (autoindex == "off" || autoindex == "off")
            loc.autoindex = false;
        else
            throw std::runtime_error("Invalid config file1");
        somthing_after(ss);
    }
    else if (key == "allowed_methods")
    {
        for (std::string method; ss >> method;)
            loc.allowed_methods.push_back(method);
    }
    else if (key == "cgi_extension")
    {
        std::string cgi_extension;
        ss >> cgi_extension;
        somthing_after(ss);
        loc.cgi_extension = cgi_extension;
    }
    else if (key == "cgi_path")
    {
        std::string cgi_path;
        ss >> cgi_path;
        struct stat info;
        if (loc.cgi_path != "")
            throw std::runtime_error("Duplicate path in location");
        if (stat(cgi_path.c_str(), &info) != 0)
            throw std::runtime_error("path does not exist in location");
        else if (access(cgi_path.c_str(), R_OK) != 0)
            throw std::runtime_error("path is not readable in location");
        else if (access(cgi_path.c_str(), W_OK) != 0)
            throw std::runtime_error("path is not writable in location");
        somthing_after(ss);
        loc.cgi_path = cgi_path;
    }
    else if (key == "cgi_timeout")
    {
        std::string cgi_timeout;
        ss >> cgi_timeout;
        if (atof(cgi_timeout.c_str()) <= 0 || !is_digit(cgi_timeout))
            throw std::runtime_error("Invalid timeout");
        somthing_after(ss);
        loc.cgi_timeout = std::atof(cgi_timeout.c_str());
    }
    else if (key == "redirect")
    {
        std::string redirect;
        ss >> redirect;
        somthing_after(ss);
        loc.redirect = redirect;
    }
    else if (key == "upload_path")
    {
        std::string path;
        ss >> path;
        struct stat info;
        if (loc.upload_path != "")
            throw std::runtime_error("Duplicate path");
        if (stat(path.c_str(), &info) != 0)
            throw std::runtime_error("path does not exist");
        else if (access(path.c_str(), R_OK) != 0)
            throw std::runtime_error("path is not readable");
        else if (access(path.c_str(), W_OK) != 0)
            throw std::runtime_error("path is not writable");
        somthing_after(ss);
        loc.upload_path = path;
    }

    else if (key.empty())
        return;
    else
        throw std::runtime_error("Invalid config file2");
}

void parse_key(std::istringstream &ss, std::string &key,
               server_config &config)
{

    if (key == "server_name")
    {
        for (std::string server_name; ss >> server_name;)
            config.server_names.push_back(server_name);
    }
    else if (key == "host")
    {
        std::string host;
        ss >> host;
        if (host != "localhost")
        {
            for (unsigned int i = 0; i < host.length(); i++)
            {
                if (!isdigit(host[i]) && host[i] != '.')
                    throw std::runtime_error("Invalid host");
            }
        }
        somthing_after(ss);
        config.host = host;
    }
    else if (key == "listen")
    {
        for (std::string port; ss >> port;)
        {
            int ports = std::atof(port.c_str());
            if (ports <= 0 || ports > 65535 || !is_digit(port))
                throw std::runtime_error("Invalid port number");
            config.ports.push_back(std::atof(port.c_str()));
        }
    }
    else if (key == "path")
    {
        std::string path;
        ss >> path;
        struct stat info;
        if (config.path != "")
            throw std::runtime_error("Duplicate path");
        if (stat(path.c_str(), &info) != 0)
            throw std::runtime_error("path does not exist");
        else if (access(path.c_str(), R_OK) != 0)
            throw std::runtime_error("path is not readable");
        else if (access(path.c_str(), W_OK) != 0)
            throw std::runtime_error("path is not writable");
        somthing_after(ss);
        config.path = path;
    }
    else if (key == "upload_path")
    {
        std::string path;
        ss >> path;
        struct stat info;
        if (config.upload_path != "")
            throw std::runtime_error("Duplicate path");
        if (stat(path.c_str(), &info) != 0)
            throw std::runtime_error("path does not exist");
        else if (access(path.c_str(), R_OK) != 0)
            throw std::runtime_error("path is not readable");
        else if (access(path.c_str(), W_OK) != 0)
            throw std::runtime_error("path is not writable");

        somthing_after(ss);
        config.upload_path = path;
    }
    else if (key == "index")
    {
        for (std::string index; ss >> index;)
            config.index.push_back(index);
    }
    else if (key == "autoindex")
    {
        std::string autoindex;
        ss >> autoindex;
        if (autoindex == "on" || autoindex == "on")
            config.autoindex = true;
        else if (autoindex == "off" || autoindex == "off")
            config.autoindex = false;
        else
            throw std::runtime_error("Invalid config file1");
        somthing_after(ss);
    }
    else if (key == "client_max_body_size")
    {
        std::string max_body_size;
        ss >> max_body_size;
        if (atof(max_body_size.c_str()) <= 0 || !is_digit(max_body_size))
            throw std::runtime_error("Invalid body size");
        somthing_after(ss);
        config.max_body_size = std::atof(max_body_size.c_str());
    }
    else if (key == "upload_max_size")
    {
        std::string upload_max_size;
        ss >> upload_max_size;
        if (atof(upload_max_size.c_str()) <= 0 || !is_digit(upload_max_size))
            throw std::runtime_error("Invalid upload size");
        somthing_after(ss);
        config.upload_max_size = std::atof(upload_max_size.c_str());
    }
    else if (key == "error_page")
    {
        std::string error_code;
        std::string error_page;
        ss >> error_code;
        ss >> error_page;
        if (error_code.empty() || !is_digit(error_code) || error_page.empty() ||
            std::atof(error_code.c_str()) < 100 ||
            std::atof(error_code.c_str()) > 599)
            throw std::runtime_error("Invalid error code");
        somthing_after(ss);
        config.error_pages[error_code] = error_page;
    }
    else if (key.empty())
        return;
    else
        throw std::runtime_error("Invalid config file2");
}
