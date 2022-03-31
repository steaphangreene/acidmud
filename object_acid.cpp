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

#include "color.hpp"
#include "infile.hpp"
#include "log.hpp"
#include "mind.hpp"
#include "object.hpp"
#include "outfile.hpp"
#include "properties.hpp"
#include "utils.hpp"
#include "version.hpp"

const struct {
  char8_t c;
  gender_t g;
} gender_save[4] = {
    {'N', gender_t::NONE},
    {'M', gender_t::MALE},
    {'F', gender_t::FEMALE},
    {'O', gender_t::NEITHER}};

const std::u8string act_save[] = {
    u8"NONE",         u8"DEAD",         u8"DYING",         u8"UNCONSCIOUS",     u8"SLEEP",
    u8"REST",         u8"WORK",         u8"HEAL",          u8"POINT",           u8"FOLLOW",
    u8"FIGHT",        u8"OFFER",        u8"HOLD",          u8"WIELD",           u8"WEAR_BACK",
    u8"WEAR_CHEST",   u8"WEAR_HEAD",    u8"WEAR_NECK",     u8"WEAR_COLLAR",     u8"WEAR_WAIST",
    u8"WEAR_SHIELD",  u8"WEAR_LARM",    u8"WEAR_RARM",     u8"WEAR_LFINGER",    u8"WEAR_RFINGER",
    u8"WEAR_LFOOT",   u8"WEAR_RFOOT",   u8"WEAR_LHAND",    u8"WEAR_RHAND",      u8"WEAR_LLEG",
    u8"WEAR_RLEG",    u8"WEAR_LWRIST",  u8"WEAR_RWRIST",   u8"WEAR_LSHOULDER",  u8"WEAR_RSHOULDER",
    u8"WEAR_LHIP",    u8"WEAR_RHIP",    u8"WEAR_FACE",     u8"SPECIAL_MONITOR", u8"SPECIAL_LINKED",
    u8"SPECIAL_HOME", u8"SPECIAL_WORK", u8"SPECIAL_OWNER", u8"SPECIAL_ACTEE",
    //	"SPECIAL_MAX"
};
// Check if there are too few/many items (forgot to add/remove one here?) in the above list.
static_assert(std::size(act_save) == static_cast<uint8_t>(act_t::SPECIAL_MAX));

static std::map<std::u8string_view, act_t> act_load_map;
static act_t act_load(const std::u8string_view& str) {
  if (act_load_map.size() < 1) {
    for (act_t a = act_t::NONE; a != act_t::SPECIAL_MAX; ++a) {
      act_load_map[act_save[static_cast<uint8_t>(a)]] = a;
    }
  }
  if (act_load_map.count(str) < 1)
    return act_t::NONE;
  return act_load_map[str];
}

static std::vector<Object*> todo;

static std::map<int, Object*> num2obj;
Object* getbynum(int num) {
  if (num2obj.count(num) < 1)
    num2obj[num] = new Object();
  return num2obj[num];
}

static int last_object_number = 0;
static std::map<Object*, int> obj2num;
int getonum(Object* obj) {
  if (!obj2num.count(obj))
    obj2num[obj] = ++last_object_number;
  return obj2num[obj];
}

int Object::Save(const std::u8string& fn) {
  outfile fl(fn);
  if (!fl)
    return -1;

  fl.append(u8"{:08X}\n", CurrentVersion.savefile_version_object);

  // Save 'prop_names' hashes and strings.
  save_prop_names_to(fl);

  obj2num.clear();
  obj2num[nullptr] = 0;
  last_object_number = 0;
  if (SaveTo(fl)) {
    return -1;
  }
  return 0;
}

int Object::SaveTo(outfile& fl) {
  // loge(u8"Saving {}\n", Name());

  fl.append(u8"{}\n", getonum(this));
  fl.append(u8"{}{:c}\n", ShortDesc(), 0);
  fl.append(u8"{}{:c}\n", Name(), 0);
  if (HasDesc()) {
    fl.append(u8"{}{:c}\n", Desc(), 0);
  } else {
    fl.append(u8"{:c}\n", 0);
  }
  if (HasLongDesc()) {
    fl.append(u8"{}{:c}\n", LongDesc(), 0);
  } else {
    fl.append(u8"{:c}\n", 0);
  }

  char8_t gen_char = 'N';
  for (auto g : gender_save) {
    if (gender == g.g) {
      gen_char = g.c;
    }
  }
  fl.append(u8"{} {} {} {} {:c}\n", weight, size, volume, value, gen_char);

  for (auto ind : known) {
    fl.append(u8";{}", ind);
  }
  fl.append(u8":\n");

  for (auto ind : completed) {
    fl.append(u8";{}", ind);
  }
  fl.append(u8":\n");

  fl.append(u8" {}\n", sexp);

  fl.append(
      u8"{},{},{},{},{},{},{},{},{};{}",
      attr[0],
      attr[1],
      attr[2],
      attr[3],
      attr[4],
      attr[5],
      phys,
      stun,
      stru,
      IsActive());

  for (const auto& sk : skills)
    fl.append(u8"|{:08X}|{}", sk.first, sk.second);
  if (cur_skill != prhash(u8"None")) { // Added current skill to end in v0x13
    fl.append(u8"|{:08X}", cur_skill);
  }
  fl.append(u8";\n");

  fl.append(u8"{}\n", quantity);

  fl.append(u8"{}\n", (int)(contents.size()));
  for (auto cind : contents) {
    fl.append(u8"{}\n", getonum(cind));
  }

  uint8_t num8 = static_cast<uint8_t>(pos);
  fl.append(u8"{}\n", num8);

  fl.append(u8"{}\n", (int)(act.size()));
  for (auto aind : act) {
    fl.append(u8"{};{}\n", act_save[static_cast<uint8_t>(aind.act())], getonum(aind.obj()));
  }

  fl.append(u8"\n");

  for (auto cind : contents) {
    cind->SaveTo(fl);
  }

  // loge(u8"Saved {}\n", Name());
  return 0;
}

static unsigned int ver;
int Object::Load(const std::u8string& fn) {
  infile fl(fn);
  if (!fl)
    return -1;

  ver = nexthex(fl);
  skipspace(fl);

  // Load 'prop_names' hashes and strings.
  load_prop_names_from(fl);

  todo.clear();
  num2obj.clear();
  num2obj[0] = nullptr;
  num2obj[1] = this;
  if (LoadFrom(fl)) {
    return -1;
  }

  // Purge old skill name hashes
  if (ver < 0x0019) {
    purge_invalid_prop_names();
  }

  for (auto ind : todo) {
    std::vector<act_t> killacts;
    for (auto aind : ind->act) {
      if (aind.act() != act_t::SPECIAL_ACTEE) {
        if (aind.obj()) {
          aind.obj()->NowTouching(ind);
        } else if (aind.act() <= act_t::REST) { // Targetless Actions
          aind.set_obj(nullptr);
        } else { // Act Targ No Longer Exists (u8"junkrestart", I hope)!
          killacts.push_back(aind.act());
        }
        if (aind.act() == act_t::FIGHT) {
          ind->BusyFor(500, ind->Tactics());
        }
      }
    }
    for (auto kill : killacts) { // Kill Actions on Non-Existent
      StopAct(kill);
    }
    if (ind->IsUsing(prhash(u8"Lumberjack"))) { // FIXME: All long-term skills?
      ind->BusyFor(500, u8"use Lumberjack");
    }
  }
  todo.clear();
  return 0;
}

int Object::LoadFrom(std::u8string_view& fl) {
  // static std::u8string debug_indent = u8"";

  int num = nextnum(fl);
  skipspace(fl);
  if (num2obj[num] != this) {
    loger(u8"Error: Acid number mismatch ({})!\n", num);
  }
  todo.push_back(this);

  std::u8string_view sd = getuntil(fl, 0);
  skipspace(fl);
  std::u8string_view n = u8"";
  if (ver >= 0x001B) {
    n = getuntil(fl, 0);
    skipspace(fl);
  }
  std::u8string_view d = getuntil(fl, 0);
  skipspace(fl);
  std::u8string_view ld = getuntil(fl, 0);
  skipspace(fl);

  SetDescs(sd, n, d, ld);

  // loge(u8"{}Loading {}:{}\n", debug_indent, num, buf);

  weight = nextnum(fl);
  skipspace(fl);
  size = nextnum(fl);
  skipspace(fl);
  volume = nextnum(fl);
  skipspace(fl);
  value = nextnum(fl);
  skipspace(fl);

  gender = gender_t::NONE;
  char8_t gen_char = nextchar(fl);
  skipspace(fl);
  for (auto g : gender_save) {
    if (gen_char == g.c) {
      gender = g.g;
    }
  }

  while (nextchar(fl) == ';') {
    known.push_back(nextnum(fl));
  }
  skipspace(fl);

  while (nextchar(fl) == ';') {
    completed.push_back(nextnum(fl));
  }
  skipspace(fl);

  sexp = nextnum(fl);
  skipspace(fl);

  attr[0] = nextnum(fl);
  nextchar(fl); // Comma
  attr[1] = nextnum(fl);
  nextchar(fl); // Comma
  attr[2] = nextnum(fl);
  nextchar(fl); // Comma
  attr[3] = nextnum(fl);
  nextchar(fl); // Comma
  attr[4] = nextnum(fl);
  nextchar(fl); // Comma
  attr[5] = nextnum(fl);
  nextchar(fl); // Comma
  phys = nextnum(fl);
  nextchar(fl); // Comma
  stun = nextnum(fl);
  nextchar(fl); // Comma
  stru = nextnum(fl);

  char8_t delim = nextchar(fl);
  if (delim == ';') {
    int do_tick = nextnum(fl);
    if (do_tick)
      Activate();
    delim = nextchar(fl);
  }

  while (delim == '|') {
    uint32_t stok = nexthex(fl);
    delim = nextchar(fl); // The '|' Separator

    if (ver < 0x0019) { // Update to new hash algo
      auto name = SkillName(stok);
      stok = crc32c(name);
      insert_skill_hash(stok, name);
    }

    if (delim == '|') {
      int val = nextnum(fl);
      delim = nextchar(fl);
      if (stok == prhash(u8"Quantity")) { // Before v0x001C Quantity was saved as a Skill
        quantity = val;
      } else {
        SetSkill(stok, val);
      }
    } else { // Only the skill name, so "currently used skill"
      StartUsing(stok);
    }
  }
  if (delim != ';') {
    logerr(u8"ERROR[{}]: Object skill list parse error (delim='{:c}').\n", ShortDesc(), delim);
  }
  skipspace(fl);

  if (ver >= 0x001C) {
    quantity = nextnum(fl);
    skipspace(fl);
  }

  num = nextnum(fl);
  skipspace(fl);

  std::vector<Object*> toload;
  contents.reserve(num);
  for (int ctr = 0; ctr < num; ++ctr) {
    Object* obj = getbynum(nextnum(fl));
    obj->SetParent(this);
    toload.push_back(obj);
    AddLink(obj);
    skipspace(fl);
  }

  pos = static_cast<pos_t>(nextnum(fl));
  skipspace(fl);

  num = nextnum(fl);
  skipspace(fl);
  for (int ctr = 0; ctr < num; ++ctr) {
    auto acts = getuntil(fl, ';');
    int onum = nextnum(fl);
    skipspace(fl);
    act_t a = act_load(acts);
    if (a != act_t::SPECIAL_ACTEE && a != act_t::NONE) {
      AddAct(a, getbynum(onum));
    }
  }

  if (IsAct(act_t::SPECIAL_WORK)) {
    Attach(std::make_shared<Mind>(mind_t::NPC));
  } else if (Skill(prhash(u8"Personality"))) {
    Attach(get_mob_mind());
  } else if (Skill(prhash(u8"TBAAction"))) {
    Attach(get_tbamob_mind());
  }

  if (Skill(prhash(u8"TBAScriptType")) & 2) { // Random/Permanent Triggers
    std::shared_ptr<Mind> trig = new_mind(mind_t::TBATRIG, this);
    trig->Suspend((rand() % 13000) + 3000); // 3-16 Seconds
  }

  //  int num_loaded = 0;
  //  if(parent && (!(parent->parent))) {
  //    loge(u8"Loading: {}\n", ShortDesc());
  //    }
  for (auto cind : toload) {
    // loge(u8"{}Calling loader from {}\n", debug_indent,
    // ShortDesc());
    // std::u8string tmp = debug_indent;
    // debug_indent += u8"  ";
    cind->LoadFrom(fl);
    // debug_indent = tmp;
    // loge(u8"{}Called loader from {}\n", debug_indent,
    // ShortDesc());

    //    if(parent && (!(parent->parent))) {
    //      loge(u8"\rLoaded: {}/{} ({})    ",
    //	++num_loaded, int(toload.size()), cind->ShortDesc()
    //	);
    //      }
  }
  //  if(parent && (!(parent->parent))) {
  //    loge(u8"\nLoaded.\n");
  //    }

  // loge(u8"{}Loaded {}\n", debug_indent, ShortDesc());

  //  if(HasSkill(prhash(u8"Drink"))) {
  //    SetSkill(prhash(u8"Drink"), Skill(prhash(u8"Drink")) * 15);
  //    SetSkill(prhash(u8"Food"), Skill(prhash(u8"Food")) * 15);
  //    }
  //  else if(HasSkill(prhash(u8"Food"))) {
  //    SetSkill(prhash(u8"Food"), Skill(prhash(u8"Food")) * 60);
  //    }

  //  if(HasSkill(prhash(u8"Heal Effect"))) {
  //    SetSkill(prhash(u8"Heal Spell"), Skill(prhash(u8"Heal Effect")));
  //    ClearSkill(prhash(u8"Heal Effect"));
  //    }

  //  if(HasSkill(prhash(u8"Open"))) {
  //    SetSkill(prhash(u8"Open"), 1000);
  //    }

  //  if(HasSkill(prhash(u8"Day Time"))) {
  //    Activate();
  //    }

  //  if(IsAct(act_t::SPECIAL_NOTSHOWN)) {
  //    SetSkill(prhash(u8"Invisible"), 1000);
  //    }

  //  if(short_desc == u8"A passage exit.") {
  //    short_desc = u8"a passage exit";
  //    }

  //  if(HasSkill(prhash(u8"Evasion Penalty"))) {
  //    SetSkill(prhash(u8"Evasion Bonus"), Skill(prhash(u8"Evasion Penalty")));
  //    ClearSkill(prhash(u8"Evasion Penalty"));
  //    }
  //  else if(HasSkill(prhash(u8"Evasion Bonus"))) {
  //    SetSkill(prhash(u8"Evasion Penalty"), Skill(prhash(u8"Evasion Bonus")));
  //    ClearSkill(prhash(u8"Evasion Bonus"));
  //    }

  //  if(IsAct(act_t::SPECIAL_PREPARE)) {
  //    loge(u8"Found one!\n");
  //    StopAct(act_t::SPECIAL_PREPARE);
  //    }

  //  if(short_desc == u8"a gold piece") {
  //    SetSkill(prhash(u8"Money"), 1);
  //    }

  return 0;
}
