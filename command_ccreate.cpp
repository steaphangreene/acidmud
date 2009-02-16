#include <cstring>

using namespace std;

#include "mind.h"
#include "object.h"

#define NUM_AVS	5	// 28 Max!
#define NUM_STS	5	// No Max
int handle_command_ccreate(Object *body, Mind *mind, const char *comline,
	int len, int stealth_t, int stealth_s) {
    if(comline[len] == 0) {
      mind->Send("You need to specify the name of the city!\n");
      }
    else {
      Object *city = new Object(body->Parent());
      city->SetShortDesc(comline+len);

//      Object *ocean = new Object(city);
//      ocean->SetShortDesc("Ocean");
//      ocean->SetDesc("The Atcific Ocean.");

//      Object *bay = new Object(city);
//      bay->SetShortDesc("Bay");
//      bay->SetDesc((string("The ") + (comline+len) + " Bay.").c_str());

//      Object *inlet = new Object(city);
//      inlet->SetShortDesc("Inlet");
//      inlet->SetDesc((string("The ") + (comline+len) + " Inlet.").c_str());

//      Object *wall = new Object(city);
//      wall->SetShortDesc("North Wall");
//      wall->SetDesc("The North Wall.");

      char alist[][8] = {
	"West", "Apple", "Breeze", "Coconut", "Drury", "Earl",
	"Farley", "Gantry", "Henry", "Indian", "Jawa", "Kindle",
	"Lucky", "Moody", "Neddle", "Orion", "Puddle", "Quaint",
	"Rocky", "Stone", "True", "Uber", "Violet", "Widget",
	"X-Ray", "Yeller", "Zebra", ""
	};
      sprintf(alist[NUM_AVS - 1], "East");

      char sname[32], aname[32], iname[32];
      Object *ave[NUM_AVS] = {NULL};
      for(int north = 0; north < NUM_STS; ++north) {
	if(north == 0) sprintf(sname, "South Street");
	else if(north == NUM_STS - 1) sprintf(sname, "North Street");
	else if(north % 10 == 1) sprintf(sname, "%dst Street", north);
	else if(north % 10 == 2) sprintf(sname, "%dnd Street", north);
	else if(north % 10 == 3) sprintf(sname, "%drd Street", north);
	else sprintf(sname, "%dth Street", north);

	Object *street = NULL;
	for(int east = 0; east < NUM_AVS; ++east) {
	  sprintf(aname, "%s Avenue", alist[east]);
	  sprintf(iname, "%s and %s", alist[east], sname);
	  iname[strlen(iname) - 7] = 0;

	  for(int off=-10; off <= 100; off += 2) {
	    if(off == 0) off += 2;	// Skip first zero

	    if(off == 100) off = 0;	// Create a second zero

	    if(off < 0 && north < 1) continue;
	    if(off > 0 && east < 1) continue;

	    Object *cur = new Object(city);
	    if(off == 0) {
	      cur->SetShortDesc(iname);
	      cur->SetDesc("A busy intersection.\n");
	      }
	    else if(off > 0) {
	      cur->SetShortDesc(sname);
	      cur->SetDesc("A nice street, running east and west.\n");
	      }
	    else /* if(off < 0) */ {
	      cur->SetShortDesc(aname);
	      cur->SetDesc("A nice avenue, running north and south.\n");
	      }

	    if(off >= 0) {
	      if(street) {
		Object *door1 = new Object(cur);
		Object *door2 = new Object(street);
		door1->SetShortDesc("west");
		door2->SetShortDesc("east");
		door1->SetDesc("The street continues west.\n");
		door2->SetDesc("The street continues east.\n");
		door1->AddAct(ACT_SPECIAL_LINKED, door2);
		door1->AddAct(ACT_SPECIAL_MASTER, door2);
		door1->SetSkill("Open", 1);
		door1->SetSkill("Enterable", 1);
		door2->AddAct(ACT_SPECIAL_LINKED, door1);
		door2->AddAct(ACT_SPECIAL_MASTER, door1);
		door2->SetSkill("Open", 1);
		door2->SetSkill("Enterable", 1);
		}
	      street = cur;
	      }
	    if(off <= 0) {
	      if(ave[east]) {
		Object *door1 = new Object(cur);
		Object *door2 = new Object(ave[east]);
		door1->SetShortDesc("south");
		door2->SetShortDesc("north");
		door1->SetDesc("The avenue continues south.\n");
		door2->SetDesc("The avenue continues north.\n");
		door1->AddAct(ACT_SPECIAL_LINKED, door2);
		door1->AddAct(ACT_SPECIAL_MASTER, door2);
		door1->SetSkill("Open", 1);
		door1->SetSkill("Enterable", 1);
		door2->AddAct(ACT_SPECIAL_LINKED, door1);
		door2->AddAct(ACT_SPECIAL_MASTER, door1);
		door2->SetSkill("Open", 1);
		door2->SetSkill("Enterable", 1);
		}
	      ave[east] = cur;
	      }

	    if(off != 0) {
	      Object *places[3] = {NULL, cur, NULL};
	      const char *dir[2] = {"south", "north"};
	      if(off < 0) {
		dir[0] = "west"; dir[1] = "east";
		}
	      for(int i = 0; i < 2; ++i) {
		char addr[32];
		if(off > 0) {
		  sprintf(addr, "%d %s", (east+1)*100 + off + i, sname);
		  }
		else {
		  sprintf(addr, "%d %s", (north+1)*10 + 10 + off + i, aname);
		  }
		places[i*2] = new Object(city);
		places[i*2]->SetShortDesc("Vacant Lot");
		places[i*2]->SetSkill("DynamicInit", 2);  //City
		places[i*2]->SetSkill("DynamicPhase", 0); //Lot
		places[i*2]->SetSkill("DynamicMojo", 1000);

		Object *door1 = new Object(places[i+1]);
		Object *door2 = new Object(places[i]);
		door1->SetShortDesc(dir[0]);
		door2->SetShortDesc(dir[1]);
		door1->SetDesc(addr);
		door2->SetDesc(addr);
		door1->AddAct(ACT_SPECIAL_LINKED, door2);
		door1->AddAct(ACT_SPECIAL_MASTER, door2);
		door1->SetSkill("Open", 1);
		door1->SetSkill("Enterable", 1);
		door2->AddAct(ACT_SPECIAL_LINKED, door1);
		door2->AddAct(ACT_SPECIAL_MASTER, door1);
		door2->SetSkill("Open", 1);
		door2->SetSkill("Enterable", 1);
		}
	      }

	    if(off == 0) off = 100;	// Zero really was 100
	    }
	  }
	}
      body->Parent()->SendOut(stealth_t, stealth_s, 
	";s creates a new city '%s' with Ninja Powers[TM].\n",
	"You create a new city '%s'.\n", body, NULL, comline+len);
      }
    return 0;
    }
