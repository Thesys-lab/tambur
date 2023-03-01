#include "serialization.hh"

using namespace std;

string WireParser::read_string(const size_t len)
{
  if (len > str_.size()) {
    throw out_of_range("WireParser::read_string(): attempted to read past end");
  }

  string ret { str_.data(), len };

  // move the start of string view forward
  str_.remove_prefix(len);

  return ret;
}

void WireParser::skip(const size_t len)
{
  if (len > str_.size()) {
    throw out_of_range("WireParser::skip(): attempted to skip past end");
  }

  str_.remove_prefix(len);
}
