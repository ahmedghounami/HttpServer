#include "server.hpp"

server::server(std::string &config_file)
{
    parse_config(config_file);
    for (unsigned int i = 0; i < config.ports.size(); i++)
    {
        start_connection = socket(AF_INET, SOCK_STREAM, 0);
        listners.push_back(start_connection);
        if (start_connection == -1)
            throw std::runtime_error("Socket creation failed");

        sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(config.ports[i]);
        server_addr.sin_addr.s_addr = INADDR_ANY;

        int opt = 1;
        setsockopt(start_connection, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        setsockopt(start_connection, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

        if (bind(start_connection, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
            throw std::runtime_error("Bind failed");

        if (listen(start_connection, 128) < 0) // 128 is the maximum number of connections that can be waiting
            throw std::runtime_error("Listen failed");

        struct pollfd server_fd;
        server_fd.fd = start_connection;
        server_fd.events = POLLIN;
        clients_fds.push_back(server_fd);

        std::cout << "Server started on port " << config.ports[i] << std::endl;
    }
}

void server::listen_for_connections()
{

    ///////////////////////////////////////////////
    std::string filename = "data";
    std::ofstream file(filename);
    if (file.good())
        std::cerr << "File opened successfully\n";
    else
    {
        std::cerr << "File open failed\n";
        throw std::runtime_error("File open failed");
    }
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
        for (unsigned int i = 0; i < clients_fds.size(); i++)
        {

            // std::vector<pollfd>::iterator it = find(clients_fds.begin(), clients_fds.end(), clients_fds[i]);
            if (clients_fds[i].revents & POLLIN)
            {
                if (std::find(listners.begin(), listners.end(), clients_fds[i].fd) != listners.end())
                    accept_connection(clients_fds[i].fd, clients_fds, clients);
                else
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
                        get_chunk(clients[clients_fds[i].fd], file, pos, 1);
                        clients_fds[i].events = POLLOUT;
                    }
                    else
                        get_chunk(clients[clients_fds[i].fd], file, 0, 0);
                    clients[clients_fds[i].fd].chunk.clear();
                }
            }

            if (clients_fds[i].revents & POLLOUT)
            {
                std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><h1>File uploaded successfully</h1></body></html>";
                ssize_t bytes = send(clients_fds[i].fd, response.c_str(), response.length(), 0);
                if (bytes < 0)
                    continue;
                close(clients_fds[i].fd);
                clients_fds.erase(clients_fds.begin() + i);
                i--;
            }
        }
    }
}

server::~server()
{
    for (unsigned int i = 0; i < clients_fds.size(); i++)
        close(clients_fds[i].fd);
}
