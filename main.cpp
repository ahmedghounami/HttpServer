#include "server.hpp"

int main(int argc, char *argv[])
{
    signal(SIGPIPE, SIG_IGN);
    try
    {
        if (argc != 2)
            throw std::runtime_error("Usage: ./server <config_file.conf>");
        int len = strlen(argv[1]);
        if (len < 6 || argv[1][len - 1] != 'f' || argv[1][len - 2] != 'n' || argv[1][len - 3] != 'o' || argv[1][len - 4] != 'c' || argv[1][len - 5] != '.')
            throw std::runtime_error("Invalid config file");
        std::ifstream file(argv[1]);
        if (!file.is_open())
            throw std::runtime_error("Config file could not be opened");
        std::string config_file_name = argv[1];
        server obj(config_file_name);
        obj.listen_for_connections();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
    return 0;
}
