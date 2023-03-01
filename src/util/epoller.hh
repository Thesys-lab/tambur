#ifndef EPOLLER_HH
#define EPOLLER_HH

#include <sys/epoll.h>

#include <map>
#include <set>
#include <functional>

class Epoller
{
public:
  // type definitions
  enum Flag : uint32_t {
    In = EPOLLIN,
    Out = EPOLLOUT
  };

  using Callback = std::function<void()>;

  Epoller();
  ~Epoller();

  // accessors
  int epfd() const { return epfd_; }

  // register a single event (flag) on fd to monitor with a callback function
  void register_event(const int fd,
                      const Flag flag,
                      const Callback callback);

  // activate an event on fd (safe to be called repeatedly)
  void activate(const int fd, const Flag flag);

  // deactivate an event on fd (safe to be called repeatedly)
  void deactivate(const int fd, const Flag flag);

  // *schedule* fd to deregister from epoll's interest list
  void deregister(const int fd);

  // execute the callbacks on the ready fds
  void poll(const int timeout_ms = -1);

private:
  // add fd to epoll's interest list and monitor provided events
  void epoll_add(const int fd, const uint32_t events);

  // modify the events setting to 'events' for fd
  void epoll_mod(const int fd, const uint32_t events);

  // *actually* deregister fds in fds_to_deregister_
  void do_deregister();

  // data members
  int epfd_;

  // fd -> {flag (bit value) -> callback}
  std::map<int, std::map<Flag, Callback>> roster_;

  // fd -> currently active events (bitmask)
  std::map<int, uint32_t> active_events_;

  // fds scheduled to deregister
  std::set<int> fds_to_deregister_;
};

#endif /* EPOLLER_HH */
