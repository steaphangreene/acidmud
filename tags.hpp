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

#include "object.hpp"

struct ItemAttrs {
  int weight;
  int size;
  int volume;
  int value;
};

class ItemTag {
 public:
  ItemTag(const std::u8string_view& tagdef);
  ItemTag(std::u8string_view& tagdef);
  bool LoadFrom(std::u8string_view& tagdef);
  void operator+=(const ItemTag&);
  std::u8string short_desc_, desc_, long_desc_;
  std::vector<skill_pair> props_;
  std::vector<std::vector<act_t>> loc_;
  ItemAttrs min_ = {0, 0, 0, 0};
  ItemAttrs max_ = {0, 0, 0, 0};
};

struct ArmorAttrs {
  int bulk; // Body-Tag (adds mass, reduces shock)
  int impact; // Cushion (absorbs shock, softens impact)
  int thread; // Tensile (won't rip/cut, prevents slashing)
  int planar; // Plate (won't bend, spreads impact)
};

class ArmorTag {
 public:
  void operator+=(const ArmorTag&);
  ArmorTag(const std::u8string_view& tagdef);
  ArmorTag(std::u8string_view& tagdef);
  bool LoadFrom(std::u8string_view& tagdef);
  std::u8string short_desc_, desc_, long_desc_;
  std::vector<skill_pair> props_;
  std::vector<std::vector<act_t>> loc_;
  ItemAttrs min_ = {0, 0, 0, 0};
  ItemAttrs max_ = {0, 0, 0, 0};
  ArmorAttrs amin_ = {0, 0, 0, 0};
  ArmorAttrs amax_ = {0, 0, 0, 0};
};

struct WeaponAttrs {
  int reach;
  int force;
  int severity;
};

class WeaponTag {
 public:
  void operator+=(const WeaponTag&);
  WeaponTag(const std::u8string_view& tagdef);
  WeaponTag(std::u8string_view& tagdef);
  bool LoadFrom(std::u8string_view& tagdef);
  std::u8string short_desc_, desc_, long_desc_;
  std::vector<skill_pair> props_;
  int wtype_ = 0;
  WeaponAttrs wmin_ = {0, 0, 0};
  WeaponAttrs wmax_ = {0, 0, 0};
  ItemAttrs min_ = {0, 0, 0, 0};
  ItemAttrs max_ = {0, 0, 0, 0};
};

struct NPCAttrs {
  int v[6];
};

class NPCTag {
 public:
  void operator+=(const NPCTag&);
  NPCTag(const std::u8string_view& tagdef);
  NPCTag(std::u8string_view& tagdef);
  bool LoadFrom(std::u8string_view& tagdef);
  void Prop(uint32_t, int, int);
  void Prop(uint32_t, int);
  void Arm(const WeaponTag&);
  void Armor(const ArmorTag&);
  void Carry(const ItemTag&);
  void SetShortDesc(const std::u8string_view&);
  void FinalizeWeaponTags(const std::map<std::u8string, WeaponTag>&);
  void FinalizeArmorTags(const std::map<std::u8string, ArmorTag>&);
  void FinalizeItemTags(const std::map<std::u8string, ItemTag>&);
  void Finalize();

  std::u8string short_desc_, desc_, long_desc_;
  std::vector<skill_pair> props_;
  std::vector<gender_t> genders_;
  NPCAttrs min_ = {0, 0, 0, 0, 0, 0};
  NPCAttrs max_ = {0, 0, 0, 0, 0, 0};
  int min_gold_ = 0;
  int max_gold_ = 0;
  std::vector<std::u8string> wtags_;
  std::vector<std::u8string> atags_;
  std::vector<std::u8string> itags_;
  std::vector<WeaponTag> weapons_;
  std::vector<ArmorTag> armor_;
  std::vector<ItemTag> items_;
};

#endif // TAGS_HPP
