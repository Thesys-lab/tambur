#ifndef UDP_SOCKET_HH
#define UDP_SOCKET_HH

#include <string>
#include <utility>

#include "address.hh"
#include "socket.hh"

class UDPSocket : public Socket
{
public:
  // constructor
  UDPSocket() : Socket(AF_INET, SOCK_DGRAM) {};

  // return true if datagram is sent in its entirety
  // return false to indicate EWOULDBLOCK in nonblocking I/O mode
  bool send(const std::string & data);
  bool sendto(const Address & dst_addr, const std::string & data);

  // receive a datagram (*supposedly* from a connected address)
  std::string recv();

  // receive a datagram and its source address
  std::pair<Address, std::string> recvfrom();

private:
  bool check_bytes_sent(const ssize_t bytes_sent, const size_t target) const;

  static constexpr size_t UDP_MTU = 65536; // bytes
};

#endif /* UDP_SOCKET_HH */
