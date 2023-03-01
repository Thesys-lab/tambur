#ifndef CONVERSION_HH
#define CONVERSION_HH

#include <stdexcept>
#include <string>
#include <type_traits>

// cast two integral types: Source to Target, and assert no precision is lost
template<typename Target, typename Source>
Target narrow_cast(const Source & s)
{
  static_assert(std::is_integral<Source>::value, "Source: integral required");
  static_assert(std::is_integral<Target>::value, "Target: integral required");

  Target t = static_cast<Target>(s);

  if (static_cast<Source>(t) != s) {
    throw std::runtime_error("narrow_cast: " + std::to_string(s)
                             + " != " + std::to_string(t));
  }

  return t;
}

#endif /* CONVERSION_HH */
