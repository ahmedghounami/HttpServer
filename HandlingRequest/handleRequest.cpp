/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handleRequest.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mkibous <mkibous@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/19 22:39:03 by hboudar           #+#    #+#             */
/*   Updated: 2025/04/17 19:00:29 by mkibous          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../server.hpp"
// Caching Considerations:
//  Because GET requests are cacheable, the caching behavior is defined
//  in related RFCs (e.g., RFC 7234). This means that responses to GET
//  requests can be stored and reused under the right conditions.
std::string getContentType(const std::string& path) {
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

int findMatchingServer(client_info &client, std::map<int, server_config> &server){
    std::string host;
    int port;
    int server_index = -1;

    for (std::map<std::string, std::string>::iterator it = client.headers.begin(); it != client.headers.end(); ++it) {
        // std::cout << it->first << ": " << it->second << std::endl;
        if(it->first == "host") {
            std::istringstream iss(it->second);
            std::getline(iss, host, ':');
            iss >> port;
            
            // std::cout << "Host: " << host << std::endl;
            // std::cout << "Port: " << port << std::endl;
            
            for (std::map<int, server_config>::iterator it = server.begin(); it != server.end(); ++it) {
                if(std::find(it->second.ports.begin(), it->second.ports.end(), port) != it->second.ports.end() && server_index == -1)
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
void success(client_info &client, std::string &body, std::string &path, bool whith_header = true) {
    
    client.poll_status = 1;
    if(whith_header)
    {
        client.response = "HTTP/1.1 200 OK\r\n";
        client.response += "Content-Type: " + getContentType(path) + "\r\n";
        client.response += "Content-Length: " + std::to_string(body.size()) + "\r\n";
        client.response += "Connection: close\r\n";
        client.response += "\r\n";
    }
    client.response += body;
}
std::string getlocation(client_info &client, server_config &server) {
    std::cout << "in getlocation funciton" << std::endl;
    //finde last / in uri
    std::string uri = client.uri;
    while(uri.size() > 0)
    {
        size_t pos = uri.find_last_of("/");
        if(pos == std::string::npos)
            break;
        if(pos == 0 && uri.size() > 1)
            pos++;
        uri = uri.substr(0, pos);
        std::cout << "location: " << uri << std::endl;
        std::map<std::string, location>::iterator it = server.locations.find(uri);
        if(it != server.locations.end())
        {
            // std::cout << "found location: " << it->first << std::endl;
            return it->first;
        }
    }
    
    return "";
}
std::string getcorectserver_path(client_info &client, std::map<int, server_config> &server) {
    std::cout << "in getcorect_path funciton" << std::endl;
    int server_index = findMatchingServer(client, server);
    
    std::string loc = getlocation(client, server[server_index]);
    if(loc != "" && server[server_index].locations[loc].path != "")
        return server[server_index].locations[loc].path;
    return server[server_index].path;
}
void handleGetRequest(client_info &client, std::map<int, server_config> &server) {
    std::cout << "in get funciton" << std::endl;

    
    // int server_index = findMatchingServer(client, server);
    
    // std::cout << "server path: " << server[server_index].path << std::endl;
    // std::cout << "client path: " << client.uri << std::endl;
    // std::string loc = getlocation(client, server[server_index]);
    
    std::string path = getcorectserver_path(client, server) + client.uri;
    
    std::cout << "path: " << path << std::endl;
    
    //try to open the file
    std::ifstream file(path.c_str());
    if(!file.is_open())
    {
        file.open(path.c_str());
        if(!file.is_open())
        {
            switch(errno)
            {
                case ENOENT:
                    not_found(client);//404
                    break;
                case EACCES:
                    forbidden(client);//403
                    break;
                default:
                    unknown_error(client);//500
                    break;
            }
            return;
        }
    }
    char *buffer = new char[READ_BUFFER_SIZE];
    //read only 1024 bytes
    file.seekg(client.bytes_sent, std::ios::beg);  // move to byte  from the beginning
    file.read(buffer, READ_BUFFER_SIZE);
    std::string body = std::string(buffer, file.gcount());
    std::cout << "body size: " << body.size() << std::endl;
    //check if file end is reached
    if(file.eof())
        file.close(), client.datafinished = 1, std::cout << "file end reached" << std::endl;
    delete[] buffer;
    //file.seekg(0, std::ios::beg); to go to the beginning
    //file.seekg(-400, std::ios::cur); to go back 400 bytes
    
    success(client, body, path);
    // file.close();

    std::cout << "get function finished!" << std::endl;
}

void handleDeleteRequest(client_info &client, std::map<int, server_config> &server) {
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