#include "base-socket.h"
#include <cstdint>
#include <stddef.h>
#include <string_view>
#include <sys/socket.h>
#include <sys/types.h>
#include <system_error>
#include <unistd.h>

Endpoint::Endpoint(std::string_view address, uint16_t port)
    : address_(address), port_(port) {
  addr_.sin_family = AF_INET;
  addr_.sin_addr.s_addr = inet_addr(address_.c_str());
  addr_.sin_port = htons(port_);
}

Endpoint::Endpoint(const sockaddr_in &addr)
    : address_(inet_ntoa(addr.sin_addr)), port_(ntohs(addr.sin_port)),
      addr_(addr) {}

Endpoint::Endpoint() : address_("ANY"), port_(0) {
  addr_.sin_family = AF_INET;
  addr_.sin_addr.s_addr = htonl(INADDR_ANY);
  addr_.sin_port = htons(0);
}

const std::string &Endpoint::address() const { return address_; }
uint16_t Endpoint::port() const { return port_; }
const sockaddr_in &Endpoint::sockaddr() const { return addr_; }
socklen_t Endpoint::size() const { return sizeof(addr_); }

std::ostream &operator<<(std::ostream &Str, Endpoint const &v) {
  Str << v.address() << ":" << v.port();
  return Str;
}

Socket::Socket(std::string_view address, int port, int type)
    : local_endpoint(address, port) {
  socket_descriptor = socket(AF_INET, type, 0);
  if (socket_descriptor < 0)
    throw std::system_error(errno, std::generic_category(),
                            "Failed to create socket");

  int rc = bind(socket_descriptor, (sockaddr *)&local_endpoint.sockaddr(),
                local_endpoint.size());
  if (rc < 0)
    throw std::system_error(errno, std::generic_category(),
                            "Failed to bind socket");
}

Socket::Socket(int fd, const Endpoint &endpoint)
    : socket_descriptor(fd), local_endpoint(endpoint) {}
