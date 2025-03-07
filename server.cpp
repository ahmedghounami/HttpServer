/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hboudar <hboudar@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/23 20:30:55 by hboudar           #+#    #+#             */
/*   Updated: 2025/03/07 22:49:53 by hboudar          ###   ########.fr       */
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
  server_addr.sin_port = htons(
      PORT); // htons() is used to convert the port number to network byte order

  int opt = 1;
  setsockopt(start_connection, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  if (bind(start_connection, (struct sockaddr *)&server_addr,
           sizeof(server_addr)) < 0)
    throw std::runtime_error("Bind failed");

  if (listen(start_connection, 128) <
      0) // 128 is the maximum number of connections that can be waiting
    throw std::runtime_error("Listen failed");

  std::cout << "Server running on port " << PORT << "\n";
}

void server::listen_for_connections() {
  struct pollfd server_fd;
  server_fd.fd = start_connection;
  server_fd.events = POLLIN;
  clients_fds.push_back(server_fd);

  ///////////////////////////////////////////////
  std::string filename = "data";
  std::ofstream file(filename);
  if (file.good())
    std::cerr << "File opened successfully\n";
  else {
    std::cerr << "File open failed\n";
    throw std::runtime_error("File open failed");
  }
  ///////////////////////////////////////////////
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

        // if (clients[clients_fds[i].fd].chunk.find("\r\n\r\n") !=
        // std::string::npos)
        //   get_boundary(clients_fds[i].fd, clients);
        // std::string boundary_end = clients[clients_fds[i].fd].boundary +
        // "--"; size_t pos =
        // clients[clients_fds[i].fd].chunk.find(boundary_end);
        // if (pos != std::string::npos) {
        //   get_chunk(clients[clients_fds[i].fd], file, pos, 1);
        //   clients_fds[i].events = POLLOUT;
        // } else
        // get_chunk(clients[clients_fds[i].fd], file);
        get_chunk(clients[clients_fds[i].fd], file, i);
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

void server::get_chunk(client_info &client, std::ofstream &file, int index) {
  if (client.boundary.empty() &&
      client.chunk.find("\r\n\r\n") != std::string::npos)
    get_boundary(clients_fds[index].fd, clients);
  std::string boundary_end = clients[clients_fds[index].fd].boundary + "--";
  size_t pos = clients[clients_fds[index].fd].chunk.find(boundary_end);
  if (pos != std::string::npos) {
    client.chunk = client.chunk.substr(0, pos - 4);
    file << client.chunk;
    file.close();
  } else
    file << client.chunk;

  // if (client.method.empty()) {
  //   size_t pos = client.chunk.find("\r\n");
  //   if (pos == std::string::npos)
  //     return;

  //   std::string clientLine = client.chunk.substr(0, pos);
  //   client.chunk.erase(0, pos + 2);

  //   std::istringstream ss(clientLine);
  //   ss >> client.method >> client.uri >> client.version;

  //   if (client.method.empty() || client.uri.empty() ||
  //   client.version.empty()) {
  //     std::cerr << "Invalid client line" << std::endl;
  //     clear(client);
  //     return;
  //   }

  //   if (client.version != "HTTP/1.1" && client.version != "HTTP/1.0") {
  //     std::cerr << "Unsupported HTTP version" << std::endl;
  //     clear(client);
  //     return;
  //   }

  //   std::cout << "Parsed client Line: " << client.method << " " << client.uri
  //             << " " << client.version << std::endl;
  // }

  // if (client.chunk.find("\r\n\r\n") != std::string::npos) {
  //   size_t pos2 = client.chunk.find("\r\n\r\n");
  //   std::string headers = client.chunk.substr(0, pos2);
  //   client.chunk.erase(0, pos2 + 4);

  //   std::istringstream headerStream(headers);
  //   std::string line;

  // while (std::getline(headerStream, line)) {
  // line = trim(line);
  // }
  // }
}

void clear(client_info &client) {
  client.method.clear();
  client.uri.clear();
  client.version.clear();
  client.header.clear();
}