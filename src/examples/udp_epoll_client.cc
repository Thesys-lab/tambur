/* UDP client that demonstrates nonblocking I/O with Epoller */

#include <iostream>
#include <string>
#include <deque>

#include "conversion.hh"
#include "timestamp.hh"
#include "udp_socket.hh"
#include "datagram.hh"
#include "epoller.hh"

using namespace std;

int main(int argc, char * argv[])
{
  if (argc != 3) {
    cerr << "Usage: " << argv[0] << " host port" << endl;
    return EXIT_FAILURE;
  }

  Address peer_addr{argv[1], narrow_cast<uint16_t>(stoi(argv[2]))};

  UDPSocket udp_sock;
  udp_sock.connect(peer_addr);
  cerr << "Local address: " << udp_sock.local_address().str() << endl;

  // set socket to be nonblocking
  udp_sock.set_blocking(false);

  Epoller epoller;
  deque<Datagram> datagram_buf;

  // if UDP socket is readable
  epoller.register_event(udp_sock.fd(), Epoller::In,
    [&udp_sock]()
    {
      Datagram datagram;
      if (not datagram.parse_from_string(udp_sock.recv())) {
        throw runtime_error("failed to parse a datagram");
      }

      cerr << "Received echo for " << datagram.header.seq_num << endl;
    }
  );

  // if UDP socket is writable
  epoller.register_event(udp_sock.fd(), Epoller::Out,
    [&epoller, &udp_sock, &datagram_buf]()
    {
      while (not datagram_buf.empty()) {
        const auto & datagram = datagram_buf.front();

        if (udp_sock.send(datagram.serialize_to_string())) {
          cerr << "Sent datagram " << datagram.header.seq_num << endl;

          datagram_buf.pop_front();
        } else { // EWOULDBLOCK
          cerr << "Try again later" << endl;
          break;
        }
      }

      if (datagram_buf.empty()) {
        // not interested in socket being writable anymore since buf is empty
        epoller.deactivate(udp_sock.fd(), Epoller::Out);
      }
    }
  );

  // send 'datagram_cnt' dummy datagrams roughly every 'interval_ms' ms
  const int interval_ms = 1000;
  const int datagram_cnt = 3;
  uint64_t target = timestamp_ms();
  uint32_t seq_num = 0;

  while (true) {
    if (timestamp_ms() >= target) {
      for (int i = 0; i < datagram_cnt; i++) {
        datagram_buf.emplace_back(Datagram {
          {seq_num++, timestamp_ms()}, // dummy header
          string(1400, 'x') // dummy payload
        });
      }

      // interested in socket being writable again since buf becomes nonempty
      epoller.activate(udp_sock.fd(), Epoller::Out);
      target += interval_ms;
    }

    epoller.poll(target - timestamp_ms());
  }

  return EXIT_SUCCESS;
}
