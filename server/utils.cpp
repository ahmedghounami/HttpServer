#include "../server.hpp"

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
}
