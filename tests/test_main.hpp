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

#include <catch2/catch.hpp>

namespace Catch {

template <>
struct StringMaker<char8_t> {
  static std::string convert(char8_t c);
};
inline std::string StringMaker<char8_t>::convert(char8_t c) {
  return ::Catch::Detail::stringify(static_cast<signed char>(c));
}

template <>
struct StringMaker<const std::u8string> {
  static std::string convert(const std::u8string str);
};
inline std::string StringMaker<const std::u8string>::convert(const std::u8string str) {
  return ::Catch::Detail::stringify(reinterpret_cast<char const*>(str.c_str()));
}

template <>
struct StringMaker<std::u8string> {
  static std::string convert(std::u8string str);
};
inline std::string StringMaker<std::u8string>::convert(std::u8string str) {
  return ::Catch::Detail::stringify(reinterpret_cast<char const*>(str.c_str()));
}

template <>
struct StringMaker<const std::u8string_view> {
  static std::string convert(const std::u8string_view str);
};
inline std::string StringMaker<const std::u8string_view>::convert(const std::u8string_view str) {
  return ::Catch::Detail::stringify(reinterpret_cast<char const*>(std::u8string(str).c_str()));
}

template <>
struct StringMaker<std::u8string_view> {
  static std::string convert(std::u8string_view str);
};
inline std::string StringMaker<std::u8string_view>::convert(std::u8string_view str) {
  return ::Catch::Detail::stringify(reinterpret_cast<char const*>(std::u8string(str).c_str()));
}

template <>
struct StringMaker<char8_t const*> {
  static std::string convert(char8_t const* str);
};
inline std::string StringMaker<char8_t const*>::convert(char8_t const* str) {
  return ::Catch::Detail::stringify(reinterpret_cast<char const*>(str));
}

template <int SZ>
struct StringMaker<char8_t[SZ]> {
  static std::string convert(char8_t const* str) {
    return ::Catch::Detail::stringify(reinterpret_cast<char const*>(str));
  }
};

} // namespace Catch

// Get full seen messages from an Object controlled by a mind_t::TEST type Mind
inline auto witness(const auto o) {
  auto ret = o->LongDesc();
  if (ret.length() >= 2) {
    ret = ret.substr(1, ret.length() - 2);
  }
  return ret;
}
