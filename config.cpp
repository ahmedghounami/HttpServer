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

void parse_key(std::istringstream &ss, std::string &key, server_config &config)
{
    ss >> key;

    if (key == "server_name")
        ss >> config.server_name;
    else if (key == "host:")
        ss >> config.host;
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
        config.upload_path = path;
    }
    else if (key == "index")
    {
        for(std::string index; ss >> index;)
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
    }
    else if (key == "client_max_body_size")
    {
        std::string max_body_size;
        ss >> max_body_size;
        if (atof(max_body_size.c_str()) <= 0 || !is_digit(max_body_size))
            throw std::runtime_error("Invalid body size");

        config.max_body_size = std::atof(max_body_size.c_str());
    }
    else if (key == "upload_max_size")
    {
        std::string upload_max_size;
        ss >> upload_max_size;
        if (atof(upload_max_size.c_str()) <= 0 || !is_digit(upload_max_size))
            throw std::runtime_error("Invalid upload size");

        config.upload_max_size = std::atof(upload_max_size.c_str());
    }
    else if (key == "keepalive_timeout")
    {
        std::string keepalive_timeout;
        ss >> keepalive_timeout;
        if (atof(keepalive_timeout.c_str()) <= 0 || !is_digit(keepalive_timeout))
            throw std::runtime_error("Invalid keepalive_timeout");

        config.keepalive_timeout = std::atof(keepalive_timeout.c_str());
    }
    else if (key == "send_timeout")
    {
        std::string send_timeout;
        ss >> send_timeout;
        if (atof(send_timeout.c_str()) <= 0 || !is_digit(send_timeout))
            throw std::runtime_error("Invalid send_timeout");

        config.send_timeout = std::atof(send_timeout.c_str());
    }
    else if (key == "recv_timeout")
    {
        std::string recv_timeout;
        ss >> recv_timeout;
        if (atof(recv_timeout.c_str()) <= 0 || !is_digit(recv_timeout))
            throw std::runtime_error("Invalid recv_timeout");

        config.recv_timeout = std::atof(recv_timeout.c_str());
    }
    else if (key == "access_log")
    {
        std::string access_log;
        ss >> access_log;
        if (access_log.empty())
            throw std::runtime_error("Invalid access_log");
        config.access_log = access_log;
    }
    else if (key == "error_log")
    {
        std::string error_log;
        ss >> error_log;
        if (error_log.empty())
            throw std::runtime_error("Invalid error_log");
        config.error_log = error_log;
    } 
    else if (key == "error_page")
    {
        std::string error_code;
        std::string error_page;
        ss >> error_code;
        ss >> error_page;
        std::string something;
        ss >> something;
        if (error_code.empty() || error_page.empty() || !is_digit(error_code) || something.empty() == false)
            throw std::runtime_error("Invalid error_page");
        config.error_pages[error_code] = error_page;
    }
    else if (key.empty())
        return;
    else
        throw std::runtime_error("Invalid config file2");
}

void server::parse_config(std::string config_file)
{
    int i = 0;
    std::stack<std::string> stack;
    std::ifstream file(config_file);
    if (!file.good())
        throw std::runtime_error("Config file could not be opened");
    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty())
            continue;
        if (line == "server {")
        {
            if (!stack.empty())
                throw std::runtime_error("cannot create server inside server");
            servers[i].server_index = i;
            stack.push("server");
            continue;
        }
        if (line == "}")
        {
            if (stack.empty() || stack.top() != "server")
                throw std::runtime_error("server block not opened");
            else if (servers[i].server_name.empty() || servers[i].host.empty() || servers[i].ports.empty() || servers[i].path.empty() || servers[i].index.empty() || servers[i].max_body_size == 0)
                throw std::runtime_error("key missing in server block");
            stack.pop();
            i++;
            continue;
        }
        if (!stack.empty() && stack.top() == "server")
        {
            std::istringstream ss(line);
            std::string key;
            parse_key(ss, key, servers[i]);
        }
        else
            throw std::runtime_error("syntax error");
    }
    // if (!stack.empty() || config.server_name.empty() || config.host.empty() || config.ports.empty() || config.path.empty() || config.index.empty() || config.max_body_size == 0)
    //     throw std::runtime_error("Invalid config file 4");
    for (unsigned int i = 0; i < servers.size(); i++)
    {
        std::cout << "server_name: " << servers[i].server_name << std::endl;
        std::cout << "host: " << servers[i].host << std::endl;
        for (unsigned int j = 0; j < servers[i].ports.size(); j++)
            std::cout << "port: " << servers[i].ports[j] << std::endl;
        std::cout << "path: " << servers[i].path << std::endl;
        std::cout << "upload_path: " << servers[i].upload_path << std::endl;
        for (unsigned int j = 0; j < servers[i].index.size(); j++)
            std::cout << "index: " << servers[i].index[j] << std::endl;
        std::cout << "autoindex: " << servers[i].autoindex << std::endl;
        std::cout << "max_body_size: " << servers[i].max_body_size << std::endl;
        std::cout << "upload_max_size: " << servers[i].upload_max_size << std::endl;
        std::cout << "keepalive_timeout: " << servers[i].keepalive_timeout << std::endl;
        std::cout << "send_timeout: " << servers[i].send_timeout << std::endl;
        std::cout << "recv_timeout: " << servers[i].recv_timeout << std::endl;
        std::cout << "access_log: " << servers[i].access_log << std::endl;
        std::cout << "error_log: " << servers[i].error_log << std::endl;
        for (std::map<std::string, std::string>::iterator it = servers[i].error_pages.begin(); it != servers[i].error_pages.end(); it++)
            std::cout << "error_page: " << it->first << " " << it->second << std::endl;
        std::cout << "-------------------" << std::endl;
    }
}
