#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <cctype>
#include <cmath>

#include "commands.h"
#include "object.h"
#include "color.h"
#include "mind.h"
#include "mob.h"

static Mind *mob_mind = NULL;
Mind *get_mob_mind() {
  if(!mob_mind) {
    mob_mind = new Mind();
    mob_mind->SetMob();
    }
  return mob_mind;
  }

static Object *gold = NULL;
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

static void give_gold(Object *mob, int qty) {
  Object *bag = new Object;

  bag->SetParent(mob);
  bag->SetShortDesc("a small purse");
  bag->SetDesc("A small, durable, practical moneypurse.");

  bag->SetSkill("Wearable on Left Hip", 1);
  bag->SetSkill("Wearable on Right Hip", 2);
  bag->SetSkill("Container", 5 * 454);
  bag->SetSkill("Capacity", 5);
  bag->SetSkill("Closeable", 1);

  bag->SetWeight(1 * 454);
  bag->SetSize(2);
  bag->SetVolume(1);
  bag->SetValue(100);

  bag->SetPos(POS_LIE);
  mob->AddAct(ACT_WEAR_LHIP, bag);

  if(!gold) init_gold();
  Object *g = new Object(*gold);
  g->SetParent(bag);
  g->SetSkill("Quantity", qty);
  }

MOBType::MOBType(const char *nm, const char *ds, const char *lds,
        int ib,int ibm, int iq,int iqm, int is,int ism,
	int ic,int icm, int ii,int iim, int iw,int iwm,
	int ig,int igm) {
  name = nm;
  desc = ds;
  long_desc = lds;
  b = ib;
  bm = ibm >? 1;
  q = iq;
  qm = iqm >? 1;
  s = is;
  sm = ism >? 1;
  c = ic;
  cm = icm >? 1;
  i = ii;
  im = iim >? 1;
  w = iw;
  wm = iwm >? 1;
  g = ig;
  gm = igm >? 1;

  armed = NULL;
  }

void MOBType::Skill(const char *nm, int percent, int mod) {
  skills[nm] = make_pair(percent, mod);
  }

void MOBType::Arm(WeaponType *weap) {
  armed = weap;
  }

void MOBType::Armor(ArmorType *arm) {
  armor.push_back(arm);
  }

void Object::AddMOB(const MOBType *type) {
  Object *mob = new Object(this);

  mob->Attach(get_mob_mind());
  mob->Activate();
  mob->SetPos(POS_STAND);
  mob->SetAttribute(0, type->b + rand() % type->bm);
  mob->SetAttribute(1, type->q + rand() % type->qm);
  mob->SetAttribute(2, type->s + rand() % type->sm);
  mob->SetAttribute(3, type->c + rand() % type->cm);
  mob->SetAttribute(4, type->i + rand() % type->im);
  mob->SetAttribute(5, type->w + rand() % type->wm);

  typeof(type->skills.begin()) sk_it;
  for(sk_it = type->skills.begin(); sk_it != type->skills.end(); ++sk_it) {
    mob->SetSkill(sk_it->first,
	mob->Attribute(get_linked(sk_it->first))
	* sk_it->second.first / 100
	- rand() % sk_it->second.second
	);
    //fprintf(stderr, "DBG: %d %d %d\n", get_linked(sk_it->first), sk_it->second.first, sk_it->second.second);
    }

  mob->SetShortDesc(type->name.c_str());
  mob->SetDesc(type->desc.c_str());
  mob->SetLongDesc(type->long_desc.c_str());

  give_gold(mob, type->g + rand() % type->gm);

  if(type->armed) {
    Object *obj = new Object(mob);
    obj->SetSkill("WeaponType", type->armed->type);
    obj->SetSkill("WeaponReach", type->armed->reach);
    obj->SetSkill("WeaponForce",
	type->armed->force + rand() % type->armed->forcem);
    obj->SetSkill("WeaponSeverity",
	type->armed->sev + rand() % type->armed->sevm);
    obj->SetShortDesc(type->armed->name.c_str());
    obj->SetDesc(type->armed->desc.c_str());
    obj->SetLongDesc(type->armed->long_desc.c_str());
    obj->SetWeight(type->armed->weight);
    obj->SetVolume(type->armed->volume);
    obj->SetValue(type->armed->value);
    obj->SetPos(POS_LIE);
    mob->AddAct(ACT_WIELD, obj);
    if(two_handed(type->armed->type)) mob->AddAct(ACT_HOLD, obj);
    }

  typeof(type->armor.begin()) ar_it;
  for(ar_it = type->armor.begin(); ar_it != type->armor.end(); ++ar_it) {
    Object *obj = new Object(mob);
    obj->SetAttribute(0, (*ar_it)->bulk + rand() %(*ar_it)->bulkm);
    obj->SetSkill("ArmorB", (*ar_it)->bulk + rand() %(*ar_it)->bulkm);
    obj->SetSkill("ArmorI", (*ar_it)->impact + rand() %(*ar_it)->impactm);
    obj->SetSkill("ArmorT", (*ar_it)->thread + rand() %(*ar_it)->threadm);
    obj->SetSkill("ArmorP", (*ar_it)->planar + rand() %(*ar_it)->planarm);
    obj->SetShortDesc((*ar_it)->name.c_str());
    obj->SetDesc((*ar_it)->desc.c_str());
    obj->SetLongDesc((*ar_it)->long_desc.c_str());
    obj->SetWeight((*ar_it)->weight);
    obj->SetVolume((*ar_it)->volume);
    obj->SetValue((*ar_it)->value);
    obj->SetPos(POS_LIE);

    typeof((*ar_it)->loc.begin()) l_it;
    for(l_it = (*ar_it)->loc.begin(); l_it != (*ar_it)->loc.end(); ++l_it) {
      mob->AddAct(*l_it, obj);
      }
    }
  }

WeaponType::WeaponType(const char *nm, const char *ds, const char *lds,
	const char *t, int r, int f,int fm, int s,int sm,
	int w, int vol, int val) {
  name = nm;
  desc = ds;
  long_desc = lds;
  type = get_weapon_type(t);
  reach = r;
  force = f;
  forcem = fm >? 1;
  sev = s;
  sevm = sm >? 1;
  weight = w;
  volume = vol;
  value = val;
  }

ArmorType::ArmorType(const char *nm, const char *ds, const char *lds,
	int b, int bm, int i, int im, int t, int tm, int p, int pm,
	int w, int vol, int val,
	act_t l1, act_t l2, act_t l3, act_t l4, act_t l5, act_t l6
	) {
  name = nm;
  desc = ds;
  long_desc = lds;

  bulk = b;
  bulkm = bm >? 1;
  impact = i;
  impactm = im >? 1;
  thread = t;
  threadm = tm >? 1;
  planar = p;
  planarm = pm >? 1;

  if(l1 != ACT_NONE) loc.push_back(l1);
  if(l2 != ACT_NONE) loc.push_back(l2);
  if(l3 != ACT_NONE) loc.push_back(l3);
  if(l4 != ACT_NONE) loc.push_back(l4);
  if(l5 != ACT_NONE) loc.push_back(l5);
  if(l6 != ACT_NONE) loc.push_back(l6);

  weight = w;
  volume = vol;
  value = val;
  }
