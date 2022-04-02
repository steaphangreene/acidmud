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

#include <string>
#include <utility>

// Replace with C++20 std::format, when widely available
#define FMT_HEADER_ONLY
#include <fmt/format.h>

#include "color.hpp"

void write_out(const std::u8string_view&);
void write_err(const std::u8string_view&);

template <typename S, typename... Args>
void logo(const S& mes, Args&&... args) {
  write_out(
      fmt::format(u8"{}\n", fmt::vformat(mes, fmt::make_args_checked<Args...>(mes, args...))));
};
template <typename S, typename... Args>
void loge(const S& mes, Args&&... args) {
  write_err(
      fmt::format(u8"{}\n", fmt::vformat(mes, fmt::make_args_checked<Args...>(mes, args...))));
};
template <typename S, typename... Args>
void loger(const S& mes, Args&&... args) {
  write_err(fmt::format(
      CRED u8"{}" CNRM u8"\n", fmt::vformat(mes, fmt::make_args_checked<Args...>(mes, args...))));
};
template <typename S, typename... Args>
void logerr(const S& mes, Args&&... args) {
  write_err(fmt::format(
      CBRD u8"{}" CNRM u8"\n", fmt::vformat(mes, fmt::make_args_checked<Args...>(mes, args...))));
};
template <typename S, typename... Args>
void logeg(const S& mes, Args&&... args) {
  write_err(fmt::format(
      CGRN u8"{}" CNRM u8"\n", fmt::vformat(mes, fmt::make_args_checked<Args...>(mes, args...))));
};
template <typename S, typename... Args>
void logegg(const S& mes, Args&&... args) {
  write_err(fmt::format(
      CBGR u8"{}" CNRM u8"\n", fmt::vformat(mes, fmt::make_args_checked<Args...>(mes, args...))));
};
template <typename S, typename... Args>
void logeb(const S& mes, Args&&... args) {
  write_err(fmt::format(
      CBLU u8"{}" CNRM u8"\n", fmt::vformat(mes, fmt::make_args_checked<Args...>(mes, args...))));
};
template <typename S, typename... Args>
void logebb(const S& mes, Args&&... args) {
  write_err(fmt::format(
      CBBL u8"{}" CNRM u8"\n", fmt::vformat(mes, fmt::make_args_checked<Args...>(mes, args...))));
};
template <typename S, typename... Args>
void logey(const S& mes, Args&&... args) {
  write_err(fmt::format(
      CYEL u8"{}" CNRM u8"\n", fmt::vformat(mes, fmt::make_args_checked<Args...>(mes, args...))));
};
template <typename S, typename... Args>
void logeyy(const S& mes, Args&&... args) {
  write_err(fmt::format(
      CBYL u8"{}" CNRM u8"\n", fmt::vformat(mes, fmt::make_args_checked<Args...>(mes, args...))));
};
template <typename S, typename... Args>
void logem(const S& mes, Args&&... args) {
  write_err(fmt::format(
      CMAG u8"{}" CNRM u8"\n", fmt::vformat(mes, fmt::make_args_checked<Args...>(mes, args...))));
};
template <typename S, typename... Args>
void logemm(const S& mes, Args&&... args) {
  write_err(fmt::format(
      CBMG u8"{}" CNRM u8"\n", fmt::vformat(mes, fmt::make_args_checked<Args...>(mes, args...))));
};
template <typename S, typename... Args>
void logec(const S& mes, Args&&... args) {
  write_err(fmt::format(
      CCYN u8"{}" CNRM u8"\n", fmt::vformat(mes, fmt::make_args_checked<Args...>(mes, args...))));
};
template <typename S, typename... Args>
void logecc(const S& mes, Args&&... args) {
  write_err(fmt::format(
      CBCN u8"{}" CNRM u8"\n", fmt::vformat(mes, fmt::make_args_checked<Args...>(mes, args...))));
};

#endif // LOG_HPP
