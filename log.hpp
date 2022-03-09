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

template <typename S, typename... Args>
void logo(const S& mes, Args&&... args) {
  std::cout << fmt::vformat(mes, fmt::make_args_checked<Args...>(mes, args...));
};
template <typename S, typename... Args>
void loge(const S& mes, Args&&... args) {
  std::cerr << fmt::vformat(mes, fmt::make_args_checked<Args...>(mes, args...));
};
template <typename S, typename... Args>
void loger(const S& mes, Args&&... args) {
  std::cerr << CRED << fmt::vformat(mes, fmt::make_args_checked<Args...>(mes, args...)) << CNRM;
};
template <typename S, typename... Args>
void logerr(const S& mes, Args&&... args) {
  std::cerr << CBRD << fmt::vformat(mes, fmt::make_args_checked<Args...>(mes, args...)) << CNRM;
};
template <typename S, typename... Args>
void logeg(const S& mes, Args&&... args) {
  std::cerr << CGRN << fmt::vformat(mes, fmt::make_args_checked<Args...>(mes, args...)) << CNRM;
};
template <typename S, typename... Args>
void logegg(const S& mes, Args&&... args) {
  std::cerr << CBGR << fmt::vformat(mes, fmt::make_args_checked<Args...>(mes, args...)) << CNRM;
};
template <typename S, typename... Args>
void logeb(const S& mes, Args&&... args) {
  std::cerr << CBLU << fmt::vformat(mes, fmt::make_args_checked<Args...>(mes, args...)) << CNRM;
};
template <typename S, typename... Args>
void logebb(const S& mes, Args&&... args) {
  std::cerr << CBBL << fmt::vformat(mes, fmt::make_args_checked<Args...>(mes, args...)) << CNRM;
};
template <typename S, typename... Args>
void logey(const S& mes, Args&&... args) {
  std::cerr << CYEL << fmt::vformat(mes, fmt::make_args_checked<Args...>(mes, args...)) << CNRM;
};
template <typename S, typename... Args>
void logeyy(const S& mes, Args&&... args) {
  std::cerr << CBYL << fmt::vformat(mes, fmt::make_args_checked<Args...>(mes, args...)) << CNRM;
};
template <typename S, typename... Args>
void logem(const S& mes, Args&&... args) {
  std::cerr << CMAG << fmt::vformat(mes, fmt::make_args_checked<Args...>(mes, args...)) << CNRM;
};
template <typename S, typename... Args>
void logemm(const S& mes, Args&&... args) {
  std::cerr << CBMG << fmt::vformat(mes, fmt::make_args_checked<Args...>(mes, args...)) << CNRM;
};
template <typename S, typename... Args>
void logec(const S& mes, Args&&... args) {
  std::cerr << CCYN << fmt::vformat(mes, fmt::make_args_checked<Args...>(mes, args...)) << CNRM;
};
template <typename S, typename... Args>
void logecc(const S& mes, Args&&... args) {
  std::cerr << CBCN << fmt::vformat(mes, fmt::make_args_checked<Args...>(mes, args...)) << CNRM;
};

#endif // LOG_HPP
