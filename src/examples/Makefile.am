AM_CPPFLAGS = $(CXX17_FLAGS) -I$(srcdir)/../util $(erasurecode_CFLAGS)
AM_CXXFLAGS = $(PICKY_CXXFLAGS)

bin_PROGRAMS = \
	udp_echo_server udp_echo_client \
	udp_sender udp_receiver udp_epoll_client

udp_echo_server_SOURCES = udp_echo_server.cc
udp_echo_server_LDADD = ../util/libutil.a

udp_echo_client_SOURCES = udp_echo_client.cc
udp_echo_client_LDADD = ../util/libutil.a

udp_sender_SOURCES = udp_sender.cc datagram.hh datagram.cc
udp_sender_LDADD = ../util/libutil.a

udp_receiver_SOURCES = udp_receiver.cc datagram.hh datagram.cc
udp_receiver_LDADD = ../util/libutil.a

udp_epoll_client_SOURCES = udp_epoll_client.cc datagram.hh datagram.cc
udp_epoll_client_LDADD = ../util/libutil.a
