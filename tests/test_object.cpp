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
    REQUIRE(item1a1a1->Quantity() == 2);
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
  room1a2->SetShortDesc(u8"room1a2");
  auto room1b1 = new Object(zone1b);
  room1b1->SetShortDesc(u8"room1b1");
  auto room1b2 = new Object(zone1b);
  room1b2->SetShortDesc(u8"room1b2");
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

TEST_CASE("Object Text", "[object]") {
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
  auto room1b1 = new Object(zone1b);
  room1b1->SetShortDesc(u8"room1b1");
  auto room1b2 = new Object(zone1b);
  room1b2->SetShortDesc(u8"room1b2");
  auto obj1a1a = new Object(room1a1);
  obj1a1a->SetName(u8"Jane Doe");
  obj1a1a->SetShortDesc(u8"Test Object");
  auto obj1b1a = new Object(room1b1);
  obj1b1a->SetName(u8"John Doe");
  obj1b1a->SetShortDesc(u8"Test Object");
  auto item1a1a1 = new Object(obj1a1a);
  item1a1a1->SetShortDesc(u8"item1a1a1");
  auto item1b1a1 = new Object(obj1b1a);
  item1b1a1->SetShortDesc(u8"item1b1a1");

  REQUIRE(item1a1a1->ShortDesc() != item1b1a1->ShortDesc());
  REQUIRE(item1a1a1->Noun() != item1b1a1->Noun());

  REQUIRE(obj1a1a->ShortDesc() == obj1b1a->ShortDesc());
  REQUIRE(obj1a1a->Name() != obj1b1a->Name());
  REQUIRE(obj1a1a->Noun() != obj1b1a->Noun());
  REQUIRE(obj1a1a->Noun(false, false, obj1b1a) == obj1b1a->Noun(false, false, obj1a1a));
  REQUIRE(obj1a1a->Noun(false, true, obj1b1a) == obj1b1a->Noun(false, true, obj1a1a));
  REQUIRE(obj1a1a->Noun(false, true, obj1a1a) == obj1b1a->Noun(false, true, obj1b1a));

  obj1a1a->SetSkill(prhash(u8"Object ID"), 42);
  obj1b1a->Learn(42, u8"Jane Doe");

  // Now, Joe knows Jane's name, so these should be different.
  REQUIRE(obj1a1a->Noun(false, false, obj1b1a) != obj1b1a->Noun(false, false, obj1a1a));
  REQUIRE(obj1a1a->Noun(false, true, obj1b1a) != obj1b1a->Noun(false, true, obj1a1a));

  destroy_universe();
}

TEST_CASE("Object Text Limits", "[object]") {
  Object obj;

  obj.SetName(u8"Jane Doe");
  REQUIRE(obj.Name() == u8"Jane Doe");

  obj.SetShortDesc(u8"a female wizard");
  REQUIRE(obj.ShortDesc() == u8"a female wizard");

  obj.SetName( // 79 Chars: Ok
      u8"AAAAAAAAAAAAAAAAAAAA"
      u8"AAAAAAAAAAAAAAAAAAAA"
      u8"AAAAAAAAAAAAAAAAAAAA"
      u8"AAAAAAAAAAAAAAAAAAA");
  REQUIRE(
      obj.Name() ==
      u8"AAAAAAAAAAAAAAAAAAAA"
      u8"AAAAAAAAAAAAAAAAAAAA"
      u8"AAAAAAAAAAAAAAAAAAAA"
      u8"AAAAAAAAAAAAAAAAAAA");

  obj.SetName( // 80 Chars: Ok
      u8"AAAAAAAAAAAAAAAAAAAA"
      u8"AAAAAAAAAAAAAAAAAAAA"
      u8"AAAAAAAAAAAAAAAAAAAA"
      u8"AAAAAAAAAAAAAAAAAAAA");
  REQUIRE(
      obj.Name() ==
      u8"AAAAAAAAAAAAAAAAAAAA"
      u8"AAAAAAAAAAAAAAAAAAAA"
      u8"AAAAAAAAAAAAAAAAAAAA"
      u8"AAAAAAAAAAAAAAAAAAAA");

  obj.SetName( // 81 Chars: Truncate to 80
      u8"AAAAAAAAAAAAAAAAAAAA"
      u8"AAAAAAAAAAAAAAAAAAAA"
      u8"AAAAAAAAAAAAAAAAAAAA"
      u8"AAAAAAAAAAAAAAAAAAAAA");
  REQUIRE(
      obj.Name() ==
      u8"AAAAAAAAAAAAAAAAAAAA"
      u8"AAAAAAAAAAAAAAAAAAAA"
      u8"AAAAAAAAAAAAAAAAAAAA"
      u8"AAAAAAAAAAAAAAAAAAAA");

  obj.SetName( // More: Truncate to 80
      u8"AAAAAAAAAAAAAAAAAAAA"
      u8"AAAAAAAAAAAAAAAAAAAA"
      u8"AAAAAAAAAAAAAAAAAAAA"
      u8"AAAAAAAAAAAAAAAAAAAA"
      u8"AAAAAAAAAAAAAAAAAAAA"
      u8"AAAAAAAAAAAAAAAAAAAA"
      u8"AAAAAAAAAAAAAAAAAAAA"
      u8"AAAAAAAAAAAAAAAAAAAA");
  REQUIRE(
      obj.Name() ==
      u8"AAAAAAAAAAAAAAAAAAAA"
      u8"AAAAAAAAAAAAAAAAAAAA"
      u8"AAAAAAAAAAAAAAAAAAAA"
      u8"AAAAAAAAAAAAAAAAAAAA");

  obj.SetShortDesc( // 79 Chars: Ok
      u8"AAAAAAAAAAAAAAAAAAAA"
      u8"AAAAAAAAAAAAAAAAAAAA"
      u8"AAAAAAAAAAAAAAAAAAAA"
      u8"AAAAAAAAAAAAAAAAAAA");
  REQUIRE(
      obj.ShortDesc() ==
      u8"AAAAAAAAAAAAAAAAAAAA"
      u8"AAAAAAAAAAAAAAAAAAAA"
      u8"AAAAAAAAAAAAAAAAAAAA"
      u8"AAAAAAAAAAAAAAAAAAA");

  obj.SetShortDesc( // 80 Chars: Ok
      u8"AAAAAAAAAAAAAAAAAAAA"
      u8"AAAAAAAAAAAAAAAAAAAA"
      u8"AAAAAAAAAAAAAAAAAAAA"
      u8"AAAAAAAAAAAAAAAAAAAA");
  REQUIRE(
      obj.ShortDesc() ==
      u8"AAAAAAAAAAAAAAAAAAAA"
      u8"AAAAAAAAAAAAAAAAAAAA"
      u8"AAAAAAAAAAAAAAAAAAAA"
      u8"AAAAAAAAAAAAAAAAAAAA");

  obj.SetShortDesc( // 81 Chars: Truncate to 80
      u8"AAAAAAAAAAAAAAAAAAAA"
      u8"AAAAAAAAAAAAAAAAAAAA"
      u8"AAAAAAAAAAAAAAAAAAAA"
      u8"AAAAAAAAAAAAAAAAAAAAA");
  REQUIRE(
      obj.ShortDesc() ==
      u8"AAAAAAAAAAAAAAAAAAAA"
      u8"AAAAAAAAAAAAAAAAAAAA"
      u8"AAAAAAAAAAAAAAAAAAAA"
      u8"AAAAAAAAAAAAAAAAAAAA");

  obj.SetShortDesc( // More: Truncate to 80
      u8"AAAAAAAAAAAAAAAAAAAA"
      u8"AAAAAAAAAAAAAAAAAAAA"
      u8"AAAAAAAAAAAAAAAAAAAA"
      u8"AAAAAAAAAAAAAAAAAAAA"
      u8"AAAAAAAAAAAAAAAAAAAA"
      u8"AAAAAAAAAAAAAAAAAAAA"
      u8"AAAAAAAAAAAAAAAAAAAA"
      u8"AAAAAAAAAAAAAAAAAAAA");
  REQUIRE(
      obj.ShortDesc() ==
      u8"AAAAAAAAAAAAAAAAAAAA"
      u8"AAAAAAAAAAAAAAAAAAAA"
      u8"AAAAAAAAAAAAAAAAAAAA"
      u8"AAAAAAAAAAAAAAAAAAAA");
}

TEST_CASE("Object Money", "[money]") {
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
  auto jane = new Object(room1a1);
  jane->SetShortDesc(u8"a person");
  jane->SetName(u8"Jane Doe");
  auto john = new Object(room1a1);
  john->SetShortDesc(u8"a person");
  john->SetName(u8"John Doe");
  auto item1a1a1 = new Object(jane);
  item1a1a1->SetShortDesc(u8"item1a1a1");
  auto item1a1a2 = new Object(jane);
  item1a1a2->SetShortDesc(u8"item1a1a2");

  // Give Jane some coins.
  Object* chits1 = new Object(jane);
  chits1->SetShortDesc(u8"a bone chit");
  chits1->SetValue(1);
  chits1->SetSkill(prhash(u8"Money"), 1);
  chits1->SetQuantity(3);
  Object* silver = new Object(jane);
  silver->SetShortDesc(u8"a silver piece");
  silver->SetValue(100);
  silver->SetSkill(prhash(u8"Money"), 100);
  silver->SetQuantity(3);
  Object* platinum = new Object(jane);
  platinum->SetShortDesc(u8"a platinum piece");
  platinum->SetValue(10000);
  platinum->SetSkill(prhash(u8"Money"), 10000);
  platinum->SetQuantity(3);
  Object* copper = new Object(jane);
  copper->SetShortDesc(u8"a copper piece");
  copper->SetValue(10);
  copper->SetSkill(prhash(u8"Money"), 10);
  copper->SetQuantity(3);
  Object* gold = new Object(jane);
  gold->SetShortDesc(u8"a gold piece");
  gold->SetValue(1000);
  gold->SetSkill(prhash(u8"Money"), 1000);
  gold->SetQuantity(3);
  // Add a different flavor chit of the same value
  Object* chits2 = new Object(jane);
  chits2->SetShortDesc(u8"a wooden chit");
  chits2->SetValue(1);
  chits2->SetSkill(prhash(u8"Money"), 1);
  chits2->SetQuantity(3);
  // And some money with non-intrinsic value
  Object* dollar = new Object(jane);
  dollar->SetShortDesc(u8"a dollar");
  dollar->SetValue(1);
  dollar->SetSkill(prhash(u8"Money"), 100);
  dollar->SetQuantity(3);
  // And some completely non-intrinsic money
  Object* script = new Object(jane);
  script->SetShortDesc(u8"a credit of script");
  script->SetSkill(prhash(u8"Money"), 10);
  script->SetQuantity(3);
  // And some worthless fake money
  Object* paper = new Object(jane);
  paper->SetShortDesc(u8"a dollar of Monopoly money");
  paper->SetQuantity(3);

  SECTION("Simple Read-Only Checks") {
    REQUIRE(jane->CanPayFor(0) == 0);
    REQUIRE(jane->CanPayFor(1) == 1);
    REQUIRE(jane->CanPayFor(2) == 2);
    REQUIRE(jane->CanPayFor(3) == 3);
    REQUIRE(jane->CanPayFor(4) == 4);
    REQUIRE(jane->CanPayFor(5) == 5);
    REQUIRE(jane->CanPayFor(6) == 6);
    REQUIRE(jane->CanPayFor(7) == 10); // Can't Make Change
    REQUIRE(jane->CanPayFor(8) == 10); // Can't Make Change
    REQUIRE(jane->CanPayFor(9) == 10); // Can't Make Change
    REQUIRE(jane->CanPayFor(10) == 10);
    REQUIRE(jane->CanPayFor(11) == 11);

    REQUIRE(jane->CanPayFor(12) == 12);
    REQUIRE(jane->CanPayFor(120) == 120);
    REQUIRE(jane->CanPayFor(1200) == 1200);
    REQUIRE(jane->CanPayFor(12000) == 12000);
    REQUIRE(jane->CanPayFor(120000) == 33336); // Can't Afford

    REQUIRE(jane->CanPayFor(13) == 13);
    REQUIRE(jane->CanPayFor(130) == 130);
    REQUIRE(jane->CanPayFor(1300) == 1300);
    REQUIRE(jane->CanPayFor(13000) == 13000);
    REQUIRE(jane->CanPayFor(130000) == 33336); // Can't Afford

    REQUIRE(jane->CanPayFor(14) == 14);
    REQUIRE(jane->CanPayFor(140) == 200); // Can't Make Change
    REQUIRE(jane->CanPayFor(1400) == 2000); // Can't Make Change
    REQUIRE(jane->CanPayFor(14000) == 20000); // Can't Make Change
    REQUIRE(jane->CanPayFor(140000) == 33336); // Can't Afford

    REQUIRE(jane->CanPayFor(20) == 20);
    REQUIRE(jane->CanPayFor(200) == 200);
    REQUIRE(jane->CanPayFor(2000) == 2000);
    REQUIRE(jane->CanPayFor(20000) == 20000);
    REQUIRE(jane->CanPayFor(200000) == 33336); // Can't Afford

    REQUIRE(jane->CanPayFor(30) == 30);
    REQUIRE(jane->CanPayFor(300) == 300);
    REQUIRE(jane->CanPayFor(3000) == 3000);
    REQUIRE(jane->CanPayFor(30000) == 30000);
    REQUIRE(jane->CanPayFor(300000) == 33336); // Can't Afford

    REQUIRE(jane->CanPayFor(40) == 100); // Can't Make Change
    REQUIRE(jane->CanPayFor(400) == 1000); // Can't Make Change
    REQUIRE(jane->CanPayFor(4000) == 10000); // Can't Make Change
    REQUIRE(jane->CanPayFor(40000) == 33336); // Can't Afford
    REQUIRE(jane->CanPayFor(400000) == 33336); // Can't Afford

    REQUIRE(jane->CanPayFor(15) == 15);
    REQUIRE(jane->CanPayFor(16) == 16);
    REQUIRE(jane->CanPayFor(17) == 20); // Can't Make Change
    REQUIRE(jane->CanPayFor(18) == 20); // Can't Make Change
    REQUIRE(jane->CanPayFor(19) == 20); // Can't Make Change
    REQUIRE(jane->CanPayFor(20) == 20);

    REQUIRE(jane->CanPayFor(4000000000UL) == 33336); // Can't Afford
    REQUIRE(jane->CanPayFor(140000000000UL) == 33336); // Can't Afford
  }

  SECTION("Actual Payment Check") {
    REQUIRE(jane->CanPayFor(222) == 222);

    auto payment = jane->PayFor(1324);
    REQUIRE(payment.size() == 5);
    REQUIRE(jane->CanPayFor(222) == 222); // Still In Her Posession

    for (auto p : payment) {
      delete p;
    }
    REQUIRE(jane->CanPayFor(222) == 1000); // Can't Make Change Anymore
  }

  SECTION("Going Broke Check") {
    REQUIRE(jane->CanPayFor(22222) == 22222);

    auto payment = jane->PayFor(31324);
    REQUIRE(payment.size() == 6);
    REQUIRE(jane->CanPayFor(22222) == 22222); // Still In Her Posession

    for (auto p : payment) {
      delete p;
    }
    REQUIRE(jane->CanPayFor(22222) == 2012); // Can No Longer Afford It
  }

  SECTION("One Big Coin Check") {
    platinum = new Object(john);
    platinum->SetShortDesc(u8"a platinum piece");
    platinum->SetValue(10000);
    platinum->SetSkill(prhash(u8"Money"), 10000);

    REQUIRE(john->CanPayFor(222) == 10000);

    auto payment = john->PayFor(31324);
    REQUIRE(payment.size() == 0);
    REQUIRE(john->CanPayFor(222) == 10000); // Still In His Posession

    for (auto p : payment) {
      delete p;
    }
    REQUIRE(john->CanPayFor(10000) == 10000); // Still In His Posession
  }

  SECTION("Many Big Coins Check") {
    platinum = new Object(john);
    platinum->SetShortDesc(u8"a platinum piece");
    platinum->SetValue(10000);
    platinum->SetSkill(prhash(u8"Money"), 10000);
    platinum->SetQuantity(323);

    REQUIRE(john->CanPayFor(222) == 10000);

    auto payment = john->PayFor(31324);
    REQUIRE(payment.size() == 0);
    REQUIRE(john->CanPayFor(222) == 10000); // Still In His Posession

    for (auto p : payment) {
      delete p;
    }
    REQUIRE(john->CanPayFor(10000) == 10000); // Still In His Posession
  }

  destroy_universe();
}

TEST_CASE("Object Quantity", "[object]") {
  init_universe();
  REQUIRE(Object::Universe() != nullptr);

  // Note: Each must have a different ShortDesc to avoid being auto-combined.
  auto world = new Object(Object::Universe());
  world->SetShortDesc(u8"world");
  auto zone = new Object(world);
  zone->SetShortDesc(u8"zone");
  auto room = new Object(zone);
  room->SetShortDesc(u8"room");
  auto obj = new Object(room);
  obj->SetShortDesc(u8"obj");
  auto item1 = new Object(obj);
  item1->SetShortDesc(u8"item1");
  REQUIRE(item1->Quantity() == 1);
  auto item2 = new Object(obj);
  item2->SetShortDesc(u8"item2");
  item2->SetQuantity(4);
  REQUIRE(item2->Quantity() == 4);
  auto item3 = item2->Split(2);
  REQUIRE(item2->Quantity() == 2);
  REQUIRE(item3->Quantity() == 2);
  item3->TryCombine();
  REQUIRE(item3->Quantity() == 4);
  REQUIRE(item2->Parent() == Object::TrashBin());
  auto item4 = new Object(obj);
  item4->SetShortDesc(u8"item2");
  REQUIRE(item4->Quantity() == 1);
  item3->TryCombine();
  REQUIRE(item3->Quantity() == 5);
  REQUIRE(item4->Parent() == Object::TrashBin());
  auto item5 = item3->Split(1);
  REQUIRE(item3->Quantity() == 4);
  REQUIRE(item5->Quantity() == 1);
  auto item6 = item3->Split(3);
  REQUIRE(item3->Quantity() == 1);
  REQUIRE(item6->Quantity() == 3);
  item6->TryCombine();
  REQUIRE(item6->Quantity() == 5);
  REQUIRE(item3->Parent() == Object::TrashBin());
  REQUIRE(item5->Parent() == Object::TrashBin());

  destroy_universe();
}
