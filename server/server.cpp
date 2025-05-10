#include "../server.hpp"

server::server(std::string &config_file)
{
    signal(SIGPIPE, SIG_IGN);
    parse_config(config_file);
    for (unsigned int i = 0; i < servers.size(); i++)
    {
        for (unsigned int j = 0; j < servers[i].ports.size(); j++)
        {
            start_connection = socket(AF_INET, SOCK_STREAM, 0);
            listners.push_back(start_connection);
            if (start_connection == -1)
                throw std::runtime_error("Socket creation failed");

            sockaddr_in server_addr;
            server_addr.sin_family = AF_INET;
            server_addr.sin_port = htons(servers[i].ports[j]);
            server_addr.sin_addr.s_addr = INADDR_ANY;
            if (inet_pton(AF_INET, servers[i].host.c_str(), &server_addr.sin_addr) <= 0 && 
                servers[i].host != "localhost")
            {
                std::cerr << "Invalid address/ Address not supported" << std::endl;
                throw std::runtime_error("Invalid address");
            }
            
            int opt = 1;
            // 1 to enable, 0 to disable
            setsockopt(start_connection, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
            setsockopt(start_connection, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

            // non blocking mode
            int flags = fcntl(start_connection, F_GETFL, 0);
            if (flags < 0)
                throw std::runtime_error("Fcntl failed");
            if (fcntl(start_connection, F_SETFL, flags | O_NONBLOCK) < 0)
                throw std::runtime_error("Fcntl failed");
            // -------------------------------------

            if (bind(start_connection, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
                throw std::runtime_error("Bind failed");

            if (listen(start_connection, SOMAXCONN) < 0) 
                throw std::runtime_error("Listen failed");

            struct pollfd server_fd;
            server_fd.fd = start_connection;
            server_fd.events = POLLIN;
            clients_fds.push_back(server_fd);
        }
    }
}

void server::check_timeout(std::vector<pollfd> &clients_fds, std::map<int, client_info> &clients)
{
    time_t current_time = time(NULL);
    for (unsigned int i = 0; i < clients_fds.size(); i++)
    {
        if (std::find(listners.begin(), listners.end(), clients_fds[i].fd) == listners.end() && \
             current_time - clients[clients_fds[i].fd].last_time > 15)
        {
            std::cerr << "Client timeout :" << clients_fds[i].fd << std::endl;
            close(clients_fds[i].fd);
            clients_fds.erase(clients_fds.begin() + i);
            i--;
        }
    }
}

void server::listen_for_connections()
{
    while (true)
    {
        int ret = poll(&clients_fds[0], clients_fds.size(), 5000);
        if (ret < 0)
        {
            std::cerr << "Poll failed\n";
            continue;
        }
        check_timeout(clients_fds, clients);
        if (ret == 0)
            continue;
        for (unsigned int i = 0; i < clients_fds.size(); i++)
        {
            if (clients_fds[i].revents & POLLIN)
            {
                if (std::find(listners.begin(), listners.end(), clients_fds[i].fd) != listners.end())
                    accept_connection(clients_fds[i].fd, clients_fds, clients);
                else
                {
                    char buffer[READ_BUFFER_SIZE];
                    int data = recv(clients_fds[i].fd, buffer, sizeof(buffer) - 1, 0);

                    if (data < 0)
                        continue;
                    if (data == 0)
                    {
                        close(clients_fds[i].fd);
                        clients_fds.erase(clients_fds.begin() + i);
                        i--;
                        continue;
                    }
                    clients[clients_fds[i].fd].last_time = time(NULL);
                    buffer[data] = '\0';
                    clients[clients_fds[i].fd].data.append(buffer, data);
                    ParseChunk(clients[clients_fds[i].fd], servers);
                    if (clients[clients_fds[i].fd].isGet == true)
                        clients_fds[i].events = POLLOUT;
                }
            }
            if (clients[clients_fds[i].fd].poll_status == 1)
                clients_fds[i].events = POLLOUT;
            if (clients_fds[i].revents & POLLOUT)
            {
                if (clients[clients_fds[i].fd].isGet == true)
                    handleGetRequest(clients[clients_fds[i].fd], servers);
                int bytes_sent = send(clients_fds[i].fd, clients[clients_fds[i].fd].response.c_str(), clients[clients_fds[i].fd].response.size(), 0);
                clients[clients_fds[i].fd].bytes_sent += bytes_sent;
                if (clients[clients_fds[i].fd].bytes_sent < 0)
                    continue;
                usleep(1000);
                if (clients[clients_fds[i].fd].datafinished == true)
                {
                    close(clients_fds[i].fd);
                    clients_fds.erase(clients_fds.begin() + i);
                    i--;
                }
                else
                    clients[clients_fds[i].fd].response.clear();
            }
        }
    }
}

server::~server()
{
    for (unsigned int i = 0; i < clients_fds.size(); i++)
        close(clients_fds[i].fd);
}
