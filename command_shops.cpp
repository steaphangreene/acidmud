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

#include "color.hpp"
#include "commands.hpp"
#include "object.hpp"
#include "properties.hpp"
#include "tags.hpp"
#include "utils.hpp"

static std::u8string coin_str(size_t amount) {
  std::u8string ret;
  if (amount == 0) {
    ret = u8"0 coins";
  } else if (amount % 10UL != 0) {
    ret = fmt::format(u8"{} chits", amount);
  } else if (amount % 100UL != 0) {
    ret = fmt::format(u8"{} copper pieces", amount / 10);
  } else if (amount % 1000UL != 0) {
    ret = fmt::format(u8"{} silver pieces", amount / 100);
  } else if (amount % 10000UL != 0) {
    ret = fmt::format(u8"{} gold pieces", amount / 1000);
  } else {
    ret = fmt::format(u8"{} platinum pieces", amount / 10000);
  }
  return ret;
}

static std::u8string coins(size_t amount) {
  std::u8string ret;
  if (amount == 0) {
    ret = u8"0";
  } else if (amount % 10UL != 0) {
    ret = fmt::format(u8"{}ch", amount);
  } else if (amount % 100UL != 0) {
    ret = fmt::format(u8"{}cp", amount / 10);
  } else if (amount % 1000UL != 0) {
    ret = fmt::format(u8"{}sp", amount / 100);
  } else if (amount % 10000UL != 0) {
    ret = fmt::format(u8"{}gp", amount / 1000);
  } else {
    ret = fmt::format(u8"{}pp", amount / 10000);
  }
  return ret;
}

int handle_command_shops(
    Object* body,
    std::shared_ptr<Mind>& mind,
    int cnum,
    const std::u8string_view& args,
    int stealth_t,
    int stealth_s) {
  if (cnum == COM_LIST) {
    if (!mind)
      return 0;

    auto people = body->Room()->PickObjects(u8"everyone", LOC_INTERNAL);
    DArr64<Object*> shpkps;
    std::u8string reason = u8"";
    for (auto shpkp : people) { // Circle/TBA Shopkeepers
      if (shpkp->Skill(prhash(u8"Sell Profit"))) {
        if (shpkp->IsAct(act_t::DEAD)) {
          reason = u8"Sorry, the shopkeeper is dead!\n";
        } else if (shpkp->IsAct(act_t::DYING)) {
          reason = u8"Sorry, the shopkeeper is dying!\n";
        } else if (shpkp->IsAct(act_t::UNCONSCIOUS)) {
          reason = u8"Sorry, the shopkeeper is unconscious!\n";
        } else if (shpkp->IsAct(act_t::SLEEP)) {
          reason = u8"Sorry, the shopkeeper is asleep!\n";
        } else {
          shpkps.push_back(shpkp);
        }
      }
    }
    auto workers = body->Room()->Touching();
    for (auto shpkp : workers) { // Acid Shopkeepers
      if (shpkp->IsAct(act_t::WORK) && shpkp->ActTarg(act_t::SPECIAL_WORK) == body->Room()) {
        shpkps.push_back(shpkp);
      }
    }
    if (shpkps.size() == 0) {
      if (mind) {
        mind->Send(reason);
        mind->Send(u8"You can only do that around a shopkeeper.\n");
      }
    } else {
      Object* shpkp = shpkps.front();
      if (shpkp->ActTarg(act_t::WEAR_RSHOULDER) &&
          shpkp->ActTarg(act_t::WEAR_RSHOULDER)->Skill(prhash(u8"Vortex"))) { // Circle/TBA
        Object* vortex = shpkp->ActTarg(act_t::WEAR_RSHOULDER);
        auto objs = vortex->Contents();
        auto oobj = objs.front();
        for (auto obj : objs) {
          if (obj != objs.front() && obj->IsSameAs(*oobj))
            continue;
          int price = obj->Value();
          if (obj->Skill(prhash(u8"Money")) != obj->Value()) { // Not 1-1 Money
            price *= shpkp->Skill(prhash(u8"Sell Profit"));
            price += 999;
            price /= 1000;
          }
          mind->Send(u8"{:>10}: {}\n", coins(price), obj->ShortDesc());
          oobj = obj;
        }
      } else {
        auto items = body->PickObjects(u8"everything", LOC_NEARBY);
        bool have_stock = false;
        for (auto item : items) {
          if (item->ActTarg(act_t::SPECIAL_OWNER) == body->Room()) {
            have_stock = true;
            int price = item->Value();
            mind->Send(CMAG u8"{:>10}: {}\n" CNRM, coins(price), item->ShortDesc());
          }
        }
        if (!have_stock) {
          mind->Send(u8"Nothing is for sale here, sorry.\n");
        }
      }
    }
    return 0;
  }

  if (cnum == COM_BUY) {
    if (args.empty()) {
      if (mind)
        mind->Send(u8"What do you want to buy?\n");
      return 0;
    }

    auto people = body->PickObjects(u8"everyone", LOC_NEARBY);
    DArr64<Object*> shpkps;
    std::u8string reason = u8"";
    for (auto shpkp : people) {
      if (shpkp->Skill(prhash(u8"Sell Profit"))) { // Circle/TBA Shopkeepers
        if (shpkp->IsAct(act_t::DEAD)) {
          reason = u8"Sorry, the shopkeeper is dead!\n";
        } else if (shpkp->IsAct(act_t::DYING)) {
          reason = u8"Sorry, the shopkeeper is dying!\n";
        } else if (shpkp->IsAct(act_t::UNCONSCIOUS)) {
          reason = u8"Sorry, the shopkeeper is unconscious!\n";
        } else if (shpkp->IsAct(act_t::SLEEP)) {
          reason = u8"Sorry, the shopkeeper is asleep!\n";
        } else {
          shpkps.push_back(shpkp);
        }
      }
    }
    auto workers = body->Room()->Touching();
    for (auto shpkp : workers) { // Acid Shopkeepers
      if (shpkp->IsAct(act_t::WORK) && shpkp->ActTarg(act_t::SPECIAL_WORK) == body->Room()) {
        shpkps.push_back(shpkp);
      }
    }
    if (shpkps.size() == 0) {
      if (mind) {
        mind->Send(reason);
        mind->Send(u8"You can only do that around a shopkeeper.\n");
      }
    } else {
      auto items = body->PickObjects(args, LOC_NEARBY);

      Object* shpkp = shpkps.front();
      if (shpkp->ActTarg(act_t::WEAR_RSHOULDER) &&
          shpkp->ActTarg(act_t::WEAR_RSHOULDER)->Skill(prhash(u8"Vortex"))) { // Circle/TBA
        Object* vortex = shpkp->ActTarg(act_t::WEAR_RSHOULDER);

        items = vortex->PickObjects(std::u8string(args), LOC_INTERNAL);

      } else {
        int price = 0;
        bool all_for_sale = true;
        for (auto item : items) {
          if (item->ActTarg(act_t::SPECIAL_OWNER) != body->Room()) {
            mind->Send(CRED u8" Not For Sale: {}\n" CNRM, item->ShortDesc());
            all_for_sale = false;
          } else {
            price += item->Value();
            mind->Send(u8"{:>10}: {}\n", coins(item->Value()), item->ShortDesc());
          }
        }
        if (!all_for_sale) {
          mind->Send(CRED u8"Sorry, you can't buy that.\n" CNRM);
          return 0;
        } else {
          mind->Send(CGRN u8"Deal.  That will be {}.\n" CNRM, coin_str(price));
        }
      }

      if (items.size() == 0) {
        if (mind)
          mind->Send(u8"The shopkeeper doesn't have that.\n");
        return 0;
      }

      for (auto item : items) {
        int price = item->Value() * item->Quantity();
        if (price < 0) {
          if (mind) {
            mind->Send(u8"You can't buy {}.\n", item->Noun(0, 0, body));
          }
          continue;
        } else if (price == 0) {
          if (mind) {
            std::u8string mes = item->Noun(0, 0, body);
            mes += u8" is worthless.\n";
            mes[0] = ascii_toupper(mes[0]);
            mind->Send(mes);
          }
          continue;
        }

        if (shpkp->HasSkill(prhash(u8"Sell Profit"))) { // Circle/TBA
          if (item->Skill(prhash(u8"Money")) != item->Value()) { // Not 1-1 Money
            price *= shpkp->Skill(prhash(u8"Sell Profit"));
            price += 999;
            price /= 1000;
          }
        }
        mind->Send(u8"{}: {}\n", coins(price), item->ShortDesc());

        auto pay = body->PayFor(price);
        if (pay.size() == 0) {
          if (mind) {
            auto offer = body->CanPayFor(price);
            if (offer > price) {
              mind->Send(
                  u8"You don't have change to pay {} (you'd have to pay {}).\n",
                  coins(price),
                  coins(offer));
            } else {
              mind->Send(
                  u8"You can't afford the {} (you only have {}).\n", coins(price), coins(offer));
            }
          }
        } else if (body->Stash(item, 0, 0)) {
          body->Parent()->SendOut(
              stealth_t,
              stealth_s,
              u8";s buys and stashes ;s.\n",
              u8"You buy and stash ;s.\n",
              body,
              item);
          item->StopAct(act_t::SPECIAL_OWNER);
          for (auto coin : pay) {
            shpkp->Stash(coin, 0, 1);
          }
        } else if (
            ((!body->IsAct(act_t::HOLD)) ||
             body->ActTarg(act_t::HOLD) == body->ActTarg(act_t::WIELD) ||
             body->ActTarg(act_t::HOLD) == body->ActTarg(act_t::WEAR_SHIELD)) &&
            (!item->Travel(body))) {
          if (body->IsAct(act_t::HOLD)) {
            body->Parent()->SendOut(
                stealth_t,
                stealth_s,
                u8";s stops holding ;s.\n",
                u8"You stop holding ;s.\n",
                body,
                body->ActTarg(act_t::HOLD));
            body->StopAct(act_t::HOLD);
          }
          body->AddAct(act_t::HOLD, item);
          body->Parent()->SendOut(
              stealth_t,
              stealth_s,
              u8";s buys and holds ;s.\n",
              u8"You buy and hold ;s.\n",
              body,
              item);
          item->StopAct(act_t::SPECIAL_OWNER);
          for (auto coin : pay) {
            shpkp->Stash(coin, 0, 1);
          }
        } else {
          if (mind)
            mind->Send(u8"You can't stash or hold {}.\n", item->Noun(1));
        }
      }
    }
    return 0;
  }

  if (cnum == COM_VALUE || cnum == COM_SELL) {
    if (args.empty()) {
      if (mind)
        mind->Send(u8"What do you want to sell?\n");
      return 0;
    }

    Object* item = body->PickObject(std::u8string(args), LOC_NOTWORN | LOC_INTERNAL);
    if ((!item) && body->ActTarg(act_t::HOLD) &&
        body->ActTarg(act_t::HOLD)->Parent() != body // Dragging
        && body->ActTarg(act_t::HOLD)->Matches(std::u8string(args))) {
      item = body->ActTarg(act_t::HOLD);
    }

    if (!item) {
      if (mind)
        mind->Send(u8"You want to sell what?\n");
      return 0;
    }

    if (item->Skill(prhash(u8"Container")) || item->Skill(prhash(u8"Liquid Container"))) {
      if (mind) {
        std::u8string mes = item->Noun(0, 0, body);
        mes += u8" is a container.";
        mes += u8"  You can't sell containers (yet).\n";
        mes[0] = ascii_toupper(mes[0]);
        mind->Send(mes);
      }
      return 0;
    }

    if (item->Contents(LOC_TOUCH | LOC_NOTFIXED).size() > 0) {
      if (mind) {
        std::u8string mes = item->Noun(0, 0, body);
        mes += u8" is not empty.";
        mes += u8"  You must empty it before you can sell it.\n";
        mes[0] = ascii_toupper(mes[0]);
        mind->Send(mes);
      }
      return 0;
    }

    int price = item->Value() * item->Quantity();
    if (price < 0 || item->HasSkill(prhash(u8"Priceless")) || item->HasSkill(prhash(u8"Cursed"))) {
      if (mind)
        mind->Send(u8"You can't sell {}.\n", item->Noun(0, 0, body));
      return 0;
    }
    if (price == 0) {
      if (mind)
        mind->Send(u8"{} is worthless.\n", item->Noun(0, 0, body));
      return 0;
    }

    int wearable = 0;
    if (item->HasSkill(prhash(u8"Wearable on Back")))
      wearable = 1;
    if (item->HasSkill(prhash(u8"Wearable on Chest")))
      wearable = 1;
    if (item->HasSkill(prhash(u8"Wearable on Head")))
      wearable = 1;
    if (item->HasSkill(prhash(u8"Wearable on Neck")))
      wearable = 1;
    if (item->HasSkill(prhash(u8"Wearable on Collar")))
      wearable = 1;
    if (item->HasSkill(prhash(u8"Wearable on Waist")))
      wearable = 1;
    if (item->HasSkill(prhash(u8"Wearable on Shield")))
      wearable = 1;
    if (item->HasSkill(prhash(u8"Wearable on Left Arm")))
      wearable = 1;
    if (item->HasSkill(prhash(u8"Wearable on Right Arm")))
      wearable = 1;
    if (item->HasSkill(prhash(u8"Wearable on Left Finger")))
      wearable = 1;
    if (item->HasSkill(prhash(u8"Wearable on Right Finger")))
      wearable = 1;
    if (item->HasSkill(prhash(u8"Wearable on Left Foot")))
      wearable = 1;
    if (item->HasSkill(prhash(u8"Wearable on Right Foot")))
      wearable = 1;
    if (item->HasSkill(prhash(u8"Wearable on Left Hand")))
      wearable = 1;
    if (item->HasSkill(prhash(u8"Wearable on Right Hand")))
      wearable = 1;
    if (item->HasSkill(prhash(u8"Wearable on Left Leg")))
      wearable = 1;
    if (item->HasSkill(prhash(u8"Wearable on Right Leg")))
      wearable = 1;
    if (item->HasSkill(prhash(u8"Wearable on Left Wrist")))
      wearable = 1;
    if (item->HasSkill(prhash(u8"Wearable on Right Wrist")))
      wearable = 1;
    if (item->HasSkill(prhash(u8"Wearable on Left Shoulder")))
      wearable = 1;
    if (item->HasSkill(prhash(u8"Wearable on Right Shoulder")))
      wearable = 1;
    if (item->HasSkill(prhash(u8"Wearable on Left Hip")))
      wearable = 1;
    if (item->HasSkill(prhash(u8"Wearable on Right Hip")))
      wearable = 1;

    auto people = body->Room()->PickObjects(u8"everyone", LOC_INTERNAL);
    DArr64<Object*> shpkps;
    std::u8string reason = u8"Sorry, nobody is buying that sort of thing here.\n";
    uint32_t skill = prhash(u8"None");
    for (auto shpkp : people) {
      if (shpkp->IsAct(act_t::DEAD)) {
        reason = u8"Sorry, the shopkeeper is dead!\n";
      } else if (shpkp->IsAct(act_t::DYING)) {
        reason = u8"Sorry, the shopkeeper is dying!\n";
      } else if (shpkp->IsAct(act_t::UNCONSCIOUS)) {
        reason = u8"Sorry, the shopkeeper is unconscious!\n";
      } else if (shpkp->IsAct(act_t::SLEEP)) {
        reason = u8"Sorry, the shopkeeper is asleep!\n";
      } else if (item->ShortDesc() == u8"a widget") { // Temporary Test Object
        if (shpkp->HasTag(crc32c(u8"master"))) {
          skill = 0; // Special Marker: Acid Seller, Not Circle/TBA
        }
      } else if (item->Skill(prhash(u8"Made of Wood")) > 1000) { // Temporary Hard-Code
        if (shpkp->HasTag(crc32c(u8"master")) && shpkp->HasTag(crc32c(u8"collier"))) {
          skill = 0; // Special Marker: Acid Seller, Not Circle/TBA
        }
      } else if (item->Skill(prhash(u8"Money")) == item->Value()) { // 1-1 Money
        for (auto skl : shpkp->GetSkills()) {
          if (SkillName(skl.first).starts_with(u8"Buy ")) {
            skill = prhash(u8"Money");
            break;
          }
        }
      } else if (wearable && item->NormAttribute(0) > 0) {
        if (shpkp->HasSkill(prhash(u8"Buy Armor"))) {
          skill = prhash(u8"Buy Armor");
        }
      } else if (item->Skill(prhash(u8"Vehicle")) == 4) {
        if (shpkp->HasSkill(prhash(u8"Buy Boat"))) {
          skill = prhash(u8"Buy Boat");
        }
      } else if (item->HasSkill(prhash(u8"Container"))) {
        if (shpkp->HasSkill(prhash(u8"Buy Container"))) {
          skill = prhash(u8"Buy Container");
        }
      } else if (item->HasSkill(prhash(u8"Food")) && (!item->HasSkill(prhash(u8"Drink")))) {
        if (shpkp->HasSkill(prhash(u8"Buy Food"))) {
          skill = prhash(u8"Buy Food");
        }
      }
      //      else if(false) {				//FIXME: Implement
      //	if(shpkp->HasSkill(prhash(u8"Buy Light"))) {
      //	  skill = prhash(u8"Buy Light");
      //	  }
      //	}
      else if (item->HasSkill(prhash(u8"Liquid Container"))) { // FIXME: Not Potions?
        if (shpkp->HasSkill(prhash(u8"Buy Liquid Container"))) {
          skill = prhash(u8"Buy Liquid Container");
        }
      } else if (item->HasSkill(prhash(u8"Liquid Container"))) { // FIXME: Not Bottles?
        if (shpkp->HasSkill(prhash(u8"Buy Potion"))) {
          skill = prhash(u8"Buy Potion");
        }
      } else if (item->HasSkill(prhash(u8"Magical Scroll"))) {
        if (shpkp->HasSkill(prhash(u8"Buy Scroll"))) {
          skill = prhash(u8"Buy Scroll");
        }
      } else if (item->HasSkill(prhash(u8"Magical Staff"))) {
        if (shpkp->HasSkill(prhash(u8"Buy Staff"))) {
          skill = prhash(u8"Buy Staff");
        }
      } else if (item->HasSkill(prhash(u8"Magical Wand"))) {
        if (shpkp->HasSkill(prhash(u8"Buy Wand"))) {
          skill = prhash(u8"Buy Wand");
        }
      }
      //      else if(false) {				//FIXME: Implement
      //	if(shpkp->HasSkill(prhash(u8"Buy Trash"))) {
      //	  skill = prhash(u8"Buy Trash");
      //	  }
      //	}
      //      else if(false) {				//FIXME: Implement
      //	if(shpkp->HasSkill(prhash(u8"Buy Treasure"))) {
      //	  skill = prhash(u8"Buy Treasure");
      //	  }
      //	}
      else if (item->Skill(prhash(u8"WeaponType")) > 0) {
        if (shpkp->HasSkill(prhash(u8"Buy Weapon"))) {
          skill = prhash(u8"Buy Weapon");
        }
      } else if (wearable && item->NormAttribute(0) == 0) {
        if (shpkp->HasSkill(prhash(u8"Buy Worn"))) {
          skill = prhash(u8"Buy Worn");
        }
      }
      //      else if(false) {					//FIXME:
      //      Implement
      //	if(shpkp->HasSkill(prhash(u8"Buy Other"))) {
      //	  skill = prhash(u8"Buy Other");
      //	  }
      //	}

      if (skill == prhash(u8"None") && shpkp->HasSkill(prhash(u8"Buy All"))) {
        skill = prhash(u8"Buy All");
      }

      if (skill != prhash(u8"None")) {
        shpkps.push_back(shpkp);
      }
    }
    if (shpkps.size() == 0) {
      if (mind) {
        mind->Send(reason);
      }
    } else {
      Object* vortex = nullptr;
      Object* shpkp = shpkps.front();
      if (shpkp->ActTarg(act_t::WEAR_RSHOULDER) &&
          shpkp->ActTarg(act_t::WEAR_RSHOULDER)->Skill(prhash(u8"Vortex"))) { // Circle/TBA
        vortex = shpkp->ActTarg(act_t::WEAR_RSHOULDER);
        if (skill != prhash(u8"Money")) { // Not 1-1 Money
          price *= shpkp->Skill(skill);
          price += 0;
          price /= 1000;
          if (price <= 0)
            price = 1;
        }
      }
      mind->Send(u8"I'll give you {} for {}\n", coins(price), item->ShortDesc());

      if (cnum == COM_SELL) {
        auto pay = shpkp->PayFor(price);

        if (pay.size() > 0) {
          body->Parent()->SendOut(
              stealth_t, stealth_s, u8";s sells ;s.\n", u8"You sell ;s.\n", body, item);
          bool success = true;
          for (auto coin : pay) {
            if (!body->Stash(coin)) {
              success = false;
              shpkp->Drop(coin);
            }
          }
          if (vortex) {
            item->Travel(vortex);
          } else {
            item->Travel(body->Room());
            if (body->ActTarg(act_t::HOLD) == item) { // Stop Dragging It
              body->StopAct(act_t::HOLD);
            }
            item->AddAct(act_t::SPECIAL_OWNER, body->Room());
          }
          if (!success) {
            if (mind) {
              mind->Send(
                  u8"You couldn't stash {}, so at least some of it fell on the floor!\n",
                  coin_str(price));
            }
          }
        } else {
          if (mind) {
            auto offer = shpkp->CanPayFor(price);
            if (offer > price) {
              mind->Send(
                  u8"I don't have change to pay {} (you'd have to make change for {}).\n",
                  coins(price),
                  coins(offer));
            } else {
              mind->Send(u8"I can't afford the {} (I only have {}).\n", coins(price), coins(offer));
            }
          }
        }
      }
    }
    return 0;
  }

  return 0;
}
