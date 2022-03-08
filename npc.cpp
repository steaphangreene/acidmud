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

static const std::u8string gen_replace[][4] = {
    {u8"{He}", u8"She", u8"He", u8"It"},
    {u8"{Him}", u8"Her", u8"Him", u8"It"},
    {u8"{His}", u8"Hers", u8"His", u8"Its"},
    {u8"{he}", u8"she", u8"he", u8"it"},
    {u8"{him}", u8"her", u8"him", u8"it"},
    {u8"{his}", u8"hers", u8"his", u8"its"},
    {u8"", u8"", u8"", u8""}};

static std::u8string gender_proc(const std::u8string_view& in, char8_t gender) {
  std::u8string ret(in);
  int ctr = 0, gen = 3;

  if (gender == 'F')
    gen = 1;
  if (gender == 'M')
    gen = 2;

  for (ctr = 0; !gen_replace[ctr][0].empty(); ++ctr) {
    size_t where = ret.find(gen_replace[ctr][0]);
    while (where <= ret.length()) {
      ret = ret.substr(0, where) + gen_replace[ctr][gen] +
          ret.substr(where + gen_replace[ctr][0].length());
      where = ret.find(gen_replace[ctr][0]);
    }
  }
  return ret;
}

// From: https://gist.github.com/Brennall/b9c3a0202eb11c5cfd54868c5752012a
static const std::vector<std::u8string> dwarf_first_names[2] = {
    {
        u8"Anbera",    u8"Artin",    u8"Audhild",  u8"Balifra",  u8"Barbena",  u8"Bardryn",
        u8"Bolhild",   u8"Dagnal",   u8"Dafifi",   u8"Delre",    u8"Diesa",    u8"Hdeth",
        u8"Eridred",   u8"Falkrunn", u8"Fallthra", u8"Finelien", u8"Gillydd",  u8"Gunnloda",
        u8"Gurdis",    u8"Helgret",  u8"Helja",    u8"Hlin",     u8"llde",     u8"Jarana",
        u8"Kathra",    u8"Kilia",    u8"Kristryd", u8"Liftrasa", u8"Marastyr", u8"Mardred",
        u8"Morana",    u8"Nalaed",   u8"Nora",     u8"Nurkara",  u8"Orifi",    u8"Ovina",
        u8"Riswynn",   u8"Sannl",    u8"Therlin",  u8"Thodris",  u8"Torbera",  u8"Tordrid",
        u8"Torgga",    u8"Urshar",   u8"Valida",   u8"Vistra",   u8"Vonana",   u8"Werydd",
        u8"Whurd red", u8"Yurgunn",
    },
    {
        u8"Adrik",   u8"Alberich", u8"Baern",   u8"Barendd",  u8"Beloril", u8"Brottor", u8"Dain",
        u8"Dalgal",  u8"Darrak",   u8"Delg",    u8"Duergath", u8"Dworic",  u8"Eberk",   u8"Einkil",
        u8"Elaim",   u8"Erias",    u8"Fallond", u8"Fargrim",  u8"Gardain", u8"Gilthur", u8"Gimgen",
        u8"Gimurt",  u8"Harbek",   u8"Kildrak", u8"Kilvar",   u8"Morgran", u8"Morkral", u8"Nalral",
        u8"Nordak",  u8"Nuraval",  u8"Oloric",  u8"Olunt",    u8"Osrik",   u8"Oskar",   u8"Rangrim",
        u8"Reirak",  u8"Rurik",    u8"Taklinn", u8"Thoradin", u8"Thorin",  u8"Thradal", u8"Tordek",
        u8"Traubon", u8"Travok",   u8"Ulfgar",  u8"Uraim",    u8"Veit",    u8"Vonbin",  u8"Vondal",
        u8"Whurbin",
    }};

// From: https://gist.github.com/Brennall/b9c3a0202eb11c5cfd54868c5752012a
static const std::vector<std::u8string> dwarf_last_names = {
    u8"Aranore",    u8"Balderk",      u8"Battlehammer", u8"Bigtoe",      u8"Bloodkith",
    u8"Bofdarm",    u8"Brawnanvil",   u8"Brazzik",      u8"Broodfist",   u8"Burrowfound",
    u8"Caebrek",    u8"Daerdahk",     u8"Dankil",       u8"Daraln",      u8"Deepdelver",
    u8"Durthane",   u8"Eversharp",    u8"FaHack",       u8"Fire-forge",  u8"Foamtankard",
    u8"Frostbeard", u8"Glanhig",      u8"Goblinbane",   u8"Goldfinder",  u8"Gorunn",
    u8"Graybeard",  u8"Hammerstone",  u8"Helcral",      u8"Holderhek",   u8"Ironfist",
    u8"Loderr",     u8"Lutgehr",      u8"Morigak",      u8"Orcfoe",      u8"Rakankrak",
    u8"Ruby-Eye",   u8"Rumnaheim",    u8"Silveraxe",    u8"Silverstone", u8"Steelfist",
    u8"Stoutale",   u8"Strakeln",     u8"Strongheart",  u8"Thrahak",     u8"Torevir",
    u8"Torunn",     u8"Trollbleeder", u8"Trueanvil",    u8"Trueblood",   u8"Ungart",
};

static NPCType npc_dwarf(
    u8"a dwarf",
    u8"{He} looks pissed.",
    u8"",
    u8"MF",
    {9, 4, 6, 4, 9, 4},
    {10, 7, 11, 8, 18, 8},
    100,
    500);

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
    const std::u8string& sds,
    const std::u8string& ds,
    const std::u8string& lds,
    const std::u8string& gens,
    NPCAttrs min,
    NPCAttrs max,
    int gmin,
    int gmax) {
  short_desc = sds;
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

void NPCType::SetShortDesc(const std::u8string_view& sds) {
  short_desc = sds;
}

void Object::AddNPC(std::mt19937& gen, const std::u8string_view& tags) {
  // TODO: Actually process tags to create NPCType
  npc_dwarf.SetShortDesc(tags);
  return AddNPC(gen, &npc_dwarf);
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

  if (type->genders.length() > 0) {
    npc->SetGender(type->genders[rand() % type->genders.length()]);
  }

  npc->SetShortDesc(gender_proc(type->short_desc, npc->Gender()));
  npc->SetDesc(gender_proc(type->desc, npc->Gender()));
  npc->SetLongDesc(gender_proc(type->long_desc, npc->Gender()));

  int gidx = (npc->Gender() == 'F') ? 0 : 1;
  std::vector<std::u8string> first = {u8""};
  std::sample(
      dwarf_first_names[gidx].begin(), dwarf_first_names[gidx].end(), first.begin(), 1, gen);
  std::vector<std::u8string> last = {u8""};
  std::sample(dwarf_last_names.begin(), dwarf_last_names.end(), last.begin(), 1, gen);
  npc->SetName(first.front() + u8" " + last.front());

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
