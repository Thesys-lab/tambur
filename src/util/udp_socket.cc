#include <stdexcept>

#include "udp_socket.hh"
#include "exception.hh"

using namespace std;

bool UDPSocket::check_bytes_sent(const ssize_t bytes_sent,
                                 const size_t target) const
{
  if (bytes_sent <= 0) {
    if (bytes_sent == -1 and errno == EWOULDBLOCK) {
      return false; // return false to indicate EWOULDBLOCK
    }

    throw unix_error("UDPSocket:send()/sendto()");
  }

  if (static_cast<size_t>(bytes_sent) != target) {
    throw runtime_error("UDPSocket failed to deliver target number of bytes");
  }

  return true;
}

bool UDPSocket::send(const string & data)
{
  if (data.empty()) {
    throw runtime_error("attempted to send empty data");
  }

  const ssize_t bytes_sent = ::send(fd(), data.data(), data.size(), 0);
  return check_bytes_sent(bytes_sent, data.size());
}

bool UDPSocket::sendto(const Address & dst_addr, const string & data)
{
  if (data.empty()) {
    throw runtime_error("attempted to send empty data");
  }

  const ssize_t bytes_sent = ::sendto(fd(), data.data(), data.size(), 0,
                                      &dst_addr.sock_addr(), dst_addr.size());
  return check_bytes_sent(bytes_sent, data.size());
}

string UDPSocket::recv()
{
  // data to receive
  char buf[UDP_MTU];

  const size_t bytes_received = check_syscall(
    ::recv(fd(), buf, sizeof(buf), MSG_TRUNC));

  if (bytes_received > UDP_MTU) {
    throw runtime_error("recv(): received an oversized datagram");
  }

  return {buf, bytes_received};
}

pair<Address, string> UDPSocket::recvfrom()
{
  // data to receive and its source address
  char buf[UDP_MTU];
  sockaddr src_addr;
  socklen_t src_addr_len = sizeof(src_addr);

  const size_t bytes_received = check_syscall(
    ::recvfrom(fd(), buf, sizeof(buf), MSG_TRUNC,
               &src_addr, &src_addr_len));

  if (bytes_received > UDP_MTU) {
    throw runtime_error("recvfrom(): received an oversized datagram");
  }

  return {{src_addr, src_addr_len}, {buf, bytes_received}};
}
