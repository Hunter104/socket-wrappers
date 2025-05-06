#include "tcp-socket.h"
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

TCPSocket::TCPSocket(std::string_view address, int port)
    : local_socket(address, port, SOCK_STREAM) {}

TCPSocket::TCPSocket(int fd, const Endpoint &endpoint)
    : local_socket(fd, endpoint) {}

void TCPSocket::connect(const Endpoint &server) {
  if (::connect(local_socket.get_socket_descriptor(),
                (sockaddr *)&server.sockaddr(), server.size()) < 0)
    throw std::system_error(errno, std::generic_category(),
                            "Error connecting to server");
}

ssize_t TCPSocket::send(const void *data, size_t length) {
  ssize_t count = ::send(local_socket.get_socket_descriptor(), data, length, 0);
  if (count < 0)
    throw std::system_error(errno, std::generic_category(),
                            "Error sending on socket");
  return count;
}

ssize_t TCPSocket::send(std::string &msg) {
  return send(msg.data(), msg.size());
}

void TCPSocket::listen(size_t queue_len) {
  if (::listen(local_socket.get_socket_descriptor(), queue_len) < 0)
    throw std::system_error(errno, std::generic_category(),
                            "Error listening on socket");
}

std::unique_ptr<TCPSocket> TCPSocket::accept() {
  if (local_socket.get_socket_descriptor() < 0) {
    throw std::runtime_error("Invalid socket descriptor");
  }
  sockaddr_in client_addr;
  socklen_t client_len = sizeof(client_addr);
  int client_fd = ::accept(local_socket.get_socket_descriptor(),
                           (sockaddr *)&client_addr, &client_len);
  if (client_fd < 0) {
    throw std::system_error(errno, std::generic_category(),
                            "Error accepting connection");
  }
  return std::make_unique<TCPSocket>(client_fd, Endpoint(client_addr));
}

std::vector<uint8_t> TCPSocket::receive(size_t max_len) {
  std::vector<uint8_t> buffer(max_len);
  ssize_t received = recv(local_socket.get_socket_descriptor(), buffer.data(),
                          buffer.size(), 0);

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

extern template ssize_t TCPSocket::send_as<int>(int);
extern template int TCPSocket::receive_as<int>();
extern template ssize_t TCPSocket::send_as<float>(float);
extern template float TCPSocket::receive_as<float>();
extern template ssize_t TCPSocket::send_as<double>(double);
extern template double TCPSocket::receive_as<double>();
extern template ssize_t TCPSocket::send_as<uint32_t>(uint32_t);
extern template uint32_t TCPSocket::receive_as<uint32_t>();
extern template ssize_t TCPSocket::send_as<uint64_t>(uint64_t);
extern template uint64_t TCPSocket::receive_as<uint64_t>();
