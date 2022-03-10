#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include <sys/mman.h>

#include "infile.hpp"

infile::infile(const std::filesystem::directory_entry& filesystem_entry) {
  int fd = open(filesystem_entry.path().c_str(), O_RDONLY);
  if (fd < 0) {
    perror("infile::infile open()");
    rawdata_ = MAP_FAILED;
  } else {
    if (filesystem_entry.file_size() > 0) {
      rawdata_ = mmap(nullptr, filesystem_entry.file_size(), PROT_READ, MAP_PRIVATE, fd, 0);
    }
    if (rawdata_ == MAP_FAILED) {
      perror("infile::infile mmap()");
    } else {
      length_ = filesystem_entry.file_size();
    }
    close(fd);
  }
}

std::u8string_view infile::all() const {
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
