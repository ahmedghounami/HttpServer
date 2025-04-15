#include "../server.hpp"

void check_ports(int port, std::string server_name, std::vector<port_used> &ports_used)
{
    for (unsigned int i = 0; i < ports_used.size(); i++)
    {
        if (ports_used[i].port == port && ports_used[i].server_name == server_name)
            throw std::runtime_error("Port already in use");
    }
}

server::server(std::string &config_file)
{
    // signal(SIGPIPE, SIG_IGN);
    parse_config(config_file);
    for (unsigned int i = 0; i < servers.size(); i++)
    {
        for (unsigned int j = 0; j < servers[i].ports.size(); j++)
        {
            // check_ports(servers[i].ports[j], servers[i].server_names, ports_used);
            start_connection = socket(AF_INET, SOCK_STREAM, 0);
            listners.push_back(start_connection);
            if (start_connection == -1)
                throw std::runtime_error("Socket creation failed");

            sockaddr_in server_addr;
            server_addr.sin_family = AF_INET;
            server_addr.sin_port = htons(servers[i].ports[j]);
            server_addr.sin_addr.s_addr = INADDR_ANY;
            inet_pton(AF_INET, servers[i].host.c_str(), &server_addr.sin_addr);
            
            int opt = 1;
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

            // port_used port;
            // port.port = servers[i].ports[j];
            // port.server_name = servers[i].server_name;
            // ports_used.push_back(port);

            std::cout << "Server started on port " << servers[i].ports[j] << std::endl;
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
            std::cerr << "Client timeout " << clients_fds[i].fd << std::endl;
            close(clients_fds[i].fd);
            clients_fds.erase(clients_fds.begin() + i);
            i--;
        }
    }
}

void server::listen_for_connections()
{

    ///////////////////////////////////////////////
    // std::string filename = "data";
    // std::ofstream file(filename);
    // if (file.good())
    //     std::cerr << "File opened successfully\n";
    // else
    // {
    //     std::cerr << "File open failed\n";
    //     throw std::runtime_error("File open failed");
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
        check_timeout(clients_fds, clients);
        if (ret == 0)
        {
            std::cerr << "No data, timeout\n";
            continue;
        }
        for (unsigned int i = 0; i < clients_fds.size(); i++)
        {

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
                    // clients_fds[i].events = POLLOUT;
                    parse_chunk(clients[clients_fds[i].fd], servers);
                }
            }
            if (clients[clients_fds[i].fd].poll_status == 1)
                clients_fds[i].events = POLLOUT;
            if (clients_fds[i].revents & POLLOUT)
            {
                std::cerr << "Sending response to client " << clients_fds[i].fd << std::endl;
                ssize_t bytes = send(clients_fds[i].fd, clients[clients_fds[i].fd].response.c_str(), clients[clients_fds[i].fd].response.size(), 0);
                if (bytes < 0)
                    continue;
                usleep(1000);
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
