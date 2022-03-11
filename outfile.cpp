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
