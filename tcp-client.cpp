#include "socket-wrappers.h"
#include <cstdlib>
#include <iostream>
#include <string>
int main(int argc, char *argv[]) {
  if (argc < 3) {
    std::cerr << "Usage: " << argv[0] << "<server_ip> <port>\n";
    return 1;
  }

  TCPSocket client;
  client.connect(Endpoint(argv[1], std::atoi(argv[2])));

  while (1) {
    std::cout << "> ";
    std::string input;
    std::getline(std::cin, input);

    client.send(input);
    if (input == "END")
      break;
  }

  std::cout << "Ending connection with server\n";
  return 0;
}
