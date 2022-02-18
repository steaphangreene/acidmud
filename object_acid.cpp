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

#include <cstdlib>
#include <cstring>
#include <map>

#include "color.hpp"
#include "mind.hpp"
#include "object.hpp"
#include "properties.hpp"
#include "utils.hpp"
#include "version.hpp"

const std::string act_save[static_cast<uint8_t>(act_t::SPECIAL_MAX)] = {
    "NONE",         "DEAD",          "DYING",           "UNCONSCIOUS",    "SLEEP",
    "REST",         "HEAL",          "POINT",           "FOLLOW",         "FIGHT",
    "OFFER",        "HOLD",          "WIELD",           "WEAR_BACK",      "WEAR_CHEST",
    "WEAR_HEAD",    "WEAR_NECK",     "WEAR_COLLAR",     "WEAR_WAIST",     "WEAR_SHIELD",
    "WEAR_LARM",    "WEAR_RARM",     "WEAR_LFINGER",    "WEAR_RFINGER",   "WEAR_LFOOT",
    "WEAR_RFOOT",   "WEAR_LHAND",    "WEAR_RHAND",      "WEAR_LLEG",      "WEAR_RLEG",
    "WEAR_LWRIST",  "WEAR_RWRIST",   "WEAR_LSHOULDER",  "WEAR_RSHOULDER", "WEAR_LHIP",
    "WEAR_RHIP",    "WEAR_FACE",     "SPECIAL_MONITOR", "SPECIAL_MASTER", "SPECIAL_LINKED",
    "SPECIAL_HOME", "SPECIAL_ACTEE",
    //	"SPECIAL_MAX"
};

static std::map<std::string, act_t> act_load_map;
static act_t act_load(const std::string& str) {
  if (act_load_map.size() < 1) {
    for (act_t a = act_t::NONE; a != act_t::SPECIAL_MAX; ++a) {
      act_load_map[act_save[static_cast<uint8_t>(a)]] = a;
    }
  }
  if (act_load_map.count(str) < 1)
    return act_t::NONE;
  return act_load_map[str];
}

static char buf[65536];
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

int Object::Save(const std::string& fn) {
  FILE* fl = fopen(fn.c_str(), "w");
  if (!fl)
    return -1;

  fprintf(fl, "%.8X\n", CurrentVersion.savefile_version_object);

  // Save 'skill_names' hashes and strings.
  save_skill_names_to(fl);

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
  // fprintf(stderr, "Saving %s\n", Name());

  fprintf(fl, "%d\n", getnum(this));
  fprintf(fl, "%s%c\n", ShortDescC(), 0);
  fprintf(fl, "%s%c\n", NameC(), 0);
  if (HasDesc()) {
    fprintf(fl, "%s%c\n", DescC(), 0);
  } else {
    fprintf(fl, "%c\n", 0);
  }
  if (HasLongDesc()) {
    fprintf(fl, "%s%c\n", LongDescC(), 0);
  } else {
    fprintf(fl, "%c\n", 0);
  }

  fprintf(fl, "%d %d %d %d %c\n", weight, size, volume, value, gender);

  for (auto ind : known) {
    fprintf(fl, ";%ld", ind);
  }
  fprintf(fl, ":\n");

  for (auto ind : completed) {
    fprintf(fl, ";%ld", ind);
  }
  fprintf(fl, ":\n");

  fprintf(fl, " %d\n", sexp);

  fprintf(
      fl,
      "%hhd,%hhd,%hhd,%hhd,%hhd,%hhd,%hhd,%hhd,%hhd;%d",
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
    fprintf(fl, "|%.8X|%d", sk.first, sk.second);
  if (cur_skill != crc32c("None")) { // Added current skill to end in v0x13
    fprintf(fl, "|%.8X", cur_skill);
  }
  fprintf(fl, ";\n");

  fprintf(fl, "%d\n", (int)(contents.size()));
  for (auto cind : contents) {
    fprintf(fl, "%d\n", getnum(cind));
  }

  uint8_t num8 = static_cast<uint8_t>(pos);
  fprintf(fl, "%hhu\n", num8);

  fprintf(fl, "%d\n", (int)(act.size()));
  for (auto aind : act) {
    fprintf(fl, "%s;%d\n", act_save[static_cast<uint8_t>(aind.act())].c_str(), getnum(aind.obj()));
  }

  fprintf(fl, "\n");

  for (auto cind : contents) {
    cind->SaveTo(fl);
  }

  // fprintf(stderr, "Saved %s\n", Name());
  return 0;
}

static unsigned int ver;
int Object::Load(const std::string& fn) {
  FILE* fl = fopen(fn.c_str(), "r");
  if (!fl)
    return -1;

  fscanf(fl, "%X\n", &ver);

  // Load 'skill_names' hashes and strings.
  if (ver >= 0x0018) {
    load_skill_names_from(fl);
  }

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
    purge_invalid_skill_names();
  }

  for (auto ind : todo) {
    std::vector<act_t> killacts;
    for (auto aind : ind->act) {
      if (aind.act() != act_t::SPECIAL_ACTEE) {
        if (aind.obj()) {
          aind.obj()->NowTouching(ind);
        } else if (aind.act() <= act_t::REST) { // Targetless Actions
          aind.set_obj(nullptr);
        } else { // Act Targ No Longer Exists ("junkrestart", I hope)!
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
    if (ind->IsUsing(crc32c("Lumberjack"))) { // FIXME: All long-term skills?
      ind->BusyFor(500, "use Lumberjack");
    }
  }
  todo.clear();

  if (ver < 0x0016) {
    fscanf(fl, "%*d\n"); // Used to be start room, which is an action now
  }

  fclose(fl);
  return 0;
}

int Object::LoadFrom(FILE* fl) {
  // static std::string debug_indent = "";

  int num, res;
  fscanf(fl, "%d ", &num);
  if (num2obj[num] != this) {
    fprintf(stderr, CRED "Error: Acid number mismatch (%d)!\n" CNRM, num);
  }
  todo.push_back(this);

  memset(buf, 0, 65536);
  if (ver < 0x0015) {
    res = fscanf(fl, "%[^;]; ", buf);
    if (res < 1)
      fscanf(fl, " ; ");
  } else {
    num = 0;
    res = getc(fl);
    while (res > 0) {
      buf[num++] = res;
      res = getc(fl);
    }
  }
  std::string sd = buf;

  memset(buf, 0, 65536);
  if (ver >= 0x001B) {
    num = 0;
    res = getc(fl);
    while (res > 0) {
      buf[num++] = res;
      res = getc(fl);
    }
  }
  std::string n = buf;

  memset(buf, 0, 65536);
  if (ver < 0x0015) {
    res = fscanf(fl, "%[^;];\n", buf);
    if (res < 1)
      fscanf(fl, " ; ");
  } else {
    num = 0;
    res = getc(fl);
    while (res > 0) {
      buf[num++] = res;
      res = getc(fl);
    }
  }
  std::string d = buf;

  memset(buf, 0, 65536);
  if (ver < 0x0015) {
    res = fscanf(fl, "%[^;]; ", buf);
    if (res < 1)
      fscanf(fl, " ; ");
    for (size_t i = 0; i < strlen(buf); ++i) {
      if (buf[i] == '\e')
        buf[i] = ';';
    }
  } else {
    num = 0;
    res = getc(fl);
    while (res > 0) {
      buf[num++] = res;
      res = getc(fl);
    }
  }
  std::string ld = buf;

  SetDescs(sd, n, d, ld);

  // fprintf(stderr, "%sLoading %d:%s\n", debug_indent.c_str(), num, buf);

  fscanf(fl, "%d %d %d %d %c", &weight, &size, &volume, &value, &gender);

  if (ver < 0x001A) {
    fscanf(fl, "%*d"); // Experience (Redundant)
    fscanf(fl, ";"); // Was present pre v0x15, causes no problems until v0x1A.
    fscanf(fl, "\n"); // Skip the white-space, if ';' was used or not.
  } else {
    fscanf(fl, "\n"); // Skip the white-space before the line of "known".
    unsigned long know;
    while (fscanf(fl, ";%ld", &know)) {
      known.push_back(know);
    }
    fscanf(fl, ":\n"); // Skip the ending colon and white-space after the line of "known".
  }

  fscanf(fl, "\n"); // Skip the white-space before the line of "completed".
  unsigned long accom;
  while (fscanf(fl, ";%ld", &accom)) {
    completed.push_back(accom);
  }
  if (ver >= 0x001A) {
    fscanf(fl, ":\n"); // Skip the ending colon and white-space after the line of "completed".
  }

  fscanf(fl, " %d\n", &sexp);

  if (ver < 0x0017) {
    fscanf(
        fl,
        "%hhd,%hhd,%hhd,%hhd,%hhd,%hhd,%*d,%*d,%hhd,%hhd,%hhd",
        &attr[0],
        &attr[1],
        &attr[2],
        &attr[3],
        &attr[4],
        &attr[5],
        &phys,
        &stun,
        &stru);
  } else if (ver < 0x0019) {
    int16_t mods[6];
    fscanf(
        fl,
        "%*d,%hhd,%hd,%*d,%hhd,%hd,%*d,%hhd,%hd,%*d,%hhd,%hd,%*d,%hhd,%hd,%*d,%hhd,%hd,%hhd,%hhd,%hhd",
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
        "%hhd,%hhd,%hhd,%hhd,%hhd,%hhd,%hhd,%hhd,%hhd",
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
  if (fscanf(fl, ";%d", &do_tick)) {
    if (do_tick)
      Activate();
  }

  fscanf(fl, "\n");

  memset(buf, 0, 65536);
  int val;
  if (ver <= 0x0010) {
    while (fscanf(fl, ":%[^\n,:],%d", buf, &val)) {
      // fprintf(stderr, "Loaded %s: %d\n", buf, val);
      SetSkill(buf, val);
    }
  } else if (ver <= 0x0011) {
    while (fscanf(fl, ":%[^\n:|]|%d", buf, &val)) {
      // fprintf(stderr, "Loaded %s: %d\n", buf, val);
      SetSkill(buf, val);
    }
  } else if (ver < 0x0018) { // Backward compatible load between v0x12 and v0x13
    int ret;
    ret = fscanf(fl, "|%[^\n|;]|%d", buf, &val);
    while (ret > 1) {
      // fprintf(stderr, "Loaded %s: %d\n", buf, val);
      SetSkill(buf, val);
      ret = fscanf(fl, "|%[^\n|;]|%d", buf, &val);
    }
    if (ret > 0) { // Added the currently used skill to the end in v0x13
      StartUsing(get_skill(buf));
    }
  } else { // Skills now saved/loaded by hash, not name
    int ret;
    uint32_t stok;
    ret = fscanf(fl, "|%X|%d", &stok, &val);
    while (ret > 1) {
      // fprintf(stderr, "Loaded %s: %d\n", buf, val);
      if (ver < 0x0019) { // Update to new hash algo
        auto name = SkillName(stok);
        stok = crc32c(name);
        insert_skill_hash(stok, name);
      }
      SetSkill(stok, val);
      ret = fscanf(fl, "|%X|%d", &stok, &val);
    }
    if (ret > 0) { // Added the currently used skill to the end in v0x13
      if (ver < 0x0019) { // Update to new hash algo
        auto name = SkillName(stok);
        stok = crc32c(name);
        insert_skill_hash(stok, name);
      }
      StartUsing(stok);
    }
  }
  fscanf(fl, ";\n");

  std::vector<Object*> toload;
  fscanf(fl, "%d ", &num);
  contents.reserve(num);
  for (int ctr = 0; ctr < num; ++ctr) {
    int num2;
    fscanf(fl, "%d ", &num2);
    Object* obj = getbynum(num2);
    obj->SetParent(this);
    toload.push_back(obj);
    AddLink(obj);
  }

  uint8_t num8;
  fscanf(fl, "%hhu\n", &num8);
  pos = static_cast<pos_t>(num8);

  fscanf(fl, "%d ", &num);
  for (int ctr = 0; ctr < num; ++ctr) {
    int num2;
    act_t a;
    if (ver < 0x0014) { // Action types stored numerically < v0x14
      uint8_t anum;
      fscanf(fl, "%hhd;%d ", &anum, &num2);
      a = static_cast<act_t>(anum);
    } else { // Action types stored as std::strings >= v0x14
      memset(buf, 0, 65536);
      fscanf(fl, "%65535[^;];%d ", buf, &num2);
      a = act_load(std::string(buf));
    }
    if (a != act_t::SPECIAL_ACTEE) {
      AddAct(a, getbynum(num2));
    }
  }

  if (Skill(crc32c("Personality")))
    get_mob_mind()->Attach(this);
  else if (Skill(crc32c("TBAAction")))
    get_tba_mob_mind()->Attach(this);

  if (Skill(crc32c("TBAScriptType")) & 2) { // Random/Permanent Triggers
    Mind* trig = new_mind(MIND_TBATRIG, this);
    trig->Suspend((rand() % 13000) + 3000); // 3-16 Seconds
  }

  //  int num_loaded = 0;
  //  if(parent && (!(parent->parent))) {
  //    fprintf(stderr, "Loading: %s\n", ShortDescC());
  //    }
  for (auto cind : toload) {
    // fprintf(stderr, "%sCalling loader from %s\n", debug_indent.c_str(),
    // ShortDescC());
    // std::string tmp = debug_indent;
    // debug_indent += "  ";
    cind->LoadFrom(fl);
    // debug_indent = tmp;
    // fprintf(stderr, "%sCalled loader from %s\n", debug_indent.c_str(),
    // ShortDescC());

    //    if(parent && (!(parent->parent))) {
    //      fprintf(stderr, "\rLoaded: %d/%d (%s)    ",
    //	++num_loaded, int(toload.size()), cind->ShortDescC()
    //	);
    //      }
  }
  //  if(parent && (!(parent->parent))) {
  //    fprintf(stderr, "\nLoaded.\n");
  //    }

  // fprintf(stderr, "%sLoaded %s\n", debug_indent.c_str(), ShortDescC());

  //  if(HasSkill(crc32c("Drink"))) {
  //    SetSkill(crc32c("Drink"), Skill(crc32c("Drink")) * 15);
  //    SetSkill(crc32c("Food"), Skill(crc32c("Food")) * 15);
  //    }
  //  else if(HasSkill(crc32c("Food"))) {
  //    SetSkill(crc32c("Food"), Skill(crc32c("Food")) * 60);
  //    }

  //  if(HasSkill(crc32c("Heal Effect"))) {
  //    SetSkill(crc32c("Heal Spell"), Skill(crc32c("Heal Effect")));
  //    SetSkill(crc32c("Heal Effect"), 0);
  //    }

  //  if(HasSkill(crc32c("Open"))) {
  //    SetSkill(crc32c("Open"), 1000);
  //    }

  //  if(HasSkill(crc32c("Day Time"))) {
  //    Activate();
  //    }

  //  if(IsAct(act_t::SPECIAL_NOTSHOWN)) {
  //    SetSkill(crc32c("Invisible"), 1000);
  //    }

  //  if(short_desc == "A passage exit.") {
  //    short_desc = "a passage exit";
  //    }

  //  if(HasSkill(crc32c("Evasion Penalty"))) {
  //    SetSkill(crc32c("Evasion Bonus"), Skill(crc32c("Evasion Penalty")));
  //    SetSkill(crc32c("Evasion Penalty"), 0);
  //    }
  //  else if(HasSkill(crc32c("Evasion Bonus"))) {
  //    SetSkill(crc32c("Evasion Penalty"), Skill(crc32c("Evasion Bonus")));
  //    SetSkill(crc32c("Evasion Bonus"), 0);
  //    }

  //  if(IsAct(act_t::SPECIAL_PREPARE)) {
  //    fprintf(stderr, "Found one!\n");
  //    StopAct(act_t::SPECIAL_PREPARE);
  //    }

  //  if(short_desc == "a gold piece") {
  //    SetSkill(crc32c("Money"), 1);
  //    }

  return 0;
}
