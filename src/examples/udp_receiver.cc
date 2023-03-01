#include <iostream>

#include "conversion.hh"
#include "udp_socket.hh"
#include "datagram.hh"

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

  // receive a datagram message
  const auto & [src_addr, binary] = udp_sock.recvfrom();

  Datagram msg;
  if (not msg.parse_from_string(binary)) {
    cerr << "Failed to parse a datagram" << endl;
    return EXIT_FAILURE;
  }

  cerr << "Received " << binary.size() << " bytes from "
       << src_addr.str() << endl;
  cerr << "  seq_num = " << msg.header.seq_num << endl;
  cerr << "  send_ts = " << msg.header.send_ts << endl;
  cerr << "  payload size = " << msg.payload.size() << endl;

  return EXIT_SUCCESS;
}
