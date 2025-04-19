/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handleRequest.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mkibous <mkibous@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/19 22:39:03 by hboudar           #+#    #+#             */
/*   Updated: 2025/04/19 11:50:40 by mkibous          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../server.hpp"
// Caching Considerations:
//  Because GET requests are cacheable, the caching behavior is defined
//  in related RFCs (e.g., RFC 7234). This means that responses to GET
//  requests can be stored and reused under the right conditions.

std::string getContentType(const std::string &path)
{
    std::string extension = path.substr(path.find_last_of(".") + 1);
    if (extension == "html" || extension == "htm")
        return "text/html";
    else if (extension == "css")
        return "text/css";
    else if (extension == "js")
        return "application/javascript";
    else if (extension == "json")
        return "application/json";
    else if (extension == "png")
        return "image/png";
    else if (extension == "jpg" || extension == "jpeg")
        return "image/jpeg";
    else if (extension == "pdf")
        return "application/pdf";
    else if (extension == "txt")
        return "text/plain";
    else if (extension == "mp4")
        return "video/mp4";
    else if (extension == "mp3")
        return "audio/mpeg";
    else if (extension == "xml")
        return "application/xml";
    return "text/plain";
}

int findMatchingServer(client_info &client, std::map<int, server_config> &server)
{
    std::string host;
    int port;
    int server_index = -1;

    for (std::map<std::string, std::string>::iterator it = client.headers.begin(); it != client.headers.end(); ++it)
    {
        if (it->first == "host")
        {
            std::istringstream iss(it->second);
            std::getline(iss, host, ':');
            iss >> port;
            for (std::map<int, server_config>::iterator it = server.begin(); it != server.end(); ++it)
            {
                if (std::find(it->second.ports.begin(), it->second.ports.end(), port) != it->second.ports.end() && server_index == -1)
                    server_index = it->second.server_index;
                if (it->second.host == host && std::find(it->second.server_names.begin(), it->second.server_names.end(), host) != it->second.server_names.end() && std::find(it->second.ports.begin(), it->second.ports.end(), port) != it->second.ports.end())
                {
                    server_index = it->second.server_index;
                    break;
                }
            }
        }
    }
    return server_index;
}
void success(client_info &client, std::string &body, std::string &path, std::string content_type = "", bool whith_header = true, double file_size = -1)
{
    if( content_type == "")
        content_type = getContentType(path);
    client.poll_status = 1;
    if (whith_header)
    {
        if(file_size == -1)
        {
            std::ifstream file(path.c_str());
            if (!file.is_open())
            {
                return;
            }
            file.seekg(0, std::ios::end);
            file_size = file.tellg();
            file.seekg(0, std::ios::beg);
        }
        client.response = "HTTP/1.1 200 OK\r\n";
        client.response += "Content-Type: " + content_type + "\r\n";
        client.response += "Content-Length: " + std::to_string(file_size) + "\r\n";
        client.response += "Connection: keep-alive\r\n";
        client.response += "\r\n";
        client.bytes_sent = ((double)client.response.size()  * -1) - 1;
    }
    else
        client.response += body;
}
std::string getlocation(client_info &client, server_config &server)
{
    // std::cout << "in getlocation funciton" << std::endl;
    
    std::string uri = client.uri;
    while (uri.size() > 0)
    {
        size_t pos = uri.find_last_of("/");
        if (pos == std::string::npos)
            break;
        if (pos == 0 && uri.size() > 1)
            pos++;
        // std::cout << "location: " << uri << std::endl;
        std::map<std::string, location>::iterator it = server.locations.find(uri);
        if (it != server.locations.end())
        {
            // std::cout << "found location: " << it->first << std::endl;
            return it->first;
        }
        uri = uri.substr(0, pos);
    }

    return "";
}
std::string getcorectserver_path(client_info &client, std::map<int, server_config> &server)
{
    // std::cout << "in getcorect_path funciton" << std::endl;
    int server_index = findMatchingServer(client, server);

    std::string loc = getlocation(client, server[server_index]);
    if (loc != "" && server[server_index].locations[loc].path != "")
        return server[server_index].locations[loc].path;
    return server[server_index].path;
}
std::string readheadercgi(int fd, std::string &body)
{

    char buffer[1024];
    int bytes_read;
    std::string headers; 
    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0)
    {
        headers += std::string(buffer, bytes_read);
        if (headers.find("\r\n\r\n") != std::string::npos)
        {
            size_t pos = headers.find("\r\n\r\n");
            size_t size = headers.size() - pos - 4;
            body = headers.substr(pos + 4, size);
            headers = headers.substr(0, pos + 4);
            break;
        }
    }
    return headers;
}
void handleCgi(client_info &client, std::map<int, server_config> &server, std::string &path)
{
    std::cout << "in cgi funciton" << std::endl;
    int fd[2];
    std::string body;
    std::string content_type = "";
    if (pipe(fd) == -1)
    {
        std::cerr << "pipe failed" << std::endl;
        return;
    }
    pid_t pid = fork();
    if (pid == -1)
    {
        std::cerr << "fork failed" << std::endl;
        return;
    }
    else if (pid == 0)
    {
        // child process
        dup2(fd[1], STDOUT_FILENO);
        close(fd[0]);
        close(fd[1]);
        char *cgi_path;
        if (path.substr(path.find_last_of(".") + 1) == "php")
            cgi_path = (char *)"/Users/mkibous/Desktop/webserver/CGI/php-cgi";
        else
            cgi_path = (char *)"/Users/mkibous/Desktop/webserver/CGI/python-cgi";
        char *args[] = {cgi_path, (char *)path.c_str(), NULL};
        alarm(5); // set timeout to 5 seconds
        execv(args[0], args);
        exit(0);
    }
    else
    {
        // parent process
        close(fd[1]);
        char buffer[1024];
        int bytes_read;
        std::string headers = readheadercgi(fd[0], body);
        if(client.bytes_sent <= 0 && client.bytes_sent != -1 )
        {
            while ((bytes_read = read(fd[0], buffer, sizeof(buffer))) > 0)
                body+= std::string(buffer, bytes_read);
            if(headers.size() > 0)
                content_type = headers.substr(headers.find("Content-Type: ") + 14, headers.find("\r\n", headers.find("Content-Type: ")) - headers.find("Content-Type: ") - 14);
            wait(NULL);
            alarm(0);
            close(fd[0]);
            if(content_type == "")
            {
                unknown_error(client);
                return ;
                
            }
            success(client, body, path, content_type, true, body.size());
            return;
        }
        else if(client.bytes_sent == -1)
            client.bytes_sent = 0;
        while  (body.size() < client.bytes_sent && (bytes_read = read(fd[0], buffer, sizeof(buffer))) > 0 )
            body += std::string(buffer, bytes_read);
        if(body.size() >= client.bytes_sent)
        {
            std::string temp = body.substr(client.bytes_sent, body.size() - client.bytes_sent);
            body = temp;
        }
        while (body.size() < READ_BUFFER_SIZE && (bytes_read = read(fd[0], buffer, sizeof(buffer))) > 0)
            body += std::string(buffer, bytes_read);
        if(bytes_read == 0)
            client.datafinished = 1;
        alarm(0);
        close(fd[0]);
        success(client, body, path, content_type, false);
        return;
    }
    (void)server;
}
void handleGetRequest(client_info &client, std::map<int, server_config> &server)
{
    std::cout << "in get funciton" << std::endl;
    client.response.clear();
    std::string content_type = "";


    std::string body;
    long content_size = -1;

    std::string path = getcorectserver_path(client, server) + client.uri;

    bool whith_header = 0;
    std::ifstream file(path.c_str());
    if (!file.is_open())
    {
        switch (errno)
        {
        case ENOENT:
            not_found(client); // 404
            break;
        case EACCES:
            forbidden(client); // 403
            break;
        default:
            unknown_error(client); // 500
            break;
        }
        return;
    }
    if(path.find_last_of(".") != std::string::npos)
    {
        std::cout << "in php file" << std::endl;
        if(path.substr(path.find_last_of(".") + 1) == "php" || path.substr(path.find_last_of(".") + 1) == "py")
        {
            handleCgi(client, server, path);
            return;
        }
    }
    if(client.bytes_sent <= 0 && client.bytes_sent != -1 )
    {
        success(client, body, path, content_type, true, content_size);
        return;
    }
    else if(client.bytes_sent == -1)
        client.bytes_sent = 0;
    char *buffer = new char[READ_BUFFER_SIZE];
    file.seekg(0, std::ios::beg);                 // move to the beginning
    file.seekg(client.bytes_sent, std::ios::beg); // move to byte  from the beginning
    file.read(buffer, READ_BUFFER_SIZE);
    if (file.eof())
    {
        client.datafinished = 1, std::cout << "file end reached" << std::endl;
    }
    body = std::string(buffer, file.gcount());
    delete[] buffer;

    success(client, body, path, content_type, whith_header);
    file.close();

}

void handleDeleteRequest(client_info &client, std::map<int, server_config> &server)
{
    std::cout << "in delete funciton" << std::endl;
    (void)client;
    (void)server;
}
// HTTP/1.1 200 OK
// Content-Type: text/html
// Content-Length: 56
// Connection: close

// html, .htm	                    text/html
// .css	                            text/css
// .js	                            application/javascript
// .json	                        application/json
// .png	                            image/png
// .jpg, .jpeg	                    image/jpeg
// .txt	                            text/plain
// .sh	                            text/x-shellscript or text/plain
// .pdf	                             application/pdf