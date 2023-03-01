#include <iostream>
#include <string>

#include "conversion.hh"
#include "udp_socket.hh"

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

  while (true) {
    // read a string line from stdin and send it to the server
    cerr << "Enter a line of data: ";
    string line;
    getline(cin, line);

    if (cin.eof()) {
      break;
    }

    if (line.empty()) {
      cerr << "Error: line cannot be empty" << endl;
      continue;
    }

    udp_sock.send(line);

    // read the string echoed from server
    cerr << "Server echoed: " << udp_sock.recv() << endl;
  }

  return EXIT_SUCCESS;
}
