#include <ctime>

#include "timestamp.hh"
#include "exception.hh"

static constexpr uint64_t THOUSAND = 1000;
static constexpr uint64_t MILLION = 1000 * 1000;
static constexpr uint64_t BILLION = 1000 * 1000 * 1000;

uint64_t timestamp_ns()
{
  timespec ts;
  check_syscall(clock_gettime(CLOCK_REALTIME, &ts));

  return ts.tv_sec * BILLION + ts.tv_nsec;
}

uint64_t timestamp_us()
{
  return timestamp_ns() / THOUSAND;
}

uint64_t timestamp_ms()
{
  return timestamp_ns() / MILLION;
}
