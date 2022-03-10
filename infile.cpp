#include <fstream>

#include "infile.hpp"

infile::infile(const std::filesystem::directory_entry& filesystem_entry)
    : filedata_(filesystem_entry.file_size(), 0) {
  std::ifstream def_file(filesystem_entry.path(), std::ios::in | std::ios::binary);
  if (!def_file.is_open()) {
    filedata_ = u8"";
  } else {
    def_file.read(reinterpret_cast<char*>(filedata_.data()), filesystem_entry.file_size());
    def_file.close();
  }
}

std::u8string_view infile::all() const {
  return filedata_;
}

infile::operator bool() const {
  return (filedata_.length() != 0);
}

infile::~infile() {}
