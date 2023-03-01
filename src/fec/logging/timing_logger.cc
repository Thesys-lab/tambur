#include <stdexcept>
#include <cassert>
#include <chrono>

#include "timing_logger.hh"

using namespace std::chrono;

void TimingLogger::begin_function(const std::string & fname)
{
  start_timings_.push(timestamp_us());
  fnames_.emplace(fname);
}

void TimingLogger::end_function(std::string extra_var)
{
  assert(!start_timings_.empty() and !fnames_.empty());
  auto elapsed = timestamp_us() - start_timings_.top();

  std::string fname = fnames_.top();
  start_timings_.pop();
  fnames_.pop();

  timings_[fname].emplace_back(extra_var, elapsed);
}
