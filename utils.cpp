#include "server.hpp"

void get_boundary(int _client_fd, std::map<int, client_info> &clients)
{
    std::string boundary_start = "boundary=";
    size_t start_pos = clients[_client_fd].chunk.find(boundary_start);
    size_t end_pos = clients[_client_fd].chunk.find("\r\n", start_pos);
    clients[_client_fd].boundary = clients[_client_fd].chunk.substr(start_pos + boundary_start.length(), end_pos - start_pos - boundary_start.length());
    size_t pos = clients[_client_fd].chunk.find("\r\n\r\n");
    pos = clients[_client_fd].chunk.find("\r\n\r\n", pos + 4);
    std::string header = clients[_client_fd].chunk.substr(0, clients[_client_fd].chunk.find(pos));
    clients[_client_fd].chunk = clients[_client_fd].chunk.substr(pos + 4);
    clients[_client_fd].header = header;
    // std::cout << "Boundary: " << clients[_client_fd].boundary << std::endl;
    // std::cout <<"========================================" << std::endl;
    // std::cout << "Header: " << clients[_client_fd].header << std::endl;
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
}

void get_chunk(client_info &client, std::ofstream &file, size_t pos, int flag)
{
    if (flag == 1)
    {
        client.chunk = client.chunk.substr(0, pos - 4);
        file << client.chunk;
        file.close();
        std::cout << "File closed\n";
    }
    else
        file << client.chunk;
}
