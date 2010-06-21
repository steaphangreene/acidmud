#include <string>
#include <cstdio>
#include <cstring>

#include "utils.h"

void replace_all(string &str, const string &oldt, const string &newt) {
  size_t loc = str.find(oldt);
  while(loc != string::npos) {
    str.replace(loc, oldt.length(), newt);
    loc = str.find(oldt);
    }
  }

void replace_all(string &str, const string &oldt, int newn) {
  char buf[4096];
  sprintf(buf, "%d", newn);
  replace_all(str, oldt, buf);
  }

void trim_string(string &str) {  //Remove extra whitespace from string
  while(isspace(str[0])) str = str.substr(1);
  while(isspace(str[str.length()-1])) str = str.substr(0, str.length()-1);
  }
