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

struct FormInfo {
  bool isChunked;
  bool bodyReached;
  bool bodyTaken;
  int contentLength;
  std::string filename;//re edit
  std::string contentType; // re edidt
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
    size_t upload_max_size;
    std::map<std::string, std::string> error_pages;
    std::map<std::string, location> locations;
};

struct client_info
{
  std::string chunk;
  std::string boundary;
  std::string method, uri, version;
  FormInfo file;
  std::map<std::string, std::string> headers;
  std::multimap<std::string, std::string> multiheaders;
  std::map<std::string, std::string> dataInfo;

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


// server
void accept_connection(int sock_connection, std::vector<pollfd> &clients_fds, std::map<int, client_info> &clients);



// config
void parse_key(std::istringstream &ss, std::string &key, server_config &config);
int is_digit(std::string str);
int somthing_after(std::istringstream &ss);
void parse_location(std::istringstream &ss, std::string &key, location &loc);


// parsing request
void pars_chunk(client_info &client);
bool request_line(client_info &client);
bool headers(client_info &client);
bool bodyType(client_info& client);
bool multiPartFormData(client_info &client);//for chunked form-data
bool takeBody_ChunkedFormData(client_info &client);

// parsing utils
std::string trim(const std::string &str);
bool isMultiValueHeader(const std::string &header);
bool isValidHeaderKey(const std::string &key);
bool isValidHeaderValue(const std::string &value);
std::string toLower(const std::string& str);
std::string getBoundary(const std::string &contentType);
bool isValidContentLength(const std::string &lengthStr);

