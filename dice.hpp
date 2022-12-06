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

  static void Sample(const auto& deck, auto target, int count = 1) {
    std::ranges::sample(deck, target, count, generator_);
  }

  static bool Percent(int chance) { // change% chance of true, else false
    auto roll = std::uniform_int_distribution<int>(0, 99)(generator_);
    return (roll < chance);
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
  static std::random_device device_;
  static std::mt19937 generator_;
};

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

#endif // DICE_HPP
