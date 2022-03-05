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

class ArmorType {
 public:
  ArmorType(
      const std::u8string& nm,
      const std::u8string& ds,
      const std::u8string& lds,
      int b,
      int bm,
      int i,
      int im,
      int t,
      int tm,
      int p,
      int pm,
      int w,
      int vol,
      int val,
      act_t l1 = act_t::NONE,
      act_t l2 = act_t::NONE,
      act_t l3 = act_t::NONE,
      act_t l4 = act_t::NONE,
      act_t l5 = act_t::NONE,
      act_t l6 = act_t::NONE);
  std::u8string name, desc, long_desc;
  std::vector<act_t> loc;
  int bulk, bulkm; // Body-Type (adds mass, reduces shock)
  int impact, impactm; // Cushion (absorbs shock, softens impact)
  int thread, threadm; // Tensile (won't rip/cut, prevents slashing)
  int planar, planarm; // Plate (won't bend, spreads impact)
  int weight, volume, value;
};

class WeaponType {
 public:
  WeaponType(
      const std::u8string& nm,
      const std::u8string& ds,
      const std::u8string& lds,
      const std::u8string& t,
      int,
      int,
      int,
      int,
      int,
      int w,
      int vol,
      int val);
  std::u8string name, desc, long_desc;
  int type, reach;
  int force, forcem;
  int sev, sevm;
  int weight, volume, value;
};

class ItemType {
 public:
  ItemType(
      const std::u8string& nm,
      const std::u8string& ds,
      const std::u8string& lds,
      const std::vector<skill_pair>& sk,
      int w,
      int vol,
      int val);
  std::u8string name, desc, long_desc;
  std::vector<skill_pair> skills;
  int weight, volume, value;
};

struct NPCAttrs {
  int v[6];
};

class NPCType {
 public:
  NPCType(
      const std::u8string& nm,
      const std::u8string& ds,
      const std::u8string& lds,
      const std::u8string& gens,
      NPCAttrs min,
      NPCAttrs max,
      int gmin = 0,
      int gmax = 0);
  void Skill(uint32_t, int, int);
  void Skill(uint32_t, int);
  void Arm(WeaponType*);
  void Armor(ArmorType*);
  void Carry(ItemType*);
  void SetName(const std::u8string&);

  std::u8string name, desc, long_desc, genders;
  NPCAttrs mins, maxes;
  int min_gold, max_gold;
  std::map<uint32_t, std::pair<int, int>> skills;
  WeaponType* armed;
  std::vector<ArmorType*> armor;
  std::vector<ItemType*> items;
};

std::u8string gender_proc(const std::u8string& in, char8_t gender);
