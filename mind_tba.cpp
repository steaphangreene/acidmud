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

// WARNING: Below Here Be Dragons.  You Can Not Unsee This "Code", Ever.
// This is just part of a dirty hack to throw some content into AcidMUD,
// by directly translating (re: shoe-horning) TBA's world data into AcidMUD.
// I actually have no plans to maintain or improve this, though may fix bugs.

#include <algorithm>

#include "color.hpp"
#include "commands.hpp"
#include "log.hpp"
#include "mind.hpp"
#include "net.hpp"
#include "object.hpp"
#include "properties.hpp"
#include "utils.hpp"

#define QUOTAERROR1 CRED u8"#{} Error: script quota exceeded - killed." CNRM
#define QUOTAERROR2 body->Skill(prhash(u8"TBAScript"))
#define PING_QUOTA()                  \
  {                                   \
    --quota;                          \
    if (quota < 1) {                  \
      loge(QUOTAERROR1, QUOTAERROR2); \
      return -1;                      \
    }                                 \
  }

static const std::u8string bstr[2] = {u8"0", u8"1"};

static std::u8string itos(int val) {
  return fmt::format(u8"{}", val);
}

static Object* decode_object(std::u8string_view& enc) {
  Object* ret = nullptr;
  if (process(enc, u8"obj:0x")) {
    // uintptr_t encint = gethex(enc); // Just preview the digits
    // ret = reinterpret_cast<Object*>(encint);
    // assert(fmt::format(u8"{}", reinterpret_cast<void*>(ret)).substr(2) == enc);
    // nexthex(enc); // Actually consume the digits
    uintptr_t encint = nexthex(enc); // Just preview the digits
    ret = reinterpret_cast<Object*>(encint);
  }
  return ret;
}

static Object* decode_object(const std::u8string_view& enc) {
  std::u8string_view stub = enc;
  return decode_object(stub);
}

static std::u8string tba_spellconvert(const std::u8string_view& tba) {
  std::u8string acid;

  switch (crc32c(tba)) {
    case (crc32c(u8"animate dead")): {
      acid = u8"Zombie Follower";
      break;
    }
    case (crc32c(u8"armor")): {
      acid = u8"Armor";
      break;
    }
    case (crc32c(u8"bless")): {
      acid = u8"Fortune";
      break;
    }
    case (crc32c(u8"blind")):
    case (crc32c(u8"blindness")): {
      acid = u8"Blindness";
      break;
    }
    case (crc32c(u8"burning hands")): {
      acid = u8"Burning Hands";
      break;
    }
    case (crc32c(u8"call lightning")): {
      acid = u8"Lightning Bolt";
      break;
    }
    case (crc32c(u8"charm")): {
      acid = u8"Influence";
      break;
    }
    case (crc32c(u8"chill touch")): {
      acid = u8"Ice Bolt";
      break;
    }
    case (crc32c(u8"clot minor")): {
      acid = u8"Shove";
      break;
    }
    case (crc32c(u8"color spray")): {
      acid = u8"Color Spray";
      break;
    }
    case (crc32c(u8"cure blind")): {
      acid = u8"Cure Blindness";
      break;
    }
    case (crc32c(u8"cure critic")): {
      acid = u8"Cure Critical Wounds";
      break;
    }
    case (crc32c(u8"cure light")): {
      acid = u8"Cure Light Wounds";
      break;
    }
    case (crc32c(u8"heal")): {
      acid = u8"Healing";
      break;
    }
    case (crc32c(u8"harm")): {
      acid = u8"Clout";
      break;
    }
    case (crc32c(u8"cure poison")): {
      acid = u8"Cure Poison";
      break;
    }
    case (crc32c(u8"curse")): {
      acid = u8"Cursed";
      break;
    }
    case (crc32c(u8"detect align")):
    case (crc32c(u8"det-align")): {
      acid = u8"Identify Character";
      break;
    }
    case (crc32c(u8"detect magic")):
    case (crc32c(u8"det-magic")): {
      acid = u8"Illuminate Magic";
      break;
    }
    case (crc32c(u8"detect invisibility")): {
      acid = u8"See Invisible";
      break;
    }
    case (crc32c(u8"detect poison")): {
      acid = u8"Detect Poison";
      break;
    }
    case (crc32c(u8"dispel evil")):
    case (crc32c(u8"dispel good")): {
      acid = u8"Bang and Puff";
      break;
    }
    case (crc32c(u8"earthquake")): {
      acid = u8"Earth Cataclysm";
      break;
    }
    case (crc32c(u8"energy drain")): {
      acid = u8"Weaken Subject";
      break;
    }
    case (crc32c(u8"fireball")): {
      acid = u8"Fireball";
      break;
    }
    case (crc32c(u8"fly")): {
      acid = u8"Fly";
      break;
    }
    case (crc32c(u8"infravision")):
    case (crc32c(u8"infra")): {
      acid = u8"Heat Vision";
      break;
    }
    case (crc32c(u8"invisibility")):
    case (crc32c(u8"invis")): {
      acid = u8"Invisibilty";
      break;
    }
    case (crc32c(u8"lightning bolt")): {
      acid = u8"Spark";
      break;
    }
    case (crc32c(u8"magic missile")): {
      acid = u8"Magic Missile";
      break;
    }
    case (crc32c(u8"poison")): {
      acid = u8"Poison";
      break;
    }
    case (crc32c(u8"protection from evil")):
    case (crc32c(u8"prot-evil")): {
      acid = u8"Protection";
      break;
    }
    case (crc32c(u8"refresh")): {
      acid = u8"Refresh";
      break;
    }
    case (crc32c(u8"remove curse")): {
      acid = u8"Remove Curse";
      break;
    }
    case (crc32c(u8"remove poison")): {
      acid = u8"Neutralize Poison";
      break;
    }
    case (crc32c(u8"sanctuary")):
    case (crc32c(u8"sanct")):
    case (crc32c(u8"sanc")): {
      acid = u8"Protection";
      break;
    }
    case (crc32c(u8"sense life")):
    case (crc32c(u8"sense-life")): {
      acid = u8"Sense Movement";
      break;
    }
    case (crc32c(u8"shocking grasp")): {
      acid = u8"Shocking Grasp";
      break;
    }
    case (crc32c(u8"silence")): {
      acid = u8"Silence";
      break;
    }
    case (crc32c(u8"strength")): {
      acid = u8"Strength";
      break;
    }
    case (crc32c(u8"sleep")): {
      acid = u8"Sleep Other";
      break;
    }
    case (crc32c(u8"waterwalk")):
    case (crc32c(u8"watwalk")): {
      acid = u8"Float";
      break;
    }
    case (crc32c(u8"word of recall")): {
      acid = u8"Recall";
      break;
    }
  }

  return acid;
}

std::u8string Mind::TBAComp(const std::u8string_view& in_expr) const {
  std::u8string expr(in_expr);
  size_t end = expr.find_first_of(u8"\n\r");
  if (end != std::u8string::npos)
    expr = expr.substr(0, end);
  trim_string(expr);

  if (expr[0] == '(') {
    size_t cls = expr.find(')');
    size_t opn = expr.find('(', 1);
    while (cls != std::u8string::npos && opn != std::u8string::npos && opn < cls) {
      cls = expr.find(')', cls + 1);
      opn = expr.find('(', opn + 1);
    }
    if (cls == std::u8string::npos)
      return TBAComp(expr.substr(1));
    expr = TBAComp(fmt::format(u8"{} {}", expr.substr(1, cls - 1), expr.substr(cls + 1)));
    trim_string(expr);
  }

  size_t skip = 0;
  if (expr[0] == '-')
    skip = 1; // Skip Leading u8"-"
  if (expr[0] == '!')
    skip = 1; // Skip Leading u8"!"
  size_t op = expr.find_first_of(u8"|&=!<>/-+*", skip);
  while (op != std::u8string::npos && expr[op] == '-' && ascii_isalpha(expr[op - 1]) &&
         ascii_isalpha(expr[op + 1])) {
    op = expr.find_first_of(u8"|&=!<>/-+*", op + 1); // Skip Hyphens
  }
  if (op == std::u8string::npos) {
    return std::u8string(expr); // No ops, just val
  }

  int oper = 0; // Positive for 2-char ops, negative for 1-char
  int weak = 0; // Reverse-Precedence!  Hack!!!
  if (expr.substr(op).starts_with(u8"/=")) {
    oper = 1;
  } else if (expr.substr(op).starts_with(u8"==")) {
    oper = 2;
  } else if (expr.substr(op).starts_with(u8"!=")) {
    oper = 3;
  } else if (expr.substr(op).starts_with(u8"<=")) {
    oper = 4;
  } else if (expr.substr(op).starts_with(u8">=")) {
    oper = 5;
  } else if (expr.substr(op).starts_with(u8"&&")) {
    oper = 6;
    weak = 1;
  } else if (expr.substr(op).starts_with(u8"||")) {
    oper = 7;
    weak = 1;
  } else if (expr.substr(op).starts_with(u8"<")) {
    oper = -1;
  } else if (expr.substr(op).starts_with(u8">")) {
    oper = -2;
  } else if (expr.substr(op).starts_with(u8"+")) {
    oper = -3;
  } else if (expr.substr(op).starts_with(u8"-")) {
    oper = -4;
  } else if (expr.substr(op).starts_with(u8"*")) {
    oper = -5;
  } else if (expr.substr(op).starts_with(u8"/")) {
    oper = -6;
  }

  if (oper != 0) {
    std::u8string arg1 = expr.substr(0, op);
    trim_string(arg1);
    if (oper > 0)
      expr = expr.substr(op + 2); // 2-char
    else
      expr = expr.substr(op + 1); // 1-char
    trim_string(expr);
    std::u8string arg2;
    if (expr[0] == '(') {
      size_t cls = expr.find(u8")"); // FIXME: Nested
      if (cls == std::u8string::npos)
        arg2 = TBAComp(expr.substr(1));
      else {
        arg2 = TBAComp(expr.substr(1, cls - 1));
        expr.replace(0, cls + 1, arg2);
      }
    }
    if (weak) {
      arg2 = TBAComp(expr);
      expr = u8"";
    } else {
      arg2 = expr;
      op = expr.find_first_of(u8"|&=!<>/-+*)\n\r");
      if (op != std::u8string::npos) {
        arg2 = expr.substr(0, op);
        expr = expr.substr(op);
      } else {
        expr = u8"";
      }
    }
    trim_string(arg2);

    int res = 0;
    std::u8string comp = u8"0";
    if (oper == 1 && arg2.contains(arg1)) {
      comp = u8"1";
    } else if (oper == 2 && (arg1 == arg2)) {
      comp = u8"1";
    } else if (oper == 3 && (arg1 != arg2)) {
      comp = u8"1";
    } else if (oper == 4 && (TBAEval(arg1) <= TBAEval(arg2))) {
      comp = u8"1";
    } else if (oper == 5 && (TBAEval(arg1) >= TBAEval(arg2))) {
      comp = u8"1";
    } else if (oper == -1 && (TBAEval(arg1) < TBAEval(arg2))) {
      comp = u8"1";
    } else if (oper == -2 && (TBAEval(arg1) > TBAEval(arg2))) {
      comp = u8"1";
    } else if (oper == 6 && (TBAEval(arg1) && TBAEval(arg2))) {
      comp = u8"1";
    } else if (oper == 7 && (TBAEval(arg1) || TBAEval(arg2))) {
      comp = u8"1";
    } else if (oper == -3) {
      res = TBAEval(arg1) + TBAEval(arg2);
    } else if (oper == -4) {
      res = TBAEval(arg1) - TBAEval(arg2);
    } else if (oper == -5) {
      res = TBAEval(arg1) * TBAEval(arg2);
    } else if (oper == -6) { // Protect from div by zero
      int val2 = TBAEval(arg2);
      res = TBAEval(arg1);
      if (val2 != 0)
        res /= val2;
    }

    if (oper <= -3) { // Non-Boolean - actual numeric value
      comp = itos(res);
      // loge(u8"RES: {}", buf);
    }

    if (expr != u8"") {
      expr = comp + u8" " + expr;
      return TBAComp(expr);
    }
    return comp;
  }

  return u8"0";
}

int Mind::TBAEval(const std::u8string_view& expr) const {
  std::u8string basestr = TBAComp(expr);
  std::u8string_view base(basestr);
  trim_string(base);

  if (base.length() == 0) {
    return 0; // Null
  }
  if (base.length() == 1 && base.front() == '!') {
    return 1; //! Null
  }

  int ret = nextnum(base);
  skipspace(base);
  if (base.length() == 0) {
    return ret; // Numeric
  }

  if (base.starts_with(u8"obj:0x")) {
    getuntil(base, 'x');
    uint64_t pointer = nexthex(base);
    if (base.length() == 0) {
      return (pointer != 0); // Object
    }
  }

  if (base.front() == '!') {
    nextchar(base);
    skipspace(base);

    ret = nextnum(base);
    skipspace(base);
    if (base.length() == 0) {
      return !ret; // Numeric
    }

    if (base.starts_with(u8"obj:0x")) {
      getuntil(base, 'x');
      uint64_t pointer = nexthex(base);
      if (base.length() == 0) {
        return (pointer == 0); // Object
      }
    }

    return 0; //! Non-Numeric, Non-nullptr, Non-Object
  }
  return 1; // Non-Numeric, Non-nullptr, Non-Object
}

std::u8string Mind::TBAMOBTactics(int phase) const {
  if (type == mind_t::TBAMOB) {
    // NON-HELPER and NON-AGGRESSIVE TBA Mobs (Innocent MOBs)
    if (body && (body->Skill(prhash(u8"TBAAction")) & 4128) == 0) {
      if (phase == -1) {
        return u8"call HELP; attack";
      }
    }
  }
  return u8"attack";
}

bool Mind::TBAMOBSend(const std::u8string_view& mes) {
  // HELPER TBA Mobs
  if (body && body->Parent() && (body->Skill(prhash(u8"TBAAction")) & 4096) // Helpers
      && ((body->Skill(prhash(u8"TBAAction")) & 2) == 0) // NON-SENTINEL
      && body->Stun() < 6 // I'm not stunned
      && body->Phys() < 6 // I'm not injured
      && (!body->IsAct(act_t::SLEEP)) // I'm not asleep
      && (!body->IsAct(act_t::REST)) // I'm not resting
      && (!body->IsAct(act_t::FIGHT)) // I'm not already fighting
  ) {
    if ((mes.starts_with(u8"From ")) && (mes.contains(u8" you hear someone shout '")) &&
        (mes.contains(u8"HELP") || mes.contains(u8"ALARM"))) {
      std::u8string_view dir = mes.substr(4);
      skipspace(dir);
      dir = getgraph(dir);

      Object* door = body->PickObject(dir, LOC_NEARBY);

      if (door && door->ActTarg(act_t::SPECIAL_LINKED) &&
          door->ActTarg(act_t::SPECIAL_LINKED)->Parent() &&
          TBACanWanderTo(door->ActTarg(act_t::SPECIAL_LINKED)->Parent())) {
        body->BusyFor(500, fmt::format(u8"enter {}", dir));
      }
      return true;
    }
  }
  return true;
}

bool Mind::TBAMOBThink(int istick) {
  if ((!body) || (istick >= 0 && body->StillBusy()))
    return true;

  // Temporary
  if (body && body->ActTarg(act_t::WEAR_SHIELD) && (!body->IsAct(act_t::HOLD))) {
    std::u8string command =
        fmt::format(u8"hold obj:{}", static_cast<void*>(body->ActTarg(act_t::WEAR_SHIELD)));
    body->BusyFor(500, command);
    return true;
  } else if (
      body && body->ActTarg(act_t::WEAR_SHIELD) && body->ActTarg(act_t::HOLD) &&
      body->ActTarg(act_t::WEAR_SHIELD) != body->ActTarg(act_t::HOLD)) {
    Object* targ = body->ActTarg(act_t::HOLD);
    if (body->Stash(targ, 0)) {
      if (body->Parent())
        body->Parent()->SendOut(ALL, 0, u8";s stashes ;s.\n", u8"", body, targ);
      std::u8string command =
          fmt::format(u8"hold obj:{}", static_cast<void*>(body->ActTarg(act_t::WEAR_SHIELD)));
      body->BusyFor(500, command);
    } else {
      // loge(u8"Warning: {} can't use his shield!", body->Noun());
    }
    return true;
  }

  // AGGRESSIVE and WIMPY TBA Mobs
  if (body && body->Parent() && (body->Skill(prhash(u8"TBAAction")) & 160) == 160 &&
      (!body->IsAct(act_t::FIGHT))) {
    auto others = body->PickObjects(u8"everyone", LOC_NEARBY);
    for (auto other : others) {
      if ((!other->Skill(prhash(u8"TBAAction"))) // FIXME: Other mobs?
          && body->Stun() < 6 // I'm not stunned
          && body->Phys() < 6 // I'm not injured
          && (!body->IsAct(act_t::SLEEP)) // I'm not asleep
          && (!body->IsAct(act_t::REST)) // I'm not resting
          && other->IsAct(act_t::SLEEP) // It's not awake (wuss!)
          && other->IsAnimate() // It's not a rock
          && (!other->IsAct(act_t::UNCONSCIOUS)) // It's not already KOed
          && (!other->IsAct(act_t::DYING)) // It's not already dying
          && (!other->IsAct(act_t::DEAD)) // It's not already dead
      ) {
        std::u8string command = fmt::format(u8"attack obj:{}", static_cast<void*>(other));
        body->BusyFor(500, command);
        // loge(u8"{}: Tried '{}'", body->ShortDesc(), command);
        return true;
      }
    }
    if (istick == 1 && body->IsUsing(prhash(u8"Perception"))) {
      body->BusyFor(500, u8"stop");
    } else if (
        istick == 0 // Triggered Only
        && (!body->IsUsing(prhash(u8"Perception"))) // Not already searching
        && (!body->StillBusy()) // Not already responding
        && body->Stun() < 6 // I'm not stunned
        && body->Phys() < 6 // I'm not injured
        && (!body->IsAct(act_t::SLEEP)) // I'm not asleep
        && (!body->IsAct(act_t::REST)) // I'm not resting
    ) {
      body->BusyFor(500, u8"search");
    }
  }
  // AGGRESSIVE and (!WIMPY) TBA Mobs
  else if (
      body && body->Parent() && (body->Skill(prhash(u8"TBAAction")) & 160) == 32 &&
      (!body->IsAct(act_t::FIGHT))) {
    auto others = body->PickObjects(u8"everyone", LOC_NEARBY);
    for (auto other : others) {
      if ((!other->Skill(prhash(u8"TBAAction"))) // FIXME: Other mobs?
          && body->Stun() < 6 // I'm not stunned
          && body->Phys() < 6 // I'm not injured
          && (!body->IsAct(act_t::SLEEP)) // I'm not asleep
          && (!body->IsAct(act_t::REST)) // I'm not resting
          && other->IsAnimate() // It's not a rock
          && (!other->IsAct(act_t::UNCONSCIOUS)) // It's not already KOed
          && (!other->IsAct(act_t::DYING)) // It's not already dying
          && (!other->IsAct(act_t::DEAD)) // It's not already dead
      ) {
        std::u8string command = fmt::format(u8"attack obj:{}", static_cast<void*>(other));
        body->BusyFor(500, command);
        // loge(u8"{}: Tried '{}'", body->ShortDesc(), command);
        return true;
      }
    }
    if (istick == 1 && body->IsUsing(prhash(u8"Perception"))) {
      body->BusyFor(500, u8"stop");
    } else if (
        istick == 0 // Triggered Only
        && (!body->IsUsing(prhash(u8"Perception"))) // Not already searching
        && (!body->StillBusy()) // Not already responding
        && body->Stun() < 6 // I'm not stunned
        && body->Phys() < 6 // I'm not injured
        && (!body->IsAct(act_t::SLEEP)) // I'm not asleep
        && (!body->IsAct(act_t::REST)) // I'm not resting
    ) {
      body->BusyFor(500, u8"search");
    }
  }
  // HELPER TBA Mobs
  if (body && body->Parent() && (body->Skill(prhash(u8"TBAAction")) & 4096) &&
      (!body->IsAct(act_t::FIGHT))) {
    auto others = body->PickObjects(u8"everyone", LOC_NEARBY);
    for (auto other : others) {
      if ((!other->Skill(prhash(u8"TBAAction"))) // FIXME: Other mobs?
          && body->Stun() < 6 // I'm not stunned
          && body->Phys() < 6 // I'm not injured
          && (!body->IsAct(act_t::SLEEP)) // I'm not asleep
          && (!body->IsAct(act_t::REST)) // I'm not resting
          && other->IsAnimate() // It's not a rock
          && (!other->IsAct(act_t::DEAD)) // It's not already dead
          && other->IsAct(act_t::FIGHT) // It's figting someone
          && other->ActTarg(act_t::FIGHT)->HasSkill(prhash(u8"TBAAction"))
          //...against another MOB
      ) {
        std::u8string command =
            fmt::format(u8"call ALARM; attack obj:{}", static_cast<void*>(other));
        body->BusyFor(500, command);
        // loge(u8"{}: Tried '{}'", body->ShortDesc(), command);
        return true;
      }
    }
    if (!body->IsUsing(prhash(u8"Perception"))) { // Don't let guard down!
      body->BusyFor(500, u8"search");
    } else if (
        istick == 1 // Perioidic searching
        && (!body->StillBusy()) // Not already responding
        && body->Stun() < 6 // I'm not stunned
        && body->Phys() < 6 // I'm not injured
        && (!body->IsAct(act_t::SLEEP)) // I'm not asleep
        && (!body->IsAct(act_t::REST)) // I'm not resting
    ) {
      body->BusyFor(500, u8"search");
    }
  }
  // NON-SENTINEL TBA Mobs
  if (body && body->Parent() && ((body->Skill(prhash(u8"TBAAction")) & 2) == 0) &&
      (!body->IsAct(act_t::FIGHT)) && (istick == 1) && (!body->IsAct(act_t::REST)) &&
      (!body->IsAct(act_t::SLEEP)) && body->Stun() < 6 && body->Phys() < 6 &&
      body->Roll(prhash(u8"Willpower"), 9)) {
    std::map<Object*, std::u8string> cons;
    cons[body->PickObject(u8"north", LOC_NEARBY)] = u8"north";
    cons[body->PickObject(u8"south", LOC_NEARBY)] = u8"south";
    cons[body->PickObject(u8"east", LOC_NEARBY)] = u8"east";
    cons[body->PickObject(u8"west", LOC_NEARBY)] = u8"west";
    cons[body->PickObject(u8"up", LOC_NEARBY)] = u8"up";
    cons[body->PickObject(u8"down", LOC_NEARBY)] = u8"down";
    cons.erase(nullptr);

    std::map<Object*, std::u8string> cons2 = cons;
    for (auto dir : cons2) {
      if ((!dir.first->ActTarg(act_t::SPECIAL_LINKED)) ||
          (!dir.first->ActTarg(act_t::SPECIAL_LINKED)->Parent())) {
        cons.erase(dir.first);
        continue;
      }

      Object* dest = dir.first->ActTarg(act_t::SPECIAL_LINKED)->Parent();
      if (!TBACanWanderTo(dest)) {
        cons.erase(dir.first);
      }
    }

    if (cons.size()) {
      int res = rand() % cons.size();
      std::map<Object*, std::u8string>::iterator dir = cons.begin();
      while (res > 0) {
        ++dir;
        --res;
      }
      if (body->StillBusy()) { // Already doing something (from above)
        body->DoWhenFree(dir->second);
      } else {
        body->BusyFor(500, dir->second);
      }
      return true;
    }
  }
  return true;
}

bool Mind::TBATriggerThink(int istick) {
  if (body && body->Parent() && spos_s.size() > 0) {
    //      logeg(u8"#{} Debug: Running Trigger.",
    //	body->Skill(prhash(u8"TBAScript"))
    //	);
    ovars[u8"self"] = body->Parent();
    ovars[u8"context"] = ovars[u8"self"]; // Initial global var context
    std::u8string_view script = svars[u8"script"];

    int quota = 1024;
    int stype = body->Skill(prhash(u8"TBAScriptType"));
    while (spos_s.size() > 0 && spos_s.back() < script.length() &&
           spos_s.back() != std::u8string::npos) {
      std::u8string line;
      size_t endl = script.find_first_of(u8"\n\r", spos_s.back());
      if (endl == std::u8string::npos)
        line = script.substr(spos_s.back());
      else
        line = script.substr(spos_s.back(), endl - spos_s.back());

      spos_s.back() = skip_line(script, spos_s.back());

      if (line[0] == '*')
        continue; // Comments

      PING_QUOTA();

      int ret = TBARunLine(line);
      if (ret < 0) {
        return false;
      } else if (ret > 0) {
        return true;
      }
    }
    if (stype & 2) { // Random Triggers
      if (!Body()->HasMultipleMinds()) { // Not Already Being Covered
        int chance = body->Skill(prhash(u8"TBAScriptNArg")); // Percent Chance
        if (chance > 0) {
          int delay = 13000; // Next try in 13 seconds.
          while (delay < 1300000 && (rand() % 100) >= chance)
            delay += 13000;
          spos_s.clear();
          spos_s.push_back(0); // We never die!
          Suspend(delay); // We'll be back!
          return true;
        }
      }
    }
  }
  return false;
}

// Return:
// 0 to continue running
// 1 to be done now (suspend)
// -1 to destroy mind (error/done)
int Mind::TBARunLine(std::u8string linestr) {
  if (0
      //	|| body->Skill(prhash(u8"TBAScript")) == 5000099
      //	|| linestr.contains(u8"eval loc ")
      //	|| linestr.contains(u8"set first ")
      //	|| linestr.contains(u8"exclaim")
      //	|| linestr.contains(u8"speech")
  ) {
    logeg(u8"#{} Debug: Running '{}'", body->Skill(prhash(u8"TBAScript")), linestr);
  }
  Object* room = ovars.at(u8"self");
  while (room && room->Skill(prhash(u8"TBARoom")) == 0) {
    if (room->Skill(prhash(u8"Invisible")) > 999)
      room = nullptr; // Not really there
    else
      room = room->Parent();
  }
  if (!room) { // Not in a room (dup clone, in a popper, etc...).
    //    loger(u8"#{} Error: No room in '{}'",
    //	body->Skill(prhash(u8"TBAScript")), linestr
    //	);
    return -1;
  }
  // Needs to be alive! MOB & MOB-* (Not -DEATH or -GLOBAL)
  if ((body->Skill(prhash(u8"TBAScriptType")) & 0x103FFDE) > 0x1000000) {
    if (ovars.at(u8"self")->IsAct(act_t::DEAD) || ovars.at(u8"self")->IsAct(act_t::DYING) ||
        ovars.at(u8"self")->IsAct(act_t::UNCONSCIOUS)) {
      //      logeg(u8"#{} Debug: Triggered on downed MOB.",
      //	body->Skill(prhash(u8"TBAScript"))
      //	);
      spos_s.back() = std::u8string::npos; // Jump to End
      return 0; // Allow re-run (in case of resurrection/waking/etc...).
    }
  }

  size_t spos = spos_s.back();
  int vnum = body->Skill(prhash(u8"TBAScript"));
  if (!TBAVarSub(linestr)) {
    loger(u8"#{} Error: VarSub failed in '{}'", vnum, linestr);
    return -1;
  }

  int com = COM_NONE; // ComNum for Pass-Through
  std::u8string_view line = linestr;
  std::u8string_view cmd = line;
  std::u8string_view script = svars[u8"script"];

  auto c1 = cmd.find_first_not_of(u8" \t\n\r;");
  if (c1 != std::u8string::npos) {
    auto c2 = cmd.find_first_of(u8" \t\n\r;", c1 + 1);
    if (c2 == std::u8string::npos) {
      cmd = cmd.substr(c1);
    } else {
      cmd = cmd.substr(c1, c2 - c1);
    }

    com = identify_command(cmd, true);
  }

  //  //Start of script command if/else if/else
  //  if(line.find(u8"%") != line.rfind(u8"%")) {		//More than one '%'
  //    loger(u8"#{} Error: Failed to fully expand '{}'",
  //	body->Skill(prhash(u8"TBAScript")), line
  //	);
  //    return -1;
  //    }

  if (line.starts_with(u8"unset ")) {
    size_t lpos = line.find_first_not_of(u8" \t", 6);
    if (lpos != std::u8string::npos) {
      std::u8string_view var = line.substr(lpos);
      trim_string(var);
      svars.erase(std::u8string(var));
      ovars.erase(std::u8string(var));
      ovars[u8"context"]->ClearSkill(fmt::format(u8"TBA:{}", var));
    } else {
      loger(u8"#{} Error: Malformed unset '{}'", body->Skill(prhash(u8"TBAScript")), line);
      return -1;
    }
    return 0;
  }

  else if ((line.starts_with(u8"eval ")) || (line.starts_with(u8"set "))) {
    char8_t coml = ascii_tolower(line[0]); // To tell eval from set later.
    size_t lpos = line.find_first_not_of(u8" \t", 4);
    if (lpos != std::u8string::npos) {
      line = line.substr(lpos);
      size_t end1 = line.find_first_of(u8" \t\n\r");
      if (end1 != std::u8string::npos) {
        std::u8string_view var = line.substr(0, end1);
        lpos = line.find_first_not_of(u8" \t", end1 + 1);
        if (lpos != std::u8string::npos) {
          std::u8string val(line.substr(lpos));
          if (0
              //		|| var.contains(u8"midgaard")
              //		|| var.contains(u8"exclaim")
              //		|| var.contains(u8"speech")
          ) {
            logeg(u8"#{} Debug: '{}' = '{}'", body->Skill(prhash(u8"TBAScript")), var, val);
          }
          if (coml == 'e') {
            int valnum = body->Skill(prhash(u8"TBAScript"));
            if (!TBAVarSub(val)) {
              loger(u8"#{} Error: Eval failed in '{}'", valnum, line);
              return -1;
            }
            val = TBAComp(val);
          }
          if (val.starts_with(u8"obj:")) { // Encoded Object
            ovars[std::u8string(var)] = decode_object(val);
            svars.erase(std::u8string(var));
          } else {
            svars[std::u8string(var)] = val;
            ovars.erase(std::u8string(var));
          }
        } else { // Only space after varname
          svars[std::u8string(var)] = u8"";
          ovars.erase(std::u8string(var));
        }
      } else { // Nothing after varname
        svars[std::u8string(line)] = u8"";
        ovars.erase(std::u8string(line));
      }
    }
    return 0;
  }

  else if ((line.starts_with(u8"extract "))) {
    size_t lpos = line.find_first_not_of(u8" \t", 8);
    if (lpos != std::u8string::npos) {
      line = line.substr(lpos);
      size_t end1 = line.find_first_of(u8" \t\n\r");
      if (end1 != std::u8string::npos) {
        std::u8string_view var = line.substr(0, end1);
        lpos = line.find_first_not_of(u8" \t", end1 + 1);
        if (lpos != std::u8string::npos) {
          int wnum = getnum(line.substr(lpos)) - 1; // Start at 0, -1=fail
          lpos = line.find_first_of(u8" \t", lpos);
          if (lpos != std::u8string::npos)
            lpos = line.find_first_not_of(u8" \t", lpos);
          if (wnum >= 0 && lpos != std::u8string::npos) {
            while (wnum > 0 && lpos != std::u8string::npos) {
              lpos = line.find_first_of(u8" \t", lpos);
              if (lpos != std::u8string::npos) {
                lpos = line.find_first_not_of(u8" \t", lpos);
              }
              --wnum;
            }
            if (lpos != std::u8string::npos) {
              end1 = line.find_first_of(u8" \t\n\r", lpos);
              if (end1 == std::u8string::npos)
                end1 = line.length();
              svars[std::u8string(var)] = line.substr(lpos, end1 - lpos);
            } else {
              svars[std::u8string(var)] = u8"";
            }
            ovars.erase(std::u8string(var));
          } else if (wnum < 0) { // Bad number after varname
            loger(
                u8"#{} Error: Malformed extract '{}'\n", body->Skill(prhash(u8"TBAScript")), line);
            return -1;
          }
        } else { // Only space after varname
          loger(u8"#{} Error: Malformed extract '{}'", body->Skill(prhash(u8"TBAScript")), line);
          return -1;
        }
      } else { // Nothing after varname
        loger(u8"#{} Error: Malformed extract '{}'", body->Skill(prhash(u8"TBAScript")), line);
        return -1;
      }
    }
    return 0;
  }

  else if (process(line, u8"at ")) {
    int dnum = nextnum(line);
    skipspace(line);
    // if (ssc:anf(std::u8string(line).c_str(), u8"at %d %n", &dnum, &pos) < 1) {
    //   loger(u8"#{} Error: Malformed at '{}'", body->Skill(prhash(u8"TBAScript")), line);
    //   return -1;
    // }
    room = room->Zone();
    auto options = room->Contents();

    room = nullptr;
    dnum += 1000000;
    for (auto opt : options) {
      int tnum = opt->Skill(prhash(u8"TBARoom"));
      if (tnum > 0 && tnum == dnum) {
        room = opt;
        break;
      }
    }
    if (!room) {
      loger(u8"#{} Error: Can't find room in '{}'", body->Skill(prhash(u8"TBAScript")), line);
      return -1;
    }
    Object* oldp = nullptr;
    if (ovars.at(u8"self")->Parent() != room) {
      oldp = ovars.at(u8"self")->Parent();
      oldp->RemoveLink(ovars.at(u8"self"));
      ovars.at(u8"self")->SetParent(room);
    }
    int ret = TBARunLine(std::u8string(line));
    if (oldp) {
      ovars.at(u8"self")->Parent()->RemoveLink(ovars.at(u8"self"));
      ovars.at(u8"self")->SetParent(oldp);
    }
    return ret;
  }

  else if (process(line, u8"context ")) {
    skipspace(line);
    Object* con = decode_object(line);
    if (con != nullptr) {
      ovars[u8"context"] = con;
    } else {
      loger(u8"#{} Error: No Context Object '{}'", body->Skill(prhash(u8"TBAScript")), line);
      return 1;
    }
  }

  else if (process(line, u8"rdelete ")) {
    skipspace(line);
    std::u8string_view var = getgraph(line);
    skipspace(line);
    Object* con = decode_object(line);
    if (var.length() > 0 && con != nullptr) {
      con->ClearSkill(fmt::format(u8"TBA:{}", var));
      if (con->IsAnimate()) {
        con->Accomplish(body->Skill(prhash(u8"Accomplishment")), u8"completing a quest");
      }
      //      logeg(u8"#{} Debug: RDelete '{}'",
      //		body->Skill(prhash(u8"TBAScript")), line);
    } else {
      loger(u8"#{} Error: No RDelete VarName/ID '{}'", body->Skill(prhash(u8"TBAScript")), line);
      return 1;
    }
  }

  else if (process(line, u8"remote ")) {
    skipspace(line);
    std::u8string_view var = getgraph(line);
    skipspace(line);
    Object* con = decode_object(line);
    if (var.length() > 0 && con != nullptr) {
      if (svars.contains(std::u8string(var))) {
        int val = getnum(svars[std::u8string(var)]);
        con->SetSkill(fmt::format(u8"TBA:{}", var), val);
        if (con->IsAnimate()) {
          con->Accomplish(body->Skill(prhash(u8"Accomplishment")), u8"role playing");
        }
        //	logeg(u8"#{} Debug: Remote {}={} '{}'",
        //		body->Skill(prhash(u8"TBAScript")), var, val, line);
      } else {
        loger(
            u8"#{} Error: Non-Existent Remote {} '{}'\n",
            body->Skill(prhash(u8"TBAScript")),
            var,
            line);
        return 1;
      }
    } else {
      loger(u8"#{} Error: No Remote VarName/ID '{}'", body->Skill(prhash(u8"TBAScript")), line);
      return 1;
    }
  }

  else if (process(line, u8"global ")) {
    Object* con = ovars[u8"context"];
    if (con != nullptr) {
      std::u8string_view var = getgraph(line);
      if (svars.count(std::u8string(var)) > 0) {
        int val = getnum(svars[std::u8string(var)]);
        con->SetSkill(fmt::format(u8"TBA:{}", var), val);
        //	logeg(u8"#{} Debug: Global {}={} '{}'",
        //		body->Skill(prhash(u8"TBAScript")), var, val, line);
      } else {
        loger(
            u8"#{} Error: Non-Existent Global {} '{}'\n",
            body->Skill(prhash(u8"TBAScript")),
            var,
            line);
        return 1;
      }
    } else {
      loger(u8"#{} Error: No Global VarName '{}'", body->Skill(prhash(u8"TBAScript")), line);
      return 1;
    }
  }

  else if (process(line, u8"wait until ")) {
    skipspace(line);
    int hour = nextnum(line);
    if (nextchar(line) == ':') {
      int minute = nextnum(line);
      if (hour >= 100) {
        hour /= 100;
      }
      minute += hour * 60;
      Object* world = room->World();
      int cur = 0;
      if (world && world->Skill(prhash(u8"Day Time")) && world->Skill(prhash(u8"Day Length"))) {
        cur = world->Skill(prhash(u8"Day Time"));
        cur *= 24 * 60;
        cur /= world->Skill(prhash(u8"Day Length"));
      }
      if (minute < cur) {
        minute += 24 * 60; // Not Time Until Tomorrow!
      }
      if (minute > cur) { // Not Time Yet!
        Suspend((minute - cur) * 1000 * world->Skill(prhash(u8"Day Length")) / 24);
        // Note: The above calculation removed the *60 and the /60
        return 1;
      }
    } else {
      loger(u8"#{} Error: Told '{}'", body->Skill(prhash(u8"TBAScript")), line);
      return -1;
    }
  }

  else if (process(line, u8"wait ")) {
    skipspace(line);
    int time = nextnum(line);
    if (time > 0) {
      // if(body->Skill(prhash(u8"TBAScript")) >= 5034503 && body->Skill(prhash(u8"TBAScript")) <=
      // 5034507)
      //  logeb(u8"#{} Suspending for: {}",
      //  body->Skill(prhash(u8"TBAScript")), time * 1000);
      Suspend(time * 1000);
      return 1;
    } else {
      loger(u8"#{} Error: Told '{}'", body->Skill(prhash(u8"TBAScript")), line);
      return -1;
    }
  }

  else if (process(line, u8"oset ") || process(line, u8"osetval ")) {
    skipspace(line);
    int v1 = nextnum(line);
    skipspace(line);
    int v2 = nextnum(line);
    if (ovars[u8"self"]->Skill(prhash(u8"Liquid Source")) && v1 == 0) {
      if (v2 < 0)
        v2 = 1 << 30;
      ovars[u8"self"]->SetSkill(prhash(u8"Liquid Source"), v2 + 1);
    } else if (ovars[u8"self"]->Skill(prhash(u8"Liquid Source")) && v1 == 1) {
      if (ovars[u8"self"]->Contents().size() < 1) {
        logey(u8"#{} Warning: Empty fountain '{}'", body->Skill(prhash(u8"TBAScript")), line);
        return -1;
      }
      ovars[u8"self"]->Contents().front()->SetQuantity(v2 + 1);
    } else {
      loger(u8"#{} Error: Unimplemented oset '{}'", body->Skill(prhash(u8"TBAScript")), line);
      return -1;
    }
  }

  else if (line.starts_with(u8"if ")) {
    if (!TBAEval(line.substr(3))) { // Was false
      int depth = 0;
      while (spos != std::u8string::npos) { // Skip to end/elseif
        //        PING_QUOTA();
        if ((!depth) && (script.substr(spos).starts_with(u8"elseif "))) {
          spos += 4; // Make it into an u8"if" and go
          break;
        } else if (script.substr(spos).starts_with(u8"else")) {
          if (!depth) { // Only right if all the way back
            spos = skip_line(script, spos);
            break;
          }
        } else if (script.substr(spos).starts_with(u8"end")) {
          if (!depth) { // Only done if all the way back
            spos = skip_line(script, spos);
            break;
          }
          --depth; // Otherwise am just 1 nesting level less deep
        } else if (script.substr(spos).starts_with(u8"if ")) {
          ++depth; // Am now 1 nesting level deeper!
        }
        spos = skip_line(script, spos);
      }
      spos_s.back() = spos; // Save skip-to position in real PC
    }
  }

  else if (line.starts_with(u8"else")) { // else/elseif
    int depth = 0;
    while (spos != std::u8string::npos) { // Skip to end (considering nesting)
      //      PING_QUOTA();
      if (script.substr(spos).starts_with(u8"end")) {
        if (depth == 0) { // Only done if all the way back
          spos = skip_line(script, spos);
          break;
        }
        --depth; // Otherwise am just 1 nesting level less deep
      } else if (script.substr(spos).starts_with(u8"if ")) {
        ++depth; // Am now 1 nesting level deeper!
      }
      spos = skip_line(script, spos);
    }
    spos_s.back() = spos; // Save skip-to position in real PC
  }

  else if (line.starts_with(u8"while ")) {
    int depth = 0;
    size_t rep = prev_line(script, spos);
    size_t begin = spos;
    while (spos != std::u8string::npos) { // Skip to end (considering nesting)
      //      PING_QUOTA();
      if (script.substr(spos).starts_with(u8"done")) {
        if (depth == 0) { // Only done if all the way back
          spos = skip_line(script, spos);
          break;
        }
        --depth; // Otherwise am just 1 nesting level less deep
      } else if (script.substr(spos).starts_with(u8"switch ")) {
        ++depth; // Am now 1 nesting level deeper!
      } else if (script.substr(spos).starts_with(u8"while ")) {
        ++depth; // Am now 1 nesting level deeper!
      }
      spos = skip_line(script, spos);
    }
    if (TBAEval(line.substr(6))) {
      spos_s.back() = rep; // Will repeat the u8"while"
      spos_s.push_back(begin); // But run the inside of the loop first.
    } else {
      spos_s.back() = spos; // Save after-done position in real PC
    }
  }

  else if (!!line.starts_with(u8"switch ")) {
    int depth = 0;
    size_t targ = 0;
    std::u8string_view value = line.substr(7);
    trim_string(value);
    while (spos != std::u8string::npos) { // Skip to end (considering nesting)
      //      PING_QUOTA();
      if (script.substr(spos).starts_with(u8"done")) {
        if (depth == 0) { // Only done if all the way back
          spos = skip_line(script, spos);
          break;
        }
        --depth; // Otherwise am just 1 nesting level less deep
      } else if (script.substr(spos).starts_with(u8"switch ")) {
        ++depth; // Am now 1 nesting level deeper!
      } else if (script.substr(spos).starts_with(u8"while ")) {
        ++depth; // Am now 1 nesting level deeper!
      } else if (
          depth == 0 && (script.substr(spos).starts_with(u8"case ")) &&
          TBAEval(fmt::format(u8"{} == {}", value, script.substr(spos + 5)))) {
        // The actual case I want!
        spos = skip_line(script, spos);
        targ = spos;
        continue;
      } else if (depth == 0 && (script.substr(spos).starts_with(u8"default"))) {
        // Maybe the case I want
        if (targ == 0) {
          spos = skip_line(script, spos);
          targ = spos;
          continue;
        }
      }
      spos = skip_line(script, spos);
    }
    spos_s.back() = spos; // Save after-done position in real PC
    if (targ != 0) { // Got a case to go to
      spos_s.push_back(targ); // Push jump-to position above real PC
    }
  }

  else if (line.starts_with(u8"break")) { // Skip to done
    int depth = 0;
    while (spos != std::u8string::npos) { // Skip to end (considering nesting)
      //      PING_QUOTA();
      if (script.substr(spos).starts_with(u8"done")) {
        if (depth == 0) { // Only done if all the way back
          break;
        }
        --depth; // Otherwise am just 1 nesting level less deep
      } else if (script.substr(spos).starts_with(u8"switch ")) {
        ++depth; // Am now 1 nesting level deeper!
      } else if (script.substr(spos).starts_with(u8"while ")) {
        ++depth; // Am now 1 nesting level deeper!
      }
      spos = skip_line(script, spos);
    }
    spos_s.back() = spos; // Save done position in real PC
  }

  else if ((!!line.starts_with(u8"asound "))) {
    size_t start = line.find_first_not_of(u8" \t\r\n", 7);
    if (room && start != std::u8string::npos) {
      std::u8string mes(line.substr(start));
      trim_string(mes);
      replace_all(mes, u8"You hear ", u8"");
      replace_all(mes, u8" can be heard close by", u8"");
      replace_all(mes, u8" is heard close by", u8"");
      replace_all(mes, u8" from close by", u8"");
      replace_all(mes, u8" from nearby", u8"");
      mes += u8"\n";
      room->Loud(2, mes); // 2 will go through 1 closed door.
    }
  }

  else if ((line.starts_with(u8"zoneecho "))) {
    size_t start = line.find_first_not_of(u8" \t\r\n", 9);
    start = line.find_first_of(u8" \t\r\n", start);
    start = line.find_first_not_of(u8" \t\r\n", start);
    if (room && start != std::u8string::npos) {
      std::u8string mes(line.substr(start));
      trim_string(mes);
      mes.push_back('\n');
      room->SendOut(ALL, 0, mes, u8"", nullptr, nullptr, false);
      room->Loud(8, mes); // 8 will go 4-8 rooms.
    }
  }

  else if (process(line, u8"echoaround ")) {
    skipspace(line);
    Object* targ = decode_object(line);
    skipspace(line);
    std::u8string mes(getuntil(line, '\n'));
    mes.push_back('\n');
    Object* troom = targ;
    while (troom && troom->Skill(prhash(u8"TBARoom")) == 0)
      troom = troom->Parent();
    if (troom && targ)
      troom->SendOut(0, 0, mes, u8"", targ, nullptr);
  }

  else if ((line.starts_with(u8"mecho "))) {
    size_t start = line.find_first_not_of(u8" \t\r\n", 6);
    if (room && start != std::u8string::npos) {
      std::u8string mes(line.substr(start));
      trim_string(mes);
      mes.push_back('\n');
      room->SendOut(0, 0, mes, mes, nullptr, nullptr);
    }
  }

  else if (process(line, u8"send ")) {
    skipspace(line);
    Object* targ = decode_object(line);
    skipspace(line);
    std::u8string mes(getuntil(line, '\n'));
    mes.push_back('\n');
    if (targ)
      targ->Send(0, 0, mes);
  }

  else if (process(line, u8"force ")) {
    skipspace(line);
    Object* targ = decode_object(line);
    if (!targ) {
      targ = room->PickObject(getgraph(line), LOC_NINJA | LOC_INTERNAL);
    }
    if (targ) {
      std::shared_ptr<Mind> amind = nullptr; // Make sure human minds see it!
      std::vector<std::shared_ptr<Mind>> mns = get_human_minds();
      for (auto mn : mns) {
        if (mn->Body() == targ) {
          amind = mn;
          break;
        }
      }
      skipspace(line);
      handle_command(targ, line, amind);
    }
  }

  else if (process(line, u8"wdamage ")) {
    //    logeg(u8"#{} Debug: WDamage '{}'",
    //	body->Skill(prhash(u8"TBAScript")), line
    //	);
    int dam = 0;
    std::u8string_view tname = getgraph(line);
    skipspace(line);

    if (tname == u8"all") {
      tname = u8"everyone";
    }
    dam = TBAEval(line);
    if (dam > 0)
      dam = (dam + 180) / 100;
    if (dam < 0)
      dam = (dam - 180) / 100;

    auto options = room->Contents();
    for (auto opt : options) {
      if (opt->Matches(tname)) {
        if (dam > 0) {
          opt->HitMent(1000, dam, 0);
          //	  logeg(u8"#{} Debug: WDamage '{}', {}",
          //		body->Skill(prhash(u8"TBAScript")), opt->Noun(), dam
          //		);
        } else if (dam < 0) {
          opt->HealStun(((-dam) + 1) / 2);
          opt->HealPhys(((-dam) + 1) / 2);
          //	  logeg(u8"#{} Debug: WHeal '{}', {}",
          //		body->Skill(prhash(u8"TBAScript")), opt->Noun(), ((-dam)+1)/2
          //		);
        }
      }
    }
  }

  else if (process(line, u8"door ")) {
    int rnum = nextnum(line);
    skipspace(line);
    std::u8string_view dir = getgraph(line);
    skipspace(line);

    // Handle abbreviated standard directions.
    if (std::u8string_view(u8"north").starts_with(dir)) {
      dir = u8"north";
    } else if (std::u8string_view(u8"south").starts_with(dir)) {
      dir = u8"south";
    } else if (std::u8string_view(u8"east").starts_with(dir)) {
      dir = u8"east";
    } else if (std::u8string_view(u8"west").starts_with(dir)) {
      dir = u8"west";
    } else if (std::u8string_view(u8"up").starts_with(dir)) {
      dir = u8"up";
    } else if (std::u8string_view(u8"down").starts_with(dir)) {
      dir = u8"down";
    }

    auto options = room->Zone()->Contents();
    room = nullptr;
    rnum += 1000000;
    for (auto opt : options) {
      int tbanum = opt->Skill(prhash(u8"TBARoom"));
      if (tbanum > 0 && tbanum == rnum) {
        room = opt;
        break;
      }
    }
    if (!room) {
      loger(u8"#{} Error: can't find target in '{}'", body->Skill(prhash(u8"TBAScript")), line);
      return -1;
    }

    Object* door = room->PickObject(dir, LOC_NINJA | LOC_INTERNAL);

    int tnum;

    if (process(line, u8"description ") >= 1) {
      //      logeg(u8"#{} Debug: door redesc '{}'",
      //	body->Skill(prhash(u8"TBAScript")), line
      //	);
      if (door) {
        door->SetDesc(line);
      }
    } else if (process(line, u8"flags ") >= 1) {
      if (!door) {
        loger(
            u8"#{} Error: No {} door to reflag in '{}'\n",
            body->Skill(prhash(u8"TBAScript")),
            dir,
            line);
        return -1;
      }
      uint32_t newfl = tba_bitvec(line);
      if ((newfl & 0xF) == 0) {
        loger(
            u8"#{} Error: bad door reflag (x{:X}) in '{}'\n",
            body->Skill(prhash(u8"TBAScript")),
            newfl,
            line);
        return -1;
      }
      if (newfl & 1) { // Can Open/Close
        door->SetSkill(prhash(u8"Open"), 1000);
        door->SetSkill(prhash(u8"Closeable"), 1);
        door->ClearSkill(prhash(u8"Locked"));
        door->SetSkill(prhash(u8"Lockable"), 1);
        door->SetSkill(prhash(u8"Pickable"), 4);
        // logeg(u8"#{} Debug: {} door can open/close in '{}'\n ",
        //       body->Skill(prhash(u8"TBAScript")),
        //       dir,
        //       line);
      }
      if (newfl & 2) { // Closed
        door->ClearSkill(prhash(u8"Open"));
        // logeg(
        //     u8"#{} Debug: {} door is closed in '{}'\n",
        //     body->Skill(prhash(u8"TBAScript")),
        //     dir,
        //     line);
      }
      if (newfl & 4) { // Locked
        door->SetSkill(prhash(u8"Locked"), 1);
        door->SetSkill(prhash(u8"Lockable"), 1);
        door->SetSkill(prhash(u8"Pickable"), 4);
        //	logeg(u8"#{} Debug: {} door is locked in '{}'"
        //,
        //		body->Skill(prhash(u8"TBAScript")), dir, line
        //		);
      }
      if (newfl & 8) { // Pick-Proof
        door->SetSkill(prhash(u8"Pickable"), 1000);
        //	logeg(u8"#{} Debug: {} door is pick-proof in
        //'{}'\n",
        //		body->Skill(prhash(u8"TBAScript")), dir, line
        //		);
      }
    } else if (process(line, u8"name ") >= 1) {
      //      logeg(u8"#{} Debug: door rename '{}'",
      //	body->Skill(prhash(u8"TBAScript")), line
      //	);
      if (door) {
        std::u8string newname(door->ShortDesc());
        size_t end = newname.find(u8"(");
        if (end != std::u8string::npos) {
          end = newname.find_last_not_of(u8" \t", end - 1);
          if (end != std::u8string::npos)
            newname = newname.substr(0, end);
        }
        door->SetShortDesc(fmt::format(u8"{} ({})", newname, line));
      }
    } else if (process(line, u8"room ") >= 1) {
      //      logeg(u8"#{} Debug: door relink '{}'",
      //	body->Skill(prhash(u8"TBAScript")), line
      //	);
      tnum = nextnum(line);
      skipspace(line);
      if (door)
        door->Recycle();
      Object* toroom = nullptr;
      tnum += 1000000;
      for (auto opt : options) {
        int onum = opt->Skill(prhash(u8"TBARoom"));
        if (onum > 0 && onum == tnum) {
          toroom = opt;
          break;
        }
      }
      if (!toroom) {
        loger(u8"#{} Error: can't find dest in '{}'", body->Skill(prhash(u8"TBAScript")), line);
        return -1;
      }
      door = new Object(room);
      door->SetShortDesc(dir);
      Object* odoor = new Object(toroom);
      odoor->SetShortDesc(u8"a passage exit");
      door->SetSkill(prhash(u8"Enterable"), 1);
      door->SetSkill(prhash(u8"Open"), 1000);
      door->AddAct(act_t::SPECIAL_LINKED, odoor);
      odoor->SetSkill(prhash(u8"Enterable"), 1);
      odoor->SetSkill(prhash(u8"Open"), 1000);
      odoor->SetSkill(prhash(u8"Invisible"), 1000);
    } else if (process(line, u8"key ") >= 1) {
      tnum = nextnum(line);
      skipspace(line);
      if (!door) {
        loger(
            u8"#{} Error: No {} door to re-key in '{}'\n",
            body->Skill(prhash(u8"TBAScript")),
            dir,
            line);
        return -1;
      }
      door->SetSkill(prhash(u8"Lockable"), 1);
      door->SetSkill(prhash(u8"Key"), 1000000 + tnum);
      if (door->Skill(prhash(u8"Pickable")) < 1)
        door->SetSkill(prhash(u8"Pickable"), 4);
      //      logeg(u8"#{} Debug: {} door re-keyed ({}) in '{}'"
      //     ,
      //	body->Skill(prhash(u8"TBAScript")), dir, tnum, line
      //	);
    } else if (process(line, u8"purge") >= 1) {
      //      logeg(u8"#{} Debug: door purge '{}'",
      //	body->Skill(prhash(u8"TBAScript")), line
      //	);
      if (door)
        door->Recycle();
    } else {
      loger(u8"#{} Error: bad door command '{}'", body->Skill(prhash(u8"TBAScript")), line);
      return -1;
    }
  }

  else if (process(line, u8"transport ")) {
    std::u8string_view tname = getgraph(line);
    skipspace(line);
    int dnum = nextnum(line);

    int nocheck = 0;
    if (tname == u8"all") {
      tname = u8"everyone";
      nocheck = 1;
    }
    Object* dest = ovars[u8"self"]->World();
    auto zones = dest->Contents();
    dest = nullptr;
    dnum += 1000000;
    bool done = false;
    for (auto zon : zones) {
      auto options = zon->Contents();
      for (auto opt : options) {
        int tnum = opt->Skill(prhash(u8"TBARoom"));
        if (tnum > 0 && tnum == dnum) {
          dest = opt;
          auto targs = room->Contents();
          for (auto targ : targs) {
            if (targ->Matches(tname)) {
              targ->Parent()->RemoveLink(targ);
              targ->SetParent(dest);
              nocheck = 1;
              break;
            }
          }
          done = true;
          break;
        }
      }
      if (done) {
        break;
      }
    }
    if (!nocheck) { // Check for a MOB by that UNIQUE name ANYWHERE.
      room = room->Parent();
      Object* targ = room->PickObject(fmt::format(u8"all's {}", tname), LOC_INTERNAL);
      if (targ) {
        targ->Parent()->RemoveLink(targ);
        targ->SetParent(dest);
      }
    }
    //    logeg(u8"#{} Debug: Transport line: '{}'",
    //	body->Skill(prhash(u8"TBAScript")), line
    //	);
  }

  else if (process(line, u8"purge ")) {
    skipspace(line);
    Object* targ = decode_object(line);
    if (targ) {
      if (!is_pc(targ))
        targ->Recycle();
    } else if (line.length() <= 6) { // No Args: Nuke All (but dirs and PCs)!
      auto tokill = room->PickObjects(u8"everyone", LOC_DARK | LOC_HEAT | LOC_INTERNAL);
      auto tokill2 = room->PickObjects(u8"everything", LOC_DARK | LOC_HEAT | LOC_INTERNAL);
      tokill.insert(tokill.end(), tokill2.begin(), tokill2.end());
      for (auto item : tokill) {
        if (!is_pc(item))
          item->Recycle();
      }
    } else {
      loger(u8"#{} Error: Bad purge target '{}'", body->Skill(prhash(u8"TBAScript")), line);
      return 1;
    }
    if (!body) { // No longer connected, must have purged self or parent.
      return -1;
    }
  }

  else if (process(line, u8"load ")) {
    int mask = 0;
    act_t loc = act_t::NONE;
    Object* dest = ovars[u8"self"];
    Object* item = nullptr;
    int params = 1;
    int tbatype = ascii_tolower(getgraph(line)[0]);
    int valnum = 0;
    std::u8string_view targ = u8"";
    std::u8string_view where = u8"";
    if (line.length() > 0 && nextchar(line) == ' ') {
      params = 2;
      valnum = nextnum(line);
      if (line.length() > 0 && nextchar(line) == ' ') {
        params = 3;
        targ = getgraph(line);
        if (line.length() > 0 && nextchar(line) == ' ') {
          params = 4;
          where = getgraph(line);
        }
      }
    }
    if ((params != 2 && params != 4) || (tbatype != 'o' && tbatype != 'm')) {
      loger(
          u8"#{} Error: Nonsensical script load '{:c}' '{}' '{}' '{}' [{}]\n",
          body->Skill(prhash(u8"TBAScript")),
          tbatype,
          valnum,
          targ,
          where,
          params);
      return -1;
    }
    Object* src = room->World();
    if (tbatype == 'o') {
      src = src->PickObject(u8"tbamud object room", LOC_NINJA | LOC_INTERNAL);
      if (src == nullptr) {
        loger(
            u8"#{} Error: Can't find Object room '{}'\n", body->Skill(prhash(u8"TBAScript")), line);
        return 1;
      }
      auto options = src->Contents();
      for (auto opt : options) {
        if (opt->Skill(prhash(u8"TBAObject")) == valnum + 1000000) {
          item = new Object(*opt);
          break;
        }
      }
    } else if (tbatype == 'm') {
      dest = room;
      src = src->PickObject(u8"tbamud mob room", LOC_NINJA | LOC_INTERNAL);
      if (src == nullptr) {
        loger(u8"#{} Error: Can't find MOB room '{}'", body->Skill(prhash(u8"TBAScript")), line);
        return 1;
      }
      auto options = src->Contents();
      for (auto opt : options) {
        if (opt->Skill(prhash(u8"TBAMOB")) == valnum + 1000000) {
          item = new Object(*opt);
          break;
        }
      }
    }
    if (item == nullptr) {
      loger(u8"#{} Error: Failed to find item '{}'", body->Skill(prhash(u8"TBAScript")), line);
      return -1;
    }
    if (params > 2) {
      //      logeg(u8"#{} Debug: ({}) '{}'",
      //	body->Skill(prhash(u8"TBAScript")), targ, line);
      dest = room->PickObject(targ, LOC_NINJA | LOC_INTERNAL);
    }
    if (!dest) {
      loger(u8"#{} Error: can't find target in '{}'", body->Skill(prhash(u8"TBAScript")), line);
      delete item;
      return -1;
    }
    if (where == u8"rfinger" || where == u8"1") {
      mask = item->Skill(prhash(u8"Wearable on Right Finger"));
      loc = act_t::WEAR_RFINGER;
    } else if (where == u8"lfinger" || where == u8"2") {
      mask = item->Skill(prhash(u8"Wearable on Left Finger"));
      loc = act_t::WEAR_LFINGER;
    } else if (where == u8"neck1" || where == u8"3") {
      mask = item->Skill(prhash(u8"Wearable on Neck"));
      mask |= item->Skill(prhash(u8"Wearable on Collar"));
      loc = act_t::WEAR_NECK;
    } else if (where == u8"neck2" || where == u8"4") {
      mask = item->Skill(prhash(u8"Wearable on Neck"));
      mask |= item->Skill(prhash(u8"Wearable on Collar"));
      loc = act_t::WEAR_COLLAR;
    } else if (where == u8"body" || where == u8"5") {
      mask = item->Skill(prhash(u8"Wearable on Chest"));
      mask &= item->Skill(prhash(u8"Wearable on Back")); // Both
      loc = act_t::WEAR_CHEST;
    } else if (where == u8"head" || where == u8"6") {
      mask = item->Skill(prhash(u8"Wearable on Head"));
      loc = act_t::WEAR_HEAD;
    } else if (where == u8"legs" || where == u8"7") {
      mask = item->Skill(prhash(u8"Wearable on Left Leg"));
      mask |= item->Skill(prhash(u8"Wearable on Right Leg"));
      loc = act_t::WEAR_LLEG;
    } else if (where == u8"feet" || where == u8"8") {
      mask = item->Skill(prhash(u8"Wearable on Left Foot"));
      mask |= item->Skill(prhash(u8"Wearable on Right Foot"));
      loc = act_t::WEAR_LFOOT;
    } else if (where == u8"hands" || where == u8"9") {
      mask = item->Skill(prhash(u8"Wearable on Left Hand"));
      mask |= item->Skill(prhash(u8"Wearable on Right Hand"));
      loc = act_t::WEAR_LHAND;
    } else if (where == u8"arms" || where == u8"10") {
      mask = item->Skill(prhash(u8"Wearable on Left Arm"));
      mask |= item->Skill(prhash(u8"Wearable on Right Arm"));
      loc = act_t::WEAR_LARM;
    } else if (where == u8"shield" || where == u8"11") {
      mask = item->Skill(prhash(u8"Wearable on Shield"));
      loc = act_t::WEAR_SHIELD;
    } else if (where == u8"about" || where == u8"12") {
      mask = item->Skill(prhash(u8"Wearable on Left Shoulder"));
      mask &= item->Skill(prhash(u8"Wearable on Right Shoulder")); // Both
      loc = act_t::WEAR_LSHOULDER;
    } else if (where == u8"waist" || where == u8"13") {
      mask = item->Skill(prhash(u8"Wearable on Waist"));
      loc = act_t::WEAR_WAIST;
    } else if (where == u8"rwrist" || where == u8"14") {
      mask = item->Skill(prhash(u8"Wearable on Right Wrist"));
      loc = act_t::WEAR_RWRIST;
    } else if (where == u8"lwrist" || where == u8"15") {
      mask = item->Skill(prhash(u8"Wearable on Left Wrist"));
      loc = act_t::WEAR_LWRIST;
    } else if (where == u8"wield" || where == u8"16") {
      loc = act_t::WIELD;
    } else if (where == u8"light" || where == u8"0") {
      loc = act_t::HOLD;
    } else if (where == u8"hold" || where == u8"17") {
      loc = act_t::HOLD;
    } else if (where == u8"inv" || where == u8"18") {
    } else if (params > 2) {
      loger(u8"#{} Error: Unsupported dest '{}'", body->Skill(prhash(u8"TBAScript")), line);
      delete item;
      return -1;
    }
    item->SetParent(dest);
    if (loc != act_t::NONE && loc != act_t::HOLD && loc != act_t::WIELD) { // Wear it
      if (!dest->Wear(item, mask))
        dest->StashOrDrop(item);
    } else if (loc == act_t::WIELD) { // Wield it
      if (dest->ActTarg(act_t::WIELD) == nullptr) {
        dest->AddAct(act_t::WIELD, item);
      } else {
        dest->StashOrDrop(item);
      }
    } else if (loc == act_t::HOLD) { // Hold it
      if (dest->ActTarg(act_t::HOLD) == nullptr ||
          dest->ActTarg(act_t::HOLD) == dest->ActTarg(act_t::WIELD) ||
          dest->ActTarg(act_t::HOLD) == dest->ActTarg(act_t::WEAR_SHIELD)) {
        dest->AddAct(act_t::HOLD, item);
      } else {
        dest->StashOrDrop(item);
      }
    } else if (dest != room) { // Have it
      dest->StashOrDrop(item);
    }
  } else if (process(line, u8"dg_cast '")) {
    auto splen = line.find_first_of(u8"'");
    if (splen != std::u8string::npos) {
      auto spell = tba_spellconvert(line.substr(0, splen));
      // logeb(u8"Cast[Acid]: {}", spell);
      // logey(u8"Cast[TBA]: {}", line.substr(splen));
      ovars[u8"self"]->SetSkill(spell + u8" Spell", 5);
      std::u8string cline = u8"shout " + spell;
      if (splen + 1 < line.length()) {
        line = line.substr(splen + 1);
        skipspace(line);
        Object* targ = decode_object(line);
        if (targ) {
          ovars[u8"self"]->AddAct(act_t::POINT, targ);
        }
      }
      cline += u8";cast " + spell + u8";point";
      handle_command(ovars[u8"self"], cline);
    } else {
      loger(u8"Error: Bad casting command: '{}'", line);
    }
  } else if (line.starts_with(u8"case ")) {
    // Ignore these, as we only hit them here when when running over them
  } else if (!!line.starts_with(u8"default")) {
    // Ignore these, as we only hit them here when when running over them
  } else if (line.starts_with(u8"end")) {
    // Ignore these, as we only hit them here when we're running inside if
  } else if (line.starts_with(u8"nop ")) {
    // Ignore these, as the varsub should have done all that's needed
  } else if (line.starts_with(u8"done")) {
    // Means we should be within a while(), pop up a level.
    if (spos_s.size() < 2) {
      loger(
          u8"#{} Error: Not in while/switch, but '{}'\n", body->Skill(prhash(u8"TBAScript")), line);
      return -1;
    }
    spos_s.pop_back();
  } else if (!!line.starts_with(u8"return ")) {
    int retval = TBAEval(line.substr(7));
    if (retval == 0) {
      status = 1; // Set special state
    }
  } else if (line.starts_with(u8"halt")) {
    return -1;
  }

  // Player commands with extra m prefix in TBA, but otherwise working the same in Acid
  else if (line.starts_with(u8"mkill ") || line.starts_with(u8"mfollow ")) {
    size_t stuff = line.find_first_of(u8" ");
    if (stuff != std::u8string::npos) {
      stuff = line.find_first_not_of(u8" \t\r\n", stuff);
    }
    if (stuff != std::u8string::npos) {
      handle_command(ovars[u8"self"], line.substr(1));
    } else {
      loger(u8"#{} Error: Told just '{}'", body->Skill(prhash(u8"TBAScript")), line);
      return -1;
    }
  }

  // Player commands different between Acid and TBA, requiring arguments
  else if (line.starts_with(u8"give ")) {
    size_t start = line.find_first_not_of(u8" \t\r\n", 5);
    if (start != std::u8string::npos) {
      size_t end = line.find_first_of(u8" \t\r\n", start);
      if (end != std::u8string::npos) {
        handle_command(ovars[u8"self"], fmt::format(u8"hold {}", line.substr(start, end - start)));
      } else {
        handle_command(ovars[u8"self"], fmt::format(u8"hold {}", line.substr(start)));
      }
      start = line.find_first_not_of(u8" \t\r\n", end);
      if (start != std::u8string::npos) {
        end = line.find_first_of(u8" \t\r\n", start);
        if (end != std::u8string::npos) {
          handle_command(
              ovars[u8"self"], fmt::format(u8"offer {}", line.substr(start, end - start)));
        } else {
          handle_command(ovars[u8"self"], fmt::format(u8"offer {}", line.substr(start)));
        }
      } else {
        loger(u8"#{} Error: Told just '{}'", body->Skill(prhash(u8"TBAScript")), line);
        return -1;
      }
    } else {
      loger(u8"#{} Error: Told just '{}'", body->Skill(prhash(u8"TBAScript")), line);
      return -1;
    }
  }

  // Player commands Acid shares with TBA, requiring arguments
  else if (
      com == COM_SAY || com == COM_SHOUT || com == COM_EMOTE || com == COM_LOCK ||
      com == COM_UNLOCK || com == COM_OPEN || com == COM_CLOSE || com == COM_GET ||
      com == COM_DROP || com == COM_WEAR || com == COM_WIELD || com == COM_FOLLOW ||
      com == COM_KILL) {
    // if (body->Skill(prhash(u8"TBAScript")) >= 5034503 && body->Skill(prhash(u8"TBAScript")) <=
    // 5034507)
    //  logem(
    //      u8"[#{}] Running command: '{}'\n",
    //      body->Skill(prhash(u8"TBAScript")),
    //      line);

    size_t stuff = line.find_first_of(u8" ");
    if (stuff != std::u8string::npos) {
      stuff = line.find_first_not_of(u8" \t\r\n", stuff);
    }
    if (stuff != std::u8string::npos) {
      handle_command(ovars[u8"self"], line);
    } else {
      loger(u8"#{} Error: Told just '{}'", body->Skill(prhash(u8"TBAScript")), line);
      return -1;
    }
  }

  // Player commands Acid shares with TBA, not requiring arguments
  else if (
      com == COM_NORTH || com == COM_SOUTH || com == COM_EAST || com == COM_WEST || com == COM_UP ||
      com == COM_DOWN || com == COM_SLEEP || com == COM_REST || com == COM_WAKE ||
      com == COM_STAND || com == COM_SIT || com == COM_LIE || com == COM_LOOK || com == COM_FLEE ||
      com > COM_LAST_STANDARD) {
    handle_command(ovars[u8"self"], line);
  }

  // Trigger-Supported (only) commands (not shared with real acid commands).
  else if (com == COM_NONE && handle_command(ovars[u8"self"], line) != 1) {
    // Do Nothing, as handle_command already did it.
  }

  else {
    loger(u8"#{} Error: Gibberish script line '{}'", body->Skill(prhash(u8"TBAScript")), line);
    return -1;
  }
  return 0;
}

bool Mind::TBAVarSub(std::u8string& edit) const {
  size_t cur = edit.find('%');
  size_t end;
  while (cur != std::u8string::npos) {
    std::u8string_view line(edit);
    end = line.find_first_of(u8"%. \t", cur + 1);
    if (end == std::u8string::npos)
      end = line.length();
    if (0
        //	|| body->Skill(prhash(u8"TBAScript")) == 5000099
        //	|| line.contains(u8"eval loc ")
        //	|| line.contains(u8"set first ")
        //	|| line.contains(u8"exclaim")
        //	|| line.contains(u8"speech")
    ) {
      logeg(u8"#{} Debug: '{}'", body->Skill(prhash(u8"TBAScript")), line);
    }
    std::u8string_view vname = line.substr(cur + 1, end - cur - 1);
    Object* obj = nullptr;
    std::u8string val = u8"";
    int is_obj = 0;
    if (ovars.count(std::u8string(vname)) > 0) {
      obj = ovars.at(std::u8string(vname));
      is_obj = 1;
    } else if (svars.count(std::u8string(vname)) > 0) {
      val = svars.at(std::u8string(vname));
    } else if (line.substr(cur).starts_with(u8"%time.hour%")) {
      Object* world = body->World();
      if (world->Skill(prhash(u8"Day Time")) && world->Skill(prhash(u8"Day Length"))) {
        int hour = world->Skill(prhash(u8"Day Time"));
        hour *= 24;
        hour /= world->Skill(prhash(u8"Day Length"));
        val = itos(hour);
      }
      end = line.find_first_of(u8"% \t", cur + 1); // Done.  Replace All.
    } else if (line.substr(cur).starts_with(u8"%random.char%")) {
      DArr64<Object*> others;
      if (ovars.at(u8"self")->HasSkill(prhash(u8"TBARoom"))) {
        others = ovars.at(u8"self")->PickObjects(u8"everyone", LOC_INTERNAL);
      } else if (ovars.at(u8"self")->Owner()) {
        others = ovars.at(u8"self")->Owner()->PickObjects(u8"everyone", LOC_NEARBY);
      } else {
        others = ovars.at(u8"self")->PickObjects(u8"everyone", LOC_NEARBY);
      }
      if (others.size() > 0) {
        int num = rand() % others.size();
        auto item = others.begin();
        for (; num > 0; --num) {
          ++item;
        }
        obj = (*item);
        if (0
            //		|| script.find(u8"%damage% %actor% -%actor.level%") !=
            // std::u8string::npos
        ) {
          logeg(u8"#{} Random: '{}'", body->Skill(prhash(u8"TBAScript")), obj->Noun());
        }
      } else {
        obj = nullptr;
      }
      is_obj = 1;
      end = line.find_first_of(u8"% \t", cur + 1); // Done.  Replace All.
    } else if (line.substr(cur).starts_with(u8"%random.dir%")) {
      Object* room = ovars.at(u8"self");
      while (room && room->Skill(prhash(u8"TBARoom")) == 0)
        room = room->Parent();
      if (room) {
        std::set<Object*> options;
        options.insert(room->PickObject(u8"north", LOC_INTERNAL));
        options.insert(room->PickObject(u8"south", LOC_INTERNAL));
        options.insert(room->PickObject(u8"east", LOC_INTERNAL));
        options.insert(room->PickObject(u8"west", LOC_INTERNAL));
        options.insert(room->PickObject(u8"up", LOC_INTERNAL));
        options.insert(room->PickObject(u8"down", LOC_INTERNAL));
        options.erase(nullptr);
        if (options.size() > 0) {
          int num = rand() % options.size();
          std::set<Object*>::iterator item = options.begin();
          for (; num > 0; --num) {
            ++item;
          }
          val = (*item)->ShortDesc();
        }
      }
      end = line.find_first_of(u8"% \t", cur + 1); // Done.  Replace All.
    } else if (line.substr(cur).starts_with(u8"%random.")) {
      if (isdigit(line[cur + 8])) {
        size_t vend = line.find_first_not_of(u8"0123456789", cur + 8);
        if (vend != std::u8string::npos && line[vend] == '%') {
          int div = getnum(line.substr(cur + 8));
          if (div > 0) {
            val = itos((rand() % div) + 1);
          } else {
            loger(
                u8"#{} Error: Division by zero in '{}'\n",
                body->Skill(prhash(u8"TBAScript")),
                line);
            return false;
          }
        }
      }
      end = line.find_first_of(u8"% \t", cur + 1); // Done.  Replace All.
    } else { // Undefined base var
      end = line.find_first_of(u8"% \t", cur + 1); // Done.  Replace All.
    }
    while (line[end] == '.') {
      size_t start = end + 1;
      end = line.find_first_of(u8"%. \t(", start);
      if (end != std::u8string::npos && line[end] == '(') {
        int paren_depth = 0;
        do {
          if (line[end] == '(')
            ++paren_depth;
          else if (line[end] == ')')
            --paren_depth;
          if (paren_depth > 0)
            end = line.find_first_of(u8"()", end + 1);
        } while (end != std::u8string::npos && paren_depth > 0);
        if (end != std::u8string::npos)
          end = line.find_first_of(u8"%", end + 1);
      }
      if (end == std::u8string::npos)
        end = line.length();
      std::u8string_view field = line.substr(start, end - start);
      if (is_obj) {
        if (field == u8"id") {
          // obj is already right
        } else if (field == u8"vnum") {
          int vnum = 0;
          if (obj) {
            vnum = obj->Skill(prhash(u8"TBAMOB"));
            if (vnum < 1)
              vnum = obj->Skill(prhash(u8"TBAObject"));
            if (vnum < 1)
              vnum = obj->Skill(prhash(u8"TBARoom"));
            if (vnum > 0)
              vnum %= 1000000; // Convert from Acid number
          }
          val = itos(vnum);
          obj = nullptr;
          is_obj = 0;
        } else if (field == u8"gold") {
          int gold = 0;
          if (obj) {
            auto pay = obj->PickObjects(u8"all a gold piece", LOC_INTERNAL);
            for (auto coin : pay) {
              gold += coin->Quantity();
            }
          }
          val = itos(gold);
          obj = nullptr;
          is_obj = 0;
        } else if (field == u8"type") {
          val = u8"OTHER";
          if (obj) {
            if (obj->HasSkill(prhash(u8"Container")))
              val = u8"CONTAINER";
            else if (obj->HasSkill(prhash(u8"Liquid Source")))
              val = u8"FOUNTAIN";
            else if (obj->HasSkill(prhash(u8"Liquid Container")))
              val = u8"LIQUID CONTAINER";
            else if (obj->HasSkill(prhash(u8"Ingestible")) <= 0)
              val = u8"FOOD";
            else if (obj->Value() <= 0)
              val = u8"TRASH";
            // FIXME: More Types!
          }
          obj = nullptr;
          is_obj = 0;
        } else if (field == u8"cost_per_day") {
          val = u8"0";
          obj = nullptr;
          is_obj = 0;
        } else if (field == u8"cost") {
          val = u8"";
          if (obj)
            val = itos(obj->Value());
          obj = nullptr;
          is_obj = 0;
        } else if (field == u8"count") {
          val = u8"";
          if (obj)
            val = itos(obj->Quantity());
          obj = nullptr;
          is_obj = 0;
        } else if (field == u8"weight") {
          val = u8"";
          if (obj)
            val = itos(obj->Weight());
          obj = nullptr;
          is_obj = 0;
        } else if (field == u8"sex") {
          val = u8"";
          if (obj) {
            if (obj->Gender() == gender_t::MALE)
              val = u8"male";
            else if (obj->Gender() == gender_t::FEMALE)
              val = u8"female";
            else if (obj->Gender() == gender_t::NEITHER)
              val = u8"other";
            else
              val = u8"none";
          }
          obj = nullptr;
          is_obj = 0;
        } else if (field == u8"race") {
          val = u8"";
          if (obj && obj->IsAnimate()) {
            val = u8"human"; // FIXME: Implement Race!
          }
          obj = nullptr;
          is_obj = 0;
        } else if (field == u8"level") {
          val = u8"";
          if (obj)
            val = itos(obj->TotalExp() / 10 + 1);
          obj = nullptr;
          is_obj = 0;
        } else if (field == u8"name") {
          val = u8"";
          if (obj)
            val = obj->Noun();
          obj = nullptr;
          is_obj = 0;
        } else if (field == u8"shortdesc") {
          val = u8"";
          if (obj)
            val = obj->ShortDesc();
          obj = nullptr;
          is_obj = 0;
        } else if (field == u8"alias") {
          val = u8"";
          if (obj)
            val = obj->ShortDesc();
          obj = nullptr;
          is_obj = 0;
        } else if (field == u8"heshe") {
          val = u8"";
          if (obj)
            val = obj->Pron();
          obj = nullptr;
          is_obj = 0;
        } else if (field == u8"hisher") {
          val = u8"";
          if (obj)
            val = obj->Poss();
          obj = nullptr;
          is_obj = 0;
        } else if (field == u8"himher") {
          val = u8"";
          if (obj)
            val = obj->Obje();
          obj = nullptr;
          is_obj = 0;
        } else if (field == u8"maxhitp") {
          val = itos(1000); // Everybody has 1000 HP.
          obj = nullptr;
          is_obj = 0;
        } else if (field == u8"hitp") {
          val = u8"";
          if (obj)
            val = itos(1000 - 50 * (obj->Phys() + obj->Stun()));
          obj = nullptr;
          is_obj = 0;
        } else if (field == u8"align") {
          val = u8"";
          if (obj) {
            int align = 0;
            align = obj->Skill(prhash(u8"Honor"));
            if (align == 0)
              align = -(obj->Skill(prhash(u8"Dishonor")));
            val = itos(align);
          }
          obj = nullptr;
          is_obj = 0;
        } else if (field == u8"is_pc") {
          val = u8"";
          if (obj)
            val = bstr[is_pc(obj)];
          obj = nullptr;
          is_obj = 0;
        } else if (field == u8"is_killer") {
          val = u8"0"; // FIXME: Real value?
          obj = nullptr;
          is_obj = 0;
        } else if (field == u8"is_thief") {
          val = u8"0"; // FIXME: Real value?
          obj = nullptr;
          is_obj = 0;
        } else if (field == u8"con") {
          val = u8"";
          if (obj)
            val = itos(obj->NormAttribute(0) * 3);
          obj = nullptr;
          is_obj = 0;
        } else if (field == u8"dex") {
          val = u8"";
          if (obj)
            val = itos(obj->NormAttribute(1) * 3);
          obj = nullptr;
          is_obj = 0;
        } else if (field == u8"str") {
          val = u8"";
          if (obj)
            val = itos(obj->NormAttribute(2) * 3);
          obj = nullptr;
          is_obj = 0;
        } else if (field == u8"stradd") { // D&D is Dumb
          val = u8"0";
          obj = nullptr;
          is_obj = 0;
        } else if (field == u8"cha") {
          val = u8"";
          if (obj)
            val = itos(obj->NormAttribute(3) * 3);
          obj = nullptr;
          is_obj = 0;
        } else if (field == u8"int") {
          val = u8"";
          if (obj)
            val = itos(obj->NormAttribute(4) * 3);
          obj = nullptr;
          is_obj = 0;
        } else if (field == u8"wis") {
          val = u8"";
          if (obj)
            val = itos(obj->NormAttribute(5) * 3);
          obj = nullptr;
          is_obj = 0;
        } else if (field == u8"pos") {
          val = u8"";
          if (obj) {
            if (obj->IsAct(act_t::SLEEP))
              val = u8"sleeping";
            else if (obj->IsAct(act_t::REST))
              val = u8"resting";
            else if (obj->IsAct(act_t::FIGHT))
              val = u8"fighting";
            else if (obj->Position() == pos_t::LIE)
              val = u8"resting";
            else if (obj->Position() == pos_t::SIT)
              val = u8"sitting";
            else if (obj->Position() == pos_t::STAND)
              val = u8"standing";
          }
          obj = nullptr;
          is_obj = 0;
        } else if (field == u8"title") {
          val = u8"";
          obj = nullptr;
          is_obj = 0;
        } else if (field == u8"val0") { // FIXME: Implement?
          val = u8"0";
          obj = nullptr;
          is_obj = 0;
        } else if (field == u8"val1") { // FIXME: Implement?
          val = u8"0";
          obj = nullptr;
          is_obj = 0;
        } else if (field == u8"val2") { // FIXME: Implement?
          val = u8"0";
          obj = nullptr;
          is_obj = 0;
        } else if (field == u8"val3") { // FIXME: Implement?
          val = u8"0";
          obj = nullptr;
          is_obj = 0;
        } else if (field == u8"timer") {
          val = u8"";
          if (obj)
            val = itos(obj->Skill(prhash(u8"Temporary"))); // FIXME: More Kinds?
          obj = nullptr;
          is_obj = 0;
        } else if (field == u8"move") {
          val = u8"";
          if (obj)
            val = itos(10 - obj->Stun());
          obj = nullptr;
          is_obj = 0;
        } else if (field == u8"maxmove") {
          val = u8"";
          if (obj)
            val = u8"10";
          obj = nullptr;
          is_obj = 0;
        } else if (field == u8"mana") {
          val = u8"";
          if (obj) {
            if (obj->HasSkill(prhash(u8"Faith"))) {
              val = itos(obj->Skill(prhash(u8"Faith Remaining")));
            } else {
              val = itos(10 - obj->Stun());
            }
          }
          obj = nullptr;
          is_obj = 0;
        } else if (field == u8"maxmana") {
          val = u8"";
          if (obj) {
            if (obj->HasSkill(prhash(u8"Faith"))) {
              val = itos(obj->Skill(prhash(u8"Faith")) * obj->Skill(prhash(u8"Faith")));
            } else {
              val = u8"10";
            }
          }
          obj = nullptr;
          is_obj = 0;
        } else if (field == u8"saving_para") {
          val = u8"";
          if (obj)
            val = u8"0";
          obj = nullptr;
          is_obj = 0;
        } else if (field == u8"saving_rod") {
          val = u8"";
          if (obj)
            val = u8"0";
          obj = nullptr;
          is_obj = 0;
        } else if (field == u8"saving_petri") {
          val = u8"";
          if (obj)
            val = u8"0";
          obj = nullptr;
          is_obj = 0;
        } else if (field == u8"saving_breath") {
          val = u8"";
          if (obj)
            val = u8"0";
          obj = nullptr;
          is_obj = 0;
        } else if (field == u8"saving_spell") {
          val = u8"";
          if (obj)
            val = u8"0";
          obj = nullptr;
          is_obj = 0;
        } else if (field == u8"prac") {
          val = u8"";
          if (obj)
            val = u8"0";
          obj = nullptr;
          is_obj = 0;
        } else if (field == u8"questpoints") {
          val = u8"";
          if (obj)
            val = u8"0";
          obj = nullptr;
          is_obj = 0;
        } else if (field == u8"exp") {
          val = u8"";
          if (obj)
            val = itos(obj->TotalExp());
          obj = nullptr;
          is_obj = 0;
        } else if (field == u8"hunger") {
          val = u8"";
          if (obj)
            val = itos(obj->Skill(prhash(u8"Hungry"))); // FIXME: Convert
          obj = nullptr;
          is_obj = 0;
        } else if (field == u8"thirst") {
          val = u8"";
          if (obj)
            val = itos(obj->Skill(prhash(u8"Thirsty"))); // FIXME: Convert
          obj = nullptr;
          is_obj = 0;
        } else if (field == u8"drunk") {
          val = u8"";
          if (obj)
            val = u8"0"; // FIXME: Query Drunkenness Here
          obj = nullptr;
          is_obj = 0;
        } else if (field == u8"class") {
          val = u8"";
          if (obj) {
            if (obj->HasSkill(prhash(u8"Spellcasting")) || obj->HasSkill(prhash(u8"Spellcraft"))) {
              val = u8"magic user";
            } else if (
                obj->HasSkill(prhash(u8"Perception")) || obj->HasSkill(prhash(u8"Stealth"))) {
              val = u8"thief";
            } else if (obj->HasSkill(prhash(u8"Faith"))) {
              val = u8"priest";
            } else {
              val = u8"warrior";
            }
          }
          obj = nullptr;
          is_obj = 0;
        } else if (field == u8"canbeseen") {
          val = u8"";
          if (obj)
            val =
                bstr[!(obj->HasSkill(prhash(u8"Invisible")) || obj->HasSkill(prhash(u8"Hidden")))];
          obj = nullptr;
          is_obj = 0;
        } else if (field == u8"affect") {
          val = u8""; // FIXME: Translate & List Spell Effects?
          obj = nullptr;
          is_obj = 0;
        } else if (field == u8"fighting") {
          if (obj)
            obj = obj->ActTarg(act_t::FIGHT);
        } else if (field == u8"worn_by") {
          if (obj) {
            Object* owner = obj->Owner();
            if (owner && owner->Wearing(obj))
              obj = owner;
            else
              obj = nullptr;
          } else
            obj = nullptr;
        } else if (field == u8"room") {
          while (obj && obj->Skill(prhash(u8"TBARoom")) == 0)
            obj = obj->Parent();
        } else if (field == u8"people") {
          if (obj)
            obj = obj->PickObject(u8"someone", LOC_INTERNAL);
        } else if (field == u8"contents") {
          if (obj)
            obj = obj->PickObject(u8"something", LOC_INTERNAL);
        } else if (field == u8"inventory") {
          if (obj)
            obj = obj->PickObject(u8"something", LOC_INTERNAL | LOC_NOTWORN);
        } else if ((field == u8"eq(*)") || (field == u8"eq")) {
          if (obj)
            obj = obj->PickObject(u8"something", LOC_INTERNAL | LOC_NOTUNWORN);
        } else if (
            (field == u8"eq(light)") || (field == u8"eq(hold)") || (field == u8"eq(0)") ||
            (field == u8"eq(17)")) {
          if (obj)
            obj = obj->ActTarg(act_t::HOLD);
        } else if ((field == u8"eq(wield)") || (field == u8"eq(16)")) {
          if (obj)
            obj = obj->ActTarg(act_t::WIELD);
        } else if ((field == u8"eq(rfinger)") || (field == u8"eq(1)")) {
          if (obj)
            obj = obj->ActTarg(act_t::WEAR_RFINGER);
        } else if ((field == u8"eq(lfinger)") || (field == u8"eq(2)")) {
          if (obj)
            obj = obj->ActTarg(act_t::WEAR_LFINGER);
        } else if ((field == u8"eq(neck1)") || (field == u8"eq(3)")) {
          if (obj)
            obj = obj->ActTarg(act_t::WEAR_NECK);
        } else if ((field == u8"eq(neck2)") || (field == u8"eq(4)")) {
          if (obj)
            obj = obj->ActTarg(act_t::WEAR_COLLAR);
        } else if ((field == u8"eq(body)") || (field == u8"eq(5)")) {
          if (obj)
            obj = obj->ActTarg(act_t::WEAR_CHEST);
        } else if ((field == u8"eq(head)") || (field == u8"eq(6)")) {
          if (obj)
            obj = obj->ActTarg(act_t::WEAR_HEAD);
        } else if ((field == u8"eq(legs)") || (field == u8"eq(7)")) {
          if (obj)
            obj = obj->ActTarg(act_t::WEAR_LLEG);
        } else if ((field == u8"eq(feet)") || (field == u8"eq(8)")) {
          if (obj)
            obj = obj->ActTarg(act_t::WEAR_LFOOT);
        } else if ((field == u8"eq(hands)") || (field == u8"eq(9)")) {
          if (obj)
            obj = obj->ActTarg(act_t::WEAR_LHAND);
        } else if ((field == u8"eq(arms)") || (field == u8"eq(10)")) {
          if (obj)
            obj = obj->ActTarg(act_t::WEAR_LARM);
        } else if ((field == u8"eq(shield)") || (field == u8"eq(11)")) {
          if (obj)
            obj = obj->ActTarg(act_t::WEAR_SHIELD);
        } else if ((field == u8"eq(about)") || (field == u8"eq(12)")) {
          if (obj)
            obj = obj->ActTarg(act_t::WEAR_LSHOULDER);
        } else if ((field == u8"eq(waits)") || (field == u8"eq(13)")) {
          if (obj)
            obj = obj->ActTarg(act_t::WEAR_WAIST);
        } else if ((field == u8"eq(rwrist)") || (field == u8"eq(14)")) {
          if (obj)
            obj = obj->ActTarg(act_t::WEAR_RWRIST);
        } else if ((field == u8"eq(lwrist)") || (field == u8"eq(15)")) {
          if (obj)
            obj = obj->ActTarg(act_t::WEAR_LWRIST);
        } else if (field == u8"carried_by") {
          if (obj)
            obj = obj->Owner();
        } else if (field == u8"next_in_list") {
          if (obj) {
            Object* par = obj->Owner();
            if (!par)
              par = obj->Parent();
            if (par) {
              auto stf = par->PickObjects(u8"everything", LOC_INTERNAL);
              auto item = stf.begin();
              while (item != stf.end() && (*item) != obj)
                ++item;
              if (item != stf.end())
                ++item;
              if (item != stf.end())
                obj = (*item);
              else
                obj = nullptr;
            } else
              obj = nullptr;
          }
        } else if (field == u8"next_in_room") {
          if (obj) {
            Object* room = obj->Parent();
            while (room && room->Skill(prhash(u8"TBARoom")) == 0)
              room = room->Parent();
            if (room) {
              auto stf = room->PickObjects(u8"everyone", LOC_INTERNAL);
              auto item = stf.begin();
              while (item != stf.end() && (*item) != obj)
                ++item;
              if (item != stf.end())
                ++item;
              if (item != stf.end())
                obj = (*item);
              else
                obj = nullptr;
            } else
              obj = nullptr;
          }
        } else if (field == u8"master") {
          if (obj)
            obj = obj->ActTarg(act_t::FOLLOW); // FIXME: More Kinds?
        } else if (field == u8"follower") {
          if (obj) {
            auto touch = obj->Touching();
            bool found = false;
            for (auto tent : touch) {
              if (tent->ActTarg(act_t::FOLLOW) == obj) {
                obj = tent;
                found = true;
                break;
              }
            }
            if (!found) {
              obj = nullptr;
            }
          } else
            obj = nullptr;
        } else if (field.starts_with(u8"skill(")) {
          val = u8"";
          if (obj) {
            size_t num = field.find_first_of(u8") .%", 6);
            auto skl = get_skill(field.substr(6, num - 6));
            val = itos(obj->Skill(skl));
          }
          obj = nullptr;
          is_obj = 0;
        } else if (field.starts_with(u8"varexists(")) {
          val = u8"";
          if (obj) {
            size_t num = field.find_first_of(u8") .%", 10);
            std::u8string_view var = field.substr(10, num - 10); // FIXME: Variables!
            val = bstr[obj->HasSkill(crc32c(fmt::format(u8"TBA:{}", var)))];
          }
          obj = nullptr;
          is_obj = 0;
        } else if (process(field, u8"has_item(")) {
          int vnum = getnum(field);
          val = u8"";
          if (obj && vnum != 0 && field[0] == ')') {
            vnum += 1000000;
            auto pos = obj->PickObjects(u8"all", LOC_INTERNAL);
            for (auto item : pos) {
              if (vnum == item->Skill(prhash(u8"TBAObject"))) {
                val = u8"1";
                break;
              }
            }
          }
          obj = nullptr;
          is_obj = 0;
        } else if (!!field.starts_with(u8"affect(")) {
          size_t len = field.find_first_not_of(u8"abcdefghijklmnopqrstuvwxyz-", 7);
          auto spell = tba_spellconvert(field.substr(7, len - 7));
          // logeb(
          //    CBLU u8"Interpreting '{}' as 'spell = {}'\n",
          //    field,
          //    spell);
          val = u8"";
          if (obj) {
            val = itos(obj->Power(crc32c(spell)));
          }
          obj = nullptr;
          is_obj = 0;
        } else if (field.starts_with(u8"vnum(")) {
          val = u8"0"; // Default - in case it doesn't have a vnum
          if (obj) {
            int vnum = obj->Skill(prhash(u8"TBAMOB"));
            if (vnum < 1)
              vnum = obj->Skill(prhash(u8"TBAObject"));
            if (vnum < 1)
              vnum = obj->Skill(prhash(u8"TBARoom"));
            if (vnum > 0) {
              vnum %= 1000000; // Convert from Acid number
              int off = field.find_first_of(u8")", 5);
              std::u8string_view query = field.substr(5, off - 5);
              int qnum = TBAEval(std::u8string(query));
              val = bstr[(vnum == qnum)];
            }
          }
          obj = nullptr;
          is_obj = 0;
        } else if (field == u8"pos(sleeping)") {
          if (obj) {
            obj->SetPosition(pos_t::LIE);
            obj->StopAct(act_t::REST);
            obj->AddAct(act_t::SLEEP);
          }
          obj = nullptr;
          val = u8"";
          is_obj = 0;
        } else if (field == u8"pos(resting)") {
          if (obj) {
            obj->StopAct(act_t::SLEEP);
            obj->SetPosition(pos_t::SIT);
            obj->AddAct(act_t::REST);
          }
          obj = nullptr;
          val = u8"";
          is_obj = 0;
        } else if (field == u8"pos(sitting)") {
          if (obj) {
            obj->StopAct(act_t::SLEEP);
            obj->StopAct(act_t::REST);
            obj->SetPosition(pos_t::SIT);
          }
          obj = nullptr;
          val = u8"";
          is_obj = 0;
        } else if (field == u8"pos(fighting)") {
          if (obj) {
            obj->StopAct(act_t::SLEEP);
            obj->StopAct(act_t::REST);
            obj->SetPosition(pos_t::STAND);
          }
          obj = nullptr;
          val = u8"";
          is_obj = 0;
        } else if (field == u8"pos(standing)") {
          if (obj) {
            obj->StopAct(act_t::SLEEP);
            obj->StopAct(act_t::REST);
            obj->SetPosition(pos_t::STAND);
          }
          obj = nullptr;
          val = u8"";
          is_obj = 0;
          // Is no general fighting state, must fight someone!
        } else {
          loger(
              u8"#{} Error: Bad sub-obj '{}' in '{}'\n",
              body->Skill(prhash(u8"TBAScript")),
              field,
              line);
          return false;
        }
      } else {
        if (field == u8"mudcommand") {
          // val is already right
        } else if (field == u8"car") {
          size_t apos = val.find_first_of(u8" \t\n\r");
          if (apos != std::u8string::npos) {
            val = val.substr(0, apos);
          }
        } else if (field == u8"cdr") {
          size_t apos = val.find_first_of(u8" \t\n\r");
          if (apos != std::u8string::npos) {
            apos = val.find_first_not_of(u8" \t\n\r", apos);
            if (apos != std::u8string::npos)
              val = val.substr(apos);
            else
              val = u8"";
          } else
            val = u8"";
        } else if (field == u8"trim") {
          trim_string(val);
        } else {
          loger(
              u8"#{} Error: Bad sub-str '{}' in '{}'\n",
              body->Skill(prhash(u8"TBAScript")),
              field,
              line);
          return false;
        }
      }
    }
    if (end == std::u8string::npos)
      end = line.length();
    else if (line[end] == '%')
      ++end;
    if (is_obj) {
      edit.replace(cur, end - cur, fmt::format(u8"obj:{}", reinterpret_cast<void*>(obj)));
    } else { // std::u8string OR u8""
      edit.replace(cur, end - cur, val);
    }
    line = edit;
    cur = line.find('%', cur + 1);
  }
  if (0
      //	|| body->Skill(prhash(u8"TBAScript")) == 5000099
      //	|| line.contains(u8"eval loc ")
      //	|| line.contains(u8"set first ")
      //	|| line.contains(u8"exclaim")
      //	|| line.contains(u8"speech")
  ) {
    logeg(u8"#{} Debug: '{}' <-Final", body->Skill(prhash(u8"TBAScript")), edit);
  }
  return true;
}

int Mind::TBACanWanderTo(Object* dest) const {
  if (dest->Skill(prhash(u8"TBAZone")) == 999999) { // NO_MOBS TBA Zone
    return 0; // Don't Enter NO_MOBS Zone!
  } else if (body->Skill(prhash(u8"TBAAction")) & 64) { // STAY_ZONE TBA MOBs
    if (dest->Skill(prhash(u8"TBAZone")) != body->Parent()->Skill(prhash(u8"TBAZone"))) {
      return 0; // Don't Leave Zone!
    }
  } else if (body->Skill(prhash(u8"Swimming")) == 0) { // Can't Swim?
    if (dest->Skill(prhash(u8"WaterDepth")) == 1) {
      return 0; // Can't swim!
    }
  } else if (!(body->Skill(prhash(u8"TBAAffection")) & 64)) { // Can't Waterwalk?
    if (dest->Skill(prhash(u8"WaterDepth")) >= 1) { // Need boat!
      return 0; // FIXME: Have boat?
    }
  }
  return 1;
}

void Mind::SetTBATrigger(
    Object* tr,
    Object* tripper,
    Object* targ,
    const std::u8string_view& text) {
  if ((!tr) || (!(tr->Parent())))
    return;

  type = mind_t::TBATRIG;
  if (cvars.size() < 1) {
    cvars[u8"damage"] = u8"wdamage";
    cvars[u8"echo"] = u8"mecho";
    cvars[u8"send"] = u8"send";
    cvars[u8"force"] = u8"force";
    cvars[u8"echoaround"] = u8"echoaround";
    cvars[u8"teleport"] = u8"transport";
    cvars[u8"zoneecho"] = u8"zoneecho";
    cvars[u8"asound"] = u8"asound";
    cvars[u8"door"] = u8"door";
    cvars[u8"load"] = u8"load";
    cvars[u8"purge"] = u8"purge";
    cvars[u8"at"] = u8"at";
    cvars[u8""] = u8"%";
  }
  svars = cvars;
  pers = fileno(stderr);
  spos_s.clear();
  spos_s.push_back(0);
  svars[u8"script"] = fmt::format(u8"{}\n", tr->LongDesc());

  if (tripper)
    ovars[u8"actor"] = tripper;

  int stype = tr->Skill(prhash(u8"TBAScriptType"));
  if ((stype & 0x2000008) == 0x0000008) { //-SPEECH MOB/ROOM Triggers
    svars[u8"speech"] = text;
  }
  if ((stype & 0x4000040) == 0x4000040 // ROOM-ENTER Triggers
      || (stype & 0x1000040) == 0x1000040 // MOB-GREET Triggers
      || stype & 0x0010000) { //*-LEAVE Triggers
    svars[u8"direction"] = text;
  }
  if ((stype & 0x4000080) == 0x4000080) { // ROOM-DROP Triggers
    ovars[u8"object"] = targ;
  }
  if ((stype & 0x1000200) == 0x1000200) { // MOB-RECEIVE Triggers
    ovars[u8"object"] = targ;
  }
  if (stype & 0x0000004) { //-COMMAND Triggers
    size_t part = text.find_first_of(u8" \t\n\r");
    if (part == std::u8string::npos)
      svars[u8"cmd"] = text;
    else {
      svars[u8"cmd"] = text.substr(0, part);
      part = text.find_first_not_of(u8" \t\n\r", part);
      if (part != std::u8string::npos)
        svars[u8"arg"] = text.substr(part);
    }
  }
}

void Mind::SetTBAMob() {
  type = mind_t::TBAMOB;
  pers = fileno(stderr);
}

static std::shared_ptr<Mind> tbamob_mind = std::make_shared<Mind>(mind_t::TBAMOB);
std::shared_ptr<Mind> get_tbamob_mind() {
  return tbamob_mind;
}

std::shared_ptr<Mind>
new_mind(mind_t tp, Object* obj, Object* obj2, Object* obj3, const std::u8string_view& text) {
  std::shared_ptr<Mind> m = std::make_shared<Mind>(tp);
  if (tp == mind_t::TBATRIG && obj) {
    m->SetTBATrigger(obj, obj2, obj3, text);
    obj->Attach(m);
  } else if (obj) {
    obj->Attach(m);
  }
  return m;
}

int new_trigger(int msec, Object* obj, Object* tripper, const std::u8string_view& text) {
  return new_trigger(msec, obj, tripper, nullptr, text);
}

static bool in_new_trigger = false;
int new_trigger(
    int msec,
    Object* obj,
    Object* tripper,
    Object* targ,
    const std::u8string_view& text) {
  if ((!obj) || (!(obj->Parent())))
    return 0;

  int status = 0;
  std::shared_ptr<Mind> m = new_mind(mind_t::TBATRIG, obj, tripper, targ, text);
  if (msec == 0 && !in_new_trigger) { // Triggers can not immediately trigger triggers
    in_new_trigger = true;
    if (!m->Think(1)) {
      status = m->Status();
      if (m->Body()) {
        m->Body()->Detach(m);
      }
    }
    in_new_trigger = false;
  } else {
    m->Suspend(msec);
  }
  return status;
}

// Returns how much NPC/MOB would pay for item, or 0.
size_t Mind::TBAMOBWouldBuyFor(const Object* item) {
  bool wearable = false;
  if (item->HasSkill(prhash(u8"Wearable on Back"))) {
    wearable = true;
  } else if (item->HasSkill(prhash(u8"Wearable on Chest"))) {
    wearable = true;
  } else if (item->HasSkill(prhash(u8"Wearable on Head"))) {
    wearable = true;
  } else if (item->HasSkill(prhash(u8"Wearable on Neck"))) {
    wearable = true;
  } else if (item->HasSkill(prhash(u8"Wearable on Collar"))) {
    wearable = true;
  } else if (item->HasSkill(prhash(u8"Wearable on Waist"))) {
    wearable = true;
  } else if (item->HasSkill(prhash(u8"Wearable on Shield"))) {
    wearable = true;
  } else if (item->HasSkill(prhash(u8"Wearable on Left Arm"))) {
    wearable = true;
  } else if (item->HasSkill(prhash(u8"Wearable on Right Arm"))) {
    wearable = true;
  } else if (item->HasSkill(prhash(u8"Wearable on Left Finger"))) {
    wearable = true;
  } else if (item->HasSkill(prhash(u8"Wearable on Right Finger"))) {
    wearable = true;
  } else if (item->HasSkill(prhash(u8"Wearable on Left Foot"))) {
    wearable = true;
  } else if (item->HasSkill(prhash(u8"Wearable on Right Foot"))) {
    wearable = true;
  } else if (item->HasSkill(prhash(u8"Wearable on Left Hand"))) {
    wearable = true;
  } else if (item->HasSkill(prhash(u8"Wearable on Right Hand"))) {
    wearable = true;
  } else if (item->HasSkill(prhash(u8"Wearable on Left Leg"))) {
    wearable = true;
  } else if (item->HasSkill(prhash(u8"Wearable on Right Leg"))) {
    wearable = true;
  } else if (item->HasSkill(prhash(u8"Wearable on Left Wrist"))) {
    wearable = true;
  } else if (item->HasSkill(prhash(u8"Wearable on Right Wrist"))) {
    wearable = true;
  } else if (item->HasSkill(prhash(u8"Wearable on Left Shoulder"))) {
    wearable = true;
  } else if (item->HasSkill(prhash(u8"Wearable on Right Shoulder"))) {
    wearable = true;
  } else if (item->HasSkill(prhash(u8"Wearable on Left Hip"))) {
    wearable = true;
  } else if (item->HasSkill(prhash(u8"Wearable on Right Hip"))) {
    wearable = true;
  }

  uint32_t skill = prhash(u8"None");

  if (item->Skill(prhash(u8"Money")) == static_cast<int>(item->Value())) { // 1-1 Money
    for (auto skl : Body()->GetSkills()) {
      if (SkillName(skl.first).starts_with(u8"Buy ")) {
        skill = prhash(u8"Money");
        break;
      }
    }
  } else if (wearable && item->NormAttribute(0) > 0) {
    if (Body()->HasSkill(prhash(u8"Buy Armor"))) {
      skill = prhash(u8"Buy Armor");
    }
  } else if (item->Skill(prhash(u8"Vehicle")) == 4) {
    if (Body()->HasSkill(prhash(u8"Buy Boat"))) {
      skill = prhash(u8"Buy Boat");
    }
  } else if (item->HasSkill(prhash(u8"Container"))) {
    if (Body()->HasSkill(prhash(u8"Buy Container"))) {
      skill = prhash(u8"Buy Container");
    }
  } else if (item->HasSkill(prhash(u8"Food")) && (!item->HasSkill(prhash(u8"Drink")))) {
    if (Body()->HasSkill(prhash(u8"Buy Food"))) {
      skill = prhash(u8"Buy Food");
    }
  }
  //      else if(false) {    //FIXME: Implement
  //	if(Body()->HasSkill(prhash(u8"Buy Light"))) {
  //	  skill = prhash(u8"Buy Light");
  //	  }
  //	}
  else if (item->HasSkill(prhash(u8"Liquid Container"))) { // FIXME: Not Potions?
    if (Body()->HasSkill(prhash(u8"Buy Liquid Container"))) {
      skill = prhash(u8"Buy Liquid Container");
    }
  } else if (item->HasSkill(prhash(u8"Liquid Container"))) { // FIXME: Not Bottles?
    if (Body()->HasSkill(prhash(u8"Buy Potion"))) {
      skill = prhash(u8"Buy Potion");
    }
  } else if (item->HasSkill(prhash(u8"Magical Scroll"))) {
    if (Body()->HasSkill(prhash(u8"Buy Scroll"))) {
      skill = prhash(u8"Buy Scroll");
    }
  } else if (item->HasSkill(prhash(u8"Magical Staff"))) {
    if (Body()->HasSkill(prhash(u8"Buy Staff"))) {
      skill = prhash(u8"Buy Staff");
    }
  } else if (item->HasSkill(prhash(u8"Magical Wand"))) {
    if (Body()->HasSkill(prhash(u8"Buy Wand"))) {
      skill = prhash(u8"Buy Wand");
    }
  }
  //      else if(false) {    //FIXME: Implement
  //	if(Body()->HasSkill(prhash(u8"Buy Trash"))) {
  //	  skill = prhash(u8"Buy Trash");
  //	  }
  //	}
  //      else if(false) {    //FIXME: Implement
  //	if(Body()->HasSkill(prhash(u8"Buy Treasure"))) {
  //	  skill = prhash(u8"Buy Treasure");
  //	  }
  //	}
  else if (item->Skill(prhash(u8"WeaponType")) > 0) {
    if (Body()->HasSkill(prhash(u8"Buy Weapon"))) {
      skill = prhash(u8"Buy Weapon");
    }
  } else if (wearable && item->NormAttribute(0) == 0) {
    if (Body()->HasSkill(prhash(u8"Buy Worn"))) {
      skill = prhash(u8"Buy Worn");
    }
  }
  //      else if(false) {  //FIXME: Implement
  //	if(Body()->HasSkill(prhash(u8"Buy Other"))) {
  //	  skill = prhash(u8"Buy Other");
  //	  }
  //	}

  if (skill == prhash(u8"None") && Body()->HasSkill(prhash(u8"Buy All"))) {
    skill = prhash(u8"Buy All");
  }

  if (skill == prhash(u8"Money")) { // 1:1 Money
    return item->Value() * item->Quantity();
  } else if (skill != prhash(u8"None")) {
    return std::max(
        1UL, item->Value() * static_cast<size_t>(Body()->Skill(skill)) * item->Quantity() / 1000);
  }

  return 0; // Not Interested
}
