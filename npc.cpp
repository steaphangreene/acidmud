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
#include "dice.hpp"
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
  int ctr = 0, gen = std::to_underlying(gender) + 1;

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

static std::vector<Object*> coin_objs;
static void init_coins() {
  coin_objs.push_back(new Object(Object::TrashBin()));
  coin_objs.back()->SetShortDesc(u8"a platinum piece");
  coin_objs.back()->SetDesc(u8"A standard one-ounce platinum piece.");
  coin_objs.back()->SetWeight(454 / 16);
  coin_objs.back()->SetVolume(0);
  coin_objs.back()->SetValue(10000);
  coin_objs.back()->SetSize(0);
  coin_objs.back()->SetPosition(pos_t::LIE);
  coin_objs.back()->SetSkill(prhash(u8"Money"), 10000);

  coin_objs.push_back(new Object(Object::TrashBin()));
  coin_objs.back()->SetShortDesc(u8"a gold piece");
  coin_objs.back()->SetDesc(u8"A standard one-ounce gold piece.");
  coin_objs.back()->SetWeight(454 / 16);
  coin_objs.back()->SetVolume(0);
  coin_objs.back()->SetValue(1000);
  coin_objs.back()->SetSize(0);
  coin_objs.back()->SetPosition(pos_t::LIE);
  coin_objs.back()->SetSkill(prhash(u8"Money"), 1000);

  coin_objs.push_back(new Object(Object::TrashBin()));
  coin_objs.back()->SetShortDesc(u8"a silver piece");
  coin_objs.back()->SetDesc(u8"A standard one-ounce silver piece.");
  coin_objs.back()->SetWeight(454 / 16);
  coin_objs.back()->SetVolume(0);
  coin_objs.back()->SetValue(100);
  coin_objs.back()->SetSize(0);
  coin_objs.back()->SetPosition(pos_t::LIE);
  coin_objs.back()->SetSkill(prhash(u8"Money"), 100);

  coin_objs.push_back(new Object(Object::TrashBin()));
  coin_objs.back()->SetShortDesc(u8"a copper piece");
  coin_objs.back()->SetDesc(u8"A standard one-ounce copper piece.");
  coin_objs.back()->SetWeight(454 / 16);
  coin_objs.back()->SetVolume(0);
  coin_objs.back()->SetValue(10);
  coin_objs.back()->SetSize(0);
  coin_objs.back()->SetPosition(pos_t::LIE);
  coin_objs.back()->SetSkill(prhash(u8"Money"), 10);

  coin_objs.push_back(new Object(Object::TrashBin()));
  coin_objs.back()->SetShortDesc(u8"a bone chit");
  coin_objs.back()->SetDesc(u8"A small, ornate, bone chit.");
  coin_objs.back()->SetWeight(454 / 16);
  coin_objs.back()->SetVolume(0);
  coin_objs.back()->SetValue(1);
  coin_objs.back()->SetSize(0);
  coin_objs.back()->SetPosition(pos_t::LIE);
  coin_objs.back()->SetSkill(prhash(u8"Money"), 1);

  coin_objs.push_back(new Object(Object::TrashBin()));
  coin_objs.back()->SetShortDesc(u8"a wooden chit");
  coin_objs.back()->SetDesc(u8"A small, ornate, wooden chit.");
  coin_objs.back()->SetWeight(454 / 16);
  coin_objs.back()->SetVolume(0);
  coin_objs.back()->SetValue(1);
  coin_objs.back()->SetSize(0);
  coin_objs.back()->SetPosition(pos_t::LIE);
  coin_objs.back()->SetSkill(prhash(u8"Money"), 1);
}

static void give_coins(Object* npc, size_t amount) {
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

  bag->SetPosition(pos_t::LIE);
  npc->AddAct(act_t::WEAR_LHIP, bag);

  if (coin_objs.empty()) {
    init_coins();
  }
  for (auto coin : coin_objs) {
    size_t val = coin->Value();
    if (val > 0 && amount >= (2 * val)) {
      Object* g = new Object(*coin);
      g->SetParent(bag);

      size_t num = amount / val;
      if (num * val < amount) {
        --num;
      }
      g->SetQuantity(num);
      amount -= (num * val);
    }
  }
}

static Object* add_pack(Object* npc) {
  Object* bag = new Object(npc);

  bag->SetShortDesc(u8"a small pack");
  bag->SetDesc(u8"A small, durable, practical shoulder pack.");

  bag->SetSkill(prhash(u8"Wearable on Left Shoulder"), 1);
  bag->SetSkill(prhash(u8"Wearable on Right Shoulder"), 2);
  bag->SetSkill(prhash(u8"Container"), 5 * 454);
  bag->SetSkill(prhash(u8"Capacity"), 5);
  bag->SetSkill(prhash(u8"Closeable"), 1);

  bag->SetWeight(1 * 454);
  bag->SetSize(2);
  bag->SetVolume(1);
  bag->SetValue(100);

  bag->SetPosition(pos_t::LIE);
  npc->AddAct(act_t::WEAR_LSHOULDER, bag);

  return bag;
}

void Object::GenerateNPC(const ObjectTag& type) {
  if (World()) {
    auto obj_id = World()->Skill(prhash(u8"Last Object ID")) + 1;
    World()->SetSkill(prhash(u8"Last Object ID"), obj_id);
    SetSkill(prhash(u8"Object ID"), obj_id);
  }

  Attach(std::make_shared<Mind>(mind_t::NPC));
  Activate();
  SetPosition(pos_t::STAND);
  for (int a : {0, 1, 2, 3, 4, 5}) {
    SetAttribute(a, Dice::Rand3(type.min_.v[a], type.max_.v[a]));
  }

  for (auto sk : type.props_) {
    SetSkill(sk.first, sk.second);
  }

  if (type.genders_.size() > 0) {
    gender_t gen;
    Dice::Sample(type.genders_, &gen);
    SetGender(gen);
  }

  SetShortDesc(gender_proc(type.short_desc_, Gender()));
  SetDesc(gender_proc(type.desc_, Gender()));
  SetLongDesc(gender_proc(type.long_desc_, Gender()));

  int gidx = (Gender() == gender_t::FEMALE) ? 0 : 1;
  std::vector<std::u8string> first = {u8""};
  Dice::Sample(dwarf_first_names[gidx], first.begin());
  std::vector<std::u8string> last = {u8""};
  Dice::Sample(dwarf_last_names, last.begin());
  SetName(first.front() + u8" " + last.front());

  if (type.min_gold_ > 0 || type.max_gold_ > 0) {
    int num_gold = Dice::Rand3(type.min_gold_, type.max_gold_);
    if (num_gold > 0) {
      give_coins(this, num_gold);
    }
  }

  for (auto wp : type.weapons_) {
    Object* obj = new Object(this);
    obj->SetSkill(prhash(u8"WeaponType"), wp.wtype_);
    obj->SetSkill(prhash(u8"WeaponReach"), Dice::Rand3(wp.wmin_.reach, wp.wmax_.reach));
    obj->SetSkill(prhash(u8"WeaponForce"), Dice::Rand3(wp.wmin_.force, wp.wmax_.force));
    obj->SetSkill(prhash(u8"WeaponSeverity"), Dice::Rand3(wp.wmin_.severity, wp.wmax_.severity));
    for (auto sk : wp.props_) {
      obj->SetSkill(sk.first, sk.second);
    }
    obj->SetShortDesc(wp.short_desc_);
    obj->SetDesc(wp.desc_);
    obj->SetLongDesc(wp.long_desc_);
    obj->SetWeight(Dice::Rand3(wp.omin_.weight, wp.omax_.weight));
    obj->SetSize(Dice::Rand3(wp.omin_.size, wp.omax_.size));
    obj->SetVolume(Dice::Rand3(wp.omin_.volume, wp.omax_.volume));
    obj->SetValue(Dice::Rand3(wp.omin_.value, wp.omax_.value));
    obj->SetPosition(pos_t::LIE);
    AddAct(act_t::WIELD, obj);
    if (two_handed(wp.wtype_)) {
      AddAct(act_t::HOLD, obj);
      obj->SetSkill(prhash(u8"Wearable on Right Shoulder"), 1);
      obj->SetSkill(prhash(u8"Wearable on Left Shoulder"), 2);
    } else {
      obj->SetSkill(prhash(u8"Wearable on Right Hip"), 1);
      obj->SetSkill(prhash(u8"Wearable on Left Hip"), 2);
    }
    for (auto t : wp.tags_) {
      obj->AddTag(t);
    }
    if (HasSkill(prhash(u8"WeaponSkill"))) { // First Weapon's Skill
      auto val = Skill(prhash(u8"WeaponSkill"));
      ClearSkill(prhash(u8"WeaponSkill"));
      SetSkill(get_weapon_skill(wp.wtype_), val);
    } else if (HasSkill(prhash(u8"Weapon2Skill"))) { // Second Weapon's Skill
      auto val = Skill(prhash(u8"Weapon2Skill"));
      ClearSkill(prhash(u8"Weapon2Skill"));
      SetSkill(get_weapon_skill(wp.wtype_), std::max(val, Skill(get_weapon_skill(wp.wtype_))));
    }
  }

  for (auto ar : type.armor_) {
    Object* obj = new Object(this);
    obj->SetAttribute(0, Dice::Rand3(ar.amin_.bulk, ar.amax_.bulk));
    obj->SetSkill(prhash(u8"ArmorB"), Dice::Rand3(ar.amin_.bulk, ar.amax_.bulk));
    obj->SetSkill(prhash(u8"ArmorI"), Dice::Rand3(ar.amin_.impact, ar.amax_.impact));
    obj->SetSkill(prhash(u8"ArmorT"), Dice::Rand3(ar.amin_.thread, ar.amax_.thread));
    obj->SetSkill(prhash(u8"ArmorP"), Dice::Rand3(ar.amin_.planar, ar.amax_.planar));
    for (auto sk : ar.props_) {
      obj->SetSkill(sk.first, sk.second);
    }
    obj->SetShortDesc(ar.short_desc_);
    obj->SetDesc(ar.desc_);
    obj->SetLongDesc(ar.long_desc_);
    obj->SetWeight(Dice::Rand3(ar.omin_.weight, ar.omax_.weight));
    obj->SetSize(Dice::Rand3(ar.omin_.size, ar.omax_.size));
    obj->SetVolume(Dice::Rand3(ar.omin_.volume, ar.omax_.volume));
    obj->SetValue(Dice::Rand3(ar.omin_.value, ar.omax_.value));
    obj->SetPosition(pos_t::LIE);

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
    for (auto t : ar.tags_) {
      obj->AddTag(t);
    }
  }

  if (type.items_.size() > 0) {
    Object* pack = add_pack(this);

    for (auto it : type.items_) {
      Object* obj = new Object(pack);
      for (auto sk : it.props_) {
        obj->SetSkill(sk.first, sk.second);
      }
      obj->SetShortDesc(it.short_desc_);
      obj->SetDesc(it.desc_);
      obj->SetLongDesc(it.long_desc_);
      obj->SetWeight(Dice::Rand3(it.omin_.weight, it.omax_.weight));
      obj->SetSize(Dice::Rand3(it.omin_.size, it.omax_.size));
      obj->SetVolume(Dice::Rand3(it.omin_.volume, it.omax_.volume));
      obj->SetValue(Dice::Rand3(it.omin_.value, it.omax_.value));
      obj->SetPosition(pos_t::LIE);

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
      for (auto t : it.tags_) {
        obj->AddTag(t);
      }
    }
  }
}
