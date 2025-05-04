#include "udp-socket.h"
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <stddef.h>
#include <string_view>
#include <sys/socket.h>
#include <sys/types.h>
#include <system_error>
#include <unistd.h>

UDPSocket::UDPSocket(std::string_view address, int port)
    : local_socket(address, port, SOCK_DGRAM) {}

ssize_t UDPSocket::send(const void *data, size_t length, const Endpoint &dest) {
  ssize_t sent = sendto(local_socket.get_socket_descriptor(), data, length, 0,
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

  ssize_t received =
      recvfrom(local_socket.get_socket_descriptor(), buffer.data(),
               buffer.size(), 0, (sockaddr *)&sender_addr, &sender_len);

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
