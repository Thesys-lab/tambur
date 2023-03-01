#ifndef SOCKET_HH
#define SOCKET_HH

#include <sys/types.h>
#include <sys/socket.h>

#include "address.hh"

class Socket
{
protected:
  // constructor
  Socket(const int domain, const int type);

  // destructor
  ~Socket();

public:
  // bind socket to a local address
  void bind(const Address & local_addr);

  // connect socket to a peer address
  void connect(const Address & peer_addr);

  // get local and peer addresses
  Address local_address() const;
  Address peer_address() const;

  // I/O mode
  bool get_blocking() const;
  void set_blocking(const bool blocking);

  int fd() const { return fd_; }

private:
  // socket file descriptor
  int fd_;
};

#endif /* SOCKET_HH */
