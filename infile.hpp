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

#ifndef INFILE_HPP
#define INFILE_HPP

#include <filesystem>
#include <string>

class infile {
 public:
  infile() = delete;
  explicit infile(const std::filesystem::directory_entry& filesystem_entry);
  explicit infile(const std::u8string_view& filename);
  operator bool() const;
  std::u8string_view all() const;
  ~infile();

 private:
  void init(const std::filesystem::directory_entry& filesystem_entry);
  void* rawdata_ = nullptr;
  size_t length_ = 0;
};

#endif // INFILE_HPP
