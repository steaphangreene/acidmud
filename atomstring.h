#ifndef ATOMSTRING_H
#define ATOMSTRING_H

#include <string>
using namespace std;

class AtomString {
public:
  AtomString() {};
  AtomString(const char *other) { val = other; };
  AtomString(const string &other) { val = other; };
  AtomString(const AtomString &other) { val = other.val; };
  operator const string &() const { return val; };

  bool operator == (const string &in) const {
    return (val == in);
    };

  bool operator != (const string &in) const {
    return (val != in);
    };

  bool operator < (const string &in) const {
    return (val < in);
    };

  bool operator <= (const string &in) const {
    return (val <= in);
    };

  bool operator > (const string &in) const {
    return (val > in);
    };

  bool operator >= (const string &in) const {
    return (val >= in);
    };

  const string &operator = (const AtomString &in) {
    return (val = in.val);
    };

  unsigned int length() const { return val.length(); }
  const char *c_str() const { return val.c_str(); }

private:
  string val;
  };

#endif	// ATOMSTRING_H
