#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <cctype>
#include <string>

#include "commands.h"
#include "object.h"
#include "color.h"
#include "mind.h"
#include "mob.h"

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
  static MOBType *dwarf_miner;
  static MOBType *dwarf_engineer;
  static MOBType *dwarf_guard;
  static MOBType *dwarf_explorer;
  if(!dwarf_miner) {
    WeaponType *weap;
    ArmorType *arm;

    dwarf_miner = new MOBType("a dwarf miner", "He looks pissed.", "",
		7,7, 4,5, 6,7, 2,2, 4,3, 8,7, 500,2001);
    dwarf_miner->Skill("Two-Handed Cleaves", 100, 4);
    weap = new WeaponType("a dwarven mining pickaxe",
	"A super-strong, super-sharp, super-heavy pickaxe.", "",
	"Two-Handed Cleaves", 2, 2,7, 3,3, 20000, 50, 2000
	);
    dwarf_miner->Arm(weap);
    arm = new ArmorType("a dwarven leather jerkin",
	"A heavy dwarven leather jerkin.  It'll probably stop an arrow.", "",
	3, 2, 2, 2, 0, 0, 1, 0, 10000, 10, 150,
	ACT_WEAR_CHEST, ACT_WEAR_BACK
	);
    dwarf_miner->Armor(arm);


    dwarf_engineer = new MOBType("a dwarf engineer", "He looks pissed.", "",
		5,7, 4,5, 5,7, 3,2, 5,3, 8,7, 2000,8001);
    dwarf_engineer->Skill("Long Cleaves", 100, 2);
    weap = new WeaponType("a dwarven combat axe",
	"A super-strong, super-sharp combat axe.", "",
	"Long Cleaves", 1, 2,7, 2,2, 4000, 10, 1000
	);
    dwarf_engineer->Arm(weap);
    arm = new ArmorType("a dwarven leather jerkin",
	"A heavy dwarven leather jerkin.  It'll probably stop an arrow.", "",
	3, 2, 2, 2, 0, 0, 1, 0, 10000, 10, 150,
	ACT_WEAR_CHEST, ACT_WEAR_BACK
	);
    dwarf_engineer->Armor(arm);

    dwarf_guard = new MOBType("a dwarf guard", "He looks pissed.", "",
		9,4, 6,4, 9,4, 1,3, 5,4, 9,4, 100,401);
    dwarf_guard->Skill("Two-Handed Cleaves", 100, 4);
    weap = new WeaponType("a dwarven war axe",
	"A super-strong, super-sharp, super-heavy, high-quality war axe.", "",
	"Two-Handed Cleaves", 2, 4,6, 4,2, 20000, 40, 5000
	);
    dwarf_guard->Arm(weap);
    arm = new ArmorType("a dwarven heavy breastplate",
	"A heavy dwarven breastplate.  It'll probably stop a warhammer.", "",
	8, 4, 6, 2, 4, 2, 10, 5, 200000, 100, 15000,
	ACT_WEAR_CHEST, ACT_WEAR_BACK
	);
    dwarf_guard->Armor(arm);
    arm = new ArmorType("a dwarven great helm",
	"A heavy dwarven great helm.  It'll probably stop a warhammer.", "",
	8, 4, 6, 2, 4, 2, 10, 5, 50000, 70, 5000,
	ACT_WEAR_HEAD
	);
    dwarf_guard->Armor(arm);
    arm = new ArmorType("a dwarven arm plate (right)",
	"A heavy dwarven arm plate.  It'll probably stop a warhammer.", "",
	8, 4, 6, 2, 4, 2, 10, 5, 50000, 60, 5000,
	ACT_WEAR_RARM
	);
    dwarf_guard->Armor(arm);
    arm = new ArmorType("a dwarven arm plate (left)",
	"A heavy dwarven arm plate.  It'll probably stop a warhammer.", "",
	8, 4, 6, 2, 4, 2, 10, 5, 50000, 60, 5000,
	ACT_WEAR_LARM
	);
    dwarf_guard->Armor(arm);
    arm = new ArmorType("a dwarven battle skirt",
	"A heavy dwarven battle skirt.  It'll probably stop a warhammer.", "",
	8, 4, 6, 2, 4, 2, 10, 5, 100000, 80, 10000,
	ACT_WEAR_RLEG, ACT_WEAR_LLEG
	);
    dwarf_guard->Armor(arm);

    dwarf_explorer = new MOBType("a dwarf explorer", "He looks pissed.", "",
		5,4, 5,4, 6,4, 3,2, 6,4, 9,5, 1000,4001);
    dwarf_explorer->Skill("Long Cleaves", 100, 4);
    weap = new WeaponType("a dwarven climbing pick",
	"A super-sharp, lightweight pick.", "",
	"Long Cleaves", 1, 1,3, 3,1, 2000, 10, 500
	);
    dwarf_explorer->Arm(weap);
    arm = new ArmorType("a dwarven leather jerkin",
	"A heavy dwarven leather jerkin.  It'll probably stop an arrow.", "",
	3, 2, 2, 2, 0, 0, 1, 0, 10000, 10, 150,
	ACT_WEAR_CHEST, ACT_WEAR_BACK
	);
    dwarf_explorer->Armor(arm);
    }

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
      if((rand()%100) < 30) { mojo -= 500; AddMOB(dwarf_miner); }
      if((rand()%100) < 10) { mojo -= 500; AddMOB(dwarf_engineer); }
      if((rand()%100) < 20) { mojo -= 500; AddMOB(dwarf_guard); }
      if((rand()%100) < 20) { mojo -= 500; AddMOB(dwarf_guard); }

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
      if((rand()%100) < 30) { mojo -= 500; AddMOB(dwarf_miner); }
      if((rand()%100) < 10) { mojo -= 500; AddMOB(dwarf_engineer); }
      if((rand()%100) < 20) { mojo -= 500; AddMOB(dwarf_guard); }
      if((rand()%100) < 20) { mojo -= 500; AddMOB(dwarf_guard); }

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
      if((rand()%100) < 30) { mojo -= 500; AddMOB(dwarf_miner); }
      if((rand()%100) < 10) { mojo -= 500; AddMOB(dwarf_engineer); }
      if((rand()%100) < 20) { mojo -= 500; AddMOB(dwarf_guard); }
      if((rand()%100) < 20) { mojo -= 500; AddMOB(dwarf_guard); }

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
      if((rand()%100) < 30) { mojo -= 500; AddMOB(dwarf_miner); }
      if((rand()%100) < 30) { mojo -= 500; AddMOB(dwarf_miner); }
      if((rand()%100) < 10) { mojo -= 500; AddMOB(dwarf_engineer); }
      if((rand()%100) < 20) { mojo -= 500; AddMOB(dwarf_explorer); }

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
      if((rand()%100) < 30) { mojo -= 500; AddMOB(dwarf_miner); }
      if((rand()%100) < 30) { mojo -= 500; AddMOB(dwarf_miner); }
      if((rand()%100) < 10) { mojo -= 500; AddMOB(dwarf_engineer); }
      if((rand()%100) < 20) { mojo -= 500; AddMOB(dwarf_explorer); }

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
      if((rand()%100) < 30) { mojo -= 500; AddMOB(dwarf_miner); }
      if((rand()%100) < 30) { mojo -= 500; AddMOB(dwarf_miner); }
      if((rand()%100) < 10) { mojo -= 500; AddMOB(dwarf_engineer); }
      if((rand()%100) < 20) { mojo -= 500; AddMOB(dwarf_explorer); }

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
