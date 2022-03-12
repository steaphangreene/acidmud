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

#include <cstdio>

#include <fcntl.h>
#include <unistd.h>

#include "outfile.hpp"

outfile::outfile(const std::u8string_view filename) {
  std::filesystem::path path(filename);
  fd = open(path.c_str(), O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
  if (fd < 0) {
    perror("outfile::init() open()");
  } else {
    buffer.reserve(65536);
  }
}

void outfile::append(const std::u8string_view& mes) {
  buffer += mes;
  while (buffer.length() > 4096) {
    write(fd, buffer.data(), 4096);
    buffer = buffer.substr(4096);
  }
}

outfile::operator bool() const {
  return (fd >= 0);
}

outfile::~outfile() {
  write(fd, buffer.data(), buffer.length());
  close(fd);
}
