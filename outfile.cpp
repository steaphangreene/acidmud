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

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "outfile.hpp"

outfile::outfile(const std::u8string_view filename) {
  std::filesystem::path path(filename);
  fd = open(path.c_str(), O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
  if (fd < 0) {
    perror("outfile::init() open()");
  }
}

void outfile::append(const std::u8string_view& mes) const {
  write(fd, mes.data(), mes.length());
}

outfile::operator bool() const {
  return (fd >= 0);
}

outfile::~outfile() {
  close(fd);
}
