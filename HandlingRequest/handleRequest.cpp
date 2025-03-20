/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handleRequest.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hboudar <hboudar@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/19 22:39:03 by hboudar           #+#    #+#             */
/*   Updated: 2025/03/19 22:51:24 by hboudar          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../server.hpp"

void handleGetRequest(client_info &client, std::map<int, server_config> &server) {
    std::cout << "in get funciton" << std::endl;
    (void)client;
    (void)server;
}

void handleDeleteRequest(client_info &client, std::map<int, server_config> &server) {
    std::cout << "in delete funciton" << std::endl;
    (void)client;
    (void)server;
}

void handlePostRequest(client_info &client, std::map<int, server_config> &server) {
    std::cout << "in post funciton" << std::endl;
    (void)client;
    (void)server;
}