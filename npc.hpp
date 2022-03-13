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
#include <string>
#include <vector>

#include "object.hpp"

struct ItemAttrs {
  int weight;
  int size;
  int volume;
  int value;
};

class ItemType {
 public:
  ItemType(
      const std::u8string& nm,
      const std::u8string& ds,
      const std::u8string& lds,
      const std::vector<skill_pair>& sk,
      ItemAttrs min,
      ItemAttrs max);
  std::u8string name_, desc_, long_desc_;
  std::vector<skill_pair> skills_;
  ItemAttrs min_;
  ItemAttrs max_;
};

struct ArmorAttrs {
  int bulk; // Body-Type (adds mass, reduces shock)
  int impact; // Cushion (absorbs shock, softens impact)
  int thread; // Tensile (won't rip/cut, prevents slashing)
  int planar; // Plate (won't bend, spreads impact)
};

class ArmorType {
 public:
  ArmorType(
      const std::u8string& nm,
      const std::u8string& ds,
      const std::u8string& lds,
      ItemAttrs min,
      ItemAttrs max,
      ArmorAttrs amin,
      ArmorAttrs amax,
      act_t l1 = act_t::NONE,
      act_t l2 = act_t::NONE,
      act_t l3 = act_t::NONE,
      act_t l4 = act_t::NONE,
      act_t l5 = act_t::NONE,
      act_t l6 = act_t::NONE);
  std::u8string name_, desc_, long_desc_;
  std::vector<act_t> loc_;
  ItemAttrs min_;
  ItemAttrs max_;
  ArmorAttrs amin_;
  ArmorAttrs amax_;
};

struct WeaponAttrs {
  int reach;
  int force;
  int severity;
};

class WeaponType {
 public:
  WeaponType(
      const std::u8string& nm,
      const std::u8string& ds,
      const std::u8string& lds,
      const std::u8string& t,
      ItemAttrs min,
      ItemAttrs max,
      WeaponAttrs wmin,
      WeaponAttrs wmax);
  std::u8string name_, desc_, long_desc_;
  int wtype_;
  WeaponAttrs wmin_;
  WeaponAttrs wmax_;
  ItemAttrs min_;
  ItemAttrs max_;
};

struct NPCAttrs {
  int v[6];
};

class NPCType {
 public:
  NPCType(
      const std::u8string& sds,
      const std::u8string& ds,
      const std::u8string& lds,
      const std::vector<gender_t>& gens,
      NPCAttrs min,
      NPCAttrs max,
      int gmin = 0,
      int gmax = 0);
  void Skill(uint32_t, int, int);
  void Skill(uint32_t, int);
  void Arm(const WeaponType&);
  void Armor(const ArmorType&);
  void Carry(const ItemType&);
  void SetShortDesc(const std::u8string_view&);
  void operator+=(const NPCType&);

  std::u8string short_desc_, desc_, long_desc_;
  std::vector<gender_t> genders_;
  NPCAttrs min_, max_;
  int min_gold_, max_gold_;
  std::map<uint32_t, std::pair<int, int>> skills_;
  std::vector<WeaponType> weapons_;
  std::vector<ArmorType> armor_;
  std::vector<ItemType> items_;
};
