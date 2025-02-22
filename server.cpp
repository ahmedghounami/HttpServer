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

    // while (true)
    // {
    // // Wait for an event on the server socket (accepting new client connection)
    // std::cout << "Waiting for client connection\n";
    // int ret = poll(&clients_fds[0], clients_fds.size(), 5000); // Timeout set to 5 seconds
    // std::cout << "a client connected\n";
    // if (ret < 0)
    // {
    //     std::cerr << "Poll failed\n";
    //     break;
    // }
    // else if (ret == 0)
    // {
    //     // Timeout, no events on the server socket
    //     continue;
    // }

    // // If there is an event on the server socket
    // if (fds[0].revents & POLLIN)
    // {
    //     sockaddr_in client_addr;
    //     socklen_t client_len = sizeof(client_addr);
    //     int client_sock = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
    //     if (client_sock < 0)
    //     {
    //         std::cerr << "Accept failed\n";
    //         continue;
    //     }

    //     char buffer[1024];
    //     std::string request;
    //     int data = 0;

    //     // Poll on the client socket to read data
    //     struct pollfd client_fds[1];
    //     client_fds[0].fd = client_sock;
    //     client_fds[0].events = POLLIN; // Monitor the client socket for reading
    //     while (true)
    // int fd = 0;
    int flag = 0;
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

        if (flag == 0)
        {
            std::cout << "new connection" << std::endl;
            sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            int client_sock = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
            if (client_sock < 0)
                continue;

            struct pollfd newfd;
            newfd.fd = client_sock;
            newfd.events = POLLIN;

            clients_fds.push_back(newfd);
            flag = 1;
        }

        if (clients_fds.size() > 1 && clients_fds[1].revents & POLLIN)
        {
            char buffer[1024];
            std::string request;
            int data = recv(clients_fds[1].fd, buffer, sizeof(buffer) - 1, 0);
            std::cout << buffer << std::endl;
        }
        // for (int i = 0; i < clients_fds.size(); i++)
        // {

        //     if (clients_fds[i].revents & POLLIN)
        //     {
        //         fd = clients_fds[i].fd;
        //         if (fd == server_fd)
        //         {
        //             sockaddr_in client_addr;
        //             socklen_t client_len = sizeof(client_addr);
        //             int client_sock = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        //             if (client_sock < 0)
        //                 continue;
        //             pollfd client_fd;
        //             client_fd.fd = client_sock;
        //             client_fd.events = POLLIN;
        //             clients_fds.push_back(client_fd);
        //             clients[client_sock] = client_info();
        //         }
        //         else
        //         {
        //             char buffer[1024];
        //             int data = 0;
        //             data = recv(fd, buffer, sizeof(buffer) - 1, 0);
        //             if (data <= 0)
        //             {
        //                 std::cerr << "Client disconnected or error\n";
        //                 continue;
        //             }
        //             buffer[data] = '\0';
        //             clients[fd].chunk.append(buffer, data);
        //             // std::cout << buffer << std::endl;
        //         }
        //     }
        // }
    }
    // }

    // // Close the client socket
    // close(client_sock);
    // }

    // Close the server socket
    close(server_fd);
    close(clients_fds[1].fd);

    return 0;
}
