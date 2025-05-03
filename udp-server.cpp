#include "socket-wrappers.h"
#include <cstddef>
#include <iostream>

const size_t MAX_BUFFER = 9999;

int main(int argc, char *argv[]) {
  if (argc < 3) {
    std::cerr << "Usage: " << argv[0] << " <ip_address> <port> ";
    return 1;
  }

  UDPSocket server(argv[1], std::atoi(argv[2]));
  while (1) {
    auto data = server.receive_string(MAX_BUFFER);
    std::cout << '[' << data.sender << "]: " << data.message << '\n';
  }
  return 0;
}
