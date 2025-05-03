#include "socket-wrappers.h"
#include <iostream>
#include <memory>
#include <thread>
constexpr size_t QUEUE_LEN = 10;
constexpr int MSG_LEN = 999;

void serve_client(std::unique_ptr<TCPSocket> client_socket) {
  std::cout << "Serving " << client_socket->get_local_endpoint() << '\n';
  while (1) {
    std::string message = client_socket->receive_string(MSG_LEN);
    if (message == "END") {
      std::cout << "Ending connection with "
                << client_socket->get_local_endpoint() << '\n';
      return;
    }

    std::cout << '[' << client_socket->get_local_endpoint() << "]: " << message
              << '\n';
  }
}

int main(int argc, char *argv[]) {
  if (argc < 3) {
    std::cerr << "Usage: " << argv[0] << "<ip_address> <port>\n";
    return 1;
  }

  TCPSocket server(argv[1], std::atoi(argv[2]));
  server.listen(QUEUE_LEN);
  std::cout << "Listening on " << server.get_local_endpoint() << "\n";

  while (1) {
    std::unique_ptr<TCPSocket> connection = server.accept();
    std::cout << "Client " << connection->get_local_endpoint()
              << " connected\n";

    std::thread client_thread(serve_client, std::move(connection));
    client_thread.detach();
  }
  return 0;
}
