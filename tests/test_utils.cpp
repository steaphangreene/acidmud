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

  // Compile Time (conteval) Version
  REQUIRE(crc32c(u8"") == 0x00000000U);
  REQUIRE(crc32c(u8"Warm") == 0x0112DB37U);
  REQUIRE(crc32c(u8"Day Time") == 0x0BDB09F5U);
  REQUIRE(crc32c(u8"abcdefghijklmnopqrstuvwxyz 0123456789-.[]():/") == 0x497FC7AD);
  REQUIRE(crc32c(u8"ABCDEFGHIJKLMNOPQRSTUVWXYZ 0123456789-.[]():/") == 0x497FC7AD);

  // Runtime (inline) Version
  REQUIRE(crc32c(std::u8string_view(u8"")) == 0x00000000U);
  REQUIRE(crc32c(std::u8string_view(u8"Warm")) == 0x0112DB37U);
  REQUIRE(crc32c(std::u8string_view(u8"Day Time")) == 0x0BDB09F5U);
  REQUIRE(
      crc32c(std::u8string_view(u8"abcdefghijklmnopqrstuvwxyz 0123456789-.[]():/")) == 0x497FC7AD);
  REQUIRE(
      crc32c(std::u8string_view(u8"ABCDEFGHIJKLMNOPQRSTUVWXYZ 0123456789-.[]():/")) == 0x497FC7AD);

  res_r = crc32c(std::u8string_view(u8""));
  res_c = crc32c(u8"");
  REQUIRE(res_r == res_c);

  res_r = crc32c(std::u8string_view(u8"Warm"));
  res_c = crc32c(u8"Warm");
  REQUIRE(res_r == res_c);

  res_r = crc32c(std::u8string_view(u8"Day Time"));
  res_c = crc32c(u8"Day Time");
  REQUIRE(res_r == res_c);

  res_r = crc32c(std::u8string_view(u8"abcdefghijklmnopqrstuvwxyz 0123456789-.[]():/"));
  res_c = crc32c(u8"abcdefghijklmnopqrstuvwxyz 0123456789-.[]():/");
  REQUIRE(res_r == res_c);

  res_r = crc32c(std::u8string_view(u8"ABCDEFGHIJKLMNOPQRSTUVWXYZ 0123456789-.[]():/"));
  res_c = crc32c(u8"ABCDEFGHIJKLMNOPQRSTUVWXYZ 0123456789-.[]():/");
  REQUIRE(res_r == res_c);

  char8_t const* test_string1 = GENERATE(
      u8"ABCDEFGHIJKLMNOPQRSTUVWXYZ 0123456789-.[]():/",
      u8"abcdefghijklmnopqrstuvwxyz 0123456789-.[]():/");
  char8_t const* test_string2 = GENERATE(
      u8"ABCDEFGHIJKLMNOPQRSTUVWXYZ 0123456789-.[]():/",
      u8"abcdefghijklmnopqrstuvwxyz 0123456789-.[]():/");
  int off1 = GENERATE(range(0, 8));
  int off2 = GENERATE(range(0, 16));

  res_r = crc32c(std::u8string_view(test_string1 + off1, 45 - off1 - off2));
  res_c = crc32c_c(test_string2 + off1, 45 - off1 - off2, 0xFFFFFFFFU, 0) ^ 0xFFFFFFFFU;
  REQUIRE(res_r == res_c);

  res_r = crc32c(std::u8string_view(test_string1 + off1, off2));
  res_c = crc32c_c(test_string2 + off1, off2, 0xFFFFFFFFU, 0) ^ 0xFFFFFFFFU;
  REQUIRE(res_r == res_c);
}
