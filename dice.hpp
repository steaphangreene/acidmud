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

#ifndef DICE_HPP
#define DICE_HPP

#include <algorithm>
#include <random>

class Dice {
 public:
  static void Shuffle(auto& deck) {
    std::ranges::shuffle(deck, generator_);
  }

  static auto Sample(const auto& deck) {
    std::remove_cvref_t<decltype(*(std::begin(deck)))> target;
    std::ranges::sample(deck, &target, 1, generator_);
    return target;
  }

  static void Sample(const auto& deck, auto target, int count = 1) {
    std::ranges::sample(deck, target, count, generator_);
  }

  static bool Odds(int num, int den) { // num in den chance of true, else false
    auto roll = std::uniform_int_distribution<int>(0, den - 1)(generator_);
    return (roll < num);
  }

  static bool Percent(int chance) { // change% chance of true, else false
    return Odds(chance, 100);
  }

  static bool Permill(int chance) { // change‰ chance of true, else false
    return Odds(chance, 1000);
  }

  static int Roll(int sides) { // 1-sides
    if (sides < 1) {
      return 1;
    }
    return std::uniform_int_distribution<int>(1, sides)(generator_);
  }

  static int Rand(int min, int max) { // Flat
    return std::uniform_int_distribution<int>(min, max)(generator_);
  }

  static int Rand3(int min, int max) { // 3dX-Style Bell-Curve-Ish
    auto rando = std::uniform_int_distribution<int>(min, max);
    return (rando(generator_) + rando(generator_) + rando(generator_) + 1) / 3;
  }

  static auto GetDie(int min, int max) {
    return Die(min, max);
  }

  class Die {
   public:
    Die(int min, int max) : distro_(min, max){};
    int operator()() {
      return distro_(Dice::generator_);
    }

   private:
    std::uniform_int_distribution<int> distro_;
  };

 private:
  static thread_local std::random_device device_;
  static thread_local std::mt19937 generator_;
};

inline int d2() { // 1-2
  return Dice::Roll(2);
}

inline int d3() { // 1-3
  return Dice::Roll(3);
}

inline int d4() { // 1-4
  return Dice::Roll(4);
}

inline int d6() { // 1-6
  return Dice::Roll(6);
}

inline int d8() { // 1-8
  return Dice::Roll(8);
}

inline int d10() { // 1-10
  return Dice::Roll(10);
}

inline int d20() { // 1-20
  return Dice::Roll(20);
}

inline int d100() { // 1-100
  return Dice::Roll(100);
}

// Constants used by dSR()
constexpr const int dSR_0d = 1;
constexpr const int dSR_1d = 6;
constexpr const int dSR_2d = 6 * 6;
constexpr const int dSR_3d = 6 * 6 * 6;
constexpr const int dSR_4d = 6 * 6 * 6 * 6;
constexpr const int dSR_5d = 6 * 6 * 6 * 6 * 6;
constexpr const int dSR_d1max = dSR_5d - dSR_4d;
constexpr const int dSR_d2max = dSR_5d - dSR_3d;
constexpr const int dSR_d3max = dSR_5d - dSR_2d;
constexpr const int dSR_d4max = dSR_5d - dSR_1d;
constexpr const int dSR_d5max = dSR_5d - dSR_0d;

inline int dSR5() { // 1-6+ (+4 Rerolls - up to 5 dice total)
  int val = Dice::Rand(0, dSR_d5max);
  if (val < dSR_d1max) { // Rolled: 1-5 on First Die: Result is 1-5
    return (val / dSR_4d) + 1;
  } else if (val < dSR_d2max) { // Rolled: 1-5 on Second Die: Result is 7-11
    return ((val - dSR_d1max) / dSR_3d) + 7;
  } else if (val < dSR_d3max) { // Rolled: 1-5 on Third Die: Result is 13-17
    return ((val - dSR_d2max) / dSR_2d) + 13;
  } else if (val < dSR_d4max) { // Rolled: 1-5 on Fourth Die: Result is 19-23
    return ((val - dSR_d3max) / dSR_1d) + 19;
  } else if (val < dSR_d5max) { // Rolled: 1-5 on Fifth Die: Result is 25-29
    return (val - dSR_d4max) + 25;
  } else { // Rolled: 6 on Fifth Die: Result is 30+
    return 30;
  }
}

inline int dSR() { // 1-6+ (Infinite Rerolls)
  int val = 0;
  int total = 0;
  do {
    val = dSR5();
    total += val;
  } while (val == 30);
  return total;
}

#endif // DICE_HPP
