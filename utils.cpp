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

// Replace with C++20 std::format, when widely available
#define FMT_HEADER_ONLY
#include <fmt/format.h>

#include "color.hpp"
#include "utils.hpp"

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
  replace_all(str, oldt, fmt::format(u8"{}", newn), st);
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

size_t skip_line(const std::u8string_view& str, size_t pos) {
  pos = str.find_first_of(u8"\n\r", pos + 1);
  while (pos != std::u8string::npos && pos < str.length() && isspace(str[pos]))
    ++pos;
  if (pos >= str.length())
    pos = std::u8string::npos;
  return pos;
}

size_t prev_line(const std::u8string_view& str, size_t pos) {
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

static bool phrase_match_sensitive(
    const std::u8string_view& str,
    const std::u8string_view& phrase) {
  auto desc = str;
  while (desc.length() >= phrase.length()) {
    if (desc.starts_with(phrase) &&
        (desc.length() == phrase.length() || !ascii_isalnum(desc.at(phrase.length())))) {
      return true;
    }
    auto off = std::find_if_not(desc.begin(), desc.end(), ascii_isalnum);
    if (off == desc.end()) {
      return false;
    }
    off = std::find_if(off, desc.end(), ascii_isalnum);
    if (off == desc.end()) {
      return false;
    }
    desc = desc.substr(off - desc.begin());
  }
  return false;
}

bool phrase_match(const std::u8string_view& str, const std::u8string_view& phrase) {
  if (phrase.length() == 0)
    return false;

  if (std::any_of(str.cbegin(), str.cend(), ascii_isupper)) {
    std::u8string str2(str);
    std::transform(str2.begin(), str2.end(), str2.begin(), ascii_tolower);
    return phrase_match_sensitive(str2, phrase);
  } else {
    return phrase_match_sensitive(str, phrase);
  }
}

bool words_match(const std::u8string_view& str, const std::u8string_view& words) {
  auto start = std::find_if(words.begin(), words.end(), ascii_isalpha);
  while (start != words.end()) {
    auto end = std::find_if_not(start, words.end(), ascii_isalnum);
    if (phrase_match(str, words.substr(start - words.begin(), end - start))) {
      return true;
    }
    start = std::find_if(end, words.end(), ascii_isalpha);
  }
  return false;
}
