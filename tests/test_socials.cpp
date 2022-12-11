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

TEST_CASE("Social Commands", "[socials]") {
  init_universe();

  // Note: Each must have a different ShortDesc to avoid being auto-combined.
  auto world = new Object(Object::Universe());
  world->SetShortDesc(u8"world");
  auto zone = new Object(world);
  zone->SetShortDesc(u8"zone");
  auto room = new Object(zone);
  room->SetShortDesc(u8"room");
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
  target->Attach(std::make_shared<Mind>(mind_t::TEST));

  auto observer = test_person(room);
  observer->SetShortDesc(u8"observer");
  observer->Attach(std::make_shared<Mind>(mind_t::TEST));

  auto uninvolved = test_person(room2);
  uninvolved->SetShortDesc(u8"uninvolved");
  uninvolved->Attach(std::make_shared<Mind>(mind_t::TEST));

  SECTION("Basic Social Command") {
    handle_command(actor, u8"rofl", mind);
    REQUIRE(witness(actor) == u8"You roll around on the floor, laughing.\n");
    REQUIRE(witness(target) == u8"Actor rolls around on the floor, laughing.\n");
    REQUIRE(witness(observer) == u8"Actor rolls around on the floor, laughing.\n");
    REQUIRE(witness(uninvolved) == u8"");
  }

  SECTION("First Social Command") {
    handle_command(actor, u8"...", mind);
    REQUIRE(witness(actor) == u8"You're speechless.\n");
    REQUIRE(witness(target) == u8"Actor is speechless.\n");
    REQUIRE(witness(observer) == u8"Actor is speechless.\n");
    REQUIRE(witness(uninvolved) == u8"");
  }

  SECTION("Last Social Command") {
    handle_command(actor, u8"zip", mind);
    REQUIRE(witness(actor) == u8"You zip your mouth shut - zzzzzip!\n");
    REQUIRE(witness(target) == u8"Actor zips its mouth shut - zzzzzip!\n");
    REQUIRE(witness(observer) == u8"Actor zips its mouth shut - zzzzzip!\n");
    REQUIRE(witness(uninvolved) == u8"");
  }

  SECTION("Social Commands Affected by Gender (Male)") {
    actor->SetGender(gender_t::MALE);
    handle_command(actor, u8"zip", mind);
    REQUIRE(witness(actor) == u8"You zip your mouth shut - zzzzzip!\n");
    REQUIRE(witness(target) == u8"Actor zips his mouth shut - zzzzzip!\n");
    REQUIRE(witness(observer) == u8"Actor zips his mouth shut - zzzzzip!\n");
    REQUIRE(witness(uninvolved) == u8"");
  }

  SECTION("Social Commands NOT Affected by Gender (Male)") {
    actor->SetGender(gender_t::MALE);
    handle_command(actor, u8"...", mind);
    REQUIRE(witness(actor) == u8"You're speechless.\n");
    REQUIRE(witness(target) == u8"Actor is speechless.\n");
    REQUIRE(witness(observer) == u8"Actor is speechless.\n");
    REQUIRE(witness(uninvolved) == u8"");
  }

  SECTION("Social Commands Affected by Gender (Female)") {
    actor->SetGender(gender_t::FEMALE);
    handle_command(actor, u8"zip", mind);
    REQUIRE(witness(actor) == u8"You zip your mouth shut - zzzzzip!\n");
    REQUIRE(witness(target) == u8"Actor zips her mouth shut - zzzzzip!\n");
    REQUIRE(witness(observer) == u8"Actor zips her mouth shut - zzzzzip!\n");
    REQUIRE(witness(uninvolved) == u8"");
  }

  SECTION("Social Commands NOT Affected by Gender (Female)") {
    actor->SetGender(gender_t::FEMALE);
    handle_command(actor, u8"...", mind);
    REQUIRE(witness(actor) == u8"You're speechless.\n");
    REQUIRE(witness(target) == u8"Actor is speechless.\n");
    REQUIRE(witness(observer) == u8"Actor is speechless.\n");
    REQUIRE(witness(uninvolved) == u8"");
  }

  SECTION("Social Commands Affected by Gender (Neither)") {
    actor->SetGender(gender_t::NEITHER);
    handle_command(actor, u8"zip", mind);
    REQUIRE(witness(actor) == u8"You zip your mouth shut - zzzzzip!\n");
    REQUIRE(witness(target) == u8"Actor zips their mouth shut - zzzzzip!\n");
    REQUIRE(witness(observer) == u8"Actor zips their mouth shut - zzzzzip!\n");
    REQUIRE(witness(uninvolved) == u8"");
  }

  SECTION("Social Commands NOT Affected by Gender (Neither)") {
    actor->SetGender(gender_t::NEITHER);
    handle_command(actor, u8"...", mind);
    REQUIRE(witness(actor) == u8"You're speechless.\n");
    REQUIRE(witness(target) == u8"Actor is speechless.\n");
    REQUIRE(witness(observer) == u8"Actor is speechless.\n");
    REQUIRE(witness(uninvolved) == u8"");
  }

  SECTION("Targeted Social Commands, Affected by Gender (None/None)") {
    handle_command(actor, u8"arrest target", mind);
    REQUIRE(witness(actor) == u8"You read target its rights.\n");
    REQUIRE(
        witness(target) ==
        u8"Actor says, 'You have the right to remain silent. "
        u8"You have the right to....' it just arrested you!\n");
    REQUIRE(
        witness(observer) ==
        u8"Actor has placed target under arrest. "
        u8"Everything it will, and can say, will be used against it.\n");
    REQUIRE(witness(uninvolved) == u8"");
  }

  SECTION("Targeted Social Commands, Affected by Gender (Male/None)") {
    actor->SetGender(gender_t::MALE);
    handle_command(actor, u8"arrest target", mind);
    REQUIRE(witness(actor) == u8"You read target its rights.\n");
    REQUIRE(
        witness(target) ==
        u8"Actor says, 'You have the right to remain silent. "
        u8"You have the right to....' he just arrested you!\n");
    REQUIRE(
        witness(observer) ==
        u8"Actor has placed target under arrest. "
        u8"Everything it will, and can say, will be used against it.\n");
    REQUIRE(witness(uninvolved) == u8"");
  }

  SECTION("Targeted Social Commands, Affected by Gender (Female/None)") {
    actor->SetGender(gender_t::FEMALE);
    handle_command(actor, u8"arrest target", mind);
    REQUIRE(witness(actor) == u8"You read target its rights.\n");
    REQUIRE(
        witness(target) ==
        u8"Actor says, 'You have the right to remain silent. "
        u8"You have the right to....' she just arrested you!\n");
    REQUIRE(
        witness(observer) ==
        u8"Actor has placed target under arrest. "
        u8"Everything it will, and can say, will be used against it.\n");
    REQUIRE(witness(uninvolved) == u8"");
  }

  SECTION("Targeted Social Commands, Affected by Gender (Neither/None)") {
    actor->SetGender(gender_t::NEITHER);
    handle_command(actor, u8"arrest target", mind);
    REQUIRE(witness(actor) == u8"You read target its rights.\n");
    REQUIRE(
        witness(target) ==
        u8"Actor says, 'You have the right to remain silent. "
        u8"You have the right to....' they just arrested you!\n");
    REQUIRE(
        witness(observer) ==
        u8"Actor has placed target under arrest. "
        u8"Everything it will, and can say, will be used against it.\n");
    REQUIRE(witness(uninvolved) == u8"");
  }

  SECTION("Targeted Social Commands, Affected by Gender (None/Male)") {
    target->SetGender(gender_t::MALE);
    handle_command(actor, u8"arrest target", mind);
    REQUIRE(witness(actor) == u8"You read target his rights.\n");
    REQUIRE(
        witness(target) ==
        u8"Actor says, 'You have the right to remain silent. "
        u8"You have the right to....' it just arrested you!\n");
    REQUIRE(
        witness(observer) ==
        u8"Actor has placed target under arrest. "
        u8"Everything he will, and can say, will be used against him.\n");
    REQUIRE(witness(uninvolved) == u8"");
  }

  SECTION("Targeted Social Commands, Affected by Gender (Male/Male)") {
    actor->SetGender(gender_t::MALE);
    target->SetGender(gender_t::MALE);
    handle_command(actor, u8"arrest target", mind);
    REQUIRE(witness(actor) == u8"You read target his rights.\n");
    REQUIRE(
        witness(target) ==
        u8"Actor says, 'You have the right to remain silent. "
        u8"You have the right to....' he just arrested you!\n");
    REQUIRE(
        witness(observer) ==
        u8"Actor has placed target under arrest. "
        u8"Everything he will, and can say, will be used against him.\n");
    REQUIRE(witness(uninvolved) == u8"");
  }

  SECTION("Targeted Social Commands, Affected by Gender (Female/Male)") {
    actor->SetGender(gender_t::FEMALE);
    target->SetGender(gender_t::MALE);
    handle_command(actor, u8"arrest target", mind);
    REQUIRE(witness(actor) == u8"You read target his rights.\n");
    REQUIRE(
        witness(target) ==
        u8"Actor says, 'You have the right to remain silent. "
        u8"You have the right to....' she just arrested you!\n");
    REQUIRE(
        witness(observer) ==
        u8"Actor has placed target under arrest. "
        u8"Everything he will, and can say, will be used against him.\n");
    REQUIRE(witness(uninvolved) == u8"");
  }

  SECTION("Targeted Social Commands, Affected by Gender (Neither/Male)") {
    actor->SetGender(gender_t::NEITHER);
    target->SetGender(gender_t::MALE);
    handle_command(actor, u8"arrest target", mind);
    REQUIRE(witness(actor) == u8"You read target his rights.\n");
    REQUIRE(
        witness(target) ==
        u8"Actor says, 'You have the right to remain silent. "
        u8"You have the right to....' they just arrested you!\n");
    REQUIRE(
        witness(observer) ==
        u8"Actor has placed target under arrest. "
        u8"Everything he will, and can say, will be used against him.\n");
    REQUIRE(witness(uninvolved) == u8"");
  }

  SECTION("Targeted Social Commands, Affected by Gender (None/Female)") {
    target->SetGender(gender_t::FEMALE);
    handle_command(actor, u8"arrest target", mind);
    REQUIRE(witness(actor) == u8"You read target her rights.\n");
    REQUIRE(
        witness(target) ==
        u8"Actor says, 'You have the right to remain silent. "
        u8"You have the right to....' it just arrested you!\n");
    REQUIRE(
        witness(observer) ==
        u8"Actor has placed target under arrest. "
        u8"Everything she will, and can say, will be used against her.\n");
    REQUIRE(witness(uninvolved) == u8"");
  }

  SECTION("Targeted Social Commands, Affected by Gender (Male/Female)") {
    actor->SetGender(gender_t::MALE);
    target->SetGender(gender_t::FEMALE);
    handle_command(actor, u8"arrest target", mind);
    REQUIRE(witness(actor) == u8"You read target her rights.\n");
    REQUIRE(
        witness(target) ==
        u8"Actor says, 'You have the right to remain silent. "
        u8"You have the right to....' he just arrested you!\n");
    REQUIRE(
        witness(observer) ==
        u8"Actor has placed target under arrest. "
        u8"Everything she will, and can say, will be used against her.\n");
    REQUIRE(witness(uninvolved) == u8"");
  }

  SECTION("Targeted Social Commands, Affected by Gender (Female/Female)") {
    actor->SetGender(gender_t::FEMALE);
    target->SetGender(gender_t::FEMALE);
    handle_command(actor, u8"arrest target", mind);
    REQUIRE(witness(actor) == u8"You read target her rights.\n");
    REQUIRE(
        witness(target) ==
        u8"Actor says, 'You have the right to remain silent. "
        u8"You have the right to....' she just arrested you!\n");
    REQUIRE(
        witness(observer) ==
        u8"Actor has placed target under arrest. "
        u8"Everything she will, and can say, will be used against her.\n");
    REQUIRE(witness(uninvolved) == u8"");
  }

  SECTION("Targeted Social Commands, Affected by Gender (Neither/Female)") {
    actor->SetGender(gender_t::NEITHER);
    target->SetGender(gender_t::FEMALE);
    handle_command(actor, u8"arrest target", mind);
    REQUIRE(witness(actor) == u8"You read target her rights.\n");
    REQUIRE(
        witness(target) ==
        u8"Actor says, 'You have the right to remain silent. "
        u8"You have the right to....' they just arrested you!\n");
    REQUIRE(
        witness(observer) ==
        u8"Actor has placed target under arrest. "
        u8"Everything she will, and can say, will be used against her.\n");
    REQUIRE(witness(uninvolved) == u8"");
  }

  SECTION("Targeted Social Commands, Affected by Gender (None/Neither)") {
    target->SetGender(gender_t::NEITHER);
    handle_command(actor, u8"arrest target", mind);
    REQUIRE(witness(actor) == u8"You read target their rights.\n");
    REQUIRE(
        witness(target) ==
        u8"Actor says, 'You have the right to remain silent. "
        u8"You have the right to....' it just arrested you!\n");
    REQUIRE(
        witness(observer) ==
        u8"Actor has placed target under arrest. "
        u8"Everything they will, and can say, will be used against them.\n");
    REQUIRE(witness(uninvolved) == u8"");
  }

  SECTION("Targeted Social Commands, Affected by Gender (Male/Neither)") {
    actor->SetGender(gender_t::MALE);
    target->SetGender(gender_t::NEITHER);
    handle_command(actor, u8"arrest target", mind);
    REQUIRE(witness(actor) == u8"You read target their rights.\n");
    REQUIRE(
        witness(target) ==
        u8"Actor says, 'You have the right to remain silent. "
        u8"You have the right to....' he just arrested you!\n");
    REQUIRE(
        witness(observer) ==
        u8"Actor has placed target under arrest. "
        u8"Everything they will, and can say, will be used against them.\n");
    REQUIRE(witness(uninvolved) == u8"");
  }

  SECTION("Targeted Social Commands, Affected by Gender (Female/Neither)") {
    actor->SetGender(gender_t::FEMALE);
    target->SetGender(gender_t::NEITHER);
    handle_command(actor, u8"arrest target", mind);
    REQUIRE(witness(actor) == u8"You read target their rights.\n");
    REQUIRE(
        witness(target) ==
        u8"Actor says, 'You have the right to remain silent. "
        u8"You have the right to....' she just arrested you!\n");
    REQUIRE(
        witness(observer) ==
        u8"Actor has placed target under arrest. "
        u8"Everything they will, and can say, will be used against them.\n");
    REQUIRE(witness(uninvolved) == u8"");
  }

  SECTION("Targeted Social Commands, Affected by Gender (Neither/Neither)") {
    actor->SetGender(gender_t::NEITHER);
    target->SetGender(gender_t::NEITHER);
    handle_command(actor, u8"arrest target", mind);
    REQUIRE(witness(actor) == u8"You read target their rights.\n");
    REQUIRE(
        witness(target) ==
        u8"Actor says, 'You have the right to remain silent. "
        u8"You have the right to....' they just arrested you!\n");
    REQUIRE(
        witness(observer) ==
        u8"Actor has placed target under arrest. "
        u8"Everything they will, and can say, will be used against them.\n");
    REQUIRE(witness(uninvolved) == u8"");
  }

  SECTION("Conflicting Social Commands, Ensure The TBA Versions Don't Run") {
    handle_command(actor, u8"point target", mind);
    REQUIRE(witness(actor) == u8"You start pointing at target.\n");
    REQUIRE(witness(target) == u8"Actor starts pointing at you.\n");
    REQUIRE(witness(observer) == u8"Actor starts pointing at target.\n");
    REQUIRE(witness(uninvolved) == u8"");
  }

  destroy_universe();
}
