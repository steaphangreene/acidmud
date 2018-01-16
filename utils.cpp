#include <cstdio>
#include <cstring>
#include <string>

#include "utils.h"

void replace_all(string& str, const string& oldt, const string& newt, size_t st) {
  size_t loc = str.find(oldt, st);
  while (loc != string::npos) {
    str.replace(loc, oldt.length(), newt);
    loc = str.find(oldt, loc + newt.length());
  }
}

void replace_all(string& str, const string& oldt, int newn, size_t st) {
  char buf[4096];
  sprintf(buf, "%d", newn);
  replace_all(str, oldt, buf, st);
}

void trim_string(string& str) { // Remove extra whitespace from string
  while (isspace(str[0]))
    str = str.substr(1);
  while (isspace(str[str.length() - 1]))
    str = str.substr(0, str.length() - 1);
}

size_t skip_line(const string& str, size_t pos) {
  pos = str.find_first_of("\n\r", pos + 1);
  while (pos != string::npos && str[pos] && isspace(str[pos]))
    ++pos;
  if (pos >= str.length())
    pos = string::npos;
  return pos;
}

size_t prev_line(const string& str, size_t pos) {
  pos = str.find_last_of("\n\r", pos - 1);
  while (pos != string::npos && pos > 0 && isspace(str[pos]))
    --pos;
  if (pos != string::npos)
    pos = str.find_last_of("\n\r", pos);
  while (pos != string::npos && str[pos] && isspace(str[pos]))
    ++pos;
  if (pos >= str.length())
    pos = string::npos;
  return pos;
}

int phrase_match(const string& str, const string& phrase) {
  int len = phrase.length();
  if (len == 0)
    return 0;

  const char* desc = str.c_str();
  while (*desc) {
    if ((!strncasecmp(desc, phrase.c_str(), len)) && (!isalnum(desc[len]))) {
      return 1;
    }
    while (isalnum(*desc))
      ++desc;
    while ((!isalnum(*desc)) && (*desc))
      ++desc;
  }
  return 0;
}

static const char* alpha = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
static const char* alnum = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
int words_match(const string& str, const string& words) {
  size_t start = words.find_first_of(alpha);
  while (start != string::npos) {
    size_t end = words.find_first_not_of(alnum, start);
    if (end == string::npos)
      end = words.length();
    if (phrase_match(str, words.substr(start, end - start)))
      return 1;
    start = words.find_first_of(alpha, end);
  }
  return 0;
}
