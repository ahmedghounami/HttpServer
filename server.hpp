#pragma once

#include <iostream>
#include <sys/_types/_ssize_t.h>
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
#include <stack>
#include <signal.h>
#include <algorithm>

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
    size_t max_body_size;
    std::map<std::string, std::string> error_pages;
    std::map<std::string, location> locations;
};

class server;

struct FormPart {
    std::string name;
    std::string filename;
    std::string contentType;
    std::string data;
};

struct client_info
{
    int file_fd;
    bool isChunked;
    bool bodyTaken;
    bool bodyReached;
    bool bodyTypeTaken;//flag
    int headersTaken;//flag
    size_t bytesLeft, chunkSize, pos;

    std::string name, filename, contentTypeform;
    int poll_status;
    std::string data;
    std::string boundary;
    std::string chunkData;
    std::vector<FormPart> formParts;
    std::string ContentType;
    std::string method, uri, version;
    std::string query;
    std::map<std::string, std::string> headers;

    std::string response;
  time_t last_time;
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

    public:
        server(std::string &config_file);
        void parse_config(std::string config_file);
        void listen_for_connections();
        void check_timeout(std::vector<pollfd> &clients_fds, std::map<int, client_info> &clients);
        ~server();

};

// response
void not_allowed_method(client_info &client);
void not_implemented_method(client_info &client);
void malformed_request(client_info &client);
void http_version_not_supported(client_info &client);


// server
void accept_connection(int sock_connection, std::vector<pollfd> &clients_fds, std::map<int, client_info> &clients);

// config
void parse_key(std::istringstream &ss, std::string &key, server_config &config);
int is_digit(std::string str);
int somthing_after(std::istringstream &ss);
void parse_location(std::istringstream &ss, std::string &key, location &loc);


// parsing request
void parse_chunk(client_info &client, std::map<int, server_config> &server);
bool request_line(client_info &client);
bool headers(client_info &client);
bool takeBody(client_info& client);
void ChunkedData(client_info &client);

//handling methods
void handleGetRequest(client_info &client, std::map<int, server_config> &server);
void handleDeleteRequest(client_info &client, std::map<int, server_config> &server);
// void handlePostRequest(client_info &client, std::map<int, server_config> &server);

// parsing utils
bool parseRequestPath(client_info& client);
std::string trim(const std::string &str);
bool isMultiValueHeader(const std::string &header);
bool isValidHeaderKey(const std::string &key);
bool isValidHeaderValue(const std::string &value);
std::string toLower(const std::string& str);
std::string getBoundary(const std::string &contentType);
bool isValidContentLength(const std::string &lengthStr);
void writeToFile(std::string &body, int fd);
