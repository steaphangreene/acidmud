#include <cstdio>
#include <cstring>
#include <string>

#include "utils.hpp"

void replace_all(std::string& str, const std::string& oldt, const std::string& newt, size_t st) {
  size_t loc = str.find(oldt, st);
  while (loc != std::string::npos) {
    str.replace(loc, oldt.length(), newt);
    loc = str.find(oldt, loc + newt.length());
  }
}

void replace_all(std::string& str, const std::string& oldt, int newn, size_t st) {
  char buf[4096];
  sprintf(buf, "%d", newn);
  replace_all(str, oldt, buf, st);
}

void trim_string(std::string& str) { // Remove extra whitespace from std::string
  size_t b = str.find_first_not_of(" \n\r\t\f\v");
  size_t e = str.find_last_not_of(" \n\r\t\f\v");
  if (b == std::string::npos || e == std::string::npos || b > e) { // No (valid) string
    str.clear();
  } else if (b != 0 || e + 1 != str.length()) { // String needs trimming
    str = str.substr(b, (e - b) + 1);
  }
}

size_t skip_line(const std::string& str, size_t pos) {
  pos = str.find_first_of("\n\r", pos + 1);
  while (pos != std::string::npos && str[pos] && isspace(str[pos]))
    ++pos;
  if (pos >= str.length())
    pos = std::string::npos;
  return pos;
}

size_t prev_line(const std::string& str, size_t pos) {
  pos = str.find_last_of("\n\r", pos - 1);
  while (pos != std::string::npos && pos > 0 && isspace(str[pos]))
    --pos;
  if (pos != std::string::npos)
    pos = str.find_last_of("\n\r", pos);
  while (pos != std::string::npos && str[pos] && isspace(str[pos]))
    ++pos;
  if (pos >= str.length())
    pos = std::string::npos;
  return pos;
}

int phrase_match(const std::string& str, const std::string& phrase) {
  int len = phrase.length();
  if (len == 0)
    return 0;

  const char* desc = str.c_str();
  while (*desc) {
    if ((!strncmp(desc, phrase.c_str(), len)) && (!isalnum(desc[len]))) {
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
int words_match(const std::string& str, const std::string& words) {
  size_t start = words.find_first_of(alpha);
  while (start != std::string::npos) {
    size_t end = words.find_first_not_of(alnum, start);
    if (end == std::string::npos)
      end = words.length();
    if (phrase_match(str, words.substr(start, end - start)))
      return 1;
    start = words.find_first_of(alpha, end);
  }
  return 0;
}
