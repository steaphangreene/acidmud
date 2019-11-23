#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>
#include <vector>

#include "color.hpp"
#include "object.hpp"
#include "utils.hpp"

extern std::map<uint32_t, int32_t> defaults;
extern std::map<int32_t, uint32_t> weaponskills;
extern std::map<uint32_t, int32_t> weapontypes;
extern std::map<std::string, std::vector<uint32_t>> skcat;

static int defaults_init = 0;
static void init_defaults() {
  if (defaults_init)
    return;
  defaults_init = 1;

  init_skill_names();
  init_skill_list();
}

int is_skill(uint32_t stok) {
  if (!defaults_init)
    init_defaults();
  return (defaults.count(stok) != 0);
}

uint32_t get_weapon_skill(int wtype) {
  if (!defaults_init)
    init_defaults();
  if (!weaponskills.count(wtype)) {
    fprintf(stderr, "Warning: No Skill Type %d!\n", wtype);
    return crc32c("None");
  }
  return weaponskills[wtype];
}

int get_weapon_type(std::string wskill) {
  if (!defaults_init)
    init_defaults();
  if (!weapontypes.count(crc32c(wskill))) {
    fprintf(stderr, "Warning: No Skill Named '%s'!\n", wskill.c_str());
    return 0;
  }
  return weapontypes[crc32c(wskill)];
}

uint32_t get_skill(std::string sk) {
  while (sk.length() > 0 && isspace(sk.back()))
    sk.pop_back();

  auto stok = crc32c(sk);
  if (defaults.count(stok))
    return stok;
  if (sk.length() < 2)
    return crc32c("None");

  for (auto itr : defaults) {
    std::string lc = SkillName(itr.first);
    std::transform(lc.begin(), lc.end(), lc.begin(), ::tolower);
    if (!strncmp(sk.c_str(), lc.c_str(), sk.length())) {
      return itr.first;
    }
  }
  return crc32c("None");
}

std::string get_skill_cat(std::string cat) {
  while (cat.length() > 0 && isspace(cat.back()))
    cat.pop_back();
  if (skcat.count(cat))
    return cat;
  if (cat.length() < 2)
    return "";

  for (auto itr : skcat) {
    if (!strncmp(cat.c_str(), itr.first.c_str(), cat.length())) {
      return itr.first;
    }
  }
  return "";
}

int get_linked(uint32_t stok) {
  if (defaults.count(stok))
    return defaults[stok];
  return 4; // Default to Int for knowledges
}

int get_linked(std::string sk) {
  while (sk.length() > 0 && isspace(sk.back()))
    sk.pop_back();
  return get_linked(crc32c(sk));
}

std::vector<uint32_t> get_skills(std::string cat) {
  std::vector<uint32_t> ret;

  if (!defaults_init)
    init_defaults();

  while (cat.length() > 0 && isspace(cat.back()))
    cat.pop_back();

  if (cat == "Categories") {
    for (auto ind : skcat) {
      ret.push_back(crc32c(ind.first));
    }
  } else if (cat == "all") {
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

int roll(int ndice, int targ) {
  int ret = 0, ctr;
  for (ctr = 0; ctr < ndice; ++ctr) {
    int val = 1 + (rand() % 6);
    while ((val % 6) == 0)
      val += (1 + rand() % 6);
    if (val > 1 && val >= targ) {
      unsigned int numwrap = (unsigned int)((val - targ) / 12);
      ret += (1 + int(numwrap));
    }
  }
  return ret;
}

void Object::SetAttribute(int a, int v) {
  if (v > 100)
    v = 100;
  else if (v < 0)
    v = 0;
  att[a].cur = v;
}

void Object::SetModifier(int a, int v) {
  if (v > 10000)
    v = 10000;
  else if (v < -10000)
    v = -10000;
  att[a].mod = v;
}

void Object::SetSkill(uint32_t stok, int v) {
  if (v > 1000000000)
    v = 1000000000;
  else if (v < -1000000000)
    v = -1000000000;

  confirm_skill_hash(stok);

  auto itr = hash_locate(skills, stok);
  if (itr == skills.end() || itr->first != stok) {
    if (v > 0) {
      skills.emplace(itr, std::make_pair(stok, v));
    }
  } else if (v <= 0) {
    skills.erase(itr);
  } else {
    itr->second = v;
  }
}

void Object::SetSkill(const std::string& s, int v) {
  if (v > 1000000000)
    v = 1000000000;
  else if (v < -1000000000)
    v = -1000000000;

  auto stok = crc32c(s);
  insert_skill_hash(stok, s);

  auto itr = hash_locate(skills, stok);
  if (itr == skills.end() || itr->first != stok) {
    if (v > 0) {
      skills.emplace(itr, std::make_pair(stok, v));
    }
  } else if (v <= 0) {
    skills.erase(itr);
  } else {
    itr->second = v;
  }
}

int Object::HasSkill(uint32_t stok) const {
  auto itr = hash_find(skills, stok);
  return (itr != skills.end());
}

int Object::Skill(uint32_t stok, int* tnum) const {
  if (stok == crc32c("Body"))
    return ModAttribute(0);
  if (stok == crc32c("Quickness"))
    return ModAttribute(1);
  if (stok == crc32c("Strength"))
    return ModAttribute(2);
  if (stok == crc32c("Charisma"))
    return ModAttribute(3);
  if (stok == crc32c("Intelligence"))
    return ModAttribute(4);
  if (stok == crc32c("Willpower"))
    return ModAttribute(5);
  if (stok == crc32c("Reaction"))
    return ModAttribute(6);

  if (!defaults_init)
    init_defaults();

  auto itr = hash_find(skills, stok);
  if (itr != skills.end())
    return itr->second;
  if (tnum) {
    (*tnum) += 4;
    return ModAttribute(defaults[stok]);
  }
  return 0;
}

int Object::Roll(uint32_t s1, const Object* p2, uint32_t s2, int bias, std::string* res) const {
  int succ = 0;

  int t1 = p2->Skill(s2) - bias;
  int t2 = Skill(s1) + bias;

  if (res)
    (*res) += "(";
  succ = Roll(s1, t1, res);
  if (s2 != crc32c("None")) {
    if (res)
      (*res) += " - ";
    succ -= p2->Roll(s2, t2, res);
  }
  if (res)
    (*res) += ")";
  return succ;
}

int Object::Roll(uint32_t s1, int targ, std::string* res) const {
  if (phys >= 10 || stun >= 10 || att[0].cur == 0 || att[1].cur == 0 || att[2].cur == 0 ||
      att[3].cur == 0 || att[4].cur == 0 || att[5].cur == 0) {
    if (res)
      (*res) += "N/A";
    return 0;
  }
  targ += WoundPenalty();
  return RollNoWounds(s1, targ, res);
}

int Object::RollNoWounds(uint32_t s1, int targ, std::string* res) const {
  int succ = 0;
  int d1 = Skill(s1, &targ);
  succ = roll(abs(d1), targ);

  if (res) {
    char buffer[256];
    sprintf(buffer, "%d%c", abs(d1), 0);
    (*res) += buffer;
    (*res) += "(";
    sprintf(buffer, "%d%c", targ, 0);
    (*res) += buffer;
    (*res) += "):";
    sprintf(buffer, "%d%c", succ, 0);
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
  return Roll(crc32c("Reaction"), 6 - ModAttribute(5));
}
