/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hboudar <hboudar@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/23 20:30:59 by hboudar           #+#    #+#             */
/*   Updated: 2025/03/09 17:59:54 by hboudar          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <cstddef>
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

#define PORT 8080

struct client_info {
  std::string chunk;
  std::string method, uri, version;
  std::map<std::string, std::string> headers;

  std::string header;
  std::string boundary;
  std::string boundary_end;
};

class server {
private:
  int start_connection;
  std::map<int, client_info> clients;
  std::vector<pollfd> clients_fds;

public:
  server();
  void listen_for_connections();
  void pars_chunk(client_info &client, int index);
  ~server();
};

// parsing request
bool request_line(client_info &client);
bool pars_headers(client_info &client);

// utils
std::string trim(const std::string &str);
void get_boundary(int _client_fd, std::map<int, client_info> &clients);
void accept_connection(int start_connection, std::vector<pollfd> &clients_fds,
                       std::map<int, client_info> &clients);
