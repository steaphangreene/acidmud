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

TEST_CASE("Test DArr64", "[DArr64]") {
  DArr64<uint64_t> arr1;
  DArr64<uint64_t, 3> arr3;
  DArr64<uint64_t, 7> arr7;
  REQUIRE(arr1.size() == 0);
  REQUIRE(arr3.size() == 0);
  REQUIRE(arr7.size() == 0);
  REQUIRE(arr1.capacity() == 1);
  REQUIRE(arr3.capacity() == 3);
  REQUIRE(arr7.capacity() == 7);

  arr1.push_back(1);
  arr3.push_back(1);
  arr7.push_back(1);
  REQUIRE(arr1.size() == 1);
  REQUIRE(arr3.size() == 1);
  REQUIRE(arr7.size() == 1);
  REQUIRE(arr1.capacity() == 1);
  REQUIRE(arr3.capacity() == 3);
  REQUIRE(arr7.capacity() == 7);

  arr1.push_back(1);
  arr3.push_back(1);
  arr7.push_back(1);
  REQUIRE(arr1.size() == 2);
  REQUIRE(arr3.size() == 2);
  REQUIRE(arr7.size() == 2);
  REQUIRE(arr1.capacity() == 2);
  REQUIRE(arr3.capacity() == 3);
  REQUIRE(arr7.capacity() == 7);

  arr1.push_back(1);
  arr3.push_back(1);
  arr7.push_back(1);
  REQUIRE(arr1.size() == 3);
  REQUIRE(arr3.size() == 3);
  REQUIRE(arr7.size() == 3);
  REQUIRE(arr1.capacity() == 4);
  REQUIRE(arr3.capacity() == 3);
  REQUIRE(arr7.capacity() == 7);

  arr1.push_back(1);
  arr3.push_back(1);
  arr7.push_back(1);
  REQUIRE(arr1.size() == 4);
  REQUIRE(arr3.size() == 4);
  REQUIRE(arr7.size() == 4);
  REQUIRE(arr1.capacity() == 4);
  REQUIRE(arr3.capacity() == 4);
  REQUIRE(arr7.capacity() == 7);

  arr1.push_back(1);
  arr3.push_back(1);
  arr7.push_back(1);
  REQUIRE(arr1.size() == 5);
  REQUIRE(arr3.size() == 5);
  REQUIRE(arr7.size() == 5);
  REQUIRE(arr1.capacity() == 8);
  REQUIRE(arr3.capacity() == 8);
  REQUIRE(arr7.capacity() == 7);

  arr1.push_back(1);
  arr3.push_back(1);
  arr7.push_back(1);
  REQUIRE(arr1.size() == 6);
  REQUIRE(arr3.size() == 6);
  REQUIRE(arr7.size() == 6);
  REQUIRE(arr1.capacity() == 8);
  REQUIRE(arr3.capacity() == 8);
  REQUIRE(arr7.capacity() == 7);

  arr1.push_back(1);
  arr3.push_back(1);
  arr7.push_back(1);
  REQUIRE(arr1.size() == 7);
  REQUIRE(arr3.size() == 7);
  REQUIRE(arr7.size() == 7);
  REQUIRE(arr1.capacity() == 8);
  REQUIRE(arr3.capacity() == 8);
  REQUIRE(arr7.capacity() == 7);

  arr1.push_back(1);
  arr3.push_back(1);
  arr7.push_back(1);
  REQUIRE(arr1.size() == 8);
  REQUIRE(arr3.size() == 8);
  REQUIRE(arr7.size() == 8);
  REQUIRE(arr1.capacity() == 8);
  REQUIRE(arr3.capacity() == 8);
  REQUIRE(arr7.capacity() == 8);

  arr1.push_back(1);
  arr3.push_back(1);
  arr7.push_back(1);
  REQUIRE(arr1.size() == 9);
  REQUIRE(arr3.size() == 9);
  REQUIRE(arr7.size() == 9);
  REQUIRE(arr1.capacity() == 16);
  REQUIRE(arr3.capacity() == 16);
  REQUIRE(arr7.capacity() == 16);

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

TEST_CASE("Test DArr32", "[DArr32]") {
  DArr32<uint32_t> arr3;
  DArr32<uint32_t, 7> arr7;
  DArr32<uint32_t, 15> arr15;
  REQUIRE(arr3.size() == 0);
  REQUIRE(arr7.size() == 0);
  REQUIRE(arr15.size() == 0);
  REQUIRE(arr3.capacity() == 3);
  REQUIRE(arr7.capacity() == 7);
  REQUIRE(arr15.capacity() == 15);

  arr3.push_back(1);
  arr7.push_back(1);
  arr15.push_back(1);
  REQUIRE(arr3.size() == 1);
  REQUIRE(arr7.size() == 1);
  REQUIRE(arr15.size() == 1);
  REQUIRE(arr3.capacity() == 3);
  REQUIRE(arr7.capacity() == 7);
  REQUIRE(arr15.capacity() == 15);

  arr3.push_back(1);
  arr7.push_back(1);
  arr15.push_back(1);
  REQUIRE(arr3.size() == 2);
  REQUIRE(arr7.size() == 2);
  REQUIRE(arr15.size() == 2);
  REQUIRE(arr3.capacity() == 3);
  REQUIRE(arr7.capacity() == 7);
  REQUIRE(arr15.capacity() == 15);

  arr3.push_back(1);
  arr7.push_back(1);
  arr15.push_back(1);
  REQUIRE(arr3.size() == 3);
  REQUIRE(arr7.size() == 3);
  REQUIRE(arr15.size() == 3);
  REQUIRE(arr3.capacity() == 3);
  REQUIRE(arr7.capacity() == 7);
  REQUIRE(arr15.capacity() == 15);

  arr3.push_back(1);
  arr7.push_back(1);
  arr15.push_back(1);
  REQUIRE(arr3.size() == 4);
  REQUIRE(arr7.size() == 4);
  REQUIRE(arr15.size() == 4);
  REQUIRE(arr3.capacity() == 4);
  REQUIRE(arr7.capacity() == 7);
  REQUIRE(arr15.capacity() == 15);

  arr3.push_back(1);
  arr7.push_back(1);
  arr15.push_back(1);
  REQUIRE(arr3.size() == 5);
  REQUIRE(arr7.size() == 5);
  REQUIRE(arr15.size() == 5);
  REQUIRE(arr3.capacity() == 8);
  REQUIRE(arr7.capacity() == 7);
  REQUIRE(arr15.capacity() == 15);

  arr3.push_back(1);
  arr7.push_back(1);
  arr15.push_back(1);
  REQUIRE(arr3.size() == 6);
  REQUIRE(arr7.size() == 6);
  REQUIRE(arr15.size() == 6);
  REQUIRE(arr3.capacity() == 8);
  REQUIRE(arr7.capacity() == 7);
  REQUIRE(arr15.capacity() == 15);

  arr3.push_back(1);
  arr7.push_back(1);
  arr15.push_back(1);
  REQUIRE(arr3.size() == 7);
  REQUIRE(arr7.size() == 7);
  REQUIRE(arr15.size() == 7);
  REQUIRE(arr3.capacity() == 8);
  REQUIRE(arr7.capacity() == 7);
  REQUIRE(arr15.capacity() == 15);

  arr3.push_back(1);
  arr7.push_back(1);
  arr15.push_back(1);
  REQUIRE(arr3.size() == 8);
  REQUIRE(arr7.size() == 8);
  REQUIRE(arr15.size() == 8);
  REQUIRE(arr3.capacity() == 8);
  REQUIRE(arr7.capacity() == 8);
  REQUIRE(arr15.capacity() == 15);

  arr3.push_back(1);
  arr7.push_back(1);
  arr15.push_back(1);
  REQUIRE(arr3.size() == 9);
  REQUIRE(arr7.size() == 9);
  REQUIRE(arr15.size() == 9);
  REQUIRE(arr3.capacity() == 16);
  REQUIRE(arr7.capacity() == 16);
  REQUIRE(arr15.capacity() == 15);

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
