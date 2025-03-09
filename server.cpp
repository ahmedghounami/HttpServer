/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hboudar <hboudar@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/23 20:30:55 by hboudar           #+#    #+#             */
/*   Updated: 2025/03/09 18:00:14 by hboudar          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"
#include <fstream>

server::server() {
  start_connection = socket(AF_INET, SOCK_STREAM, 0);
  if (start_connection == -1)
    throw std::runtime_error("Socket creation failed");

  sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  // htons() is used to convert the port number to network byte order
  server_addr.sin_port = htons(PORT);

  int opt = 1;
  setsockopt(start_connection, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  if (bind(start_connection, (struct sockaddr *)&server_addr,
           sizeof(server_addr)) < 0)
    throw std::runtime_error("Bind failed");

  // 128 is the maximum number of connections that can be waiting
  if (listen(start_connection, 128) < 0)
    throw std::runtime_error("Listen failed");
}

void server::listen_for_connections() {
  struct pollfd server_fd;
  server_fd.fd = start_connection;
  server_fd.events = POLLIN;
  clients_fds.push_back(server_fd);

  while (true) {
    int ret = poll(&clients_fds[0], clients_fds.size(), 5000);
    if (ret < 0) {
      std::cerr << "Poll failed\n";
      continue;
    } else if (ret == 0) {
      std::cerr << "No data, timeout\n";
      continue;
    }

    if (clients_fds[0].revents & POLLIN)
      accept_connection(start_connection, clients_fds, clients);

    for (unsigned int i = 1; i < clients_fds.size(); i++) {
      if (clients_fds[i].revents & POLLIN) {
        char buffer[1024];
        int data = recv(clients_fds[i].fd, buffer, sizeof(buffer) - 1, 0);

        if (data < 0)
          continue;

        buffer[data] = '\0';
        clients[clients_fds[i].fd].chunk.append(buffer, data);
        pars_chunk(clients[clients_fds[i].fd], i);
        clients[clients_fds[i].fd].chunk.clear();
      }
      if (clients_fds[i].revents & POLLOUT) {
        std::string response = "HTTP/1.1 200 OK\r\nContent-Type: "
                               "text/html\r\n\r\n<html><body><h1>File uploaded "
                               "successfully</h1></body></html>";
        ssize_t bytes =
            send(clients_fds[i].fd, response.c_str(), response.length(), 0);
        if (bytes < 0)
          continue;
        close(clients_fds[i].fd);
        clients_fds.erase(clients_fds.begin() + i);
      }
    }
  }
}

server::~server() {
  for (unsigned int i = 0; i < clients_fds.size(); i++)
    close(clients_fds[i].fd);
}

//-------------------------------------

void server::pars_chunk(client_info &client, int index) {
  if (request_line(client) == false || pars_headers(client) == false)
    return;
  // if (client.boundary.empty() &&
  //     client.chunk.find("\r\n\r\n") != std::string::npos)
  //   get_boundary(clients_fds[index].fd, clients);
  // std::string boundary_end = clients[clients_fds[index].fd].boundary +
  // "--"; size_t pos =
  // clients[clients_fds[index].fd].chunk.find(boundary_end); if (pos !=
  // std::string::npos) {
  //   client.chunk = client.chunk.substr(0, pos - 4);
  //   file << client.chunk;
  //   file.close();
  // } else
  //   file << client.chunk;
  (void)index;
}
