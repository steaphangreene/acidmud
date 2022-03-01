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

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include "cchar8.hpp"
#include "color.hpp"
#include "object.hpp"
#include "properties.hpp"
#include "utils.hpp"

extern std::map<uint32_t, int32_t> defaults;
extern std::map<int32_t, uint32_t> weaponskills;
extern std::map<uint32_t, int32_t> weapontypes;
extern std::map<std::u8string, std::vector<uint32_t>> skcat;

static bool defaults_init = false;
static void init_defaults() {
  defaults_init = true;
  init_skill_list();
}
void Object::InitSkillsData() {
  if (!defaults_init) {
    init_defaults();
  }
}

int is_skill(uint32_t stok) {
  return (defaults.count(stok) != 0);
}

uint32_t get_weapon_skill(int wtype) {
  if (!weaponskills.count(wtype)) {
    fprintf(stderr, u8"Warning: No Skill Type %d!\n", wtype);
    return prhash(u8"None");
  }
  return weaponskills[wtype];
}

int get_weapon_type(std::u8string wskill) {
  if (!weapontypes.count(crc32c(wskill))) {
    fprintf(stderr, u8"Warning: No Skill Named '%s'!\n", wskill.c_str());
    return 0;
  }
  return weapontypes[crc32c(wskill)];
}

uint32_t get_skill(std::u8string sk) {
  while (sk.length() > 0 && isspace(sk.back()))
    sk.pop_back();

  auto stok = crc32c(sk);
  if (defaults.count(stok))
    return stok;
  if (sk.length() < 2)
    return prhash(u8"None");

  for (auto itr : defaults) {
    std::u8string lc = SkillName(itr.first);
    std::transform(lc.begin(), lc.end(), lc.begin(), ascii_tolower);
    if (!strncmp(sk.c_str(), lc.c_str(), sk.length())) {
      return itr.first;
    }
  }
  return prhash(u8"None");
}

std::u8string get_skill_cat(std::u8string cat) {
  trim_string(cat);

  if (cat.length() < 2)
    return u8"";

  for (auto itr : skcat) {
    auto skn = itr.first;
    std::transform(skn.begin(), skn.end(), skn.begin(), ascii_tolower);
    if (!strncmp(cat.c_str(), skn.c_str(), cat.length())) {
      return itr.first;
    }
  }
  return u8"";
}

int get_linked(uint32_t stok) {
  if (defaults.count(stok))
    return defaults[stok];
  return 4; // Default to Int for knowledges
}

int get_linked(std::u8string sk) {
  while (sk.length() > 0 && isspace(sk.back()))
    sk.pop_back();
  return get_linked(crc32c(sk));
}

std::vector<uint32_t> get_skills(std::u8string cat) {
  std::vector<uint32_t> ret;

  while (cat.length() > 0 && isspace(cat.back()))
    cat.pop_back();

  if (cat == u8"Categories") {
    for (auto ind : skcat) {
      ret.push_back(crc32c(ind.first));
    }
  } else if (cat == u8"all") {
    for (auto ind : defaults) {
      ret.push_back(ind.first);
    }
  } else if (skcat.count(cat)) {
    for (auto ind : skcat[cat]) {
      ret.push_back(ind);
    }
  }

  return ret;
}

int roll(int ndice, int targ, std::u8string* res) {
  int succ = 0;
  for (int die = 0; die < ndice; ++die) {
    int val = 1 + (rand() % 6);
    while ((val % 6) == 0)
      val += (1 + rand() % 6);
    if (res) {
      char8_t buffer[16];
      sprintf(buffer, u8"%d,", val);
      (*res) += buffer;
    }
    if (val > 1 && val >= targ) {
      unsigned int numwrap = (unsigned int)((val - targ) / 12);
      succ += (1 + int(numwrap));
    }
  }
  if (res && !res->empty()) {
    res->pop_back();
  }
  return succ;
}

void Object::SetAttribute(int a, int v) {
  if (v > 100)
    v = 100;
  else if (v < 0)
    v = 0;
  attr[a] = v;
}

static const char8_t* const attr_name[6] = {
    u8"Body",
    u8"Quickness",
    u8"Strength",
    u8"Charisma",
    u8"Intelligence",
    u8"Willpower",
};
void Object::SetModifier(int a, int v) {
  if (v > 10000)
    v = 10000;
  else if (v < -10000)
    v = -10000;
  if (v > 0) {
    SetSkill(std::u8string(attr_name[a]) + u8" Bonus", v);
    SetSkill(std::u8string(attr_name[a]) + u8" Penalty", 0);
  } else {
    SetSkill(std::u8string(attr_name[a]) + u8" Bonus", 0);
    SetSkill(std::u8string(attr_name[a]) + u8" Penalty", -v);
  }
}

void Object::SetSkill(uint32_t stok, int v) {
  if (v > 1000000000)
    v = 1000000000;
  else if (v < -1000000000)
    v = -1000000000;

  confirm_skill_hash(stok);

  auto itr = skills.begin();
  for (; itr != skills.end(); ++itr) {
    if (itr->first == stok) {
      break;
    }
  }
  if (itr == skills.end()) {
    if (v != 0) {
      skills.push_back(skill_pair{stok, v});
    }
  } else if (v == 0) {
    skills.erase(itr);
  } else {
    itr->second = v;
  }
}

void Object::SetSkill(const std::u8string& s, int v) {
  if (v > 1000000000)
    v = 1000000000;
  else if (v < -1000000000)
    v = -1000000000;

  auto stok = crc32c(s);
  insert_skill_hash(stok, s);

  auto itr = skills.begin();
  for (; itr != skills.end(); ++itr) {
    if (itr->first == stok) {
      break;
    }
  }
  if (itr == skills.end()) {
    if (v != 0) {
      skills.push_back(skill_pair{stok, v});
    }
  } else if (v == 0) {
    skills.erase(itr);
  } else {
    itr->second = v;
  }
}

int Object::SkillTarget(uint32_t stok) const {
  auto itr = skills.begin();
  for (; itr != skills.end(); ++itr) {
    if (itr->first == stok) {
      break;
    }
  }
  if (itr != skills.end()) {
    return itr->second + ModAttribute(defaults[stok]);
  } else {
    return ModAttribute(defaults[stok]);
  }
  return 0;
}

int Object::Roll(uint32_t s1, const Object* p2, uint32_t s2, int bias, std::u8string* res) const {
  int succ = 0;

  int t1 = p2->SkillTarget(s2) - bias;
  int t2 = SkillTarget(s1) + bias;

  if (res)
    (*res) += u8"(";
  succ = Roll(s1, t1, res);
  if (s2 != prhash(u8"None")) {
    if (res)
      (*res) += u8" - ";
    succ -= p2->Roll(s2, t2, res);
  }
  if (res)
    (*res) += u8")";
  return succ;
}

int Object::Roll(uint32_t stok, int targ, std::u8string* res) const {
  if (phys >= 10 || stun >= 10 || attr[0] == 0 || attr[1] == 0 || attr[2] == 0 || attr[3] == 0 ||
      attr[4] == 0 || attr[5] == 0) {
    if (res)
      (*res) += u8"N/A";
    return 0;
  }
  return RollNoWounds(stok, targ, WoundPenalty(), res);
}

int Object::RollNoWounds(uint32_t stok, int targ, int penalty, std::u8string* res) const {
  int succ = 0;
  int dice = 0;
  int mod = 0;

  if (stok == prhash(u8"Body")) {
    dice = ModAttribute(0);
  } else if (stok == prhash(u8"Quickness")) {
    dice = ModAttribute(1);
  } else if (stok == prhash(u8"Strength")) {
    dice = ModAttribute(2);
  } else if (stok == prhash(u8"Charisma")) {
    dice = ModAttribute(3);
  } else if (stok == prhash(u8"Intelligence")) {
    dice = ModAttribute(4);
  } else if (stok == prhash(u8"Willpower")) {
    dice = ModAttribute(5);
  } else if (stok == prhash(u8"Reaction")) {
    dice = ModAttribute(6);
  } else if (defaults.count(stok) > 0) {
    dice = Skill(stok);
    if (dice > 0) {
      mod = ModAttribute(defaults[stok]);
    } else {
      dice = ModAttribute(defaults[stok]);
    }
  } else {
    fprintf(
        stderr,
        CRED u8"Error: Asked to roll non-existant skill: '%s'\n" CNRM,
        SkillName(stok).c_str());
    return 0;
  }

  std::u8string rolls = u8"";
  succ = roll(abs(dice), (targ - mod) + penalty, &rolls);

  if (res) {
    char8_t buffer[256];
    sprintf(buffer, u8"%d%c", abs(dice), 0);
    (*res) += buffer;
    (*res) += u8"(";
    sprintf(buffer, u8"%d%c", targ, 0);
    (*res) += buffer;
    (*res) += u8")";
    if (mod > 0) {
      (*res) += u8"+";
      sprintf(buffer, u8"%d%c", mod, 0);
      (*res) += buffer;
    }
    if (penalty > 0) {
      (*res) += u8"-";
      sprintf(buffer, u8"%d%c", penalty, 0);
      (*res) += buffer;
    }
    if (!rolls.empty()) {
      (*res) += u8"[";
      (*res) += rolls;
      (*res) += u8"]";
    }
    (*res) += u8":";
    sprintf(buffer, u8"%d%c", succ, 0);
    (*res) += buffer;
  }

  return succ;
}

int Object::WoundPenalty() const {
  int ret = 0;
  if (stun >= 10)
    ret += 4;
  else if (stun >= 6)
    ret += 3;
  else if (stun >= 3)
    ret += 2;
  else if (stun >= 1)
    ret += 1;
  if (phys >= 10)
    ret += 4;
  else if (phys >= 6)
    ret += 3;
  else if (phys >= 3)
    ret += 2;
  else if (phys >= 1)
    ret += 1;

  return ret;
}

int Object::RollInitiative() const {
  return Roll(prhash(u8"Reaction"), 6 - ModAttribute(5));
}
