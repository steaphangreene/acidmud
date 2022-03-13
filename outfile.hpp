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

#ifndef OUTFILE_HPP
#define OUTFILE_HPP

#include <filesystem>
#include <string>

// Replace with C++20 std::format, when widely available
#define FMT_HEADER_ONLY
#include <fmt/format.h>

class outfile {
 public:
  outfile() = delete;
  explicit outfile(const std::u8string_view& filename);
  operator bool() const;
  void append(const std::u8string_view& mes);
  template <typename... Args>
  void append(const std::u8string_view& mes, Args&&... args) {
    append(fmt::vformat(mes, fmt::make_args_checked<Args...>(mes, args...)));
  };
  ~outfile();

 private:
  std::u8string buffer;
  int fd;
};

#endif // OUTFILE_HPP
