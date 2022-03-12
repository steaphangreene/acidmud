#include <unistd.h>

#include "log.hpp"

void write_out(const std::u8string_view& out) {
  write(1, out.data(), out.length());
}

void write_err(const std::u8string_view& out) {
  write(2, out.data(), out.length());
}
