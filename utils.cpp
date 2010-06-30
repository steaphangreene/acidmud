#include <string>
#include <cstdio>
#include <cstring>

#include "utils.h"

void replace_all(string &str, const string &oldt, const string &newt, size_t st) {
  size_t loc = str.find(oldt, st);
  while(loc != string::npos) {
    str.replace(loc, oldt.length(), newt);
    loc = str.find(oldt, loc + newt.length());
    }
  }

void replace_all(string &str, const string &oldt, int newn, size_t st) {
  char buf[4096];
  sprintf(buf, "%d", newn);
  replace_all(str, oldt, buf, st);
  }

void trim_string(string &str) {  //Remove extra whitespace from string
  while(isspace(str[0])) str = str.substr(1);
  while(isspace(str[str.length()-1])) str = str.substr(0, str.length()-1);
  }

size_t skip_line(const string &str, size_t pos) {
  pos = str.find_first_of("\n\r", pos+1);
  while(pos != string::npos && str[pos] && isspace(str[pos])) ++pos;
  if(pos >= str.length()) pos = string::npos;
  return pos;
  }

size_t prev_line(const string &str, size_t pos) {
  pos = str.find_last_of("\n\r", pos-1);
  while(pos != string::npos && pos > 0 && isspace(str[pos])) --pos;
  if(pos != string::npos) pos = str.find_last_of("\n\r", pos);
  while(pos != string::npos && str[pos] && isspace(str[pos])) ++pos;
  if(pos >= str.length()) pos = string::npos;
  return pos;
  }
