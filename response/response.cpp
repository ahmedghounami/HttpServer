#include "../server.hpp"
#include <fstream>

void handleGetRequest(client_info &client, std::map<int, server_config> &server)
{
    (void)server;
    (void)client;
    // std::cout << client.boundary << std::endl;
    // std::cout << client.method << std::endl;
    // std::cout << client.uri << std::endl;
    // std::cout << client.version << std::endl;
}

void not_allowed_method(client_info &client)
{
    client.poll_status = 1;
    client.response = "HTTP/1.1 405 Method Not Allowed\r\nContent-Type: text/html\r\nContent-Length: ";
    std::ifstream file("errors/405.html");
    std::string body((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    client.response += std::to_string(body.size()) + "\r\n\r\n" + body;
}

void not_implemented_method(client_info &client)
{
    client.poll_status = 1;
    client.response = "HTTP/1.1 501 Not Implemented\r\nContent-Type: text/html\r\nContent-Length: ";
    std::ifstream file("errors/501.html");
    std::string body((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    client.response += std::to_string(body.size()) + "\r\n\r\n" + body;
}

void http_version_not_supported(client_info &client)
{
    client.poll_status = 1;
    client.response = "HTTP/1.1 505 HTTP Version Not Supported\r\nContent-Type: text/html\r\nContent-Length: ";
    std::ifstream file("errors/505.html");
    std::string body((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    client.response += std::to_string(body.size()) + "\r\n\r\n" + body;
}

void bad_request(client_info &client)
{
    client.poll_status = 1;
    client.response = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/html\r\nContent-Length: ";
    std::ifstream file("errors/400.html");
    std::string body((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    client.response += std::to_string(body.size()) + "\r\n\r\n" + body;
}

void not_found(client_info &client)
{
    client.poll_status = 1;
    client.response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length: ";
    std::ifstream file("errors/404.html");
    std::string body((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    client.response += std::to_string(body.size()) + "\r\n\r\n" + body;
}

