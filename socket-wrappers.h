#pragma once

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
#include <vector>

class Endpoint {
public:
  Endpoint(std::string_view address, uint16_t port);
  explicit Endpoint(const sockaddr_in &addr);
  Endpoint();

  const std::string &address() const;
  uint16_t port() const;
  const sockaddr_in &sockaddr() const;
  socklen_t size() const;

private:
  std::string address_;
  uint16_t port_;
  sockaddr_in addr_ = {};
};

std::ostream &operator<<(std::ostream &Str, Endpoint const &v);

class TCPSocket {
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

  const Endpoint &get_local_endpoint() const { return local_endpoint; }

private:
  int socket_descriptor;
  Endpoint local_endpoint;
};

class UDPSocket {
public:
  UDPSocket(std::string_view address, int port);
  UDPSocket() : UDPSocket("0.0.0.0", 0) {}
  ~UDPSocket();

  // Send methods
  ssize_t send(const void *data, size_t length, const Endpoint &dest);
  ssize_t send(const std::string &message, const Endpoint &destination);

  // Receive methods
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

  // Get the local endpoint
  const Endpoint &get_local_endpoint() const { return local_endpoint; }

private:
  int socket_descriptor;
  Endpoint local_endpoint;
};
