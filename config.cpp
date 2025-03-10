#include "server.hpp"

void server::parse_config(std::string config_file)
{
    std::stack<std::string> stack;
    std::ifstream file(config_file);
    if (!file.good())
        throw std::runtime_error("Config file could not be opened");
    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty())
            continue;
        // stream to parse the line
        std::istringstream ss(line);
        if (line == "server {")
        {
            stack.push("server");
            continue;
        }
        if (line == "}")
        {
            stack.pop();
            break;
        }
        if (stack.top() == "server")
        {
            std::string key;
            ss >> key;

            if (key == "server_name")
            {
                ss >> config.server_name;
            }
            else if (key == "host:")
            {
                ss >> config.host;
            }
            else if (key == "listen")
            {
                for(std::string port; ss >> port;)
                {
                    int ports = std::stoi(port);
                    if (ports < 0 || ports > 65535) 
                        throw std::runtime_error("Invalid port number");
                    config.ports.push_back(std::stoi(port));
                }
            }
            else if (key == "path")
            {
                ss >> config.path;
            }
            else if (key == "index")
            {
                ss >> config.index;
            }
            else if (key == "autoindex")
            {
                std::string autoindex;
                ss >> autoindex;
                if (autoindex == "on;")
                    config.autoindex = true;
                else if (autoindex == "off;")
                    config.autoindex = false;
                else
                    throw std::runtime_error("Invalid config file");
            }
            else if (key == "client_max_body_size")
            {
                ss >> config.max_body_size;
            }
            else if(key.empty())
            {
                continue;
            }
            else
            {
                throw std::runtime_error("Invalid config file");
            }
        }
    }

    if (!stack.empty() || config.server_name.empty() || config.host.empty() || config.ports.empty() || config.path.empty() || config.index.empty() || config.max_body_size == 0)
        throw std::runtime_error("Invalid config file");
    // std::cout << "server_name: " << config.server_name << std::endl;
    // std::cout << "host: " << config.host << std::endl;
    // std::cout << "ports: ";
    // for (unsigned int i = 0; i < config.ports.size(); i++)
    //     std::cout << config.ports[i] << " ";
    // std::cout << std::endl;
    // std::cout << "path: " << config.path << std::endl;
    // std::cout << "index: " << config.index << std::endl;
    // std::cout << "autoindex: " << config.autoindex << std::endl;
    // std::cout << "max_body_size: " << config.max_body_size << std::endl;
}