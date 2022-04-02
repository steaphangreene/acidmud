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
          size_t price = obj->Value();
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
          if (item->ActTarg(act_t::SPECIAL_OWNER) == body->Room() &&
              item->Position() == pos_t::PROP) {
            have_stock = true;
            size_t price = item->Value();
            if (item->Quantity() > 1) {
              mind->Send(
                  CMAG u8"{:>10}: {} (x{})\n" CNRM,
                  coins(price),
                  item->ShortDesc(),
                  item->Quantity());
            } else {
              mind->Send(CMAG u8"{:>10}: {}\n" CNRM, coins(price), item->ShortDesc());
            }
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
        size_t price = 0;
        bool all_for_sale = true;
        for (auto item : items) {
          if (item->ActTarg(act_t::SPECIAL_OWNER) != body->Room() ||
              item->Position() != pos_t::PROP) {
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
        size_t price = item->Value() * item->Quantity();
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
              auto refund = offer - price;
              auto change = shpkp->PayFor(refund);
              if (change.size() > 0) {
                pay = body->PayFor(offer);
                if (pay.size() <= 0) {
                  mind->Send(u8"...and something went horribly wrong (tell the Ninjas).\n");
                } else {
                  for (auto coin : change) {
                    body->Stash(coin, 0, 1);
                  }
                  mind->Send(u8"You get {} from {}.\n", coins(refund), shpkp->ShortDesc());
                }
              } else {
                mind->Send(u8"...and I can't make change for that, sorry.\n");
              }
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
          item->SetPosition(pos_t::LIE);
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
          item->SetPosition(pos_t::LIE);
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

    size_t price = item->Value() * item->Quantity();
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

    auto people = body->Room()->PickObjects(u8"everyone", LOC_INTERNAL);
    DArr64<Object*> shpkps;
    std::u8string reason = u8"Sorry, nobody is buying that sort of thing here.\n";
    price = 0;
    for (auto shpkp : people) {
      if (shpkp->IsAct(act_t::DEAD)) {
        reason = u8"Sorry, the shopkeeper is dead!\n";
      } else if (shpkp->IsAct(act_t::DYING)) {
        reason = u8"Sorry, the shopkeeper is dying!\n";
      } else if (shpkp->IsAct(act_t::UNCONSCIOUS)) {
        reason = u8"Sorry, the shopkeeper is unconscious!\n";
      } else if (shpkp->IsAct(act_t::SLEEP)) {
        reason = u8"Sorry, the shopkeeper is asleep!\n";
      } else if (shpkp->ActTarg(act_t::SPECIAL_WORK) == body->Room() && shpkp->IsAct(act_t::WORK)) {
        price = shpkp->WouldBuyFor(item);

        if (price > 0) {
          shpkps.push_back(shpkp);
          break;
        }
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
      }
      mind->Send(u8"I'll give you {} for {}\n", coins(price), item->ShortDesc());

      if (cnum == COM_SELL) {
        auto pay = shpkp->PayFor(price);

        if (pay.size() <= 0) {
          if (mind) {
            auto offer = shpkp->CanPayFor(price);
            if (offer > price) {
              mind->Send(
                  u8"I don't have change to pay {}.  You'd have to make change for {}.\n",
                  coins(price),
                  coins(offer));
              auto refund = offer - price;
              auto change = body->PayFor(refund);
              if (change.size() > 0) {
                pay = shpkp->PayFor(offer);
                if (pay.size() <= 0) {
                  mind->Send(u8"...and something went horribly wrong (tell Stea).\n");
                } else {
                  for (auto coin : change) {
                    shpkp->Stash(coin, 0, 1);
                  }
                  mind->Send(u8"You pay {} to {}.\n", coins(refund), shpkp->ShortDesc());
                }
              } else {
                mind->Send(u8"...but you don't have that change.\n");
              }
            } else {
              mind->Send(u8"I can't afford the {} (I only have {}).\n", coins(price), coins(offer));
            }
          }
        }

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
            item->SetPosition(pos_t::LIE); // FIXME: Support Resellers
            item->TryCombine();
          }
          if (!success) {
            if (mind) {
              mind->Send(
                  u8"You couldn't stash {}, so at least some of it fell on the floor!\n",
                  coin_str(price));
            }
          }
        }
      }
    }
  }
  return 0;
}
