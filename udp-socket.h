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
#include <unistd.h>
#include <vector>

class UDPSocket {
private:
  Socket local_socket;

public:
  UDPSocket(std::string_view address, int port);
  UDPSocket() : UDPSocket("0.0.0.0", 0) {}
  ~UDPSocket();

  ssize_t send(const void *data, size_t length, const Endpoint &dest);
  ssize_t send(const std::string &message, const Endpoint &destination);

  struct ReceiveResult {
    std::vector<uint8_t> buffer;
    Endpoint sender;
  };
  ReceiveResult receive(size_t max_buffer_size = 4096);

  struct StringReceiveResult {
    std::string message;
    Endpoint sender;
  };
  StringReceiveResult receive_string(size_t max_length = 4096);
  const Endpoint &get_local_endpoint() const {
    return local_socket.get_local_endpoint();
  }
};
