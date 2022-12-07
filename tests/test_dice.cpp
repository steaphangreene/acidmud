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

#include <map>

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

TEST_CASE("SR Dice Rolls", "[Dice]") {
  int count = 0;
  std::map<int, int> freq;
  while (count < 2160000) {
    auto val = dSR();
    REQUIRE(val > 0);
    ++freq[val];
    ++count;
  }
  REQUIRE(freq[1] > 355000); // Theoretically can fail, but not realistically.
  REQUIRE(freq[1] < 365000); // Theoretically can fail, but not realistically.
  REQUIRE(freq[2] > 355000); // Theoretically can fail, but not realistically.
  REQUIRE(freq[2] < 365000); // Theoretically can fail, but not realistically.
  REQUIRE(freq[3] > 355000); // Theoretically can fail, but not realistically.
  REQUIRE(freq[3] < 365000); // Theoretically can fail, but not realistically.
  REQUIRE(freq[4] > 355000); // Theoretically can fail, but not realistically.
  REQUIRE(freq[4] < 365000); // Theoretically can fail, but not realistically.
  REQUIRE(freq[5] > 355000); // Theoretically can fail, but not realistically.
  REQUIRE(freq[5] < 365000); // Theoretically can fail, but not realistically.
  REQUIRE(freq[6] == 0); // Should be impossible.
  REQUIRE(freq[7] > 55000); // Theoretically can fail, but not realistically.
  REQUIRE(freq[7] < 65000); // Theoretically can fail, but not realistically.
  REQUIRE(freq[8] > 55000); // Theoretically can fail, but not realistically.
  REQUIRE(freq[8] < 65000); // Theoretically can fail, but not realistically.
  REQUIRE(freq[9] > 55000); // Theoretically can fail, but not realistically.
  REQUIRE(freq[9] < 65000); // Theoretically can fail, but not realistically.
  REQUIRE(freq[10] > 55000); // Theoretically can fail, but not realistically.
  REQUIRE(freq[10] < 65000); // Theoretically can fail, but not realistically.
  REQUIRE(freq[11] > 55000); // Theoretically can fail, but not realistically.
  REQUIRE(freq[11] < 65000); // Theoretically can fail, but not realistically.
  REQUIRE(freq[12] == 0); // Should be impossible.
  REQUIRE(freq[13] > 5000); // Theoretically can fail, but not realistically.
  REQUIRE(freq[13] < 15000); // Theoretically can fail, but not realistically.
  REQUIRE(freq[14] > 5000); // Theoretically can fail, but not realistically.
  REQUIRE(freq[14] < 15000); // Theoretically can fail, but not realistically.
  REQUIRE(freq[15] > 5000); // Theoretically can fail, but not realistically.
  REQUIRE(freq[15] < 15000); // Theoretically can fail, but not realistically.
  REQUIRE(freq[16] > 5000); // Theoretically can fail, but not realistically.
  REQUIRE(freq[16] < 15000); // Theoretically can fail, but not realistically.
  REQUIRE(freq[17] > 5000); // Theoretically can fail, but not realistically.
  REQUIRE(freq[17] < 15000); // Theoretically can fail, but not realistically.
  REQUIRE(freq[18] == 0); // Should be impossible.
  REQUIRE(freq[19] > 0); // Theoretically can fail, but not realistically.
  REQUIRE(freq[19] < 5000); // Theoretically can fail, but not realistically.
  REQUIRE(freq[20] > 0); // Theoretically can fail, but not realistically.
  REQUIRE(freq[20] < 5000); // Theoretically can fail, but not realistically.
  REQUIRE(freq[21] > 0); // Theoretically can fail, but not realistically.
  REQUIRE(freq[21] < 5000); // Theoretically can fail, but not realistically.
  REQUIRE(freq[22] > 0); // Theoretically can fail, but not realistically.
  REQUIRE(freq[22] < 5000); // Theoretically can fail, but not realistically.
  REQUIRE(freq[23] > 0); // Theoretically can fail, but not realistically.
  REQUIRE(freq[23] < 5000); // Theoretically can fail, but not realistically.
  REQUIRE(freq[24] == 0); // Should be impossible.
  REQUIRE(freq[25] > 0); // Theoretically can fail, but not realistically.
  REQUIRE(freq[25] < 1000); // Theoretically can fail, but not realistically.
  REQUIRE(freq[26] > 0); // Theoretically can fail, but not realistically.
  REQUIRE(freq[26] < 1000); // Theoretically can fail, but not realistically.
  REQUIRE(freq[27] > 0); // Theoretically can fail, but not realistically.
  REQUIRE(freq[27] < 1000); // Theoretically can fail, but not realistically.
  REQUIRE(freq[28] > 0); // Theoretically can fail, but not realistically.
  REQUIRE(freq[28] < 1000); // Theoretically can fail, but not realistically.
  REQUIRE(freq[29] > 0); // Theoretically can fail, but not realistically.
  REQUIRE(freq[29] < 1000); // Theoretically can fail, but not realistically.
  REQUIRE(freq[30] == 0); // Should be impossible.
  REQUIRE(freq[31] > 0); // Theoretically can fail, but not realistically.
  REQUIRE(freq[31] < 200); // Theoretically can fail, but not realistically.
  REQUIRE(freq[32] > 0); // Theoretically can fail, but not realistically.
  REQUIRE(freq[32] < 200); // Theoretically can fail, but not realistically.
  REQUIRE(freq[33] > 0); // Theoretically can fail, but not realistically.
  REQUIRE(freq[33] < 200); // Theoretically can fail, but not realistically.
  REQUIRE(freq[34] > 0); // Theoretically can fail, but not realistically.
  REQUIRE(freq[34] < 200); // Theoretically can fail, but not realistically.
  REQUIRE(freq[35] > 0); // Theoretically can fail, but not realistically.
  REQUIRE(freq[35] < 200); // Theoretically can fail, but not realistically.
  REQUIRE(freq[36] == 0); // Should be impossible.
  REQUIRE(freq.rbegin()->first > 36); // Theoretically can fail, but not realistically.
  REQUIRE(freq[37] < 100); // Theoretically can fail, but not realistically.
  REQUIRE(freq[38] < 100); // Theoretically can fail, but not realistically.
  REQUIRE(freq[39] < 100); // Theoretically can fail, but not realistically.
  REQUIRE(freq[40] < 100); // Theoretically can fail, but not realistically.
  REQUIRE(freq[41] < 100); // Theoretically can fail, but not realistically.
  REQUIRE(freq[42] == 0); // Should be impossible.
  REQUIRE(freq[43] < 20); // Theoretically can fail, but not realistically.
  REQUIRE(freq[44] < 20); // Theoretically can fail, but not realistically.
  REQUIRE(freq[45] < 20); // Theoretically can fail, but not realistically.
  REQUIRE(freq[46] < 20); // Theoretically can fail, but not realistically.
  REQUIRE(freq[47] < 20); // Theoretically can fail, but not realistically.
  REQUIRE(freq[48] == 0); // Should be impossible.
}
