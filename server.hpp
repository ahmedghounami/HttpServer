#pragma once

#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <poll.h>
#include <fstream>
#include <vector>
#include <map>
#include <stack>
#include <sstream>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <string>
#include <signal.h>

#define READ_BUFFER_SIZE 100000
struct location
{
    std::string location_index;
    std::string path;
    std::vector<std::string> index;
    std::vector<std::string> allowed_methods;
    bool autoindex;
    std::vector<std::string> cgi_extension;
    std::string cgi_path_php;
    std::string cgi_path_py;
    int cgi_timeout;
    std::pair<std::string, std::string> redirect;
    std::string upload_path;
    int cout_index;
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
     int cout_index;
};




struct client_info
{
    std::string Path;
    int file_fd;
    int poll_status;
    int bodyTypeTaken;//flag
    size_t FileSize, chunkSize, pos;
    int index_server;
    bool isCgi;
    bool ReadFlag;
    bool ReadSize;
    bool isChunked;
    bool bodyTaken;
    bool bodyReached;
    bool headersTaken;
    
    std::string name, filename, contentTypeform;
    std::string data;
    std::string boundary;
    std::string chunkData;
    std::string ContentType;
    std::string method, uri, version, path_info;
    std::string query;
    std::map<std::string, std::string> headers;
    bool datafinished;
    int error_code;
    std::string response;
    std::string cgi_output;
    double bytes_sent;
    bool isGet;
    std::string upload_path;
    std::string post_cgi_filename;
    std::string ip;
    time_t last_time;
    std::string location;
    pid_t pid;
    
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


template <typename T>
std::string to_string_custom(const T& value) {
    std::ostringstream oss;
    oss << value;  // Converts any type that can be streamed into a string
    return oss.str();
}

// response

void post_success(client_info &client, std::string body);
void error_response(client_info &client, server_config& server, int error_code);
//to add new error just add in it a condition to handle the error header


// server
void accept_connection(int sock_connection, std::vector<pollfd> &clients_fds, std::map<int, client_info> &clients);

// config
void parse_key(std::istringstream &ss, std::string &key, server_config &config);
int is_digit(std::string str);
int somthing_after(std::istringstream &ss);
void parse_location(std::istringstream &ss, std::string &key, location &loc);


//Parsing
void ParseChunk(client_info &client, std::map<int, server_config> &server);
void FormData(client_info& client, std::map<int, server_config> &server);// Multipart/form-data
void OtherData(client_info &client, std::map<int, server_config> &server);// Raw/Binary data
void ChunkedFormData(client_info &client, std::map<int, server_config> &server);// Chunked data -> Multipart/form-data
void ChunkedOtherData(client_info &client, std::map<int, server_config> &server);// Chunked data -> Raw/Binary data

//Parsing Utils
std::string trim(const std::string &str);
bool isValidHeaderKey(const std::string &key);
bool isValidHeaderValue(const std::string &value);
std::string toLower(const std::string& str);
std::string getBoundary(const std::string &contentType);
void writeToFile(std::string &body, int fd);
bool NewFile(client_info &client, std::map<int, server_config> &server);
std::string nameGenerator(std::string MimeType, std::string upload_path, bool isCgi);
bool validateAndNormalizePath(client_info &client, std::map<int, server_config> &server);
void tracing_uri(std::string &uri);

//handling methods
void handleGetRequest(client_info &client, std::map<int, server_config> &server);
void handleDeleteRequest(client_info &client, std::map<int, server_config> &server);

//find which server config to use returns the server index
int findMatchingServer(client_info &client, std::map<int, server_config> &server);
//this fuction to get the location of the file if she exists in config file if not it return ""
std::string getlocation(client_info &client, server_config &server); 
//this function to get the path from the config file from location if she exists if not it return the server path
std::string getcorectserver_path(client_info &client, std::map<int, server_config> &server);
//this function to send the body of the file to the client part by part
void sendbodypart(client_info &client, std::string path);
std::string getContentType(const std::string &path);


// autoindex
void generateAutoindexToFile(const std::string &uri, const std::string &directory_path, client_info &client);
bool autoindex_server(client_info &client, server_config &loc);
bool check_autoindex(client_info &client, std::map<int, server_config> &server);
void listingdirec(client_info &client, std::string body);

// redirect
void redirect(client_info &client, std::pair<std::string, std::string> &redirect);
bool handlepathinfo(client_info &client);
void handleCgi(client_info &client, std::map<int, server_config> &server);
