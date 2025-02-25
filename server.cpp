#include "server.hpp"

int main()
{
    std::map<int, client_info> clients;
    int start_connection = socket(AF_INET, SOCK_STREAM, 0);
    if (start_connection == -1)
    {
        std::cerr << "Socket creation failed\n";
        return 1;
    }

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    int opt = 1;
    setsockopt(start_connection, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(start_connection, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        std::cerr << "Bind failed\n";
        return 1;
    }

    if (listen(start_connection, 128) < 0)
    {
        std::cerr << "Listen failed\n";
        return 1;
    }

    std::cout << "Server running on port " << PORT << "\n";

    struct pollfd server_fd;
    server_fd.fd = start_connection;
    server_fd.events = POLLIN;

    std::vector<pollfd> clients_fds;

    clients_fds.push_back(server_fd);

    ///////////////////////////////////////////////
    std::string filename = "file.jpg";
    std::ofstream file(filename);
    if (file.good())
    {
        std::cerr << "File opened successfully\n";
    }
    else
    {
        std::cerr << "File open failed\n";
        return 1;
    }
    // std::string filename2 = "file.mp4";
    // std::ofstream file2(filename2);
    // if (file2.good())
    // {
    //     std::cerr << "File opened successfully\n";
    // }
    // else
    // {
    //     std::cerr << "File open failed\n";
    //     return 1;
    // }
    ///////////////////////////////////////////////

    
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
            accept_connection(start_connection, clients_fds, clients);

        for (int i = 1; i < clients_fds.size(); i++)
        {
            if (clients_fds[i].revents & POLLIN)
            {
                char buffer[1024];
                int data = recv(clients_fds[i].fd, buffer, sizeof(buffer) - 1, 0);

                if (data < 0)
                    continue;

                buffer[data] = '\0';
                clients[clients_fds[i].fd].chunk.append(buffer, data);

                if (clients[clients_fds[i].fd].chunk.find("\r\n\r\n") != std::string::npos)
                    get_boundary(clients_fds[i].fd, clients);
                std::string boundary_end = clients[clients_fds[i].fd].boundary + "--";
                size_t pos = clients[clients_fds[i].fd].chunk.find(boundary_end);
                if (pos != std::string::npos)
                {
                    clients[clients_fds[i].fd].chunk = clients[clients_fds[i].fd].chunk.substr(0, pos - 4);
                    file << clients[clients_fds[i].fd].chunk;
                    file.close();
                }
                else
                    file << clients[clients_fds[i].fd].chunk;
                clients[clients_fds[i].fd].chunk.clear();
            }
        }
    }
    for (int i = 1; i < clients_fds.size(); i++)
        close(clients_fds[i].fd);
    return 0;
}
