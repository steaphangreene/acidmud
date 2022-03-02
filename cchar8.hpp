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

#ifndef CCHAR8_HPP
#define CCHAR8_HPP

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include <crypt.h>
#include <netdb.h>
#include <unistd.h>

const char8_t* crypt8(auto&& ps, auto&& sl) {
  return reinterpret_cast<const char8_t*>(
      crypt(reinterpret_cast<const char*>(ps), reinterpret_cast<const char*>(sl)));
};

auto fopen(auto&& fn, auto&& md) {
  return std::fopen(reinterpret_cast<const char*>(fn), reinterpret_cast<const char*>(md));
};

template <typename... Args>
auto fprintf(auto&& fl, auto&& fm, Args... args) {
  return std::fprintf(fl, reinterpret_cast<const char*>(fm), args...);
};

template <typename... Args>
auto fscanf(auto&& fl, auto&& fm, Args... args) {
  return std::fscanf(fl, reinterpret_cast<const char*>(fm), args...);
};

template <typename... Args>
auto sprintf(auto&& st, auto&& fm, Args... args) {
  return std::sprintf(reinterpret_cast<char*>(st), reinterpret_cast<const char*>(fm), args...);
};

template <typename... Args>
auto sscanf(auto&& st, auto&& fm, Args... args) {
  return std::sscanf(reinterpret_cast<const char*>(st), reinterpret_cast<const char*>(fm), args...);
};

auto strstr(auto&& s1, auto&& s2) {
  return std::strstr(reinterpret_cast<const char*>(s1), reinterpret_cast<const char*>(s2));
};

auto strcasestr8(auto&& s1, auto&& s2) {
  return strcasestr(reinterpret_cast<const char*>(s1), reinterpret_cast<const char*>(s2));
};

auto strcmp(auto&& s1, auto&& s2) {
  return std::strcmp(reinterpret_cast<const char*>(s1), reinterpret_cast<const char*>(s2));
};

auto strncmp(auto&& s1, auto&& s2, auto&& sz) {
  return std::strncmp(reinterpret_cast<const char*>(s1), reinterpret_cast<const char*>(s2), sz);
};

auto strcat(auto&& ds, auto&& sr) {
  return std::strcat(reinterpret_cast<char*>(ds), reinterpret_cast<const char*>(sr));
};

auto strcpy(auto&& ds, auto&& sr) {
  return std::strcpy(reinterpret_cast<char*>(ds), reinterpret_cast<const char*>(sr));
};

auto strlen(auto&& s) {
  return std::strlen(reinterpret_cast<const char*>(s));
};

auto atoi(auto&& s) {
  return std::atoi(reinterpret_cast<const char*>(s));
};

#endif // CCHAR8_HPP
