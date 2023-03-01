#ifndef EXCEPTION_HH
#define EXCEPTION_HH

#include <system_error>

class unix_error : public std::system_error
{
public:
  unix_error()
    : system_error(errno, std::system_category()) {}

  unix_error(const std::string & tag)
    : system_error(errno, std::system_category(), tag) {}
};

inline int check_syscall(const int return_value)
{
  if (return_value >= 0) {
    return return_value;
  }
  throw unix_error();
}

inline int check_syscall(const int return_value, const std::string & tag)
{
  if (return_value >= 0) {
    return return_value;
  }
  throw unix_error(tag);
}

#endif /* EXCEPTION_HH */
