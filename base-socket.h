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
#include <unistd.h>
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

class Socket {
protected:
  int socket_descriptor;
  Endpoint local_endpoint;

public:
  Socket(std::string_view address, int port, int type);
  Socket(int fd, const Endpoint &endpoint);
  Socket(int type) : Socket("0.0.0.0", 0, type) {}

  ~Socket() { close(socket_descriptor); }
  const Endpoint &get_local_endpoint() const { return local_endpoint; }
  int get_socket_descriptor() const { return socket_descriptor; }
};
