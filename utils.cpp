/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hboudar <hboudar@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/09 14:24:50 by hboudar           #+#    #+#             */
/*   Updated: 2025/03/12 15:33:03 by hboudar          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"

// void get_boundary(int _client_fd, std::map<int, client_info> &clients) {
//   std::string boundary_start = "boundary=";
//   size_t start_pos = clients[_client_fd].chunk.find(boundary_start);
//   size_t end_pos = clients[_client_fd].chunk.find("\r\n", start_pos);
//   clients[_client_fd].boundary = clients[_client_fd].chunk.substr(
//       start_pos + boundary_start.length(),
//       end_pos - start_pos - boundary_start.length());
//   size_t pos = clients[_client_fd].chunk.find("\r\n\r\n");
//   pos = clients[_client_fd].chunk.find("\r\n\r\n", pos + 4);
//   std::string header =
//       clients[_client_fd].chunk.substr(0,
//       clients[_client_fd].chunk.find(pos));
//   clients[_client_fd].chunk = clients[_client_fd].chunk.substr(pos + 4);
//   clients[_client_fd].header = header;
//   std::cout << "Boundary: " << clients[_client_fd].boundary << std::endl;
//   std::cout << "========================================" << std::endl;
//   std::cout << "Header: " << clients[_client_fd].header << std::endl;
// }

void accept_connection(int start_connection, std::vector<pollfd> &clients_fds,
                       std::map<int, client_info> &clients) {
  sockaddr_in client_addr;
  socklen_t client_len = sizeof(client_addr);
  int client_sock =
      accept(start_connection, (struct sockaddr *)&client_addr, &client_len);
  // std::cout << "new connection---------------: " << client_sock << std::endl;
  if (client_sock < 0)
    return;
  struct pollfd newfd;
  newfd.fd = client_sock;
  newfd.events = POLLIN;

  clients_fds.push_back(newfd);
  clients[client_sock] = client_info();
}

std::string trim(const std::string &str) {
  size_t first = str.find_first_not_of(" \t");
  if (first == std::string::npos)
    return "";
  size_t last = str.find_last_not_of(" \t");
  return str.substr(first, last - first + 1);
}

bool isMultiValueHeader(const std::string &header) {
  static const char *multiHeader[] = {"set-cookie",          "www-authenticate",
                                      "proxy-authenticate",  "authorization",
                                      "proxy-authorization", "warning"};
  for (size_t i = 0; i < sizeof(multiHeader) / sizeof(multiHeader[0]); ++i) {
    if (header == multiHeader[i])
      return true;
  }

  return false;
}

bool isValidHeaderKey(const std::string &key) {
  if (key.empty())
    return false;
  for (size_t i = 0; i < key.length(); ++i) {
    if (!isalpha(key[i]) && key[i] != '-' && key[i] != '_')
      return false;
  }
  return true;
}

bool isValidHeaderValue(const std::string &value) {
  for (size_t i = 0; i < value.length(); ++i) {
    if (iscntrl(value[i]) && value[i] != '\t') {
      return false;
    }
  }
  return true;
}

std::string toLower(const std::string& str) {
    std::string lowerStr = str;
    for (size_t i = 0; i < lowerStr.length(); ++i) {
        lowerStr[i] = std::tolower(lowerStr[i]);
    }
    return lowerStr;
}

std::string getBoundary(const std::string &contentType) {
  size_t pos = contentType.find("boundary=");
  if (pos != std::string::npos)
    return "--" + contentType.substr(pos + 9);
  return "";
}

bool isValidContentLength(const std::string &lengthStr) {
  std::string trlen = trim(lengthStr);
  
  for(size_t i = 0; i < trlen.length(); ++i) {
    if (!std::isdigit(trlen[i]))
      return false;
  }
  return true;
}