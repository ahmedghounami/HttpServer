#pragma once

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
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

class server
{
    private:
        int start_connection;
        std::map<int, client_info> clients;
        std::vector<pollfd> clients_fds;
        std::vector<int> listners;

    public:
        server();
        void listen_for_connections();
        ~server();

};

void get_boundary(int _client_fd, std::map<int, client_info> &clients);
void accept_connection(int sock_connection, std::vector<pollfd> &clients_fds, std::map<int, client_info> &clients);
void get_chunk(client_info &client, std::ofstream &file, size_t pos, int flag);

#define PORT 9090
#define PORt_2 8080