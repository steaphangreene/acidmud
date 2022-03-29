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

#include "../darr32.hpp"
#include "../darr64.hpp"

TEST_CASE("Basic DArr64 Tests", "[DArr64]") {
  uint64_t val = 0;

  DArr64<uint64_t> arr1;
  DArr64<uint64_t, 3> arr3;
  DArr64<uint64_t, 7> arr7;
  REQUIRE(arr1.size() == 0);
  REQUIRE(arr3.size() == 0);
  REQUIRE(arr7.size() == 0);
  REQUIRE(arr1.capacity() == 1);
  REQUIRE(arr3.capacity() == 3);
  REQUIRE(arr7.capacity() == 7);

  arr1.push_back(++val);
  arr3.push_back(++val);
  arr7.push_back(++val);
  REQUIRE(arr1.size() == 1);
  REQUIRE(arr3.size() == 1);
  REQUIRE(arr7.size() == 1);
  REQUIRE(arr1.capacity() == 1);
  REQUIRE(arr3.capacity() == 3);
  REQUIRE(arr7.capacity() == 7);

  arr1.push_back(++val);
  arr3.push_back(++val);
  arr7.push_back(++val);
  REQUIRE(arr1.size() == 2);
  REQUIRE(arr3.size() == 2);
  REQUIRE(arr7.size() == 2);
  REQUIRE(arr1.capacity() == 2);
  REQUIRE(arr3.capacity() == 3);
  REQUIRE(arr7.capacity() == 7);

  arr1.push_back(++val);
  arr3.push_back(++val);
  arr7.push_back(++val);
  REQUIRE(arr1.size() == 3);
  REQUIRE(arr3.size() == 3);
  REQUIRE(arr7.size() == 3);
  REQUIRE(arr1.capacity() == 4);
  REQUIRE(arr3.capacity() == 3);
  REQUIRE(arr7.capacity() == 7);

  arr1.push_back(++val);
  arr3.push_back(++val);
  arr7.push_back(++val);
  REQUIRE(arr1.size() == 4);
  REQUIRE(arr3.size() == 4);
  REQUIRE(arr7.size() == 4);
  REQUIRE(arr1.capacity() == 4);
  REQUIRE(arr3.capacity() == 4);
  REQUIRE(arr7.capacity() == 7);

  arr1.push_back(++val);
  arr3.push_back(++val);
  arr7.push_back(++val);
  REQUIRE(arr1.size() == 5);
  REQUIRE(arr3.size() == 5);
  REQUIRE(arr7.size() == 5);
  REQUIRE(arr1.capacity() == 8);
  REQUIRE(arr3.capacity() == 8);
  REQUIRE(arr7.capacity() == 7);

  arr1.push_back(++val);
  arr3.push_back(++val);
  arr7.push_back(++val);
  REQUIRE(arr1.size() == 6);
  REQUIRE(arr3.size() == 6);
  REQUIRE(arr7.size() == 6);
  REQUIRE(arr1.capacity() == 8);
  REQUIRE(arr3.capacity() == 8);
  REQUIRE(arr7.capacity() == 7);

  arr1.push_back(++val);
  arr3.push_back(++val);
  arr7.push_back(++val);
  REQUIRE(arr1.size() == 7);
  REQUIRE(arr3.size() == 7);
  REQUIRE(arr7.size() == 7);
  REQUIRE(arr1.capacity() == 8);
  REQUIRE(arr3.capacity() == 8);
  REQUIRE(arr7.capacity() == 7);

  arr1.push_back(++val);
  arr3.push_back(++val);
  arr7.push_back(++val);
  REQUIRE(arr1.size() == 8);
  REQUIRE(arr3.size() == 8);
  REQUIRE(arr7.size() == 8);
  REQUIRE(arr1.capacity() == 8);
  REQUIRE(arr3.capacity() == 8);
  REQUIRE(arr7.capacity() == 8);

  arr1.push_back(++val);
  arr3.push_back(++val);
  arr7.push_back(++val);
  REQUIRE(arr1.size() == 9);
  REQUIRE(arr3.size() == 9);
  REQUIRE(arr7.size() == 9);
  REQUIRE(arr1.capacity() == 16);
  REQUIRE(arr3.capacity() == 16);
  REQUIRE(arr7.capacity() == 16);

  REQUIRE(arr1[0] == 1);
  REQUIRE(arr1[1] == 4);
  REQUIRE(arr1[2] == 7);
  REQUIRE(arr1[3] == 10);
  REQUIRE(arr1[4] == 13);
  REQUIRE(arr1[5] == 16);
  REQUIRE(arr1[6] == 19);
  REQUIRE(arr1[7] == 22);
  REQUIRE(arr1[8] == 25);

  REQUIRE(arr3[0] == 2);
  REQUIRE(arr3[1] == 5);
  REQUIRE(arr3[2] == 8);
  REQUIRE(arr3[3] == 11);
  REQUIRE(arr3[4] == 14);
  REQUIRE(arr3[5] == 17);
  REQUIRE(arr3[6] == 20);
  REQUIRE(arr3[7] == 23);
  REQUIRE(arr3[8] == 26);

  REQUIRE(arr7[0] == 3);
  REQUIRE(arr7[1] == 6);
  REQUIRE(arr7[2] == 9);
  REQUIRE(arr7[3] == 12);
  REQUIRE(arr7[4] == 15);
  REQUIRE(arr7[5] == 18);
  REQUIRE(arr7[6] == 21);
  REQUIRE(arr7[7] == 24);
  REQUIRE(arr7[8] == 27);

  arr1.clear();
  arr3.clear();
  arr7.clear();
  REQUIRE(arr1.size() == 0);
  REQUIRE(arr3.size() == 0);
  REQUIRE(arr7.size() == 0);
  REQUIRE(arr1.capacity() == 1);
  REQUIRE(arr3.capacity() == 3);
  REQUIRE(arr7.capacity() == 7);
}

TEST_CASE("Basic DArr32 Tests", "[DArr32]") {
  uint32_t val = 0;

  DArr32<uint32_t> arr3;
  DArr32<uint32_t, 7> arr7;
  DArr32<uint32_t, 15> arr15;
  REQUIRE(arr3.size() == 0);
  REQUIRE(arr7.size() == 0);
  REQUIRE(arr15.size() == 0);
  REQUIRE(arr3.capacity() == 3);
  REQUIRE(arr7.capacity() == 7);
  REQUIRE(arr15.capacity() == 15);

  arr3.push_back(++val);
  arr7.push_back(++val);
  arr15.push_back(++val);
  REQUIRE(arr3.size() == 1);
  REQUIRE(arr7.size() == 1);
  REQUIRE(arr15.size() == 1);
  REQUIRE(arr3.capacity() == 3);
  REQUIRE(arr7.capacity() == 7);
  REQUIRE(arr15.capacity() == 15);

  arr3.push_back(++val);
  arr7.push_back(++val);
  arr15.push_back(++val);
  REQUIRE(arr3.size() == 2);
  REQUIRE(arr7.size() == 2);
  REQUIRE(arr15.size() == 2);
  REQUIRE(arr3.capacity() == 3);
  REQUIRE(arr7.capacity() == 7);
  REQUIRE(arr15.capacity() == 15);

  arr3.push_back(++val);
  arr7.push_back(++val);
  arr15.push_back(++val);
  REQUIRE(arr3.size() == 3);
  REQUIRE(arr7.size() == 3);
  REQUIRE(arr15.size() == 3);
  REQUIRE(arr3.capacity() == 3);
  REQUIRE(arr7.capacity() == 7);
  REQUIRE(arr15.capacity() == 15);

  arr3.push_back(++val);
  arr7.push_back(++val);
  arr15.push_back(++val);
  REQUIRE(arr3.size() == 4);
  REQUIRE(arr7.size() == 4);
  REQUIRE(arr15.size() == 4);
  REQUIRE(arr3.capacity() == 4);
  REQUIRE(arr7.capacity() == 7);
  REQUIRE(arr15.capacity() == 15);

  arr3.push_back(++val);
  arr7.push_back(++val);
  arr15.push_back(++val);
  REQUIRE(arr3.size() == 5);
  REQUIRE(arr7.size() == 5);
  REQUIRE(arr15.size() == 5);
  REQUIRE(arr3.capacity() == 8);
  REQUIRE(arr7.capacity() == 7);
  REQUIRE(arr15.capacity() == 15);

  arr3.push_back(++val);
  arr7.push_back(++val);
  arr15.push_back(++val);
  REQUIRE(arr3.size() == 6);
  REQUIRE(arr7.size() == 6);
  REQUIRE(arr15.size() == 6);
  REQUIRE(arr3.capacity() == 8);
  REQUIRE(arr7.capacity() == 7);
  REQUIRE(arr15.capacity() == 15);

  arr3.push_back(++val);
  arr7.push_back(++val);
  arr15.push_back(++val);
  REQUIRE(arr3.size() == 7);
  REQUIRE(arr7.size() == 7);
  REQUIRE(arr15.size() == 7);
  REQUIRE(arr3.capacity() == 8);
  REQUIRE(arr7.capacity() == 7);
  REQUIRE(arr15.capacity() == 15);

  arr3.push_back(++val);
  arr7.push_back(++val);
  arr15.push_back(++val);
  REQUIRE(arr3.size() == 8);
  REQUIRE(arr7.size() == 8);
  REQUIRE(arr15.size() == 8);
  REQUIRE(arr3.capacity() == 8);
  REQUIRE(arr7.capacity() == 8);
  REQUIRE(arr15.capacity() == 15);

  arr3.push_back(++val);
  arr7.push_back(++val);
  arr15.push_back(++val);
  REQUIRE(arr3.size() == 9);
  REQUIRE(arr7.size() == 9);
  REQUIRE(arr15.size() == 9);
  REQUIRE(arr3.capacity() == 16);
  REQUIRE(arr7.capacity() == 16);
  REQUIRE(arr15.capacity() == 15);

  REQUIRE(arr3[0] == 1);
  REQUIRE(arr3[1] == 4);
  REQUIRE(arr3[2] == 7);
  REQUIRE(arr3[3] == 10);
  REQUIRE(arr3[4] == 13);
  REQUIRE(arr3[5] == 16);
  REQUIRE(arr3[6] == 19);
  REQUIRE(arr3[7] == 22);
  REQUIRE(arr3[8] == 25);

  REQUIRE(arr7[0] == 2);
  REQUIRE(arr7[1] == 5);
  REQUIRE(arr7[2] == 8);
  REQUIRE(arr7[3] == 11);
  REQUIRE(arr7[4] == 14);
  REQUIRE(arr7[5] == 17);
  REQUIRE(arr7[6] == 20);
  REQUIRE(arr7[7] == 23);
  REQUIRE(arr7[8] == 26);

  REQUIRE(arr15[0] == 3);
  REQUIRE(arr15[1] == 6);
  REQUIRE(arr15[2] == 9);
  REQUIRE(arr15[3] == 12);
  REQUIRE(arr15[4] == 15);
  REQUIRE(arr15[5] == 18);
  REQUIRE(arr15[6] == 21);
  REQUIRE(arr15[7] == 24);
  REQUIRE(arr15[8] == 27);

  arr3.clear();
  arr7.clear();
  arr15.clear();
  REQUIRE(arr3.size() == 0);
  REQUIRE(arr7.size() == 0);
  REQUIRE(arr15.size() == 0);
  REQUIRE(arr3.capacity() == 16);
  REQUIRE(arr7.capacity() == 16);
  REQUIRE(arr15.capacity() == 15);
}

// Simple Total-Counting Wrapper Class
template <typename B>
class counted {
 public:
  counted() {
    ++total;
  };
  counted(const B& in) {
    data = in;
    ++total;
  };
  counted(B&& in) {
    data = std::move(in);
    ++total;
  };
  counted(const counted& in) {
    data = in.data;
    ++total;
  };
  counted(counted&& in) {
    data = std::move(in.data);
    ++total;
  };
  void operator=(const counted& in) {
    data = in.data;
  };
  void operator=(counted&& in) {
    data = std::move(in.data);
  };
  ~counted() {
    --total;
  };
  static size_t total;

 private:
  B data;
};

template <>
size_t counted<int64_t>::total = 0;
TEST_CASE("Traced DArr64 Tests", "[DArr64]") {
  DArr64<counted<int64_t>> arr;

  arr.push_back(1);
  REQUIRE(counted<int64_t>::total == 1);

  arr.pop_back();
  REQUIRE(counted<int64_t>::total == 0);

  arr.push_back(2);
  arr.push_back(3);
  arr.push_back(4);
  REQUIRE(counted<int64_t>::total == 3);

  arr.pop_back();
  REQUIRE(counted<int64_t>::total == 2);

  arr.clear();
  REQUIRE(counted<int64_t>::total == 0);

  arr.reserve(21);
  REQUIRE(counted<int64_t>::total == 0);

  arr.push_back(1);
  arr.push_back(2);
  arr.push_back(3);
  arr.push_back(4);
  arr.push_back(5);
  arr.push_back(6);
  arr.push_back(7);
  arr.push_back(8);
  arr.push_back(9);
  arr.push_back(10);
  arr.push_back(11);
  arr.push_back(12);
  arr.push_back(13);
  arr.push_back(14);
  arr.push_back(15);
  arr.push_back(16);
  arr.push_back(17);
  arr.push_back(18);
  arr.push_back(19);
  arr.push_back(20);
  REQUIRE(counted<int64_t>::total == 20);

  arr.push_back(21);
  REQUIRE(counted<int64_t>::total == 21);

  arr.push_back(22);
  REQUIRE(counted<int64_t>::total == 22);

  arr.push_back(23);
  REQUIRE(counted<int64_t>::total == 23);
}
