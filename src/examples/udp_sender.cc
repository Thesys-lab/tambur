#include <iostream>
#include <string>

#include "conversion.hh"
#include "udp_socket.hh"
#include "datagram.hh"
#include "timestamp.hh"

using namespace std;

int main(int argc, char * argv[])
{
  if (argc != 3) {
    cerr << "Usage: " << argv[0] << " host port" << endl;
    return EXIT_FAILURE;
  }

  Address dst_addr{argv[1], narrow_cast<uint16_t>(stoi(argv[2]))};

  UDPSocket udp_sock;
  udp_sock.connect(dst_addr);
  cerr << "Local address: " << udp_sock.local_address().str() << endl;

  // construct a dummy datagram
  Datagram datagram {
    {12345, timestamp_ms()}, // dummy header
    {string(1000, 'x')} // dummy payload
  };

  const string & binary = datagram.serialize_to_string();
  udp_sock.send(binary);

  cerr << "Sent " << binary.size() << " bytes to " << dst_addr.str() << endl;
  cerr << "  seq_num = " << datagram.header.seq_num << endl;
  cerr << "  send_ts = " << datagram.header.send_ts << endl;
  cerr << "  payload size = " << datagram.payload.size() << endl;

  return EXIT_SUCCESS;
}
