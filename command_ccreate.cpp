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
#include <cstdlib>
#include <cstring>
#include <random>

#include "commands.hpp"
#include "mind.hpp"
#include "object.hpp"
#include "utils.hpp"

#define NUM_AVS 5 // 28 Max!
#define NUM_STS 5 // No Max
int handle_command_ccreate(
    Object* body,
    Mind* mind,
    const std::string_view args,
    int stealth_t,
    int stealth_s) {
  if (args.empty()) {
    mind->Send("You need to specify the name of the city!\n");
  } else {
    Object* city = new Object(body->Parent());
    city->SetShortDesc(std::string(args));

    // Object *ocean = new Object(city);
    // ocean->SetShortDesc("Ocean");
    // ocean->SetDesc("The Atcific Ocean.");

    // Object *bay = new Object(city);
    // bay->SetShortDesc("Bay");
    // bay->SetDesc((string("The ") + args + " Bay.").c_str());

    // Object *inlet = new Object(city);
    // inlet->SetShortDesc("Inlet");
    // inlet->SetDesc((string("The ") + args + " Inlet.").c_str());

    // Object *wall = new Object(city);
    // wall->SetShortDesc("North Wall");
    // wall->SetDesc("The North Wall.");

    char alist[][8] = {"West",   "Apple",  "Breeze", "Coconut", "Drury",  "Earl",  "Farley",
                       "Gantry", "Henry",  "Indian", "Jawa",    "Kindle", "Lucky", "Moody",
                       "Neddle", "Orion",  "Puddle", "Quaint",  "Rocky",  "Stone", "True",
                       "Uber",   "Violet", "Widget", "X-Ray",   "Yeller", "Zebra", ""};
    sprintf(alist[NUM_AVS - 1], "East");

    std::vector<Object*> bldg;

    bldg.push_back(new Object(city));
    bldg.back()->SetShortDesc("Municipal Building");
    bldg.back()->SetSkill(crc32c("DynamicInit"), 2); // City
    bldg.back()->SetSkill(crc32c("DynamicPhase"), 10); // Courthouse
    bldg.back()->SetSkill(crc32c("DynamicMojo"), 1000);
    for (int n = 0; n < 5; ++n) {
      bldg.push_back(new Object(city));
      bldg.back()->SetShortDesc("Fire Station");
      bldg.back()->SetSkill(crc32c("DynamicInit"), 2); // City
      bldg.back()->SetSkill(crc32c("DynamicPhase"), 20); // Fire House
      bldg.back()->SetSkill(crc32c("DynamicMojo"), 1000);
      bldg.back()->SetSkill(crc32c("Healing"), 1);
    }
    for (int n = 0; n < 8; ++n) {
      bldg.push_back(new Object(city));
      bldg.back()->SetShortDesc("Police Station");
      bldg.back()->SetSkill(crc32c("DynamicInit"), 2); // City
      bldg.back()->SetSkill(crc32c("DynamicPhase"), 30); // Police Station
      bldg.back()->SetSkill(crc32c("DynamicMojo"), 1000);
      bldg.back()->SetSkill(crc32c("Healing"), 1);
    }
    for (int n = 0; n < 4; ++n) {
      bldg.push_back(new Object(city));
      bldg.back()->SetShortDesc("Hospital");
      bldg.back()->SetSkill(crc32c("DynamicInit"), 2); // City
      bldg.back()->SetSkill(crc32c("DynamicPhase"), 40); // City Hospital
      bldg.back()->SetSkill(crc32c("DynamicMojo"), 1000);
      bldg.back()->SetSkill(crc32c("Healing"), 10);
    }
    for (int n = 0; n < 25; ++n) {
      bldg.push_back(new Object(city));
      bldg.back()->SetShortDesc("Coffin Motel");
      bldg.back()->SetSkill(crc32c("DynamicInit"), 2); // City
      bldg.back()->SetSkill(crc32c("DynamicPhase"), 50); // Coffin Motel
      bldg.back()->SetSkill(crc32c("DynamicMojo"), 1000);
      bldg.back()->SetSkill(crc32c("Rest"), 10);
    }
    for (int n = 0; n < 12; ++n) {
      bldg.push_back(new Object(city));
      bldg.back()->SetShortDesc("Seedy Motel");
      bldg.back()->SetSkill(crc32c("DynamicInit"), 2); // City
      bldg.back()->SetSkill(crc32c("DynamicPhase"), 60); // Seedy Motel
      bldg.back()->SetSkill(crc32c("DynamicMojo"), 1000);
      bldg.back()->SetSkill(crc32c("Rest"), 5);
    }
    for (int n = 0; n < 8; ++n) {
      bldg.push_back(new Object(city));
      bldg.back()->SetShortDesc("Nice Motel");
      bldg.back()->SetSkill(crc32c("DynamicInit"), 2); // City
      bldg.back()->SetSkill(crc32c("DynamicPhase"), 70); // Nice Motel
      bldg.back()->SetSkill(crc32c("DynamicMojo"), 1000);
      bldg.back()->SetSkill(crc32c("Rest"), 2);
    }
    for (int n = 0; n < 5; ++n) {
      bldg.push_back(new Object(city));
      bldg.back()->SetShortDesc("Nice Hotel");
      bldg.back()->SetSkill(crc32c("DynamicInit"), 2); // City
      bldg.back()->SetSkill(crc32c("DynamicPhase"), 80); // Nice Hotel
      bldg.back()->SetSkill(crc32c("DynamicMojo"), 1000);
      bldg.back()->SetSkill(crc32c("Rest"), 1);
    }
    for (int n = 0; n < 4; ++n) {
      bldg.push_back(new Object(city));
      bldg.back()->SetShortDesc("Luxury Hotel");
      bldg.back()->SetSkill(crc32c("DynamicInit"), 2); // City
      bldg.back()->SetSkill(crc32c("DynamicPhase"), 90); // Luxury Hotel
      bldg.back()->SetSkill(crc32c("DynamicMojo"), 1000);
      bldg.back()->SetSkill(crc32c("Rest"), 1);
    }
    bldg.push_back(new Object(city));
    bldg.back()->SetShortDesc("Renraku Archology");
    bldg.back()->SetSkill(crc32c("DynamicInit"), 2); // City
    bldg.back()->SetSkill(crc32c("DynamicPhase"), 100); // Archology
    bldg.back()->SetSkill(crc32c("DynamicMojo"), 1000);
    bldg.back()->SetSkill(crc32c("Food"), 1);

    bldg.push_back(new Object(city));
    bldg.back()->SetShortDesc("Disney Archology");
    bldg.back()->SetSkill(crc32c("DynamicInit"), 2); // City
    bldg.back()->SetSkill(crc32c("DynamicPhase"), 100); // Archology
    bldg.back()->SetSkill(crc32c("DynamicMojo"), 1000);
    bldg.back()->SetSkill(crc32c("Food"), 1);

    bldg.push_back(new Object(city));
    bldg.back()->SetShortDesc("IBM Archology");
    bldg.back()->SetSkill(crc32c("DynamicInit"), 2); // City
    bldg.back()->SetSkill(crc32c("DynamicPhase"), 100); // Archology
    bldg.back()->SetSkill(crc32c("DynamicMojo"), 1000);
    bldg.back()->SetSkill(crc32c("Food"), 1);

    bldg.push_back(new Object(city));
    bldg.back()->SetShortDesc("CLA Archology");
    bldg.back()->SetSkill(crc32c("DynamicInit"), 2); // City
    bldg.back()->SetSkill(crc32c("DynamicPhase"), 100); // Archology
    bldg.back()->SetSkill(crc32c("DynamicMojo"), 1000);
    bldg.back()->SetSkill(crc32c("Food"), 1);

    bldg.push_back(new Object(city));
    bldg.back()->SetShortDesc("Fidget Technology Center");
    bldg.back()->SetSkill(crc32c("DynamicInit"), 2); // City
    bldg.back()->SetSkill(crc32c("DynamicPhase"), 110); // Research Facility
    bldg.back()->SetSkill(crc32c("DynamicMojo"), 1000);

    bldg.push_back(new Object(city));
    bldg.back()->SetShortDesc("Lone Star Station");
    bldg.back()->SetSkill(crc32c("DynamicInit"), 2); // City
    bldg.back()->SetSkill(crc32c("DynamicPhase"), 120); // Security Facility
    bldg.back()->SetSkill(crc32c("DynamicMojo"), 1000);

    bldg.push_back(new Object(city));
    bldg.back()->SetShortDesc("Doc-Wagon Hospital");
    bldg.back()->SetSkill(crc32c("DynamicInit"), 2); // City
    bldg.back()->SetSkill(crc32c("DynamicPhase"), 130); // Corp Hospital
    bldg.back()->SetSkill(crc32c("DynamicMojo"), 1000);
    bldg.back()->SetSkill(crc32c("Healing"), 100);
    for (int n = 0; n < 10; ++n) {
      bldg.push_back(new Object(city));
      bldg.back()->SetShortDesc("McHugh's Burgers");
      bldg.back()->SetSkill(crc32c("DynamicInit"), 2); // City
      bldg.back()->SetSkill(crc32c("DynamicPhase"), 200); // McHugh's
      bldg.back()->SetSkill(crc32c("DynamicMojo"), 1000);
      bldg.back()->SetSkill(crc32c("Food"), 10);
    }
    for (int n = 0; n < 4; ++n) {
      bldg.push_back(new Object(city));
      bldg.back()->SetShortDesc("Super-Shopper Mall");
      bldg.back()->SetSkill(crc32c("DynamicInit"), 2); // City
      bldg.back()->SetSkill(crc32c("DynamicPhase"), 300); // Shopping Mall
      bldg.back()->SetSkill(crc32c("DynamicMojo"), 1000);
      bldg.back()->SetSkill(crc32c("Food"), 2);
      bldg.back()->SetSkill(crc32c("Stuff"), 5);
      bldg.back()->SetSkill(crc32c("Fun"), 1);
    }
    for (int n = 0; n < 2; ++n) {
      bldg.push_back(new Object(city));
      bldg.back()->SetShortDesc("Sam's Locker");
      bldg.back()->SetSkill(crc32c("DynamicInit"), 2); // City
      bldg.back()->SetSkill(crc32c("DynamicPhase"), 310); // Sam's Locker
      bldg.back()->SetSkill(crc32c("DynamicMojo"), 1000);
      bldg.back()->SetSkill(crc32c("Stuff"), 10);
    }
    for (int n = 0; n < 4; ++n) {
      bldg.push_back(new Object(city));
      bldg.back()->SetShortDesc("Disney Entertainment Center");
      bldg.back()->SetSkill(crc32c("DynamicInit"), 2); // City
      bldg.back()->SetSkill(crc32c("DynamicPhase"), 400); // Cinema
      bldg.back()->SetSkill(crc32c("DynamicMojo"), 1000);
      bldg.back()->SetSkill(crc32c("Fun"), 10);
    }
    for (int n = 0; n < 2; ++n) {
      bldg.push_back(new Object(city));
      bldg.back()->SetShortDesc("Disney Park");
      bldg.back()->SetSkill(crc32c("DynamicInit"), 2); // City
      bldg.back()->SetSkill(crc32c("DynamicPhase"), 410); // Amusement Park
      bldg.back()->SetSkill(crc32c("DynamicMojo"), 1000);
      bldg.back()->SetSkill(crc32c("Fun"), 10);
    }

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(bldg.begin(), bldg.end(), g);

    char sname[32], aname[32], iname[64];
    Object* ave[NUM_AVS] = {nullptr};
    for (int north = 0; north < NUM_STS; ++north) {
      if (north == 0)
        sprintf(sname, "South Street");
      else if (north == NUM_STS - 1)
        sprintf(sname, "North Street");
      else if (north % 10 == 1)
        sprintf(sname, "%dst Street", north);
      else if (north % 10 == 2)
        sprintf(sname, "%dnd Street", north);
      else if (north % 10 == 3)
        sprintf(sname, "%drd Street", north);
      else
        sprintf(sname, "%dth Street", north);

      Object* street = nullptr;
      for (int east = 0; east < NUM_AVS; ++east) {
        sprintf(aname, "%s Avenue", alist[east]);
        sprintf(iname, "%s and %s", alist[east], sname);
        iname[strlen(iname) - 7] = 0;

        for (int off = -10; off <= 100; off += 2) {
          if (off == 0)
            off += 2; // Skip first zero

          if (off == 100)
            off = 0; // Create a second zero

          if (off < 0 && north < 1)
            continue;
          if (off > 0 && east < 1)
            continue;

          Object* cur = new Object(city);
          if (off == 0) {
            cur->SetShortDesc(iname);
            cur->SetDesc("A busy intersection.\n");
          } else if (off > 0) {
            cur->SetShortDesc(sname);
            cur->SetDesc("A nice street, running east and west.\n");
          } else /* if(off < 0) */ {
            cur->SetShortDesc(aname);
            cur->SetDesc("A nice avenue, running north and south.\n");
          }

          // for(int i = 0; i < 3; ++i) {
          for (int i = 1; i < 2; ++i) { // Only one kind for now
            Object* people;
            people = new Object(cur);
            people->SetShortDesc("a person");
            if (i <= 0) {
              people->SetDesc("a sketchy-looking punk.");
              people->SetSkill(crc32c("Quantity"), 10);
              people->SetSkill(crc32c("Personality"), 3);
            } else if (i == 1) {
              people->SetDesc("an upstanding citizen.");
              people->SetSkill(crc32c("Quantity"), 100);
              people->SetSkill(crc32c("Personality"), 5);
            } else if (i >= 2) {
              people->SetDesc("a wealthy citizen.");
              people->SetSkill(crc32c("Quantity"), 10);
              people->SetSkill(crc32c("Personality"), 9);
            }
            people->SetSkill(crc32c("Hungry"), 10000);
            people->SetSkill(crc32c("Bored"), 100000);
            people->SetSkill(crc32c("Tired"), 10000);
            people->SetSkill(crc32c("Needy"), 1000);
            people->SetPos(POS_STAND);
            people->Attach(get_mob_mind());
            for (int a = 0; a < 6; ++a)
              people->SetAttribute(a, 3);
            people->Activate();
          }
          if (off >= 0) {
            if (street) {
              cur->Link(
                  street,
                  "west",
                  "The street continues west.\n",
                  "east",
                  "The street continues east.\n");
            }
            street = cur;
          }
          if (off <= 0) {
            if (ave[east]) {
              cur->Link(
                  ave[east],
                  "south",
                  "The avenue continues south.\n",
                  "north",
                  "The avenue continues north.\n");
            }
            ave[east] = cur;
          }

          if (off != 0) {
            Object* places[3] = {nullptr, cur, nullptr};
            std::string dir[2] = {"south", "north"};
            if (off < 0) {
              dir[0] = "west";
              dir[1] = "east";
            }
            for (int i = 0; i < 2; ++i) {
              char addr[64];
              if (off > 0) {
                sprintf(addr, "%d %s", (east + 1) * 100 + off + i, sname);
              } else {
                sprintf(addr, "%d %s", (north + 1) * 10 + 10 + off + i, aname);
              }

              static int vacants = 0;
              if (bldg.size() > 0 && vacants <= 0) {
                vacants = rand() & 0x0F;
                places[i * 2] = bldg.back();
                bldg.pop_back();
                places[i + 1]->Link(places[i], dir[0], addr, dir[1], addr);
              } else {
                places[i * 2] = new Object(city);
                places[i * 2]->SetShortDesc("a vacant lot");
                places[i * 2]->SetSkill(crc32c("DynamicInit"), 2); // City
                places[i * 2]->SetSkill(crc32c("DynamicPhase"), 0); // Lot
                places[i * 2]->SetSkill(crc32c("DynamicMojo"), 1000);
                places[i + 1]->LinkClosed(places[i], dir[0], addr, dir[1], addr);
              }
              --vacants;
            }
          }

          if (off == 0)
            off = 100; // Zero really was 100
        }
      }
    }
    body->Parent()->SendOutF(
        stealth_t,
        stealth_s,
        ";s creates a new city '%s' with Ninja Powers[TM].\n",
        "You create a new city '%s'.\n",
        body,
        nullptr,
        std::string(args).c_str());
  }
  return 0;
}
