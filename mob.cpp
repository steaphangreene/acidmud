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
  }

void MOBType::Skill(const char *nm, int percent, int mod) {
  skills[nm] = make_pair(percent, mod);
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
	sk_it->second.first - rand() % sk_it->second.second);
    }

  mob->SetShortDesc(type->name.c_str());
  mob->SetDesc(type->desc.c_str());
  mob->SetLongDesc(type->long_desc.c_str());

  give_gold(mob, type->g + rand() % type->gm);

/*
  Object *obj = new Object(mob);
  obj->SetSkill("WeaponType", get_weapon_type("Two-Handed Cleaves"));
  obj->SetSkill("WeaponForce", 2 + rand()%7);
  obj->SetSkill("WeaponSeverity", 3 + rand()%3);
  obj->SetSkill("WeaponReach", 2);
  obj->SetShortDesc("a dwarven mining pickaxe");
  obj->SetDesc("A super-strong, super-sharp, super-heavy pickaxe.");
  obj->SetWeight(20000);
  obj->SetVolume(50);
  obj->SetValue(2000);
  obj->SetPos(POS_LIE);
  mob->AddAct(ACT_WIELD, obj);
  mob->AddAct(ACT_HOLD, obj);
*/
  }
