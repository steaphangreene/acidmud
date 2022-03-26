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

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include "../utils.hpp"

TEST_CASE("Test crc32c", "[crc32c]") {
  uint32_t res_r, res_c;

  res_r = crc32c_r(std::u8string_view(u8"", 0), 0xFFFFFFFFU) ^ 0xFFFFFFFFU;
  res_c = crc32c_c(u8"", 0, 0xFFFFFFFFU, 0) ^ 0xFFFFFFFFU;
  REQUIRE(res_r == res_c);

  res_r = crc32c_r(std::u8string_view(u8"Warm", 4), 0xFFFFFFFFU) ^ 0xFFFFFFFFU;
  res_c = crc32c_c(u8"Warm", 4, 0xFFFFFFFFU, 0) ^ 0xFFFFFFFFU;
  REQUIRE(res_r == res_c);

  res_r = crc32c_r(std::u8string_view(u8"Day Time", 8), 0xFFFFFFFFU) ^ 0xFFFFFFFFU;
  res_c = crc32c_c(u8"Day Time", 8, 0xFFFFFFFFU, 0) ^ 0xFFFFFFFFU;
  REQUIRE(res_r == res_c);

  res_r =
      crc32c_r(
          std::u8string_view(u8"abcdefghijklmnopqrstuvwxyz 0123456789-.[]():/", 45), 0xFFFFFFFFU) ^
      0xFFFFFFFFU;
  res_c =
      crc32c_c(u8"abcdefghijklmnopqrstuvwxyz 0123456789-.[]():/", 45, 0xFFFFFFFFU, 0) ^ 0xFFFFFFFFU;
  REQUIRE(res_r == res_c);

  res_r =
      crc32c_r(
          std::u8string_view(u8"ABCDEFGHIJKLMNOPQRSTUVWXYZ 0123456789-.[]():/", 45), 0xFFFFFFFFU) ^
      0xFFFFFFFFU;
  res_c =
      crc32c_c(u8"ABCDEFGHIJKLMNOPQRSTUVWXYZ 0123456789-.[]():/", 45, 0xFFFFFFFFU, 0) ^ 0xFFFFFFFFU;
  REQUIRE(res_r == res_c);

  char8_t const* test_string = u8"ABCDEFGHIJKLMNOPQRSTUVWXYZ 0123456789-.[]():/";
  for (int off = 0; off < 8; ++off) {
    res_r = crc32c_r(std::u8string_view(test_string + off, 45 - off), 0xFFFFFFFFU) ^ 0xFFFFFFFFU;
    res_c = crc32c_c(test_string + off, 45 - off, 0xFFFFFFFFU, 0) ^ 0xFFFFFFFFU;
    REQUIRE(res_r == res_c);
  }

  for (int off = 1; off < 8; ++off) {
    res_r = crc32c_r(std::u8string_view(test_string, 45 - off), 0xFFFFFFFFU) ^ 0xFFFFFFFFU;
    res_c = crc32c_c(test_string, 45 - off, 0xFFFFFFFFU, 0) ^ 0xFFFFFFFFU;
    REQUIRE(res_r == res_c);
  }
}
