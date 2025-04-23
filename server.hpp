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
#include <arpa/inet.h>
#include <dirent.h>

#define READ_BUFFER_SIZE 10000
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
    int poll_status;
    int bodyTypeTaken;//flag
    size_t chunkSize, pos;
    
    bool ReadFlag;
    bool autoindex;
    bool ReadSize;
    bool isChunked;
    bool bodyTaken;
    bool bodyReached;
    bool headersTaken;

    std::string name, filename, contentTypeform;
    std::string data;
    std::string boundary;
    std::string chunkData;
    std::vector<FormPart> formParts;
    std::string ContentType;
    std::string method, uri, version;
    std::string query;
    std::map<std::string, std::string> headers;
    bool datafinished;
    int error_code;
    std::string response;
    double bytes_sent;
    bool isGet;
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
void invalid_uri(client_info &client); // example: uri must start with '/'
void bad_request(client_info &client); // example: invalid or malformed HTTP version
void not_found(client_info &client); // example: file not found
void forbidden(client_info &client); // example: you are not allowed to access the file
void unknown_error(client_info &client); // example: unknown error
void timeoutserver(client_info &client); // example: timeout error from server to backend

void error_response(client_info &client, int error_code, std::string path);//this function can hndle all the errors
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
bool RequestLine(client_info &client, std::map<int, server_config> &server);
bool ParseHeaders(client_info &client);
bool TakeBodyType(client_info& client);//     Taking body type
void FormData(client_info& client);//         Raw/Binary data
void OtherData(client_info &client);//for other data
void ChunkedFormData(client_info &client);//  Chunked data -> Multipart/form-data
void ChunkedOtherData(client_info &client);// Chunked data -> Raw/Binary data

//Parsing Utils
std::string trim(const std::string &str);
bool isMultiValueHeader(const std::string &header);
bool isValidHeaderKey(const std::string &key);
bool isValidHeaderValue(const std::string &value);
std::string toLower(const std::string& str);
std::string getBoundary(const std::string &contentType);
void writeToFile(std::string &body, int fd);
void NewFileChunked(client_info &client);
void NewFile(client_info &client);
void ParseContentDisposition(client_info& client);
void ParseContentType(client_info& client);

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
void generateAutoindexToFile(const std::string &uri, const std::string &directory_path, const std::string &output_file_path);
bool autoindex_server(client_info &client, server_config &loc);
bool autoindex(client_info &client, location &loc);
bool check_autoindex(client_info &client, std::map<int, server_config> &server);

// redirect
void redirect(client_info &client, std::pair<std::string, std::string> &redirect);