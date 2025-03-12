#pragma once

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <poll.h>
#include <fstream>
#include <vector>
#include <map>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>
#include <string>

struct server_config
{
    int  server_index;
    std::string host;
    std::vector<int> ports;
    std::string server_name;
    std::string path;
    std::string upload_path;
    std::vector<std::string> index;
    bool autoindex;
    int max_body_size;
    int upload_max_size;
    std::map<std::string, std::string> error_pages;
};

struct client_info
{
    std::string chunk;
    std::string boundary;
    std::string header;
};

struct port_used
{
    int port;
    std::string server_name;
};

class server
{
    private:
        int start_connection;
        std::map<int, client_info> clients;
        std::vector<pollfd> clients_fds;
        std::vector<int> listners;
        std::map<int, server_config> servers;
        std::vector<port_used> ports_used;

    public:
        server(std::string &config_file);
        void parse_config(std::string config_file);
        void listen_for_connections();
        ~server();

};

void get_boundary(int _client_fd, std::map<int, client_info> &clients);
void accept_connection(int sock_connection, std::vector<pollfd> &clients_fds, std::map<int, client_info> &clients);
void get_chunk(client_info &client, std::ofstream &file, size_t pos, int flag);

#define PORT 9090
#define PORt_2 8080