#include "../server.hpp"


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

void path_checker(std::string path)
{
    struct stat info;
    if (stat(path.c_str(), &info) != 0)
        throw std::runtime_error(path + " : path does not exist");
    else if (access(path.c_str(), R_OK) != 0)
        throw std::runtime_error(path + " : path is not readable");
    else if (access(path.c_str(), W_OK) != 0)
        throw std::runtime_error(path + " : path is not writable");
}

void parse_location(std::istringstream &ss, std::string &key, location &loc)
{

    if (key == "path")
    {
        std::string path;
        ss >> path;
        if (loc.path != "")
            throw std::runtime_error("location: Duplicate path ");
        else if (path.empty())
            throw std::runtime_error("location: emtpy path");
        path_checker(path);
        somthing_after(ss);
        loc.path = path;
    }
    else if (key == "index")
    {
        if (loc.index.size() != 0)
            throw std::runtime_error("location: Duplicate index");
        for (std::string index; ss >> index;)
            loc.index.push_back(index);
        if (loc.index.size() == 0)
            throw std::runtime_error("location: Empty index");
    }
    else if (key == "autoindex")
    {
        if (loc.cout_index != 0)
            throw std::runtime_error("location: Duplicate autoindex");
        std::string autoindex;
        ss >> autoindex;
        if (autoindex == "on" || autoindex == "on")
            loc.autoindex = true;
        else if (autoindex == "off" || autoindex == "off")
            loc.autoindex = false;
        else
            throw std::runtime_error("location: Invalid autoindex");
        somthing_after(ss);
        loc.cout_index = 1;
    }
    else if (key == "allowed_methods")
    {
        if (loc.allowed_methods.size() != 0)
            throw std::runtime_error("location: Duplicate allowed_methods");
        for (std::string method; ss >> method;)
        {
            if (method != "GET" && method != "POST" && method != "DELETE")
                throw std::runtime_error("location: Invalid method");
            loc.allowed_methods.push_back(method);
        }
        if (loc.allowed_methods.size() == 0)
            throw std::runtime_error("location: Empty allowed_methods");
    }
    else if (key == "cgi_extensions")
    {
        if (loc.cgi_extension.size() != 0)
            throw std::runtime_error("location: Duplicate cgi_extension");
        for (std::string cgi_extension; ss >> cgi_extension;)
            loc.cgi_extension.push_back(cgi_extension);
        if (loc.cgi_extension.size() == 0)
            throw std::runtime_error("location: Empty cgi_extension");
    }
    else if (key == "cgi_path")
    {
        std::string cgi_path;
        ss >> cgi_path;
        if (loc.cgi_path != "")
            throw std::runtime_error("location: Duplicate cgi_path ");
        path_checker(cgi_path);
        somthing_after(ss);
        loc.cgi_path = cgi_path;
    }
    else if (key == "cgi_timeout")
    {
        if (loc.cgi_timeout != 0)
            throw std::runtime_error("location: Duplicate timeout");
        std::string cgi_timeout;
        ss >> cgi_timeout;
        if (atof(cgi_timeout.c_str()) <= 0 || !is_digit(cgi_timeout))
            throw std::runtime_error("location: Invalid timeout");
        somthing_after(ss);
        loc.cgi_timeout = std::atof(cgi_timeout.c_str());
    }
    else if (key == "redirect")
    {
        if (loc.redirect.first != "")
            throw std::runtime_error("location: Duplicate redirect");
        std::string status;
        std::string redirect_path;
        ss >> status;
        ss >> redirect_path;
        if (redirect_path.empty() || status.empty() || !is_digit(status))
            throw std::runtime_error("location: Invalid redirect");
        somthing_after(ss);
        loc.redirect.first = status;
        if (std::atof(loc.redirect.first.c_str()) <= 300 || std::atof(loc.redirect.first.c_str()) >= 309)
            throw std::runtime_error("location: Invalid redirect status code");
        loc.redirect.second = redirect_path;
    }
    else if (key == "upload_path")
    {
        if (loc.upload_path != "")
            throw std::runtime_error("location: Duplicate path");
        std::string upload_path;
        ss >> upload_path;
        path_checker(upload_path);
        somthing_after(ss);
        loc.upload_path = upload_path;
    }

    else if (key.empty())
        return;
    else
        throw std::runtime_error("Invalid key in location : " + key);
}

void parse_key(std::istringstream &ss, std::string &key,
               server_config &config)
{

    if (key == "server_name")
    {
        if (config.server_names.size() != 0)
            throw std::runtime_error("Duplicate server name");
        for (std::string server_name; ss >> server_name;)
            config.server_names.push_back(server_name);
        if (config.server_names.size() == 0)
            throw std::runtime_error("Empty server name");
    }
    else if (key == "host")
    {
        if (config.host != "")
            throw std::runtime_error("Duplicate host");
        std::string host;
        ss >> host;
        somthing_after(ss);
        config.host = host;
    }
    else if (key == "listen")
    {
        if (config.ports.size() != 0)
            throw std::runtime_error("Duplicate port");
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
        if (config.path != "")
            throw std::runtime_error("Duplicate path");
        path_checker(path);
        somthing_after(ss);
        config.path = path;
    }
    else if (key == "upload_path")
    {
        std::string path;
        ss >> path;
        if (config.upload_path != "")
            throw std::runtime_error("Duplicate path");
        path_checker(path);
        somthing_after(ss);
        config.upload_path = path;
    }
    else if (key == "index")
    {
        if (config.index.size() != 0)
            throw std::runtime_error("Duplicate index");
        for (std::string index; ss >> index;)
            config.index.push_back(index);
        if (config.index.size() == 0)
            throw std::runtime_error("Empty index");
    }
    else if (key == "autoindex")
    {
        if (config.cout_index != 0)
            throw std::runtime_error("Duplicate autoindex");
        std::string autoindex;
        ss >> autoindex;
        if (autoindex == "on" || autoindex == "on")
            config.autoindex = true;
        else if (autoindex == "off" || autoindex == "off")
            config.autoindex = false;
        else
            throw std::runtime_error("Invalid config file1");
        somthing_after(ss);
        config.cout_index = 1;
    }
    else if (key == "client_max_body_size")
    {
        if (config.max_body_size != 0)
            throw std::runtime_error("Duplicate body size");
        std::string max_body_size;
        ss >> max_body_size;
        if (atof(max_body_size.c_str()) <= 0 || !is_digit(max_body_size) || std::numeric_limits<size_t>::max() < std::atof(max_body_size.c_str()))
            throw std::runtime_error("Invalid body size");
        somthing_after(ss);
        config.max_body_size = std::atof(max_body_size.c_str());
    }
    else if (key == "error_page")
    {
        std::string error_code;
        std::string error_page;
        ss >> error_code;
        ss >> error_page;
        if (config.error_pages.find(error_code) != config.error_pages.end())
            throw std::runtime_error("Duplicate error code");
        if (error_code.empty() || !is_digit(error_code) || error_page.empty() ||
            std::atof(error_code.c_str()) < 100 ||
            std::atof(error_code.c_str()) > 599)
            throw std::runtime_error("Invalid error_page");
        somthing_after(ss);
        config.error_pages[error_code] = error_page;
    }
    else if (key.empty())
        return;
    else
        throw std::runtime_error("Invalid key in config : " + key);
}
