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

#include "cchar8.hpp"
#include "color.hpp"
#include "log.hpp"
#include "mind.hpp"
#include "object.hpp"
#include "properties.hpp"
#include "utils.hpp"
#include "version.hpp"

const std::u8string act_save[static_cast<uint8_t>(act_t::SPECIAL_MAX)] = {
    u8"NONE",           u8"DEAD",         u8"DYING",        u8"UNCONSCIOUS",     u8"SLEEP",
    u8"REST",           u8"WORK",         u8"HEAL",         u8"POINT",           u8"FOLLOW",
    u8"FIGHT",          u8"OFFER",        u8"HOLD",         u8"WIELD",           u8"WEAR_BACK",
    u8"WEAR_CHEST",     u8"WEAR_HEAD",    u8"WEAR_NECK",    u8"WEAR_COLLAR",     u8"WEAR_WAIST",
    u8"WEAR_SHIELD",    u8"WEAR_LARM",    u8"WEAR_RARM",    u8"WEAR_LFINGER",    u8"WEAR_RFINGER",
    u8"WEAR_LFOOT",     u8"WEAR_RFOOT",   u8"WEAR_LHAND",   u8"WEAR_RHAND",      u8"WEAR_LLEG",
    u8"WEAR_RLEG",      u8"WEAR_LWRIST",  u8"WEAR_RWRIST",  u8"WEAR_LSHOULDER",  u8"WEAR_RSHOULDER",
    u8"WEAR_LHIP",      u8"WEAR_RHIP",    u8"WEAR_FACE",    u8"SPECIAL_MONITOR", u8"SPECIAL_MASTER",
    u8"SPECIAL_LINKED", u8"SPECIAL_HOME", u8"SPECIAL_WORK", u8"SPECIAL_ACTEE",
    //	"SPECIAL_MAX"
};

static std::map<std::u8string, act_t> act_load_map;
static act_t act_load(const std::u8string& str) {
  if (act_load_map.size() < 1) {
    for (act_t a = act_t::NONE; a != act_t::SPECIAL_MAX; ++a) {
      act_load_map[act_save[static_cast<uint8_t>(a)]] = a;
    }
  }
  if (act_load_map.count(str) < 1)
    return act_t::NONE;
  return act_load_map[str];
}

static char8_t buf[65536];
static std::vector<Object*> todo;

static std::map<int, Object*> num2obj;
Object* getbynum(int num) {
  if (num2obj.count(num) < 1)
    num2obj[num] = new Object();
  return num2obj[num];
}

static int last_object_number = 0;
static std::map<Object*, int> obj2num;
int getnum(Object* obj) {
  if (!obj2num.count(obj))
    obj2num[obj] = ++last_object_number;
  return obj2num[obj];
}

int Object::Save(const std::u8string& fn) {
  FILE* fl = fopen(fn.c_str(), u8"w");
  if (!fl)
    return -1;

  fprintf(fl, u8"%.8X\n", CurrentVersion.savefile_version_object);

  // Save 'prop_names' hashes and strings.
  save_prop_names_to(fl);

  obj2num.clear();
  obj2num[nullptr] = 0;
  last_object_number = 0;
  if (SaveTo(fl)) {
    fclose(fl);
    return -1;
  }

  fclose(fl);
  return 0;
}

int Object::SaveTo(FILE* fl) {
  // loge(u8"Saving {}\n", Name());

  fprintf(fl, u8"%d\n", getnum(this));
  fprintf(fl, u8"%s%c\n", ShortDescC(), 0);
  fprintf(fl, u8"%s%c\n", NameC(), 0);
  if (HasDesc()) {
    fprintf(fl, u8"%s%c\n", DescC(), 0);
  } else {
    fprintf(fl, u8"%c\n", 0);
  }
  if (HasLongDesc()) {
    fprintf(fl, u8"%s%c\n", LongDescC(), 0);
  } else {
    fprintf(fl, u8"%c\n", 0);
  }

  fprintf(fl, u8"%d %d %d %d %c\n", weight, size, volume, value, gender);

  for (auto ind : known) {
    fprintf(fl, u8";%ld", ind);
  }
  fprintf(fl, u8":\n");

  for (auto ind : completed) {
    fprintf(fl, u8";%ld", ind);
  }
  fprintf(fl, u8":\n");

  fprintf(fl, u8" %d\n", sexp);

  fprintf(
      fl,
      u8"%hhd,%hhd,%hhd,%hhd,%hhd,%hhd,%hhd,%hhd,%hhd;%d",
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
    fprintf(fl, u8"|%.8X|%d", sk.first, sk.second);
  if (cur_skill != prhash(u8"None")) { // Added current skill to end in v0x13
    fprintf(fl, u8"|%.8X", cur_skill);
  }
  fprintf(fl, u8";\n");

  fprintf(fl, u8"%d\n", (int)(contents.size()));
  for (auto cind : contents) {
    fprintf(fl, u8"%d\n", getnum(cind));
  }

  uint8_t num8 = static_cast<uint8_t>(pos);
  fprintf(fl, u8"%hhu\n", num8);

  fprintf(fl, u8"%d\n", (int)(act.size()));
  for (auto aind : act) {
    fprintf(
        fl, u8"%s;%d\n", act_save[static_cast<uint8_t>(aind.act())].c_str(), getnum(aind.obj()));
  }

  fprintf(fl, u8"\n");

  for (auto cind : contents) {
    cind->SaveTo(fl);
  }

  // loge(u8"Saved {}\n", Name());
  return 0;
}

static unsigned int ver;
int Object::Load(const std::u8string& fn) {
  FILE* fl = fopen(fn.c_str(), u8"r");
  if (!fl)
    return -1;

  fscanf(fl, u8"%X\n", &ver);

  // Load 'prop_names' hashes and strings.
  load_prop_names_from(fl);

  todo.clear();
  num2obj.clear();
  num2obj[0] = nullptr;
  num2obj[1] = this;
  if (LoadFrom(fl)) {
    fclose(fl);
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
          ind->BusyFor(500, ind->Tactics().c_str());
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

  fclose(fl);
  return 0;
}

int Object::LoadFrom(FILE* fl) {
  // static std::u8string debug_indent = u8"";

  int num, res;
  fscanf(fl, u8"%d ", &num);
  if (num2obj[num] != this) {
    loger(u8"Error: Acid number mismatch ({})!\n", num);
  }
  todo.push_back(this);

  memset(buf, 0, 65536);
  num = 0;
  res = getc(fl);
  while (res > 0) {
    buf[num++] = res;
    res = getc(fl);
  }
  std::u8string sd = buf;

  memset(buf, 0, 65536);
  if (ver >= 0x001B) {
    num = 0;
    res = getc(fl);
    while (res > 0) {
      buf[num++] = res;
      res = getc(fl);
    }
  }
  std::u8string n = buf;

  memset(buf, 0, 65536);
  num = 0;
  res = getc(fl);
  while (res > 0) {
    buf[num++] = res;
    res = getc(fl);
  }
  std::u8string d = buf;

  memset(buf, 0, 65536);
  num = 0;
  res = getc(fl);
  while (res > 0) {
    buf[num++] = res;
    res = getc(fl);
  }
  std::u8string ld = buf;

  SetDescs(sd, n, d, ld);

  // loge(u8"{}Loading {}:{}\n", debug_indent, num, buf);

  fscanf(fl, u8"%d %d %d %d %c", &weight, &size, &volume, &value, &gender);

  if (ver < 0x001A) {
    fscanf(fl, u8"%*d"); // Experience (Redundant)
    fscanf(fl, u8";"); // Was present pre v0x15, causes no problems until v0x1A.
    fscanf(fl, u8"\n"); // Skip the white-space, if ';' was used or not.
  } else {
    fscanf(fl, u8"\n"); // Skip the white-space before the line of u8"known".
    unsigned long know;
    while (fscanf(fl, u8";%ld", &know)) {
      known.push_back(know);
    }
    fscanf(fl, u8":\n"); // Skip the ending colon and white-space after the line of u8"known".
  }

  fscanf(fl, u8"\n"); // Skip the white-space before the line of u8"completed".
  unsigned long accom;
  while (fscanf(fl, u8";%ld", &accom)) {
    completed.push_back(accom);
  }
  if (ver >= 0x001A) {
    fscanf(fl, u8":\n"); // Skip the ending colon and white-space after the line of u8"completed".
  }

  fscanf(fl, u8" %d\n", &sexp);

  if (ver < 0x0019) {
    int16_t mods[6];
    fscanf(
        fl,
        u8"%*d,%hhd,%hd,%*d,%hhd,%hd,%*d,%hhd,%hd,%*d,%hhd,%hd,%*d,%hhd,%hd,%*d,%hhd,%hd,%hhd,%hhd,%hhd",
        &attr[0],
        &mods[0],
        &attr[1],
        &mods[1],
        &attr[2],
        &mods[2],
        &attr[3],
        &mods[3],
        &attr[4],
        &mods[4],
        &attr[5],
        &mods[5],
        &phys,
        &stun,
        &stru);
    for (int a = 0; a < 6; ++a) {
      if (mods[a] != 0) {
        SetModifier(a, mods[a]);
      }
    }
  } else {
    fscanf(
        fl,
        u8"%hhd,%hhd,%hhd,%hhd,%hhd,%hhd,%hhd,%hhd,%hhd",
        &attr[0],
        &attr[1],
        &attr[2],
        &attr[3],
        &attr[4],
        &attr[5],
        &phys,
        &stun,
        &stru);
  }

  int do_tick;
  if (fscanf(fl, u8";%d", &do_tick)) {
    if (do_tick)
      Activate();
  }

  fscanf(fl, u8"\n");

  memset(buf, 0, 65536);
  int val;
  int ret;
  uint32_t stok;
  ret = fscanf(fl, u8"|%X|%d", &stok, &val);
  while (ret > 1) {
    // loge(u8"Loaded {}: {}\n", buf, val);
    if (ver < 0x0019) { // Update to new hash algo
      auto name = SkillName(stok);
      stok = crc32c(name);
      insert_skill_hash(stok, name);
    }
    SetSkill(stok, val);
    ret = fscanf(fl, u8"|%X|%d", &stok, &val);
  }
  if (ret > 0) { // Added the currently used skill to the end in v0x13
    if (ver < 0x0019) { // Update to new hash algo
      auto name = SkillName(stok);
      stok = crc32c(name);
      insert_skill_hash(stok, name);
    }
    StartUsing(stok);
  }
  fscanf(fl, u8";\n");

  std::vector<Object*> toload;
  fscanf(fl, u8"%d ", &num);
  contents.reserve(num);
  for (int ctr = 0; ctr < num; ++ctr) {
    int num2;
    fscanf(fl, u8"%d ", &num2);
    Object* obj = getbynum(num2);
    obj->SetParent(this);
    toload.push_back(obj);
    AddLink(obj);
  }

  uint8_t num8;
  fscanf(fl, u8"%hhu\n", &num8);
  pos = static_cast<pos_t>(num8);

  fscanf(fl, u8"%d ", &num);
  for (int ctr = 0; ctr < num; ++ctr) {
    int num2;
    act_t a;
    memset(buf, 0, 65536);
    fscanf(fl, u8"%65535[^;];%d ", buf, &num2);
    a = act_load(std::u8string(buf));
    if (a != act_t::SPECIAL_ACTEE) {
      AddAct(a, getbynum(num2));
    }
  }

  if (IsAct(act_t::SPECIAL_WORK)) {
    Attach(new Mind(mind_t::NPC));
  } else if (Skill(prhash(u8"Personality"))) {
    Attach(new Mind(mind_t::MOB));
  } else if (Skill(prhash(u8"TBAAction"))) {
    Attach(new Mind(mind_t::TBAMOB));
  }

  if (Skill(prhash(u8"TBAScriptType")) & 2) { // Random/Permanent Triggers
    Mind* trig = new_mind(mind_t::TBATRIG, this);
    trig->Suspend((rand() % 13000) + 3000); // 3-16 Seconds
  }

  //  int num_loaded = 0;
  //  if(parent && (!(parent->parent))) {
  //    loge(u8"Loading: {}\n", ShortDescC());
  //    }
  for (auto cind : toload) {
    // loge(u8"{}Calling loader from {}\n", debug_indent.c_str(),
    // ShortDescC());
    // std::u8string tmp = debug_indent;
    // debug_indent += u8"  ";
    cind->LoadFrom(fl);
    // debug_indent = tmp;
    // loge(u8"{}Called loader from {}\n", debug_indent.c_str(),
    // ShortDescC());

    //    if(parent && (!(parent->parent))) {
    //      loge(u8"\rLoaded: {}/{} ({})    ",
    //	++num_loaded, int(toload.size()), cind->ShortDescC()
    //	);
    //      }
  }
  //  if(parent && (!(parent->parent))) {
  //    loge(u8"\nLoaded.\n");
  //    }

  // loge(u8"{}Loaded {}\n", debug_indent.c_str(), ShortDescC());

  //  if(HasSkill(prhash(u8"Drink"))) {
  //    SetSkill(prhash(u8"Drink"), Skill(prhash(u8"Drink")) * 15);
  //    SetSkill(prhash(u8"Food"), Skill(prhash(u8"Food")) * 15);
  //    }
  //  else if(HasSkill(prhash(u8"Food"))) {
  //    SetSkill(prhash(u8"Food"), Skill(prhash(u8"Food")) * 60);
  //    }

  //  if(HasSkill(prhash(u8"Heal Effect"))) {
  //    SetSkill(prhash(u8"Heal Spell"), Skill(prhash(u8"Heal Effect")));
  //    SetSkill(prhash(u8"Heal Effect"), 0);
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
  //    SetSkill(prhash(u8"Evasion Penalty"), 0);
  //    }
  //  else if(HasSkill(prhash(u8"Evasion Bonus"))) {
  //    SetSkill(prhash(u8"Evasion Penalty"), Skill(prhash(u8"Evasion Bonus")));
  //    SetSkill(prhash(u8"Evasion Bonus"), 0);
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
