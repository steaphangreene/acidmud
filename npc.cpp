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
#include "tags.hpp"
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

void Object::GenerateNPC(const NPCTag& type, std::mt19937& gen) {
  if (World()) {
    auto obj_id = World()->Skill(prhash(u8"Last Object ID")) + 1;
    World()->SetSkill(prhash(u8"Last Object ID"), obj_id);
    SetSkill(prhash(u8"Object ID"), obj_id);
  }

  Attach(std::make_shared<Mind>(mind_t::NPC));
  Activate();
  SetPos(pos_t::STAND);
  for (int a : {0, 1, 2, 3, 4, 5}) {
    SetAttribute(a, rint3(gen, type.min_.v[a], type.max_.v[a]));
  }

  for (auto sk : type.props_) {
    SetSkill(sk.first, sk.second);
  }

  if (type.genders_.size() > 0) {
    SetGender(type.genders_[rint1(gen, 0, type.genders_.size() - 1)]);
  }

  SetShortDesc(gender_proc(type.short_desc_, Gender()));
  SetDesc(gender_proc(type.desc_, Gender()));
  SetLongDesc(gender_proc(type.long_desc_, Gender()));

  int gidx = (Gender() == gender_t::FEMALE) ? 0 : 1;
  std::vector<std::u8string> first = {u8""};
  std::sample(
      dwarf_first_names[gidx].begin(), dwarf_first_names[gidx].end(), first.begin(), 1, gen);
  std::vector<std::u8string> last = {u8""};
  std::sample(dwarf_last_names.begin(), dwarf_last_names.end(), last.begin(), 1, gen);
  SetName(first.front() + u8" " + last.front());

  if (type.min_gold_ > 0 || type.max_gold_ > 0) {
    int num_gold = rint3(gen, type.min_gold_, type.max_gold_);
    if (num_gold > 0) {
      give_gold(this, num_gold);
    }
  }

  if (true) { // TODO: Figure out who should get these, and who should not
    add_pouch(this);
  }

  for (auto wp : type.weapons_) {
    Object* obj = new Object(this);
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
    AddAct(act_t::WIELD, obj);
    if (two_handed(wp.wtype_)) {
      AddAct(act_t::HOLD, obj);
    }
  }

  for (auto ar : type.armor_) {
    Object* obj = new Object(this);
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
          AddAct(loc, obj);
        }
      }
      modenum *= 2;
    }
  }

  if (type.items_.size() > 0) {
    Object* sack = new Object(this);
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
    AddAct(act_t::WEAR_RHIP, sack);

    for (auto it : type.items_) {
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
            AddAct(loc, obj);
          }
        }
        modenum *= 2;
      }
    }
  }
}
