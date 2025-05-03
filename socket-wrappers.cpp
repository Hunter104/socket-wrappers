#include "socket-wrappers.h"
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <stddef.h>
#include <string_view>
#include <sys/socket.h>
#include <sys/types.h>
#include <system_error>
#include <unistd.h>

// Endpoint implementation
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

// NOTE: code duplication
TCPSocket::TCPSocket(std::string_view address, int port)
    : local_endpoint(address, port) {
  socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_descriptor < 0)
    throw std::system_error(errno, std::generic_category(),
                            "Failed to create socket");

  int rc = bind(socket_descriptor, (sockaddr *)&local_endpoint.sockaddr(),
                local_endpoint.size());
  if (rc < 0)
    throw std::system_error(errno, std::generic_category(),
                            "Failed to bind socket");
}

TCPSocket::TCPSocket(int fd, const Endpoint &endpoint)
    : socket_descriptor(fd), local_endpoint(endpoint) {}

void TCPSocket::connect(const Endpoint &server) {
  if (::connect(socket_descriptor, (sockaddr *)&server.sockaddr(),
                server.size()) < 0)
    throw std::system_error(errno, std::generic_category(),
                            "Error cconnecting to server");
}

ssize_t TCPSocket::send(const void *data, size_t length) {
  ssize_t count = ::send(socket_descriptor, data, length, 0);
  if (count < 0)
    throw std::system_error(errno, std::generic_category(),
                            "Error sending on socket");
  return count;
}

ssize_t TCPSocket::send(std::string &msg) {
  return send(msg.data(), msg.size());
}

void TCPSocket::listen(size_t queue_len) {
  if (::listen(socket_descriptor, queue_len) < 0)
    throw std::system_error(errno, std::generic_category(),
                            "Error listening on socket");
}

std::unique_ptr<TCPSocket> TCPSocket::accept() {
  if (socket_descriptor < 0) {
    throw std::runtime_error("Invalid socket descriptor for accepting");
  }
  sockaddr_in client_addr;
  socklen_t client_len = sizeof(client_addr);
  int client_fd =
      ::accept(socket_descriptor, (sockaddr *)&client_addr, &client_len);
  if (client_fd < 0) {
    throw std::system_error(errno, std::generic_category(),
                            "Error accepting connection");
  }
  return std::make_unique<TCPSocket>(client_fd, Endpoint(client_addr));
}

std::vector<uint8_t> TCPSocket::receive(size_t max_len) {
  std::vector<uint8_t> buffer(max_len);
  ssize_t received = recv(socket_descriptor, buffer.data(), buffer.size(), 0);

  if (received < 0)
    throw std::system_error(errno, std::generic_category(),
                            "Failed to receive data");

  buffer.resize(received);
  return buffer;
}

std::string TCPSocket::receive_string(size_t max_length) {
  auto result = receive(max_length);
  std::string message(reinterpret_cast<char *>(result.data()), result.size());

  return message;
}

TCPSocket::~TCPSocket() { close(socket_descriptor); }

// UDPSocket implementation
UDPSocket::UDPSocket(std::string_view address, int port)
    : local_endpoint(address, port) {
  socket_descriptor = socket(AF_INET, SOCK_DGRAM, 0);
  if (socket_descriptor < 0)
    throw std::system_error(errno, std::generic_category(),
                            "Failed to create socket");

  int rc = bind(socket_descriptor, (sockaddr *)&local_endpoint.sockaddr(),
                local_endpoint.size());
  if (rc < 0)
    throw std::system_error(errno, std::generic_category(),
                            "Failed to bind socket");

  // For default constructor (client mode), update local_endpoint with assigned
  // port
  if (port == 0) {
    sockaddr_in actual_addr;
    socklen_t addr_len = sizeof(actual_addr);
    if (getsockname(socket_descriptor, (sockaddr *)&actual_addr, &addr_len) >=
        0) {
      local_endpoint = Endpoint(actual_addr);
    }
  }
}

UDPSocket::~UDPSocket() { close(socket_descriptor); }

ssize_t UDPSocket::send(const void *data, size_t length, const Endpoint &dest) {
  ssize_t sent = sendto(socket_descriptor, data, length, 0,
                        (sockaddr *)&dest.sockaddr(), dest.size());
  if (sent < 0)
    throw std::system_error(errno, std::generic_category(),
                            "Failed to send data");

  return sent;
}

ssize_t UDPSocket::send(const std::string &message,
                        const Endpoint &destination) {
  return send(message.data(), message.size(), destination);
}

UDPSocket::ReceiveResult UDPSocket::receive(size_t max_buffer_size) {
  std::vector<uint8_t> buffer(max_buffer_size);
  sockaddr_in sender_addr{};
  socklen_t sender_len = sizeof(sender_addr);

  ssize_t received = recvfrom(socket_descriptor, buffer.data(), buffer.size(),
                              0, (sockaddr *)&sender_addr, &sender_len);

  if (received < 0)
    throw std::system_error(errno, std::generic_category(),
                            "Failed to receive data");

  buffer.resize(received);

  return {std::move(buffer), Endpoint(sender_addr)};
}

UDPSocket::StringReceiveResult UDPSocket::receive_string(size_t max_length) {
  auto result = receive(max_length);
  std::string message(reinterpret_cast<char *>(result.buffer.data()),
                      result.buffer.size());
  return {std::move(message), std::move(result.sender)};
}
