#include "server.hpp"

int main()
{
    try
    {
        server obj;
        obj.listen_for_connections();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    return 0;

}
