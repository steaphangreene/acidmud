#include <unistd.h>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "color.hpp"
#include "commands.hpp"
#include "mind.hpp"
#include "mob.hpp"
#include "object.hpp"
#include "utils.hpp"

static std::map<act_t, std::string> wear_attribs;
static void init_wear_attribs() {
  wear_attribs[ACT_WEAR_BACK] = "Wearable on Back";
  wear_attribs[ACT_WEAR_CHEST] = "Wearable on Chest";
  wear_attribs[ACT_WEAR_HEAD] = "Wearable on Head";
  wear_attribs[ACT_WEAR_NECK] = "Wearable on Neck";
  wear_attribs[ACT_WEAR_COLLAR] = "Wearable on Collar";
  wear_attribs[ACT_WEAR_WAIST] = "Wearable on Waist";
  wear_attribs[ACT_WEAR_SHIELD] = "Wearable on Shield";
  wear_attribs[ACT_WEAR_LARM] = "Wearable on Left Arm";
  wear_attribs[ACT_WEAR_RARM] = "Wearable on Right Arm";
  wear_attribs[ACT_WEAR_LFINGER] = "Wearable on Left Finger";
  wear_attribs[ACT_WEAR_RFINGER] = "Wearable on Right Finger";
  wear_attribs[ACT_WEAR_LFOOT] = "Wearable on Left Foot";
  wear_attribs[ACT_WEAR_RFOOT] = "Wearable on Right Foot";
  wear_attribs[ACT_WEAR_LHAND] = "Wearable on Left Hand";
  wear_attribs[ACT_WEAR_RHAND] = "Wearable on Right Hand";
  wear_attribs[ACT_WEAR_LLEG] = "Wearable on Left Leg";
  wear_attribs[ACT_WEAR_RLEG] = "Wearable on Right Leg";
  wear_attribs[ACT_WEAR_LWRIST] = "Wearable on Left Wrist";
  wear_attribs[ACT_WEAR_RWRIST] = "Wearable on Right Wrist";
  wear_attribs[ACT_WEAR_LSHOULDER] = "Wearable on Left Shoulder";
  wear_attribs[ACT_WEAR_RSHOULDER] = "Wearable on Right Shoulder";
  wear_attribs[ACT_WEAR_LHIP] = "Wearable on Left Hip";
  wear_attribs[ACT_WEAR_RHIP] = "Wearable on Right Hip";
  wear_attribs[ACT_WEAR_FACE] = "Wearable on Face";
}

static Mind* mob_mind = nullptr;
Mind* get_mob_mind() {
  if (!mob_mind) {
    mob_mind = new Mind();
    mob_mind->SetMob();
  }
  return mob_mind;
}

static Object* gold = nullptr;
static void init_gold() {
  gold = new Object();
  gold->SetShortDesc("a gold piece");
  gold->SetDesc("A standard one-ounce gold piece.");
  gold->SetWeight(454 / 16);
  gold->SetVolume(0);
  gold->SetValue(1);
  gold->SetSize(0);
  gold->SetPos(POS_LIE);
}

static void give_gold(Object* mob, int qty) {
  Object* bag = new Object;

  bag->SetParent(mob);
  bag->SetShortDesc("a small purse");
  bag->SetDesc("A small, durable, practical moneypurse.");

  bag->SetSkill(crc32c("Wearable on Left Hip"), 1);
  bag->SetSkill(crc32c("Wearable on Right Hip"), 2);
  bag->SetSkill(crc32c("Container"), 5 * 454);
  bag->SetSkill(crc32c("Capacity"), 5);
  bag->SetSkill(crc32c("Closeable"), 1);

  bag->SetWeight(1 * 454);
  bag->SetSize(2);
  bag->SetVolume(1);
  bag->SetValue(100);

  bag->SetPos(POS_LIE);
  mob->AddAct(ACT_WEAR_LHIP, bag);

  if (!gold)
    init_gold();
  Object* g = new Object(*gold);
  g->SetParent(bag);
  g->SetSkill(crc32c("Quantity"), qty);
}

MOBType::MOBType(
    const std::string& nm,
    const std::string& ds,
    const std::string& lds,
    const std::string& gens,
    int ib,
    int ibm,
    int iq,
    int iqm,
    int is,
    int ism,
    int ic,
    int icm,
    int ii,
    int iim,
    int iw,
    int iwm,
    int ig,
    int igm) {
  name = nm;
  desc = ds;
  long_desc = lds;
  genders = gens;
  b = ib;
  bm = std::max(ibm, 1);
  q = iq;
  qm = std::max(iqm, 1);
  s = is;
  sm = std::max(ism, 1);
  c = ic;
  cm = std::max(icm, 1);
  i = ii;
  im = std::max(iim, 1);
  w = iw;
  wm = std::max(iwm, 1);
  g = ig;
  gm = std::max(igm, 1);

  armed = nullptr;
}

void MOBType::Skill(uint32_t stok, int percent, int mod) {
  skills[stok] = std::make_pair(percent, mod);
}

void MOBType::Skill(uint32_t stok, int val) {
  skills[stok] = std::make_pair(-1, val);
}

void MOBType::Arm(WeaponType* weap) {
  armed = weap;
}

void MOBType::Armor(ArmorType* arm) {
  armor.push_back(arm);
}

void Object::AddMOB(const MOBType* type) {
  Object* mob = new Object(this);

  mob->Attach(get_mob_mind());
  mob->Activate();
  mob->SetPos(POS_STAND);
  mob->SetAttribute(0, type->b + rand() % type->bm);
  mob->SetAttribute(1, type->q + rand() % type->qm);
  mob->SetAttribute(2, type->s + rand() % type->sm);
  mob->SetAttribute(3, type->c + rand() % type->cm);
  mob->SetAttribute(4, type->i + rand() % type->im);
  mob->SetAttribute(5, type->w + rand() % type->wm);

  for (auto sk : type->skills) {
    if (sk.second.first < 0) {
      mob->SetSkill(sk.first, sk.second.second);
    } else {
      mob->SetSkill(
          sk.first,
          mob->NormAttribute(get_linked(sk.first)) * sk.second.first / 100 -
              rand() % sk.second.second);
    }
    // fprintf(stderr, "DBG: %d %d %d\n", get_linked(sk.first),
    // sk.second.first, sk.second.second);
  }

  mob->SetShortDesc(type->name.c_str());
  mob->SetDesc(type->desc.c_str());
  mob->SetLongDesc(type->long_desc.c_str());
  if (type->genders.length() > 0) {
    mob->SetGender(type->genders[rand() % type->genders.length()]);
  }
  mob->SetDesc(gender_proc(type->desc, mob->Gender()));
  mob->SetLongDesc(gender_proc(type->long_desc, mob->Gender()));

  if (type->g > 0 || type->gm > 1)
    give_gold(mob, type->g + rand() % type->gm);

  if (type->armed) {
    Object* obj = new Object(mob);
    obj->SetSkill(crc32c("WeaponType"), type->armed->type);
    obj->SetSkill(crc32c("WeaponReach"), type->armed->reach);
    obj->SetSkill(crc32c("WeaponForce"), type->armed->force + rand() % type->armed->forcem);
    obj->SetSkill(crc32c("WeaponSeverity"), type->armed->sev + rand() % type->armed->sevm);
    obj->SetShortDesc(type->armed->name.c_str());
    obj->SetDesc(type->armed->desc.c_str());
    obj->SetLongDesc(type->armed->long_desc.c_str());
    obj->SetWeight(type->armed->weight);
    obj->SetVolume(type->armed->volume);
    obj->SetValue(type->armed->value);
    obj->SetPos(POS_LIE);
    mob->AddAct(ACT_WIELD, obj);
    if (two_handed(type->armed->type))
      mob->AddAct(ACT_HOLD, obj);
  }

  for (auto ar : type->armor) {
    if (wear_attribs.size() <= 0) {
      init_wear_attribs();
    }
    Object* obj = new Object(mob);
    obj->SetAttribute(0, ar->bulk + rand() % ar->bulkm);
    obj->SetSkill(crc32c("ArmorB"), ar->bulk + rand() % ar->bulkm);
    obj->SetSkill(crc32c("ArmorI"), ar->impact + rand() % ar->impactm);
    obj->SetSkill(crc32c("ArmorT"), ar->thread + rand() % ar->threadm);
    obj->SetSkill(crc32c("ArmorP"), ar->planar + rand() % ar->planarm);
    obj->SetShortDesc(ar->name.c_str());
    obj->SetDesc(ar->desc.c_str());
    obj->SetLongDesc(ar->long_desc.c_str());
    obj->SetWeight(ar->weight);
    obj->SetVolume(ar->volume);
    obj->SetValue(ar->value);
    obj->SetPos(POS_LIE);

    for (auto loc : ar->loc) {
      obj->SetSkill(wear_attribs[loc], 1);
      mob->AddAct(loc, obj);
    }
  }
}

WeaponType::WeaponType(
    const std::string& nm,
    const std::string& ds,
    const std::string& lds,
    const std::string& t,
    int r,
    int f,
    int fm,
    int s,
    int sm,
    int w,
    int vol,
    int val) {
  name = nm;
  desc = ds;
  long_desc = lds;
  type = get_weapon_type(t);
  reach = r;
  force = f;
  forcem = std::max(fm, 1);
  sev = s;
  sevm = std::max(sm, 1);
  weight = w;
  volume = vol;
  value = val;
}

ArmorType::ArmorType(
    const std::string& nm,
    const std::string& ds,
    const std::string& lds,
    int b,
    int bm,
    int i,
    int im,
    int t,
    int tm,
    int p,
    int pm,
    int w,
    int vol,
    int val,
    act_t l1,
    act_t l2,
    act_t l3,
    act_t l4,
    act_t l5,
    act_t l6) {
  name = nm;
  desc = ds;
  long_desc = lds;

  bulk = b;
  bulkm = std::max(bm, 1);
  impact = i;
  impactm = std::max(im, 1);
  thread = t;
  threadm = std::max(tm, 1);
  planar = p;
  planarm = std::max(pm, 1);

  if (l1 != ACT_NONE)
    loc.push_back(l1);
  if (l2 != ACT_NONE)
    loc.push_back(l2);
  if (l3 != ACT_NONE)
    loc.push_back(l3);
  if (l4 != ACT_NONE)
    loc.push_back(l4);
  if (l5 != ACT_NONE)
    loc.push_back(l5);
  if (l6 != ACT_NONE)
    loc.push_back(l6);

  weight = w;
  volume = vol;
  value = val;
}

static const std::string gen_replace[][4] = {
    {"{He}", "She", "He", "It"},
    {"{Him}", "Her", "Him", "It"},
    {"{His}", "Hers", "His", "Its"},
    {"{he}", "she", "he", "it"},
    {"{him}", "her", "him", "it"},
    {"{his}", "hers", "his", "its"},
    {"", "", "", ""}};

std::string gender_proc(const std::string& in, char gender) {
  std::string ret = in;
  int ctr = 0, gen = 3;

  if (gender == 'F')
    gen = 1;
  if (gender == 'M')
    gen = 2;

  for (ctr = 0; !gen_replace[ctr][0].empty(); ++ctr) {
    int where = ret.find(gen_replace[ctr][0]);
    while (where >= 0 && where <= int(ret.length())) {
      fprintf(stderr, "Yep: %d %s (%s)\n", where, ret.c_str(), gen_replace[ctr][0].c_str());
      ret = ret.substr(0, where) + gen_replace[ctr][gen] +
          ret.substr(where + gen_replace[ctr][0].length());
      where = ret.find(gen_replace[ctr][0]);
    }
  }
  return ret;
}
