#include <iostream>

#include "conversion.hh"
#include "udp_socket.hh"

using namespace std;

int main(int argc, char * argv[])
{
  if (argc != 2) {
    cerr << "Usage: " << argv[0] << " port" << endl;
    return EXIT_FAILURE;
  }

  UDPSocket udp_sock;
  udp_sock.bind({"0", narrow_cast<uint16_t>(stoi(argv[1]))});
  cerr << "Local address: " << udp_sock.local_address().str() << endl;

  while (true) {
    // receive a datagram and echo it back
    const auto & [src_addr, data] = udp_sock.recvfrom();
    udp_sock.sendto(src_addr, data);
    cerr << "Received " << data.size() << " bytes from "
         << src_addr.str() << endl;
  }

  return EXIT_SUCCESS;
}
