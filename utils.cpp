/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hboudar <hboudar@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/09 14:24:50 by hboudar           #+#    #+#             */
/*   Updated: 2025/03/09 15:45:27 by hboudar          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"

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

bool pars_header(client_info &client) {
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