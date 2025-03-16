/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hboudar <hboudar@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/23 20:30:59 by hboudar           #+#    #+#             */
/*   Updated: 2025/03/16 18:08:21 by hboudar          ###   ########.fr       */
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

struct FormInfo {
  bool isChunked;
  bool bodyReached;
  bool bodyTaken;
  // std::string body; //re edit
  int contentLength;
  std::string filename;//re edit
  std::string contentType; // re edidt
};

struct client_info {
  std::string chunk;
  std::string boundary;
  std::string method, uri, version;
  FormInfo file;
  std::map<std::string, std::string> headers;
  std::multimap<std::string, std::string> multiheaders;
  std::map<std::string, std::string> dataInfo;

  // std::string header;
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
  ~server();
};

void accept_connection(int start_connection, std::vector<pollfd> &clients_fds,
                       std::map<int, client_info> &clients);

// parsing request
void pars_chunk(client_info &client);
bool request_line(client_info &client);
bool headers(client_info &client);
bool bodyType(client_info& client);
bool multiPartFormData(client_info &client);//for chunked form-data
bool takeBody_ChunkedFormData(client_info &client);

// utils
std::string trim(const std::string &str);
bool isMultiValueHeader(const std::string &header);
bool isValidHeaderKey(const std::string &key);
bool isValidHeaderValue(const std::string &value);
std::string toLower(const std::string& str);
std::string getBoundary(const std::string &contentType);
bool isValidContentLength(const std::string &lengthStr);

// void get_boundary(int _client_fd, std::map<int, client_info> &clients);
