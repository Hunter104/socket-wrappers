#include "socket-wrappers.h"
#include <iostream>

int main(int argc, char *argv[]) {
  if (argc < 3) {
    std::cerr << "Usage: " << argv[0]
              << "<server_ip_address> <server_port> <data_1> <data_2> ...";
    return 1;
  }

  UDPSocket client;
  Endpoint server(argv[1], std::atoi(argv[2]));

  for (int i = 3; i < argc; i++) {
    size_t bytes = client.send(std::string(argv[i]), server);
    std::cout << bytes << " bytes sent." << '\n';
  }
  return 0;
}
