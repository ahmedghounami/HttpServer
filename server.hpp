#pragma once

#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <fstream>
#include <vector>
#include <map>

struct client_info
{
    std::string chunk;
    std::string boundary;
    std::string header;
};

void get_boundary(int _client_fd, std::map<int, client_info> &clients);
void accept_connection(int start_connection, std::vector<pollfd> &clients_fds, std::map<int, client_info> &clients);

#define PORT 8080