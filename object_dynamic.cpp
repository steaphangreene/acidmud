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
      if(mojo < 1000) break; //End of Tunnel

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
    }
  SetSkill("DynamicPhase", 0);
  }
