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

#include <array>
#include <map>

#include <magic_enum.hpp>

template <typename T>
static std::u8string_view enum_save(const T enum_val) {
  static std::array<std::u8string, std::to_underlying(T::MAX)> save_cache;
  if (save_cache.size() > 0 && save_cache.front() == u8"") {
    for (T e = T::NONE; e != T::MAX; ++e) {
      auto char_string_name = magic_enum::enum_name(e);
      std::u8string char8_string_name(
          reinterpret_cast<const char8_t*>(char_string_name.data()), char_string_name.length());
      save_cache[std::to_underlying(e)] = char8_string_name;
    }
  }
  return save_cache.at(std::to_underlying(enum_val));
}

template <typename T>
static T enum_load(const std::u8string_view& str) {
  static std::map<std::u8string_view, T> load_cache;
  if (load_cache.size() < 1) {
    for (T e = T::NONE; e != T::MAX; ++e) {
      auto name = enum_save(e);
      load_cache[name] = e;
    }
  }
  if (load_cache.contains(str)) {
    return load_cache[str];
  }
  return T::NONE;
}
