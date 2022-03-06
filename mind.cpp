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

#include <arpa/telnet.h>

#include <fcntl.h>
#include <unistd.h>

#include <algorithm>
#include <cstdarg>
#include <filesystem>
#include <limits>
#include <random>
#include <string>

#include "cchar8.hpp"
#include "color.hpp"
#include "commands.hpp"
#include "mind.hpp"
#include "net.hpp"
#include "object.hpp"
#include "player.hpp"
#include "properties.hpp"
#include "utils.hpp"

extern int64_t current_time; // From main.cpp

static const std::u8string bstr[2] = {u8"0", u8"1"};

static std::u8string itos(int val) {
  char8_t buf[256];
  sprintf(buf, u8"%d", val);
  return std::u8string(buf);
}

static uint32_t tba_bitvec(const std::u8string& val) {
  uint32_t ret = atoi(val.c_str());
  if (ret == 0) {
    for (size_t idx = 0; idx < val.length(); ++idx) {
      ret |= 1 << ((val[idx] & 31) - 1);
    }
  }
  return ret;
}

static std::u8string tba_spellconvert(const std::u8string& tba) {
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

std::u8string Mind::TBAComp(std::u8string expr) const {
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
    expr = TBAComp(expr.substr(1, cls - 1)) + u8" " + expr.substr(cls + 1);
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
  if (op == std::u8string::npos)
    return expr; // No ops, just val

  int oper = 0; // Positive for 2-char ops, negative for 1-char
  int weak = 0; // Reverse-Precedence!  Hack!!!
  if (!strncmp(expr.c_str() + op, u8"/=", 2)) {
    oper = 1;
  } else if (!strncmp(expr.c_str() + op, u8"==", 2)) {
    oper = 2;
  } else if (!strncmp(expr.c_str() + op, u8"!=", 2)) {
    oper = 3;
  } else if (!strncmp(expr.c_str() + op, u8"<=", 2)) {
    oper = 4;
  } else if (!strncmp(expr.c_str() + op, u8">=", 2)) {
    oper = 5;
  } else if (!strncmp(expr.c_str() + op, u8"&&", 2)) {
    oper = 6;
    weak = 1;
  } else if (!strncmp(expr.c_str() + op, u8"||", 2)) {
    oper = 7;
    weak = 1;
  } else if (!strncmp(expr.c_str() + op, u8"<", 1)) {
    oper = -1;
  } else if (!strncmp(expr.c_str() + op, u8">", 1)) {
    oper = -2;
  } else if (!strncmp(expr.c_str() + op, u8"+", 1)) {
    oper = -3;
  } else if (!strncmp(expr.c_str() + op, u8"-", 1)) {
    oper = -4;
  } else if (!strncmp(expr.c_str() + op, u8"*", 1)) {
    oper = -5;
  } else if (!strncmp(expr.c_str() + op, u8"/", 1)) {
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
    if (oper == 1 && (arg2.find(arg1) != std::u8string::npos)) {
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
      // fprintf(stderr, u8"RES: %s\n", buf);
    }

    if (expr != u8"") {
      expr = comp + u8" " + expr;
      return TBAComp(expr);
    }
    return comp;
  }

  return u8"0";
}

int Mind::TBAEval(std::u8string expr) const {
  std::u8string base = TBAComp(expr);
  trim_string(base);

  if (base.length() == 0)
    return 0; // Null
  if (base.length() == 1 && base[0] == '!')
    return 1; //! Null

  int ret = 0, len = 0;
  sscanf(base.c_str(), u8" %d %n", &ret, &len);
  if (len == int(base.length()))
    return ret; // Numeric
  sscanf(base.c_str(), u8" ! %d %n", &ret, &len);
  if (len == int(base.length()))
    return !ret; //! Numeric

  Object* holder;
  sscanf(base.c_str(), u8" OBJ:%p %n", &holder, &len);
  if (len == int(base.length()))
    return (holder != nullptr); // Object
  sscanf(base.c_str(), u8" ! OBJ:%p %n", &holder, &len);
  if (len == int(base.length()))
    return (holder == nullptr); //! Object

  if (base[0] == '!')
    return 0; //! Non-Numeric, Non-nullptr, Non-Object
  return 1; // Non-Numeric, Non-nullptr, Non-Object
}

std::map<std::u8string, std::u8string> Mind::cvars;

Mind::~Mind() {
  // if(body && body->Skill(prhash(u8"TBAScript")) >= 5034503 && body->Skill(prhash(u8"TBAScript"))
  // <= 5034507)
  //  fprintf(stderr, CBLU u8"Disabled(%p): '%d'\n" CNRM, this, body->Skill(prhash(u8"TBAScript")));
  if (type == mind_t::REMOTE)
    close_socket(pers);
  type = mind_t::NONE;
  Unattach();
  if (log >= 0)
    close(log);
  log = -1;

  auto itr = waiting.begin();
  for (; itr != waiting.end() && itr->second != this; ++itr) {
  }
  if (itr != waiting.end()) {
    // Set it for u8"never", so it won't run, and will be purged
    itr->first = std::numeric_limits<int64_t>::max();
  }

  svars = cvars; // Reset all variables
  ovars.clear();
}

void Mind::SetRemote(int fd) {
  type = mind_t::REMOTE;
  pers = fd;
  char8_t buf[256];

  if (log >= 0)
    return;

  static unsigned long lognum = 0;
  sprintf(buf, u8"logs/%.8lX.log%c", lognum, 0);
  while (std::filesystem::exists(buf)) {
    ++lognum;
    sprintf(buf, u8"logs/%.8lX.log%c", lognum, 0);
  }
  log =
      open(reinterpret_cast<char*>(buf), O_WRONLY | O_CREAT | O_TRUNC | O_SYNC, S_IRUSR | S_IWUSR);
}

void Mind::SetMob() {
  type = mind_t::MOB;
  pers = fileno(stderr);
}

void Mind::SetTBATrigger(Object* tr, Object* tripper, Object* targ, std::u8string text) {
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
  Attach(tr);
  spos_s.clear();
  spos_s.push_back(0);
  script = body->LongDesc();
  script += u8"\n";
  if (tripper)
    ovars[u8"actor"] = tripper;

  int stype = body->Skill(prhash(u8"TBAScriptType"));
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

void Mind::SetNPC() {
  type = mind_t::NPC;
  pers = fileno(stderr);
}

void Mind::SetSystem() {
  type = mind_t::SYSTEM;
  pers = fileno(stderr);
}

static const std::u8string sevs_p[] =
    {u8"-", u8"L", u8"L", u8"M", u8"M", u8"M", u8"S", u8"S", u8"S", u8"S", u8"D"};
static const std::u8string sevs_s[] =
    {u8"-", u8"l", u8"l", u8"m", u8"m", u8"m", u8"s", u8"s", u8"s", u8"s", u8"u"};
void Mind::UpdatePrompt() {
  static char8_t buf[65536];
  if (!Owner()) {
    SetPrompt(pers, u8"Player Name: ");
    if (pname.length() >= 1)
      SetPrompt(pers, u8"Password: ");
  } else if (prompt.length() > 0) {
    sprintf(buf, u8"%s> %c", prompt.c_str(), 0);
    SetPrompt(pers, buf);
  } else if (Body()) {
    sprintf(
        buf,
        u8"[%s][%s] %s> %c",
        sevs_p[std::min(10, Body()->Phys())].c_str(),
        sevs_s[std::min(10, Body()->Stun())].c_str(),
        Body()->NameC(),
        0);
    SetPrompt(pers, buf);
  } else
    SetPrompt(pers, u8"No Character> ");
}

void Mind::Attach(Object* bod) {
  body = bod;
  if (body)
    body->Attach(this);
}

void Mind::Unattach() {
  Object* bod = body;
  body = nullptr;
  if (bod)
    bod->Unattach(this);
}

bool Mind::Send(const std::u8string& mes) {
  if (type == mind_t::REMOTE) {
    SendOut(pers, mes);
  } else if (type == mind_t::MOB) {
    // return Think(); //Reactionary actions (NO!).
  } else if (type == mind_t::TBAMOB) {
    // HELPER TBA Mobs
    if (body && body->Parent() && (body->Skill(prhash(u8"TBAAction")) & 4096) // Helpers
        && ((body->Skill(prhash(u8"TBAAction")) & 2) == 0) // NON-SENTINEL
        && body->Stun() < 6 // I'm not stunned
        && body->Phys() < 6 // I'm not injured
        && (!body->IsAct(act_t::SLEEP)) // I'm not asleep
        && (!body->IsAct(act_t::REST)) // I'm not resting
        && (!body->IsAct(act_t::FIGHT)) // I'm not already fighting
    ) {
      if ((!strncmp(mes.c_str(), u8"From ", 5)) &&
          (mes.find(u8" you hear someone shout '") != std::u8string::npos) &&
          ((strstr(mes.c_str(), u8"HELP")) || (strstr(mes.c_str(), u8"ALARM")))) {
        char8_t buf[256] = u8"                                               ";
        sscanf(mes.c_str() + 4, u8"%128s", buf);

        Object* door = body->PickObject(buf, LOC_NEARBY);

        if (door && door->ActTarg(act_t::SPECIAL_LINKED) &&
            door->ActTarg(act_t::SPECIAL_LINKED)->Parent() &&
            TBACanWanderTo(door->ActTarg(act_t::SPECIAL_LINKED)->Parent())) {
          char8_t buf2[256] = u8"enter                                          ";
          sscanf(mes.c_str() + 4, u8"%128s", buf2 + 6);
          body->BusyFor(500, buf2);
        }
        return true;
      }
    }
    return Think(); // Reactionary actions (HACK!).
  } else if (type == mind_t::SYSTEM) {
    std::u8string newmes = u8"";
    if (body)
      newmes += body->ShortDesc();
    newmes += u8": ";
    newmes += mes;

    for (auto chr : newmes) {
      if (chr == '\n' || chr == '\r')
        chr = ' ';
    }
    newmes += u8"\n";

    write(pers, newmes.c_str(), newmes.length());
  }
  return true;
}

void Mind::SetPName(std::u8string pn) {
  pname = pn;
  if (player_exists(pname))
    Send(
        u8"{}{}{}Returning player - welcome back!\n",
        static_cast<char8_t>(IAC),
        static_cast<char8_t>(WILL),
        static_cast<char8_t>(TELOPT_ECHO));
  else
    Send(
        u8"{}{}{}New player ({}) - enter SAME new password twice.\n",
        static_cast<char8_t>(IAC),
        static_cast<char8_t>(WILL),
        static_cast<char8_t>(TELOPT_ECHO),
        pname);
}

void Mind::SetPPass(std::u8string ppass) {
  if (player_exists(pname)) {
    player = player_login(pname, ppass);
    if (player == nullptr) {
      if (player_exists(pname))
        Send(
            u8"{}{}{}Name and/or password is incorrect.\n",
            static_cast<char8_t>(IAC),
            static_cast<char8_t>(WONT),
            static_cast<char8_t>(TELOPT_ECHO));
      else
        Send(
            u8"{}{}{}Passwords do not match - try again.\n",
            static_cast<char8_t>(IAC),
            static_cast<char8_t>(WONT),
            static_cast<char8_t>(TELOPT_ECHO));
      pname = u8"";
      return;
    }
  } else if (!player) {
    new Player(pname, ppass);
    Send(
        u8"{}{}{}Enter password again for verification.\n",
        static_cast<char8_t>(IAC),
        static_cast<char8_t>(WILL),
        static_cast<char8_t>(TELOPT_ECHO));
    return;
  }

  Send(
      u8"{}{}{}",
      static_cast<char8_t>(IAC),
      static_cast<char8_t>(WONT),
      static_cast<char8_t>(TELOPT_ECHO));
  svars = player->Vars();
  player->Room()->SendDesc(this);
  player->Room()->SendContents(this);
}

void Mind::SetPlayer(std::u8string pn) {
  if (player_exists(pn)) {
    pname = pn;
    player = get_player(pname);
    svars = player->Vars();
  }
}

std::u8string Mind::Tactics(int phase) const {
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

bool Mind::TBAVarSub(std::u8string& line) const {
  size_t cur = line.find('%');
  size_t end;
  while (cur != std::u8string::npos) {
    end = line.find_first_of(u8"%. \t", cur + 1);
    if (end == std::u8string::npos)
      end = line.length();
    if (0
        //	|| body->Skill(prhash(u8"TBAScript")) == 5000099
        //	|| line.find(u8"eval loc ") != std::u8string::npos
        //	|| line.find(u8"set first ") != std::u8string::npos
        //	|| line.find(u8"exclaim") != std::u8string::npos
        //	|| line.find(u8"speech") != std::u8string::npos
    ) {
      fprintf(
          stderr,
          CGRN u8"#%d Debug: '%s'\n" CNRM,
          body->Skill(prhash(u8"TBAScript")),
          line.c_str());
    }
    std::u8string vname = line.substr(cur + 1, end - cur - 1);
    Object* obj = nullptr;
    std::u8string val = u8"";
    int is_obj = 0;
    if (ovars.count(vname) > 0) {
      obj = ovars.at(vname);
      is_obj = 1;
    } else if (svars.count(vname) > 0) {
      val = svars.at(vname);
    } else if (!strncmp(line.c_str() + cur, u8"%time.hour%", 11)) {
      Object* world = body->World();
      if (world->Skill(prhash(u8"Day Time")) && world->Skill(prhash(u8"Day Length"))) {
        int hour = world->Skill(prhash(u8"Day Time"));
        hour *= 24;
        hour /= world->Skill(prhash(u8"Day Length"));
        val = itos(hour);
      }
      end = line.find_first_of(u8"% \t", cur + 1); // Done.  Replace All.
    } else if (!strncmp(line.c_str() + cur, u8"%random.char%", 13)) {
      MinVec<1, Object*> others;
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
          fprintf(
              stderr,
              CGRN u8"#%d Random: '%s'\n" CNRM,
              body->Skill(prhash(u8"TBAScript")),
              obj->Noun().c_str());
        }
      } else {
        obj = nullptr;
      }
      is_obj = 1;
      end = line.find_first_of(u8"% \t", cur + 1); // Done.  Replace All.
    } else if (!strncmp(line.c_str() + cur, u8"%random.dir%", 12)) {
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
    } else if (!strncmp(line.c_str() + cur, u8"%random.", 8)) {
      if (isdigit(line[cur + 8])) {
        size_t vend = line.find_first_not_of(u8"0123456789", cur + 8);
        if (vend != std::u8string::npos && line[vend] == '%') {
          val = itos((rand() % atoi(line.c_str() + cur + 8)) + 1);
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
      std::u8string field = line.substr(start, end - start);
      if (is_obj) {
        if (!strcmp(field.c_str(), u8"id")) {
          // obj is already right
        } else if (!strcmp(field.c_str(), u8"vnum")) {
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
        } else if (!strcmp(field.c_str(), u8"gold")) {
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
        } else if (!strcmp(field.c_str(), u8"type")) {
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
        } else if (!strcmp(field.c_str(), u8"cost_per_day")) {
          val = u8"0";
          obj = nullptr;
          is_obj = 0;
        } else if (!strcmp(field.c_str(), u8"cost")) {
          val = u8"";
          if (obj)
            val = itos(obj->Value());
          obj = nullptr;
          is_obj = 0;
        } else if (!strcmp(field.c_str(), u8"count")) {
          val = u8"";
          if (obj)
            val = itos(obj->Quantity());
          obj = nullptr;
          is_obj = 0;
        } else if (!strcmp(field.c_str(), u8"weight")) {
          val = u8"";
          if (obj)
            val = itos(obj->Weight());
          obj = nullptr;
          is_obj = 0;
        } else if (!strcmp(field.c_str(), u8"sex")) {
          val = u8"";
          if (obj) {
            if (obj->Gender() == 'M')
              val = u8"male";
            else if (obj->Gender() == 'F')
              val = u8"female";
            else
              val = u8"none";
          }
          obj = nullptr;
          is_obj = 0;
        } else if (!strcmp(field.c_str(), u8"race")) {
          val = u8"";
          if (obj && obj->IsAnimate()) {
            val = u8"human"; // FIXME: Implement Race!
          }
          obj = nullptr;
          is_obj = 0;
        } else if (!strcmp(field.c_str(), u8"level")) {
          val = u8"";
          if (obj)
            val = itos(obj->TotalExp() / 10 + 1);
          obj = nullptr;
          is_obj = 0;
        } else if (!strcmp(field.c_str(), u8"name")) {
          val = u8"";
          if (obj)
            val = obj->Noun();
          obj = nullptr;
          is_obj = 0;
        } else if (!strcmp(field.c_str(), u8"shortdesc")) {
          val = u8"";
          if (obj)
            val = obj->ShortDesc();
          obj = nullptr;
          is_obj = 0;
        } else if (!strcmp(field.c_str(), u8"alias")) {
          val = u8"";
          if (obj)
            val = obj->ShortDesc();
          obj = nullptr;
          is_obj = 0;
        } else if (!strcmp(field.c_str(), u8"heshe")) {
          val = u8"";
          if (obj)
            val = obj->Pron();
          obj = nullptr;
          is_obj = 0;
        } else if (!strcmp(field.c_str(), u8"hisher")) {
          val = u8"";
          if (obj)
            val = obj->Poss();
          obj = nullptr;
          is_obj = 0;
        } else if (!strcmp(field.c_str(), u8"himher")) {
          val = u8"";
          if (obj)
            val = obj->Obje();
          obj = nullptr;
          is_obj = 0;
        } else if (!strcmp(field.c_str(), u8"maxhitp")) {
          val = itos(1000); // Everybody has 1000 HP.
          obj = nullptr;
          is_obj = 0;
        } else if (!strcmp(field.c_str(), u8"hitp")) {
          val = u8"";
          if (obj)
            val = itos(1000 - 50 * (obj->Phys() + obj->Stun()));
          obj = nullptr;
          is_obj = 0;
        } else if (!strcmp(field.c_str(), u8"align")) {
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
        } else if (!strcmp(field.c_str(), u8"is_pc")) {
          val = u8"";
          if (obj)
            val = bstr[is_pc(obj)];
          obj = nullptr;
          is_obj = 0;
        } else if (!strcmp(field.c_str(), u8"is_killer")) {
          val = u8"0"; // FIXME: Real value?
          obj = nullptr;
          is_obj = 0;
        } else if (!strcmp(field.c_str(), u8"is_thief")) {
          val = u8"0"; // FIXME: Real value?
          obj = nullptr;
          is_obj = 0;
        } else if (!strcmp(field.c_str(), u8"con")) {
          val = u8"";
          if (obj)
            val = itos(obj->NormAttribute(0) * 3);
          obj = nullptr;
          is_obj = 0;
        } else if (!strcmp(field.c_str(), u8"dex")) {
          val = u8"";
          if (obj)
            val = itos(obj->NormAttribute(1) * 3);
          obj = nullptr;
          is_obj = 0;
        } else if (!strcmp(field.c_str(), u8"str")) {
          val = u8"";
          if (obj)
            val = itos(obj->NormAttribute(2) * 3);
          obj = nullptr;
          is_obj = 0;
        } else if (!strcmp(field.c_str(), u8"stradd")) { // D&D is Dumb
          val = u8"0";
          obj = nullptr;
          is_obj = 0;
        } else if (!strcmp(field.c_str(), u8"cha")) {
          val = u8"";
          if (obj)
            val = itos(obj->NormAttribute(3) * 3);
          obj = nullptr;
          is_obj = 0;
        } else if (!strcmp(field.c_str(), u8"int")) {
          val = u8"";
          if (obj)
            val = itos(obj->NormAttribute(4) * 3);
          obj = nullptr;
          is_obj = 0;
        } else if (!strcmp(field.c_str(), u8"wis")) {
          val = u8"";
          if (obj)
            val = itos(obj->NormAttribute(5) * 3);
          obj = nullptr;
          is_obj = 0;
        } else if (!strcmp(field.c_str(), u8"pos")) {
          val = u8"";
          if (obj) {
            if (obj->IsAct(act_t::SLEEP))
              val = u8"sleeping";
            else if (obj->IsAct(act_t::REST))
              val = u8"resting";
            else if (obj->IsAct(act_t::FIGHT))
              val = u8"fighting";
            else if (obj->Pos() == pos_t::LIE)
              val = u8"resting";
            else if (obj->Pos() == pos_t::SIT)
              val = u8"sitting";
            else if (obj->Pos() == pos_t::STAND)
              val = u8"standing";
          }
          obj = nullptr;
          is_obj = 0;
        } else if (!strcmp(field.c_str(), u8"title")) {
          val = u8"";
          obj = nullptr;
          is_obj = 0;
        } else if (!strcmp(field.c_str(), u8"val0")) { // FIXME: Implement?
          val = u8"0";
          obj = nullptr;
          is_obj = 0;
        } else if (!strcmp(field.c_str(), u8"val1")) { // FIXME: Implement?
          val = u8"0";
          obj = nullptr;
          is_obj = 0;
        } else if (!strcmp(field.c_str(), u8"val2")) { // FIXME: Implement?
          val = u8"0";
          obj = nullptr;
          is_obj = 0;
        } else if (!strcmp(field.c_str(), u8"val3")) { // FIXME: Implement?
          val = u8"0";
          obj = nullptr;
          is_obj = 0;
        } else if (!strcmp(field.c_str(), u8"timer")) {
          val = u8"";
          if (obj)
            val = itos(obj->Skill(prhash(u8"Temporary"))); // FIXME: More Kinds?
          obj = nullptr;
          is_obj = 0;
        } else if (!strcmp(field.c_str(), u8"move")) {
          val = u8"";
          if (obj)
            val = itos(10 - obj->Stun());
          obj = nullptr;
          is_obj = 0;
        } else if (!strcmp(field.c_str(), u8"maxmove")) {
          val = u8"";
          if (obj)
            val = u8"10";
          obj = nullptr;
          is_obj = 0;
        } else if (!strcmp(field.c_str(), u8"mana")) {
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
        } else if (!strcmp(field.c_str(), u8"maxmana")) {
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
        } else if (!strcmp(field.c_str(), u8"saving_para")) {
          val = u8"";
          if (obj)
            val = u8"0";
          obj = nullptr;
          is_obj = 0;
        } else if (!strcmp(field.c_str(), u8"saving_rod")) {
          val = u8"";
          if (obj)
            val = u8"0";
          obj = nullptr;
          is_obj = 0;
        } else if (!strcmp(field.c_str(), u8"saving_petri")) {
          val = u8"";
          if (obj)
            val = u8"0";
          obj = nullptr;
          is_obj = 0;
        } else if (!strcmp(field.c_str(), u8"saving_breath")) {
          val = u8"";
          if (obj)
            val = u8"0";
          obj = nullptr;
          is_obj = 0;
        } else if (!strcmp(field.c_str(), u8"saving_spell")) {
          val = u8"";
          if (obj)
            val = u8"0";
          obj = nullptr;
          is_obj = 0;
        } else if (!strcmp(field.c_str(), u8"prac")) {
          val = u8"";
          if (obj)
            val = u8"0";
          obj = nullptr;
          is_obj = 0;
        } else if (!strcmp(field.c_str(), u8"questpoints")) {
          val = u8"";
          if (obj)
            val = u8"0";
          obj = nullptr;
          is_obj = 0;
        } else if (!strcmp(field.c_str(), u8"exp")) {
          val = u8"";
          if (obj)
            val = itos(obj->TotalExp());
          obj = nullptr;
          is_obj = 0;
        } else if (!strcmp(field.c_str(), u8"hunger")) {
          val = u8"";
          if (obj)
            val = itos(obj->Skill(prhash(u8"Hungry"))); // FIXME: Convert
          obj = nullptr;
          is_obj = 0;
        } else if (!strcmp(field.c_str(), u8"thirst")) {
          val = u8"";
          if (obj)
            val = itos(obj->Skill(prhash(u8"Thirsty"))); // FIXME: Convert
          obj = nullptr;
          is_obj = 0;
        } else if (!strcmp(field.c_str(), u8"drunk")) {
          val = u8"";
          if (obj)
            val = u8"0"; // FIXME: Query Drunkenness Here
          obj = nullptr;
          is_obj = 0;
        } else if (!strcmp(field.c_str(), u8"class")) {
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
        } else if (!strcmp(field.c_str(), u8"canbeseen")) {
          val = u8"";
          if (obj)
            val =
                bstr[!(obj->HasSkill(prhash(u8"Invisible")) || obj->HasSkill(prhash(u8"Hidden")))];
          obj = nullptr;
          is_obj = 0;
        } else if (!strcmp(field.c_str(), u8"affect")) {
          val = u8""; // FIXME: Translate & List Spell Effects?
          obj = nullptr;
          is_obj = 0;
        } else if (!strcmp(field.c_str(), u8"fighting")) {
          if (obj)
            obj = obj->ActTarg(act_t::FIGHT);
        } else if (!strcmp(field.c_str(), u8"worn_by")) {
          if (obj) {
            Object* owner = obj->Owner();
            if (owner && owner->Wearing(obj))
              obj = owner;
            else
              obj = nullptr;
          } else
            obj = nullptr;
        } else if (!strcmp(field.c_str(), u8"room")) {
          while (obj && obj->Skill(prhash(u8"TBARoom")) == 0)
            obj = obj->Parent();
        } else if (!strcmp(field.c_str(), u8"people")) {
          if (obj)
            obj = obj->PickObject(u8"someone", LOC_INTERNAL);
        } else if (!strcmp(field.c_str(), u8"contents")) {
          if (obj)
            obj = obj->PickObject(u8"something", LOC_INTERNAL);
        } else if (!strcmp(field.c_str(), u8"inventory")) {
          if (obj)
            obj = obj->PickObject(u8"something", LOC_INTERNAL | LOC_NOTWORN);
        } else if ((!strcmp(field.c_str(), u8"eq(*)")) || (!strcmp(field.c_str(), u8"eq"))) {
          if (obj)
            obj = obj->PickObject(u8"something", LOC_INTERNAL | LOC_NOTUNWORN);
        } else if (
            (!strcmp(field.c_str(), u8"eq(light)")) || (!strcmp(field.c_str(), u8"eq(hold)")) ||
            (!strcmp(field.c_str(), u8"eq(0)")) || (!strcmp(field.c_str(), u8"eq(17)"))) {
          if (obj)
            obj = obj->ActTarg(act_t::HOLD);
        } else if (
            (!strcmp(field.c_str(), u8"eq(wield)")) || (!strcmp(field.c_str(), u8"eq(16)"))) {
          if (obj)
            obj = obj->ActTarg(act_t::WIELD);
        } else if (
            (!strcmp(field.c_str(), u8"eq(rfinger)")) || (!strcmp(field.c_str(), u8"eq(1)"))) {
          if (obj)
            obj = obj->ActTarg(act_t::WEAR_RFINGER);
        } else if (
            (!strcmp(field.c_str(), u8"eq(lfinger)")) || (!strcmp(field.c_str(), u8"eq(2)"))) {
          if (obj)
            obj = obj->ActTarg(act_t::WEAR_LFINGER);
        } else if ((!strcmp(field.c_str(), u8"eq(neck1)")) || (!strcmp(field.c_str(), u8"eq(3)"))) {
          if (obj)
            obj = obj->ActTarg(act_t::WEAR_NECK);
        } else if ((!strcmp(field.c_str(), u8"eq(neck2)")) || (!strcmp(field.c_str(), u8"eq(4)"))) {
          if (obj)
            obj = obj->ActTarg(act_t::WEAR_COLLAR);
        } else if ((!strcmp(field.c_str(), u8"eq(body)")) || (!strcmp(field.c_str(), u8"eq(5)"))) {
          if (obj)
            obj = obj->ActTarg(act_t::WEAR_CHEST);
        } else if ((!strcmp(field.c_str(), u8"eq(head)")) || (!strcmp(field.c_str(), u8"eq(6)"))) {
          if (obj)
            obj = obj->ActTarg(act_t::WEAR_HEAD);
        } else if ((!strcmp(field.c_str(), u8"eq(legs)")) || (!strcmp(field.c_str(), u8"eq(7)"))) {
          if (obj)
            obj = obj->ActTarg(act_t::WEAR_LLEG);
        } else if ((!strcmp(field.c_str(), u8"eq(feet)")) || (!strcmp(field.c_str(), u8"eq(8)"))) {
          if (obj)
            obj = obj->ActTarg(act_t::WEAR_LFOOT);
        } else if ((!strcmp(field.c_str(), u8"eq(hands)")) || (!strcmp(field.c_str(), u8"eq(9)"))) {
          if (obj)
            obj = obj->ActTarg(act_t::WEAR_LHAND);
        } else if ((!strcmp(field.c_str(), u8"eq(arms)")) || (!strcmp(field.c_str(), u8"eq(10)"))) {
          if (obj)
            obj = obj->ActTarg(act_t::WEAR_LARM);
        } else if (
            (!strcmp(field.c_str(), u8"eq(shield)")) || (!strcmp(field.c_str(), u8"eq(11)"))) {
          if (obj)
            obj = obj->ActTarg(act_t::WEAR_SHIELD);
        } else if (
            (!strcmp(field.c_str(), u8"eq(about)")) || (!strcmp(field.c_str(), u8"eq(12)"))) {
          if (obj)
            obj = obj->ActTarg(act_t::WEAR_LSHOULDER);
        } else if (
            (!strcmp(field.c_str(), u8"eq(waits)")) || (!strcmp(field.c_str(), u8"eq(13)"))) {
          if (obj)
            obj = obj->ActTarg(act_t::WEAR_WAIST);
        } else if (
            (!strcmp(field.c_str(), u8"eq(rwrist)")) || (!strcmp(field.c_str(), u8"eq(14)"))) {
          if (obj)
            obj = obj->ActTarg(act_t::WEAR_RWRIST);
        } else if (
            (!strcmp(field.c_str(), u8"eq(lwrist)")) || (!strcmp(field.c_str(), u8"eq(15)"))) {
          if (obj)
            obj = obj->ActTarg(act_t::WEAR_LWRIST);
        } else if (!strcmp(field.c_str(), u8"carried_by")) {
          if (obj)
            obj = obj->Owner();
        } else if (!strcmp(field.c_str(), u8"next_in_list")) {
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
        } else if (!strcmp(field.c_str(), u8"next_in_room")) {
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
        } else if (!strcmp(field.c_str(), u8"master")) {
          if (obj)
            obj = obj->ActTarg(act_t::FOLLOW); // FIXME: More Kinds?
        } else if (!strcmp(field.c_str(), u8"follower")) {
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
        } else if (!strncmp(field.c_str(), u8"skill(", 6)) {
          val = u8"";
          if (obj) {
            size_t num = field.find_first_of(u8") .%", 6);
            auto skl = get_skill(field.substr(6, num - 6));
            val = itos(obj->Skill(skl));
          }
          obj = nullptr;
          is_obj = 0;
        } else if (!strncmp(field.c_str(), u8"varexists(", 10)) {
          val = u8"";
          if (obj) {
            size_t num = field.find_first_of(u8") .%", 10);
            std::u8string var = field.substr(10, num - 10); // FIXME: Variables!
            val = bstr[obj->HasSkill(crc32c(fmt::format(u8"TBA:{}", var)))];
          }
          obj = nullptr;
          is_obj = 0;
        } else if (!strncmp(field.c_str(), u8"has_item(", 9)) {
          int vnum = -1;
          size_t num = field.find_first_not_of(u8"0123456789", 9);
          sscanf(field.c_str() + 9, u8"%d", &vnum);
          val = u8"";
          if (obj && vnum != -1 && field[num] == ')') {
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
        } else if (!strncmp(field.c_str(), u8"affect(", 7)) {
          size_t len = field.find_first_not_of(u8"abcdefghijklmnopqrstuvwxyz-", 7);
          auto spell = tba_spellconvert(field.substr(7, len - 7));
          // fprintf(
          //    stderr,
          //    CBLU u8"Interpreting '%s' as 'spell = %s'\n" CNRM,
          //    field.c_str(),
          //    spell.c_str());
          val = u8"";
          if (obj) {
            val = itos(obj->Power(crc32c(spell)));
          }
          obj = nullptr;
          is_obj = 0;
        } else if (!strncmp(field.c_str(), u8"vnum(", 5)) {
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
              std::u8string query = field.substr(5, off - 5);
              int qnum = TBAEval(query.c_str());
              val = bstr[(vnum == qnum)];
            }
          }
          obj = nullptr;
          is_obj = 0;
        } else if (!strcmp(field.c_str(), u8"pos(sleeping)")) {
          if (obj) {
            obj->SetPos(pos_t::LIE);
            obj->StopAct(act_t::REST);
            obj->AddAct(act_t::SLEEP);
          }
          obj = nullptr;
          val = u8"";
          is_obj = 0;
        } else if (!strcmp(field.c_str(), u8"pos(resting)")) {
          if (obj) {
            obj->StopAct(act_t::SLEEP);
            obj->SetPos(pos_t::SIT);
            obj->AddAct(act_t::REST);
          }
          obj = nullptr;
          val = u8"";
          is_obj = 0;
        } else if (!strcmp(field.c_str(), u8"pos(sitting)")) {
          if (obj) {
            obj->StopAct(act_t::SLEEP);
            obj->StopAct(act_t::REST);
            obj->SetPos(pos_t::SIT);
          }
          obj = nullptr;
          val = u8"";
          is_obj = 0;
        } else if (!strcmp(field.c_str(), u8"pos(fighting)")) {
          if (obj) {
            obj->StopAct(act_t::SLEEP);
            obj->StopAct(act_t::REST);
            obj->SetPos(pos_t::STAND);
          }
          obj = nullptr;
          val = u8"";
          is_obj = 0;
        } else if (!strcmp(field.c_str(), u8"pos(standing)")) {
          if (obj) {
            obj->StopAct(act_t::SLEEP);
            obj->StopAct(act_t::REST);
            obj->SetPos(pos_t::STAND);
          }
          obj = nullptr;
          val = u8"";
          is_obj = 0;
          // Is no general fighting state, must fight someone!
        } else {
          fprintf(
              stderr,
              CRED u8"#%d Error: Bad sub-obj '%s' in '%s'\n" CNRM,
              body->Skill(prhash(u8"TBAScript")),
              field.c_str(),
              line.c_str());
          return false;
        }
      } else {
        if (!strcmp(field.c_str(), u8"mudcommand")) {
          // val is already right
        } else if (!strcmp(field.c_str(), u8"car")) {
          size_t apos = val.find_first_of(u8" \t\n\r");
          if (apos != std::u8string::npos) {
            val = val.substr(0, apos);
          }
        } else if (!strcmp(field.c_str(), u8"cdr")) {
          size_t apos = val.find_first_of(u8" \t\n\r");
          if (apos != std::u8string::npos) {
            apos = val.find_first_not_of(u8" \t\n\r", apos);
            if (apos != std::u8string::npos)
              val = val.substr(apos);
            else
              val = u8"";
          } else
            val = u8"";
        } else if (!strcmp(field.c_str(), u8"trim")) {
          trim_string(val);
        } else {
          fprintf(
              stderr,
              CRED u8"#%d Error: Bad sub-str '%s' in '%s'\n" CNRM,
              body->Skill(prhash(u8"TBAScript")),
              field.c_str(),
              line.c_str());
          return false;
        }
      }
    }
    if (end == std::u8string::npos)
      end = line.length();
    else if (line[end] == '%')
      ++end;
    if (is_obj) {
      char8_t buf2[256] = u8"";
      sprintf(buf2, u8"OBJ:%p", obj);
      line.replace(cur, end - cur, buf2);
    } else { // std::u8string OR u8""
      line.replace(cur, end - cur, val);
    }
    cur = line.find('%', cur + 1);
  }
  if (0
      //	|| body->Skill(prhash(u8"TBAScript")) == 5000099
      //	|| line.find(u8"eval loc ") != std::u8string::npos
      //	|| line.find(u8"set first ") != std::u8string::npos
      //	|| line.find(u8"exclaim") != std::u8string::npos
      //	|| line.find(u8"speech") != std::u8string::npos
  ) {
    fprintf(
        stderr,
        CGRN u8"#%d Debug: '%s' <-Final\n" CNRM,
        body->Skill(prhash(u8"TBAScript")),
        line.c_str());
  }
  return true;
}

#define QUOTAERROR1 CRED u8"#%d Error: script quota exceeded - killed.\n" CNRM
#define QUOTAERROR2 body->Skill(prhash(u8"TBAScript"))
#define PING_QUOTA()                             \
  {                                              \
    --quota;                                     \
    if (quota < 1) {                             \
      fprintf(stderr, QUOTAERROR1, QUOTAERROR2); \
      return -1;                                 \
    }                                            \
  }

// Return:
// 0 to continue running
// 1 to be done now (suspend)
// -1 to destroy mind (error/done)
int Mind::TBARunLine(std::u8string line) {
  if (0
      //	|| body->Skill(prhash(u8"TBAScript")) == 5000099
      //	|| line.find(u8"eval loc ") != std::u8string::npos
      //	|| line.find(u8"set first ") != std::u8string::npos
      //	|| line.find(u8"exclaim") != std::u8string::npos
      //	|| line.find(u8"speech") != std::u8string::npos
  ) {
    fprintf(
        stderr,
        CGRN u8"#%d Debug: Running '%s'\n" CNRM,
        body->Skill(prhash(u8"TBAScript")),
        line.c_str());
  }
  Object* room = ovars.at(u8"self");
  while (room && room->Skill(prhash(u8"TBARoom")) == 0) {
    if (room->Skill(prhash(u8"Invisible")) > 999)
      room = nullptr; // Not really there
    else
      room = room->Parent();
  }
  if (!room) { // Not in a room (dup clone, in a popper, etc...).
    //    fprintf(stderr, CRED u8"#%d Error: No room in '%s'\n" CNRM,
    //	body->Skill(prhash(u8"TBAScript")), line.c_str()
    //	);
    return -1;
  }
  // Needs to be alive! MOB & MOB-* (Not -DEATH or -GLOBAL)
  if ((body->Skill(prhash(u8"TBAScriptType")) & 0x103FFDE) > 0x1000000) {
    if (ovars.at(u8"self")->IsAct(act_t::DEAD) || ovars.at(u8"self")->IsAct(act_t::DYING) ||
        ovars.at(u8"self")->IsAct(act_t::UNCONSCIOUS)) {
      //      fprintf(stderr, CGRN u8"#%d Debug: Triggered on downed MOB.\n" CNRM,
      //	body->Skill(prhash(u8"TBAScript"))
      //	);
      spos_s.back() = std::u8string::npos; // Jump to End
      return 0; // Allow re-run (in case of resurrection/waking/etc...).
    }
  }

  size_t spos = spos_s.back();
  int vnum = body->Skill(prhash(u8"TBAScript"));
  if (!TBAVarSub(line)) {
    fprintf(stderr, CRED u8"#%d Error: VarSub failed in '%s'\n" CNRM, vnum, line.c_str());
    return -1;
  }

  int com = COM_NONE; // ComNum for Pass-Through
  std::u8string_view cmd = line;

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
  //    fprintf(stderr, CRED u8"#%d Error: Failed to fully expand '%s'\n" CNRM,
  //	body->Skill(prhash(u8"TBAScript")), line.c_str()
  //	);
  //    return -1;
  //    }

  if (!strncmp(line.c_str(), u8"unset ", 6)) {
    size_t lpos = line.find_first_not_of(u8" \t", 6);
    if (lpos != std::u8string::npos) {
      std::u8string var = line.substr(lpos);
      trim_string(var);
      svars.erase(var);
      ovars.erase(var);
      ovars[u8"context"]->SetSkill(fmt::format(u8"TBA:{}", var), 0);
    } else {
      fprintf(
          stderr,
          CRED u8"#%d Error: Malformed unset '%s'\n" CNRM,
          body->Skill(prhash(u8"TBAScript")),
          line.c_str());
      return -1;
    }
    return 0;
  }

  else if ((!strncmp(line.c_str(), u8"eval ", 5)) || (!strncmp(line.c_str(), u8"set ", 4))) {
    char8_t coml = ascii_tolower(line[0]); // To tell eval from set later.
    size_t lpos = line.find_first_not_of(u8" \t", 4);
    if (lpos != std::u8string::npos) {
      line = line.substr(lpos);
      size_t end1 = line.find_first_of(u8" \t\n\r");
      if (end1 != std::u8string::npos) {
        std::u8string var = line.substr(0, end1);
        lpos = line.find_first_not_of(u8" \t", end1 + 1);
        if (lpos != std::u8string::npos) {
          std::u8string val = line.substr(lpos);
          if (0
              //		|| var.find(u8"midgaard") != std::u8string::npos
              //		|| var.find(u8"exclaim") != std::u8string::npos
              //		|| var.find(u8"speech") != std::u8string::npos
          ) {
            fprintf(
                stderr,
                CGRN u8"#%d Debug: '%s' = '%s'\n" CNRM,
                body->Skill(prhash(u8"TBAScript")),
                var.c_str(),
                val.c_str());
          }
          if (coml == 'e') {
            int valnum = body->Skill(prhash(u8"TBAScript"));
            if (!TBAVarSub(val)) {
              fprintf(stderr, CRED u8"#%d Error: Eval failed in '%s'\n" CNRM, valnum, line.c_str());
              return -1;
            }
            val = TBAComp(val);
          }
          if (!strncmp(val.c_str(), u8"OBJ:", 4)) { // Encoded Object
            ovars[var] = nullptr;
            sscanf(val.c_str(), u8"OBJ:%p", &(ovars[var]));
            svars.erase(var);
          } else {
            svars[var] = val;
            ovars.erase(var);
          }
        } else { // Only space after varname
          svars[var] = u8"";
          ovars.erase(var);
        }
      } else { // Nothing after varname
        svars[line] = u8"";
        ovars.erase(line);
      }
    }
    return 0;
  }

  else if ((!strncmp(line.c_str(), u8"extract ", 8))) {
    size_t lpos = line.find_first_not_of(u8" \t", 8);
    if (lpos != std::u8string::npos) {
      line = line.substr(lpos);
      size_t end1 = line.find_first_of(u8" \t\n\r");
      if (end1 != std::u8string::npos) {
        std::u8string var = line.substr(0, end1);
        lpos = line.find_first_not_of(u8" \t", end1 + 1);
        if (lpos != std::u8string::npos) {
          int wnum = atoi(line.c_str() + lpos) - 1; // Start at 0, -1=fail
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
              svars[var] = line.substr(lpos, end1 - lpos);
            } else {
              svars[var] = u8"";
            }
            ovars.erase(var);
          } else if (wnum < 0) { // Bad number after varname
            fprintf(
                stderr,
                CRED u8"#%d Error: Malformed extract '%s'\n" CNRM,
                body->Skill(prhash(u8"TBAScript")),
                line.c_str());
            return -1;
          }
        } else { // Only space after varname
          fprintf(
              stderr,
              CRED u8"#%d Error: Malformed extract '%s'\n" CNRM,
              body->Skill(prhash(u8"TBAScript")),
              line.c_str());
          return -1;
        }
      } else { // Nothing after varname
        fprintf(
            stderr,
            CRED u8"#%d Error: Malformed extract '%s'\n" CNRM,
            body->Skill(prhash(u8"TBAScript")),
            line.c_str());
        return -1;
      }
    }
    return 0;
  }

  else if (!strncmp(line.c_str(), u8"at ", 3)) {
    int dnum, pos;
    if (sscanf(line.c_str(), u8"at %d %n", &dnum, &pos) < 1) {
      fprintf(
          stderr,
          CRED u8"#%d Error: Malformed at '%s'\n" CNRM,
          body->Skill(prhash(u8"TBAScript")),
          line.c_str());
      return -1;
    }
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
      fprintf(
          stderr,
          CRED u8"#%d Error: Can't find room in '%s'\n" CNRM,
          body->Skill(prhash(u8"TBAScript")),
          line.c_str());
      return -1;
    }
    Object* oldp = nullptr;
    if (ovars.at(u8"self")->Parent() != room) {
      oldp = ovars.at(u8"self")->Parent();
      oldp->RemoveLink(ovars.at(u8"self"));
      ovars.at(u8"self")->SetParent(room);
    }
    int ret = TBARunLine(line.substr(pos));
    if (oldp) {
      ovars.at(u8"self")->Parent()->RemoveLink(ovars.at(u8"self"));
      ovars.at(u8"self")->SetParent(oldp);
    }
    return ret;
  }

  else if (!strncmp(line.c_str(), u8"context ", 8)) {
    Object* con = nullptr;
    if (sscanf(line.c_str() + 8, u8" OBJ:%p", &con) >= 1 && con != nullptr) {
      ovars[u8"context"] = con;
    } else {
      fprintf(
          stderr,
          CRED u8"#%d Error: No Context Object '%s'\n" CNRM,
          body->Skill(prhash(u8"TBAScript")),
          line.c_str());
      return 1;
    }
  }

  else if (!strncmp(line.c_str(), u8"rdelete ", 8)) {
    Object* con = nullptr;
    char8_t var[256] = u8"";
    if (sscanf(line.c_str() + 7, u8" %s OBJ:%p", var, &con) >= 2 && con != nullptr) {
      con->SetSkill(fmt::format(u8"TBA:{}", var), 0);
      if (con->IsAnimate()) {
        con->Accomplish(body->Skill(prhash(u8"Accomplishment")), u8"completing a quest");
      }
      //      fprintf(stderr, CGRN u8"#%d Debug: RDelete '%s'\n" CNRM,
      //		body->Skill(prhash(u8"TBAScript")), line.c_str());
    } else {
      fprintf(
          stderr,
          CRED u8"#%d Error: No RDelete VarName/ID '%s'\n" CNRM,
          body->Skill(prhash(u8"TBAScript")),
          line.c_str());
      return 1;
    }
  }

  else if (!strncmp(line.c_str(), u8"remote ", 7)) {
    Object* con = nullptr;
    char8_t var[256] = u8"";
    if (sscanf(line.c_str() + 7, u8" %s OBJ:%p", var, &con) >= 2 && con != nullptr) {
      if (svars.count(var) > 0) {
        int val = atoi(svars[var].c_str());
        con->SetSkill(fmt::format(u8"TBA:{}", var), val);
        if (con->IsAnimate()) {
          con->Accomplish(body->Skill(prhash(u8"Accomplishment")), u8"role playing");
        }
        //	fprintf(stderr, CGRN u8"#%d Debug: Remote %s=%d '%s'\n" CNRM,
        //		body->Skill(prhash(u8"TBAScript")), var, val, line.c_str());
      } else {
        fprintf(
            stderr,
            CRED u8"#%d Error: Non-Existent Remote %s '%s'\n" CNRM,
            body->Skill(prhash(u8"TBAScript")),
            var,
            line.c_str());
        return 1;
      }
    } else {
      fprintf(
          stderr,
          CRED u8"#%d Error: No Remote VarName/ID '%s'\n" CNRM,
          body->Skill(prhash(u8"TBAScript")),
          line.c_str());
      return 1;
    }
  }

  else if (!strncmp(line.c_str(), u8"global ", 7)) {
    Object* con = ovars[u8"context"];
    char8_t var[256] = u8"";
    if (sscanf(line.c_str() + 7, u8" %s", var) >= 1 && con != nullptr) {
      if (svars.count(var) > 0) {
        int val = atoi(svars[var].c_str());
        con->SetSkill(fmt::format(u8"TBA:{}", var), val);
        //	fprintf(stderr, CGRN u8"#%d Debug: Global %s=%d '%s'\n" CNRM,
        //		body->Skill(prhash(u8"TBAScript")), var, val, line.c_str());
      } else {
        fprintf(
            stderr,
            CRED u8"#%d Error: Non-Existent Global %s '%s'\n" CNRM,
            body->Skill(prhash(u8"TBAScript")),
            var,
            line.c_str());
        return 1;
      }
    } else {
      fprintf(
          stderr,
          CRED u8"#%d Error: No Global VarName '%s'\n" CNRM,
          body->Skill(prhash(u8"TBAScript")),
          line.c_str());
      return 1;
    }
  }

  else if (!strncmp(line.c_str(), u8"wait until ", 11)) {
    int hour = 0, minute = 0, cur = 0;
    if (sscanf(line.c_str() + 11, u8"%d:%d", &hour, &minute) > 0) {
      if (hour >= 100)
        hour /= 100;
      minute += hour * 60;
      Object* world = room->World();
      if (world->Skill(prhash(u8"Day Time")) && world->Skill(prhash(u8"Day Length"))) {
        cur = world->Skill(prhash(u8"Day Time"));
        cur *= 24 * 60;
        cur /= world->Skill(prhash(u8"Day Length"));
      }
      if (minute < cur)
        minute += 24 * 60; // Not Time Until Tomorrow!
      if (minute > cur) { // Not Time Yet!
        Suspend((minute - cur) * 1000 * world->Skill(prhash(u8"Day Length")) / 24);
        // Note: The above calculation removed the *60 and the /60
        return 1;
      }
    } else {
      fprintf(
          stderr,
          CRED u8"#%d Error: Told '%s'\n" CNRM,
          body->Skill(prhash(u8"TBAScript")),
          line.c_str());
      return -1;
    }
  }

  else if (!strncmp(line.c_str(), u8"wait ", 5)) {
    int time = 0;
    sscanf(line.c_str() + 5, u8"%d", &time);
    if (time > 0) {
      // if(body->Skill(prhash(u8"TBAScript")) >= 5034503 && body->Skill(prhash(u8"TBAScript")) <=
      // 5034507)
      //  fprintf(stderr, CBLU u8"#%d Suspending for: %d\n" CNRM,
      //  body->Skill(prhash(u8"TBAScript")), time * 1000);
      Suspend(time * 1000);
      return 1;
    } else {
      fprintf(
          stderr,
          CRED u8"#%d Error: Told '%s'\n" CNRM,
          body->Skill(prhash(u8"TBAScript")),
          line.c_str());
      return -1;
    }
  }

  else if ((!strncmp(line.c_str(), u8"oset ", 5)) || (!strncmp(line.c_str(), u8"osetval ", 8))) {
    int v1, v2;
    size_t end = line.find(u8" ");
    if (sscanf(line.c_str() + end, u8" %d %d", &v1, &v2) != 2) {
      fprintf(
          stderr,
          CRED u8"#%d Error: Told '%s'\n" CNRM,
          body->Skill(prhash(u8"TBAScript")),
          line.c_str());
      return -1;
    } else if (ovars[u8"self"]->Skill(prhash(u8"Liquid Source")) && v1 == 0) {
      if (v2 < 0)
        v2 = 1 << 30;
      ovars[u8"self"]->SetSkill(prhash(u8"Liquid Source"), v2 + 1);
    } else if (ovars[u8"self"]->Skill(prhash(u8"Liquid Source")) && v1 == 1) {
      if (ovars[u8"self"]->Contents().size() < 1) {
        fprintf(
            stderr,
            CYEL u8"#%d Warning: Empty fountain '%s'\n" CNRM,
            body->Skill(prhash(u8"TBAScript")),
            line.c_str());
        return -1;
      }
      ovars[u8"self"]->Contents().front()->SetSkill(prhash(u8"Quantity"), v2 + 1);
    } else {
      fprintf(
          stderr,
          CRED u8"#%d Error: Unimplemented oset '%s'\n" CNRM,
          body->Skill(prhash(u8"TBAScript")),
          line.c_str());
      return -1;
    }
  }

  else if (!strncmp(line.c_str(), u8"if ", 3)) {
    if (!TBAEval(line.c_str() + 3)) { // Was false
      int depth = 0;
      while (spos != std::u8string::npos) { // Skip to end/elseif
        //        PING_QUOTA();
        if ((!depth) && (!strncmp(script.c_str() + spos, u8"elseif ", 7))) {
          spos += 4; // Make it into an u8"if" and go
          break;
        } else if (!strncmp(script.c_str() + spos, u8"else", 4)) {
          if (!depth) { // Only right if all the way back
            spos = skip_line(script, spos);
            break;
          }
        } else if (!strncmp(script.c_str() + spos, u8"end", 3)) {
          if (!depth) { // Only done if all the way back
            spos = skip_line(script, spos);
            break;
          }
          --depth; // Otherwise am just 1 nesting level less deep
        } else if (!strncmp(script.c_str() + spos, u8"if ", 3)) {
          ++depth; // Am now 1 nesting level deeper!
        }
        spos = skip_line(script, spos);
      }
      spos_s.back() = spos; // Save skip-to position in real PC
    }
  }

  else if (!strncmp(line.c_str(), u8"else", 4)) { // else/elseif
    int depth = 0;
    while (spos != std::u8string::npos) { // Skip to end (considering nesting)
      //      PING_QUOTA();
      if (!strncmp(script.c_str() + spos, u8"end", 3)) {
        if (depth == 0) { // Only done if all the way back
          spos = skip_line(script, spos);
          break;
        }
        --depth; // Otherwise am just 1 nesting level less deep
      } else if (!strncmp(script.c_str() + spos, u8"if ", 3)) {
        ++depth; // Am now 1 nesting level deeper!
      }
      spos = skip_line(script, spos);
    }
    spos_s.back() = spos; // Save skip-to position in real PC
  }

  else if (!strncmp(line.c_str(), u8"while ", 6)) {
    int depth = 0;
    size_t rep = prev_line(script, spos);
    size_t begin = spos;
    while (spos != std::u8string::npos) { // Skip to end (considering nesting)
      //      PING_QUOTA();
      if (!strncmp(script.c_str() + spos, u8"done", 4)) {
        if (depth == 0) { // Only done if all the way back
          spos = skip_line(script, spos);
          break;
        }
        --depth; // Otherwise am just 1 nesting level less deep
      } else if (!strncmp(script.c_str() + spos, u8"switch ", 7)) {
        ++depth; // Am now 1 nesting level deeper!
      } else if (!strncmp(script.c_str() + spos, u8"while ", 6)) {
        ++depth; // Am now 1 nesting level deeper!
      }
      spos = skip_line(script, spos);
    }
    if (TBAEval(line.c_str() + 6)) {
      spos_s.back() = rep; // Will repeat the u8"while"
      spos_s.push_back(begin); // But run the inside of the loop first.
    } else {
      spos_s.back() = spos; // Save after-done position in real PC
    }
  }

  else if (!strncmp(line.c_str(), u8"switch ", 7)) {
    int depth = 0;
    size_t targ = 0;
    std::u8string value = line.substr(7);
    trim_string(value);
    while (spos != std::u8string::npos) { // Skip to end (considering nesting)
      //      PING_QUOTA();
      if (!strncmp(script.c_str() + spos, u8"done", 4)) {
        if (depth == 0) { // Only done if all the way back
          spos = skip_line(script, spos);
          break;
        }
        --depth; // Otherwise am just 1 nesting level less deep
      } else if (!strncmp(script.c_str() + spos, u8"switch ", 7)) {
        ++depth; // Am now 1 nesting level deeper!
      } else if (!strncmp(script.c_str() + spos, u8"while ", 6)) {
        ++depth; // Am now 1 nesting level deeper!
      } else if (
          depth == 0 && (!strncmp(script.c_str() + spos, u8"case ", 5)) &&
          TBAEval(value + u8" == " + script.substr(spos + 5))) { // The actual case I want!
        spos = skip_line(script, spos);
        targ = spos;
        continue;
      } else if (
          depth == 0 &&
          (!strncmp(
              script.c_str() + spos,
              u8"default",
              7))) { // Maybe the case I want
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

  else if (!strncmp(line.c_str(), u8"break", 5)) { // Skip to done
    int depth = 0;
    while (spos != std::u8string::npos) { // Skip to end (considering nesting)
      //      PING_QUOTA();
      if (!strncmp(script.c_str() + spos, u8"done", 4)) {
        if (depth == 0) { // Only done if all the way back
          break;
        }
        --depth; // Otherwise am just 1 nesting level less deep
      } else if (!strncmp(script.c_str() + spos, u8"switch ", 7)) {
        ++depth; // Am now 1 nesting level deeper!
      } else if (!strncmp(script.c_str() + spos, u8"while ", 6)) {
        ++depth; // Am now 1 nesting level deeper!
      }
      spos = skip_line(script, spos);
    }
    spos_s.back() = spos; // Save done position in real PC
  }

  else if ((!strncmp(line.c_str(), u8"asound ", 7))) {
    size_t start = line.find_first_not_of(u8" \t\r\n", 7);
    if (room && start != std::u8string::npos) {
      std::u8string mes = line.substr(start);
      trim_string(mes);
      replace_all(mes, u8"You hear ", u8"");
      replace_all(mes, u8" can be heard close by", u8"");
      replace_all(mes, u8" is heard close by", u8"");
      replace_all(mes, u8" from close by", u8"");
      replace_all(mes, u8" from nearby", u8"");
      mes += u8"\n";
      room->Loud(2, mes.c_str()); // 2 will go through 1 closed door.
    }
  }

  else if ((!strncmp(line.c_str(), u8"zoneecho ", 9))) {
    size_t start = line.find_first_not_of(u8" \t\r\n", 9);
    start = line.find_first_of(u8" \t\r\n", start);
    start = line.find_first_not_of(u8" \t\r\n", start);
    if (room && start != std::u8string::npos) {
      std::u8string mes = line.substr(start);
      trim_string(mes);
      mes += u8"\n";
      room->SendOut(ALL, 0, mes.c_str(), u8"", nullptr, nullptr, false);
      room->Loud(8, mes.c_str()); // 8 will go 4-8 rooms.
    }
  }

  else if (!strncmp(line.c_str(), u8"echoaround ", 11)) {
    Object* targ = nullptr;
    char8_t mes[256] = u8"";
    sscanf(line.c_str() + 11, u8" OBJ:%p %254[^\n\r]", &targ, mes);
    mes[strlen(mes) + 1] = 0;
    mes[strlen(mes)] = '\n';
    Object* troom = targ;
    while (troom && troom->Skill(prhash(u8"TBARoom")) == 0)
      troom = troom->Parent();
    if (troom && targ)
      troom->SendOut(0, 0, mes, u8"", targ, nullptr);
  }

  else if ((!strncmp(line.c_str(), u8"mecho ", 6))) {
    size_t start = line.find_first_not_of(u8" \t\r\n", 6);
    if (room && start != std::u8string::npos) {
      std::u8string mes = line.substr(start);
      trim_string(mes);
      mes += u8"\n";
      room->SendOut(0, 0, mes.c_str(), mes.c_str(), nullptr, nullptr);
    }
  }

  else if (!strncmp(line.c_str(), u8"send ", 5)) {
    Object* targ = nullptr;
    char8_t mes[1024] = u8"";
    sscanf(line.c_str() + 5, u8" OBJ:%p %1022[^\n\r]", &targ, mes);
    mes[strlen(mes) + 1] = 0;
    mes[strlen(mes)] = '\n';
    if (targ)
      targ->Send(0, 0, mes);
  }

  else if (!strncmp(line.c_str(), u8"force ", 6)) {
    Object* targ = nullptr;
    char8_t tstr[256] = u8"", tcmd[1024] = u8"";
    if (sscanf(line.c_str() + 6, u8" OBJ:%p %1023[^\n\r]", &targ, tcmd) < 2) {
      if (sscanf(line.c_str() + 6, u8" %255s %1023[^\n\r]", tstr, tcmd) >= 2) {
        targ = room->PickObject(tstr, LOC_NINJA | LOC_INTERNAL);
      }
    }
    if (targ) {
      Mind* amind = nullptr; // Make sure human minds see it!
      std::vector<Mind*> mns = get_human_minds();
      for (auto mn : mns) {
        if (mn->Body() == targ) {
          amind = mn;
          break;
        }
      }
      handle_command(targ, tcmd, amind);
    }
  }

  else if (!strncmp(line.c_str(), u8"wdamage ", 8)) {
    //    fprintf(stderr, CGRN u8"#%d Debug: WDamage '%s'\n" CNRM,
    //	body->Skill(prhash(u8"TBAScript")), line.c_str()
    //	);
    int pos = 0;
    int dam = 0;
    char8_t buf2[256] = {};
    if (sscanf(line.c_str() + 8, u8" %254[^\n\r] %n", buf2, &pos) >= 1) {
      if (!strcmp(buf2, u8"all")) {
        strcpy(buf2, u8"everyone");
      }
      dam = TBAEval(line.c_str() + 8 + pos);
      if (dam > 0)
        dam = (dam + 180) / 100;
      if (dam < 0)
        dam = (dam - 180) / 100;
    }
    auto options = room->Contents();
    for (auto opt : options) {
      if (opt->Matches(buf2)) {
        if (dam > 0) {
          opt->HitMent(1000, dam, 0);
          //	  fprintf(stderr, CGRN u8"#%d Debug: WDamage '%s', %d\n" CNRM,
          //		body->Skill(prhash(u8"TBAScript")), opt->Noun().c_str(), dam
          //		);
        } else if (dam < 0) {
          opt->HealStun(((-dam) + 1) / 2);
          opt->HealPhys(((-dam) + 1) / 2);
          //	  fprintf(stderr, CGRN u8"#%d Debug: WHeal '%s', %d\n" CNRM,
          //		body->Skill(prhash(u8"TBAScript")), opt->Noun().c_str(), ((-dam)+1)/2
          //		);
        }
      }
    }
  }

  else if (!strncmp(line.c_str(), u8"door ", 5)) {
    int rnum, tnum, len;
    char8_t dname[16], xtra;
    const std::u8string args = line.substr(5);
    if (sscanf(args.c_str(), u8"%d %s", &rnum, dname) < 2) {
      fprintf(
          stderr,
          CRED u8"#%d Error: short door command '%s'\n" CNRM,
          body->Skill(prhash(u8"TBAScript")),
          line.c_str());
      return -1;
    }

    // Handle abbreviated standard directions.
    std::u8string dir = dname;
    if (!strncmp(u8"north", dir.c_str(), dir.length()))
      dir = u8"north";
    else if (!strncmp(u8"south", dir.c_str(), dir.length()))
      dir = u8"south";
    else if (!strncmp(u8"east", dir.c_str(), dir.length()))
      dir = u8"east";
    else if (!strncmp(u8"west", dir.c_str(), dir.length()))
      dir = u8"west";
    else if (!strncmp(u8"up", dir.c_str(), dir.length()))
      dir = u8"up";
    else if (!strncmp(u8"down", dir.c_str(), dir.length()))
      dir = u8"down";

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
      fprintf(
          stderr,
          CRED u8"#%d Error: can't find target in '%s'\n" CNRM,
          body->Skill(prhash(u8"TBAScript")),
          line.c_str());
      return -1;
    }

    Object* door = room->PickObject(dir, LOC_NINJA | LOC_INTERNAL);

    if (sscanf(args.c_str(), u8"%*d %*s descriptio%c %n", &xtra, &len) >= 1) {
      //      fprintf(stderr, CGRN u8"#%d Debug: door redesc '%s'\n" CNRM,
      //	body->Skill(prhash(u8"TBAScript")), line.c_str()
      //	);
      if (door)
        door->SetDesc(line.substr(len + 5));
    } else if (sscanf(args.c_str(), u8"%*d %*s flag%c %n", &xtra, &len) >= 1) {
      if (!door) {
        fprintf(
            stderr,
            CRED u8"#%d Error: No %s door to reflag in '%s'\n" CNRM,
            body->Skill(prhash(u8"TBAScript")),
            dir.c_str(),
            line.c_str());
        return -1;
      }
      uint32_t newfl = tba_bitvec(line.substr(len + 5));
      if ((newfl & 0xF) == 0) {
        fprintf(
            stderr,
            CRED u8"#%d Error: bad door reflag (x%X) in '%s'\n" CNRM,
            body->Skill(prhash(u8"TBAScript")),
            newfl,
            line.c_str());
        return -1;
      }
      if (newfl & 1) { // Can Open/Close
        door->SetSkill(prhash(u8"Open"), 1000);
        door->SetSkill(prhash(u8"Closeable"), 1);
        door->SetSkill(prhash(u8"Locked"), 0);
        door->SetSkill(prhash(u8"Lockable"), 1);
        door->SetSkill(prhash(u8"Pickable"), 4);
        //	fprintf(stderr, CGRN u8"#%d Debug: %s door can open/close in
        //'%s'\n" CNRM,
        //		body->Skill(prhash(u8"TBAScript")), dir.c_str(), line.c_str()
        //		);
      }
      if (newfl & 2) { // Closed
        door->SetSkill(prhash(u8"Open"), 0);
        //	fprintf(stderr, CGRN u8"#%d Debug: %s door is closed in '%s'\n"
        // CNRM,
        //		body->Skill(prhash(u8"TBAScript")), dir.c_str(), line.c_str()
        //		);
      }
      if (newfl & 4) { // Locked
        door->SetSkill(prhash(u8"Locked"), 1);
        door->SetSkill(prhash(u8"Lockable"), 1);
        door->SetSkill(prhash(u8"Pickable"), 4);
        //	fprintf(stderr, CGRN u8"#%d Debug: %s door is locked in '%s'\n"
        // CNRM,
        //		body->Skill(prhash(u8"TBAScript")), dir.c_str(), line.c_str()
        //		);
      }
      if (newfl & 8) { // Pick-Proof
        door->SetSkill(prhash(u8"Pickable"), 1000);
        //	fprintf(stderr, CGRN u8"#%d Debug: %s door is pick-proof in
        //'%s'\n" CNRM,
        //		body->Skill(prhash(u8"TBAScript")), dir.c_str(), line.c_str()
        //		);
      }
    } else if (sscanf(args.c_str(), u8"%*d %*s nam%c %n", &xtra, &len) >= 1) {
      //      fprintf(stderr, CGRN u8"#%d Debug: door rename '%s'\n" CNRM,
      //	body->Skill(prhash(u8"TBAScript")), line.c_str()
      //	);
      if (door) {
        std::u8string newname = door->ShortDescS();
        size_t end = newname.find(u8"(");
        if (end != std::u8string::npos) {
          end = newname.find_last_not_of(u8" \t", end - 1);
          if (end != std::u8string::npos)
            newname = newname.substr(0, end);
        }
        door->SetShortDesc(newname + u8" (" + line.substr(len + 5) + u8")");
      }
    } else if (sscanf(args.c_str(), u8"%*d %*s room %d", &tnum) == 1) {
      //      fprintf(stderr, CGRN u8"#%d Debug: door relink '%s'\n" CNRM,
      //	body->Skill(prhash(u8"TBAScript")), line.c_str()
      //	);
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
        fprintf(
            stderr,
            CRED u8"#%d Error: can't find dest in '%s'\n" CNRM,
            body->Skill(prhash(u8"TBAScript")),
            line.c_str());
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
      odoor->AddAct(act_t::SPECIAL_MASTER, door);
    } else if (sscanf(args.c_str(), u8"%*d %*s key %d", &tnum) == 1) {
      if (!door) {
        fprintf(
            stderr,
            CRED u8"#%d Error: No %s door to re-key in '%s'\n" CNRM,
            body->Skill(prhash(u8"TBAScript")),
            dir.c_str(),
            line.c_str());
        return -1;
      }
      door->SetSkill(prhash(u8"Lockable"), 1);
      door->SetSkill(prhash(u8"Key"), 1000000 + tnum);
      if (door->Skill(prhash(u8"Pickable")) < 1)
        door->SetSkill(prhash(u8"Pickable"), 4);
      //      fprintf(stderr, CGRN u8"#%d Debug: %s door re-keyed (%d) in '%s'\n"
      //      CNRM,
      //	body->Skill(prhash(u8"TBAScript")), dir.c_str(), tnum, line.c_str()
      //	);
    } else if (sscanf(args.c_str(), u8"%*d %*s purg%c", &xtra) == 1) {
      //      fprintf(stderr, CGRN u8"#%d Debug: door purge '%s'\n" CNRM,
      //	body->Skill(prhash(u8"TBAScript")), line.c_str()
      //	);
      if (door)
        door->Recycle();
    } else {
      fprintf(
          stderr,
          CRED u8"#%d Error: bad door command '%s'\n" CNRM,
          body->Skill(prhash(u8"TBAScript")),
          line.c_str());
      return -1;
    }
  }

  else if (!strncmp(line.c_str(), u8"transport ", 10)) {
    int dnum;
    int nocheck = 0;
    char8_t buf2[256];
    if (sscanf(line.c_str() + 10, u8"%s %d", buf2, &dnum) != 2) {
      fprintf(
          stderr,
          CRED u8"#%d Error: Bad teleport line '%s'\n" CNRM,
          body->Skill(prhash(u8"TBAScript")),
          line.c_str());
      return -1;
    }
    if (!strcmp(buf2, u8"all")) {
      strcpy(buf2, u8"everyone");
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
            if (targ->Matches(buf2)) {
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
      Object* targ = room->PickObject(fmt::format(u8"all's {}", buf2), LOC_INTERNAL);
      if (targ) {
        targ->Parent()->RemoveLink(targ);
        targ->SetParent(dest);
      }
    }
    //    fprintf(stderr, CGRN u8"#%d Debug: Transport line: '%s'\n" CNRM,
    //	body->Skill(prhash(u8"TBAScript")), line.c_str()
    //	);
  }

  else if (!strncmp(line.c_str(), u8"purge", 5)) {
    Object* targ = nullptr;
    sscanf(line.c_str() + 5, u8" OBJ:%p", &targ);
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
      fprintf(
          stderr,
          CRED u8"#%d Error: Bad purge target '%s'\n" CNRM,
          body->Skill(prhash(u8"TBAScript")),
          line.c_str());
      return 1;
    }
    if (!body) { // No longer connected, must have purged self or parent.
      return -1;
    }
  }

  else if (!strncmp(line.c_str(), u8"load ", 5)) {
    int valnum, params, tbatype, mask = 0;
    act_t loc = act_t::NONE;
    char8_t buf2[256] = u8"";
    char8_t targ[256] = u8"";
    char8_t where[256] = u8"";
    Object* dest = ovars[u8"self"];
    Object* item = nullptr;
    params = sscanf(line.c_str() + 5, u8" %s %d %s %s", buf2, &valnum, targ, where);
    tbatype = ascii_tolower(buf2[0]);
    if ((params != 2 && params != 4) || (tbatype != 'o' && tbatype != 'm')) {
      fprintf(
          stderr,
          CRED u8"#%d Error: Nonsensical script load '%s'\n" CNRM,
          body->Skill(prhash(u8"TBAScript")),
          line.c_str());
      return -1;
    }
    Object* src = room->World();
    if (tbatype == 'o') {
      src = src->PickObject(u8"tbamud object room", LOC_NINJA | LOC_INTERNAL);
      if (src == nullptr) {
        fprintf(
            stderr,
            CRED u8"#%d Error: Can't find Object room '%s'\n" CNRM,
            body->Skill(prhash(u8"TBAScript")),
            line.c_str());
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
        fprintf(
            stderr,
            CRED u8"#%d Error: Can't find MOB room '%s'\n" CNRM,
            body->Skill(prhash(u8"TBAScript")),
            line.c_str());
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
      fprintf(
          stderr,
          CRED u8"#%d Error: Failed to find item '%s'\n" CNRM,
          body->Skill(prhash(u8"TBAScript")),
          line.c_str());
      return -1;
    }
    if (params > 2) {
      //      fprintf(stderr, CGRN u8"#%d Debug: (%s) '%s'\n" CNRM,
      //	body->Skill(prhash(u8"TBAScript")), targ, line.c_str());
      dest = room->PickObject(targ, LOC_NINJA | LOC_INTERNAL);
    }
    if (!dest) {
      fprintf(
          stderr,
          CRED u8"#%d Error: Can't find target in '%s'\n" CNRM,
          body->Skill(prhash(u8"TBAScript")),
          line.c_str());
      delete item;
      return -1;
    }
    if (strcmp(u8"rfinger", where) == 0 || strcmp(u8"1", where) == 0) {
      mask = item->Skill(prhash(u8"Wearable on Right Finger"));
      loc = act_t::WEAR_RFINGER;
    } else if (strcmp(u8"lfinger", where) == 0 || strcmp(u8"2", where) == 0) {
      mask = item->Skill(prhash(u8"Wearable on Left Finger"));
      loc = act_t::WEAR_LFINGER;
    } else if (strcmp(u8"neck1", where) == 0 || strcmp(u8"3", where) == 0) {
      mask = item->Skill(prhash(u8"Wearable on Neck"));
      mask |= item->Skill(prhash(u8"Wearable on Collar"));
      loc = act_t::WEAR_NECK;
    } else if (strcmp(u8"neck2", where) == 0 || strcmp(u8"4", where) == 0) {
      mask = item->Skill(prhash(u8"Wearable on Neck"));
      mask |= item->Skill(prhash(u8"Wearable on Collar"));
      loc = act_t::WEAR_COLLAR;
    } else if (strcmp(u8"body", where) == 0 || strcmp(u8"5", where) == 0) {
      mask = item->Skill(prhash(u8"Wearable on Chest"));
      mask &= item->Skill(prhash(u8"Wearable on Back")); // Both
      loc = act_t::WEAR_CHEST;
    } else if (strcmp(u8"head", where) == 0 || strcmp(u8"6", where) == 0) {
      mask = item->Skill(prhash(u8"Wearable on Head"));
      loc = act_t::WEAR_HEAD;
    } else if (strcmp(u8"legs", where) == 0 || strcmp(u8"7", where) == 0) {
      mask = item->Skill(prhash(u8"Wearable on Left Leg"));
      mask |= item->Skill(prhash(u8"Wearable on Right Leg"));
      loc = act_t::WEAR_LLEG;
    } else if (strcmp(u8"feet", where) == 0 || strcmp(u8"8", where) == 0) {
      mask = item->Skill(prhash(u8"Wearable on Left Foot"));
      mask |= item->Skill(prhash(u8"Wearable on Right Foot"));
      loc = act_t::WEAR_LFOOT;
    } else if (strcmp(u8"hands", where) == 0 || strcmp(u8"9", where) == 0) {
      mask = item->Skill(prhash(u8"Wearable on Left Hand"));
      mask |= item->Skill(prhash(u8"Wearable on Right Hand"));
      loc = act_t::WEAR_LHAND;
    } else if (strcmp(u8"arms", where) == 0 || strcmp(u8"10", where) == 0) {
      mask = item->Skill(prhash(u8"Wearable on Left Arm"));
      mask |= item->Skill(prhash(u8"Wearable on Right Arm"));
      loc = act_t::WEAR_LARM;
    } else if (strcmp(u8"shield", where) == 0 || strcmp(u8"11", where) == 0) {
      mask = item->Skill(prhash(u8"Wearable on Shield"));
      loc = act_t::WEAR_SHIELD;
    } else if (strcmp(u8"about", where) == 0 || strcmp(u8"12", where) == 0) {
      mask = item->Skill(prhash(u8"Wearable on Left Shoulder"));
      mask &= item->Skill(prhash(u8"Wearable on Right Shoulder")); // Both
      loc = act_t::WEAR_LSHOULDER;
    } else if (strcmp(u8"waist", where) == 0 || strcmp(u8"13", where) == 0) {
      mask = item->Skill(prhash(u8"Wearable on Waist"));
      loc = act_t::WEAR_WAIST;
    } else if (strcmp(u8"rwrist", where) == 0 || strcmp(u8"14", where) == 0) {
      mask = item->Skill(prhash(u8"Wearable on Right Wrist"));
      loc = act_t::WEAR_RWRIST;
    } else if (strcmp(u8"lwrist", where) == 0 || strcmp(u8"15", where) == 0) {
      mask = item->Skill(prhash(u8"Wearable on Left Wrist"));
      loc = act_t::WEAR_LWRIST;
    } else if (strcmp(u8"wield", where) == 0 || strcmp(u8"16", where) == 0) {
      loc = act_t::WIELD;
    } else if (strcmp(u8"light", where) == 0 || strcmp(u8"0", where) == 0) {
      loc = act_t::HOLD;
    } else if (strcmp(u8"hold", where) == 0 || strcmp(u8"17", where) == 0) {
      loc = act_t::HOLD;
    } else if (strcmp(u8"inv", where) == 0 || strcmp(u8"18", where) == 0) {
    } else if (params > 2) {
      fprintf(
          stderr,
          CRED u8"#%d Error: Unsupported dest '%s'\n" CNRM,
          body->Skill(prhash(u8"TBAScript")),
          line.c_str());
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
  } else if (!strncmp(line.c_str(), u8"dg_cast '", 9)) {
    auto splen = line.find_first_of(u8"'", 9);
    if (splen != std::u8string::npos) {
      auto spell = tba_spellconvert(line.substr(9, splen - 9));
      // fprintf(stderr, CBLU u8"Cast[Acid]: %s\n" CNRM, spell.c_str());
      // fprintf(stderr, CYEL u8"Cast[TBA]: %s\n" CNRM, line.substr(9, splen - 9).c_str());
      ovars[u8"self"]->SetSkill(spell + u8" Spell", 5);
      std::u8string cline = u8"shout " + spell;
      if (splen + 1 < line.length()) {
        Object* targ;
        if (sscanf(line.c_str() + splen + 1, u8" OBJ:%p", &targ) > 0) {
          ovars[u8"self"]->AddAct(act_t::POINT, targ);
        }
      }
      cline += u8";cast " + spell + u8";point";
      handle_command(ovars[u8"self"], cline.c_str());
    } else {
      fprintf(stderr, CRED u8"Error: Bad casting command: '%s'\n" CNRM, line.c_str());
    }
  } else if (!strncmp(line.c_str(), u8"case ", 5)) {
    // Ignore these, as we only hit them here when when running over them
  } else if (!strncmp(line.c_str(), u8"default", 7)) {
    // Ignore these, as we only hit them here when when running over them
  } else if (!strncmp(line.c_str(), u8"end", 3)) {
    // Ignore these, as we only hit them here when we're running inside if
  } else if (!strncmp(line.c_str(), u8"nop ", 4)) {
    // Ignore these, as the varsub should have done all that's needed
  } else if (!strncmp(line.c_str(), u8"done", 4)) {
    // Means we should be within a while(), pop up a level.
    if (spos_s.size() < 2) {
      fprintf(
          stderr,
          CRED u8"#%d Error: Not in while/switch, but '%s'\n" CNRM,
          body->Skill(prhash(u8"TBAScript")),
          line.c_str());
      return -1;
    }
    spos_s.pop_back();
  } else if (!strncmp(line.c_str(), u8"return ", 7)) {
    int retval = TBAEval(line.c_str() + 7);
    if (retval == 0) {
      status = 1; // Set special state
    }
  } else if (!strncmp(line.c_str(), u8"halt", 4)) {
    return -1;
  }

  // Player commands different between Acid and TBA, requiring arguments
  else if (!strncmp(line.c_str(), u8"give ", 5)) {
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
        fprintf(
            stderr,
            CRED u8"#%d Error: Told just '%s'\n" CNRM,
            body->Skill(prhash(u8"TBAScript")),
            line.c_str());
        return -1;
      }
    } else {
      fprintf(
          stderr,
          CRED u8"#%d Error: Told just '%s'\n" CNRM,
          body->Skill(prhash(u8"TBAScript")),
          line.c_str());
      return -1;
    }
  }

  // Player commands Acid shares with TBA, requiring arguments
  else if (
      com == COM_SAY || com == COM_SHOUT || com == COM_EMOTE || com == COM_LOCK ||
      com == COM_UNLOCK || com == COM_OPEN || com == COM_CLOSE || com == COM_GET ||
      com == COM_DROP || com == COM_WEAR || com == COM_WIELD || com == COM_FOLLOW) {
    // if (body->Skill(prhash(u8"TBAScript")) >= 5034503 && body->Skill(prhash(u8"TBAScript")) <=
    // 5034507)
    //  fprintf(
    //      stderr,
    //      CMAG u8"[#%d] Running command: '%s'\n" CNRM,
    //      body->Skill(prhash(u8"TBAScript")),
    //      line.c_str());

    size_t stuff = line.find_first_of(u8" ");
    if (stuff != std::u8string::npos) {
      stuff = line.find_first_not_of(u8" \t\r\n", stuff);
    }
    if (stuff != std::u8string::npos) {
      handle_command(ovars[u8"self"], line.c_str());
    } else {
      fprintf(
          stderr,
          CRED u8"#%d Error: Told just '%s'\n" CNRM,
          body->Skill(prhash(u8"TBAScript")),
          line.c_str());
      return -1;
    }
  }

  // Player commands Acid shares with TBA, not requiring arguments
  else if (
      com == COM_NORTH || com == COM_SOUTH || com == COM_EAST || com == COM_WEST || com == COM_UP ||
      com == COM_DOWN || com == COM_SLEEP || com == COM_REST || com == COM_WAKE ||
      com == COM_STAND || com == COM_SIT || com == COM_LIE || com == COM_LOOK || com == COM_FLEE ||
      com >= COM_SOCIAL) {
    handle_command(ovars[u8"self"], line.c_str());
  }

  // Trigger-Supported (only) commands (not shared with real acid commands).
  else if (com == COM_NONE && handle_command(ovars[u8"self"], line.c_str()) != 1) {
    // Do Nothing, as handle_command already did it.
  }

  else {
    fprintf(
        stderr,
        CRED u8"#%d Error: Gibberish script line '%s'\n" CNRM,
        body->Skill(prhash(u8"TBAScript")),
        line.c_str());
    return -1;
  }
  return 0;
}

static const std::u8string dirnames[4] = {u8"north", u8"south", u8"east", u8"west"};
uint32_t items[8] = {
    prhash(u8"Food"),
    prhash(u8"Hungry"), // Order is Most to Least Important
    prhash(u8"Rest"),
    prhash(u8"Tired"),
    prhash(u8"Fun"),
    prhash(u8"Bored"),
    prhash(u8"Stuff"),
    prhash(u8"Needy")};
bool Mind::Think(int istick) {
  if (type == mind_t::NPC) {
    if (body->HasSkill(prhash(u8"Day Worker")) || body->HasSkill(prhash(u8"Night Worker"))) {
      int day = body->World()->Skill(prhash(u8"Day Length"));
      int time = body->World()->Skill(prhash(u8"Day Time"));
      int late = body->Skill(prhash(u8"Day Worker"));
      if (body->HasSkill(prhash(u8"Night Worker"))) {
        time = (time + (day / 2)) % day;
        late = body->Skill(prhash(u8"Night Worker"));
      }
      int early = late / 5;
      if (late < 0) { // Early Risers
        early = -late;
        late = early / 5;
      }

      // Work Day (6AM-6PM)
      if (time > day / 4 - early && time < 3 * day / 4 + late) {
        if (body->Pos() == pos_t::LIE) {
          handle_command(body, u8"wake;stand");
        }
        if (body->Parent() != body->ActTarg(act_t::SPECIAL_WORK)) {
          if (!svars.contains(u8"path")) {
            handle_command(body, u8"say 'Time to head to work!");
            auto path = body->Parent()->DirectionsTo(body->ActTarg(act_t::SPECIAL_WORK));
            svars[u8"path"] = path;
          } else if (svars[u8"path"].length() < 1) {
            svars.erase(u8"path");
          } else {
            auto old = body->Parent();
            handle_command(body, fmt::format(u8"{}", svars[u8"path"][0]));
            if (old != body->Parent()) { // Actually went somewhere
              svars[u8"path"] = svars[u8"path"].substr(1);
            } else {
              // TODO: Open/Unlock Doors, Reroute, Etc.
              std::u8string_view dir = u8"north";
              std::u8string_view odir = u8"south";
              if (svars[u8"path"][0] == 's') {
                dir = u8"south";
                odir = u8"north";
              } else if (svars[u8"path"][0] == 'e') {
                dir = u8"east";
                odir = u8"west";
              } else if (svars[u8"path"][0] == 'w') {
                dir = u8"west";
                odir = u8"east";
              } else if (svars[u8"path"][0] == 'u') {
                dir = u8"up";
                odir = u8"down";
              } else if (svars[u8"path"][0] == 'd') {
                dir = u8"down";
                odir = u8"up";
              }

              handle_command(body, fmt::format(u8"open {0};{0}", dir));
              if (old != body->Parent()) { // Actually went somewhere
                handle_command(body, fmt::format(u8"close {0}", odir));
                svars[u8"path"] = svars[u8"path"].substr(1);
              } else {
                handle_command(body, fmt::format(u8"unlock {0};open {0};{0}", dir));
                if (old != body->Parent()) { // Actually went somewhere
                  handle_command(body, fmt::format(u8"close {0};lock {0}", odir));
                  svars[u8"path"] = svars[u8"path"].substr(1);
                } else {
                  // Object* door = PickObject(dir, LOC_NEARBY);
                  // if (door) {
                  //  TODO: Actually figure out what's wrong.
                  //}
                }
              }
            }
          }
        } else if (!body->IsAct(act_t::WORK)) {
          body->AddAct(act_t::WORK);
          body->Parent()->SendOut(ALL, 0, u8";s starts on the daily work.\n", u8"", body, nullptr);
        }

        // Night (10PM-4AM)
      } else if (time > 11 * day / 12 - early || time < day / 6 + late) {
        if (body->Parent() != body->ActTarg(act_t::SPECIAL_HOME)) {
          if (!svars.contains(u8"path")) {
            handle_command(body, u8"say Time to head home!");
            auto path = body->Parent()->DirectionsTo(body->ActTarg(act_t::SPECIAL_HOME));
            svars[u8"path"] = path;
          } else if (svars[u8"path"].length() < 1) {
            svars.erase(u8"path");
          } else {
            auto old = body->Parent();
            handle_command(body, fmt::format(u8"{}", svars[u8"path"][0]));
            if (old != body->Parent()) { // Actually went somewhere
              svars[u8"path"] = svars[u8"path"].substr(1);
            } else {
              // TODO: Open/Unlock Doors, Reroute, Etc.
              std::u8string_view dir = u8"north";
              std::u8string_view odir = u8"south";
              if (svars[u8"path"][0] == 's') {
                dir = u8"south";
                odir = u8"north";
              } else if (svars[u8"path"][0] == 'e') {
                dir = u8"east";
                odir = u8"west";
              } else if (svars[u8"path"][0] == 'w') {
                dir = u8"west";
                odir = u8"east";
              } else if (svars[u8"path"][0] == 'u') {
                dir = u8"up";
                odir = u8"down";
              } else if (svars[u8"path"][0] == 'd') {
                dir = u8"down";
                odir = u8"up";
              }

              handle_command(body, fmt::format(u8"open {0};{0}", dir));
              if (old != body->Parent()) { // Actually went somewhere
                handle_command(body, fmt::format(u8"close {0}", odir));
                svars[u8"path"] = svars[u8"path"].substr(1);
              } else {
                handle_command(body, fmt::format(u8"unlock {0};open {0};{0}", dir));
                if (old != body->Parent()) { // Actually went somewhere
                  handle_command(body, fmt::format(u8"close {0};lock {0}", odir));
                  svars[u8"path"] = svars[u8"path"].substr(1);
                } else {
                  // Object* door = PickObject(dir, LOC_NEARBY);
                  // if (door) {
                  //  TODO: Actually figure out what's wrong.
                  //}
                }
              }
            }
          }
        } else if (body->Pos() != pos_t::LIE) {
          handle_command(body, u8"lie;sleep");
        }

        // Evening (6PM-10PM)
      } else if (time > day / 2) {
        if (body->IsAct(act_t::WORK)) {
          body->StopAct(act_t::WORK);
          body->Parent()->SendOut(
              ALL, 0, u8";s finishes up with the daily work.\n", u8"", body, nullptr);
          handle_command(body, u8"say I am hungry");
        }

        // Morning (4AM-6AM)
      } else {
        if (body->Pos() != pos_t::STAND) {
          handle_command(body, u8"wake;stand");
          handle_command(body, u8"say I am thirsty");
        }
      }
    }

  } else if (type == mind_t::MOB) {
    if (body->Skill(prhash(u8"Personality")) & 1) { // Group Mind
      //      body->TryCombine();	// I AM a group, after all.
      int qty = body->Skill(prhash(u8"Quantity")), req = -1;
      if (qty < 1)
        qty = 1;
      for (int item = 0; item < 8; item += 2) {
        if (body->Parent()->Skill(items[item])) {
          int val = body->Skill(items[item + 1]);
          val -= body->Parent()->Skill(items[item]) * qty * 100;
          if (val < 0)
            val = 0;
          body->SetSkill(items[item + 1], val);
        } else {
          if ((item & 2) == 0) { // Food & Fun grow faster
            body->SetSkill(items[item + 1], body->Skill(items[item + 1]) + qty * 2);
          } else {
            body->SetSkill(items[item + 1], body->Skill(items[item + 1]) + qty);
          }
        }
        if (req < 0 && body->Skill(items[item + 1]) >= 10000) {
          req = item;
        }
      }
      if (req >= 0) { // Am I already getting what I most need?
        if (body->Parent()->Skill(items[req]) > 0)
          req = -1;
      }
      if (req >= 0) {
        std::vector<std::u8string> dirs;
        for (int i = 0; i < 4; ++i) {
          Object* dir = body->PickObject(dirnames[i], LOC_NEARBY);
          if (dir && dir->Skill(prhash(u8"Open")) > 0) {
            dirs.push_back(dirnames[i]);
          }
        }

        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(dirs.begin(), dirs.end(), g);

        int orig = body->Skill(items[req + 1]);
        int leave = orig / 10000;
        if (dirs.size() > 0 && leave >= qty) {
          body->BusyFor(0, dirs.front());
        } else {
          if (dirs.size() > 0) {
            Object* trav = body->Split(leave);
            trav->SetSkill(items[req + 1], leave * 10000);
            body->SetSkill(items[req + 1], orig - leave * 10000);
            trav->BusyFor(0, dirs.front());
          }
          body->BusyFor(8000 + (rand() & 0xFFF));
        }
      } else {
        body->BusyFor(8000 + (rand() & 0xFFF));
      }

      //      if(body->Skill(prhash(u8"Personality")) & 2) {		// Punk
      //	//body->BusyFor(500, u8"say Yo yo!");
      //	}
      //      else if(body->Skill(prhash(u8"Personality")) & 4) {		// Normal
      //	//body->BusyFor(500, u8"say How do you do?");
      //	}
      //      else if(body->Skill(prhash(u8"Personality")) & 8) {		// Rich
      //	//body->BusyFor(500, u8"say Hi.");
      //	}
    }

  } else if (type == mind_t::TBATRIG) {
    if (body && body->Parent() && spos_s.size() > 0) {
      //      fprintf(stderr, CGRN u8"#%d Debug: Running Trigger.\n" CNRM,
      //	body->Skill(prhash(u8"TBAScript"))
      //	);
      ovars[u8"self"] = body->Parent();
      ovars[u8"context"] = ovars[u8"self"]; // Initial global var context

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
        if (Body()->NumMinds() < 2) { // Not Already Being Covered
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
  } else if (type == mind_t::TBAMOB) {
    if ((!body) || (istick >= 0 && body->StillBusy()))
      return true;

    // Temporary
    if (body && body->ActTarg(act_t::WEAR_SHIELD) && (!body->IsAct(act_t::HOLD))) {
      std::u8string command =
          fmt::format(u8"hold OBJ:{}", static_cast<void*>(body->ActTarg(act_t::WEAR_SHIELD)));
      body->BusyFor(500, command.c_str());
      return true;
    } else if (
        body && body->ActTarg(act_t::WEAR_SHIELD) && body->ActTarg(act_t::HOLD) &&
        body->ActTarg(act_t::WEAR_SHIELD) != body->ActTarg(act_t::HOLD)) {
      Object* targ = body->ActTarg(act_t::HOLD);
      if (body->Stash(targ, 0)) {
        if (body->Parent())
          body->Parent()->SendOut(ALL, 0, u8";s stashes ;s.\n", u8"", body, targ);
        std::u8string command =
            fmt::format(u8"hold OBJ:{}", static_cast<void*>(body->ActTarg(act_t::WEAR_SHIELD)));
        body->BusyFor(500, command.c_str());
      } else {
        // fprintf(stderr, u8"Warning: %s can't use his shield!\n", body->Noun().c_str());
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
          std::u8string command = fmt::format(u8"attack OBJ:{}", static_cast<void*>(other));
          body->BusyFor(500, command.c_str());
          // fprintf(stderr, u8"%s: Tried '%s'\n", body->ShortDescC(), command.c_str());
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
          std::u8string command = fmt::format(u8"attack OBJ:{}", static_cast<void*>(other));
          body->BusyFor(500, command.c_str());
          // fprintf(stderr, u8"%s: Tried '%s'\n", body->ShortDescC(), command.c_str());
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
              fmt::format(u8"call ALARM; attack OBJ:{}", static_cast<void*>(other));
          body->BusyFor(500, command.c_str());
          // fprintf(stderr, u8"%s: Tried '%s'\n", body->ShortDescC(), command.c_str());
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

void Mind::SetSpecialPrompt(const std::u8string& newp) {
  prompt = newp;
  UpdatePrompt();
}

std::u8string Mind::SpecialPrompt() const {
  return prompt;
}

Mind* new_mind(mind_t tp, Object* obj, Object* obj2, Object* obj3, std::u8string text) {
  Mind* m = new Mind(tp);
  if (tp == mind_t::TBATRIG && obj) {
    m->SetTBATrigger(obj, obj2, obj3, text);
  } else if (obj) {
    m->Attach(obj);
  }
  return m;
}

int new_trigger(int msec, Object* obj, Object* tripper, std::u8string text) {
  return new_trigger(msec, obj, tripper, nullptr, text);
}

static bool in_new_trigger = false;
int new_trigger(int msec, Object* obj, Object* tripper, Object* targ, std::u8string text) {
  if ((!obj) || (!(obj->Parent())))
    return 0;

  int status = 0;
  Mind* m = new_mind(mind_t::TBATRIG, obj, tripper, targ, text);
  if (msec == 0 && !in_new_trigger) { // Triggers can not immediately trigger triggers
    in_new_trigger = true;
    if (!m->Think(1)) {
      status = m->Status();
      delete m;
    }
    in_new_trigger = false;
  } else {
    m->Suspend(msec);
  }
  return status;
}

std::vector<std::pair<int64_t, Mind*>> Mind::waiting;
void Mind::Suspend(int msec) {
  // if(body && body->Skill(prhash(u8"TBAScript")) >= 5034503 && body->Skill(prhash(u8"TBAScript"))
  // <= 5034507)
  //  fprintf(stderr, CBLU u8"Suspended(%d): '%d'\n" CNRM, msec,
  //  body->Skill(prhash(u8"TBAScript")));

  int64_t when = current_time + int64_t(msec) * int64_t(1000);

  auto itr = waiting.begin();
  for (; itr != waiting.end() && itr->second != this; ++itr) {
  }
  if (itr != waiting.end()) {
    itr->first = when;
  } else {
    waiting.emplace_back(std::make_pair(when, this));
  }
}

void Mind::Resume() {
  // Sort by time-to-run, so earlier events happen first
  std::sort(waiting.begin(), waiting.end());

  // Quickly purge the u8"never" entries off the end, now that it's been sorted
  auto itr = waiting.begin();
  for (; itr != waiting.end() && itr->first != std::numeric_limits<int64_t>::max(); ++itr) {
  }
  if (itr != waiting.end()) {
    waiting.erase(itr, waiting.end());
  }

  // Now fire off those at the beginning that should have already happened
  itr = waiting.begin();
  for (; itr != waiting.end() && itr->first <= current_time; ++itr) {
    if (!itr->second->Think(0)) {
      itr->first = std::numeric_limits<int64_t>::max();
      delete itr->second;
    }
  }
}

int Mind::Status() const {
  return status;
}

void Mind::ClearStatus() {
  status = 0;
}

static const std::u8string blank = u8"";
void Mind::SetSVar(const std::u8string& var, const std::u8string& val) {
  svars[var] = val;
}

void Mind::ClearSVar(const std::u8string& var) {
  svars.erase(var);
}

const std::u8string& Mind::SVar(const std::u8string& var) const {
  if (svars.count(var) <= 0)
    return blank;
  return svars.at(var);
}

int Mind::IsSVar(const std::u8string& var) const {
  return (svars.count(var) > 0);
}

const std::map<std::u8string, std::u8string> Mind::SVars() const {
  return svars;
}

void Mind::SetSVars(const std::map<std::u8string, std::u8string>& sv) {
  svars = sv;
}
