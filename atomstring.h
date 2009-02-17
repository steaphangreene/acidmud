#ifndef ATOMSTRING_H
#define ATOMSTRING_H

#include <set>
#include <string>
using namespace std;

class AtomString {
public:
  AtomString() { val = strlist.insert("").first; };
  AtomString(const char *other) { val = strlist.insert(other).first; };
  AtomString(const string &other) { val = strlist.insert(other).first; };
  AtomString(const AtomString &other) { val = other.val; };
  operator const string &() const { return *val; };

  bool operator == (const AtomString &in) const {
    return ((*val) == (*(in.val)));
    };
  bool operator != (const AtomString &in) const {
    return ((*val) != (*(in.val)));
    };
  bool operator < (const AtomString &in) const {
    return ((*val) < (*(in.val)));
    };
  bool operator <= (const AtomString &in) const {
    return ((*val) <= (*(in.val)));
    };
  bool operator > (const AtomString &in) const {
    return ((*val) > (*(in.val)));
    };
  bool operator >= (const AtomString &in) const {
    return ((*val) >= (*(in.val)));
    };
  void operator = (const AtomString &in) {	// Returns VOID!
    val = in.val;
    };
  unsigned int length() const { return val->length(); }
  const char *c_str() const { return val->c_str(); }

private:
  set<string>::iterator val;
  static set<string> strlist;
  };

#endif	// ATOMSTRING_H
