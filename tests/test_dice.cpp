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

#include <catch2/catch.hpp>

#include "../dice.hpp"

TEST_CASE("Basic Dice Function Sanity Tests", "[Dice]") {
  int freq[8] = {0};
  REQUIRE(freq[0] == 0); // Something Horrible Has Happened
  REQUIRE(freq[1] == 0); // Something Horrible Has Happened
  REQUIRE(freq[6] == 0); // Something Horrible Has Happened
  REQUIRE(freq[7] == 0); // Something Horrible Has Happened

  int count = 0;
  unsigned int seen = 0;
  while (seen != 0x7E && count < 1000000000) {
    int val = d6();
    seen |= (1 << val);
    REQUIRE(val <= 6);
    REQUIRE(val >= 1);
    ++freq[val];
    ++count;
  }

  REQUIRE(freq[0] == 0); // Something Horrible Has Happened
  REQUIRE(freq[7] == 0); // Something Horrible Has Happened

  REQUIRE(freq[1] > 0); // Theoretically can fail, but not realistically.
  REQUIRE(freq[2] > 0); // Theoretically can fail, but not realistically.
  REQUIRE(freq[3] > 0); // Theoretically can fail, but not realistically.
  REQUIRE(freq[4] > 0); // Theoretically can fail, but not realistically.
  REQUIRE(freq[5] > 0); // Theoretically can fail, but not realistically.
  REQUIRE(freq[6] > 0); // Theoretically can fail, but not realistically.
}

TEST_CASE("Dice::Die Function Sanity Tests", "[Dice]") {
  int freq[8] = {0};
  REQUIRE(freq[0] == 0); // Something Horrible Has Happened
  REQUIRE(freq[1] == 0); // Something Horrible Has Happened
  REQUIRE(freq[6] == 0); // Something Horrible Has Happened
  REQUIRE(freq[7] == 0); // Something Horrible Has Happened

  int count = 0;
  unsigned int seen = 0;
  auto my_d6 = Dice::GetDie(1, 6);
  while (seen != 0x7E && count < 1000000000) {
    int val = my_d6();
    seen |= (1 << val);
    REQUIRE(val <= 6);
    REQUIRE(val >= 1);
    ++freq[val];
    ++count;
  }

  REQUIRE(freq[0] == 0); // Something Horrible Has Happened
  REQUIRE(freq[7] == 0); // Something Horrible Has Happened

  REQUIRE(freq[1] > 0); // Theoretically can fail, but not realistically.
  REQUIRE(freq[2] > 0); // Theoretically can fail, but not realistically.
  REQUIRE(freq[3] > 0); // Theoretically can fail, but not realistically.
  REQUIRE(freq[4] > 0); // Theoretically can fail, but not realistically.
  REQUIRE(freq[5] > 0); // Theoretically can fail, but not realistically.
  REQUIRE(freq[6] > 0); // Theoretically can fail, but not realistically.
}

TEST_CASE("Dice::Shuffle() Sanity Tests", "[Dice]") {
  int data[6] = {1, 2, 3, 4, 5, 6};
  REQUIRE(std::ranges::is_sorted(data)); // Something Horrible Has Happened

  int count = 0;
  while (std::ranges::is_sorted(data) && count < 1000) {
    Dice::Shuffle(data);
    ++count;
  }
  REQUIRE(!std::ranges::is_sorted(data)); // Theoretically can fail, but not realistically.

  int freq[8] = {0};
  REQUIRE(freq[0] == 0); // Something Horrible Has Happened
  REQUIRE(freq[1] == 0); // Something Horrible Has Happened
  REQUIRE(freq[2] == 0); // Something Horrible Has Happened
  REQUIRE(freq[3] == 0); // Something Horrible Has Happened
  REQUIRE(freq[4] == 0); // Something Horrible Has Happened
  REQUIRE(freq[5] == 0); // Something Horrible Has Happened
  REQUIRE(freq[6] == 0); // Something Horrible Has Happened
  REQUIRE(freq[7] == 0); // Something Horrible Has Happened

  for (auto val : data) {
    REQUIRE(val <= 6);
    REQUIRE(val >= 1);
    ++freq[val];
  }

  REQUIRE(freq[0] == 0); // Something Horrible Has Happened
  REQUIRE(freq[1] == 1);
  REQUIRE(freq[2] == 1);
  REQUIRE(freq[3] == 1);
  REQUIRE(freq[4] == 1);
  REQUIRE(freq[5] == 1);
  REQUIRE(freq[6] == 1);
  REQUIRE(freq[7] == 0); // Something Horrible Has Happened
}

TEST_CASE("Dice::Sample() [C Array] Sanity Tests", "[Dice]") {
  int data[6] = {1, 2, 3, 4, 5, 6};
  REQUIRE(std::ranges::is_sorted(data)); // Something Horrible Has Happened

  int count = 0;
  unsigned int seen = 0;
  while (seen != 0x7E && count < 1000000) {
    auto val = Dice::Sample(data);
    seen |= (1 << val);
    REQUIRE(val <= 6);
    REQUIRE(val >= 1);
    ++count;
  }
  REQUIRE(seen == 0x7E);
}

TEST_CASE("Dice::Sample() [std::array] Sanity Tests", "[Dice]") {
  std::array<int, 6> data = {1, 2, 3, 4, 5, 6};
  REQUIRE(std::ranges::is_sorted(data)); // Something Horrible Has Happened

  int count = 0;
  unsigned int seen = 0;
  while (seen != 0x7E && count < 1000000) {
    auto val = Dice::Sample(data);
    seen |= (1 << val);
    REQUIRE(val <= 6);
    REQUIRE(val >= 1);
    ++count;
  }
  REQUIRE(seen == 0x7E);
}

TEST_CASE("Dice::Sample() [multi] Sanity Tests", "[Dice]") {
  int data[6] = {1, 2, 3, 4, 5, 6};
  REQUIRE(std::ranges::is_sorted(data)); // Something Horrible Has Happened

  int count = 0;
  unsigned int seen = 0;
  while (seen != 0x7E && count < 1000) {
    std::vector<int> vals;
    vals.reserve(1000);
    Dice::Sample(data, std::back_inserter(vals), 1000);
    for (auto val : vals) {
      seen |= (1 << val);
      REQUIRE(val <= 6);
      REQUIRE(val >= 1);
    }
    ++count;
  }
  REQUIRE(seen == 0x7E);
}
