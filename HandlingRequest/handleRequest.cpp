/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handleRequest.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mkibous <mkibous@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/19 22:39:03 by hboudar           #+#    #+#             */
/*   Updated: 2025/04/15 16:10:31 by mkibous          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../server.hpp"
// Caching Considerations:
//  Because GET requests are cacheable, the caching behavior is defined
//  in related RFCs (e.g., RFC 7234). This means that responses to GET
//  requests can be stored and reused under the right conditions.

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
void handleGetRequest(client_info &client, std::map<int, server_config> &server) {
    std::cout << "in get funciton" << std::endl;

    
    int server_index = findMatchingServer(client, server);
    
    // std::cout << "server path: " << server[server_index].path << std::endl;
    // std::cout << "client path: " << client.uri << std::endl;
    
    std::string path = server[server_index].path + client.uri;
    
    
    std::cout << "content type: " << client.ContentType << std::endl;
    
    //try to open the file
    std::ifstream file(path.c_str());
    int status_code = 200;
    if(!file.is_open()) {
        switch(errno) {
            case ENOENT:
                status_code = 404;
                break;
            case EACCES:
                status_code = 403;
                break;
            default:
                status_code = 500;
                break;
        }
    }
    std::string notFoundBody = "<h1>404 Not Found</h1>";
    std::string response = 
    "HTTP/1.1 404 Not Found\r\n"
    "Content-Type: text/plain\r\n"
    "Content-Length: " + std::to_string(notFoundBody.size()) + "\r\n"
    "\r\n" +
    notFoundBody;
    
    client.response = response;
    client.poll_status = 1;
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

// HTTP/1.1 404 Not Found
// Content-Type: text/html
// Content-Length: 25

// <h1>404 Not Found</h1>
// 404	Not Found	                        File or resource doesn’t exist
// 403	Forbidden	                        You’re not allowed to access the file
// 400	Bad Request	                        The request is malformed (wrong syntax)
// 405	Method Not Allowed	                GET is not allowed for that route
// 500	Internal Server Error	            Something broke inside the server
// 502	Bad Gateway	                        Server acting as a gateway got an invalid response
// 503	Service Unavailable	                Server is temporarily overloaded or down



// html, .htm	                    text/html
// .css	                            text/css
// .js	                            application/javascript
// .json	                        application/json
// .png	                            image/png
// .jpg, .jpeg	                    image/jpeg
// .txt	                            text/plain
// .sh	                            text/x-shellscript or text/plain
// .pdf	                             application/pdf