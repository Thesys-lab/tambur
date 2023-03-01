#include <fcntl.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

#include "socket.hh"
#include "exception.hh"

using namespace std;

Socket::Socket(const int domain, const int type)
{
  fd_ = check_syscall(socket(domain, type, 0));
}

Socket::~Socket()
{
  // don't throw errors in a destructor
  if (close(fd_) < 0) {
    cerr << "failed to close socket: " << strerror(errno) << endl;
  }
}

void Socket::bind(const Address & local_addr)
{
  check_syscall(::bind(fd_, &local_addr.sock_addr(), local_addr.size()));
}

void Socket::connect(const Address & peer_addr)
{
  check_syscall(::connect(fd_, &peer_addr.sock_addr(), peer_addr.size()));
}

Address Socket::local_address() const
{
  sockaddr addr;
  socklen_t size = sizeof(addr);

  check_syscall(getsockname(fd(), &addr, &size));
  return {addr, size};
}

Address Socket::peer_address() const
{
  sockaddr addr;
  socklen_t size = sizeof(addr);

  check_syscall(getpeername(fd(), &addr, &size));
  return {addr, size};
}

bool Socket::get_blocking() const
{
  int flags = check_syscall(fcntl(fd_, F_GETFL));
  return !(flags & O_NONBLOCK);
}

void Socket::set_blocking(const bool blocking)
{
  int flags = check_syscall(fcntl(fd_, F_GETFL));

  if (blocking) {
    flags &= ~O_NONBLOCK;
  } else {
    flags |= O_NONBLOCK;
  }

  check_syscall(fcntl(fd_, F_SETFL, flags));
}
