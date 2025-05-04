#include "base-socket.h"
#include <arpa/inet.h>
#include <cstdint>
#include <memory>
#include <netinet/in.h>
#include <ostream>
#include <stddef.h>
#include <stdint.h>
#include <string>
#include <string_view>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

class TCPSocket {
private:
  Socket local_socket;

public:
  TCPSocket(std::string_view address, int port);
  TCPSocket(int fd, const Endpoint &endpoint);
  TCPSocket() : TCPSocket("0.0.0.0", 0) {}
  ~TCPSocket();

  void connect(const Endpoint &server);
  ssize_t send(const void *data, size_t length);
  ssize_t send(std::string &msg);

  void listen(size_t queue_len);
  std::unique_ptr<TCPSocket> accept();
  std::vector<uint8_t> receive(size_t max_len);
  std::string receive_string(size_t max_len);
  const Endpoint &get_local_endpoint() const {
    return local_socket.get_local_endpoint();
  }
};
