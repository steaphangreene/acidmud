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

#include <string>

#include "color.hpp"
#include "commands.hpp"
#include "log.hpp"
#include "mind.hpp"
#include "npc.hpp"
#include "object.hpp"
#include "properties.hpp"
#include "utils.hpp"

static const std::u8string gen_replace[][5] = {
    {u8"{He}", u8"It", u8"She", u8"He", u8"They"},
    {u8"{Him}", u8"It", u8"Her", u8"Him", u8"Their"},
    {u8"{His}", u8"Its", u8"Hers", u8"His", u8"Theirs"},
    {u8"{he}", u8"it", u8"she", u8"he", u8"they"},
    {u8"{him}", u8"it", u8"her", u8"him", u8"their"},
    {u8"{his}", u8"its", u8"hers", u8"his", u8"theirs"},
    {u8"", u8"", u8"", u8"", u8""}};

static std::u8string gender_proc(const std::u8string_view& in, gender_t gender) {
  std::u8string ret(in);
  int ctr = 0, gen = static_cast<int8_t>(gender) + 1;

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

static int rint1(auto& gen, int min, int max) { // Flat
  return std::uniform_int_distribution<int>(min, max)(gen);
}

static int rint3(auto& gen, int min, int max) { // Bell-Curve-Ish
  auto rando = std::uniform_int_distribution<int>(min, max);
  return (rando(gen) + rando(gen) + rando(gen) + 2) / 3;
}

static NPCType base_npc(
    u8"a person",
    u8"{He} seems normal.",
    u8"",
    {gender_t::MALE, gender_t::FEMALE},
    {u8"spear"},
    {u8"shirt"},
    {u8"pack"},
    {1, 1, 1, 1, 1, 1},
    {7, 7, 7, 7, 7, 7},
    0,
    0);

// Definitions for all built-in item tags go here.
static std::map<std::u8string_view, ItemType> itemtagdefs = {};

// Definitions for all built-in armor tags go here.
static std::map<std::u8string_view, ArmorType> armortagdefs = {
    {u8"shirt",
     {u8"a shirt",
      u8"A shirt.",
      u8"",
      {10000, 10, 150},
      {10000, 10, 150},
      {0, 0, 0, 0},
      {0, 0, 0, 0}}},
    {u8"jerkin",
     {u8"a jerkin",
      u8"A jerkin.",
      u8"",
      {10000, 10, 150},
      {10000, 10, 150},
      {0, 0, 0, 0},
      {1, 1, 1, 0}}},
    {u8"leather",
     {u8"a leather ",
      u8"A leather ",
      u8"",
      {10000, 10, 150},
      {10000, 10, 150},
      {1, 1, 0, 1},
      {1, 1, 0, 1}}},
    {u8"steel",
     {u8"a steel ",
      u8"A steel ",
      u8"",
      {10000, 10, 150},
      {10000, 10, 150},
      {0, 0, 2, 4},
      {0, 0, 2, 6}}},
    {u8"dwarven",
     {u8"a dwarven ",
      u8"A dwarven ",
      u8"",
      {10000, 10, 150},
      {10000, 10, 150},
      {0, 0, 0, 0},
      {1, 1, 0, 1}}},
    {u8"heavy",
     {u8"", u8"A heavy ", u8"", {10000, 10, 150}, {10000, 10, 150}, {1, 0, 0, 0}, {0, 1, 0, 0}}},
    {u8"breastplate",
     {u8"breastplate",
      u8"A breastplate.",
      u8"",
      {200000, 100, 15000},
      {200000, 100, 15000},
      {6, 4, 2, 1},
      {10, 6, 6, 2},
      act_t::WEAR_CHEST,
      act_t::WEAR_BACK}},
    {u8"great helm",
     {u8"a great helm",
      u8"A great helm.",
      u8"",
      {50000, 70, 5000},
      {50000, 70, 5000},
      {6, 4, 2, 1},
      {10, 6, 6, 2},
      act_t::WEAR_HEAD}},
    {u8"helm",
     {u8"a helm",
      u8"A helm.",
      u8"",
      {50000, 70, 5000},
      {50000, 70, 5000},
      {6, 4, 2, 1},
      {10, 6, 6, 2},
      act_t::WEAR_HEAD}},
    {u8"right arm plate",
     {u8"an arm plate (right)",
      u8"An arm plate.",
      u8"",
      {50000, 60, 5000},
      {50000, 60, 5000},
      {6, 4, 2, 1},
      {10, 6, 6, 2},
      act_t::WEAR_RARM}},
    {u8"left arm plate",
     {u8"an arm plate (left)",
      u8"An arm plate.",
      u8"",
      {50000, 60, 5000},
      {50000, 60, 5000},
      {6, 4, 2, 1},
      {10, 6, 6, 2},
      act_t::WEAR_LARM}},
    {u8"battle skirt",
     {u8"a battle skirt",
      u8"A battle skirt.",
      u8"",
      {100000, 80, 10000},
      {100000, 80, 10000},
      {6, 4, 2, 1},
      {10, 6, 6, 2},
      act_t::WEAR_RLEG,
      act_t::WEAR_LLEG}},
    {u8"shield",
     {u8"a shield",
      u8"A shield.",
      u8"",
      {100000, 80, 10000},
      {100000, 80, 10000},
      {6, 4, 2, 1},
      {10, 6, 6, 2},
      act_t::WEAR_SHIELD}},
};

// Definitions for all built-in weapon tags go here.
static std::map<std::u8string_view, WeaponType> weapontagdefs = {
    {u8"spear",
     {u8"a spear",
      u8"A spear.",
      u8"",
      u8"Long Piercing",
      {1000, 10, 1000},
      {1000, 10, 1000},
      {1, -2, 2},
      {1, 2, 2}}},
    {u8"sword",
     {u8"a sword",
      u8"A sword.",
      u8"",
      u8"Long Blades",
      {1000, 10, 1000},
      {1000, 10, 1000},
      {1, -2, 2},
      {1, 2, 2}}},
    {u8"axe",
     {u8"an axe",
      u8"An axe.",
      u8"",
      u8"Long Cleaves",
      {1000, 10, 1000},
      {1000, 10, 1000},
      {1, -2, 2},
      {1, 2, 2}}},
    {u8"pick",
     {u8"a pick",
      u8"A pick.",
      u8"",
      u8"Long Cleaves",
      {1000, 10, 1000},
      {1000, 10, 1000},
      {1, -2, 2},
      {1, 2, 2}}},
    {u8"pickaxe",
     {u8"a pickaxe",
      u8"A pickaxe.",
      u8"",
      u8"Two-Handed Cleaves",
      {1000, 10, 1000},
      {1000, 10, 1000},
      {2, -3, 3},
      {2, 1, 3}}},
    {u8"dwarven",
     {u8"dwarven ",
      u8"A super-strong ",
      u8"",
      u8"",
      {1000, 10, 1000},
      {1000, 10, 1000},
      {0, 1, 0},
      {0, 1, 0}}},
    {u8"combat",
     {u8"combat ",
      u8"A combat ",
      u8"",
      u8"",
      {4000, 10, 1000},
      {4000, 10, 1000},
      {0, 1, 0},
      {0, 2, 0}}},
    {u8"climbing",
     {u8"climbing ",
      u8"A lightweight ",
      u8"",
      u8"",
      {4000, 10, 1000},
      {4000, 10, 1000},
      {0, 2, -1},
      {0, 4, -1}}},
    {u8"sharp",
     {u8"", u8"A super-sharp ", u8"", u8"", {0, 0, 1000}, {0, 0, 1000}, {0, 1, 0}, {0, 1, 0}}},
    {u8"heavy",
     {u8"",
      u8"A super-heavy ",
      u8"",
      u8"",
      {1000, 0, 1000},
      {1000, 0, 1000},
      {0, 0, 0},
      {0, 0, 0}}},
    {u8"light",
     {u8"",
      u8"A super-light ",
      u8"",
      u8"",
      {-500, 0, 1000},
      {-500, 0, 1000},
      {0, 1, 0},
      {0, 1, 0}}},
    {u8"high-quality",
     {u8"",
      u8"A high-quality ",
      u8"",
      u8"",
      {4000, 10, 1000},
      {4000, 10, 1000},
      {0, 1, 0},
      {0, 1, 0}}},
    {u8"war",
     {u8"a war ",
      u8"A war ",
      u8"",
      u8"Two-Handed ",
      {20000, 40, 5000},
      {20000, 40, 5000},
      {1, 1, 1},
      {1, 0, 1}}},
};

// Definitions for all built-in NPC tags go here.
static std::map<std::u8string_view, NPCType> npctagdefs = {
    {u8"dwarf",
     {u8"a dwarven ",
      u8"a dwarven ",
      u8"{He} looks pissed.",
      {gender_t::NEITHER},
      {u8"axe", u8"dwarven"},
      {u8"dwarven"},
      {u8"dwarven"},
      {2, 0, 1, 0, 0, 2},
      {2, -2, 1, -2, 0, 2},
      100,
      500}},
    {u8"elf",
     {u8"an elven ",
      u8"an elven ",
      u8"{He} looks arrogant.",
      {gender_t::MALE, gender_t::FEMALE},
      {u8"sword", u8"elven"},
      {u8"elven"},
      {u8"elven"},
      {0, 2, 0, 2, 0, 0},
      {-2, 0, 0, 2, 0, 0},
      100,
      500}},
    {u8"civilian",
     {u8"a civilian",
      u8"",
      u8"{He} is not really a martial person.",
      {gender_t::MALE, gender_t::FEMALE},
      {},
      {},
      {},
      {0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0},
      200,
      500}},
    {u8"citizen",
     {u8"a citizen",
      u8"",
      u8"{He} clearly has a solid place in society.",
      {gender_t::MALE, gender_t::FEMALE},
      {},
      {},
      {},
      {0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0},
      200,
      500}},
    {u8"slave",
     {u8"a slave",
      u8"",
      u8"{He} seems to have had a hard life.  {His} will was broken long ago.",
      {gender_t::MALE, gender_t::FEMALE},
      {u8"none"},
      {u8"none"},
      {u8"none"},
      {2, 2, 2, 0, 0, 0},
      {0, 0, 0, 0, 0, -4},
      -1000000,
      -1000000}},
    {u8"miner",
     {u8"a miner",
      u8"",
      u8"{He} clearly has digging in {his} blood.",
      {gender_t::MALE, gender_t::FEMALE},
      {u8"pickaxe"},
      {},
      {},
      {0, 0, 2, 0, 0, 2},
      {0, 0, 0, 0, 0, 1},
      200,
      500}},
    {u8"explorer",
     {u8"an explorer",
      u8"",
      u8"{He} seems to be motivated only by what is not yet known or discovered.",
      {gender_t::MALE, gender_t::FEMALE},
      {u8"pick", u8"climbing"},
      {},
      {},
      {0, 1, 0, 0, 1, 0},
      {0, 1, 0, 0, 1, 0},
      200,
      500}},
    {u8"engineer",
     {u8"an engineer",
      u8"",
      u8"{He} is reflexively inspecting all the construction around {him}.",
      {gender_t::MALE, gender_t::FEMALE},
      {u8"pick"},
      {},
      {},
      {0, 0, 0, 0, 2, 1},
      {0, 0, 0, 0, 2, 1},
      200,
      500}},
    {u8"master",
     {u8"the master",
      u8"",
      u8"{He} is among the best at what {he} does.",
      {gender_t::MALE, gender_t::FEMALE},
      {u8"hammer"},
      {},
      {},
      {0, 1, 1, 1, 2, 2},
      {0, 1, 1, 1, 2, 2},
      200,
      500}},
    {u8"leader",
     {u8"a leader",
      u8"a master of inspiring others.",
      u8"{He} is an experienced leader of people.",
      {gender_t::MALE, gender_t::FEMALE},
      {},
      {},
      {},
      {0, 0, 0, 2, 0, 0},
      {0, 0, 0, 2, 0, 0},
      500,
      1000}},
    {u8"guard",
     {u8"a guard",
      u8"a professional guard.",
      u8"{He} seems to take his job guarding stuff very seriously.",
      {gender_t::MALE, gender_t::FEMALE},
      {u8"combat"},
      {u8"breastplate", u8"helm", u8"shield"},
      {},
      {0, 0, 1, 0, 1, 1},
      {0, 0, 1, 0, 1, 1},
      100,
      200}},
    {u8"soldier",
     {u8"a soldier",
      u8"a professional soldier.",
      u8"{He} is prepared for war, from wherever it may come.",
      {gender_t::MALE, gender_t::FEMALE},
      {u8"war"},
      {u8"breastplate", u8"great helm", u8"battle skirt", u8"left arm plate", u8"right arm plate"},
      {},
      {0, 0, 1, 0, 1, 1},
      {0, 0, 1, 0, 1, 1},
      100,
      200}},
    {u8"elite",
     {u8"an elite ",
      u8"a pretty special ",
      u8"{He} seems quite prepared, and well-trained.",
      {gender_t::MALE, gender_t::FEMALE},
      {u8"high-quality"},
      {u8"high-quality"},
      {},
      {2, 2, 2, 2, 2, 2},
      {2, 2, 2, 2, 2, 2},
      1000,
      2000}},
};

static std::map<act_t, std::u8string> wear_attribs = {
    {act_t::WEAR_BACK, u8"Wearable on Back"},
    {act_t::WEAR_CHEST, u8"Wearable on Chest"},
    {act_t::WEAR_HEAD, u8"Wearable on Head"},
    {act_t::WEAR_NECK, u8"Wearable on Neck"},
    {act_t::WEAR_COLLAR, u8"Wearable on Collar"},
    {act_t::WEAR_WAIST, u8"Wearable on Waist"},
    {act_t::WEAR_SHIELD, u8"Wearable on Shield"},
    {act_t::WEAR_LARM, u8"Wearable on Left Arm"},
    {act_t::WEAR_RARM, u8"Wearable on Right Arm"},
    {act_t::WEAR_LFINGER, u8"Wearable on Left Finger"},
    {act_t::WEAR_RFINGER, u8"Wearable on Right Finger"},
    {act_t::WEAR_LFOOT, u8"Wearable on Left Foot"},
    {act_t::WEAR_RFOOT, u8"Wearable on Right Foot"},
    {act_t::WEAR_LHAND, u8"Wearable on Left Hand"},
    {act_t::WEAR_RHAND, u8"Wearable on Right Hand"},
    {act_t::WEAR_LLEG, u8"Wearable on Left Leg"},
    {act_t::WEAR_RLEG, u8"Wearable on Right Leg"},
    {act_t::WEAR_LWRIST, u8"Wearable on Left Wrist"},
    {act_t::WEAR_RWRIST, u8"Wearable on Right Wrist"},
    {act_t::WEAR_LSHOULDER, u8"Wearable on Left Shoulder"},
    {act_t::WEAR_RSHOULDER, u8"Wearable on Right Shoulder"},
    {act_t::WEAR_LHIP, u8"Wearable on Left Hip"},
    {act_t::WEAR_RHIP, u8"Wearable on Right Hip"},
    {act_t::WEAR_FACE, u8"Wearable on Face"},
};

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
    const std::vector<gender_t>& gens,
    const std::vector<std::u8string_view>& weapons,
    const std::vector<std::u8string_view>& armor,
    const std::vector<std::u8string_view>& items,
    NPCAttrs min,
    NPCAttrs max,
    int gmin,
    int gmax) {
  short_desc_ = sds;
  desc_ = ds;
  long_desc_ = lds;
  genders_ = gens;
  min_ = min;
  max_ = max;
  min_gold_ = gmin;
  max_gold_ = gmax;

  wtags_.insert(wtags_.end(), weapons.begin(), weapons.end());
  atags_.insert(atags_.end(), armor.begin(), armor.end());
  itags_.insert(itags_.end(), items.begin(), items.end());
}

static std::u8string desc_merge(std::u8string_view d1, std::u8string_view d2) {
  if (d1.length() == 0) {
    return std::u8string(d2);
  } else if (d2.length() == 0) {
    return std::u8string(d1);
  } else if (d1.back() == ' ') {
    process(d2, u8"a ");
    process(d2, u8"an ");
    process(d2, u8"A ");
    process(d2, u8"An ");
    bool add_definite = (process(d2, u8"the ") || process(d2, u8"The "));
    if (add_definite) {
      add_definite &=
          (process(d1, u8"a ") || process(d1, u8"an ") || process(d1, u8"A ") ||
           process(d1, u8"An "));
    }
    if (add_definite) {
      return fmt::format(u8"the {}{}", d1, d2);
    } else {
      return fmt::format(u8"{}{}", d1, d2);
    }
  } else if (d2.back() == ' ') {
    return (desc_merge(d2, d1));
  } else {
    return fmt::format(u8"{}", d2); // New base after old base, replace *all* previous text
  }
}

void NPCType::operator+=(const NPCType& in) {
  // TODO: Real string compositions
  short_desc_ = desc_merge(short_desc_, in.short_desc_);
  desc_ = desc_merge(desc_, in.desc_);
  if (in.long_desc_ != u8"") {
    long_desc_ += '\n';
    long_desc_ += in.long_desc_;
  }

  // TODO: Real set operations
  if (genders_.size() > in.genders_.size()) {
    genders_ = in.genders_;
  }

  for (auto i : {0, 1, 2, 3, 4, 5}) {
    min_.v[i] += in.min_.v[i];
    max_.v[i] += in.max_.v[i];
  }
  min_gold_ += in.min_gold_;
  max_gold_ += in.max_gold_;

  wtags_.insert(wtags_.end(), in.wtags_.begin(), in.wtags_.end());
  atags_.insert(atags_.end(), in.atags_.begin(), in.atags_.end());
  itags_.insert(itags_.end(), in.itags_.begin(), in.itags_.end());
}

void NPCType::Skill(uint32_t stok, int percent, int mod) {
  skills_[stok] = std::make_pair(percent, mod);
}

void NPCType::Skill(uint32_t stok, int val) {
  skills_[stok] = std::make_pair(-1, val);
}

void NPCType::Arm(const WeaponType& weap) {
  weapons_.emplace_back(weap);
}

void NPCType::Armor(const ArmorType& arm) {
  armor_.emplace_back(arm);
}

void NPCType::Carry(const ItemType& item) {
  items_.emplace_back(item);
}

void NPCType::SetShortDesc(const std::u8string_view& sds) {
  short_desc_ = sds;
}

void NPCType::FinalizeTags() {
  // Merge Given Weapon Tags into Weapon Defs
  for (auto wtag : wtags_) {
    if (weapontagdefs.contains(wtag)) {
      if (weapons_.size() == 0) {
        weapons_.emplace_back(weapontagdefs.at(wtag));
      } else {
        weapons_.back() += weapontagdefs.at(wtag);
      }
    } else {
      loger(u8"ERROR: Use of undefined Weapon tag: '{}'.  Skipping.\n", wtag);
    }
  }

  // Merge Given Armor Tags into Armor Defs
  for (auto atag : atags_) {
    if (armortagdefs.contains(atag)) {
      if (armor_.size() == 0) {
        armor_.emplace_back(armortagdefs.at(atag));
      } else if (armortagdefs.at(atag).loc_.size() > 0) {
        armor_.emplace_back(armortagdefs.at(atag));
      } else if (armor_.front().loc_.size() > 0) {
        armor_.emplace(armor_.begin(), armortagdefs.at(atag));
      } else {
        armor_.front() += armortagdefs.at(atag);
      }
    } else {
      loger(u8"ERROR: Use of undefined Armor tag: '{}'.  Skipping.\n", atag);
    }
  }
  if (armor_.size() > 0 && armor_.front().loc_.size() == 0) { // Unlocated up front
    for (auto arm : armor_) { // Merge with the individually located bits
      if (arm.loc_.size() > 0) {
        arm += armor_.front();
      }
    }
  }

  // Merge Given Item Tags into Item Defs
  for (auto itag : itags_) {
    if (itemtagdefs.contains(itag)) {
      items_.emplace_back(itemtagdefs.at(itag));
    } else {
      loger(u8"ERROR: Use of undefined Item tag: '{}'.  Skipping.\n", itag);
    }
  }
}

Object* Object::AddNPC(std::mt19937& gen, const std::u8string_view& tags) {
  auto npcdef = base_npc;
  auto start = tags.cbegin();
  auto end = std::find(start, tags.cend(), ',');
  while (start != tags.cend()) {
    std::u8string_view tag = tags.substr(start - tags.cbegin(), end - start);
    if (npctagdefs.contains(tag)) {
      npcdef += npctagdefs.at(tag);
    } else {
      loger(u8"ERROR: Use of undefined NPC tag: '{}'.  Skipping.\n", tag);
    }
    start = end;
    if (start != tags.cend()) {
      ++start;
      end = std::find(start, tags.cend(), ',');
    }
  }
  npcdef.FinalizeTags();

  return AddNPC(gen, &npcdef, tags);
}

Object* Object::AddNPC(std::mt19937& gen, const NPCType* type, const std::u8string_view& tags) {
  Object* npc = new Object(this);
  npc->SetTags(tags);

  if (npc->World()) {
    auto obj_id = npc->World()->Skill(prhash(u8"Last Object ID")) + 1;
    npc->World()->SetSkill(prhash(u8"Last Object ID"), obj_id);
    npc->SetSkill(prhash(u8"Object ID"), obj_id);
  }

  npc->Attach(new Mind(mind_t::NPC));
  npc->Activate();
  npc->SetPos(pos_t::STAND);
  for (int a : {0, 1, 2, 3, 4, 5}) {
    npc->SetAttribute(a, rint3(gen, type->min_.v[a], type->max_.v[a]));
  }

  for (auto sk : type->skills_) {
    if (sk.second.first < 0) {
      npc->SetSkill(sk.first, sk.second.second);
    } else {
      npc->SetSkill(
          sk.first,
          npc->NormAttribute(get_linked(sk.first)) * sk.second.first / 100 -
              rint1(gen, 0, sk.second.second));
    }
    // loge(u8"DBG: {} {} {}\n", get_linked(sk.first), sk.second.first, sk.second.second);
  }

  if (type->genders_.size() > 0) {
    npc->SetGender(type->genders_[rint1(gen, 0, type->genders_.size() - 1)]);
  }

  npc->SetShortDesc(gender_proc(type->short_desc_, npc->Gender()));
  npc->SetDesc(gender_proc(type->desc_, npc->Gender()));
  npc->SetLongDesc(gender_proc(type->long_desc_, npc->Gender()));

  int gidx = (npc->Gender() == gender_t::FEMALE) ? 0 : 1;
  std::vector<std::u8string> first = {u8""};
  std::sample(
      dwarf_first_names[gidx].begin(), dwarf_first_names[gidx].end(), first.begin(), 1, gen);
  std::vector<std::u8string> last = {u8""};
  std::sample(dwarf_last_names.begin(), dwarf_last_names.end(), last.begin(), 1, gen);
  npc->SetName(first.front() + u8" " + last.front());

  if (type->min_gold_ > 0 || type->max_gold_ > 0) {
    int num_gold = rint3(gen, type->min_gold_, type->max_gold_);
    if (num_gold > 0) {
      give_gold(npc, num_gold);
    }
  }

  if (true) { // TODO: Figure out who should get these, and who should not
    add_pouch(npc);
  }

  for (auto wp : type->weapons_) {
    Object* obj = new Object(npc);
    obj->SetSkill(prhash(u8"WeaponType"), wp.wtype_);
    obj->SetSkill(prhash(u8"WeaponReach"), rint3(gen, wp.wmin_.reach, wp.wmax_.reach));
    obj->SetSkill(prhash(u8"WeaponForce"), rint3(gen, wp.wmin_.force, wp.wmax_.force));
    obj->SetSkill(prhash(u8"WeaponSeverity"), rint3(gen, wp.wmin_.severity, wp.wmax_.severity));
    obj->SetShortDesc(wp.short_desc_);
    obj->SetDesc(wp.desc_);
    obj->SetLongDesc(wp.long_desc_);
    obj->SetWeight(rint3(gen, wp.min_.weight, wp.max_.weight));
    obj->SetSize(rint3(gen, wp.min_.size, wp.max_.size));
    obj->SetVolume(rint3(gen, wp.min_.volume, wp.max_.volume));
    obj->SetValue(rint3(gen, wp.min_.value, wp.max_.value));
    obj->SetPos(pos_t::LIE);
    npc->AddAct(act_t::WIELD, obj);
    if (two_handed(wp.wtype_)) {
      npc->AddAct(act_t::HOLD, obj);
    }
  }

  for (auto ar : type->armor_) {
    Object* obj = new Object(npc);
    obj->SetAttribute(0, rint3(gen, ar.amin_.bulk, ar.amax_.bulk));
    obj->SetSkill(prhash(u8"ArmorB"), rint3(gen, ar.amin_.bulk, ar.amax_.bulk));
    obj->SetSkill(prhash(u8"ArmorI"), rint3(gen, ar.amin_.impact, ar.amax_.impact));
    obj->SetSkill(prhash(u8"ArmorT"), rint3(gen, ar.amin_.thread, ar.amax_.thread));
    obj->SetSkill(prhash(u8"ArmorP"), rint3(gen, ar.amin_.planar, ar.amax_.planar));
    obj->SetShortDesc(ar.short_desc_);
    obj->SetDesc(ar.desc_);
    obj->SetLongDesc(ar.long_desc_);
    obj->SetWeight(rint3(gen, ar.min_.weight, ar.max_.weight));
    obj->SetSize(rint3(gen, ar.min_.size, ar.max_.size));
    obj->SetVolume(rint3(gen, ar.min_.volume, ar.max_.volume));
    obj->SetValue(rint3(gen, ar.min_.value, ar.max_.value));
    obj->SetPos(pos_t::LIE);

    for (auto loc : ar.loc_) {
      obj->SetSkill(wear_attribs[loc], 1);
      npc->AddAct(loc, obj);
    }
  }

  if (type->items_.size() > 0) {
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

    for (auto it : type->items_) {
      Object* obj = new Object(sack);
      for (auto sk : it.skills_) {
        obj->SetSkill(sk.first, sk.second);
      }
      obj->SetShortDesc(it.short_desc_);
      obj->SetDesc(it.desc_);
      obj->SetLongDesc(it.long_desc_);
      obj->SetWeight(rint3(gen, it.min_.weight, it.max_.weight));
      obj->SetSize(rint3(gen, it.min_.size, it.max_.size));
      obj->SetVolume(rint3(gen, it.min_.volume, it.max_.volume));
      obj->SetValue(rint3(gen, it.min_.value, it.max_.value));
      obj->SetPos(pos_t::LIE);
    }
  }
  return npc;
}

void WeaponType::operator+=(const WeaponType& in) {
  // TODO: Real string compositions
  short_desc_ = desc_merge(short_desc_, in.short_desc_);
  desc_ = desc_merge(desc_, in.desc_);
  if (in.long_desc_ != u8"") {
    long_desc_ += '\n';
    long_desc_ += in.long_desc_;
  }

  // TODO: Real set operations on tags

  wmin_.reach += in.wmin_.reach;
  wmax_.reach += in.wmax_.reach;
  wmin_.force += in.wmin_.force;
  wmax_.force += in.wmax_.force;
  wmin_.severity += in.wmin_.severity;
  wmax_.severity += in.wmax_.severity;

  min_.weight += in.min_.weight;
  max_.weight += in.max_.weight;
  min_.size += in.min_.size;
  max_.size += in.max_.size;
  min_.volume += in.min_.volume;
  max_.volume += in.max_.volume;
  min_.value += in.min_.value;
  max_.value += in.max_.value;

  if (in.wtype_ != 0) {
    wtype_ = in.wtype_;
  }
}

WeaponType::WeaponType(
    const std::u8string& nm,
    const std::u8string& ds,
    const std::u8string& lds,
    const std::u8string& t,
    ItemAttrs min,
    ItemAttrs max,
    WeaponAttrs wmin,
    WeaponAttrs wmax) {
  short_desc_ = nm;
  desc_ = ds;
  long_desc_ = lds;
  wtype_ = get_weapon_type(t);
  wmin_ = wmin;
  wmax_ = wmax;
  min_ = min;
  max_ = max;
}

void ArmorType::operator+=(const ArmorType& in) {
  // TODO: Real string compositions
  short_desc_ = desc_merge(short_desc_, in.short_desc_);
  desc_ = desc_merge(desc_, in.desc_);
  if (in.long_desc_ != u8"") {
    long_desc_ += '\n';
    long_desc_ += in.long_desc_;
  }

  // TODO: Real set operations on tags

  amin_.bulk += in.amin_.bulk;
  amax_.bulk += in.amax_.bulk;
  amin_.impact += in.amin_.impact;
  amax_.impact += in.amax_.impact;
  amin_.thread += in.amin_.thread;
  amax_.thread += in.amax_.thread;
  amin_.planar += in.amin_.planar;
  amax_.planar += in.amax_.planar;

  min_.weight += in.min_.weight;
  max_.weight += in.max_.weight;
  min_.size += in.min_.size;
  max_.size += in.max_.size;
  min_.volume += in.min_.volume;
  max_.volume += in.max_.volume;
  min_.value += in.min_.value;
  max_.value += in.max_.value;

  // FIXME: Make this sort armor correctly
  if (loc_.empty()) {
    loc_ = in.loc_;
  }
}

ArmorType::ArmorType(
    const std::u8string& nm,
    const std::u8string& ds,
    const std::u8string& lds,
    ItemAttrs min,
    ItemAttrs max,
    ArmorAttrs amin,
    ArmorAttrs amax,
    act_t l1,
    act_t l2,
    act_t l3,
    act_t l4,
    act_t l5,
    act_t l6) {
  short_desc_ = nm;
  desc_ = ds;
  long_desc_ = lds;
  min_ = min;
  max_ = max;
  amin_ = amin;
  amax_ = amax;

  if (l1 != act_t::NONE)
    loc_.push_back(l1);
  if (l2 != act_t::NONE)
    loc_.push_back(l2);
  if (l3 != act_t::NONE)
    loc_.push_back(l3);
  if (l4 != act_t::NONE)
    loc_.push_back(l4);
  if (l5 != act_t::NONE)
    loc_.push_back(l5);
  if (l6 != act_t::NONE)
    loc_.push_back(l6);
}

ItemType::ItemType(
    const std::u8string& nm,
    const std::u8string& ds,
    const std::u8string& lds,
    const std::vector<skill_pair>& sk,
    ItemAttrs min,
    ItemAttrs max) {
  short_desc_ = nm;
  desc_ = ds;
  long_desc_ = lds;
  skills_ = sk;
  min_ = min;
  max_ = max;
}
