// *************************************************************************
//  This file is part of AcidMUD by Steaphan Greene
//
//  Copyright 1999-2022 Steaphan Greene <steaphan@gmail.com>
//
//  AcidMUD is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 3 of the License, or
//  (at your option) any later version.
//
//  AcidMUD is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with AcidMUD (see the file named "COPYING");
//  If not, see <http://www.gnu.org/licenses/>.
//
// *************************************************************************

#include <algorithm>
#include <string>

#include "cchar8.hpp"
#include "color.hpp"
#include "utils.hpp"

static const std::u8string alpha = u8"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
static const std::u8string alnum =
    u8"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

void replace_all(
    std::u8string& str,
    const std::u8string& oldt,
    const std::u8string& newt,
    size_t st) {
  size_t loc = str.find(oldt, st);
  while (loc != std::u8string::npos) {
    str.replace(loc, oldt.length(), newt);
    loc = str.find(oldt, loc + newt.length());
  }
}

void replace_all(std::u8string& str, const std::u8string& oldt, int newn, size_t st) {
  char8_t buf[4096];
  sprintf(buf, u8"%d", newn);
  replace_all(str, oldt, buf, st);
}

void trim_string(std::u8string& str) { // Remove extra whitespace from std::u8string
  size_t b = str.find_first_not_of(u8" \n\r\t\f\v");
  size_t e = str.find_last_not_of(u8" \n\r\t\f\v");
  if (b == std::u8string::npos || e == std::u8string::npos || b > e) { // No (valid) string
    str.clear();
  } else if (b != 0 || e + 1 != str.length()) { // String needs trimming
    str = str.substr(b, (e - b) + 1);
  }
}

void trim_string(std::u8string_view& str) { // Remove extra whitespace from std::u8string
  size_t b = str.find_first_not_of(u8" \n\r\t\f\v");
  size_t e = str.find_last_not_of(u8" \n\r\t\f\v");
  if (b == std::u8string::npos || e == std::u8string::npos || b > e) { // No (valid) string
    str = str.substr(0, 0);
  } else if (b != 0 || e + 1 != str.length()) { // String needs trimming
    str = str.substr(b, (e - b) + 1);
  }
}

size_t skip_line(const std::u8string& str, size_t pos) {
  pos = str.find_first_of(u8"\n\r", pos + 1);
  while (pos != std::u8string::npos && pos < str.length() && isspace(str[pos]))
    ++pos;
  if (pos >= str.length())
    pos = std::u8string::npos;
  return pos;
}

size_t prev_line(const std::u8string& str, size_t pos) {
  pos = str.find_last_of(u8"\n\r", pos - 1);
  while (pos != std::u8string::npos && pos > 0 && isspace(str[pos]))
    --pos;
  if (pos != std::u8string::npos)
    pos = str.find_last_of(u8"\n\r", pos);
  while (pos != std::u8string::npos && pos < str.length() && isspace(str[pos]))
    ++pos;
  if (pos >= str.length())
    pos = std::u8string::npos;
  return pos;
}

int phrase_match_sensitive(const std::u8string_view& str, const std::u8string_view& phrase) {
  auto len = phrase.length();
  auto desc = str.data();
  size_t off = 0;
  while ((str.length() - off) >= len) {
    if ((!strncmp(desc, phrase.data(), len)) &&
        ((str.length() - off) == len || str.find_first_not_of(alnum, off + len) == (off + len))) {
      return 1;
    }
    off = str.find_first_not_of(alnum, off);
    if (off == std::u8string::npos)
      return 0;
    off = str.find_first_of(alnum, off);
    if (off == std::u8string::npos)
      return 0;
    desc = str.data() + off;
  }
  return 0;
}

int phrase_match(const std::u8string_view& str, const std::u8string_view& phrase) {
  if (phrase.length() == 0)
    return 0;

  if (std::any_of(str.cbegin(), str.cend(), ascii_isupper)) {
    std::u8string str2(str);
    std::transform(str2.begin(), str2.end(), str2.begin(), ascii_tolower);
    return phrase_match_sensitive(str2, phrase);
  } else {
    return phrase_match_sensitive(str, phrase);
  }
}

int words_match(const std::u8string_view& str, const std::u8string_view& words) {
  size_t start = words.find_first_of(alpha);
  while (start != std::u8string::npos) {
    size_t end = words.find_first_not_of(alnum, start);
    if (end == std::u8string::npos)
      end = words.length();
    if (phrase_match(str, words.substr(start, end - start)))
      return 1;
    start = words.find_first_of(alpha, end);
  }
  return 0;
}
