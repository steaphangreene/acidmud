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
#include <type_traits>

#define MAGIC_ENUM_RANGE_MIN 0
#define MAGIC_ENUM_RANGE_MAX 255
#include <magic_enum.hpp>

// Base case: use Magic Enum's size for the enum, assuming all values are used: [0-N]
template <typename T>
struct enum_size : std::integral_constant<uint8_t, magic_enum::enum_count<T>()> {};

// But, if it is an enum with ::MAX and ::NONE, use MAX as the limit: [NONE-MAX)
template <typename T>
  requires(std::is_enum_v<T> && T::MAX > T::NONE && std::to_underlying(T::NONE) == 0)
struct enum_size<T> : std::integral_constant<uint8_t, std::to_underlying(T::MAX)> {};

template <typename T>
static std::u8string_view enum_save(const T enum_val) {
  constexpr uint8_t max = enum_size<T>();
  static std::array<std::u8string, max> save_cache;
  if (save_cache.size() > 0 && save_cache.front() == u8"") {
    for (T e = T::NONE; e != static_cast<T>(max); ++e) {
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
  constexpr uint8_t max = enum_size<T>();
  static std::map<std::u8string_view, T> load_cache;
  if (load_cache.size() < 1) {
    for (T e = T::NONE; e != static_cast<T>(max); ++e) {
      auto name = enum_save(e);
      load_cache[name] = e;
    }
  }
  if (load_cache.contains(str)) {
    return load_cache[str];
  }
  return T::NONE;
}
