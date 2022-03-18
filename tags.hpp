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

#ifndef TAGS_HPP
#define TAGS_HPP

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

struct ArmorAttrs {
  int bulk; // Body-Tag (adds mass, reduces shock)
  int impact; // Cushion (absorbs shock, softens impact)
  int thread; // Tensile (won't rip/cut, prevents slashing)
  int planar; // Plate (won't bend, spreads impact)
};

struct WeaponAttrs {
  int reach;
  int force;
  int severity;
};

struct NPCAttrs {
  int v[6];
};

class ObjectTag {
 public:
  ObjectTag(const std::u8string_view& tagdef);
  ObjectTag(std::u8string_view& tagdef);
  bool LoadFrom(std::u8string_view& tagdef);
  void operator+=(const ObjectTag&);

  void FinalizeWeaponTags(const std::map<std::u8string, ObjectTag>&);
  void FinalizeArmorTags(const std::map<std::u8string, ObjectTag>&);
  void FinalizeItemTags(const std::map<std::u8string, ObjectTag>&);
  void Finalize();

  std::u8string short_desc_, desc_, long_desc_;
  std::vector<skill_pair> props_;
  std::vector<std::vector<act_t>> loc_;
  int wtype_ = 0;
  ItemAttrs omin_ = {0, 0, 0, 0};
  ItemAttrs omax_ = {0, 0, 0, 0};
  ArmorAttrs amin_ = {0, 0, 0, 0};
  ArmorAttrs amax_ = {0, 0, 0, 0};
  WeaponAttrs wmin_ = {0, 0, 0};
  WeaponAttrs wmax_ = {0, 0, 0};
  std::vector<std::u8string> otags_;
  std::vector<std::u8string> ntags_;

  std::vector<gender_t> genders_;
  NPCAttrs min_ = {0, 0, 0, 0, 0, 0};
  NPCAttrs max_ = {0, 0, 0, 0, 0, 0};
  int min_gold_ = 0;
  int max_gold_ = 0;
  std::vector<std::u8string> wtags_;
  std::vector<std::u8string> atags_;
  std::vector<std::u8string> itags_;
  std::vector<ObjectTag> weapons_;
  std::vector<ObjectTag> armor_;
  std::vector<ObjectTag> items_;
};

#endif // TAGS_HPP
