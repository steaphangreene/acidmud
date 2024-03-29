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

#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>

// String Functions
void replace_all(
    std::u8string& str,
    const std::u8string& oldt,
    const std::u8string& newt,
    size_t st = 0);

void replace_all(std::u8string& str, const std::u8string& oldt, int newn, size_t st = 0);

size_t skip_line( // IDX of next line start, skipping leading whitespace
    const std::u8string_view& str,
    size_t pos);

size_t prev_line( // IDX of prev line start, skipping leading whitespace
    const std::u8string_view& str,
    size_t pos);

void trim_string(std::u8string& str); // Remove extra whitespace from string
void trim_string(std::u8string_view& str); // Remove extra whitespace from string

bool phrase_match(const std::u8string_view& str, const std::u8string_view& phrase /* lowercase! */);
bool words_match(const std::u8string_view& str, const std::u8string_view& words);

constexpr char8_t ascii_tolower(char8_t chr) {
  return (chr >= 'A' && chr <= 'Z') ? (chr - 'A') + 'a' : chr;
}

constexpr char8_t ascii_toupper(char8_t chr) {
  return (chr >= 'a' && chr <= 'z') ? (chr - 'a') + 'A' : chr;
}

constexpr bool ascii_isupper(char8_t chr) {
  return (chr >= 'A' && chr <= 'Z');
}

constexpr bool ascii_islower(char8_t chr) {
  return (chr >= 'a' && chr <= 'z');
}

constexpr bool ascii_isalpha(char8_t chr) {
  return ((chr | 32) >= 'a' && (chr | 32) <= 'z');
}

constexpr bool ascii_isalnum(char8_t chr) {
  return (((chr | 32) >= 'a' && (chr | 32) <= 'z') || (chr >= '0' && chr <= '9'));
}

constexpr bool ascii_isdigit(char8_t chr) {
  return (chr >= '0' && chr <= '9');
}

constexpr bool ascii_ishexdigit(char8_t chr) {
  return (chr >= '0' && chr <= '9') || ((chr | 32) >= 'a' && (chr | 32) <= 'f');
}

constexpr bool ascii_isspace(char8_t chr) {
  return (chr == ' ' || chr == '\t' || chr == '\n' || chr == '\r' || chr == '\f' || chr == '\v');
}

constexpr bool ascii_isgraph(char8_t chr) {
  return (chr > ' '); // Note: Doesn't check for '\e'(127), nor for non-ascii characters.
}

inline char8_t nextchar(std::u8string_view& line) {
  char8_t ret = line.front();
  line = line.substr(1);
  return ret;
}

inline double nextdouble(std::u8string_view& line) {
  double ret = 0;
  bool negative = false;
  if (line.front() == '-') {
    line = line.substr(1);
    negative = true;
  }
  while (ascii_isdigit(line.front())) {
    ret *= 10;
    ret += (line.front() - '0');
    line = line.substr(1);
  }
  if (line.front() == '.') {
    line = line.substr(1);
    double factor = 10.0;
    while (ascii_isdigit(line.front())) {
      ret += (line.front() - '0') / factor;
      line = line.substr(1);
      factor *= 10.0;
    }
  }
  return (negative) ? -ret : ret;
}

inline double getdouble(const std::u8string_view& in_line) {
  std::u8string_view line(in_line);
  return nextdouble(line);
}

inline int64_t nextnum(std::u8string_view& line) {
  int64_t ret = 0;
  bool negative = false;
  if (line.front() == '-') {
    line = line.substr(1);
    negative = true;
  }
  while (ascii_isdigit(line.front())) {
    ret *= 10;
    ret += (line.front() - '0');
    line = line.substr(1);
  }
  return (negative) ? -ret : ret;
}

inline int64_t getnum(const std::u8string_view& in_line) {
  std::u8string_view line(in_line);
  return nextnum(line);
}

inline uint64_t nexthex(std::u8string_view& line) {
  uint64_t ret = 0;
  while (ascii_ishexdigit(line.front())) {
    ret *= 16;
    ret += (ascii_isdigit(line.front())) ? (line.front() - '0') : ((line.front() | 32) - 'a' + 10);
    line = line.substr(1);
  }
  return ret;
}

inline uint64_t gethex(const std::u8string_view& in_line) {
  std::u8string_view line(in_line);
  return nexthex(line);
}

inline std::u8string_view getuntil(std::u8string_view& line, char8_t delim) {
  auto pos = line.find(delim);
  if (pos == std::u8string_view::npos) {
    auto ret = line;
    line = u8"";
    return ret;
  } else {
    auto ret = line.substr(0, pos);
    line = line.substr(pos + 1);
    return ret;
  }
}

inline std::u8string_view getgraph(std::u8string_view& line) {
  auto pos = std::find_if(line.begin(), line.end(), ascii_isspace);
  if (pos == line.end()) {
    auto ret = line;
    line = u8"";
    return ret;
  } else {
    auto ret = line.substr(0, pos - line.begin());
    line = line.substr(pos - line.begin());
    return ret;
  }
}

inline bool process(std::u8string_view& line, const std::u8string_view& dir) {
  if (line.starts_with(dir)) {
    line = line.substr(dir.length());
    return true;
  } else {
    return false;
  }
}

inline void skipspace(std::u8string_view& line) {
  while (line.length() > 0 && ascii_isspace(line[0])) {
    line = line.substr(1);
  }
}

inline std::u8string_view next_line(const std::u8string_view& file, auto& cursor) {
  if (cursor == std::u8string::npos) {
    return u8"";
  }
  auto next_cursor = file.find_first_of('\n', cursor);

  std::u8string_view ret = file.substr(cursor, next_cursor - cursor);

  if (next_cursor != std::u8string::npos) {
    cursor = file.find_first_not_of('\n', next_cursor);
  } else {
    cursor = next_cursor;
  }
  return ret;
}

inline uint32_t tba_bitvec(const std::u8string_view& val) {
  uint32_t ret = 0;
  for (auto ch : val) {
    if (ascii_isalpha(ch)) {
      ret |= (1 << ((static_cast<uint32_t>(ch) & 0x1FU) - 1));
    }
  }
  if (ret == 0) {
    for (auto ch : val) { // std::stoul() still doesnt support u8string.  :/
      if (ascii_isdigit(ch)) {
        ret *= 10;
        ret += (static_cast<uint32_t>(ch) & 0x0FU);
      }
    }
  }
  return ret;
}

constexpr uint32_t crc32tab[] = {
    0x00000000, 0xF26B8303, 0xE13B70F7, 0x1350F3F4, 0xC79A971F, 0x35F1141C, 0x26A1E7E8, 0xD4CA64EB,
    0x8AD958CF, 0x78B2DBCC, 0x6BE22838, 0x9989AB3B, 0x4D43CFD0, 0xBF284CD3, 0xAC78BF27, 0x5E133C24,
    0x105EC76F, 0xE235446C, 0xF165B798, 0x030E349B, 0xD7C45070, 0x25AFD373, 0x36FF2087, 0xC494A384,
    0x9A879FA0, 0x68EC1CA3, 0x7BBCEF57, 0x89D76C54, 0x5D1D08BF, 0xAF768BBC, 0xBC267848, 0x4E4DFB4B,
    0x20BD8EDE, 0xD2D60DDD, 0xC186FE29, 0x33ED7D2A, 0xE72719C1, 0x154C9AC2, 0x061C6936, 0xF477EA35,
    0xAA64D611, 0x580F5512, 0x4B5FA6E6, 0xB93425E5, 0x6DFE410E, 0x9F95C20D, 0x8CC531F9, 0x7EAEB2FA,
    0x30E349B1, 0xC288CAB2, 0xD1D83946, 0x23B3BA45, 0xF779DEAE, 0x05125DAD, 0x1642AE59, 0xE4292D5A,
    0xBA3A117E, 0x4851927D, 0x5B016189, 0xA96AE28A, 0x7DA08661, 0x8FCB0562, 0x9C9BF696, 0x6EF07595,
    0x417B1DBC, 0xB3109EBF, 0xA0406D4B, 0x522BEE48, 0x86E18AA3, 0x748A09A0, 0x67DAFA54, 0x95B17957,
    0xCBA24573, 0x39C9C670, 0x2A993584, 0xD8F2B687, 0x0C38D26C, 0xFE53516F, 0xED03A29B, 0x1F682198,
    0x5125DAD3, 0xA34E59D0, 0xB01EAA24, 0x42752927, 0x96BF4DCC, 0x64D4CECF, 0x77843D3B, 0x85EFBE38,
    0xDBFC821C, 0x2997011F, 0x3AC7F2EB, 0xC8AC71E8, 0x1C661503, 0xEE0D9600, 0xFD5D65F4, 0x0F36E6F7,
    0x61C69362, 0x93AD1061, 0x80FDE395, 0x72966096, 0xA65C047D, 0x5437877E, 0x4767748A, 0xB50CF789,
    0xEB1FCBAD, 0x197448AE, 0x0A24BB5A, 0xF84F3859, 0x2C855CB2, 0xDEEEDFB1, 0xCDBE2C45, 0x3FD5AF46,
    0x7198540D, 0x83F3D70E, 0x90A324FA, 0x62C8A7F9, 0xB602C312, 0x44694011, 0x5739B3E5, 0xA55230E6,
    0xFB410CC2, 0x092A8FC1, 0x1A7A7C35, 0xE811FF36, 0x3CDB9BDD, 0xCEB018DE, 0xDDE0EB2A, 0x2F8B6829,
    0x82F63B78, 0x709DB87B, 0x63CD4B8F, 0x91A6C88C, 0x456CAC67, 0xB7072F64, 0xA457DC90, 0x563C5F93,
    0x082F63B7, 0xFA44E0B4, 0xE9141340, 0x1B7F9043, 0xCFB5F4A8, 0x3DDE77AB, 0x2E8E845F, 0xDCE5075C,
    0x92A8FC17, 0x60C37F14, 0x73938CE0, 0x81F80FE3, 0x55326B08, 0xA759E80B, 0xB4091BFF, 0x466298FC,
    0x1871A4D8, 0xEA1A27DB, 0xF94AD42F, 0x0B21572C, 0xDFEB33C7, 0x2D80B0C4, 0x3ED04330, 0xCCBBC033,
    0xA24BB5A6, 0x502036A5, 0x4370C551, 0xB11B4652, 0x65D122B9, 0x97BAA1BA, 0x84EA524E, 0x7681D14D,
    0x2892ED69, 0xDAF96E6A, 0xC9A99D9E, 0x3BC21E9D, 0xEF087A76, 0x1D63F975, 0x0E330A81, 0xFC588982,
    0xB21572C9, 0x407EF1CA, 0x532E023E, 0xA145813D, 0x758FE5D6, 0x87E466D5, 0x94B49521, 0x66DF1622,
    0x38CC2A06, 0xCAA7A905, 0xD9F75AF1, 0x2B9CD9F2, 0xFF56BD19, 0x0D3D3E1A, 0x1E6DCDEE, 0xEC064EED,
    0xC38D26C4, 0x31E6A5C7, 0x22B65633, 0xD0DDD530, 0x0417B1DB, 0xF67C32D8, 0xE52CC12C, 0x1747422F,
    0x49547E0B, 0xBB3FFD08, 0xA86F0EFC, 0x5A048DFF, 0x8ECEE914, 0x7CA56A17, 0x6FF599E3, 0x9D9E1AE0,
    0xD3D3E1AB, 0x21B862A8, 0x32E8915C, 0xC083125F, 0x144976B4, 0xE622F5B7, 0xF5720643, 0x07198540,
    0x590AB964, 0xAB613A67, 0xB831C993, 0x4A5A4A90, 0x9E902E7B, 0x6CFBAD78, 0x7FAB5E8C, 0x8DC0DD8F,
    0xE330A81A, 0x115B2B19, 0x020BD8ED, 0xF0605BEE, 0x24AA3F05, 0xD6C1BC06, 0xC5914FF2, 0x37FACCF1,
    0x69E9F0D5, 0x9B8273D6, 0x88D28022, 0x7AB90321, 0xAE7367CA, 0x5C18E4C9, 0x4F48173D, 0xBD23943E,
    0xF36E6F75, 0x0105EC76, 0x12551F82, 0xE03E9C81, 0x34F4F86A, 0xC69F7B69, 0xD5CF889D, 0x27A40B9E,
    0x79B737BA, 0x8BDCB4B9, 0x988C474D, 0x6AE7C44E, 0xBE2DA0A5, 0x4C4623A6, 0x5F16D052, 0xAD7D5351,
};

constexpr uint32_t crc32c_c(const char8_t* str, std::size_t len, uint32_t crc, std::size_t pos) {
  return (pos == len)
      ? crc
      : crc32c_c(str, len, (crc >> 8) ^ crc32tab[(crc ^ ascii_tolower(str[pos])) & 0xFFU], pos + 1);
}

constexpr uint32_t crc32c_c(const char8_t* str, uint32_t crc, std::size_t pos) {
  return (str[pos] == 0)
      ? crc
      : crc32c_c(str, (crc >> 8) ^ crc32tab[(crc ^ ascii_tolower(str[pos])) & 0xFFU], pos + 1);
}

consteval uint32_t crc32c(const char8_t* str) {
  return crc32c_c(str, 0xFFFFFFFFU, 0) ^ 0xFFFFFFFFU;
}

inline uint32_t crc32c_r_lc(const char8_t* str, std::size_t len, uint32_t crc) {
  // First, process any leading bytes not aligned to 8.
  uintptr_t align = (reinterpret_cast<uintptr_t>(str) & 7) ^ 7;
  if (len == 0) {
    return crc;
  } else if (len + 4 < align) {
    align &= 1;
  } else if (len < align) {
    if ((align & 4) != 0) {
      align &= 3;
    } else {
      align &= 1;
    }
  }
  switch (align) {
    case (1): {
      __asm__("crc32b (%1),%0" : "=r"(crc) : "r"(str), "m"(*str), "0"(crc));
    } break;
    case (2): {
      __asm__("crc32w (%1),%0" : "=r"(crc) : "r"(str), "m"(*str), "0"(crc));
    } break;
    case (3): {
      __asm__(
          "crc32b (%1),%0"
          "\n\t"
          "crc32w 1(%1),%0"
          : "=r"(crc)
          : "r"(str), "m"(*str), "0"(crc));
    } break;
    case (4): {
      __asm__("crc32l (%1),%0" : "=r"(crc) : "r"(str), "m"(*str), "0"(crc));
    } break;
    case (5): {
      __asm__(
          "crc32b (%1),%0"
          "\n\t"
          "crc32l 1(%1),%0"
          : "=r"(crc)
          : "r"(str), "m"(*str), "0"(crc));
    } break;
    case (6): {
      __asm__(
          "crc32w (%1),%0"
          "\n\t"
          "crc32l 2(%1),%0"
          : "=r"(crc)
          : "r"(str), "m"(*str), "0"(crc));
    } break;
    case (7): {
      __asm__(
          "crc32b (%1),%0"
          "\n\t"
          "crc32w 1(%1),%0"
          "\n\t"
          "crc32l 3(%1),%0"
          : "=r"(crc)
          : "r"(str), "m"(*str), "0"(crc));
    } break;
  }

  const char8_t* bptr = str + align;

  // Process main body of text 8 aligned bytes at a time
  for (; bptr + 7 < str + len; bptr += 8) {
    uint64_t crcreg = crc;
    __asm__("crc32q (%1),%0" : "=r"(crcreg) : "r"(bptr), "m"(*bptr), "0"(crcreg));
    crc = crcreg;
  }

  // Last, process any trailing bytes not aligned to 8.
  int rem = str + len - bptr;
  switch (rem) {
    case (1): {
      __asm__("crc32b (%1),%0" : "=r"(crc) : "r"(bptr), "m"(*bptr), "0"(crc));
    } break;
    case (2): {
      __asm__("crc32w (%1),%0" : "=r"(crc) : "r"(bptr), "m"(*bptr), "0"(crc));
    } break;
    case (3): {
      __asm__(
          "crc32w (%1),%0"
          "\n\t"
          "crc32b 2(%1),%0"
          : "=r"(crc)
          : "r"(bptr), "m"(*bptr), "0"(crc));
    } break;
    case (4): {
      __asm__("crc32l (%1),%0" : "=r"(crc) : "r"(bptr), "m"(*bptr), "0"(crc));
    } break;
    case (5): {
      __asm__(
          "crc32l (%1),%0"
          "\n\t"
          "crc32b 4(%1),%0"
          : "=r"(crc)
          : "r"(bptr), "m"(*bptr), "0"(crc));
    } break;
    case (6): {
      __asm__(
          "crc32l (%1),%0"
          "\n\t"
          "crc32w 4(%1),%0"
          : "=r"(crc)
          : "r"(bptr), "m"(*bptr), "0"(crc));
    } break;
    case (7): {
      __asm__(
          "crc32l (%1),%0"
          "\n\t"
          "crc32w 4(%1),%0"
          "\n\t"
          "crc32b 6(%1),%0"
          : "=r"(crc)
          : "r"(bptr), "m"(*bptr), "0"(crc));
    } break;
  }

  return crc;
}

inline uint32_t crc32c_r_mc(const std::u8string_view& str, uint32_t crc) {
  for (const auto& b : str) {
    crc = (crc >> 8) ^ crc32tab[(crc ^ ascii_tolower(b)) & 0xFFU];
  }
  return crc;
}

inline uint32_t crc32c_r(const std::u8string_view& str, uint32_t crc) {
  uint32_t ret = crc32c_r_lc(str.data(), str.length(), crc);
  bool uppercase_free = true;
  for (const auto& b : str) {
    if (ascii_isupper(b)) {
      uppercase_free = false;
    }
  }
  if (!uppercase_free) [[unlikely]] {
    ret = crc32c_r_mc(str, crc);
  }
  return ret;
}

inline uint32_t crc32c(const char8_t* str, std::size_t len) {
  return crc32c_r(std::u8string_view(str, len), 0xFFFFFFFFU) ^ 0xFFFFFFFFU;
}

inline uint32_t crc32c(const std::u8string_view& str) {
  return crc32c_r(str, 0xFFFFFFFFU) ^ 0xFFFFFFFFU;
}

// Assert A Few Independently Confirmed CRC32C Hashes
static_assert(crc32c(u8"") == 0x00000000U);
static_assert(crc32c(u8"Warm") == 0x0112DB37U);
static_assert(crc32c(u8"Day Time") == 0x0BDB09F5U);
static_assert(crc32c(u8"abcdefghijklmnopqrstuvwxyz 0123456789-.[]():/") == 0x497FC7AD);
static_assert(crc32c(u8"ABCDEFGHIJKLMNOPQRSTUVWXYZ 0123456789-.[]():/") == 0x497FC7AD);

#endif // UTILS_HPP
