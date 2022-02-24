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

#include <unistd.h>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "color.hpp"
#include "commands.hpp"
#include "mind.hpp"
#include "mob.hpp"
#include "object.hpp"
#include "utils.hpp"

static std::map<act_t, std::string> wear_attribs;
static void init_wear_attribs() {
  wear_attribs[act_t::WEAR_BACK] = "Wearable on Back";
  wear_attribs[act_t::WEAR_CHEST] = "Wearable on Chest";
  wear_attribs[act_t::WEAR_HEAD] = "Wearable on Head";
  wear_attribs[act_t::WEAR_NECK] = "Wearable on Neck";
  wear_attribs[act_t::WEAR_COLLAR] = "Wearable on Collar";
  wear_attribs[act_t::WEAR_WAIST] = "Wearable on Waist";
  wear_attribs[act_t::WEAR_SHIELD] = "Wearable on Shield";
  wear_attribs[act_t::WEAR_LARM] = "Wearable on Left Arm";
  wear_attribs[act_t::WEAR_RARM] = "Wearable on Right Arm";
  wear_attribs[act_t::WEAR_LFINGER] = "Wearable on Left Finger";
  wear_attribs[act_t::WEAR_RFINGER] = "Wearable on Right Finger";
  wear_attribs[act_t::WEAR_LFOOT] = "Wearable on Left Foot";
  wear_attribs[act_t::WEAR_RFOOT] = "Wearable on Right Foot";
  wear_attribs[act_t::WEAR_LHAND] = "Wearable on Left Hand";
  wear_attribs[act_t::WEAR_RHAND] = "Wearable on Right Hand";
  wear_attribs[act_t::WEAR_LLEG] = "Wearable on Left Leg";
  wear_attribs[act_t::WEAR_RLEG] = "Wearable on Right Leg";
  wear_attribs[act_t::WEAR_LWRIST] = "Wearable on Left Wrist";
  wear_attribs[act_t::WEAR_RWRIST] = "Wearable on Right Wrist";
  wear_attribs[act_t::WEAR_LSHOULDER] = "Wearable on Left Shoulder";
  wear_attribs[act_t::WEAR_RSHOULDER] = "Wearable on Right Shoulder";
  wear_attribs[act_t::WEAR_LHIP] = "Wearable on Left Hip";
  wear_attribs[act_t::WEAR_RHIP] = "Wearable on Right Hip";
  wear_attribs[act_t::WEAR_FACE] = "Wearable on Face";
}

static Mind* mob_mind = nullptr;
Mind* get_mob_mind() {
  if (!mob_mind) {
    mob_mind = new Mind();
    mob_mind->SetMob();
  }
  return mob_mind;
}

static Object* gold = nullptr;
static void init_gold() {
  gold = new Object();
  gold->SetShortDesc("a gold piece");
  gold->SetDesc("A standard one-ounce gold piece.");
  gold->SetWeight(454 / 16);
  gold->SetVolume(0);
  gold->SetValue(1);
  gold->SetSize(0);
  gold->SetPos(pos_t::LIE);
}

static void give_gold(Object* mob, int qty) {
  Object* bag = new Object;

  bag->SetParent(mob);
  bag->SetShortDesc("a small purse");
  bag->SetDesc("A small, durable, practical moneypurse.");

  bag->SetSkill(crc32c("Wearable on Left Hip"), 1);
  bag->SetSkill(crc32c("Wearable on Right Hip"), 2);
  bag->SetSkill(crc32c("Container"), 5 * 454);
  bag->SetSkill(crc32c("Capacity"), 5);
  bag->SetSkill(crc32c("Closeable"), 1);

  bag->SetWeight(1 * 454);
  bag->SetSize(2);
  bag->SetVolume(1);
  bag->SetValue(100);

  bag->SetPos(pos_t::LIE);
  mob->AddAct(act_t::WEAR_LHIP, bag);

  if (!gold)
    init_gold();
  Object* g = new Object(*gold);
  g->SetParent(bag);
  g->SetSkill(crc32c("Quantity"), qty);
}

MOBType::MOBType(
    const std::string& nm,
    const std::string& ds,
    const std::string& lds,
    const std::string& gens,
    MOBAttrs min,
    MOBAttrs max,
    int gmin,
    int gmax) {
  name = nm;
  desc = ds;
  long_desc = lds;
  genders = gens;
  mins = min;
  maxes = max;
  min_gold = gmin;
  max_gold = gmax;

  armed = nullptr;
}

void MOBType::Skill(uint32_t stok, int percent, int mod) {
  skills[stok] = std::make_pair(percent, mod);
}

void MOBType::Skill(uint32_t stok, int val) {
  skills[stok] = std::make_pair(-1, val);
}

void MOBType::Arm(WeaponType* weap) {
  armed = weap;
}

void MOBType::Armor(ArmorType* arm) {
  armor.push_back(arm);
}

void MOBType::Carry(ItemType* item) {
  items.push_back(item);
}

void MOBType::SetName(const std::string& nm) {
  name = nm;
}

void Object::AddMOB(std::mt19937& gen, const MOBType* type) {
  Object* mob = new Object(this);

  mob->Attach(get_mob_mind());
  mob->Activate();
  mob->SetPos(pos_t::STAND);
  for (int a : {0, 1, 2, 3, 4, 5}) {
    mob->SetAttribute(
        a, std::uniform_int_distribution<int>(type->mins.v[a], type->maxes.v[a])(gen));
  }

  for (auto sk : type->skills) {
    if (sk.second.first < 0) {
      mob->SetSkill(sk.first, sk.second.second);
    } else {
      mob->SetSkill(
          sk.first,
          mob->NormAttribute(get_linked(sk.first)) * sk.second.first / 100 -
              rand() % sk.second.second);
    }
    // fprintf(stderr, "DBG: %d %d %d\n", get_linked(sk.first),
    // sk.second.first, sk.second.second);
  }

  mob->SetShortDesc(type->name.c_str());
  mob->SetDesc(type->desc.c_str());
  mob->SetLongDesc(type->long_desc.c_str());
  if (type->genders.length() > 0) {
    mob->SetGender(type->genders[rand() % type->genders.length()]);
  }
  mob->SetDesc(gender_proc(type->desc, mob->Gender()));
  mob->SetLongDesc(gender_proc(type->long_desc, mob->Gender()));

  if (type->min_gold > 0 || type->max_gold > 0) {
    std::uniform_int_distribution<int> gnum(type->min_gold, type->max_gold);
    give_gold(mob, gnum(gen));
  }

  if (type->armed) {
    Object* obj = new Object(mob);
    obj->SetSkill(crc32c("WeaponType"), type->armed->type);
    obj->SetSkill(crc32c("WeaponReach"), type->armed->reach);
    obj->SetSkill(crc32c("WeaponForce"), type->armed->force + rand() % type->armed->forcem);
    obj->SetSkill(crc32c("WeaponSeverity"), type->armed->sev + rand() % type->armed->sevm);
    obj->SetShortDesc(type->armed->name.c_str());
    obj->SetDesc(type->armed->desc.c_str());
    obj->SetLongDesc(type->armed->long_desc.c_str());
    obj->SetWeight(type->armed->weight);
    obj->SetVolume(type->armed->volume);
    obj->SetValue(type->armed->value);
    obj->SetPos(pos_t::LIE);
    mob->AddAct(act_t::WIELD, obj);
    if (two_handed(type->armed->type)) {
      mob->AddAct(act_t::HOLD, obj);
    }
  }

  for (auto ar : type->armor) {
    if (wear_attribs.size() <= 0) {
      init_wear_attribs();
    }
    Object* obj = new Object(mob);
    obj->SetAttribute(0, ar->bulk + rand() % ar->bulkm);
    obj->SetSkill(crc32c("ArmorB"), ar->bulk + rand() % ar->bulkm);
    obj->SetSkill(crc32c("ArmorI"), ar->impact + rand() % ar->impactm);
    obj->SetSkill(crc32c("ArmorT"), ar->thread + rand() % ar->threadm);
    obj->SetSkill(crc32c("ArmorP"), ar->planar + rand() % ar->planarm);
    obj->SetShortDesc(ar->name.c_str());
    obj->SetDesc(ar->desc.c_str());
    obj->SetLongDesc(ar->long_desc.c_str());
    obj->SetWeight(ar->weight);
    obj->SetVolume(ar->volume);
    obj->SetValue(ar->value);
    obj->SetPos(pos_t::LIE);

    for (auto loc : ar->loc) {
      obj->SetSkill(wear_attribs[loc], 1);
      mob->AddAct(loc, obj);
    }
  }

  if (type->items.size() > 0) {
    Object* sack = new Object(mob);
    sack->SetShortDesc("a small sack");
    sack->SetDesc("A small, durable, practical belt sack.");

    sack->SetSkill(crc32c("Wearable on Left Hip"), 1);
    sack->SetSkill(crc32c("Wearable on Right Hip"), 2);
    sack->SetSkill(crc32c("Container"), 5 * 454);
    sack->SetSkill(crc32c("Capacity"), 5);
    sack->SetSkill(crc32c("Closeable"), 1);

    sack->SetWeight(1 * 454);
    sack->SetSize(2);
    sack->SetVolume(1);
    sack->SetValue(100);

    sack->SetPos(pos_t::LIE);
    mob->AddAct(act_t::WEAR_RHIP, sack);

    for (auto it : type->items) {
      Object* obj = new Object(sack);
      for (auto sk : it->skills) {
        obj->SetSkill(sk.first, sk.second);
      }
      obj->SetShortDesc(it->name.c_str());
      obj->SetDesc(it->desc.c_str());
      obj->SetLongDesc(it->long_desc.c_str());
      obj->SetWeight(it->weight);
      obj->SetVolume(it->volume);
      obj->SetValue(it->value);
      obj->SetPos(pos_t::LIE);
    }
  }
}

WeaponType::WeaponType(
    const std::string& nm,
    const std::string& ds,
    const std::string& lds,
    const std::string& t,
    int r,
    int f,
    int fm,
    int s,
    int sm,
    int w,
    int vol,
    int val) {
  name = nm;
  desc = ds;
  long_desc = lds;
  type = get_weapon_type(t);
  reach = r;
  force = f;
  forcem = std::max(fm, 1);
  sev = s;
  sevm = std::max(sm, 1);
  weight = w;
  volume = vol;
  value = val;
}

ArmorType::ArmorType(
    const std::string& nm,
    const std::string& ds,
    const std::string& lds,
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
    act_t l1,
    act_t l2,
    act_t l3,
    act_t l4,
    act_t l5,
    act_t l6) {
  name = nm;
  desc = ds;
  long_desc = lds;

  bulk = b;
  bulkm = std::max(bm, 1);
  impact = i;
  impactm = std::max(im, 1);
  thread = t;
  threadm = std::max(tm, 1);
  planar = p;
  planarm = std::max(pm, 1);

  if (l1 != act_t::NONE)
    loc.push_back(l1);
  if (l2 != act_t::NONE)
    loc.push_back(l2);
  if (l3 != act_t::NONE)
    loc.push_back(l3);
  if (l4 != act_t::NONE)
    loc.push_back(l4);
  if (l5 != act_t::NONE)
    loc.push_back(l5);
  if (l6 != act_t::NONE)
    loc.push_back(l6);

  weight = w;
  volume = vol;
  value = val;
}

ItemType::ItemType(
    const std::string& nm,
    const std::string& ds,
    const std::string& lds,
    const std::vector<skill_pair>& sk,
    int w,
    int vol,
    int val) {
  name = nm;
  desc = ds;
  long_desc = lds;
  skills = sk;
  weight = w;
  volume = vol;
  value = val;
}

static const std::string gen_replace[][4] = {
    {"{He}", "She", "He", "It"},
    {"{Him}", "Her", "Him", "It"},
    {"{His}", "Hers", "His", "Its"},
    {"{he}", "she", "he", "it"},
    {"{him}", "her", "him", "it"},
    {"{his}", "hers", "his", "its"},
    {"", "", "", ""}};

std::string gender_proc(const std::string& in, char gender) {
  std::string ret = in;
  int ctr = 0, gen = 3;

  if (gender == 'F')
    gen = 1;
  if (gender == 'M')
    gen = 2;

  for (ctr = 0; !gen_replace[ctr][0].empty(); ++ctr) {
    int where = ret.find(gen_replace[ctr][0]);
    while (where >= 0 && where <= int(ret.length())) {
      fprintf(stderr, "Yep: %d %s (%s)\n", where, ret.c_str(), gen_replace[ctr][0].c_str());
      ret = ret.substr(0, where) + gen_replace[ctr][gen] +
          ret.substr(where + gen_replace[ctr][0].length());
      where = ret.find(gen_replace[ctr][0]);
    }
  }
  return ret;
}
