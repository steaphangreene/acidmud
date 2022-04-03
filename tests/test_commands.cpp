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

static void give_money(Object* owner, ssize_t pp, ssize_t gp, ssize_t sp, ssize_t cp) {
  if (pp > 0) {
    Object* platinum = new Object(owner);
    platinum->SetShortDesc(u8"a platinum piece");
    platinum->SetValue(10000);
    platinum->SetSkill(prhash(u8"Money"), 10000);
    platinum->SetQuantity(pp);
  }
  if (gp > 0) {
    Object* gold = new Object(owner);
    gold->SetShortDesc(u8"a gold piece");
    gold->SetValue(1000);
    gold->SetSkill(prhash(u8"Money"), 1000);
    gold->SetQuantity(gp);
  }
  if (sp > 0) {
    Object* silver = new Object(owner);
    silver->SetShortDesc(u8"a silver piece");
    silver->SetValue(100);
    silver->SetSkill(prhash(u8"Money"), 100);
    silver->SetQuantity(sp);
  }
  if (cp > 0) {
    Object* copper = new Object(owner);
    copper->SetShortDesc(u8"a copper piece");
    copper->SetValue(10);
    copper->SetSkill(prhash(u8"Money"), 10);
    copper->SetQuantity(cp);
  }
}

TEST_CASE("Shop Commands", "[commands]") {
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

  auto shopkeeper = new Object(room);
  shopkeeper->SetShortDesc(u8"shopkeeper");
  shopkeeper->AddTag(crc32c(u8"master"));
  shopkeeper->AddTag(crc32c(u8"sawyer"));
  shopkeeper->SetAttribute(0, 4);
  shopkeeper->SetAttribute(1, 4);
  shopkeeper->SetAttribute(2, 4);
  shopkeeper->SetAttribute(3, 4);
  shopkeeper->SetAttribute(4, 4);
  shopkeeper->SetAttribute(5, 4);
  shopkeeper->SetPosition(pos_t::STAND);
  shopkeeper->AddAct(act_t::SPECIAL_WORK, room);
  shopkeeper->AddAct(act_t::WORK);
  shopkeeper->Attach(std::make_shared<Mind>(mind_t::NPC));
  give_money(shopkeeper, 3, 3, 3, 3);

  auto customer = new Object(room);
  customer->SetShortDesc(u8"customer");
  customer->SetAttribute(0, 4);
  customer->SetAttribute(1, 4);
  customer->SetAttribute(2, 4);
  customer->SetAttribute(3, 4);
  customer->SetAttribute(4, 4);
  customer->SetAttribute(5, 4);
  customer->SetPosition(pos_t::STAND);
  customer->Attach(mind);
  give_money(customer, 3, 3, 3, 3);

  auto equip = new Object(room);
  equip->SetShortDesc(u8"equip");
  equip->AddTag(crc32c(u8"sawmill"));

  auto item1 = new Object(room);
  item1->SetShortDesc(u8"item1");
  item1->SetValue(220);
  item1->AddAct(act_t::SPECIAL_OWNER, room);
  item1->SetPosition(pos_t::PROP);
  item1->SetSkill(prhash(u8"Pure Wood"), 100000);

  auto item2 = new Object(customer);
  item2->SetShortDesc(u8"item2");
  item2->SetValue(120);
  item2->SetPosition(pos_t::LIE);
  item2->SetSkill(prhash(u8"Pure Wood"), 100000);

  SECTION("List Items") {
    handle_command(customer, u8"list", mind);
    REQUIRE(!shopkeeper->HasWithin(item1));
    REQUIRE(!customer->HasWithin(item1));
    REQUIRE(item1->Parent() == room);
    REQUIRE(!shopkeeper->HasWithin(item2));
    REQUIRE(customer->HasWithin(item2));
    REQUIRE(item2->Parent() != room);
    REQUIRE(customer->LongDesc().contains(u8"    22cp: item1\n"));
  }

  SECTION("List Items Without Seller") {
    shopkeeper->StopAct(act_t::WORK);
    handle_command(customer, u8"list", mind);
    REQUIRE(!shopkeeper->HasWithin(item1));
    REQUIRE(!customer->HasWithin(item1));
    REQUIRE(item1->Parent() == room);
    REQUIRE(!shopkeeper->HasWithin(item2));
    REQUIRE(customer->HasWithin(item2));
    REQUIRE(item2->Parent() != room);
    REQUIRE(customer->LongDesc() == u8"You can only do that around a shopkeeper.");
  }

  SECTION("Buy Item") {
    handle_command(customer, u8"buy item1", mind);
    REQUIRE(!shopkeeper->HasWithin(item1));
    REQUIRE(customer->HasWithin(item1));
    REQUIRE(item1->Parent() != room);
    REQUIRE(!shopkeeper->HasWithin(item2));
    REQUIRE(customer->HasWithin(item2));
    REQUIRE(item2->Parent() != room);
    REQUIRE(customer->LongDesc().ends_with(u8"You buy and hold your item1."));
  }

  SECTION("Buy Item Without Seller") {
    shopkeeper->StopAct(act_t::WORK);
    handle_command(customer, u8"buy item1", mind);
    REQUIRE(!shopkeeper->HasWithin(item1));
    REQUIRE(!customer->HasWithin(item1));
    REQUIRE(item1->Parent() == room);
    REQUIRE(!shopkeeper->HasWithin(item2));
    REQUIRE(customer->HasWithin(item2));
    REQUIRE(item2->Parent() != room);
    REQUIRE(customer->LongDesc() == u8"You can only do that around a shopkeeper.");
  }

  SECTION("Value Item") {
    handle_command(customer, u8"value item2", mind);
    REQUIRE(!shopkeeper->HasWithin(item1));
    REQUIRE(!customer->HasWithin(item1));
    REQUIRE(item1->Parent() == room);
    REQUIRE(!shopkeeper->HasWithin(item2));
    REQUIRE(customer->HasWithin(item2));
    REQUIRE(item2->Parent() != room);
    REQUIRE(customer->LongDesc() == u8"I'll give you 12cp for item2");
  }

  SECTION("Value Item Without Seller") {
    shopkeeper->StopAct(act_t::WORK);
    handle_command(customer, u8"value item2", mind);
    REQUIRE(!shopkeeper->HasWithin(item1));
    REQUIRE(!customer->HasWithin(item1));
    REQUIRE(item1->Parent() == room);
    REQUIRE(!shopkeeper->HasWithin(item2));
    REQUIRE(customer->HasWithin(item2));
    REQUIRE(item2->Parent() != room);
    REQUIRE(customer->LongDesc() == u8"Sorry, nobody is buying that sort of thing here.");
  }

  SECTION("Sell Item") {
    handle_command(customer, u8"sell item2", mind);
    REQUIRE(!shopkeeper->HasWithin(item1));
    REQUIRE(!customer->HasWithin(item1));
    REQUIRE(item1->Parent() == room);
    REQUIRE(!shopkeeper->HasWithin(item2));
    REQUIRE(!customer->HasWithin(item2));
    REQUIRE(item2->Parent() == room);
    REQUIRE(customer->LongDesc().starts_with(u8"I'll give you 12cp for item2"));
    REQUIRE(customer->LongDesc().contains(u8"You sell your item2."));
  }

  SECTION("Sell Item Without Seller") {
    shopkeeper->StopAct(act_t::WORK);
    handle_command(customer, u8"sell item2", mind);
    REQUIRE(!shopkeeper->HasWithin(item1));
    REQUIRE(!customer->HasWithin(item1));
    REQUIRE(item1->Parent() == room);
    REQUIRE(!shopkeeper->HasWithin(item2));
    REQUIRE(customer->HasWithin(item2));
    REQUIRE(item2->Parent() != room);
    REQUIRE(customer->LongDesc() == u8"Sorry, nobody is buying that sort of thing here.");
  }

  destroy_universe();
}
