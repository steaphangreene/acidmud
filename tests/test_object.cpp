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

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include "../object.hpp"

TEST_CASE("Object Sanity", "[object]") {
  auto universe = new Object();
  REQUIRE(universe != nullptr);

  new Object(universe);
  new Object(universe);
  new Object(universe);
  auto inobj = new Object(universe);
  REQUIRE(universe->HasWithin(inobj));

  REQUIRE(universe->Contents().size() == 4);

  delete inobj;
  REQUIRE(universe->Contents().size() == 3);

  delete universe;
}

TEST_CASE("Object Movement", "[object]") {
  auto universe = new Object();
  REQUIRE(universe != nullptr);

  // Note: Each must have a different ShortDesc to avoid being auto-combined.
  auto world1 = new Object(universe);
  world1->SetShortDesc(u8"world1");
  auto world2 = new Object(universe);
  world2->SetShortDesc(u8"world2");
  auto zone1a = new Object(world1);
  zone1a->SetShortDesc(u8"zone1a");
  auto zone1b = new Object(world1);
  zone1b->SetShortDesc(u8"zone1b");
  auto room1a1 = new Object(zone1a);
  room1a1->SetShortDesc(u8"room1a1");
  auto room1a2 = new Object(zone1a);
  room1a2->SetShortDesc(u8"room1a2");
  auto obj1a1a = new Object(room1a1);
  obj1a1a->SetShortDesc(u8"obj1a1a");
  auto obj1a1b = new Object(room1a1);
  obj1a1b->SetShortDesc(u8"obj1a1b");
  auto item1a1a1 = new Object(obj1a1a);
  item1a1a1->SetShortDesc(u8"item1a1a1");
  auto item1a1a2 = new Object(obj1a1a);
  item1a1a2->SetShortDesc(u8"item1a1a2");

  REQUIRE(universe->HasWithin(item1a1a1));
  REQUIRE(obj1a1a->HasWithin(item1a1a1));

  REQUIRE(item1a1a1->World() == world1);
  REQUIRE(item1a1a1->Zone() == zone1a);
  REQUIRE(item1a1a1->Room() == room1a1);

  REQUIRE(universe->Contents().size() == 2);
  REQUIRE(world1->Contents().size() == 2);
  REQUIRE(zone1a->Contents().size() == 2);
  REQUIRE(room1a1->Contents().size() == 2);
  REQUIRE(obj1a1a->Contents().size() == 2);

  SECTION("Drop Item") {
    obj1a1a->Drop(item1a1a1);
    REQUIRE(!obj1a1a->HasWithin(item1a1a1));
    REQUIRE(room1a1->HasWithin(item1a1a1));
    REQUIRE(zone1a->HasWithin(room1a1));
  }

  SECTION("Move Item") {
    item1a1a1->Travel(obj1a1b);
    REQUIRE(obj1a1a->Contents().size() == 1);
    REQUIRE(obj1a1b->Contents().size() == 1);
    REQUIRE(!obj1a1a->HasWithin(item1a1a1));
    REQUIRE(room1a1->HasWithin(item1a1a1));
    REQUIRE(obj1a1b->HasWithin(item1a1a1));
    REQUIRE(room1a1->HasWithin(obj1a1b));
    REQUIRE(zone1a->HasWithin(room1a1));
    item1a1a1->Travel(obj1a1a);
    REQUIRE(obj1a1a->Contents().size() == 2);
    REQUIRE(obj1a1b->Contents().size() == 0);
  }

  SECTION("Move Object") {
    obj1a1a->Travel(room1a2);
    REQUIRE(!room1a1->HasWithin(obj1a1a));
    REQUIRE(room1a2->HasWithin(obj1a1a));
    REQUIRE(room1a2->HasWithin(item1a1a1));
    REQUIRE(room1a2->HasWithin(item1a1a2));
    REQUIRE(zone1a->HasWithin(room1a1));
    obj1a1a->Travel(room1a1);
  }

  REQUIRE(item1a1a1->World() == world1);
  REQUIRE(item1a1a1->Zone() == zone1a);
  REQUIRE(item1a1a1->Room() == room1a1);

  delete universe;
}
