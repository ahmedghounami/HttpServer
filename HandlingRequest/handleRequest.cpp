/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handleRequest.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mkibous <mkibous@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/19 22:39:03 by hboudar           #+#    #+#             */
/*   Updated: 2025/04/18 21:47:56 by mkibous          ###   ########.fr       */
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
        // std::cout << it->first << ": " << it->second << std::endl;
        if (it->first == "host")
        {
            std::istringstream iss(it->second);
            std::getline(iss, host, ':');
            iss >> port;

            // std::cout << "Host: " << host << std::endl;
            // std::cout << "Port: " << port << std::endl;

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
    // std::cout << "Server config found for host: " << host << " and port: " << port << "in server config index: " << server_index << std::endl;
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
            // std::cout << "file size: " << file_size << std::endl;
        }
        client.response = "HTTP/1.1 200 OK\r\n";
        client.response += "Content-Type: " + content_type + "\r\n";
        client.response += "Content-Length: " + std::to_string(file_size) + "\r\n";
        client.response += "Connection: keep-alive\r\n";
        client.response += "\r\n";
        client.bytes_sent = ((double)client.response.size()  * -1) - 1;
        // std::cout << "response size: " << client.bytes_sent << std::endl;
        // std::cout << "file size: " << file_size << std::endl;
        // sleep(10);
    }
    else
        client.response += body;
}
std::string getlocation(client_info &client, server_config &server)
{
    // std::cout << "in getlocation funciton" << std::endl;
    // finde last / in uri
    std::string uri = client.uri;
    while (uri.size() > 0)
    {
        size_t pos = uri.find_last_of("/");
        if (pos == std::string::npos)
            break;
        if (pos == 0 && uri.size() > 1)
            pos++;
        uri = uri.substr(0, pos);
        // std::cout << "location: " << uri << std::endl;
        std::map<std::string, location>::iterator it = server.locations.find(uri);
        if (it != server.locations.end())
        {
            // std::cout << "found location: " << it->first << std::endl;
            return it->first;
        }
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
// void timeout_handler(int signum) {
//     exit(1);  // or set a flag
// }
void handleGetRequest(client_info &client, std::map<int, server_config> &server)
{
    std::cout << "in get funciton" << std::endl;
    client.response.clear();
    std::string content_type = "";
    // bool is_php = false;
    // int server_index = findMatchingServer(client, server);

    // std::cout << "server path: " << server[server_index].path << std::endl;
    // std::cout << "client path: " << client.uri << std::endl;
    // std::string loc = getlocation(client, server[server_index]);
    std::string body;
    long content_size = -1;
    // FILE *fp;

    std::string path = getcorectserver_path(client, server) + client.uri;

    // std::cout << "path: " << path << std::endl;
    if(path.find_last_of("/") == path.size() - 1)
    {
        path += "index.html";
        // std::cout << "path: " << path << std::endl;
    }
    // try to open the file
    bool whith_header = 0;
    std::ifstream file(path.c_str());
    //get the file size cpp 98
    
    // if (!file.is_open())
    // {
        // std::cout << "file not open" << std::endl;
        // file.open(path.c_str());
        // whith_header = 1;
        // client.datafinished = 0;
        // client.bytes_sent = 0;
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
        // success(client,body, path, whith_header);
        // std::cout << "file open" << std::endl;
        // return;
    // }
    if(path.find_last_of(".") != std::string::npos)
    {
        std::cout << "in php file" << std::endl;
        if(path.substr(path.find_last_of(".") + 1) == "php")
        {
            int fd[2];
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
                // std::string command = "/Users/mkibous/Desktop/webserver/www/php-cgi " + path;
                // std::cout << "command: " << command << std::endl;
                char *args[] = {(char *)"/Users/mkibous/Desktop/webserver/www/php-cgi", (char *)path.c_str(), NULL};
                // std::cout << "in php file2" << std::endl;
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
                std::string headers;
                //readline by line until we find \r\n\r\n
                while ((bytes_read = read(fd[0], buffer, sizeof(buffer))) > 0)
                {
                    headers += std::string(buffer, bytes_read);
                    if (headers.find("\r\n\r\n") != std::string::npos){
                        //remove what is after \r\n\r\n
                        size_t pos = headers.find("\r\n\r\n");
                        //calculate the size of what is after \r\n\r\n
                        size_t size = headers.size() - pos - 4;
                        body = headers.substr(pos + 4, size);
                        headers = headers.substr(0, pos + 4);
                        break;}
                    
                }
                if(client.bytes_sent <= 0 && client.bytes_sent != -1 )
                {
                    while ((bytes_read = read(fd[0], buffer, sizeof(buffer))) > 0){
                        body+= std::string(buffer, bytes_read);
                        
                    }
                    if(headers.size() > 0)
                        content_type = headers.substr(headers.find("Content-Type: ") + 14, headers.find("\r\n", headers.find("Content-Type: ")) - headers.find("Content-Type: ") - 14);
                    // if(content_type == "")
                    //     content_type = "text/html";
                    //openfile ant write to it
                    // std::ofstream body_file("body.txt");
                    // if (body_file.is_open())
                    // {
                    //     body_file << body;
                    //     body_file.close();
                    // }
                    wait(NULL);
                    // exit(0);
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
                {
                    body += std::string(buffer, bytes_read);
                    // std::cout << "body size: " << body.size() << std::endl;
                    
                }
                if(body.size() >= client.bytes_sent)
                {
                    std::string temp = body.substr(client.bytes_sent, body.size() - client.bytes_sent);
                    body = temp;
                    // std::cout << "body size: " << body.size() << std::endl;
                }
                while (body.size() < READ_BUFFER_SIZE && (bytes_read = read(fd[0], buffer, sizeof(buffer))) > 0)
                {
                    body += std::string(buffer, bytes_read);
                    // std::cout << "body size: " << body.size() << std::endl;
                    
                }
                if(bytes_read == 0)
                {
                    std::cout << "file end reached" << std::endl;
                    client.datafinished = 1;
                }
                // wait(NULL);
                alarm(0);
                close(fd[0]);
                success(client, body, path, content_type, false);
                return;
            }
        }
        
    }
    if(client.bytes_sent <= 0 && client.bytes_sent != -1 )
    {
        success(client, body, path, content_type, true, content_size);
        return;
    }
    else if(client.bytes_sent == -1)
        client.bytes_sent = 0;
    // if(is_php)
    // {
    //     std::cout << "in php file3" << std::endl;
    //     std::string command = "/Users/mkibous/Desktop/webserver/www/php-cgi " + path;
    //     fp = popen(command.c_str(), "r");
    //     if (fp == NULL)
    //     {
    //         std::cerr << "Failed to run command: " << command << std::endl;
    //         return;
    //     }
    //     char buffer[128];
    //     //skip headers
    //     std::string headers;
    //     while (fread(buffer, 1, sizeof(buffer), fp) > 0)
    //     {
    //         headers += buffer;
    //         if (headers.find("\r\n\r\n") != std::string::npos)
    //             break;
    //     }
    //     // std::cout << "bytes sent: " << client.bytes_sent << std::endl;
    //     while(fread(buffer, 1, sizeof(buffer), fp) > 0 && body.size() < client.bytes_sent)
    //     {
    //         body += buffer;
    //         // std::cout << "body size: " << body.size() << std::endl;
    //     }
    //     std::cout <<"bytes sent: " << client.bytes_sent << std::endl;
    //     body.clear();
    //     std::cout << "in php file4" << std::endl;
    //     while(fread(buffer, 1, sizeof(buffer), fp) > 0 && body.size() < READ_BUFFER_SIZE)
    //     {
    //         body += buffer;
    //         // std::cout << "body size: " << body.size() << std::endl;
    //     }
    //     // std::cout << "in php file5" << std::endl;
    //     // std::ofstream body_file("body.txt");
    //     // if (body_file.is_open())
    //     // {
    //     //     body_file << body;
    //     //     body_file.close();
    //     // }
    //     std::cout << "body size: " << body.size() << std::endl;
    //     // exit(0);
    //     // if (feof(fp))
    //     //     client.datafinished = 1, std::cout << "file end reached" << std::endl;
    //     //check if file end is reached
    //     if(feof(fp))
    //     {
    //         std::cout << "file end reached" << std::endl;
    //         client.datafinished = 1;
    //     }
    //     success(client, body, path, content_type, false);
    //     pclose(fp);
    //     // exit(0);
    //     return;
    // }
    char *buffer = new char[READ_BUFFER_SIZE];
    // read only 1024 bytes
    file.seekg(0, std::ios::beg);                 // move to the beginning
    file.seekg(client.bytes_sent, std::ios::beg); // move to byte  from the beginning
    // std::cout << "bytes sent: " << client.bytes_sent << std::endl;
    file.read(buffer, READ_BUFFER_SIZE);
    if (file.eof())
    {
        file.close(), client.datafinished = 1, std::cout << "file end reached" << std::endl;
    }
    body = std::string(buffer, file.gcount());
    // check if file end is reached
    delete[] buffer;
    // file.seekg(0, std::ios::beg); to go to the beginning
    // file.seekg(-400, std::ios::cur); to go back 400 bytes

    success(client, body, path, content_type, whith_header);
    // file.close();

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