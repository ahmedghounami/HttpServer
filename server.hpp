/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hboudar <hboudar@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/23 20:30:59 by hboudar           #+#    #+#             */
/*   Updated: 2025/03/07 22:47:34 by hboudar          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <netinet/in.h>
#include <poll.h>
#include <sstream>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

struct client_info {
  std::string chunk;
  std::string boundary;
  std::string boundary_end;
  std::string header;
  std::string method, uri, version;
};

class server {
private:
  int start_connection;
  std::map<int, client_info> clients;
  std::vector<pollfd> clients_fds;

public:
  server();
  void listen_for_connections();
  void get_chunk(client_info &client, std::ofstream &file, int index);
  void clear(client_info &client);
  ~server();
};

// class HttpRequest {
// public:
//   std::string method;
//   std::string uri;
//   std::string version;
//   std::unordered_map<std::string, std::string> headers;
//   std::string body;

//   void clear() {
//     method.clear();
//     uri.clear();
//     version.clear();
//     headers.clear();
//     body.clear();
//   }
// };

//-------------------------------------

void get_boundary(int _client_fd, std::map<int, client_info> &clients);
void accept_connection(int start_connection, std::vector<pollfd> &clients_fds,
                       std::map<int, client_info> &clients);

#define PORT 8080