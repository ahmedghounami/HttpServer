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
#include <utility>
#include <fcntl.h>

struct location
{
    std::string location_index;
    std::string path;
    std::vector<std::string> index;
    std::vector<std::string> allowed_methods;
    bool autoindex;
    std::vector<std::string> cgi_extension;
    std::string cgi_path;
    int cgi_timeout;
    std::pair<std::string, std::string> redirect;
    std::string upload_path;
};

struct server_config
{
    int  server_index;
    std::string host;
    std::vector<int> ports;
    std::vector<std::string> server_names;
    std::string path;
    std::string upload_path;
    std::vector<std::string> index;
    bool autoindex;
    int max_body_size;
    int upload_max_size;
    std::map<std::string, std::string> error_pages;
    std::map<std::string, location> locations;
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
void parse_key(std::istringstream &ss, std::string &key,
               server_config &config);
int is_digit(std::string str);
int somthing_after(std::istringstream &ss);
void parse_location(std::istringstream &ss, std::string &key, location &loc);
