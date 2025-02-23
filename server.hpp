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
};

std::map<int, client_info> clients;

#define PORT 8080