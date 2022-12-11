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
  person->AddTag(crc32c(u8"master"));
  person->AddTag(crc32c(u8"sawyer"));
  person->SetAttribute(0, 4);
  person->SetAttribute(1, 4);
  person->SetAttribute(2, 4);
  person->SetAttribute(3, 4);
  person->SetAttribute(4, 4);
  person->SetAttribute(5, 4);
  person->SetPosition(pos_t::STAND);
  return person;
}

static Object* give_purse(Object* owner) {
  Object* purse = new Object(owner);
  purse->SetShortDesc(u8"a small purse");
  purse->SetDesc(u8"A small, durable, practical moneypurse.");
  purse->SetSkill(prhash(u8"Wearable on Left Hip"), 1);
  purse->SetSkill(prhash(u8"Wearable on Right Hip"), 2);
  purse->SetSkill(prhash(u8"Container"), 5 * 454);
  purse->SetSkill(prhash(u8"Capacity"), 5);
  purse->SetSkill(prhash(u8"Closeable"), 1);
  purse->SetWeight(1 * 454);
  purse->SetSize(2);
  purse->SetVolume(1);
  purse->SetValue(100);
  purse->SetPosition(pos_t::LIE);
  owner->AddAct(act_t::WEAR_LHIP, purse);
  return purse;
}

static void give_money(Object* purse, size_t pp, size_t gp, size_t sp, size_t cp) {
  if (pp > 0) {
    Object* platinum = new Object(purse);
    platinum->SetShortDesc(u8"a platinum piece");
    platinum->SetValue(10000);
    platinum->SetSkill(prhash(u8"Money"), 10000);
    platinum->SetQuantity(pp);
  }
  if (gp > 0) {
    Object* gold = new Object(purse);
    gold->SetShortDesc(u8"a gold piece");
    gold->SetValue(1000);
    gold->SetSkill(prhash(u8"Money"), 1000);
    gold->SetQuantity(gp);
  }
  if (sp > 0) {
    Object* silver = new Object(purse);
    silver->SetShortDesc(u8"a silver piece");
    silver->SetValue(100);
    silver->SetSkill(prhash(u8"Money"), 100);
    silver->SetQuantity(sp);
  }
  if (cp > 0) {
    Object* copper = new Object(purse);
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

  auto shopkeeper = test_person(room);
  shopkeeper->SetShortDesc(u8"shopkeeper");
  shopkeeper->AddAct(act_t::SPECIAL_WORK, room);
  shopkeeper->AddAct(act_t::WORK);
  shopkeeper->Attach(std::make_shared<Mind>(mind_t::NPC));
  Object* shop_purse = give_purse(shopkeeper);
  give_money(shop_purse, 3, 3, 3, 3);

  auto customer = test_person(room);
  customer->SetShortDesc(u8"customer");
  customer->Attach(mind);
  Object* cust_purse = give_purse(customer);
  give_money(cust_purse, 3, 3, 3, 3);

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

  SECTION("Shop Commands With Missing Args") {
    handle_command(customer, u8"buy", mind);
    REQUIRE(witness(customer) == u8"What do you want to buy?\n");
    customer->SetLongDesc(u8"");

    handle_command(customer, u8"value", mind);
    REQUIRE(witness(customer) == u8"What do you want to sell?\n");
    customer->SetLongDesc(u8"");

    handle_command(customer, u8"sell", mind);
    REQUIRE(witness(customer) == u8"What do you want to sell?\n");
    customer->SetLongDesc(u8"");
  }

  SECTION("Shop Commands With Invalid Args") {
    handle_command(customer, u8"buy nothing", mind);
    REQUIRE(witness(customer) == u8"The shopkeeper doesn't have that.\n");
    customer->SetLongDesc(u8"");

    handle_command(customer, u8"value nothing", mind);
    REQUIRE(witness(customer) == u8"You want to sell what?\n");
    customer->SetLongDesc(u8"");

    handle_command(customer, u8"sell nothing", mind);
    REQUIRE(witness(customer) == u8"You want to sell what?\n");
    customer->SetLongDesc(u8"");
  }

  SECTION("List Items") {
    handle_command(customer, u8"list", mind);
    REQUIRE(!shopkeeper->HasWithin(item1));
    REQUIRE(!customer->HasWithin(item1));
    REQUIRE(item1->Parent() == room);
    REQUIRE(!shopkeeper->HasWithin(item2));
    REQUIRE(customer->HasWithin(item2));
    REQUIRE(item2->Parent() != room);
    REQUIRE(witness(customer) == u8"      22cp: item1\n");
  }

  SECTION("List Items With Profit") {
    shopkeeper->SetSkill(prhash(u8"Sell Profit"), 2000);
    handle_command(customer, u8"list", mind);
    REQUIRE(!shopkeeper->HasWithin(item1));
    REQUIRE(!customer->HasWithin(item1));
    REQUIRE(item1->Parent() == room);
    REQUIRE(!shopkeeper->HasWithin(item2));
    REQUIRE(customer->HasWithin(item2));
    REQUIRE(item2->Parent() != room);
    REQUIRE(witness(customer) == u8"      44cp: item1\n");
  }

  SECTION("List Items Missing Mind") {
    handle_command(customer, u8"list");
    REQUIRE(!shopkeeper->HasWithin(item1));
    REQUIRE(!customer->HasWithin(item1));
    REQUIRE(item1->Parent() == room);
    REQUIRE(!shopkeeper->HasWithin(item2));
    REQUIRE(customer->HasWithin(item2));
    REQUIRE(item2->Parent() != room);
    REQUIRE(witness(customer) == u8"");
  }

  SECTION("List Plural-Quantity Items") {
    item1->SetQuantity(3);
    handle_command(customer, u8"list", mind);
    REQUIRE(!shopkeeper->HasWithin(item1));
    REQUIRE(!customer->HasWithin(item1));
    REQUIRE(item1->Parent() == room);
    REQUIRE(!shopkeeper->HasWithin(item2));
    REQUIRE(customer->HasWithin(item2));
    REQUIRE(item2->Parent() != room);
    REQUIRE(witness(customer) == u8"      22cp: item1 (x3)\n");
  }

  SECTION("List With A Not-For-Sale Item") {
    handle_command(customer, u8"sell item2", mind);
    customer->SetLongDesc(u8"");
    handle_command(customer, u8"list", mind);
    REQUIRE(!shopkeeper->HasWithin(item1));
    REQUIRE(!customer->HasWithin(item1));
    REQUIRE(item1->Parent() == room);
    REQUIRE(!shopkeeper->HasWithin(item2));
    REQUIRE(!customer->HasWithin(item2));
    REQUIRE(item2->Parent() == room);
    REQUIRE(witness(customer) == u8"      22cp: item1\n");
  }

  SECTION("List With No Items") {
    delete item1;
    handle_command(customer, u8"list", mind);
    REQUIRE(!shopkeeper->HasWithin(item2));
    REQUIRE(customer->HasWithin(item2));
    REQUIRE(item2->Parent() != room);
    REQUIRE(witness(customer) == u8"Nothing is for sale here, sorry.\n");
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
    REQUIRE(witness(customer) == u8"You can only do that around a shopkeeper.\n");
  }

  SECTION("List From Dead Shopkeeper") {
    shopkeeper->AddAct(act_t::DEAD);
    handle_command(customer, u8"list", mind);
    REQUIRE(!shopkeeper->HasWithin(item1));
    REQUIRE(!customer->HasWithin(item1));
    REQUIRE(item1->Parent() == room);
    REQUIRE(!shopkeeper->HasWithin(item2));
    REQUIRE(customer->HasWithin(item2));
    REQUIRE(item2->Parent() != room);
    REQUIRE(witness(customer) == u8"Sorry, the shopkeeper is dead!\n");
  }

  SECTION("List From Dying Shopkeeper") {
    shopkeeper->AddAct(act_t::DYING);
    handle_command(customer, u8"list", mind);
    REQUIRE(!shopkeeper->HasWithin(item1));
    REQUIRE(!customer->HasWithin(item1));
    REQUIRE(item1->Parent() == room);
    REQUIRE(!shopkeeper->HasWithin(item2));
    REQUIRE(customer->HasWithin(item2));
    REQUIRE(item2->Parent() != room);
    REQUIRE(witness(customer) == u8"Sorry, the shopkeeper is dying!\n");
  }

  SECTION("List From Unconscious Shopkeeper") {
    shopkeeper->AddAct(act_t::UNCONSCIOUS);
    handle_command(customer, u8"list", mind);
    REQUIRE(!shopkeeper->HasWithin(item1));
    REQUIRE(!customer->HasWithin(item1));
    REQUIRE(item1->Parent() == room);
    REQUIRE(!shopkeeper->HasWithin(item2));
    REQUIRE(customer->HasWithin(item2));
    REQUIRE(item2->Parent() != room);
    REQUIRE(witness(customer) == u8"Sorry, the shopkeeper is unconscious!\n");
  }

  SECTION("List From Sleeping Shopkeeper") {
    shopkeeper->AddAct(act_t::SLEEP);
    handle_command(customer, u8"list", mind);
    REQUIRE(!shopkeeper->HasWithin(item1));
    REQUIRE(!customer->HasWithin(item1));
    REQUIRE(item1->Parent() == room);
    REQUIRE(!shopkeeper->HasWithin(item2));
    REQUIRE(customer->HasWithin(item2));
    REQUIRE(item2->Parent() != room);
    REQUIRE(witness(customer) == u8"Sorry, the shopkeeper is asleep!\n");
  }

  SECTION("Buy Item") {
    handle_command(customer, u8"buy item1", mind);
    REQUIRE(!shopkeeper->HasWithin(item1));
    REQUIRE(customer->HasWithin(item1));
    REQUIRE(item1->Parent() != room);
    REQUIRE(!shopkeeper->HasWithin(item2));
    REQUIRE(customer->HasWithin(item2));
    REQUIRE(item2->Parent() != room);
    REQUIRE(
        witness(customer) ==
        u8"22cp: item1\n"
        u8"You buy and stash your item1.\n");
  }

  SECTION("Buy Item Missing Mind") {
    handle_command(customer, u8"buy item1");
    REQUIRE(!shopkeeper->HasWithin(item1));
    REQUIRE(customer->HasWithin(item1));
    REQUIRE(item1->Parent() != room);
    REQUIRE(!shopkeeper->HasWithin(item2));
    REQUIRE(customer->HasWithin(item2));
    REQUIRE(item2->Parent() != room);
    REQUIRE(witness(customer) == u8"You buy and stash your item1.\n");
  }

  SECTION("Buy Item With Profit") {
    shopkeeper->SetSkill(prhash(u8"Sell Profit"), 2000);
    give_money(customer, 3, 3, 3, 3);
    handle_command(customer, u8"buy item1", mind);
    REQUIRE(!shopkeeper->HasWithin(item1));
    REQUIRE(customer->HasWithin(item1));
    REQUIRE(item1->Parent() != room);
    REQUIRE(!shopkeeper->HasWithin(item2));
    REQUIRE(customer->HasWithin(item2));
    REQUIRE(item2->Parent() != room);
    REQUIRE(
        witness(customer) ==
        u8"44cp: item1\n"
        u8"You buy and stash your item1.\n");
  }

  SECTION("Buy Item With Too Much Profit") {
    shopkeeper->SetSkill(prhash(u8"Sell Profit"), 2000);
    handle_command(customer, u8"buy item1", mind);
    REQUIRE(!shopkeeper->HasWithin(item1));
    REQUIRE(!customer->HasWithin(item1));
    REQUIRE(item1->Parent() == room);
    REQUIRE(!shopkeeper->HasWithin(item2));
    REQUIRE(customer->HasWithin(item2));
    REQUIRE(item2->Parent() != room);
    REQUIRE(
        witness(customer) ==
        u8"44cp: item1\n"
        u8"You don't have change to pay 44cp (you'd have to pay 1gp).\n"
        u8"...and I can't make change for that, sorry.\n");
  }

  SECTION("Buy Item Too Heavy For Storage") {
    item1->SetWeight(5000);
    handle_command(customer, u8"buy item1", mind);
    REQUIRE(!shopkeeper->HasWithin(item1));
    REQUIRE(customer->HasWithin(item1));
    REQUIRE(item1->Parent() != room);
    REQUIRE(customer->ActTarg(act_t::HOLD) == item1);
    REQUIRE(!shopkeeper->HasWithin(item2));
    REQUIRE(customer->HasWithin(item2));
    REQUIRE(item2->Parent() != room);
    REQUIRE(
        witness(customer) ==
        u8"22cp: item1\n"
        u8"You buy and hold your item1.\n");
  }

  SECTION("Buy Item Too Big For Storage") {
    item1->SetVolume(50);
    handle_command(customer, u8"buy item1", mind);
    REQUIRE(!shopkeeper->HasWithin(item1));
    REQUIRE(customer->HasWithin(item1));
    REQUIRE(item1->Parent() != room);
    REQUIRE(customer->ActTarg(act_t::HOLD) == item1);
    REQUIRE(!shopkeeper->HasWithin(item2));
    REQUIRE(customer->HasWithin(item2));
    REQUIRE(item2->Parent() != room);
    REQUIRE(
        witness(customer) ==
        u8"22cp: item1\n"
        u8"You buy and hold your item1.\n");
  }

  SECTION("Buy Item Requiring Release of Shield") {
    item2->SetVolume(50);
    customer->AddAct(act_t::WEAR_SHIELD, item2);
    customer->AddAct(act_t::HOLD, item2);
    item1->SetVolume(50);
    handle_command(customer, u8"buy item1", mind);
    REQUIRE(!shopkeeper->HasWithin(item1));
    REQUIRE(customer->HasWithin(item1));
    REQUIRE(item1->Parent() != room);
    REQUIRE(customer->ActTarg(act_t::HOLD) == item1);
    REQUIRE(customer->ActTarg(act_t::WEAR_SHIELD) == item2);
    REQUIRE(!shopkeeper->HasWithin(item2));
    REQUIRE(customer->HasWithin(item2));
    REQUIRE(item2->Parent() != room);
    REQUIRE(
        witness(customer) ==
        u8"22cp: item1\n"
        u8"You stop holding your item2.\n"
        u8"You buy and hold your item1.\n");
  }

  SECTION("Buy Item Can't Store Or Hold") {
    item2->SetVolume(50);
    customer->AddAct(act_t::HOLD, item2);
    item1->SetVolume(50);
    handle_command(customer, u8"buy item1", mind);
    REQUIRE(!shopkeeper->HasWithin(item1));
    REQUIRE(!customer->HasWithin(item1));
    REQUIRE(item1->Parent() == room);
    REQUIRE(customer->ActTarg(act_t::HOLD) == item2);
    REQUIRE(!shopkeeper->HasWithin(item2));
    REQUIRE(customer->HasWithin(item2));
    REQUIRE(item2->Parent() != room);
    REQUIRE(
        witness(customer) ==
        u8"22cp: item1\n"
        u8"You can't stash or hold item1.\n");
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
    REQUIRE(witness(customer) == u8"You can only do that around a shopkeeper.\n");
  }

  SECTION("Buy From Dead Shopkeeper") {
    shopkeeper->AddAct(act_t::DEAD);
    handle_command(customer, u8"buy item1", mind);
    REQUIRE(!shopkeeper->HasWithin(item1));
    REQUIRE(!customer->HasWithin(item1));
    REQUIRE(item1->Parent() == room);
    REQUIRE(!shopkeeper->HasWithin(item2));
    REQUIRE(customer->HasWithin(item2));
    REQUIRE(item2->Parent() != room);
    REQUIRE(witness(customer) == u8"Sorry, the shopkeeper is dead!\n");
  }

  SECTION("Buy From Dying Shopkeeper") {
    shopkeeper->AddAct(act_t::DYING);
    handle_command(customer, u8"buy item1", mind);
    REQUIRE(!shopkeeper->HasWithin(item1));
    REQUIRE(!customer->HasWithin(item1));
    REQUIRE(item1->Parent() == room);
    REQUIRE(!shopkeeper->HasWithin(item2));
    REQUIRE(customer->HasWithin(item2));
    REQUIRE(item2->Parent() != room);
    REQUIRE(witness(customer) == u8"Sorry, the shopkeeper is dying!\n");
  }

  SECTION("Buy From Unconscious Shopkeeper") {
    shopkeeper->AddAct(act_t::UNCONSCIOUS);
    handle_command(customer, u8"buy item1", mind);
    REQUIRE(!shopkeeper->HasWithin(item1));
    REQUIRE(!customer->HasWithin(item1));
    REQUIRE(item1->Parent() == room);
    REQUIRE(!shopkeeper->HasWithin(item2));
    REQUIRE(customer->HasWithin(item2));
    REQUIRE(item2->Parent() != room);
    REQUIRE(witness(customer) == u8"Sorry, the shopkeeper is unconscious!\n");
  }

  SECTION("Buy From Sleeping Shopkeeper") {
    shopkeeper->AddAct(act_t::SLEEP);
    handle_command(customer, u8"buy item1", mind);
    REQUIRE(!shopkeeper->HasWithin(item1));
    REQUIRE(!customer->HasWithin(item1));
    REQUIRE(item1->Parent() == room);
    REQUIRE(!shopkeeper->HasWithin(item2));
    REQUIRE(customer->HasWithin(item2));
    REQUIRE(item2->Parent() != room);
    REQUIRE(witness(customer) == u8"Sorry, the shopkeeper is asleep!\n");
  }

  SECTION("Buy Item Without Any Coin") {
    handle_command(customer, u8"drop all", mind);
    customer->SetLongDesc(u8"");
    handle_command(customer, u8"buy item1", mind);
    REQUIRE(!shopkeeper->HasWithin(item1));
    REQUIRE(!customer->HasWithin(item1));
    REQUIRE(item1->Parent() == room);
    REQUIRE(!shopkeeper->HasWithin(item2));
    REQUIRE(!customer->HasWithin(item2));
    REQUIRE(item2->Parent() == room);
    REQUIRE(
        witness(customer) ==
        u8"22cp: item1\n"
        u8"You can't afford the 22cp (you only have 0).\n");
  }

  SECTION("Buy Item Without Enough Coin") {
    handle_command(customer, u8"drop all", mind);
    give_money(customer, 0, 0, 2, 1);
    customer->SetLongDesc(u8"");
    handle_command(customer, u8"buy item1", mind);
    REQUIRE(!shopkeeper->HasWithin(item1));
    REQUIRE(!customer->HasWithin(item1));
    REQUIRE(item1->Parent() == room);
    REQUIRE(!shopkeeper->HasWithin(item2));
    REQUIRE(!customer->HasWithin(item2));
    REQUIRE(item2->Parent() == room);
    REQUIRE(
        witness(customer) ==
        u8"22cp: item1\n"
        u8"You can't afford the 22cp (you only have 21cp).\n");
  }

  SECTION("Buy Item Without Enough Change") {
    handle_command(customer, u8"drop all", mind);
    give_money(customer, 1, 0, 0, 0);
    customer->SetLongDesc(u8"");
    handle_command(customer, u8"buy item1", mind);
    REQUIRE(!shopkeeper->HasWithin(item1));
    REQUIRE(!customer->HasWithin(item1));
    REQUIRE(item1->Parent() == room);
    REQUIRE(!shopkeeper->HasWithin(item2));
    REQUIRE(!customer->HasWithin(item2));
    REQUIRE(item2->Parent() == room);
    REQUIRE(
        witness(customer) ==
        u8"22cp: item1\n"
        u8"You don't have change to pay 22cp (you'd have to pay 1pp).\n"
        u8"...and I can't make change for that, sorry.\n");
  }

  SECTION("Buy Item Requiring Change") {
    handle_command(customer, u8"drop all", mind);
    give_money(customer, 0, 1, 0, 0);
    give_money(shopkeeper, 0, 0, 9, 9);
    customer->SetLongDesc(u8"");
    handle_command(customer, u8"buy item1", mind);
    REQUIRE(!shopkeeper->HasWithin(item1));
    REQUIRE(!customer->HasWithin(item1));
    REQUIRE(item1->Parent() == room);
    REQUIRE(!shopkeeper->HasWithin(item2));
    REQUIRE(!customer->HasWithin(item2));
    REQUIRE(item2->Parent() == room);
    REQUIRE(
        witness(customer) ==
        u8"22cp: item1\n"
        u8"You don't have change to pay 22cp (you'd have to pay 1gp).\n"
        u8"You get 78cp from shopkeeper.\n");
  }

  SECTION("Buy All With One For-Sale Item") {
    handle_command(customer, u8"buy all", mind);
    REQUIRE(!shopkeeper->HasWithin(item1));
    REQUIRE(!customer->HasWithin(item1));
    REQUIRE(item1->Parent() == room);
    REQUIRE(!shopkeeper->HasWithin(item2));
    REQUIRE(customer->HasWithin(item2));
    REQUIRE(item2->Parent() != room);
    REQUIRE(
        witness(customer) ==
        u8" Not For Sale: shopkeeper\n"
        u8" Not For Sale: equip\n"
        u8"Sorry, you can't buy that.\n");
  }

  SECTION("Buy All With A Not-For-Sale Item") {
    handle_command(customer, u8"sell item2", mind);
    customer->SetLongDesc(u8"");
    handle_command(customer, u8"buy all", mind);
    REQUIRE(!shopkeeper->HasWithin(item1));
    REQUIRE(!customer->HasWithin(item1));
    REQUIRE(item1->Parent() == room);
    REQUIRE(!shopkeeper->HasWithin(item2));
    REQUIRE(!customer->HasWithin(item2));
    REQUIRE(item2->Parent() == room);
    REQUIRE(
        witness(customer) ==
        u8" Not For Sale: shopkeeper\n"
        u8" Not For Sale: equip\n"
        u8" Not For Sale: item2\n"
        u8"Sorry, you can't buy that.\n");
  }

  SECTION("Buy Worthless Item") {
    item1->SetValue(0);
    handle_command(customer, u8"buy item1", mind);
    REQUIRE(!shopkeeper->HasWithin(item1));
    REQUIRE(!customer->HasWithin(item1));
    REQUIRE(item1->Parent() == room);
    REQUIRE(!shopkeeper->HasWithin(item2));
    REQUIRE(customer->HasWithin(item2));
    REQUIRE(item2->Parent() != room);
    REQUIRE(witness(customer) == u8"Item1 is worthless.\n");
  }

  SECTION("Buy Priceless Item") {
    item1->SetSkill(prhash(u8"Priceless"), 1);
    handle_command(customer, u8"buy item1", mind);
    REQUIRE(!shopkeeper->HasWithin(item1));
    REQUIRE(!customer->HasWithin(item1));
    REQUIRE(item1->Parent() == room);
    REQUIRE(!shopkeeper->HasWithin(item2));
    REQUIRE(customer->HasWithin(item2));
    REQUIRE(item2->Parent() != room);
    REQUIRE(witness(customer) == u8"You can't buy item1.\n");
  }

  SECTION("Buy Cursed Item") {
    item1->SetSkill(prhash(u8"Cursed"), 1);
    handle_command(customer, u8"buy item1", mind);
    REQUIRE(!shopkeeper->HasWithin(item1));
    REQUIRE(!customer->HasWithin(item1));
    REQUIRE(item1->Parent() == room);
    REQUIRE(!shopkeeper->HasWithin(item2));
    REQUIRE(customer->HasWithin(item2));
    REQUIRE(item2->Parent() != room);
    REQUIRE(witness(customer) == u8"You can't buy item1.\n");
  }

  SECTION("Value Item") {
    handle_command(customer, u8"value item2", mind);
    REQUIRE(!shopkeeper->HasWithin(item1));
    REQUIRE(!customer->HasWithin(item1));
    REQUIRE(item1->Parent() == room);
    REQUIRE(!shopkeeper->HasWithin(item2));
    REQUIRE(customer->HasWithin(item2));
    REQUIRE(item2->Parent() != room);
    REQUIRE(witness(customer) == u8"I'll give you 12cp for item2\n");
  }

  SECTION("Value Item Missing Mind") {
    handle_command(customer, u8"value item2");
    REQUIRE(!shopkeeper->HasWithin(item1));
    REQUIRE(!customer->HasWithin(item1));
    REQUIRE(item1->Parent() == room);
    REQUIRE(!shopkeeper->HasWithin(item2));
    REQUIRE(customer->HasWithin(item2));
    REQUIRE(item2->Parent() != room);
    REQUIRE(witness(customer) == u8"");
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
    REQUIRE(witness(customer) == u8"Sorry, nobody is buying that sort of thing here.\n");
  }

  SECTION("Value From Dead Shopkeeper") {
    shopkeeper->AddAct(act_t::DEAD);
    handle_command(customer, u8"value item2", mind);
    REQUIRE(!shopkeeper->HasWithin(item1));
    REQUIRE(!customer->HasWithin(item1));
    REQUIRE(item1->Parent() == room);
    REQUIRE(!shopkeeper->HasWithin(item2));
    REQUIRE(customer->HasWithin(item2));
    REQUIRE(item2->Parent() != room);
    REQUIRE(witness(customer) == u8"Sorry, the shopkeeper is dead!\n");
  }

  SECTION("Value From Dying Shopkeeper") {
    shopkeeper->AddAct(act_t::DYING);
    handle_command(customer, u8"value item2", mind);
    REQUIRE(!shopkeeper->HasWithin(item1));
    REQUIRE(!customer->HasWithin(item1));
    REQUIRE(item1->Parent() == room);
    REQUIRE(!shopkeeper->HasWithin(item2));
    REQUIRE(customer->HasWithin(item2));
    REQUIRE(item2->Parent() != room);
    REQUIRE(witness(customer) == u8"Sorry, the shopkeeper is dying!\n");
  }

  SECTION("Value From Unconscious Shopkeeper") {
    shopkeeper->AddAct(act_t::UNCONSCIOUS);
    handle_command(customer, u8"value item2", mind);
    REQUIRE(!shopkeeper->HasWithin(item1));
    REQUIRE(!customer->HasWithin(item1));
    REQUIRE(item1->Parent() == room);
    REQUIRE(!shopkeeper->HasWithin(item2));
    REQUIRE(customer->HasWithin(item2));
    REQUIRE(item2->Parent() != room);
    REQUIRE(witness(customer) == u8"Sorry, the shopkeeper is unconscious!\n");
  }

  SECTION("Value From Sleeping Shopkeeper") {
    shopkeeper->AddAct(act_t::SLEEP);
    handle_command(customer, u8"value item2", mind);
    REQUIRE(!shopkeeper->HasWithin(item1));
    REQUIRE(!customer->HasWithin(item1));
    REQUIRE(item1->Parent() == room);
    REQUIRE(!shopkeeper->HasWithin(item2));
    REQUIRE(customer->HasWithin(item2));
    REQUIRE(item2->Parent() != room);
    REQUIRE(witness(customer) == u8"Sorry, the shopkeeper is asleep!\n");
  }

  SECTION("Sell Item") {
    handle_command(customer, u8"sell item2", mind);
    REQUIRE(!shopkeeper->HasWithin(item1));
    REQUIRE(!customer->HasWithin(item1));
    REQUIRE(item1->Parent() == room);
    REQUIRE(!shopkeeper->HasWithin(item2));
    REQUIRE(!customer->HasWithin(item2));
    REQUIRE(item2->Parent() == room);
    const std::u8string_view res =
        u8"I'll give you 12cp for item2\n"
        u8"You sell your item2.\n";
    REQUIRE(witness(customer).substr(0, res.length()) == res);
  }

  SECTION("Sell Item Missing Mind") {
    handle_command(customer, u8"sell item2");
    REQUIRE(!shopkeeper->HasWithin(item1));
    REQUIRE(!customer->HasWithin(item1));
    REQUIRE(item1->Parent() == room);
    REQUIRE(!shopkeeper->HasWithin(item2));
    REQUIRE(!customer->HasWithin(item2));
    REQUIRE(item2->Parent() == room);
    const std::u8string_view res = u8"You sell your item2.\n";
    REQUIRE(witness(customer).substr(0, res.length()) == res);
  }

  SECTION("Sell Item Without Purse") {
    handle_command(customer, u8"remove purse;drop purse", mind);
    customer->SetLongDesc(u8"");
    handle_command(customer, u8"sell item2", mind);
    REQUIRE(!shopkeeper->HasWithin(item1));
    REQUIRE(!customer->HasWithin(item1));
    REQUIRE(item1->Parent() == room);
    REQUIRE(!shopkeeper->HasWithin(item2));
    REQUIRE(!customer->HasWithin(item2));
    REQUIRE(item2->Parent() == room);
    REQUIRE(
        witness(customer) ==
        u8"I'll give you 12cp for item2\n"
        u8"You sell your item2.\n"
        u8"Shopkeeper drops a silver piece.\n"
        u8"Shopkeeper drops a copper piece.\n"
        u8"You couldn't stash 12 copper pieces, so at least some of it fell on the floor!\n");
  }

  SECTION("Sell Collection") {
    Object* subitem = new Object(item2);
    subitem->SetPosition(pos_t::LIE);
    handle_command(customer, u8"sell item2", mind);
    REQUIRE(!shopkeeper->HasWithin(item1));
    REQUIRE(!customer->HasWithin(item1));
    REQUIRE(item1->Parent() == room);
    REQUIRE(!shopkeeper->HasWithin(item2));
    REQUIRE(customer->HasWithin(item2));
    REQUIRE(item2->Parent() != room);
    REQUIRE(
        witness(customer) ==
        u8"Your item2 is not empty.  You must empty it before you can sell it.\n");
  }

  SECTION("Sell Container") {
    item2->SetSkill(prhash(u8"Container"), 1000);
    handle_command(customer, u8"sell item2", mind);
    REQUIRE(!shopkeeper->HasWithin(item1));
    REQUIRE(!customer->HasWithin(item1));
    REQUIRE(item1->Parent() == room);
    REQUIRE(!shopkeeper->HasWithin(item2));
    REQUIRE(customer->HasWithin(item2));
    REQUIRE(item2->Parent() != room);
    REQUIRE(
        witness(customer) == u8"Your item2 is a container.  You can't sell containers (yet).\n");
  }

  SECTION("Sell Liquid Container") {
    item2->SetSkill(prhash(u8"Liquid Container"), 1000);
    handle_command(customer, u8"sell item2", mind);
    REQUIRE(!shopkeeper->HasWithin(item1));
    REQUIRE(!customer->HasWithin(item1));
    REQUIRE(item1->Parent() == room);
    REQUIRE(!shopkeeper->HasWithin(item2));
    REQUIRE(customer->HasWithin(item2));
    REQUIRE(item2->Parent() != room);
    REQUIRE(
        witness(customer) == u8"Your item2 is a container.  You can't sell containers (yet).\n");
  }

  SECTION("Sell Held Item") {
    customer->AddAct(act_t::HOLD, item2);
    handle_command(customer, u8"sell item2", mind);
    REQUIRE(!shopkeeper->HasWithin(item1));
    REQUIRE(!customer->HasWithin(item1));
    REQUIRE(item1->Parent() == room);
    REQUIRE(!shopkeeper->HasWithin(item2));
    REQUIRE(!customer->HasWithin(item2));
    REQUIRE(item2->Parent() == room);
    REQUIRE(!customer->IsAct(act_t::HOLD));
    const std::u8string_view res =
        u8"I'll give you 12cp for item2\n"
        u8"You sell your item2.\n";
    REQUIRE(witness(customer).substr(0, res.length()) == res);
  }

  SECTION("Sell Dragged Item") {
    item2->Travel(room);
    customer->AddAct(act_t::HOLD, item2);
    handle_command(customer, u8"sell item2", mind);
    REQUIRE(!shopkeeper->HasWithin(item1));
    REQUIRE(!customer->HasWithin(item1));
    REQUIRE(item1->Parent() == room);
    REQUIRE(!shopkeeper->HasWithin(item2));
    REQUIRE(!customer->HasWithin(item2));
    REQUIRE(item2->Parent() == room);
    REQUIRE(!customer->IsAct(act_t::HOLD));
    const std::u8string_view res =
        u8"I'll give you 12cp for item2\n"
        u8"You sell item2.\n";
    REQUIRE(witness(customer).substr(0, res.length()) == res);
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
    REQUIRE(witness(customer) == u8"Sorry, nobody is buying that sort of thing here.\n");
  }

  SECTION("Sell Item Without Any Coin") {
    handle_command(shopkeeper, u8"drop all", mind);
    customer->SetLongDesc(u8"");
    handle_command(customer, u8"sell item2", mind);
    REQUIRE(!shopkeeper->HasWithin(item1));
    REQUIRE(!customer->HasWithin(item1));
    REQUIRE(item1->Parent() == room);
    REQUIRE(!shopkeeper->HasWithin(item2));
    REQUIRE(customer->HasWithin(item2));
    REQUIRE(item2->Parent() != room);
    REQUIRE(
        witness(customer) ==
        u8"I'll give you 12cp for item2\n"
        u8"I can't afford the 12cp (I only have 0).\n");
  }

  SECTION("Sell Item Without Enough Coin") {
    handle_command(shopkeeper, u8"drop all", mind);
    give_money(shopkeeper, 0, 0, 1, 1);
    customer->SetLongDesc(u8"");
    handle_command(customer, u8"sell item2", mind);
    REQUIRE(!shopkeeper->HasWithin(item1));
    REQUIRE(!customer->HasWithin(item1));
    REQUIRE(item1->Parent() == room);
    REQUIRE(!shopkeeper->HasWithin(item2));
    REQUIRE(customer->HasWithin(item2));
    REQUIRE(item2->Parent() != room);
    REQUIRE(
        witness(customer) ==
        u8"I'll give you 12cp for item2\n"
        u8"I can't afford the 12cp (I only have 11cp).\n");
  }

  SECTION("Sell Item Without Enough Change") {
    handle_command(shopkeeper, u8"drop all", mind);
    give_money(shopkeeper, 1, 0, 0, 0);
    customer->SetLongDesc(u8"");
    handle_command(customer, u8"sell item2", mind);
    REQUIRE(!shopkeeper->HasWithin(item1));
    REQUIRE(!customer->HasWithin(item1));
    REQUIRE(item1->Parent() == room);
    REQUIRE(!shopkeeper->HasWithin(item2));
    REQUIRE(customer->HasWithin(item2));
    REQUIRE(item2->Parent() != room);
    REQUIRE(
        witness(customer) ==
        u8"I'll give you 12cp for item2\n"
        u8"I don't have change to pay 12cp.  You'd have to make change for 1pp.\n"
        u8"...but you don't have that change.\n");
  }

  SECTION("Sell Item Requiring Change") {
    handle_command(shopkeeper, u8"drop all", mind);
    give_money(customer, 0, 0, 9, 9);
    give_money(shopkeeper, 0, 1, 0, 0);
    customer->SetLongDesc(u8"");
    handle_command(customer, u8"sell item2", mind);
    REQUIRE(!shopkeeper->HasWithin(item1));
    REQUIRE(!customer->HasWithin(item1));
    REQUIRE(item1->Parent() == room);
    REQUIRE(!shopkeeper->HasWithin(item2));
    REQUIRE(!customer->HasWithin(item2));
    REQUIRE(item2->Parent() == room);
    const std::u8string_view res =
        u8"I'll give you 12cp for item2\n"
        u8"I don't have change to pay 12cp.  You'd have to make change for 1gp.\n"
        u8"You pay 88cp to shopkeeper.\n"
        u8"You sell your item2.\n";
    REQUIRE(witness(customer).substr(0, res.length()) == res);
  }

  SECTION("Sell Worthless Item") {
    item2->SetValue(0);
    handle_command(customer, u8"sell item2", mind);
    REQUIRE(!shopkeeper->HasWithin(item1));
    REQUIRE(!customer->HasWithin(item1));
    REQUIRE(item1->Parent() == room);
    REQUIRE(!shopkeeper->HasWithin(item2));
    REQUIRE(customer->HasWithin(item2));
    REQUIRE(item2->Parent() != room);
    REQUIRE(witness(customer) == u8"your item2 is worthless.\n");
  }

  SECTION("Sell Priceless Item") {
    item2->SetSkill(prhash(u8"Priceless"), 1);
    handle_command(customer, u8"sell item2", mind);
    REQUIRE(!shopkeeper->HasWithin(item1));
    REQUIRE(!customer->HasWithin(item1));
    REQUIRE(item1->Parent() == room);
    REQUIRE(!shopkeeper->HasWithin(item2));
    REQUIRE(customer->HasWithin(item2));
    REQUIRE(item2->Parent() != room);
    REQUIRE(witness(customer) == u8"You can't sell your item2.\n");
  }

  SECTION("Sell Cursed Item") {
    item2->SetSkill(prhash(u8"Cursed"), 1);
    handle_command(customer, u8"sell item2", mind);
    REQUIRE(!shopkeeper->HasWithin(item1));
    REQUIRE(!customer->HasWithin(item1));
    REQUIRE(item1->Parent() == room);
    REQUIRE(!shopkeeper->HasWithin(item2));
    REQUIRE(customer->HasWithin(item2));
    REQUIRE(item2->Parent() != room);
    REQUIRE(witness(customer) == u8"You can't sell your item2.\n");
  }

  SECTION("Sell To Dead Shopkeeper") {
    shopkeeper->AddAct(act_t::DEAD);
    handle_command(customer, u8"sell item2", mind);
    REQUIRE(!shopkeeper->HasWithin(item1));
    REQUIRE(!customer->HasWithin(item1));
    REQUIRE(item1->Parent() == room);
    REQUIRE(!shopkeeper->HasWithin(item2));
    REQUIRE(customer->HasWithin(item2));
    REQUIRE(item2->Parent() != room);
    REQUIRE(witness(customer) == u8"Sorry, the shopkeeper is dead!\n");
  }

  SECTION("Sell To Dying Shopkeeper") {
    shopkeeper->AddAct(act_t::DYING);
    handle_command(customer, u8"sell item2", mind);
    REQUIRE(!shopkeeper->HasWithin(item1));
    REQUIRE(!customer->HasWithin(item1));
    REQUIRE(item1->Parent() == room);
    REQUIRE(!shopkeeper->HasWithin(item2));
    REQUIRE(customer->HasWithin(item2));
    REQUIRE(item2->Parent() != room);
    REQUIRE(witness(customer) == u8"Sorry, the shopkeeper is dying!\n");
  }

  SECTION("Sell To Unconscious Shopkeeper") {
    shopkeeper->AddAct(act_t::UNCONSCIOUS);
    handle_command(customer, u8"sell item2", mind);
    REQUIRE(!shopkeeper->HasWithin(item1));
    REQUIRE(!customer->HasWithin(item1));
    REQUIRE(item1->Parent() == room);
    REQUIRE(!shopkeeper->HasWithin(item2));
    REQUIRE(customer->HasWithin(item2));
    REQUIRE(item2->Parent() != room);
    REQUIRE(witness(customer) == u8"Sorry, the shopkeeper is unconscious!\n");
  }

  SECTION("Sell To Sleeping Shopkeeper") {
    shopkeeper->AddAct(act_t::SLEEP);
    handle_command(customer, u8"sell item2", mind);
    REQUIRE(!shopkeeper->HasWithin(item1));
    REQUIRE(!customer->HasWithin(item1));
    REQUIRE(item1->Parent() == room);
    REQUIRE(!shopkeeper->HasWithin(item2));
    REQUIRE(customer->HasWithin(item2));
    REQUIRE(item2->Parent() != room);
    REQUIRE(witness(customer) == u8"Sorry, the shopkeeper is asleep!\n");
  }

  destroy_universe();
}
