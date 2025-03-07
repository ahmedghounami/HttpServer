/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hboudar <hboudar@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/23 20:30:59 by hboudar           #+#    #+#             */
/*   Updated: 2025/03/07 17:02:50 by hboudar          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <fstream>
#include <iostream>
#include <map>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

struct client_info {
  std::string chunk;
  std::string boundary;
  std::string header;
  std::string methd;
  std::string request_uri;
  std::string version;
};

class server {
private:
  int start_connection;
  std::map<int, client_info> clients;
  std::vector<pollfd> clients_fds;

public:
  server();
  void listen_for_connections();
  ~server();
};

void get_boundary(int _client_fd, std::map<int, client_info> &clients);
void accept_connection(int start_connection, std::vector<pollfd> &clients_fds,
                       std::map<int, client_info> &clients);
void get_chunk(client_info &client, std::ofstream &file, size_t pos, int flag);

#define PORT 8080