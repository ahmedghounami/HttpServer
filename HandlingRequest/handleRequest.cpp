/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handleRequest.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mkibous <mkibous@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/19 22:39:03 by hboudar           #+#    #+#             */
/*   Updated: 2025/04/15 14:22:38 by mkibous          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../server.hpp"
// Caching Considerations:
//  Because GET requests are cacheable, the caching behavior is defined
//  in related RFCs (e.g., RFC 7234). This means that responses to GET
//  requests can be stored and reused under the right conditions.

//finding which server config to use
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

    //print request line
    // std::cout << "Request Line: " << client.method << " " << client.uri << " " << client.version << std::endl;
    //print query
    // if (!client.query.empty())
    //     std::cout << "Query: " << client.query << std::endl;
    //print headers whitout for each
    // print client info
    // std::cout << "Client Info: " << std::endl;
    // std::cout << "last_time: " << client.last_time << std::endl;
    // std::cout << "method: " << client.method << std::endl;
    // std::cout << "uri: " << client.uri << std::endl;
    // std::cout << "version: " << client.version << std::endl;
    // std::cout << "query: " << client.query << std::endl;
    // std::cout << "Headers: " << std::endl;
    //fine the server config
    findMatchingServer(client, server);
    (void)client;
    (void)server;
}

void handleDeleteRequest(client_info &client, std::map<int, server_config> &server) {
    std::cout << "in delete funciton" << std::endl;
    (void)client;
    (void)server;
}
