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

#include "color.hpp"
#include "commands.hpp"
#include "log.hpp"
#include "mind.hpp"
#include "object.hpp"
#include "properties.hpp"
#include "utils.hpp"

struct ItemAttrs {
  int weight;
  int size;
  int volume;
  int value;
};

class ItemType {
 public:
  ItemType(const std::u8string_view& tagdef);
  ItemType(std::u8string_view& tagdef);
  bool LoadFrom(std::u8string_view& tagdef);
  void operator+=(const ItemType&);
  std::u8string short_desc_, desc_, long_desc_;
  std::vector<skill_pair> props_;
  std::vector<std::vector<act_t>> loc_;
  ItemAttrs min_ = {0, 0, 0, 0};
  ItemAttrs max_ = {0, 0, 0, 0};
};

struct ArmorAttrs {
  int bulk; // Body-Type (adds mass, reduces shock)
  int impact; // Cushion (absorbs shock, softens impact)
  int thread; // Tensile (won't rip/cut, prevents slashing)
  int planar; // Plate (won't bend, spreads impact)
};

class ArmorType {
 public:
  void operator+=(const ArmorType&);
  ArmorType(const std::u8string_view& tagdef);
  ArmorType(std::u8string_view& tagdef);
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

class WeaponType {
 public:
  void operator+=(const WeaponType&);
  WeaponType(const std::u8string_view& tagdef);
  WeaponType(std::u8string_view& tagdef);
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

class NPCType {
 public:
  void operator+=(const NPCType&);
  NPCType(const std::u8string_view& tagdef);
  NPCType(std::u8string_view& tagdef);
  bool LoadFrom(std::u8string_view& tagdef);
  void Prop(uint32_t, int, int);
  void Prop(uint32_t, int);
  void Arm(const WeaponType&);
  void Armor(const ArmorType&);
  void Carry(const ItemType&);
  void SetShortDesc(const std::u8string_view&);
  void FinalizeWeaponTags(const std::map<std::u8string, WeaponType>&);
  void FinalizeArmorTags(const std::map<std::u8string, ArmorType>&);
  void FinalizeItemTags(const std::map<std::u8string, ItemType>&);

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
  std::vector<WeaponType> weapons_;
  std::vector<ArmorType> armor_;
  std::vector<ItemType> items_;
};

static std::map<const Object*, std::map<std::u8string, NPCType>> npctagdefs;
static std::map<const Object*, std::map<std::u8string, WeaponType>> weapontagdefs;
static std::map<const Object*, std::map<std::u8string, ArmorType>> armortagdefs;
static std::map<const Object*, std::map<std::u8string, ItemType>> itemtagdefs;

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
    u8"short:a person\n"
    u8"desc:{He} seems normal.\n"
    u8"genders:FM\n"
    u8"wtag:spear\n"
    u8"atag:shirt\n"
    u8"itag:pack\n"
    u8"b:1-7\n"
    u8"q:1-7\n"
    u8"s:1-7\n"
    u8"c:1-7\n"
    u8"i:1-7\n"
    u8"w:1-7\n");

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

static bool intparam(std::u8string_view& line, const std::u8string_view& lab, int& min, int& max) {
  if (process(line, lab)) {
    skipspace(line);
    min = nextnum(line);
    skipspace(line);
    if (line.length() > 0 && nextchar(line) == '-') {
      skipspace(line);
      max = nextnum(line);
      skipspace(line);
    } else {
      max = min;
    }
    return true;
  } else {
    return false;
  }
}

static bool wearparam(std::u8string_view& line, std::vector<std::vector<act_t>>& defs) {
  if (process(line, u8"wear:")) {
    defs.push_back(std::vector<act_t>());
    while (line.length() > 0) {
      std::u8string wearstr(getuntil(line, ','));
      std::transform(wearstr.begin(), wearstr.end(), wearstr.begin(), ascii_toupper);
      std::u8string_view wear = wearstr;
      if (wear.starts_with(u8"SHIE")) {
        defs.back().push_back(act_t::WEAR_SHIELD);
      } else if (wear.starts_with(u8"BACK")) {
        defs.back().push_back(act_t::WEAR_BACK);
      } else if (wear.starts_with(u8"CHES")) {
        defs.back().push_back(act_t::WEAR_CHEST);
      } else if (wear.starts_with(u8"HEAD")) {
        defs.back().push_back(act_t::WEAR_HEAD);
      } else if (wear.starts_with(u8"FACE")) {
        defs.back().push_back(act_t::WEAR_FACE);
      } else if (wear.starts_with(u8"NECK")) {
        defs.back().push_back(act_t::WEAR_NECK);
      } else if (wear.starts_with(u8"COLL")) {
        defs.back().push_back(act_t::WEAR_COLLAR);
      } else if (wear.starts_with(u8"WAIS")) {
        defs.back().push_back(act_t::WEAR_WAIST);
      } else if (
          process(wear, u8"LEFT_") || process(wear, u8"LEFT.") || process(wear, u8"LEFT ") ||
          process(wear, u8"LEFT") || process(wear, u8"L_") || process(wear, u8"L.") ||
          process(wear, u8"L ") || process(wear, u8"L")) {
        if (wear.starts_with(u8"ARM")) {
          defs.back().push_back(act_t::WEAR_LARM);
        } else if (wear.starts_with(u8"ARM")) {
          defs.back().push_back(act_t::WEAR_LARM);
        } else if (wear.starts_with(u8"FIN")) {
          defs.back().push_back(act_t::WEAR_LFINGER);
        } else if (wear.starts_with(u8"FOO")) {
          defs.back().push_back(act_t::WEAR_LFOOT);
        } else if (wear.starts_with(u8"HAN")) {
          defs.back().push_back(act_t::WEAR_LHAND);
        } else if (wear.starts_with(u8"LEG")) {
          defs.back().push_back(act_t::WEAR_LLEG);
        } else if (wear.starts_with(u8"SHO")) {
          defs.back().push_back(act_t::WEAR_LSHOULDER);
        } else if (wear.starts_with(u8"WRI")) {
          defs.back().push_back(act_t::WEAR_LWRIST);
        } else if (wear.starts_with(u8"HIP")) {
          defs.back().push_back(act_t::WEAR_LHIP);
        } else {
          loger(u8"ERROR: Unknown left wear location: '{}'\n", line);
        }
      } else if (
          process(wear, u8"RIGHT_") || process(wear, u8"RIGHT.") || process(wear, u8"RIGHT ") ||
          process(wear, u8"RIGHT") || process(wear, u8"R_") || process(wear, u8"R.") ||
          process(wear, u8"R ") || process(wear, u8"R")) {
        if (wear.starts_with(u8"ARM")) {
          defs.back().push_back(act_t::WEAR_RARM);
        } else if (wear.starts_with(u8"ARM")) {
          defs.back().push_back(act_t::WEAR_RARM);
        } else if (wear.starts_with(u8"FIN")) {
          defs.back().push_back(act_t::WEAR_RFINGER);
        } else if (wear.starts_with(u8"FOO")) {
          defs.back().push_back(act_t::WEAR_RFOOT);
        } else if (wear.starts_with(u8"HAN")) {
          defs.back().push_back(act_t::WEAR_RHAND);
        } else if (wear.starts_with(u8"LEG")) {
          defs.back().push_back(act_t::WEAR_RLEG);
        } else if (wear.starts_with(u8"SHO")) {
          defs.back().push_back(act_t::WEAR_RSHOULDER);
        } else if (wear.starts_with(u8"WRI")) {
          defs.back().push_back(act_t::WEAR_RWRIST);
        } else if (wear.starts_with(u8"HIP")) {
          defs.back().push_back(act_t::WEAR_RHIP);
        } else {
          loger(u8"ERROR: Unknown right wear location: '{}'\n", line);
        }
      } else {
        loger(u8"ERROR: Unknown wear location: '{}'\n", line);
      }
    }
    return true;
  } else {
    return false;
  }
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
  if (genders_.size() == 0 || (in.genders_.size() > 0 && genders_.size() > in.genders_.size())) {
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

void NPCType::Prop(uint32_t stok, int sval) {
  props_.push_back({stok, sval});
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

void NPCType::FinalizeWeaponTags(const std::map<std::u8string, WeaponType>& tagdefs) {
  // Merge Given Weapon Tags into Weapon Defs
  for (auto wtag : wtags_) {
    if (tagdefs.contains(wtag)) {
      if (weapons_.size() == 0) {
        weapons_.emplace_back(tagdefs.at(wtag));
      } else {
        weapons_.back() += tagdefs.at(wtag);
      }
    } else {
      loger(u8"ERROR: Use of undefined Weapon tag: '{}'.  Skipping.\n", wtag);
    }
  }
}

void NPCType::FinalizeArmorTags(const std::map<std::u8string, ArmorType>& tagdefs) {
  // Merge Given Armor Tags into Armor Defs
  for (auto atag : atags_) {
    if (tagdefs.contains(atag)) {
      if (armor_.size() == 0) {
        armor_.emplace_back(tagdefs.at(atag));
      } else if (tagdefs.at(atag).loc_.size() > 0) {
        armor_.emplace_back(tagdefs.at(atag));
      } else if (armor_.front().loc_.size() > 0) {
        armor_.emplace(armor_.begin(), tagdefs.at(atag));
      } else {
        armor_.front() += tagdefs.at(atag);
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
}

void NPCType::FinalizeItemTags(const std::map<std::u8string, ItemType>& tagdefs) {
  // Merge Given Item Tags into Item Defs
  for (auto itag : itags_) {
    if (tagdefs.contains(itag)) {
      items_.emplace_back(tagdefs.at(itag));
    } else {
      loger(u8"ERROR: Use of undefined Item tag: '{}'.  Skipping.\n", itag);
    }
  }
}

Object* Object::AddNPC(std::mt19937& gen, const std::u8string_view& tags) {
  if (!npctagdefs.contains(World())) {
    // Definitions for all built-in NPC tags go here.
  }

  if (!weapontagdefs.contains(World())) {
    // Definitions for all built-in weapon tags go here.
    weapontagdefs[World()] = {};
  }

  if (!armortagdefs.contains(World())) {
    // Definitions for all built-in armor tags go here.
    armortagdefs[World()] = {};
  }

  if (!itemtagdefs.contains(World())) {
    // Definitions for all built-in item tags go here.
  }

  // Merge Given NPC Tags into new NPC Def
  auto npcdef = base_npc;
  auto start = tags.cbegin();
  auto end = std::find(start, tags.cend(), ',');
  while (start != tags.cend()) {
    std::u8string tag(tags.substr(start - tags.cbegin(), end - start));
    if (npctagdefs.at(World()).contains(tag)) {
      npcdef += npctagdefs.at(World()).at(tag);
    } else {
      loger(u8"ERROR: Use of undefined NPC tag: '{}'.  Skipping.\n", tag);
    }
    start = end;
    if (start != tags.cend()) {
      ++start;
      end = std::find(start, tags.cend(), ',');
    }
  }
  npcdef.FinalizeWeaponTags(weapontagdefs.at(World()));
  npcdef.FinalizeArmorTags(armortagdefs.at(World()));
  npcdef.FinalizeItemTags(itemtagdefs.at(World()));

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

  for (auto sk : type->props_) {
    npc->SetSkill(sk.first, sk.second);
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

    int modenum = 1;
    for (auto mode : ar.loc_) {
      for (auto loc : mode) {
        obj->SetSkill(wear_attribs[loc], modenum);
        if (modenum == 1) {
          npc->AddAct(loc, obj);
        }
      }
      modenum *= 2;
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
      for (auto sk : it.props_) {
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

      int modenum = 1;
      for (auto mode : it.loc_) {
        for (auto loc : mode) {
          obj->SetSkill(wear_attribs[loc], modenum);
          if (modenum == 1) {
            npc->AddAct(loc, obj);
          }
        }
        modenum *= 2;
      }
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

  if (wtype_ == 0) {
    wtype_ = in.wtype_;
  } else if (in.wtype_ == 0) {
  } else if (wtype_ == in.wtype_) {
  } else if (two_handed(in.wtype_)) { // Convert Long to Two-Handed
    wtype_ += get_weapon_type(u8"Two-Handed Blades") - get_weapon_type(u8"Long Blades");
  } else if (two_handed(wtype_)) { // Convert Long to Two-Handed
    wtype_ = in.wtype_ + get_weapon_type(u8"Two-Handed Blades") - get_weapon_type(u8"Long Blades");
  } else {
    // logey(u8"Warning: No idea how to combine weapon types {} and {}.\n", wtype_, in.wtype_);
    wtype_ = in.wtype_;
  }
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

NPCType::NPCType(const std::u8string_view& tagdef) {
  std::u8string_view def = tagdef;
  LoadFrom(def);
}

NPCType::NPCType(std::u8string_view& tagdef) {
  LoadFrom(tagdef);
}

bool NPCType::LoadFrom(std::u8string_view& def) {
  skipspace(def);
  while (def.length() > 0 && !def.starts_with(u8"tag:")) {
    auto line = getuntil(def, '\n');
    skipspace(line); // Ignore indentation, blank lines, etc.
    if (line.length() == 0 || line.front() == '#') {
      // Comment or blank link - skip it.
    } else if (process(line, u8"short:")) {
      short_desc_ = std::u8string(getuntil(line, '\n'));
    } else if (process(line, u8"desc:")) {
      desc_ = std::u8string(getuntil(line, '\n'));
    } else if (process(line, u8"long:")) {
      long_desc_ = std::u8string(getuntil(line, '\n'));
    } else if (process(line, u8"genders:")) {
      while (line.length() > 0) {
        char8_t gen = nextchar(line);
        if (ascii_toupper(gen) == 'N') {
          genders_.push_back(gender_t::NONE);
        } else if (ascii_toupper(gen) == 'F') {
          genders_.push_back(gender_t::FEMALE);
        } else if (ascii_toupper(gen) == 'M') {
          genders_.push_back(gender_t::MALE);
        } else if (ascii_toupper(gen) == 'O') {
          genders_.push_back(gender_t::NEITHER);
        }
      }
    } else if (process(line, u8"prop:")) {
      auto sname = getuntil(line, ':');
      int32_t sval = getnum(line);
      props_.push_back({crc32c(sname), sval});
    } else if (intparam(line, u8"b:", min_.v[0], max_.v[0])) {
    } else if (intparam(line, u8"q:", min_.v[1], max_.v[1])) {
    } else if (intparam(line, u8"s:", min_.v[2], max_.v[2])) {
    } else if (intparam(line, u8"c:", min_.v[3], max_.v[3])) {
    } else if (intparam(line, u8"i:", min_.v[4], max_.v[4])) {
    } else if (intparam(line, u8"w:", min_.v[5], max_.v[5])) {
    } else if (intparam(line, u8"gold:", min_gold_, max_gold_)) {
    } else if (process(line, u8"wtag:")) {
      wtags_.emplace_back(getuntil(line, '\n'));
    } else if (process(line, u8"atag:")) {
      atags_.emplace_back(getuntil(line, '\n'));
    } else if (process(line, u8"itag:")) {
      itags_.emplace_back(getuntil(line, '\n'));
    } else {
      loger(u8"ERROR: bad npc tag file entry: '{}'\n", line);
      return false;
    }
  }
  return true;
}

WeaponType::WeaponType(const std::u8string_view& tagdef) {
  std::u8string_view def = tagdef;
  LoadFrom(def);
}

WeaponType::WeaponType(std::u8string_view& tagdef) {
  LoadFrom(tagdef);
}

bool WeaponType::LoadFrom(std::u8string_view& def) {
  skipspace(def);
  while (def.length() > 0 && !def.starts_with(u8"tag:")) {
    auto line = getuntil(def, '\n');
    skipspace(line); // Ignore indentation, blank lines, etc.
    if (line.length() == 0 || line.front() == '#') {
      // Comment or blank link - skip it.
    } else if (process(line, u8"short:")) {
      short_desc_ = std::u8string(getuntil(line, '\n'));
    } else if (process(line, u8"desc:")) {
      desc_ = std::u8string(getuntil(line, '\n'));
    } else if (process(line, u8"long:")) {
      long_desc_ = std::u8string(getuntil(line, '\n'));
    } else if (process(line, u8"skill:")) {
      wtype_ = get_weapon_type(getuntil(line, '\n'));
    } else if (process(line, u8"prop:")) {
      auto sname = getuntil(line, ':');
      int32_t sval = getnum(line);
      props_.push_back({crc32c(sname), sval});
    } else if (intparam(line, u8"weight:", min_.weight, max_.weight)) {
    } else if (intparam(line, u8"size:", min_.size, max_.size)) {
    } else if (intparam(line, u8"volume:", min_.volume, max_.volume)) {
    } else if (intparam(line, u8"value:", min_.value, max_.value)) {
    } else if (intparam(line, u8"reach:", wmin_.reach, wmax_.reach)) {
    } else if (intparam(line, u8"force:", wmin_.force, wmax_.force)) {
    } else if (intparam(line, u8"severity:", wmin_.severity, wmax_.severity)) {
    } else {
      loger(u8"ERROR: bad weapon tag file entry: '{}'\n", line);
      return false;
    }
  }
  return true;
}

ArmorType::ArmorType(const std::u8string_view& tagdef) {
  std::u8string_view def = tagdef;
  LoadFrom(def);
}

ArmorType::ArmorType(std::u8string_view& tagdef) {
  LoadFrom(tagdef);
}

bool ArmorType::LoadFrom(std::u8string_view& def) {
  skipspace(def);
  while (def.length() > 0 && !def.starts_with(u8"tag:")) {
    auto line = getuntil(def, '\n');
    skipspace(line); // Ignore indentation, blank lines, etc.
    if (line.length() == 0 || line.front() == '#') {
      // Comment or blank link - skip it.
    } else if (process(line, u8"short:")) {
      short_desc_ = std::u8string(getuntil(line, '\n'));
    } else if (process(line, u8"desc:")) {
      desc_ = std::u8string(getuntil(line, '\n'));
    } else if (process(line, u8"long:")) {
      long_desc_ = std::u8string(getuntil(line, '\n'));
    } else if (process(line, u8"prop:")) {
      auto sname = getuntil(line, ':');
      int32_t sval = getnum(line);
      props_.push_back({crc32c(sname), sval});
    } else if (wearparam(line, loc_)) {
    } else if (intparam(line, u8"weight:", min_.weight, max_.weight)) {
    } else if (intparam(line, u8"size:", min_.size, max_.size)) {
    } else if (intparam(line, u8"volume:", min_.volume, max_.volume)) {
    } else if (intparam(line, u8"value:", min_.value, max_.value)) {
    } else if (intparam(line, u8"bulk:", amin_.bulk, amax_.bulk)) {
    } else if (intparam(line, u8"impact:", amin_.impact, amax_.impact)) {
    } else if (intparam(line, u8"thread:", amin_.thread, amax_.thread)) {
    } else if (intparam(line, u8"planar:", amin_.planar, amax_.planar)) {
    } else {
      loger(u8"ERROR: bad armor tag file entry: '{}'\n", line);
      return false;
    }
  }
  return true;
}

ItemType::ItemType(const std::u8string_view& tagdef) {
  std::u8string_view def = tagdef;
  LoadFrom(def);
}

ItemType::ItemType(std::u8string_view& tagdef) {
  LoadFrom(tagdef);
}

bool ItemType::LoadFrom(std::u8string_view& def) {
  skipspace(def);
  while (def.length() > 0 && !def.starts_with(u8"tag:")) {
    auto line = getuntil(def, '\n');
    skipspace(line); // Ignore indentation, blank lines, etc.
    if (line.length() == 0 || line.front() == '#') {
      // Comment or blank link - skip it.
    } else if (process(line, u8"short:")) {
      short_desc_ = std::u8string(getuntil(line, '\n'));
    } else if (process(line, u8"desc:")) {
      desc_ = std::u8string(getuntil(line, '\n'));
    } else if (process(line, u8"long:")) {
      long_desc_ = std::u8string(getuntil(line, '\n'));
    } else if (process(line, u8"prop:")) {
      auto sname = getuntil(line, ':');
      int32_t sval = getnum(line);
      props_.push_back({crc32c(sname), sval});
    } else if (wearparam(line, loc_)) {
    } else if (intparam(line, u8"weight:", min_.weight, max_.weight)) {
    } else if (intparam(line, u8"size:", min_.size, max_.size)) {
    } else if (intparam(line, u8"volume:", min_.volume, max_.volume)) {
    } else if (intparam(line, u8"value:", min_.value, max_.value)) {
    } else {
      loger(u8"ERROR: bad item tag file entry: '{}'\n", line);
      return false;
    }
  }
  return true;
}

bool Object::LoadTagsFrom(const std::u8string_view& tagdefs) const {
  std::u8string_view defs = tagdefs;

  skipspace(defs);
  while (process(defs, u8"tag:")) {
    if (process(defs, u8"npc:")) {
      std::u8string tag(getuntil(defs, '\n'));
      if (!npctagdefs[this].try_emplace(tag, defs).second) {
        loger(u8"Duplicate NPC tag '{}' insertion into {} rejected.\n", tag, ShortDesc());
        return false;
      }
    } else if (process(defs, u8"weapon:")) {
      std::u8string tag(getuntil(defs, '\n'));
      if (!weapontagdefs[this].try_emplace(tag, defs).second) {
        loger(u8"Duplicate Weapon tag '{}' insertion into {} rejected.\n", tag, ShortDesc());
        return false;
      }
    } else if (process(defs, u8"armor:")) {
      std::u8string tag(getuntil(defs, '\n'));
      if (!armortagdefs[this].try_emplace(tag, defs).second) {
        loger(u8"Duplicate Armor tag '{}' insertion into {} rejected.\n", tag, ShortDesc());
        return false;
      }
    } else if (process(defs, u8"item:")) {
      std::u8string tag(getuntil(defs, '\n'));
      if (!itemtagdefs[this].try_emplace(tag, defs).second) {
        loger(u8"Duplicate Item tag '{}' insertion into {} rejected.\n", tag, ShortDesc());
        return false;
      }
    } else {
      loger(u8"Bad tag type '{}'.\n", getuntil(defs, '\n'));
      return false;
    }
    skipspace(defs);
  }

  if (defs.length() > 0) {
    loger(u8"Bad content at end of .tags file: '{}'.\n", getuntil(defs, '\n'));
    return false;
  }

  return true;
}
