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

#include "color.hpp"
#include "log.hpp"
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
    loge(u8"Warning: No Skill Type {}!\n", wtype);
    return prhash(u8"None");
  }
  return weaponskills[wtype];
}

int get_weapon_type(const std::u8string_view& wskill) {
  if (!weapontypes.count(crc32c(wskill))) {
    loge(u8"Warning: No Skill Named '{}'!\n", wskill);
    return 0;
  }
  return weapontypes[crc32c(wskill)];
}

uint32_t get_skill(const std::u8string_view& in_sk) {
  std::u8string_view sk(in_sk);
  trim_string(sk);

  auto stok = crc32c(sk);
  if (defaults.count(stok))
    return stok;
  if (sk.length() < 2)
    return prhash(u8"None");

  for (auto itr : defaults) {
    std::u8string lc = SkillName(itr.first);
    std::transform(lc.begin(), lc.end(), lc.begin(), ascii_tolower);
    if (lc.starts_with(sk)) {
      return itr.first;
    }
  }
  return prhash(u8"None");
}

std::u8string get_skill_cat(const std::u8string_view& in_cat) {
  std::u8string_view cat(in_cat);
  trim_string(cat);

  if (cat.length() < 2)
    return u8"";

  for (auto itr : skcat) {
    auto skn = itr.first;
    std::transform(skn.begin(), skn.end(), skn.begin(), ascii_tolower);
    if (skn.starts_with(cat)) {
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

std::vector<uint32_t> get_skills(const std::u8string_view& in_cat) {
  std::vector<uint32_t> ret;

  std::u8string_view cat(in_cat);
  trim_string(cat);

  if (cat == u8"Categories") {
    for (auto ind : skcat) {
      ret.push_back(crc32c(ind.first));
    }
  } else if (cat == u8"all") {
    for (auto ind : defaults) {
      ret.push_back(ind.first);
    }
  } else if (skcat.count(std::u8string(cat))) {
    for (auto ind : skcat[std::u8string(cat)]) {
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
      (*res) += fmt::format(u8"{},", val);
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

static const uint32_t attr_bonus[6] = {
    prhash(u8"Body Bonus"),
    prhash(u8"Quickness Bonus"),
    prhash(u8"Strength Bonus"),
    prhash(u8"Charisma Bonus"),
    prhash(u8"Intelligence Bonus"),
    prhash(u8"Willpower Bonus"),
};
static const uint32_t attr_penalty[6] = {
    prhash(u8"Body Penalty"),
    prhash(u8"Quickness Penalty"),
    prhash(u8"Strength Penalty"),
    prhash(u8"Charisma Penalty"),
    prhash(u8"Intelligence Penalty"),
    prhash(u8"Willpower Penalty"),
};
void Object::SetModifier(int a, int v) {
  if (v > 10000)
    v = 10000;
  else if (v < -10000)
    v = -10000;
  if (v > 0) {
    SetSkill(attr_bonus[a], v);
    ClearSkill(attr_penalty[a]);
  } else {
    ClearSkill(attr_bonus[a]);
    SetSkill(attr_penalty[a], -v);
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
    skills.push_back(skill_pair{stok, v});
  } else {
    itr->second = v;
  }
}

void Object::SetSkill(const std::u8string_view& s, int v) {
  auto stok = crc32c(s);
  insert_skill_hash(stok, s);
  SetSkill(stok, v);
}

void Object::ClearSkill(uint32_t stok) {
  auto itr = skills.begin();
  for (; itr != skills.end(); ++itr) {
    if (itr->first == stok) {
      break;
    }
  }
  if (itr != skills.end()) {
    skills.erase(itr);
  }
}

void Object::ClearSkill(const std::u8string_view& s) {
  auto stok = crc32c(s);
  insert_skill_hash(stok, s);
  ClearSkill(stok);
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
    loger(u8"Error: Asked to roll non-existant skill: '{}'\n", SkillName(stok));
    return 0;
  }

  std::u8string rolls = u8"";
  succ = roll(abs(dice), (targ - mod) + penalty, &rolls);

  if (res) {
    (*res) += fmt::format(u8"{}", abs(dice));
    (*res) += u8"(";
    (*res) += fmt::format(u8"{}", targ);
    (*res) += u8")";
    if (mod > 0) {
      (*res) += u8"+";
      (*res) += fmt::format(u8"{}", mod);
    }
    if (penalty > 0) {
      (*res) += u8"-";
      (*res) += fmt::format(u8"{}", penalty);
    }
    if (!rolls.empty()) {
      (*res) += u8"[";
      (*res) += rolls;
      (*res) += u8"]";
    }
    (*res) += u8":";
    (*res) += fmt::format(u8"{}", succ);
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
