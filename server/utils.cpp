#include "../server.hpp"

std::string ipToString(in_addr_t addr)
{
    unsigned char bytes[4];
    bytes[0] = addr & 0xFF;
    bytes[1] = (addr >> 8) & 0xFF;
    bytes[2] = (addr >> 16) & 0xFF;
    bytes[3] = (addr >> 24) & 0xFF;

    std::stringstream ss;
    ss << (int)bytes[0] << "."
       << (int)bytes[1] << "."
       << (int)bytes[2] << "."
       << (int)bytes[3];

    return ss.str();
}

void accept_connection(int sock_connection, std::vector<pollfd> &clients_fds, std::map<int, client_info> &clients)
{
    sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_sock = accept(sock_connection, (struct sockaddr *)&client_addr, &client_len);
    std::cout << "new connection---------------: " << client_sock << std::endl;
    if (client_sock < 0)
        return;
    struct pollfd newfd;
    newfd.fd = client_sock;
    newfd.events = POLLIN;

    clients_fds.push_back(newfd);
    clients[client_sock] = client_info();
    clients[client_sock].last_time = time(NULL);
    clients[client_sock].poll_status = 0;
    clients[client_sock].bytes_sent = 0;
    clients[client_sock].datafinished = false;
    std::string ip = ipToString(client_addr.sin_addr.s_addr);
    clients[client_sock].ip = ip;
}
