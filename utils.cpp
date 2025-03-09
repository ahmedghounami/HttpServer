/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hboudar <hboudar@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/09 14:24:50 by hboudar           #+#    #+#             */
/*   Updated: 2025/03/09 17:32:52 by hboudar          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"
#include <algorithm>
#include <cctype>
#include <cstddef>
#include <string>

void get_boundary(int _client_fd, std::map<int, client_info> &clients) {
  std::string boundary_start = "boundary=";
  size_t start_pos = clients[_client_fd].chunk.find(boundary_start);
  size_t end_pos = clients[_client_fd].chunk.find("\r\n", start_pos);
  clients[_client_fd].boundary = clients[_client_fd].chunk.substr(
      start_pos + boundary_start.length(),
      end_pos - start_pos - boundary_start.length());
  size_t pos = clients[_client_fd].chunk.find("\r\n\r\n");
  pos = clients[_client_fd].chunk.find("\r\n\r\n", pos + 4);
  clients[_client_fd].header =
      clients[_client_fd].chunk.substr(0, clients[_client_fd].chunk.find(pos));
  clients[_client_fd].chunk = clients[_client_fd].chunk.substr(pos + 4);
}

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

bool request_line(client_info &client) {
  if (!client.method.empty())
    return true;
  size_t pos = client.chunk.find("\r\n");
  if (pos == std::string::npos)
    return true;

  std::string clientLine = client.chunk.substr(0, pos);
  client.chunk.erase(0, pos + 2);

  size_t firstSP = client.chunk.find(' ');
  size_t secondSP = client.chunk.find(' ', firstSP + 1);
  size_t thirdSP = client.chunk.find(' ', secondSP + 1);

  if (firstSP == std::string::npos || secondSP == std::string::npos ||
      thirdSP != std::string::npos) {
    // std::cerr << "Error: Malformed request line (Incorrect spaces)"
    //           << std::endl;
    // respond then clear client;
    return false;
  }

  client.method = client.chunk.substr(0, firstSP);
  client.uri = client.chunk.substr(firstSP + 1, secondSP - firstSP - 1);
  client.version = client.chunk.substr(secondSP + 1);

  if (client.method != "GET" && client.method != "DELETE" &&
      client.method != "POST") {
    // std::cerr << "Error: Unsupported HTTP method: " << client.method
    //           << std::endl;
    // respond then clear client;
    return false;
  }

  if (client.uri.empty() || client.uri[0] != '/') {
    // std::cerr << "Error: Invalid request-target (URI must start with '/')"
    // << std::endl;
    // respond then clear client;
    return false;
  }

  if (client.version != "HTTP/1.1") {
    // std::cerr << "Error: Unsupported HTTP version: " << client.version
    //           << std::endl;
    // respond then clear client;
    return false;
  }
  return true;
}

std::string trim(const std::string &str) {
  size_t first = str.find_first_not_of(" \t");
  if (first == std::string::npos)
    return "";
  size_t last = str.find_last_not_of(" \t");
  return str.substr(first, last - first + 1);
}

bool pars_headers(client_info &client) {
  if (!client.headers.empty())
    return true;

  size_t pos = client.chunk.find("\r\n\r\n");
  if (pos == std::string::npos)
    return true;

  std::string headers = client.chunk.substr(0, pos);
  client.chunk.erase(0, pos + 4);

  size_t startPos = 0;
  while (startPos < headers.size()) {
    size_t endPos = headers.find("\r\n", startPos);
    if (endPos == std::string::npos)
      break;

    std::string line = headers.substr(startPos, endPos - startPos);
    startPos = endPos + 2;

    if (line.empty())
      continue;

    size_t delimiterPos = line.find(":");
    if (delimiterPos == std::string::npos) {
      std::cerr << "Error: Malformed header (missing ':'): " << line
                << std::endl;
      // respond and clear client;
      return false;
    }

    std::string key = trim(line.substr(0, delimiterPos));
    std::string value = trim(line.substr(delimiterPos + 1));

    if (key.empty() || value.empty()) {
      std::cerr << "Error: Empty header name or value" << std::endl;
      // respond and clear client;
      return false;
    }

    std::transform(key.begin(), key.end(), key.begin(), ::tolower);
    client.headers[key] = value;
  }

  if (client.headers.find("host") == client.headers.end()) {
    std::cerr << "Error: Missing 'Host' header" << std::endl;
    // respond and clear client;
    return false;
  }

  std::map<std::string, std::string>::iterator it;
  for (it = client.headers.begin(); it != client.headers.end(); ++it) {
    std::cout << it->first << ": " << it->second << std::endl;
  }
}

return true;
}