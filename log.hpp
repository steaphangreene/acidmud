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

#ifndef LOG_HPP
#define LOG_HPP

#include <iostream>
#include <string>
#include <utility>

// Replace with C++20 std::format, when widely available
#define FMT_HEADER_ONLY
#include <fmt/format.h>

#include "color.hpp"

// Seriously, this type isn't supported by iostream?
inline std::ostream& operator<<(std::ostream& out, const std::u8string& in) {
  return out << reinterpret_cast<const char*>(in.c_str());
}

// Seriously, this type is totally broken in iostream?
inline std::ostream& operator<<(std::ostream& out, const char8_t* in) {
  return out << reinterpret_cast<const char*>(in);
}

template <typename... Args>
void logo(const std::u8string_view& mes, Args&&... args) {
  std::cout << fmt::format(mes, std::forward<Args>(args)...);
};
template <typename... Args>
void loge(const std::u8string_view& mes, Args&&... args) {
  std::cerr << fmt::format(mes, std::forward<Args>(args)...);
};
template <typename... Args>
void loger(const std::u8string_view& mes, Args&&... args) {
  std::cerr << CRED << fmt::format(mes, std::forward<Args>(args)...) << CNRM;
};
template <typename... Args>
void logerr(const std::u8string_view& mes, Args&&... args) {
  std::cerr << CBRD << fmt::format(mes, std::forward<Args>(args)...) << CNRM;
};
template <typename... Args>
void logeg(const std::u8string_view& mes, Args&&... args) {
  std::cerr << CGRN << fmt::format(mes, std::forward<Args>(args)...) << CNRM;
};
template <typename... Args>
void logegg(const std::u8string_view& mes, Args&&... args) {
  std::cerr << CBGR << fmt::format(mes, std::forward<Args>(args)...) << CNRM;
};
template <typename... Args>
void logeb(const std::u8string_view& mes, Args&&... args) {
  std::cerr << CBLU << fmt::format(mes, std::forward<Args>(args)...) << CNRM;
};
template <typename... Args>
void logebb(const std::u8string_view& mes, Args&&... args) {
  std::cerr << CBBL << fmt::format(mes, std::forward<Args>(args)...) << CNRM;
};
template <typename... Args>
void logey(const std::u8string_view& mes, Args&&... args) {
  std::cerr << CYEL << fmt::format(mes, std::forward<Args>(args)...) << CNRM;
};
template <typename... Args>
void logeyy(const std::u8string_view& mes, Args&&... args) {
  std::cerr << CBYL << fmt::format(mes, std::forward<Args>(args)...) << CNRM;
};
template <typename... Args>
void logem(const std::u8string_view& mes, Args&&... args) {
  std::cerr << CMAG << fmt::format(mes, std::forward<Args>(args)...) << CNRM;
};
template <typename... Args>
void logemm(const std::u8string_view& mes, Args&&... args) {
  std::cerr << CBMG << fmt::format(mes, std::forward<Args>(args)...) << CNRM;
};
template <typename... Args>
void logec(const std::u8string_view& mes, Args&&... args) {
  std::cerr << CCYN << fmt::format(mes, std::forward<Args>(args)...) << CNRM;
};
template <typename... Args>
void logecc(const std::u8string_view& mes, Args&&... args) {
  std::cerr << CBCN << fmt::format(mes, std::forward<Args>(args)...) << CNRM;
};

#endif // LOG_HPP
