#include "../server.hpp"
#include <fstream>
std::string getresponse(int error_code, std::string &path, server_config &server)
{
    std::string response;
    if (error_code == 400)
        response = "HTTP/1.1 400 Bad Request\r\n";
    else if (error_code == 403)
        response = "HTTP/1.1 403 Forbidden\r\n";
    else if (error_code == 404)
        response = "HTTP/1.1 404 Not Found\r\n";
    else if (error_code == 405)
        response = "HTTP/1.1 405 Method Not Allowed\r\n";
    else if (error_code == 501)
        response = "HTTP/1.1 501 Not Implemented\r\n";
    else if (error_code == 504)
        response = "HTTP/1.1 504 Gateway Timeout\r\n";
    else if (error_code == 505)
        response = "HTTP/1.1 505 HTTP Version Not Supported\r\n";
    else
        response = "HTTP/1.1 500 Internal Server Error\r\n", error_code = 500;
    if(server.error_pages.find(std::to_string(error_code)) != server.error_pages.end())
        path = server.error_pages[std::to_string(error_code)];
    if(path == "" || std::ifstream(path.c_str()).fail())
    {
        path = "./error_pages/" + std::to_string(error_code) + ".html";
        if (std::ifstream(path.c_str()).fail())
        {
            path = "./error_pages/500.html";
            if (std::ifstream(path.c_str()).fail())
            {
                path = "";
                return response;
            }
        }
    }
    return response;
}
void error_response(client_info &client, server_config& server, int error_code)
{
    (void)server;
    std::string path = "";
    client.isGet = true;
    std::string response = getresponse(error_code, path, server);
    client.response.clear();
    client.error_code = error_code;
    client.poll_status = 1;

    if (client.bytes_sent <= 0 && client.bytes_sent != -1)
    {

        std::string conection = client.headers["connection"];
        client.response = response;
        client.response += "Content-Type: " + getContentType(path) + "\r\n";
        client.response += "Content-Length: ";
        std::ifstream file(path.c_str());
        file.seekg(0, std::ios::end);
        client.response += std::to_string(file.tellg()) + "\r\n";
        client.response += "Connection: " + conection + "\r\n";
        client.response += "\r\n";
        client.bytes_sent = ((double)client.response.size() * -1) - 1;
    }
    else
        sendbodypart(client, path);
}



void redirect(client_info &client, std::pair<std::string, std::string> &redirect)
{
    client.poll_status = 1;
    client.datafinished = true;
    std::string status_code = redirect.first;
    std::string location_url = redirect.second;
    client.response += "HTTP/1.1 " ;
    client.response += status_code += " ";
    if (status_code == "301")
        client.response += "Moved Permanently";
    else if (status_code == "302")
        client.response += "Found";
    else
        client.response += "Redirect"; // fallback

    client.response += "\r\n";
    client.response += "Location: ";
    client.response += location_url;
    client.response += "\r\n";
    client.response += "Content-Length: 0\r\n";
    client.response += "Connection: close\r\n";
    client.response += "\r\n";

    // Send response to client
}

void post_success(client_info &client, std::string body)
{
    client.poll_status = 1;
    client.datafinished = true;
    client.response = "HTTP/1.1 200 OK\r\n";
    client.response += "Content-Type: text/html\r\n";
    client.response += "Content-Length: ";
    client.response += std::to_string(body.size()) + "\r\n";
    client.response += "Connection: close\r\n";
    client.response += "\r\n";
    client.response += body;
    if (client.uri != "/")
        client.isGet = true;
}

