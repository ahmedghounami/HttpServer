/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hamza <hamza@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/23 20:30:59 by hboudar           #+#    #+#             */
<<<<<<< HEAD
/*   Updated: 2025/03/10 22:29:25 by hboudar          ###   ########.fr       */
=======
/*   Updated: 2025/03/10 17:08:09 by hamza            ###   ########.fr       */
>>>>>>> 291919284494711e40333dc31ca0dff459549358
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <netinet/in.h>
#include <poll.h>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

#define PORT 8080

struct client_info {
  std::string chunk;
  bool isChunked;
  int contentLength;
  std::string method, uri, version;
  std::map<std::string, std::string> headers;
  std::multimap<std::string, std::string> multiheaders;

  // std::string header;
  // std::string boundary;
  // std::string boundary_end;
};

class server {
private:
  int start_connection;
  std::map<int, client_info> clients;
  std::vector<pollfd> clients_fds;
  std::vector<pollfd> listners;

public:
  server();
  void listen_for_connections();
  void pars_chunk(client_info &client, int index);
  ~server();
};

void accept_connection(int start_connection, std::vector<pollfd> &clients_fds,
                       std::map<int, client_info> &clients);

// parsing request
bool request_line(client_info &client);
bool pars_headers(client_info &client);
bool detectBodyType(client_info& client);

// utils
std::string trim(const std::string &str);
bool isMultiValueHeader(const std::string &header);
bool isValidHeaderKey(const std::string &key);
bool isValidHeaderValue(const std::string &value);
std::string toLower(const std::string& str);

// void get_boundary(int _client_fd, std::map<int, client_info> &clients);
