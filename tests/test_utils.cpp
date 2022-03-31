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

#include "test_main.hpp"

#include "../utils.hpp"

TEST_CASE("Test crc32c", "[crc32c]") {
  uint32_t res_r, res_c;

  SECTION("Basic Tests") {
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
        crc32c(std::u8string_view(u8"abcdefghijklmnopqrstuvwxyz 0123456789-.[]():/")) ==
        0x497FC7AD);
    REQUIRE(
        crc32c(std::u8string_view(u8"ABCDEFGHIJKLMNOPQRSTUVWXYZ 0123456789-.[]():/")) ==
        0x497FC7AD);

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
  }

  SECTION("Alignment Tests") {
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
}

TEST_CASE("Test ascii_* functions", "[ascii]") {
  SECTION("Spaces") {
    char8_t ch = GENERATE(' ', '\t', '\n', '\r', '\v');
    REQUIRE(ascii_isspace(ch) == true);
  }
  SECTION("Not Spaces") {
    char8_t ch = GENERATE('\0', range('!', '~'), '~');
    REQUIRE(ascii_isspace(ch) == false);
  }

  SECTION("Letters") {
    char8_t ch = GENERATE(range('A', 'Z'), 'Z', range('a', 'z'), 'z');
    REQUIRE(ascii_isalpha(ch) == true);
  }
  SECTION("Not Letters") {
    char8_t ch = GENERATE(range('\0', 'A'), range('[', 'a'), range('{', '~'), '~');
    REQUIRE(ascii_isalpha(ch) == false);
  }

  SECTION("Uppercase") {
    char8_t ch = GENERATE(range('A', 'Z'), 'Z');
    REQUIRE(ascii_isupper(ch) == true);
  }
  SECTION("Not Uppercase") {
    char8_t ch = GENERATE(range('\0', 'A'), range('[', '~'), '~');
    REQUIRE(ascii_isupper(ch) == false);
  }

  SECTION("Lowercase") {
    char8_t ch = GENERATE(range('a', 'z'), 'z');
    REQUIRE(ascii_islower(ch) == true);
  }
  SECTION("Not Lowercase") {
    char8_t ch = GENERATE(range('\0', 'a'), range('{', '~'), '~');
    REQUIRE(ascii_islower(ch) == false);
  }

  SECTION("Decimal Digits") {
    char8_t ch = GENERATE(range('0', '9'), '9');
    REQUIRE(ascii_isdigit(ch) == true);
  }
  SECTION("Not Decimal Digits") {
    char8_t ch = GENERATE(range('\0', '0'), range(':', '~'), '~');
    REQUIRE(ascii_isdigit(ch) == false);
  }

  SECTION("Hexadecimal Digits") {
    char8_t ch = GENERATE(range('0', '9'), '9', range('A', 'G'), range('a', 'g'));
    REQUIRE(ascii_ishexdigit(ch) == true);
  }
  SECTION("Not Hexadecimal Digits") {
    char8_t ch = GENERATE(range('\0', '0'), range(':', 'A'), range('G', 'a'), range('g', '~'), '~');
    REQUIRE(ascii_ishexdigit(ch) == false);
  }

  SECTION("Alphanumeric") {
    char8_t ch = GENERATE(range('0', '9'), '9', range('A', 'Z'), 'Z', range('a', 'z'), 'z');
    REQUIRE(ascii_isalnum(ch) == true);
  }
  SECTION("Not Alphanumeric") {
    char8_t ch = GENERATE(range('\0', '0'), range(':', 'A'), range('[', 'a'), range('{', '~'), '~');
    REQUIRE(ascii_isalnum(ch) == false);
  }

  SECTION("Uppercase to Lowercase") {
    char8_t ch = GENERATE(range('A', 'Z'), 'Z');
    REQUIRE(ascii_tolower(ch) == static_cast<char8_t>(ch + 32));
  }
  SECTION("Not Uppercase to Lowercase") {
    char8_t ch = GENERATE(range('\0', 'A'), range('[', '~'), '~');
    REQUIRE(ascii_tolower(ch) == ch);
  }

  SECTION("Lowercase to Uppercase") {
    char8_t ch = GENERATE(range('a', 'z'), 'z');
    REQUIRE(ascii_toupper(ch) == static_cast<char8_t>(ch - 32));
  }
  SECTION("Not Lowercase to Uppercase") {
    char8_t ch = GENERATE(range('\0', 'a'), range('{', '~'), '~');
    REQUIRE(ascii_toupper(ch) == ch);
  }

  SECTION("Baseline Case Sanity Check") {
    REQUIRE(ascii_tolower('H') == 'h');
    REQUIRE(ascii_toupper('h') == 'H');
  }
}

TEST_CASE("Test string_view functions", "[string_view]") {
  SECTION("Process Input Tests") {
    std::u8string_view input =
        u8" \t This is the 4th number, like a 6, 3.25, or 0xA4 in a sentence.\r\n";

    skipspace(input);
    REQUIRE(input == u8"This is the 4th number, like a 6, 3.25, or 0xA4 in a sentence.\r\n");

    trim_string(input);
    REQUIRE(input == u8"This is the 4th number, like a 6, 3.25, or 0xA4 in a sentence.");

    input = u8" \t This is the 4th number, like a 6, 3.25, or 0xA4 in a sentence.\r\n";
    trim_string(input);
    REQUIRE(input == u8"This is the 4th number, like a 6, 3.25, or 0xA4 in a sentence.");

    std::u8string_view word = getgraph(input);
    REQUIRE(input == u8" is the 4th number, like a 6, 3.25, or 0xA4 in a sentence.");
    REQUIRE(word == u8"This");

    skipspace(input);
    REQUIRE(input == u8"is the 4th number, like a 6, 3.25, or 0xA4 in a sentence.");

    word = getgraph(input);
    skipspace(input);
    REQUIRE(input == u8"the 4th number, like a 6, 3.25, or 0xA4 in a sentence.");
    REQUIRE(word == u8"is");

    word = getgraph(input);
    skipspace(input);
    REQUIRE(input == u8"4th number, like a 6, 3.25, or 0xA4 in a sentence.");
    REQUIRE(word == u8"the");

    int num = getnum(input);
    word = getgraph(input);
    skipspace(input);
    REQUIRE(input == u8"number, like a 6, 3.25, or 0xA4 in a sentence.");
    REQUIRE(word == u8"4th");
    REQUIRE(num == 4);

    word = getgraph(input);
    skipspace(input);
    REQUIRE(input == u8"like a 6, 3.25, or 0xA4 in a sentence.");
    REQUIRE(word == u8"number,");

    word = getgraph(input);
    skipspace(input);
    REQUIRE(input == u8"a 6, 3.25, or 0xA4 in a sentence.");
    REQUIRE(word == u8"like");

    word = getgraph(input);
    skipspace(input);
    REQUIRE(input == u8"6, 3.25, or 0xA4 in a sentence.");
    REQUIRE(word == u8"a");

    num = nextnum(input);
    skipspace(input);
    REQUIRE(nextchar(input) == ',');
    REQUIRE(nextchar(input) == ' ');
    REQUIRE(input == u8"3.25, or 0xA4 in a sentence.");
    REQUIRE(num == 6);

    double dnum = getdouble(input);
    REQUIRE(dnum == nextdouble(input));
    REQUIRE(nextchar(input) == ',');
    REQUIRE(nextchar(input) == ' ');
    REQUIRE(input == u8"or 0xA4 in a sentence.");
    REQUIRE(dnum == 3.25);

    word = getuntil(input, 'x');
    REQUIRE(input == u8"A4 in a sentence.");
    REQUIRE(word == u8"or 0");

    num = gethex(input);
    REQUIRE(num == nexthex(input));
    skipspace(input);
    REQUIRE(input == u8"in a sentence.");
    REQUIRE(num == 164);

    word = getuntil(input, '.');
    REQUIRE(input == u8"");
    REQUIRE(word == u8"in a sentence");
  }
}
