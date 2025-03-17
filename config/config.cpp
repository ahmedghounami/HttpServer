#include "../server.hpp"


void server::parse_config(std::string config_file)
{
  int i = 0;
  std::stack<std::string> stack;
  std::ifstream file(config_file);
  if (!file.good())
    throw std::runtime_error("Config file could not be opened");
  std::string line;
  std::string location_index;
  while (std::getline(file, line))
  {
    std::istringstream ss(line);
    std::string key;
    ss >> key;
    if (line.empty())
      continue;
    else if (line == "server {")
    {
      if (!stack.empty())
        throw std::runtime_error("cannot create server inside server");
      servers[i].server_index = i;
      stack.push("server");
      continue;
    }
    else if (key == "location" && stack.top() == "server")
    {
      ss >> location_index;
      if (location_index.empty())
        throw std::runtime_error("location block not opened");
      if (servers[i].locations.find(location_index) != servers[i].locations.end())
        throw std::runtime_error("Duplicate location");
      std::string close;
      ss >> close;
      if (close != "{")
        throw std::runtime_error("location block not opened");
      stack.push("location");
      servers[i].locations[location_index].location_index = location_index;
      continue;
    }
    else if (key == "}" && somthing_after(ss) == 0)
    {
      if (stack.empty())
        throw std::runtime_error("server block not opened");
      else if (stack.top() == "server" &&
               (servers[i].host.empty() || servers[i].ports.empty() \
                || servers[i].path.empty()))
      {
        throw std::runtime_error("key missing in server block");
      }
      if (stack.top() == "server")
        i++;
      stack.pop();
      continue;
    }
    else if (!stack.empty() && stack.top() == "location")
      parse_location(ss, key, servers[i].locations[location_index]);
    else if (!stack.empty() && stack.top() == "server")
      parse_key(ss, key, servers[i]);
    else
      throw std::runtime_error("syntax error");
  }
  if (!stack.empty())
    throw std::runtime_error("server block not closed");
  else if (servers.empty())
    throw std::runtime_error("no server block found");
  for (unsigned int i = 0; i < servers.size(); i++)
  {
    //     std::cout << "host: " << servers[i].host << std::endl;
    //     for (unsigned int j = 0; j < servers[i].server_names.size(); j++)
    //       std::cout << "server_name: " << servers[i].server_names[j] <<
    //       std::endl;
    //     for (unsigned int j = 0; j < servers[i].ports.size(); j++)
    //       std::cout << "port: " << servers[i].ports[j] << std::endl;
    //     std::cout << "path: " << servers[i].path << std::endl;
    //     std::cout << "upload_path: " << servers[i].upload_path << std::endl;
    //     for (unsigned int j = 0; j < servers[i].index.size(); j++)
    //       std::cout << "index: " << servers[i].index[j] << std::endl;
    //     std::cout << "autoindex: " << servers[i].autoindex << std::endl;
    //     std::cout << "max_body_size: " << servers[i].max_body_size <<
    //     std::endl; std::cout << "upload_max_size: " <<
    //     servers[i].upload_max_size << std::endl; for (std::map<std::string,
    //     std::string>::iterator it =
    //              servers[i].error_pages.begin();
    //          it != servers[i].error_pages.end(); it++)
    //       std::cout << "error_page: " << it->first << " " << it->second
    //                 << std::endl;
    for (std::map<std::string, location>::iterator it = servers[i].locations.begin();
         it != servers[i].locations.end(); it++)
    {
      std::cout << "location: " << it->second.path << std::endl;
    }
    std::cout << "-------------------" << std::endl;
  }
}
