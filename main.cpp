#include "server.hpp"

int main() {
  try {
    server obj;
    obj.listen_for_connections();
    std::cout << "************* Here where we start *****************"
              << std::endl;
  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
  }
  return 0;
}
