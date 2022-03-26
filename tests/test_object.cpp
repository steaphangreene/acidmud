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

#include "catch2.hpp"

#include "../object.hpp"
#include "../properties.hpp"

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
  init_universe();
  REQUIRE(Object::Universe() != nullptr);

  // Note: Each must have a different ShortDesc to avoid being auto-combined.
  auto world1 = new Object(Object::Universe());
  world1->SetShortDesc(u8"world1");
  auto world2 = new Object(Object::Universe());
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

  REQUIRE(Object::Universe()->HasWithin(item1a1a1));
  REQUIRE(obj1a1a->HasWithin(item1a1a1));

  REQUIRE(item1a1a1->World() == world1);
  REQUIRE(item1a1a1->Zone() == zone1a);
  REQUIRE(item1a1a1->Room() == room1a1);

  REQUIRE(Object::Universe()->Contents().size() == 2);
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
    REQUIRE(room1a1->HasWithin(obj1a1b));
    REQUIRE(!room1a2->HasWithin(obj1a1b));
    REQUIRE(room1a2->HasWithin(item1a1a1));
    REQUIRE(room1a2->HasWithin(item1a1a2));
    REQUIRE(zone1a->HasWithin(room1a1));
    obj1a1a->Travel(room1a1);
    REQUIRE(room1a1->HasWithin(obj1a1a));
    REQUIRE(!room1a2->HasWithin(obj1a1a));
    REQUIRE(room1a1->HasWithin(obj1a1b));
    REQUIRE(!room1a2->HasWithin(obj1a1b));
  }

  SECTION("Do Not Combine Non-Empty Objects") {
    obj1a1a->SetShortDesc(u8"do not merge me");
    obj1a1b->SetShortDesc(u8"do not merge me");
    obj1a1a->Travel(room1a2);
    REQUIRE(!room1a1->HasWithin(obj1a1a));
    REQUIRE(room1a2->HasWithin(obj1a1a));
    REQUIRE(room1a1->HasWithin(obj1a1b));
    REQUIRE(!room1a2->HasWithin(obj1a1b));
    REQUIRE(room1a2->HasWithin(item1a1a1));
    REQUIRE(room1a2->HasWithin(item1a1a2));
    REQUIRE(zone1a->HasWithin(room1a1));
    obj1a1a->Travel(room1a1);
    REQUIRE(room1a1->HasWithin(obj1a1a));
    REQUIRE(!room1a2->HasWithin(obj1a1a));
    REQUIRE(room1a1->HasWithin(obj1a1b));
    REQUIRE(!room1a2->HasWithin(obj1a1b));
  }

  SECTION("Combine Empty Objects") {
    item1a1a1->SetShortDesc(u8"merge me");
    item1a1a2->SetShortDesc(u8"merge me");
    item1a1a1->Travel(obj1a1b);
    REQUIRE(obj1a1a->Contents().size() == 1);
    REQUIRE(obj1a1b->Contents().size() == 1);
    REQUIRE(room1a1->HasWithin(obj1a1a));
    REQUIRE(!room1a2->HasWithin(obj1a1a));
    REQUIRE(room1a1->HasWithin(obj1a1b));
    REQUIRE(!room1a2->HasWithin(obj1a1b));
    REQUIRE(room1a1->HasWithin(item1a1a1));
    REQUIRE(room1a1->HasWithin(item1a1a2));
    REQUIRE(!room1a2->HasWithin(item1a1a1));
    REQUIRE(!room1a2->HasWithin(item1a1a2));
    REQUIRE(zone1a->HasWithin(room1a1));
    item1a1a1->Travel(obj1a1a);
    REQUIRE(item1a1a1->HasSkill(prhash(u8"Quantity")));
    REQUIRE(item1a1a1->Skill(prhash(u8"Quantity")) == 2);
    REQUIRE(item1a1a2->Parent() == Object::TrashBin());
    REQUIRE(obj1a1a->Contents().size() == 1);
    REQUIRE(obj1a1b->Contents().size() == 0);
    REQUIRE(room1a1->HasWithin(obj1a1a));
    REQUIRE(!room1a2->HasWithin(obj1a1a));
    REQUIRE(room1a1->HasWithin(obj1a1b));
    REQUIRE(!room1a2->HasWithin(obj1a1b));
  }

  REQUIRE(item1a1a1->World() == world1);
  REQUIRE(item1a1a1->Zone() == zone1a);
  REQUIRE(item1a1a1->Room() == room1a1);

  destroy_universe();
}

TEST_CASE("Object Actions", "[object]") {
  init_universe();
  REQUIRE(Object::Universe() != nullptr);

  // Note: Each must have a different ShortDesc to avoid being auto-combined.
  auto world1 = new Object(Object::Universe());
  world1->SetShortDesc(u8"world1");
  auto world2 = new Object(Object::Universe());
  world2->SetShortDesc(u8"world2");
  auto zone1a = new Object(world1);
  zone1a->SetShortDesc(u8"zone1a");
  auto zone1b = new Object(world1);
  zone1b->SetShortDesc(u8"zone1b");
  auto room1a1 = new Object(zone1a);
  room1a1->SetShortDesc(u8"room1a1");
  auto room1a2 = new Object(zone1a);
  room1a1->SetShortDesc(u8"room1a2");
  auto room1b1 = new Object(zone1b);
  room1b1->SetShortDesc(u8"room1b1");
  auto room1b2 = new Object(zone1b);
  room1b1->SetShortDesc(u8"room1b2");
  auto obj1a1a = new Object(room1a1);
  obj1a1a->SetShortDesc(u8"obj1a1a");
  auto obj1b1a = new Object(room1b1);
  obj1b1a->SetShortDesc(u8"obj1b1a");
  auto item1a1a1 = new Object(obj1a1a);
  item1a1a1->SetShortDesc(u8"item1a1a1");
  auto item1b1a1 = new Object(obj1b1a);
  item1b1a1->SetShortDesc(u8"item1b1a1");

  for (auto a = act_t::NONE; a < act_t::SPECIAL_ACTEE; ++a) {
    if (a != act_t::NONE && a != act_t::SPECIAL_LINKED) {
      obj1a1a->AddAct(a, obj1b1a);
    }
  }

  SECTION("Distant Actions") {
    REQUIRE(obj1a1a->ActTarg(act_t::FIGHT) == obj1b1a);
    REQUIRE(obj1a1a->ActTarg(act_t::SPECIAL_HOME) == obj1b1a);
    REQUIRE(obj1b1a->Touching().size() == 1);
    REQUIRE(obj1b1a->Touching().front() == obj1a1a);
    REQUIRE(obj1a1a->Parent() == room1a1);
    REQUIRE(obj1b1a->Parent() == room1b1);
  }

  SECTION("Delete Actor") {
    delete obj1a1a;
    REQUIRE(obj1b1a->Touching().size() == 0);
    REQUIRE(obj1b1a->Parent() == room1b1);
  }

  SECTION("Delete Actee") {
    delete obj1b1a;
    REQUIRE(obj1a1a->IsAct(act_t::FIGHT) == false);
    REQUIRE(obj1a1a->ActTarg(act_t::FIGHT) == nullptr);
    REQUIRE(obj1a1a->IsAct(act_t::SPECIAL_HOME) == false);
    REQUIRE(obj1a1a->ActTarg(act_t::SPECIAL_HOME) == nullptr);
    REQUIRE(obj1a1a->Parent() == room1a1);
  }

  SECTION("Linked Delete Actor") {
    obj1a1a->AddAct(act_t::SPECIAL_LINKED, obj1b1a);
    delete obj1a1a;
    REQUIRE(obj1b1a->Touching().size() == 0);
    REQUIRE(obj1b1a->Parent() == room1b1);
  }

  SECTION("Linked Delete Actee") {
    obj1a1a->AddAct(act_t::SPECIAL_LINKED, obj1b1a);
    delete obj1b1a;
    REQUIRE(obj1a1a->IsAct(act_t::FIGHT) == false);
    REQUIRE(obj1a1a->ActTarg(act_t::FIGHT) == nullptr);
    REQUIRE(obj1a1a->IsAct(act_t::SPECIAL_HOME) == false);
    REQUIRE(obj1a1a->ActTarg(act_t::SPECIAL_HOME) == nullptr);
    REQUIRE(obj1a1a->Parent() == Object::TrashBin());
  }

  SECTION("Move Distant Actor") {
    obj1a1a->Travel(room1a2);
    // FIXME: Object movement should use Touching() mechanism, not notifications.
    // REQUIRE(obj1a1a->IsAct(act_t::FIGHT) == false);
    // REQUIRE(obj1a1a->ActTarg(act_t::FIGHT) == nullptr);
    REQUIRE(obj1a1a->ActTarg(act_t::SPECIAL_HOME) == obj1b1a);
    REQUIRE(obj1b1a->Touching().size() == 1);
    REQUIRE(obj1b1a->Parent() == room1b1);
  }

  SECTION("Move Distant Actee") {
    obj1b1a->Travel(room1b2);
    REQUIRE(obj1a1a->IsAct(act_t::FIGHT) == false);
    REQUIRE(obj1a1a->ActTarg(act_t::FIGHT) == nullptr);
    REQUIRE(obj1a1a->ActTarg(act_t::SPECIAL_HOME) == obj1b1a);
    // FIXME: Follow should not work at a distance.
    // REQUIRE(obj1a1a->Parent() == room1a1);
  }

  destroy_universe();
}
