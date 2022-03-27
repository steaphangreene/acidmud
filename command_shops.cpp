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

#include "commands.hpp"
#include "object.hpp"
#include "properties.hpp"
#include "tags.hpp"
#include "utils.hpp"

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
          mind->Send(u8"{:>10} gp: {}\n", price, obj->ShortDesc());
          oobj = obj;
        }
      } else {
        auto items = body->Room()->PickObjects(u8"all", LOC_INTERNAL);
        bool have_stock = false;
        for (auto item : items) {
          if (item->ActTarg(act_t::SPECIAL_OWNER) == body->Room()) {
            have_stock = true;
            int price = item->Value();
            mind->Send(u8"{:>10} cp: {}\n", price, item->ShortDesc());
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

    auto people = body->Room()->PickObjects(u8"everyone", LOC_INTERNAL);
    DArr64<Object*> shpkps;
    std::u8string reason = u8"";
    for (auto shpkp : people) {
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
    if (shpkps.size() == 0) {
      if (mind) {
        mind->Send(reason);
        mind->Send(u8"You can only do that around a shopkeeper.\n");
      }
    } else {
      Object* shpkp = shpkps.front();
      if (shpkp->ActTarg(act_t::WEAR_RSHOULDER) &&
          shpkp->ActTarg(act_t::WEAR_RSHOULDER)->Skill(prhash(u8"Vortex"))) {
        Object* vortex = shpkp->ActTarg(act_t::WEAR_RSHOULDER);

        auto targs = vortex->PickObjects(std::u8string(args), LOC_INTERNAL);
        if (!targs.size()) {
          if (mind)
            mind->Send(u8"The shopkeeper doesn't have that.\n");
          return 0;
        }

        for (auto targ : targs) {
          int price = targ->Value() * targ->Quantity();
          if (price < 0) {
            if (mind)
              mind->Send(u8"You can't buy {}.\n", targ->Noun(0, 0, body));
            continue;
          } else if (price == 0) {
            if (mind) {
              std::u8string mes = targ->Noun(0, 0, body);
              mes += u8" is worthless.\n";
              mes[0] = ascii_toupper(mes[0]);
              mind->Send(mes);
            }
            continue;
          }

          if (targ->Skill(prhash(u8"Money")) != targ->Value()) { // Not 1-1 Money
            price *= shpkp->Skill(prhash(u8"Sell Profit"));
            price += 999;
            price /= 1000;
          }
          mind->Send(u8"{} gp: {}\n", price, targ->ShortDesc());

          int togo = price, ord = -price;
          auto pay = body->PickObjects(u8"a gold piece", LOC_INTERNAL, &ord);
          for (auto coin : pay) {
            togo -= coin->Quantity();
          }

          if (togo > 0) {
            if (mind)
              mind->Send(u8"You can't afford the {} gold (short {}).\n", price, togo);
          } else if (body->Stash(targ, 0, 0)) {
            body->Parent()->SendOut(
                stealth_t,
                stealth_s,
                u8";s buys and stashes ;s.\n",
                u8"You buy and stash ;s.\n",
                body,
                targ);
            for (auto coin : pay) {
              shpkp->Stash(coin, 0, 1);
            }
          } else if (
              ((!body->IsAct(act_t::HOLD)) ||
               body->ActTarg(act_t::HOLD) == body->ActTarg(act_t::WIELD) ||
               body->ActTarg(act_t::HOLD) == body->ActTarg(act_t::WEAR_SHIELD)) &&
              (!targ->Travel(body))) {
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
            body->AddAct(act_t::HOLD, targ);
            body->Parent()->SendOut(
                stealth_t,
                stealth_s,
                u8";s buys and holds ;s.\n",
                u8"You buy and hold ;s.\n",
                body,
                targ);
            for (auto coin : pay) {
              shpkp->Stash(coin, 0, 1);
            }
          } else {
            if (mind)
              mind->Send(u8"You can't stash or hold {}.\n", targ->Noun(1));
          }
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

    Object* targ = body->PickObject(std::u8string(args), LOC_NOTWORN | LOC_INTERNAL);
    if ((!targ) && body->ActTarg(act_t::HOLD) &&
        body->ActTarg(act_t::HOLD)->Parent() != body // Dragging
        && body->ActTarg(act_t::HOLD)->Matches(std::u8string(args))) {
      targ = body->ActTarg(act_t::HOLD);
    }

    if (!targ) {
      if (mind)
        mind->Send(u8"You want to sell what?\n");
      return 0;
    }

    if (targ->Skill(prhash(u8"Container")) || targ->Skill(prhash(u8"Liquid Container"))) {
      if (mind) {
        std::u8string mes = targ->Noun(0, 0, body);
        mes += u8" is a container.";
        mes += u8"  You can't sell containers (yet).\n";
        mes[0] = ascii_toupper(mes[0]);
        mind->Send(mes);
      }
      return 0;
    }

    if (targ->Contents(LOC_TOUCH | LOC_NOTFIXED).size() > 0) {
      if (mind) {
        std::u8string mes = targ->Noun(0, 0, body);
        mes += u8" is not empty.";
        mes += u8"  You must empty it before you can sell it.\n";
        mes[0] = ascii_toupper(mes[0]);
        mind->Send(mes);
      }
      return 0;
    }

    int price = targ->Value() * targ->Quantity();
    if (price < 0 || targ->HasSkill(prhash(u8"Priceless")) || targ->HasSkill(prhash(u8"Cursed"))) {
      if (mind)
        mind->Send(u8"You can't sell {}.\n", targ->Noun(0, 0, body));
      return 0;
    }
    if (price == 0) {
      if (mind)
        mind->Send(u8"{} is worthless.\n", targ->Noun(0, 0, body));
      return 0;
    }

    int wearable = 0;
    if (targ->HasSkill(prhash(u8"Wearable on Back")))
      wearable = 1;
    if (targ->HasSkill(prhash(u8"Wearable on Chest")))
      wearable = 1;
    if (targ->HasSkill(prhash(u8"Wearable on Head")))
      wearable = 1;
    if (targ->HasSkill(prhash(u8"Wearable on Neck")))
      wearable = 1;
    if (targ->HasSkill(prhash(u8"Wearable on Collar")))
      wearable = 1;
    if (targ->HasSkill(prhash(u8"Wearable on Waist")))
      wearable = 1;
    if (targ->HasSkill(prhash(u8"Wearable on Shield")))
      wearable = 1;
    if (targ->HasSkill(prhash(u8"Wearable on Left Arm")))
      wearable = 1;
    if (targ->HasSkill(prhash(u8"Wearable on Right Arm")))
      wearable = 1;
    if (targ->HasSkill(prhash(u8"Wearable on Left Finger")))
      wearable = 1;
    if (targ->HasSkill(prhash(u8"Wearable on Right Finger")))
      wearable = 1;
    if (targ->HasSkill(prhash(u8"Wearable on Left Foot")))
      wearable = 1;
    if (targ->HasSkill(prhash(u8"Wearable on Right Foot")))
      wearable = 1;
    if (targ->HasSkill(prhash(u8"Wearable on Left Hand")))
      wearable = 1;
    if (targ->HasSkill(prhash(u8"Wearable on Right Hand")))
      wearable = 1;
    if (targ->HasSkill(prhash(u8"Wearable on Left Leg")))
      wearable = 1;
    if (targ->HasSkill(prhash(u8"Wearable on Right Leg")))
      wearable = 1;
    if (targ->HasSkill(prhash(u8"Wearable on Left Wrist")))
      wearable = 1;
    if (targ->HasSkill(prhash(u8"Wearable on Right Wrist")))
      wearable = 1;
    if (targ->HasSkill(prhash(u8"Wearable on Left Shoulder")))
      wearable = 1;
    if (targ->HasSkill(prhash(u8"Wearable on Right Shoulder")))
      wearable = 1;
    if (targ->HasSkill(prhash(u8"Wearable on Left Hip")))
      wearable = 1;
    if (targ->HasSkill(prhash(u8"Wearable on Right Hip")))
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
      } else if (targ->Skill(prhash(u8"Money")) == targ->Value()) { // 1-1 Money
        for (auto skl : shpkp->GetSkills()) {
          if (SkillName(skl.first).starts_with(u8"Buy ")) {
            skill = prhash(u8"Money");
            break;
          }
        }
      } else if (wearable && targ->NormAttribute(0) > 0) {
        if (shpkp->HasSkill(prhash(u8"Buy Armor"))) {
          skill = prhash(u8"Buy Armor");
        }
      } else if (targ->Skill(prhash(u8"Vehicle")) == 4) {
        if (shpkp->HasSkill(prhash(u8"Buy Boat"))) {
          skill = prhash(u8"Buy Boat");
        }
      } else if (targ->HasSkill(prhash(u8"Container"))) {
        if (shpkp->HasSkill(prhash(u8"Buy Container"))) {
          skill = prhash(u8"Buy Container");
        }
      } else if (targ->HasSkill(prhash(u8"Food")) && (!targ->HasSkill(prhash(u8"Drink")))) {
        if (shpkp->HasSkill(prhash(u8"Buy Food"))) {
          skill = prhash(u8"Buy Food");
        }
      }
      //      else if(false) {				//FIXME: Implement
      //	if(shpkp->HasSkill(prhash(u8"Buy Light"))) {
      //	  skill = prhash(u8"Buy Light");
      //	  }
      //	}
      else if (targ->HasSkill(prhash(u8"Liquid Container"))) { // FIXME: Not Potions?
        if (shpkp->HasSkill(prhash(u8"Buy Liquid Container"))) {
          skill = prhash(u8"Buy Liquid Container");
        }
      } else if (targ->HasSkill(prhash(u8"Liquid Container"))) { // FIXME: Not Bottles?
        if (shpkp->HasSkill(prhash(u8"Buy Potion"))) {
          skill = prhash(u8"Buy Potion");
        }
      } else if (targ->HasSkill(prhash(u8"Magical Scroll"))) {
        if (shpkp->HasSkill(prhash(u8"Buy Scroll"))) {
          skill = prhash(u8"Buy Scroll");
        }
      } else if (targ->HasSkill(prhash(u8"Magical Staff"))) {
        if (shpkp->HasSkill(prhash(u8"Buy Staff"))) {
          skill = prhash(u8"Buy Staff");
        }
      } else if (targ->HasSkill(prhash(u8"Magical Wand"))) {
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
      else if (targ->Skill(prhash(u8"WeaponType")) > 0) {
        if (shpkp->HasSkill(prhash(u8"Buy Weapon"))) {
          skill = prhash(u8"Buy Weapon");
        }
      } else if (wearable && targ->NormAttribute(0) == 0) {
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
      Object* shpkp = shpkps.front();
      if (shpkp->ActTarg(act_t::WEAR_RSHOULDER) &&
          shpkp->ActTarg(act_t::WEAR_RSHOULDER)->Skill(prhash(u8"Vortex"))) {
        Object* vortex = shpkp->ActTarg(act_t::WEAR_RSHOULDER);

        if (skill != prhash(u8"Money")) { // Not 1-1 Money
          price *= shpkp->Skill(skill);
          price += 0;
          price /= 1000;
          if (price <= 0)
            price = 1;
        }
        mind->Send(u8"I'll give you {}gp for {}\n", price, targ->ShortDesc());

        if (cnum == COM_SELL) {
          int togo = price, ord = -price;
          auto pay = shpkp->PickObjects(u8"a gold piece", LOC_INTERNAL, &ord);
          for (auto coin : pay) {
            togo -= std::max(1, coin->Skill(prhash(u8"Quantity")));
          }

          if (togo <= 0) {
            body->Parent()->SendOut(
                stealth_t, stealth_s, u8";s sells ;s.\n", u8"You sell ;s.\n", body, targ);
            Object* payment = new Object;
            for (auto coin : pay) {
              coin->Travel(payment);
            }
            if (body->Stash(payment->Contents().front())) {
              targ->Travel(vortex);
            } else { // Keeper gets it back
              shpkp->Stash(payment->Contents().front(), 0, 1);
              if (mind)
                mind->Send(u8"You couldn't stash {} gold!\n", price);
            }
            delete payment;
          } else {
            if (mind)
              mind->Send(u8"I can't afford the {} gold.\n", price);
          }
        }
      }
    }
    return 0;
  }

  return 0;
}