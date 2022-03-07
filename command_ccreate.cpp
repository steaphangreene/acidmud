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
#include <random>

#include "cchar8.hpp"
#include "commands.hpp"
#include "mind.hpp"
#include "object.hpp"
#include "properties.hpp"
#include "utils.hpp"

#define NUM_AVS 5 // 28 Max!
#define NUM_STS 5 // No Max
int handle_command_ccreate(
    Object* body,
    Mind* mind,
    const std::u8string_view args,
    int stealth_t,
    int stealth_s) {
  if (args.empty()) {
    mind->Send(u8"You need to specify the name of the city!\n");
  } else {
    Object* city = new Object(body->Parent());
    city->SetShortDesc(args);
    city->SetSkill(prhash(u8"Light Source"), 1000);
    city->SetSkill(prhash(u8"Day Length"), 240);
    city->SetSkill(prhash(u8"Day Time"), 120);

    // Object *ocean = new Object(city);
    // ocean->SetShortDesc(u8"Ocean");
    // ocean->SetDesc(u8"The Atcific Ocean.");

    // Object *bay = new Object(city);
    // bay->SetShortDesc(u8"Bay");
    // bay->SetDesc((string(u8"The ") + args + u8" Bay.").c_str());

    // Object *inlet = new Object(city);
    // inlet->SetShortDesc(u8"Inlet");
    // inlet->SetDesc((string(u8"The ") + args + u8" Inlet.").c_str());

    // Object *wall = new Object(city);
    // wall->SetShortDesc(u8"North Wall");
    // wall->SetDesc(u8"The North Wall.");

    const char8_t* alist[] = {
        u8"West",   u8"Apple",  u8"Breeze", u8"Coconut", u8"Drury",  u8"Earl",  u8"Farley",
        u8"Gantry", u8"Henry",  u8"Indian", u8"Jawa",    u8"Kindle", u8"Lucky", u8"Moody",
        u8"Neddle", u8"Orion",  u8"Puddle", u8"Quaint",  u8"Rocky",  u8"Stone", u8"True",
        u8"Uber",   u8"Violet", u8"Widget", u8"X-Ray",   u8"Yeller", u8"Zebra", u8"",
    };
    alist[NUM_AVS - 1] = u8"East";

    std::vector<Object*> bldg;

    bldg.push_back(new Object(city));
    bldg.back()->SetShortDesc(u8"Municipal Building");
    bldg.back()->SetSkill(prhash(u8"DynamicInit"), 2); // City
    bldg.back()->SetSkill(prhash(u8"DynamicPhase"), 10); // Courthouse
    bldg.back()->SetSkill(prhash(u8"DynamicMojo"), 1000);
    for (int n = 0; n < 5; ++n) {
      bldg.push_back(new Object(city));
      bldg.back()->SetShortDesc(u8"Fire Station");
      bldg.back()->SetSkill(prhash(u8"DynamicInit"), 2); // City
      bldg.back()->SetSkill(prhash(u8"DynamicPhase"), 20); // Fire House
      bldg.back()->SetSkill(prhash(u8"DynamicMojo"), 1000);
      bldg.back()->SetSkill(prhash(u8"Healing"), 1);
    }
    for (int n = 0; n < 8; ++n) {
      bldg.push_back(new Object(city));
      bldg.back()->SetShortDesc(u8"Police Station");
      bldg.back()->SetSkill(prhash(u8"DynamicInit"), 2); // City
      bldg.back()->SetSkill(prhash(u8"DynamicPhase"), 30); // Police Station
      bldg.back()->SetSkill(prhash(u8"DynamicMojo"), 1000);
      bldg.back()->SetSkill(prhash(u8"Healing"), 1);
    }
    for (int n = 0; n < 4; ++n) {
      bldg.push_back(new Object(city));
      bldg.back()->SetShortDesc(u8"Hospital");
      bldg.back()->SetSkill(prhash(u8"DynamicInit"), 2); // City
      bldg.back()->SetSkill(prhash(u8"DynamicPhase"), 40); // City Hospital
      bldg.back()->SetSkill(prhash(u8"DynamicMojo"), 1000);
      bldg.back()->SetSkill(prhash(u8"Healing"), 10);
    }
    for (int n = 0; n < 25; ++n) {
      bldg.push_back(new Object(city));
      bldg.back()->SetShortDesc(u8"Coffin Motel");
      bldg.back()->SetSkill(prhash(u8"DynamicInit"), 2); // City
      bldg.back()->SetSkill(prhash(u8"DynamicPhase"), 50); // Coffin Motel
      bldg.back()->SetSkill(prhash(u8"DynamicMojo"), 1000);
      bldg.back()->SetSkill(prhash(u8"Rest"), 10);
    }
    for (int n = 0; n < 12; ++n) {
      bldg.push_back(new Object(city));
      bldg.back()->SetShortDesc(u8"Seedy Motel");
      bldg.back()->SetSkill(prhash(u8"DynamicInit"), 2); // City
      bldg.back()->SetSkill(prhash(u8"DynamicPhase"), 60); // Seedy Motel
      bldg.back()->SetSkill(prhash(u8"DynamicMojo"), 1000);
      bldg.back()->SetSkill(prhash(u8"Rest"), 5);
    }
    for (int n = 0; n < 8; ++n) {
      bldg.push_back(new Object(city));
      bldg.back()->SetShortDesc(u8"Nice Motel");
      bldg.back()->SetSkill(prhash(u8"DynamicInit"), 2); // City
      bldg.back()->SetSkill(prhash(u8"DynamicPhase"), 70); // Nice Motel
      bldg.back()->SetSkill(prhash(u8"DynamicMojo"), 1000);
      bldg.back()->SetSkill(prhash(u8"Rest"), 2);
    }
    for (int n = 0; n < 5; ++n) {
      bldg.push_back(new Object(city));
      bldg.back()->SetShortDesc(u8"Nice Hotel");
      bldg.back()->SetSkill(prhash(u8"DynamicInit"), 2); // City
      bldg.back()->SetSkill(prhash(u8"DynamicPhase"), 80); // Nice Hotel
      bldg.back()->SetSkill(prhash(u8"DynamicMojo"), 1000);
      bldg.back()->SetSkill(prhash(u8"Rest"), 1);
    }
    for (int n = 0; n < 4; ++n) {
      bldg.push_back(new Object(city));
      bldg.back()->SetShortDesc(u8"Luxury Hotel");
      bldg.back()->SetSkill(prhash(u8"DynamicInit"), 2); // City
      bldg.back()->SetSkill(prhash(u8"DynamicPhase"), 90); // Luxury Hotel
      bldg.back()->SetSkill(prhash(u8"DynamicMojo"), 1000);
      bldg.back()->SetSkill(prhash(u8"Rest"), 1);
    }
    bldg.push_back(new Object(city));
    bldg.back()->SetShortDesc(u8"Renraku Archology");
    bldg.back()->SetSkill(prhash(u8"DynamicInit"), 2); // City
    bldg.back()->SetSkill(prhash(u8"DynamicPhase"), 100); // Archology
    bldg.back()->SetSkill(prhash(u8"DynamicMojo"), 1000);
    bldg.back()->SetSkill(prhash(u8"Food"), 1);

    bldg.push_back(new Object(city));
    bldg.back()->SetShortDesc(u8"Disney Archology");
    bldg.back()->SetSkill(prhash(u8"DynamicInit"), 2); // City
    bldg.back()->SetSkill(prhash(u8"DynamicPhase"), 100); // Archology
    bldg.back()->SetSkill(prhash(u8"DynamicMojo"), 1000);
    bldg.back()->SetSkill(prhash(u8"Food"), 1);

    bldg.push_back(new Object(city));
    bldg.back()->SetShortDesc(u8"IBM Archology");
    bldg.back()->SetSkill(prhash(u8"DynamicInit"), 2); // City
    bldg.back()->SetSkill(prhash(u8"DynamicPhase"), 100); // Archology
    bldg.back()->SetSkill(prhash(u8"DynamicMojo"), 1000);
    bldg.back()->SetSkill(prhash(u8"Food"), 1);

    bldg.push_back(new Object(city));
    bldg.back()->SetShortDesc(u8"CLA Archology");
    bldg.back()->SetSkill(prhash(u8"DynamicInit"), 2); // City
    bldg.back()->SetSkill(prhash(u8"DynamicPhase"), 100); // Archology
    bldg.back()->SetSkill(prhash(u8"DynamicMojo"), 1000);
    bldg.back()->SetSkill(prhash(u8"Food"), 1);

    bldg.push_back(new Object(city));
    bldg.back()->SetShortDesc(u8"Fidget Technology Center");
    bldg.back()->SetSkill(prhash(u8"DynamicInit"), 2); // City
    bldg.back()->SetSkill(prhash(u8"DynamicPhase"), 110); // Research Facility
    bldg.back()->SetSkill(prhash(u8"DynamicMojo"), 1000);

    bldg.push_back(new Object(city));
    bldg.back()->SetShortDesc(u8"Lone Star Station");
    bldg.back()->SetSkill(prhash(u8"DynamicInit"), 2); // City
    bldg.back()->SetSkill(prhash(u8"DynamicPhase"), 120); // Security Facility
    bldg.back()->SetSkill(prhash(u8"DynamicMojo"), 1000);

    bldg.push_back(new Object(city));
    bldg.back()->SetShortDesc(u8"Doc-Wagon Hospital");
    bldg.back()->SetSkill(prhash(u8"DynamicInit"), 2); // City
    bldg.back()->SetSkill(prhash(u8"DynamicPhase"), 130); // Corp Hospital
    bldg.back()->SetSkill(prhash(u8"DynamicMojo"), 1000);
    bldg.back()->SetSkill(prhash(u8"Healing"), 100);
    for (int n = 0; n < 10; ++n) {
      bldg.push_back(new Object(city));
      bldg.back()->SetShortDesc(u8"McHugh's Burgers");
      bldg.back()->SetSkill(prhash(u8"DynamicInit"), 2); // City
      bldg.back()->SetSkill(prhash(u8"DynamicPhase"), 200); // McHugh's
      bldg.back()->SetSkill(prhash(u8"DynamicMojo"), 1000);
      bldg.back()->SetSkill(prhash(u8"Food"), 10);
    }
    for (int n = 0; n < 4; ++n) {
      bldg.push_back(new Object(city));
      bldg.back()->SetShortDesc(u8"Super-Shopper Mall");
      bldg.back()->SetSkill(prhash(u8"DynamicInit"), 2); // City
      bldg.back()->SetSkill(prhash(u8"DynamicPhase"), 300); // Shopping Mall
      bldg.back()->SetSkill(prhash(u8"DynamicMojo"), 1000);
      bldg.back()->SetSkill(prhash(u8"Food"), 2);
      bldg.back()->SetSkill(prhash(u8"Stuff"), 5);
      bldg.back()->SetSkill(prhash(u8"Fun"), 1);
    }
    for (int n = 0; n < 2; ++n) {
      bldg.push_back(new Object(city));
      bldg.back()->SetShortDesc(u8"Sam's Locker");
      bldg.back()->SetSkill(prhash(u8"DynamicInit"), 2); // City
      bldg.back()->SetSkill(prhash(u8"DynamicPhase"), 310); // Sam's Locker
      bldg.back()->SetSkill(prhash(u8"DynamicMojo"), 1000);
      bldg.back()->SetSkill(prhash(u8"Stuff"), 10);
    }
    for (int n = 0; n < 4; ++n) {
      bldg.push_back(new Object(city));
      bldg.back()->SetShortDesc(u8"Disney Entertainment Center");
      bldg.back()->SetSkill(prhash(u8"DynamicInit"), 2); // City
      bldg.back()->SetSkill(prhash(u8"DynamicPhase"), 400); // Cinema
      bldg.back()->SetSkill(prhash(u8"DynamicMojo"), 1000);
      bldg.back()->SetSkill(prhash(u8"Fun"), 10);
    }
    for (int n = 0; n < 2; ++n) {
      bldg.push_back(new Object(city));
      bldg.back()->SetShortDesc(u8"Disney Park");
      bldg.back()->SetSkill(prhash(u8"DynamicInit"), 2); // City
      bldg.back()->SetSkill(prhash(u8"DynamicPhase"), 410); // Amusement Park
      bldg.back()->SetSkill(prhash(u8"DynamicMojo"), 1000);
      bldg.back()->SetSkill(prhash(u8"Fun"), 10);
    }

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(bldg.begin(), bldg.end(), g);

    std::u8string sname, aname, iname;
    Object* ave[NUM_AVS] = {nullptr};
    for (int north = 0; north < NUM_STS; ++north) {
      if (north == 0)
        sname = u8"South Street";
      else if (north == NUM_STS - 1)
        sname = u8"North Street";
      else if (north % 10 == 1)
        sname = fmt::format(u8"{}st Street", north);
      else if (north % 10 == 2)
        sname = fmt::format(u8"{}nd Street", north);
      else if (north % 10 == 3)
        sname = fmt::format(u8"{}rd Street", north);
      else
        sname = fmt::format(u8"{}th Street", north);

      Object* street = nullptr;
      for (int east = 0; east < NUM_AVS; ++east) {
        aname = fmt::format(u8"{} Avenue", alist[east]);
        iname = fmt::format(u8"{} and {}", alist[east], sname.substr(0, sname.length() - 7));

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
            cur->SetDesc(u8"A busy intersection.\n");
          } else if (off > 0) {
            cur->SetShortDesc(sname);
            cur->SetDesc(u8"A nice street, running east and west.\n");
          } else /* if(off < 0) */ {
            cur->SetShortDesc(aname);
            cur->SetDesc(u8"A nice avenue, running north and south.\n");
          }

          // for(int i = 0; i < 3; ++i) {
          for (int i = 1; i < 2; ++i) { // Only one kind for now
            Object* people;
            people = new Object(cur);
            people->SetShortDesc(u8"a person");
            if (i <= 0) {
              people->SetDesc(u8"a sketchy-looking punk.");
              people->SetSkill(prhash(u8"Quantity"), 10);
              people->SetSkill(prhash(u8"Personality"), 3);
            } else if (i == 1) {
              people->SetDesc(u8"an upstanding citizen.");
              people->SetSkill(prhash(u8"Quantity"), 100);
              people->SetSkill(prhash(u8"Personality"), 5);
            } else if (i >= 2) {
              people->SetDesc(u8"a wealthy citizen.");
              people->SetSkill(prhash(u8"Quantity"), 10);
              people->SetSkill(prhash(u8"Personality"), 9);
            }
            people->SetSkill(prhash(u8"Hungry"), 10000);
            people->SetSkill(prhash(u8"Bored"), 100000);
            people->SetSkill(prhash(u8"Tired"), 10000);
            people->SetSkill(prhash(u8"Needy"), 1000);
            people->SetPos(pos_t::STAND);
            people->Attach(new Mind(mind_t::MOB));
            for (int a = 0; a < 6; ++a)
              people->SetAttribute(a, 3);
            people->Activate();
          }
          if (off >= 0) {
            if (street) {
              cur->Link(
                  street,
                  u8"west",
                  u8"The street continues west.\n",
                  u8"east",
                  u8"The street continues east.\n");
            }
            street = cur;
          }
          if (off <= 0) {
            if (ave[east]) {
              cur->Link(
                  ave[east],
                  u8"south",
                  u8"The avenue continues south.\n",
                  u8"north",
                  u8"The avenue continues north.\n");
            }
            ave[east] = cur;
          }

          if (off != 0) {
            Object* places[3] = {nullptr, cur, nullptr};
            std::u8string dir[2] = {u8"south", u8"north"};
            if (off < 0) {
              dir[0] = u8"west";
              dir[1] = u8"east";
            }
            for (int i = 0; i < 2; ++i) {
              std::u8string addr;
              if (off > 0) {
                addr = fmt::format(u8"{} {}", (east + 1) * 100 + off + i, sname);
              } else {
                addr = fmt::format(u8"{} {}", (north + 1) * 10 + 10 + off + i, aname);
              }

              static int vacants = 0;
              if (bldg.size() > 0 && vacants <= 0) {
                vacants = rand() & 0x0F;
                places[i * 2] = bldg.back();
                bldg.pop_back();
                places[i + 1]->Link(places[i], dir[0], addr, dir[1], addr);
              } else {
                places[i * 2] = new Object(city);
                places[i * 2]->SetShortDesc(u8"a vacant lot");
                places[i * 2]->SetSkill(prhash(u8"DynamicInit"), 2); // City
                places[i * 2]->SetSkill(prhash(u8"DynamicPhase"), 0); // Lot
                places[i * 2]->SetSkill(prhash(u8"DynamicMojo"), 1000);
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
    body->Parent()->SendOut(
        stealth_t,
        stealth_s,
        u8";s creates a new city '{}' with Ninja Powers[TM].\n",
        u8"You create a new city '{}'.\n",
        body,
        nullptr,
        args);
  }
  return 0;
}
