#include "server.hpp"
#include <sstream>
#include <string>

int is_digit(std::string str) {
  for (unsigned int i = 0; i < str.length(); i++) {
    if (!isdigit(str[i]))
      return 0;
  }
  return 1;
}

int somthing_after(std::istringstream &ss) {
  std::string something;
  ss >> something;
  if (something.empty() == false)
    throw std::runtime_error("something wrong after value");
  return 0;
}

void parse_location(std::istringstream &ss, std::string &key, location &loc) {
  if (key == "path") {
    std::string path;
    ss >> path;
    loc.path = path;
    somthing_after(ss);
  }
}

void parse_key(std::istringstream &ss, std::string &key,
               server_config &config) {

  if (key == "server_name") {
    for (std::string server_name; ss >> server_name;)
      config.server_names.push_back(server_name);
  } else if (key == "host") {
    std::string host;
    ss >> host;
    if (host != "localhost") {
      for (unsigned int i = 0; i < host.length(); i++) {
        if (!isdigit(host[i]) && host[i] != '.')
          throw std::runtime_error("Invalid host");
      }
    }
    somthing_after(ss);
    config.host = host;
  } else if (key == "listen") {
    for (std::string port; ss >> port;) {
      int ports = std::atof(port.c_str());
      if (ports <= 0 || ports > 65535 || !is_digit(port))
        throw std::runtime_error("Invalid port number");
      config.ports.push_back(std::atof(port.c_str()));
    }
  } else if (key == "path") {
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
  } else if (key == "upload_path") {
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
  } else if (key == "index") {
    for (std::string index; ss >> index;)
      config.index.push_back(index);
  } else if (key == "autoindex") {
    std::string autoindex;
    ss >> autoindex;
    if (autoindex == "on" || autoindex == "on")
      config.autoindex = true;
    else if (autoindex == "off" || autoindex == "off")
      config.autoindex = false;
    else
      throw std::runtime_error("Invalid config file1");
    somthing_after(ss);
  } else if (key == "client_max_body_size") {
    std::string max_body_size;
    ss >> max_body_size;
    if (atof(max_body_size.c_str()) <= 0 || !is_digit(max_body_size))
      throw std::runtime_error("Invalid body size");
    somthing_after(ss);
    config.max_body_size = std::atof(max_body_size.c_str());
  } else if (key == "upload_max_size") {
    std::string upload_max_size;
    ss >> upload_max_size;
    if (atof(upload_max_size.c_str()) <= 0 || !is_digit(upload_max_size))
      throw std::runtime_error("Invalid upload size");
    somthing_after(ss);
    config.upload_max_size = std::atof(upload_max_size.c_str());
  } else if (key == "error_page") {
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
  } else if (key.empty())
    return;
  else
    throw std::runtime_error("Invalid config file2");
}

void server::parse_config(std::string config_file) {
  int i = 0;
  std::stack<std::string> stack;
  std::ifstream file(config_file);
  if (!file.good())
    throw std::runtime_error("Config file could not be opened");
  std::string line;
  while (std::getline(file, line)) {
    std::istringstream ss(line);
    std::string key;
    std::string loc;
    ss >> key;
    if (line.empty())
      continue;
    if (line == "server {") {
      if (!stack.empty())
        throw std::runtime_error("cannot create server inside server");
      servers[i].server_index = i;
      stack.push("server");
      continue;
    }
    if (key == "location" && stack.top() == "server") {
      ss >> loc;
      if (loc.empty())
        throw std::runtime_error("location block not opened");
      if (line[line.length() - 1] != '{')
        throw std::runtime_error("location block not opened");
      stack.push("location");
      servers[i].locations[loc] = location();
      continue;
    } else if (key == "}" && somthing_after(ss) == 0) {
      if (stack.empty())
        throw std::runtime_error("server block not opened");
      else if (stack.top() == "server" &&
               (servers[i].server_names.empty() || servers[i].host.empty() ||
                servers[i].ports.empty() || servers[i].path.empty() ||
                servers[i].index.empty() || servers[i].max_body_size == 0)) {
        throw std::runtime_error("key missing in server block");
      }
      if (stack.top() == "server")
        i++;
      stack.pop();
      continue;
    }
    if (!stack.empty() && stack.top() == "location")
      parse_location(ss, key, servers[i].locations[loc]);
    else if (!stack.empty() && stack.top() == "server")
      parse_key(ss, key, servers[i]);
    else
      throw std::runtime_error("syntax error");
  }
  if (!stack.empty())
	throw std::runtime_error("server block not closed");
  //   for (unsigned int i = 0; i < servers.size(); i++) {
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
  //     std::cout << "-------------------" << std::endl;
  //   }
}
