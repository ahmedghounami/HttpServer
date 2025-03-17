#include "../server.hpp"

void handleGetRequest(client_info &client, std::map<int, server_config> &server)
{
    (void)server;
    // std::cout << client.boundary << std::endl;
    std::cout << client.method << std::endl;
    std::cout << client.uri << std::endl;
    std::cout << client.version << std::endl;
}