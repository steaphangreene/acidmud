#include <algorithm>
#include <cstdio>
#include <cstring>
#include <string>

#include "color.hpp"
#include "utils.hpp"

static const std::string alpha = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
static const std::string alnum = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

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
  while (pos != std::string::npos && pos < str.length() && isspace(str[pos]))
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
  while (pos != std::string::npos && pos < str.length() && isspace(str[pos]))
    ++pos;
  if (pos >= str.length())
    pos = std::string::npos;
  return pos;
}

int phrase_match_sensitive(const std::string& str, const std::string& phrase) {
  auto len = phrase.length();
  auto desc = str.data();
  auto off = desc - str.data();
  while ((str.length() - off) >= len) {
    if ((!strncmp(desc, phrase.data(), len)) &&
        ((str.length() - off) == len || str.find_first_not_of(alnum, off + len) == (off + len))) {
      return 1;
    }
    off = str.find_first_not_of(alnum, off);
    if (off == std::string::npos)
      return 0;
    off = str.find_first_of(alnum, off);
    if (off == std::string::npos)
      return 0;
    desc = str.data() + off;
  }
  return 0;
}

int phrase_match(const std::string& str, const std::string& phrase) {
  if (phrase.length() == 0)
    return 0;

  if (std::any_of(str.cbegin(), str.cend(), ::isupper)) {
    std::string str2 = str;
    std::transform(str2.begin(), str2.end(), str2.begin(), ::tolower);
    return phrase_match_sensitive(str2, phrase);
  } else {
    return phrase_match_sensitive(str, phrase);
  }
}

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
