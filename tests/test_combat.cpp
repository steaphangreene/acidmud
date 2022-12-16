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
#include "../dice.hpp"
#include "../global.hpp"
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

static Object* give_sword(Object* owner) {
  Object* sword = new Object(owner);
  sword->SetShortDesc(u8"an arming sword");
  sword->SetDesc(u8"A well-made cutting sort, intended for one-handed use.");
  sword->SetSkill(prhash(u8"WeaponType"), 14);
  sword->SetSkill(prhash(u8"Wearable on Left Hip"), 1);
  sword->SetSkill(prhash(u8"Wearable on Right Hip"), 2);
  sword->SetSkill(prhash(u8"WeaponSeverity"), 1);
  sword->SetSkill(prhash(u8"WeaponForce"), 1);
  sword->SetSkill(prhash(u8"WeaponReach"), 2);
  sword->SetWeight(1 * 454);
  sword->SetSize(2);
  sword->SetVolume(1);
  sword->SetValue(100);
  sword->SetPosition(pos_t::LIE);
  owner->AddAct(act_t::WEAR_LHIP, sword);
  return sword;
}

TEST_CASE("Basic Fight", "[commands]") {
  Dice::Seed(0); // Always the same "random" die results.

  init_universe();

  auto mind = std::make_shared<Mind>(mind_t::TEST);

  // Note: Each must have a different ShortDesc to avoid being auto-combined.
  auto world = new Object(Object::Universe());
  world->SetShortDesc(u8"world");
  auto zone = new Object(world);
  zone->SetShortDesc(u8"zone");
  auto room = new Object(zone);
  room->SetShortDesc(u8"room");
  room->SetSkill(prhash(u8"Light Source"), 1000);

  auto attacker = test_person(room);
  attacker->SetGender(gender_t::FEMALE);
  attacker->SetShortDesc(u8"attacker");
  attacker->SetSkill(prhash(u8"Long Blades"), 3);
  attacker->Attach(mind);
  Object* att_sword = give_sword(attacker);
  att_sword->SetLongDesc(u8"an aggressive arming sword");

  auto defender = test_person(room);
  defender->SetGender(gender_t::MALE);
  defender->SetShortDesc(u8"defender");
  defender->Attach(std::make_shared<Mind>(mind_t::TEST));
  Object* def_sword = give_sword(defender);
  def_sword->SetLongDesc(u8"a defensive arming sword");

  auto bystander = test_person(room);
  bystander->SetGender(gender_t::NEITHER);
  bystander->SetShortDesc(u8"bystander");
  bystander->Attach(std::make_shared<Mind>(mind_t::TEST));

  current_time = 1001001001; // Don't start at time zero, since fights don't.

  SECTION("Wield Then Attack") {
    handle_command(attacker, u8"wield my sword", mind);

    attacker->SetLongDesc(u8"");
    defender->SetLongDesc(u8"");
    bystander->SetLongDesc(u8"");
    handle_command(attacker, u8"attack defender", mind);
    REQUIRE(witness(attacker) == u8"You move to attack defender.\n");
    REQUIRE(witness(defender) == u8"Attacker moves to attack you.\n");
    REQUIRE(witness(bystander) == u8"Attacker moves to attack defender.\n");

    attacker->SetLongDesc(u8"");
    defender->SetLongDesc(u8"");
    bystander->SetLongDesc(u8"");
    current_time += 3000000; // Next combat round
    Object::FreeActions(); // Execute next combat round's actions
    REQUIRE(
        witness(attacker) ==
        u8"Defender wields his arming sword.\n"
        u8"Defender tries to attack you, but misses.\n"
        u8"You hit defender in the chest with an arming sword.\n");
    REQUIRE(
        witness(defender) ==
        u8"You wield your arming sword.\n"
        u8"You try to attack attacker, but miss.\n"
        u8"Attacker hits you in the chest with an arming sword.\n");
    REQUIRE(
        witness(bystander) ==
        u8"Defender wields his arming sword.\n"
        u8"Defender tries to attack attacker, but misses.\n"
        u8"Attacker hits defender in the chest with an arming sword.\n");

    attacker->SetLongDesc(u8"");
    defender->SetLongDesc(u8"");
    bystander->SetLongDesc(u8"");
    current_time += 3000000; // Next combat round
    Object::FreeActions(); // Execute next combat round's actions
    REQUIRE(
        witness(attacker) ==
        u8"Defender tries to attack you, but misses.\n"
        u8"You hit defender in the chest with an arming sword.\n");

    attacker->SetLongDesc(u8"");
    defender->SetLongDesc(u8"");
    bystander->SetLongDesc(u8"");
    current_time += 3000000; // Next combat round
    Object::FreeActions(); // Execute next combat round's actions
    REQUIRE(
        witness(attacker) ==
        u8"You hit defender in the chest with an arming sword.\n"
        u8"Defender tries to attack you, but misses.\n");

    // Skip to end of fight
    for (int rounds = 0; rounds < 7; ++rounds) {
      current_time += 3000000; // Next combat round
      Object::FreeActions(); // Execute next combat round's actions
    }

    attacker->SetLongDesc(u8"");
    defender->SetLongDesc(u8"");
    bystander->SetLongDesc(u8"");
    current_time += 3000000; // Next combat round
    Object::FreeActions(); // Execute next combat round's actions
    REQUIRE(
        witness(attacker) ==
        u8"You hit defender in the chest with an arming sword.\n"
        u8"Defender collapses, bleeding and dying!\n"
        u8"Defender collapses!\n"
        u8"Defender drops an arming sword!\n");
  }

  SECTION("Normal Draw and Attack") {
    handle_command(attacker, u8"attack defender", mind);
    REQUIRE(
        witness(attacker) ==
        u8"You wield your arming sword.\n"
        u8"You move to attack defender.\n");
    REQUIRE(
        witness(defender) ==
        u8"Attacker wields her arming sword.\n"
        u8"Attacker moves to attack you.\n");
    REQUIRE(
        witness(bystander) ==
        u8"Attacker wields her arming sword.\n"
        u8"Attacker moves to attack defender.\n");

    attacker->SetLongDesc(u8"");
    defender->SetLongDesc(u8"");
    bystander->SetLongDesc(u8"");
    current_time += 3000000; // Next combat round
    Object::FreeActions(); // Execute next combat round's actions
    REQUIRE(
        witness(attacker) ==
        u8"Defender wields his arming sword.\n"
        u8"Defender tries to attack you, but misses.\n"
        u8"You hit defender in the chest with an arming sword.\n");
    REQUIRE(
        witness(defender) ==
        u8"You wield your arming sword.\n"
        u8"You try to attack attacker, but miss.\n"
        u8"Attacker hits you in the chest with an arming sword.\n");
    REQUIRE(
        witness(bystander) ==
        u8"Defender wields his arming sword.\n"
        u8"Defender tries to attack attacker, but misses.\n"
        u8"Attacker hits defender in the chest with an arming sword.\n");

    attacker->SetLongDesc(u8"");
    defender->SetLongDesc(u8"");
    bystander->SetLongDesc(u8"");
    current_time += 3000000; // Next combat round
    Object::FreeActions(); // Execute next combat round's actions
    REQUIRE(
        witness(attacker) ==
        u8"Defender tries to attack you, but misses.\n"
        u8"You hit defender in the chest with an arming sword.\n");

    attacker->SetLongDesc(u8"");
    defender->SetLongDesc(u8"");
    bystander->SetLongDesc(u8"");
    current_time += 3000000; // Next combat round
    Object::FreeActions(); // Execute next combat round's actions
    REQUIRE(
        witness(attacker) ==
        u8"You hit defender in the chest with an arming sword.\n"
        u8"Defender tries to attack you, but misses.\n");

    // Skip to end of fight
    for (int rounds = 0; rounds < 7; ++rounds) {
      current_time += 3000000; // Next combat round
      Object::FreeActions(); // Execute next combat round's actions
    }

    attacker->SetLongDesc(u8"");
    defender->SetLongDesc(u8"");
    bystander->SetLongDesc(u8"");
    current_time += 3000000; // Next combat round
    Object::FreeActions(); // Execute next combat round's actions
    REQUIRE(
        witness(attacker) ==
        u8"You hit defender in the chest with an arming sword.\n"
        u8"Defender collapses, bleeding and dying!\n"
        u8"Defender collapses!\n"
        u8"Defender drops an arming sword!\n");
  }

  SECTION("Draw and Attack with 'kill' Command") {
    handle_command(attacker, u8"kill defender", mind);
    REQUIRE(
        witness(attacker) ==
        u8"You wield your arming sword.\n"
        u8"You move to attack defender.\n");
    REQUIRE(
        witness(defender) ==
        u8"Attacker wields her arming sword.\n"
        u8"Attacker moves to attack you.\n");
    REQUIRE(
        witness(bystander) ==
        u8"Attacker wields her arming sword.\n"
        u8"Attacker moves to attack defender.\n");

    attacker->SetLongDesc(u8"");
    defender->SetLongDesc(u8"");
    bystander->SetLongDesc(u8"");
    current_time += 3000000; // Next combat round
    Object::FreeActions(); // Execute next combat round's actions
    REQUIRE(
        witness(attacker) ==
        u8"Defender wields his arming sword.\n"
        u8"Defender tries to attack you, but misses.\n"
        u8"You hit defender in the chest with an arming sword.\n");
    REQUIRE(
        witness(defender) ==
        u8"You wield your arming sword.\n"
        u8"You try to attack attacker, but miss.\n"
        u8"Attacker hits you in the chest with an arming sword.\n");
    REQUIRE(
        witness(bystander) ==
        u8"Defender wields his arming sword.\n"
        u8"Defender tries to attack attacker, but misses.\n"
        u8"Attacker hits defender in the chest with an arming sword.\n");

    attacker->SetLongDesc(u8"");
    defender->SetLongDesc(u8"");
    bystander->SetLongDesc(u8"");
    current_time += 3000000; // Next combat round
    Object::FreeActions(); // Execute next combat round's actions
    REQUIRE(
        witness(attacker) ==
        u8"Defender tries to attack you, but misses.\n"
        u8"You hit defender in the chest with an arming sword.\n");

    attacker->SetLongDesc(u8"");
    defender->SetLongDesc(u8"");
    bystander->SetLongDesc(u8"");
    current_time += 3000000; // Next combat round
    Object::FreeActions(); // Execute next combat round's actions
    REQUIRE(
        witness(attacker) ==
        u8"You hit defender in the chest with an arming sword.\n"
        u8"Defender tries to attack you, but misses.\n");

    // Skip to end of fight
    for (int rounds = 0; rounds < 7; ++rounds) {
      current_time += 3000000; // Next combat round
      Object::FreeActions(); // Execute next combat round's actions
    }

    attacker->SetLongDesc(u8"");
    defender->SetLongDesc(u8"");
    bystander->SetLongDesc(u8"");
    current_time += 3000000; // Next combat round
    Object::FreeActions(); // Execute next combat round's actions
    REQUIRE(
        witness(attacker) ==
        u8"You hit defender in the chest with an arming sword.\n"
        u8"Defender collapses, bleeding and dying!\n"
        u8"Defender collapses!\n"
        u8"Defender drops an arming sword!\n");
  }

  SECTION("Draw and Attack with 'kick' Command") {
    handle_command(attacker, u8"kick defender", mind);
    REQUIRE(
        witness(attacker) ==
        u8"You wield your arming sword.\n"
        u8"You move to attack defender.\n");
    REQUIRE(
        witness(defender) ==
        u8"Attacker wields her arming sword.\n"
        u8"Attacker moves to attack you.\n");
    REQUIRE(
        witness(bystander) ==
        u8"Attacker wields her arming sword.\n"
        u8"Attacker moves to attack defender.\n");

    attacker->SetLongDesc(u8"");
    defender->SetLongDesc(u8"");
    bystander->SetLongDesc(u8"");
    current_time += 3000000; // Next combat round
    Object::FreeActions(); // Execute next combat round's actions
    REQUIRE(
        witness(attacker) ==
        u8"Defender wields his arming sword.\n"
        u8"Defender tries to attack you, but misses.\n"
        u8"You try to kick defender, but miss.\n");
    REQUIRE(
        witness(defender) ==
        u8"You wield your arming sword.\n"
        u8"You try to attack attacker, but miss.\n"
        u8"Attacker tries to kick you, but misses.\n");
    REQUIRE(
        witness(bystander) ==
        u8"Defender wields his arming sword.\n"
        u8"Defender tries to attack attacker, but misses.\n"
        u8"Attacker tries to kick defender, but misses.\n");

    attacker->SetLongDesc(u8"");
    defender->SetLongDesc(u8"");
    bystander->SetLongDesc(u8"");
    current_time += 3000000; // Next combat round
    Object::FreeActions(); // Execute next combat round's actions
    REQUIRE(
        witness(attacker) ==
        u8"You hit defender in the chest with an arming sword.\n"
        u8"Defender tries to attack you, but misses.\n");

    // Skip to end of fight
    for (int rounds = 0; rounds < 14; ++rounds) {
      current_time += 3000000; // Next combat round
      Object::FreeActions(); // Execute next combat round's actions
    }

    attacker->SetLongDesc(u8"");
    defender->SetLongDesc(u8"");
    bystander->SetLongDesc(u8"");
    current_time += 3000000; // Next combat round
    Object::FreeActions(); // Execute next combat round's actions
    REQUIRE(
        witness(attacker) ==
        u8"You hit defender in the head with an arming sword.\n"
        u8"Defender collapses, bleeding and dying!\n"
        u8"Defender collapses!\n"
        u8"Defender drops an arming sword!\n");
  }

  SECTION("Draw and Attack with 'punch' Command") {
    handle_command(attacker, u8"punch defender", mind);
    REQUIRE(
        witness(attacker) ==
        u8"You wield your arming sword.\n"
        u8"You move to attack defender.\n");
    REQUIRE(
        witness(defender) ==
        u8"Attacker wields her arming sword.\n"
        u8"Attacker moves to attack you.\n");
    REQUIRE(
        witness(bystander) ==
        u8"Attacker wields her arming sword.\n"
        u8"Attacker moves to attack defender.\n");

    attacker->SetLongDesc(u8"");
    defender->SetLongDesc(u8"");
    bystander->SetLongDesc(u8"");
    current_time += 3000000; // Next combat round
    Object::FreeActions(); // Execute next combat round's actions
    REQUIRE(
        witness(attacker) ==
        u8"Defender wields his arming sword.\n"
        u8"Defender tries to attack you, but misses.\n"
        u8"You hit defender in the chest with an arming sword.\n");
    REQUIRE(
        witness(defender) ==
        u8"You wield your arming sword.\n"
        u8"You try to attack attacker, but miss.\n"
        u8"Attacker hits you in the chest with an arming sword.\n");
    REQUIRE(
        witness(bystander) ==
        u8"Defender wields his arming sword.\n"
        u8"Defender tries to attack attacker, but misses.\n"
        u8"Attacker hits defender in the chest with an arming sword.\n");

    attacker->SetLongDesc(u8"");
    defender->SetLongDesc(u8"");
    bystander->SetLongDesc(u8"");
    current_time += 3000000; // Next combat round
    Object::FreeActions(); // Execute next combat round's actions
    REQUIRE(
        witness(attacker) ==
        u8"Defender tries to attack you, but misses.\n"
        u8"You hit defender in the chest with an arming sword.\n");

    attacker->SetLongDesc(u8"");
    defender->SetLongDesc(u8"");
    bystander->SetLongDesc(u8"");
    current_time += 3000000; // Next combat round
    Object::FreeActions(); // Execute next combat round's actions
    REQUIRE(
        witness(attacker) ==
        u8"You hit defender in the chest with an arming sword.\n"
        u8"Defender tries to attack you, but misses.\n");

    // Skip to end of fight
    for (int rounds = 0; rounds < 7; ++rounds) {
      current_time += 3000000; // Next combat round
      Object::FreeActions(); // Execute next combat round's actions
    }

    attacker->SetLongDesc(u8"");
    defender->SetLongDesc(u8"");
    bystander->SetLongDesc(u8"");
    current_time += 3000000; // Next combat round
    Object::FreeActions(); // Execute next combat round's actions
    REQUIRE(
        witness(attacker) ==
        u8"You hit defender in the chest with an arming sword.\n"
        u8"Defender collapses, bleeding and dying!\n"
        u8"Defender collapses!\n"
        u8"Defender drops an arming sword!\n");
  }

  destroy_universe();
}
