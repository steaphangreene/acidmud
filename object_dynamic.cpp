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

void Object::DynamicInit() {
  switch(Skill("DynamicInit")) {
    case(1): {
      DynamicInit1();
      }break;
    default: {
      fprintf(stderr, "Unknown dynamic-type (%d) init requested!\n",
	Skill("DynamicInit"));
      }break;
    }
  SetSkill("DynamicInit", 0);
  }

static void make_dwarf_miner(Object *loc) {
  Object *mob = new Object(loc);

  mob->Attach(get_mob_mind());
  mob->Activate();
  mob->SetPos(POS_STAND);
  mob->SetAttribute(0, 7+rand()%7);
  mob->SetAttribute(1, 4+rand()%5);
  mob->SetAttribute(2, 6+rand()%7);
  mob->SetAttribute(3, 2+rand()%2);
  mob->SetAttribute(4, 4+rand()%3);
  mob->SetAttribute(5, 8+rand()%7);
  mob->SetSkill("Two-Handed Cleaves", mob->Attribute(2) - rand()%4);

  mob->SetShortDesc("a dwarf miner");
  mob->SetDesc("He looks pissed.");

  mob->SetSkill("CircleGold", 500+rand()%2001);
  loc->CircleFinishMob(mob);

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
  }


void Object::DynamicInit9() {
  }

void Object::DynamicInit8() {
  }

void Object::DynamicInit7() {
  }

void Object::DynamicInit6() {
  }

void Object::DynamicInit5() {
  }

void Object::DynamicInit4() {
  }

void Object::DynamicInit3() {
  }

void Object::DynamicInit2() {
  }

void Object::DynamicInit1() {		//Dwarven mine
  int mojo = Skill("DynamicMojo");
  SetSkill("DynamicMojo", 0);

  //Tree structure, start with one door - dir of travel is opposite it.
  char *dirb="south", *dir="north", *dir2="west", *dir3="east";

  if(PickObject("north", LOC_INTERNAL) != NULL) {
    dirb = "north"; dir = "south"; dir2 = "east"; dir3 = "west";
    }
  else if(PickObject("east", LOC_INTERNAL) != NULL) {
    dirb = "east"; dir = "west"; dir2 = "south"; dir3 = "north";
    }
  else if(PickObject("west", LOC_INTERNAL) != NULL) {
    dirb = "west"; dir = "east"; dir2 = "north"; dir3 = "south";
    }
  //Up and Down shafts are handled specially.


  switch(Skill("DynamicPhase")) {
    case(0): { //Entrance
      if(mojo == 0) mojo = 100000;

      Object *next = new Object(parent);
      next->SetShortDesc("A Large Mining Tunnel");
      next->SetDesc(
	"This tunnel looks to have been carved centuries ago.  It is so well crafted\n"
	"that you think it will stand as-is for another millenia.\n");
      next->SetSkill("DynamicInit", 1);
      next->SetSkill("DynamicPhase", 1); //Major Shaft
      next->SetSkill("DynamicMojo", mojo-1000);

      Object *door1 = new Object(this);
      Object *door2 = new Object(next);
      door1->SetShortDesc(dir);
      door2->SetShortDesc(dirb);
      door1->SetDesc((string("You see a solid passage leading ") + dir + ".\n").c_str());
      door2->SetDesc((string("You see a solid passage leading ") + dirb + ".\n").c_str());
      door1->AddAct(ACT_SPECIAL_LINKED, door2);
      door1->AddAct(ACT_SPECIAL_MASTER, door2);
      door1->SetSkill("Open", 1);
      door1->SetSkill("Enterable", 1);
      door2->AddAct(ACT_SPECIAL_LINKED, door1);
      door2->AddAct(ACT_SPECIAL_MASTER, door1);
      door2->SetSkill("Open", 1);
      door2->SetSkill("Enterable", 1);

      }break;
    case(1): { //Major Shaft
      if((rand()%100) < 30) { mojo -= 500; make_dwarf_miner(this); }

      if(mojo <= 0) break; //End of Tunnel

//      int ntypes[] = { 1, 1, 1, 1, 1, 1, 1, 2, 2, 3, 4 };
      int ntypes[] = { 1, 1, 1, 1, 1, 1, 1, 2, 2, 3 };
      int ntype = ntypes[rand() % (sizeof(ntypes)/sizeof(int))];

      Object *next = new Object(parent);
      next->SetShortDesc("A Large Mining Tunnel");
      next->SetDesc(
	"This tunnel looks to have been carved centuries ago.  It is so well crafted\n"
	"that you think it will stand as-is for another millenia.\n");
      next->SetSkill("DynamicInit", 1);
      next->SetSkill("DynamicPhase", ntype);
      next->SetSkill("DynamicMojo", mojo-1000);

      Object *door1 = new Object(this);
      Object *door2 = new Object(next);
      door1->SetShortDesc(dir);
      door2->SetShortDesc(dirb);
      door1->SetDesc((string("You see a solid passage leading ") + dir + ".\n").c_str());
      door2->SetDesc((string("You see a solid passage leading ") + dirb + ".\n").c_str());
      door1->AddAct(ACT_SPECIAL_LINKED, door2);
      door1->AddAct(ACT_SPECIAL_MASTER, door2);
      door1->SetSkill("Open", 1);
      door1->SetSkill("Enterable", 1);
      door2->AddAct(ACT_SPECIAL_LINKED, door1);
      door2->AddAct(ACT_SPECIAL_MASTER, door1);
      door2->SetSkill("Open", 1);
      door2->SetSkill("Enterable", 1);
      }break;
    case(2): { //Major Shaft w/ Minor Offshoot
      if((rand()%100) < 30) { mojo -= 500; make_dwarf_miner(this); }

      if(mojo <= 0) break; //End of Tunnel

//      int ntypes[] = { 1, 1, 1, 1, 1, 1, 1, 2, 2, 3, 4 };
      int ntypes[] = { 1, 1, 1, 1, 1, 1, 1, 2, 2, 3 };
      int ntype = ntypes[rand() % (sizeof(ntypes)/sizeof(int))];

      Object *next = new Object(parent);
      next->SetShortDesc("A Large Mining Tunnel");
      next->SetDesc(
	"This tunnel looks to have been carved centuries ago.  It is so well crafted\n"
	"that you think it will stand as-is for another millenia.\n");
      next->SetSkill("DynamicInit", 1);
      next->SetSkill("DynamicPhase", ntype);
      next->SetSkill("DynamicMojo", (mojo-1000) * 9 / 10);

      Object *door1 = new Object(this);
      Object *door2 = new Object(next);
      door1->SetShortDesc(dir);
      door2->SetShortDesc(dirb);
      door1->SetDesc((string("You see a solid passage leading ") + dir + ".\n").c_str());
      door2->SetDesc((string("You see a solid passage leading ") + dirb + ".\n").c_str());
      door1->AddAct(ACT_SPECIAL_LINKED, door2);
      door1->AddAct(ACT_SPECIAL_MASTER, door2);
      door1->SetSkill("Open", 1);
      door1->SetSkill("Enterable", 1);
      door2->AddAct(ACT_SPECIAL_LINKED, door1);
      door2->AddAct(ACT_SPECIAL_MASTER, door1);
      door2->SetSkill("Open", 1);
      door2->SetSkill("Enterable", 1);

      if(rand()%2) swap(dir2, dir3);	// Half left, half right
      next = new Object(parent);
      next->SetShortDesc("A Small Mining Tunnel");
      next->SetDesc(
	"This tunnel looks to have been carved quickly.  It looks like it might\n"
	"collapse at any moment.\n");
      next->SetSkill("DynamicInit", 1);
      next->SetSkill("DynamicPhase", 5); //Minor Shaft
      next->SetSkill("DynamicMojo", (mojo-1000) / 10);

      door1 = new Object(this);
      door2 = new Object(next);
      door1->SetShortDesc(dir2);
      door2->SetShortDesc(dir3);
      door1->SetDesc((string("You see a crumbling passage leading ") + dir2 + ".\n").c_str());
      door2->SetDesc((string("You see a crumbling passage leading ") + dir3 + ".\n").c_str());
      door1->AddAct(ACT_SPECIAL_LINKED, door2);
      door1->AddAct(ACT_SPECIAL_MASTER, door2);
      door1->SetSkill("Open", 1);
      door1->SetSkill("Enterable", 1);
      door2->AddAct(ACT_SPECIAL_LINKED, door1);
      door2->AddAct(ACT_SPECIAL_MASTER, door1);
      door2->SetSkill("Open", 1);
      door2->SetSkill("Enterable", 1);
      }break;
    case(3): { //Major Shaft w/ Secret Minor Offshoot
      if((rand()%100) < 30) { mojo -= 500; make_dwarf_miner(this); }

      if(mojo <= 0) break; //End of Tunnel

//      int ntypes[] = { 1, 1, 1, 1, 1, 1, 1, 2, 2, 3, 4 };
      int ntypes[] = { 1, 1, 1, 1, 1, 1, 1, 2, 2, 3 };
      int ntype = ntypes[rand() % (sizeof(ntypes)/sizeof(int))];

      Object *next = new Object(parent);
      next->SetShortDesc("A Large Mining Tunnel");
      next->SetDesc(
	"This tunnel looks to have been carved centuries ago.  It is so well crafted\n"
	"that you think it will stand as-is for another millenia.\n");
      next->SetSkill("DynamicInit", 1);
      next->SetSkill("DynamicPhase", ntype);
      next->SetSkill("DynamicMojo", (mojo-1000) * 3 / 4);

      Object *door1 = new Object(this);
      Object *door2 = new Object(next);
      door1->SetShortDesc(dir);
      door2->SetShortDesc(dirb);
      door1->SetDesc((string("You see a solid passage leading ") + dir + ".\n").c_str());
      door2->SetDesc((string("You see a solid passage leading ") + dirb + ".\n").c_str());
      door1->AddAct(ACT_SPECIAL_LINKED, door2);
      door1->AddAct(ACT_SPECIAL_MASTER, door2);
      door1->SetSkill("Open", 1);
      door1->SetSkill("Enterable", 1);
      door2->AddAct(ACT_SPECIAL_LINKED, door1);
      door2->AddAct(ACT_SPECIAL_MASTER, door1);
      door2->SetSkill("Open", 1);
      door2->SetSkill("Enterable", 1);

      if(rand()%2) swap(dir2, dir3);	// Half left, half right
      next = new Object(parent);
      next->SetShortDesc("A Small Mining Tunnel");
      next->SetDesc(
	"This tunnel looks to have been carved quickly.  It looks like it might\n"
	"collapse at any moment.\n");
      next->SetSkill("DynamicInit", 1);
      next->SetSkill("DynamicPhase", 5); //Minor Shaft
      next->SetSkill("DynamicMojo", (mojo-1000) / 4);

      door1 = new Object(this);
      door2 = new Object(next);
      door1->SetShortDesc(dir2);
      door2->SetShortDesc(dir3);
      door1->SetDesc((string("You see a crumbling passage leading ") + dir2 + ".\n").c_str());
      door2->SetDesc((string("You see a crumbling passage leading ") + dir3 + ".\n").c_str());
      door1->AddAct(ACT_SPECIAL_LINKED, door2);
      door1->AddAct(ACT_SPECIAL_MASTER, door2);
      door1->SetSkill("Open", 1);
      door1->SetSkill("Enterable", 1);
      door2->AddAct(ACT_SPECIAL_LINKED, door1);
      door2->AddAct(ACT_SPECIAL_MASTER, door1);
      door2->SetSkill("Open", 1);
      door2->SetSkill("Enterable", 1);
      door1->SetSkill("Hidden", 4 + rand()%13);
      }break;
    case(5): { //Minor Shaft
      if((rand()%100) < 30) { mojo -= 500; make_dwarf_miner(this); }

      if(mojo <= 0) break; //End of Tunnel

//      int ntypes[] = { 5, 5, 5, 5, 6, 6, 6, 7, 7, 8, 9 };
      int ntypes[] = { 5, 5, 5, 5, 5, 6, 6, 7, 7 };
      int ntype = ntypes[rand() % (sizeof(ntypes)/sizeof(int))];

      Object *next = new Object(parent);
      next->SetShortDesc("A Small Mining Tunnel");
      next->SetDesc(
	"This tunnel looks to have been carved quickly.  It looks like it might\n"
	"collapse at any moment.\n");
      next->SetSkill("DynamicInit", 1);
      next->SetSkill("DynamicPhase", ntype);
      next->SetSkill("DynamicMojo", mojo-100);

      Object *door1 = new Object(this);
      Object *door2 = new Object(next);
      door1->SetShortDesc(dir);
      door2->SetShortDesc(dirb);
      door1->SetDesc((string("You see a crumbling passage leading ") + dir + ".\n").c_str());
      door2->SetDesc((string("You see a crumbling passage leading ") + dirb + ".\n").c_str());
      door1->AddAct(ACT_SPECIAL_LINKED, door2);
      door1->AddAct(ACT_SPECIAL_MASTER, door2);
      door1->SetSkill("Open", 1);
      door1->SetSkill("Enterable", 1);
      door2->AddAct(ACT_SPECIAL_LINKED, door1);
      door2->AddAct(ACT_SPECIAL_MASTER, door1);
      door2->SetSkill("Open", 1);
      door2->SetSkill("Enterable", 1);
      }break;
    case(6): { //Minor Shaft (Bend)
      if((rand()%100) < 30) { mojo -= 500; make_dwarf_miner(this); }

//      int ntypes[] = { 5, 5, 5, 5, 5, 6, 6, 7, 7, 8, 9 };
      int ntypes[] = { 5, 5, 5, 5, 5, 6, 6, 7, 7 };
      int ntype = ntypes[rand() % (sizeof(ntypes)/sizeof(int))];

      if(rand()%2) swap(dir2, dir3);	// Half left, half right
      Object *next = new Object(parent);
      next->SetShortDesc("A Small Mining Tunnel");
      next->SetDesc(
	"This tunnel looks to have been carved quickly.  It looks like it might\n"
	"collapse at any moment.\n");
      next->SetSkill("DynamicInit", 1);
      next->SetSkill("DynamicPhase", ntype);
      next->SetSkill("DynamicMojo", mojo-100);

      Object *door1 = new Object(this);
      Object *door2 = new Object(next);
      door1->SetShortDesc(dir2);
      door2->SetShortDesc(dir3);
      door1->SetDesc((string("You see a crumbling passage leading ") + dir2 + ".\n").c_str());
      door2->SetDesc((string("You see a crumbling passage leading ") + dir3 + ".\n").c_str());
      door1->AddAct(ACT_SPECIAL_LINKED, door2);
      door1->AddAct(ACT_SPECIAL_MASTER, door2);
      door1->SetSkill("Open", 1);
      door1->SetSkill("Enterable", 1);
      door2->AddAct(ACT_SPECIAL_LINKED, door1);
      door2->AddAct(ACT_SPECIAL_MASTER, door1);
      door2->SetSkill("Open", 1);
      door2->SetSkill("Enterable", 1);
      }break;
    case(7): { //Minor Shaft Fork
      if((rand()%100) < 30) { mojo -= 500; make_dwarf_miner(this); }

//      int ntypes[] = { 5, 5, 5, 5, 5, 6, 6, 7, 7, 8, 9 };
      int ntypes[] = { 5, 5, 5, 5, 5, 6, 6, 7, 7 };
      int ntype = ntypes[rand() % (sizeof(ntypes)/sizeof(int))];

      Object *next = new Object(parent);
      next->SetShortDesc("A Small Mining Tunnel");
      next->SetDesc(
	"This tunnel looks to have been carved quickly.  It looks like it might\n"
	"collapse at any moment.\n");
      next->SetSkill("DynamicInit", 1);
      next->SetSkill("DynamicPhase", ntype);
      next->SetSkill("DynamicMojo", (mojo-100)/2);

      Object *door1 = new Object(this);
      Object *door2 = new Object(next);
      door1->SetShortDesc(dir2);
      door2->SetShortDesc(dir3);
      door1->SetDesc((string("You see a crumbling passage leading ") + dir2 + ".\n").c_str());
      door2->SetDesc((string("You see a crumbling passage leading ") + dir3 + ".\n").c_str());
      door1->AddAct(ACT_SPECIAL_LINKED, door2);
      door1->AddAct(ACT_SPECIAL_MASTER, door2);
      door1->SetSkill("Open", 1);
      door1->SetSkill("Enterable", 1);
      door2->AddAct(ACT_SPECIAL_LINKED, door1);
      door2->AddAct(ACT_SPECIAL_MASTER, door1);
      door2->SetSkill("Open", 1);
      door2->SetSkill("Enterable", 1);

      ntype = ntypes[rand() % (sizeof(ntypes)/sizeof(int))];
      swap(dir2, dir3);		// Same code, opposite dir.
      next = new Object(parent);
      next->SetShortDesc("A Small Mining Tunnel");
      next->SetDesc(
	"This tunnel looks to have been carved quickly.  It looks like it might\n"
	"collapse at any moment.\n");
      next->SetSkill("DynamicInit", 1);
      next->SetSkill("DynamicPhase", ntype);
      next->SetSkill("DynamicMojo", (mojo-100)/2);

      door1 = new Object(this);
      door2 = new Object(next);
      door1->SetShortDesc(dir2);
      door2->SetShortDesc(dir3);
      door1->SetDesc((string("You see a crumbling passage leading ") + dir2 + ".\n").c_str());
      door2->SetDesc((string("You see a crumbling passage leading ") + dir3 + ".\n").c_str());
      door1->AddAct(ACT_SPECIAL_LINKED, door2);
      door1->AddAct(ACT_SPECIAL_MASTER, door2);
      door1->SetSkill("Open", 1);
      door1->SetSkill("Enterable", 1);
      door2->AddAct(ACT_SPECIAL_LINKED, door1);
      door2->AddAct(ACT_SPECIAL_MASTER, door1);
      door2->SetSkill("Open", 1);
      door2->SetSkill("Enterable", 1);
      }break;
    default: {
      fprintf(stderr, "Unknown dynamic-phase-type (%d-%d) init requested!\n",
	Skill("DynamicInit"), Skill("DynamicPhase"));
      }break;
    }
  SetSkill("DynamicPhase", 0);
  }
