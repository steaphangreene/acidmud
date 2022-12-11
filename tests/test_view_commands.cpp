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

#include "test_main.hpp"

#include "../commands.hpp"
#include "../mind.hpp"
#include "../object.hpp"
#include "../properties.hpp"

static Object* test_person(Object* room) {
  Object* person = new Object(room);
  person->SetAttribute(0, 4);
  person->SetAttribute(1, 4);
  person->SetAttribute(2, 4);
  person->SetAttribute(3, 4);
  person->SetAttribute(4, 4);
  person->SetAttribute(5, 4);
  person->SetPosition(pos_t::STAND);
  return person;
}

TEST_CASE("Look Command", "[look]") {
  init_universe();

  // Note: Each must have a different ShortDesc to avoid being auto-combined.
  auto world = new Object(Object::Universe());
  world->SetShortDesc(u8"world");
  auto zone = new Object(world);
  zone->SetShortDesc(u8"zone");
  auto room = new Object(zone);
  room->SetShortDesc(u8"room");
  room->SetDesc(u8"This is a test room.");
  room->SetSkill(prhash(u8"Light Source"), 1000);
  auto room2 = new Object(zone);
  room2->SetShortDesc(u8"room2");
  room2->SetSkill(prhash(u8"Light Source"), 1000);

  auto mind = std::make_shared<Mind>(mind_t::TEST);
  auto actor = test_person(room);
  actor->SetShortDesc(u8"actor");
  actor->Attach(mind);

  auto target = test_person(room);
  target->SetShortDesc(u8"target");
  target->SetDesc(u8"This person looks like a good testing target.");
  target->Attach(std::make_shared<Mind>(mind_t::TEST));

  auto observer = test_person(room);
  observer->SetShortDesc(u8"observer");
  observer->SetDesc(u8"This person seems like someone to have nearby.");
  observer->Attach(std::make_shared<Mind>(mind_t::TEST));

  auto uninvolved = test_person(room2);
  uninvolved->SetShortDesc(u8"uninvolved");
  uninvolved->SetDesc(u8"This person does not appear to be involved.");
  uninvolved->Attach(std::make_shared<Mind>(mind_t::TEST));

  auto thing = new Object(room);
  thing->SetPosition(pos_t::LIE);
  thing->SetShortDesc(u8"a thing");
  thing->SetDesc(u8"This thing is just a generic thing.");
  thing->SetLongDesc(u8"This thing has a detailed description too.");
  thing->Attach(std::make_shared<Mind>(mind_t::TEST));

  SECTION("Basic Look Command") {
    handle_command(actor, u8"look", mind);
    REQUIRE(
        witness(actor) ==
        u8"Room\n   "
        u8"This is a test room.\n"
        u8"Target is standing here.\n"
        u8"Observer is standing here.\n"
        u8"A thing is lying here.\n");
    REQUIRE(witness(target) == u8"Actor looks around.\n");
    REQUIRE(witness(observer) == u8"Actor looks around.\n");
    REQUIRE(witness(uninvolved) == u8"");
  }

  SECTION("Basic Look At Command (Object)") {
    handle_command(actor, u8"look thing", mind);
    REQUIRE(
        witness(actor) ==
        u8"A thing is lying here in room.\n"
        u8"   This thing is just a generic thing.\n");
    REQUIRE(witness(target) == u8"Actor looks at a thing.\n");
    REQUIRE(witness(observer) == u8"Actor looks at a thing.\n");
    REQUIRE(witness(uninvolved) == u8"");
  }

  SECTION("Basic Look At Command (Person)") {
    handle_command(actor, u8"look target", mind);
    REQUIRE(
        witness(actor) ==
        u8"Target is standing here in room.\n"
        u8"   This person looks like a good testing target.\n");
    REQUIRE(witness(target) == u8"Actor looks at you.\n");
    REQUIRE(witness(observer) == u8"Actor looks at target.\n");
    REQUIRE(witness(uninvolved) == u8"");
  }

  SECTION("Basic Examine Command (Object)") {
    handle_command(actor, u8"examine thing", mind);
    REQUIRE(
        witness(actor) ==
        u8"A thing is lying here in room.\n"
        u8"   This thing has a detailed description too.\n");
    REQUIRE(witness(target) == u8"Actor examines a thing.\n");
    REQUIRE(witness(observer) == u8"Actor examines a thing.\n");
    REQUIRE(witness(uninvolved) == u8"");
  }

  destroy_universe();
}
