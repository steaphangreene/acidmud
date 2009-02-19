#include <cstring>
#include <cstdlib>
#include <algorithm>

using namespace std;

#include "mind.h"
#include "object.h"
#include "commands.h"

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

//    Object *ocean = new Object(city);
//    ocean->SetShortDesc("Ocean");
//    ocean->SetDesc("The Atcific Ocean.");

//    Object *bay = new Object(city);
//    bay->SetShortDesc("Bay");
//    bay->SetDesc((string("The ") + (comline+len) + " Bay.").c_str());

//    Object *inlet = new Object(city);
//    inlet->SetShortDesc("Inlet");
//    inlet->SetDesc((string("The ") + (comline+len) + " Inlet.").c_str());

//    Object *wall = new Object(city);
//    wall->SetShortDesc("North Wall");
//    wall->SetDesc("The North Wall.");

    char alist[][8] = {
	"West", "Apple", "Breeze", "Coconut", "Drury", "Earl",
	"Farley", "Gantry", "Henry", "Indian", "Jawa", "Kindle",
	"Lucky", "Moody", "Neddle", "Orion", "Puddle", "Quaint",
	"Rocky", "Stone", "True", "Uber", "Violet", "Widget",
	"X-Ray", "Yeller", "Zebra", ""
	};
    sprintf(alist[NUM_AVS - 1], "East");

    vector<Object *> bldg;

    bldg.push_back(new Object(city));
    bldg.back()->SetShortDesc("Municipal Building");
    bldg.back()->SetSkill("DynamicInit", 2);	//City
    bldg.back()->SetSkill("DynamicPhase", 10);	//Courthouse
    bldg.back()->SetSkill("DynamicMojo", 1000);
    for(int n=0; n<5; ++n) {
      bldg.push_back(new Object(city));
      bldg.back()->SetShortDesc("Fire Station");
      bldg.back()->SetSkill("DynamicInit", 2);	//City
      bldg.back()->SetSkill("DynamicPhase", 20);	//Fire House
      bldg.back()->SetSkill("DynamicMojo", 1000);
      bldg.back()->SetSkill("Healing", 100);
      }
    for(int n=0; n<8; ++n) {
      bldg.push_back(new Object(city));
      bldg.back()->SetShortDesc("Police Station");
      bldg.back()->SetSkill("DynamicInit", 2);	//City
      bldg.back()->SetSkill("DynamicPhase", 30);	//Police Station
      bldg.back()->SetSkill("DynamicMojo", 1000);
      bldg.back()->SetSkill("Healing", 100);
      }
    for(int n=0; n<4; ++n) {
      bldg.push_back(new Object(city));
      bldg.back()->SetShortDesc("Hospital");
      bldg.back()->SetSkill("DynamicInit", 2);	//City
      bldg.back()->SetSkill("DynamicPhase", 40);	//City Hospital
      bldg.back()->SetSkill("DynamicMojo", 1000);
      bldg.back()->SetSkill("Healing", 1000);
      }
    for(int n=0; n<25; ++n) {
      bldg.push_back(new Object(city));
      bldg.back()->SetShortDesc("Coffin Motel");
      bldg.back()->SetSkill("DynamicInit", 2);	//City
      bldg.back()->SetSkill("DynamicPhase", 50);	//Coffin Motel
      bldg.back()->SetSkill("DynamicMojo", 1000);
      bldg.back()->SetSkill("Rest", 1000);
      }
    for(int n=0; n<12; ++n) {
      bldg.push_back(new Object(city));
      bldg.back()->SetShortDesc("Seedy Motel");
      bldg.back()->SetSkill("DynamicInit", 2);	//City
      bldg.back()->SetSkill("DynamicPhase", 60);	//Seedy Motel
      bldg.back()->SetSkill("DynamicMojo", 1000);
      bldg.back()->SetSkill("Rest", 500);
      }
    for(int n=0; n<8; ++n) {
      bldg.push_back(new Object(city));
      bldg.back()->SetShortDesc("Nice Motel");
      bldg.back()->SetSkill("DynamicInit", 2);	//City
      bldg.back()->SetSkill("DynamicPhase", 70);	//Nice Motel
      bldg.back()->SetSkill("DynamicMojo", 1000);
      bldg.back()->SetSkill("Rest", 200);
      }
    for(int n=0; n<5; ++n) {
      bldg.push_back(new Object(city));
      bldg.back()->SetShortDesc("Nice Hotel");
      bldg.back()->SetSkill("DynamicInit", 2);	//City
      bldg.back()->SetSkill("DynamicPhase", 80);	//Nice Hotel
      bldg.back()->SetSkill("DynamicMojo", 1000);
      bldg.back()->SetSkill("Rest", 100);
      }
    for(int n=0; n<4; ++n) {
      bldg.push_back(new Object(city));
      bldg.back()->SetShortDesc("Luxury Hotel");
      bldg.back()->SetSkill("DynamicInit", 2);	//City
      bldg.back()->SetSkill("DynamicPhase", 90);	//Luxury Hotel
      bldg.back()->SetSkill("DynamicMojo", 1000);
      bldg.back()->SetSkill("Rest", 100);
      }
    bldg.push_back(new Object(city));
    bldg.back()->SetShortDesc("Renraku Archology");
    bldg.back()->SetSkill("DynamicInit", 2);	//City
    bldg.back()->SetSkill("DynamicPhase", 100);	//Archology
    bldg.back()->SetSkill("DynamicMojo", 1000);
    bldg.back()->SetSkill("Food", 100);

    bldg.push_back(new Object(city));
    bldg.back()->SetShortDesc("Disney Archology");
    bldg.back()->SetSkill("DynamicInit", 2);	//City
    bldg.back()->SetSkill("DynamicPhase", 100);	//Archology
    bldg.back()->SetSkill("DynamicMojo", 1000);
    bldg.back()->SetSkill("Food", 100);

    bldg.push_back(new Object(city));
    bldg.back()->SetShortDesc("IBM Archology");
    bldg.back()->SetSkill("DynamicInit", 2);	//City
    bldg.back()->SetSkill("DynamicPhase", 100);	//Archology
    bldg.back()->SetSkill("DynamicMojo", 1000);
    bldg.back()->SetSkill("Food", 100);

    bldg.push_back(new Object(city));
    bldg.back()->SetShortDesc("CLA Archology");
    bldg.back()->SetSkill("DynamicInit", 2);	//City
    bldg.back()->SetSkill("DynamicPhase", 100);	//Archology
    bldg.back()->SetSkill("DynamicMojo", 1000);
    bldg.back()->SetSkill("Food", 100);

    bldg.push_back(new Object(city));
    bldg.back()->SetShortDesc("Fidget Technology Center");
    bldg.back()->SetSkill("DynamicInit", 2);	//City
    bldg.back()->SetSkill("DynamicPhase", 110);	//Research Facility
    bldg.back()->SetSkill("DynamicMojo", 1000);

    bldg.push_back(new Object(city));
    bldg.back()->SetShortDesc("Lone Star Station");
    bldg.back()->SetSkill("DynamicInit", 2);	//City
    bldg.back()->SetSkill("DynamicPhase", 120);	//Security Facility
    bldg.back()->SetSkill("DynamicMojo", 1000);

    bldg.push_back(new Object(city));
    bldg.back()->SetShortDesc("Doc-Wagon Hospital");
    bldg.back()->SetSkill("DynamicInit", 2);	//City
    bldg.back()->SetSkill("DynamicPhase", 130);	//Corp Hospital
    bldg.back()->SetSkill("DynamicMojo", 1000);
    bldg.back()->SetSkill("Healing", 1000000);
    for(int n=0; n<10; ++n) {
      bldg.push_back(new Object(city));
      bldg.back()->SetShortDesc("McHugh's Burgers");
      bldg.back()->SetSkill("DynamicInit", 2);	//City
      bldg.back()->SetSkill("DynamicPhase", 200);	//McHugh's
      bldg.back()->SetSkill("DynamicMojo", 1000);
      bldg.back()->SetSkill("Food", 1000);
      }
    for(int n=0; n<4; ++n) {
      bldg.push_back(new Object(city));
      bldg.back()->SetShortDesc("Super-Shopper Mall");
      bldg.back()->SetSkill("DynamicInit", 2);	//City
      bldg.back()->SetSkill("DynamicPhase", 300);	//Shopping Mall
      bldg.back()->SetSkill("DynamicMojo", 1000);
      bldg.back()->SetSkill("Food", 200);
      bldg.back()->SetSkill("Stuff", 500);
      bldg.back()->SetSkill("Fun", 100);
      }
    for(int n=0; n<2; ++n) {
      bldg.push_back(new Object(city));
      bldg.back()->SetShortDesc("Sam's Locker");
      bldg.back()->SetSkill("DynamicInit", 2);	//City
      bldg.back()->SetSkill("DynamicPhase", 310);	//Sam's Locker
      bldg.back()->SetSkill("DynamicMojo", 1000);
      bldg.back()->SetSkill("Stuff", 1000);
      }
    for(int n=0; n<4; ++n) {
      bldg.push_back(new Object(city));
      bldg.back()->SetShortDesc("Disney Entertainment Center");
      bldg.back()->SetSkill("DynamicInit", 2);	//City
      bldg.back()->SetSkill("DynamicPhase", 400);	//Cinema
      bldg.back()->SetSkill("DynamicMojo", 1000);
      bldg.back()->SetSkill("Fun", 1000);
      }
    for(int n=0; n<2; ++n) {
      bldg.push_back(new Object(city));
      bldg.back()->SetShortDesc("Disney Park");
      bldg.back()->SetSkill("DynamicInit", 2);	//City
      bldg.back()->SetSkill("DynamicPhase", 410);	//Amusement Park
      bldg.back()->SetSkill("DynamicMojo", 1000);
      bldg.back()->SetSkill("Fun", 1000);
      }

    random_shuffle(bldg.begin(), bldg.end());

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
	      cur->Link(street,
		"west", "The street continues west.\n",
		"east", "The street continues east.\n"
		);
	      }
	    street = cur;
	    }
	  if(off <= 0) {
	    if(ave[east]) {
	      cur->Link(ave[east],
		"south", "The avenue continues south.\n",
		"north", "The avenue continues north.\n"
		);
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

	      static int vacants = 0;
	      if(bldg.size() > 0 && vacants <= 0) {
		vacants = rand() & 0x0F;
		places[i*2] = bldg.back();
		bldg.pop_back();
		}
	      else {
		places[i*2] = new Object(city);
		places[i*2]->SetShortDesc("Vacant Lot");
		places[i*2]->SetSkill("DynamicInit", 2);	//City
		places[i*2]->SetSkill("DynamicPhase", 0);	//Lot
		places[i*2]->SetSkill("DynamicMojo", 1000);
		}
	      --vacants;

	      char addr[32];
	      if(off > 0) {
		sprintf(addr, "%d %s", (east+1)*100 + off + i, sname);
		}
	      else {
		sprintf(addr, "%d %s", (north+1)*10 + 10 + off + i, aname);
		}

	      places[i+1]->Link(places[i], dir[0], addr, dir[1], addr);

	      Object *people;
	      people = new Object(places[i*2]);
	      people->SetShortDesc("person");
	      people->SetDesc("a sketchy-looking punk.");
	      people->SetSkill("Quantity", 10);
	      people->SetSkill("Personality", 3);
	      people->SetSkill("Hungry", 10000);
	      people->SetSkill("Bored", 100000);
	      people->SetSkill("Tired", 10000);
	      people->SetSkill("Needy", 1000);
	      people->SetPos(POS_STAND);
	      people->Attach(get_mob_mind());
	      people->Activate();
	      people->SetAttribute(0, 3);
	      people->SetAttribute(1, 3);
	      people->SetAttribute(2, 3);
	      people->SetAttribute(3, 3);
	      people->SetAttribute(4, 3);
	      people->SetAttribute(5, 3);

	      people = new Object(places[i*2]);
	      people->SetShortDesc("person");
	      people->SetDesc("an upstanding citizen.");
	      people->SetSkill("Quantity", 100);
	      people->SetSkill("Personality", 5);
	      people->SetSkill("Hungry", 10000);
	      people->SetSkill("Bored", 100000);
	      people->SetSkill("Tired", 10000);
	      people->SetSkill("Needy", 1000);
	      people->SetPos(POS_STAND);
	      people->Attach(get_mob_mind());
	      people->Activate();
	      people->SetAttribute(0, 3);
	      people->SetAttribute(1, 3);
	      people->SetAttribute(2, 3);
	      people->SetAttribute(3, 3);
	      people->SetAttribute(4, 3);
	      people->SetAttribute(5, 3);

	      people = new Object(places[i*2]);
	      people->SetShortDesc("person");
	      people->SetDesc("a wealthy citizen.");
	      people->SetSkill("Quantity", 10);
	      people->SetSkill("Personality", 9);
	      people->SetSkill("Hungry", 10000);
	      people->SetSkill("Bored", 100000);
	      people->SetSkill("Tired", 10000);
	      people->SetSkill("Needy", 1000);
	      people->SetPos(POS_STAND);
	      people->Attach(get_mob_mind());
	      people->Activate();
	      people->SetAttribute(0, 3);
	      people->SetAttribute(1, 3);
	      people->SetAttribute(2, 3);
	      people->SetAttribute(3, 3);
	      people->SetAttribute(4, 3);
	      people->SetAttribute(5, 3);
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
