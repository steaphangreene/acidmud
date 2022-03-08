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

auto fopen(auto&& fn, auto&& md) {
  return std::fopen(reinterpret_cast<const char*>(fn), reinterpret_cast<const char*>(md));
};

template <typename... Args>
auto fprintf(auto&& fl, auto&& fm, Args&&... args) {
  return std::fprintf(fl, reinterpret_cast<const char*>(fm), std::forward<Args>(args)...);
};

template <typename... Args>
auto fscanf(auto&& fl, auto&& fm, Args&&... args) {
  return std::fscanf(fl, reinterpret_cast<const char*>(fm), std::forward<Args>(args)...);
};

template <typename... Args>
auto sscanf(auto&& st, auto&& fm, Args&&... args) {
  return std::sscanf(
      reinterpret_cast<const char*>(st),
      reinterpret_cast<const char*>(fm),
      std::forward<Args>(args)...);
};

#endif // CCHAR8_HPP
