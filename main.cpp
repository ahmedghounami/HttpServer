#include "server.hpp"

int main(int argc, char *argv[])
{
    signal(SIGPIPE, SIG_IGN);
    try
    {
        std::string config_file_name;
        if (argc > 2)
            throw std::runtime_error("Usage: ./server <config_file.conf>");
        if (argc == 1)
            config_file_name = "config/default.conf";
        else
            config_file_name = argv[1];
        server obj(config_file_name);
        obj.listen_for_connections();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
    return 0;
}
