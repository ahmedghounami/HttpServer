#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <fstream>
#include <vector>
#include <map>

struct client_info
{
    std::string chunk;
};

std::map<int, client_info> clients;

#define PORT 8080

int main()
{
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
    {
        std::cerr << "Socket creation failed\n";
        return 1;
    }

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        std::cerr << "Bind failed\n";
        return 1;
    }

    if (listen(server_fd, 128) < 0)
    {
        std::cerr << "Listen failed\n";
        return 1;
    }

    std::cout << "Server running on port " << PORT << "\n";

    struct pollfd fd;
    fd.fd = server_fd;
    fd.events = POLLIN;

    std::vector<pollfd> clients_fds;

    clients_fds.push_back(fd);

    while (true)
    {
        int ret = poll(&clients_fds[0], clients_fds.size(), 5000);
        if (ret < 0)
        {
            std::cerr << "Poll failed\n";
            continue;
        }
        else if (ret == 0)
        {
            std::cerr << "No data, timeout\n";
            continue;
        }

        if (clients_fds[0].revents & POLLIN)
        {
            sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            int client_sock = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
            std::cout << "new connection---------------: " << client_sock << std::endl;
            sleep(1);
            if (client_sock < 0)
                continue;

            struct pollfd newfd;
            newfd.fd = client_sock;
            newfd.events = POLLIN;

            clients_fds.push_back(newfd);
            clients[client_sock] = client_info();
        }

        for (int i = 1; i < clients_fds.size(); i++)
        {
            if (clients_fds[i].revents & POLLIN)
            {
                char buffer[1024];
                int data = 0;
                data = recv(clients_fds[i].fd, buffer, sizeof(buffer) - 1, 0);
                if (data <= 0)
                    continue;
                buffer[data] = '\0';
                clients[clients_fds[i].fd].chunk += buffer;
                std::cout << buffer << std::endl;
            }
        }
    }
    for (int i = 1; i < clients_fds.size(); i++)
        close(clients_fds[i].fd);
    return 0;
}