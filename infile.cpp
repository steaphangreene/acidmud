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

#include <sys/mman.h>

#include "infile.hpp"

void infile::init(const std::filesystem::directory_entry& filesystem_entry) {
  int fd = ::open(filesystem_entry.path().c_str(), O_RDONLY | O_CLOEXEC);
  if (fd < 0) {
    ::perror("infile::init() open()");
    rawdata_ = MAP_FAILED;
  } else {
    if (filesystem_entry.file_size() > 0) {
      rawdata_ = mmap(nullptr, filesystem_entry.file_size(), PROT_READ, MAP_PRIVATE, fd, 0);
    }
    if (rawdata_ == MAP_FAILED) {
      perror("infile::init() mmap()");
    } else {
      length_ = filesystem_entry.file_size();
      (*(static_cast<std::u8string_view*>(this))) = // Yuk!
          std::u8string_view(static_cast<char8_t*>(rawdata_), length_);
    }
    ::close(fd);
  }
}

infile::infile(const std::filesystem::directory_entry& filesystem_entry) {
  init(filesystem_entry);
}

infile::infile(const std::u8string_view& filename) {
  init(std::filesystem::directory_entry(filename));
}

auto infile::all() const -> std::u8string_view {
  if (rawdata_ == nullptr || rawdata_ == MAP_FAILED || length_ == 0) {
    return u8"";
  }
  return std::u8string_view(static_cast<char8_t*>(rawdata_), length_);
}

infile::operator bool() const {
  return (rawdata_ != MAP_FAILED);
}

infile::~infile() {
  if (rawdata_ != nullptr && rawdata_ != MAP_FAILED && length_ > 0) {
    if (munmap(rawdata_, length_) < 0) {
      perror("infile::~infile() munmap()");
    }
  }
}
