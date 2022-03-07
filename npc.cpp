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
#include <string>

#include "cchar8.hpp"
#include "color.hpp"
#include "commands.hpp"
#include "log.hpp"
#include "mind.hpp"
#include "npc.hpp"
#include "object.hpp"
#include "properties.hpp"
#include "utils.hpp"

static std::map<act_t, std::u8string> wear_attribs;
static void init_wear_attribs() {
  wear_attribs[act_t::WEAR_BACK] = u8"Wearable on Back";
  wear_attribs[act_t::WEAR_CHEST] = u8"Wearable on Chest";
  wear_attribs[act_t::WEAR_HEAD] = u8"Wearable on Head";
  wear_attribs[act_t::WEAR_NECK] = u8"Wearable on Neck";
  wear_attribs[act_t::WEAR_COLLAR] = u8"Wearable on Collar";
  wear_attribs[act_t::WEAR_WAIST] = u8"Wearable on Waist";
  wear_attribs[act_t::WEAR_SHIELD] = u8"Wearable on Shield";
  wear_attribs[act_t::WEAR_LARM] = u8"Wearable on Left Arm";
  wear_attribs[act_t::WEAR_RARM] = u8"Wearable on Right Arm";
  wear_attribs[act_t::WEAR_LFINGER] = u8"Wearable on Left Finger";
  wear_attribs[act_t::WEAR_RFINGER] = u8"Wearable on Right Finger";
  wear_attribs[act_t::WEAR_LFOOT] = u8"Wearable on Left Foot";
  wear_attribs[act_t::WEAR_RFOOT] = u8"Wearable on Right Foot";
  wear_attribs[act_t::WEAR_LHAND] = u8"Wearable on Left Hand";
  wear_attribs[act_t::WEAR_RHAND] = u8"Wearable on Right Hand";
  wear_attribs[act_t::WEAR_LLEG] = u8"Wearable on Left Leg";
  wear_attribs[act_t::WEAR_RLEG] = u8"Wearable on Right Leg";
  wear_attribs[act_t::WEAR_LWRIST] = u8"Wearable on Left Wrist";
  wear_attribs[act_t::WEAR_RWRIST] = u8"Wearable on Right Wrist";
  wear_attribs[act_t::WEAR_LSHOULDER] = u8"Wearable on Left Shoulder";
  wear_attribs[act_t::WEAR_RSHOULDER] = u8"Wearable on Right Shoulder";
  wear_attribs[act_t::WEAR_LHIP] = u8"Wearable on Left Hip";
  wear_attribs[act_t::WEAR_RHIP] = u8"Wearable on Right Hip";
  wear_attribs[act_t::WEAR_FACE] = u8"Wearable on Face";
}

static Object* gold = nullptr;
static void init_gold() {
  gold = new Object();
  gold->SetShortDesc(u8"a gold piece");
  gold->SetDesc(u8"A standard one-ounce gold piece.");
  gold->SetWeight(454 / 16);
  gold->SetVolume(0);
  gold->SetValue(1);
  gold->SetSize(0);
  gold->SetPos(pos_t::LIE);
}

static void give_gold(Object* npc, int qty) {
  Object* bag = new Object(npc);

  bag->SetShortDesc(u8"a small purse");
  bag->SetDesc(u8"A small, durable, practical moneypurse.");

  bag->SetSkill(prhash(u8"Wearable on Left Hip"), 1);
  bag->SetSkill(prhash(u8"Wearable on Right Hip"), 2);
  bag->SetSkill(prhash(u8"Container"), 5 * 454);
  bag->SetSkill(prhash(u8"Capacity"), 5);
  bag->SetSkill(prhash(u8"Closeable"), 1);

  bag->SetWeight(1 * 454);
  bag->SetSize(2);
  bag->SetVolume(1);
  bag->SetValue(100);

  bag->SetPos(pos_t::LIE);
  npc->AddAct(act_t::WEAR_LHIP, bag);

  if (!gold)
    init_gold();
  Object* g = new Object(*gold);
  g->SetParent(bag);
  g->SetSkill(prhash(u8"Quantity"), qty);
}

static void add_pouch(Object* npc) {
  Object* bag = new Object(npc);

  bag->SetShortDesc(u8"a small pouch");
  bag->SetDesc(u8"A small, durable, practical beltpouch.");

  bag->SetSkill(prhash(u8"Wearable on Left Hip"), 1);
  bag->SetSkill(prhash(u8"Wearable on Right Hip"), 2);
  bag->SetSkill(prhash(u8"Container"), 5 * 454);
  bag->SetSkill(prhash(u8"Capacity"), 5);
  bag->SetSkill(prhash(u8"Closeable"), 1);

  bag->SetWeight(1 * 454);
  bag->SetSize(2);
  bag->SetVolume(1);
  bag->SetValue(100);

  bag->SetPos(pos_t::LIE);
  npc->AddAct(act_t::WEAR_RHIP, bag);
}

NPCType::NPCType(
    const std::u8string& nm,
    const std::u8string& ds,
    const std::u8string& lds,
    const std::u8string& gens,
    NPCAttrs min,
    NPCAttrs max,
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

void NPCType::Skill(uint32_t stok, int percent, int mod) {
  skills[stok] = std::make_pair(percent, mod);
}

void NPCType::Skill(uint32_t stok, int val) {
  skills[stok] = std::make_pair(-1, val);
}

void NPCType::Arm(WeaponType* weap) {
  armed = weap;
}

void NPCType::Armor(ArmorType* arm) {
  armor.push_back(arm);
}

void NPCType::Carry(ItemType* item) {
  items.push_back(item);
}

void NPCType::SetName(const std::u8string& nm) {
  name = nm;
}

void Object::AddNPC(std::mt19937& gen, const NPCType* type) {
  Object* npc = new Object(this);

  if (npc->World()) {
    auto obj_id = npc->World()->Skill(prhash(u8"Last Object ID")) + 1;
    npc->World()->SetSkill(prhash(u8"Last Object ID"), obj_id);
    npc->SetSkill(prhash(u8"Object ID"), obj_id);
  }

  npc->Attach(new Mind(mind_t::NPC));
  npc->Activate();
  npc->SetPos(pos_t::STAND);
  for (int a : {0, 1, 2, 3, 4, 5}) {
    npc->SetAttribute(
        a, std::uniform_int_distribution<int>(type->mins.v[a], type->maxes.v[a])(gen));
  }

  for (auto sk : type->skills) {
    if (sk.second.first < 0) {
      npc->SetSkill(sk.first, sk.second.second);
    } else {
      npc->SetSkill(
          sk.first,
          npc->NormAttribute(get_linked(sk.first)) * sk.second.first / 100 -
              rand() % sk.second.second);
    }
    // loge(u8"DBG: {} {} {}\n", get_linked(sk.first), sk.second.first, sk.second.second);
  }

  npc->SetShortDesc(type->name.c_str());
  npc->SetDesc(type->desc.c_str());
  npc->SetLongDesc(type->long_desc.c_str());
  if (type->genders.length() > 0) {
    npc->SetGender(type->genders[rand() % type->genders.length()]);
  }
  npc->SetDesc(gender_proc(type->desc, npc->Gender()));
  npc->SetLongDesc(gender_proc(type->long_desc, npc->Gender()));

  if (type->min_gold > 0 || type->max_gold > 0) {
    std::uniform_int_distribution<int> gnum(type->min_gold, type->max_gold);
    give_gold(npc, gnum(gen));
  }

  if (true) { // TODO: Figure out who should get these, and who should not
    add_pouch(npc);
  }

  if (type->armed) {
    Object* obj = new Object(npc);
    obj->SetSkill(prhash(u8"WeaponType"), type->armed->type);
    obj->SetSkill(prhash(u8"WeaponReach"), type->armed->reach);
    obj->SetSkill(prhash(u8"WeaponForce"), type->armed->force + rand() % type->armed->forcem);
    obj->SetSkill(prhash(u8"WeaponSeverity"), type->armed->sev + rand() % type->armed->sevm);
    obj->SetShortDesc(type->armed->name.c_str());
    obj->SetDesc(type->armed->desc.c_str());
    obj->SetLongDesc(type->armed->long_desc.c_str());
    obj->SetWeight(type->armed->weight);
    obj->SetVolume(type->armed->volume);
    obj->SetValue(type->armed->value);
    obj->SetPos(pos_t::LIE);
    npc->AddAct(act_t::WIELD, obj);
    if (two_handed(type->armed->type)) {
      npc->AddAct(act_t::HOLD, obj);
    }
  }

  for (auto ar : type->armor) {
    if (wear_attribs.size() <= 0) {
      init_wear_attribs();
    }
    Object* obj = new Object(npc);
    obj->SetAttribute(0, ar->bulk + rand() % ar->bulkm);
    obj->SetSkill(prhash(u8"ArmorB"), ar->bulk + rand() % ar->bulkm);
    obj->SetSkill(prhash(u8"ArmorI"), ar->impact + rand() % ar->impactm);
    obj->SetSkill(prhash(u8"ArmorT"), ar->thread + rand() % ar->threadm);
    obj->SetSkill(prhash(u8"ArmorP"), ar->planar + rand() % ar->planarm);
    obj->SetShortDesc(ar->name.c_str());
    obj->SetDesc(ar->desc.c_str());
    obj->SetLongDesc(ar->long_desc.c_str());
    obj->SetWeight(ar->weight);
    obj->SetVolume(ar->volume);
    obj->SetValue(ar->value);
    obj->SetPos(pos_t::LIE);

    for (auto loc : ar->loc) {
      obj->SetSkill(wear_attribs[loc], 1);
      npc->AddAct(loc, obj);
    }
  }

  if (type->items.size() > 0) {
    Object* sack = new Object(npc);
    sack->SetShortDesc(u8"a small sack");
    sack->SetDesc(u8"A small, durable, practical belt sack.");

    sack->SetSkill(prhash(u8"Wearable on Left Hip"), 1);
    sack->SetSkill(prhash(u8"Wearable on Right Hip"), 2);
    sack->SetSkill(prhash(u8"Container"), 5 * 454);
    sack->SetSkill(prhash(u8"Capacity"), 5);
    sack->SetSkill(prhash(u8"Closeable"), 1);

    sack->SetWeight(1 * 454);
    sack->SetSize(2);
    sack->SetVolume(1);
    sack->SetValue(100);

    sack->SetPos(pos_t::LIE);
    npc->AddAct(act_t::WEAR_RHIP, sack);

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
    const std::u8string& nm,
    const std::u8string& ds,
    const std::u8string& lds,
    const std::u8string& t,
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
    const std::u8string& nm,
    const std::u8string& ds,
    const std::u8string& lds,
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

static const std::u8string gen_replace[][4] = {
    {u8"{He}", u8"She", u8"He", u8"It"},
    {u8"{Him}", u8"Her", u8"Him", u8"It"},
    {u8"{His}", u8"Hers", u8"His", u8"Its"},
    {u8"{he}", u8"she", u8"he", u8"it"},
    {u8"{him}", u8"her", u8"him", u8"it"},
    {u8"{his}", u8"hers", u8"his", u8"its"},
    {u8"", u8"", u8"", u8""}};

std::u8string gender_proc(const std::u8string& in, char8_t gender) {
  std::u8string ret = in;
  int ctr = 0, gen = 3;

  if (gender == 'F')
    gen = 1;
  if (gender == 'M')
    gen = 2;

  for (ctr = 0; !gen_replace[ctr][0].empty(); ++ctr) {
    int where = ret.find(gen_replace[ctr][0]);
    while (where >= 0 && where <= int(ret.length())) {
      ret = ret.substr(0, where) + gen_replace[ctr][gen] +
          ret.substr(where + gen_replace[ctr][0].length());
      where = ret.find(gen_replace[ctr][0]);
    }
  }
  return ret;
}
