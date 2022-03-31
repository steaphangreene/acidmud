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
#include <filesystem>
#include <limits>
#include <random>
#include <string>

// Replace with C++20 std::ranges, when widely available
#include <range/v3/algorithm.hpp>
namespace rng = ranges;

#include "color.hpp"
#include "commands.hpp"
#include "global.hpp"
#include "log.hpp"
#include "mind.hpp"
#include "net.hpp"
#include "object.hpp"
#include "player.hpp"
#include "properties.hpp"
#include "utils.hpp"

static std::random_device rd;
static std::mt19937 gen(rd());

static const std::u8string dirnames[6] =
    {u8"north", u8"south", u8"east", u8"west", u8"up", u8"down"};

std::map<std::u8string, std::u8string> Mind::cvars;

Mind::~Mind() {
  if (type == mind_t::REMOTE)
    close_socket(pers);
  type = mind_t::NONE;
  if (log >= 0)
    close(log);
  log = -1;

  auto itr = waiting.begin();
  for (; itr != waiting.end() && itr->second.get() != this; ++itr) {
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

  if (log >= 0)
    return;

  static unsigned long lognum = 0;
  std::u8string logname = fmt::format(u8"logs/{:08X}.log", lognum);
  while (std::filesystem::exists(logname)) {
    ++lognum;
    logname = fmt::format(u8"logs/{:08X}.log", lognum);
  }
  log = open(
      reinterpret_cast<const char*>(logname.c_str()),
      O_WRONLY | O_CREAT | O_TRUNC | O_SYNC,
      S_IRUSR | S_IWUSR);
}

void Mind::SetMob() {
  type = mind_t::MOB;
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
  if (!Owner()) {
    SetPrompt(pers, u8"Player Name: ");
    if (pname.length() >= 1)
      SetPrompt(pers, u8"Password: ");
  } else if (prompt.length() > 0) {
    SetPrompt(pers, fmt::format(u8"{}> ", prompt));
  } else if (Body()) {
    SetPrompt(
        pers,
        fmt::format(
            u8"[{}][{}] {}> ",
            sevs_p[std::min(10, Body()->Phys())],
            sevs_s[std::min(10, Body()->Stun())],
            Body()->Name()));
  } else
    SetPrompt(pers, u8"No Character> ");
}

bool Mind::Send(const std::u8string_view& mes) {
  if (type == mind_t::REMOTE) {
    SendOut(pers, mes);
  } else if (type == mind_t::NPC) {
    if (body->HasTag(crc32c(u8"guard")) && !body->IsAct(act_t::SLEEP)) {
      if (mes.starts_with(u8"From ") && mes.contains(u8" you hear someone shout '") &&
          (mes.contains(u8"HELP") || mes.contains(u8"ALARM") || mes.contains(u8"TO ARMS") ||
           mes.contains(u8"GUARDS") || mes.contains(u8"AAAAAAAA"))) {
        std::u8string_view dir = mes.substr(5);
        skipspace(dir);
        dir = getgraph(dir);

        Object* door = body->PickObject(dir, LOC_NEARBY);

        if (door && door->ActTarg(act_t::SPECIAL_LINKED)) {
          svars[u8"path"] = dir.substr(0, 1);
        }
      } else if (!body->IsAct(act_t::FIGHT)) { // Guards help others in fights
        // FIXME: Ability to 'hear' directly who is fighting
        auto folks = body->PickObjects(u8"everyone", LOC_NEARBY);
        for (auto folk : folks) {
          auto other = folk->ActTarg(act_t::FIGHT);
          if (other) {
            if ((!folk->HasTag(crc32c(u8"citizen"))) && other->HasTag(crc32c(u8"citizen"))) {
              handle_command(body, fmt::format(u8"attack obj:{}", reinterpret_cast<void*>(folk)));
            } else if ((!folk->HasTag(crc32c(u8"guard"))) && other->HasTag(crc32c(u8"guard"))) {
              handle_command(body, fmt::format(u8"attack obj:{}", reinterpret_cast<void*>(folk)));
            }
          }
        }
      }
    } else if (body->HasTag(crc32c(u8"soldier"))) {
      if (mes.starts_with(u8"From ") && mes.contains(u8" you hear someone shout '") &&
          (mes.contains(u8"TO ARMS"))) {
        std::u8string_view dir = mes.substr(5);
        skipspace(dir);
        dir = getgraph(dir);

        Object* door = body->PickObject(dir, LOC_NEARBY);

        if (door && door->ActTarg(act_t::SPECIAL_LINKED)) {
          svars[u8"path"] = dir.substr(0, 1);
        }
      } else if (!body->IsAct(act_t::FIGHT)) { // Soldiers help their own (and guards) in fights
        // FIXME: Ability to 'hear' directly who is fighting
        auto folks = body->PickObjects(u8"everyone", LOC_NEARBY);
        for (auto folk : folks) {
          auto other = folk->ActTarg(act_t::FIGHT);
          if (other) {
            if ((!folk->HasTag(crc32c(u8"soldier"))) && other->HasTag(crc32c(u8"soldier"))) {
              handle_command(body, fmt::format(u8"attack obj:{}", reinterpret_cast<void*>(folk)));
            } else if ((!folk->HasTag(crc32c(u8"guard"))) && other->HasTag(crc32c(u8"guard"))) {
              handle_command(body, fmt::format(u8"attack obj:{}", reinterpret_cast<void*>(folk)));
            }
          }
        }
      }
    }
  } else if (type == mind_t::MOB) {
    // ...these folks aren't big listeners....
  } else if (type == mind_t::TBAMOB) {
    return TBAMOBSend(mes);
  } else if (type == mind_t::SYSTEM) {
    std::u8string_view inmes = mes;
    trim_string(inmes);

    std::u8string newmes = fmt::format(
        CMAG u8"{}|{}" CNRM u8": {}\n",
        (body) ? body->Name() : u8"?",
        (body) ? body->ShortDesc() : u8"?",
        inmes);
    for (auto& chr : newmes) {
      if (chr == '\n' || chr == '\r')
        chr = ' ';
    }
    newmes.back() = '\n';

    write(pers, newmes.data(), newmes.length());
  }
  return true;
}

void Mind::SetPName(const std::u8string_view& pn) {
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

void Mind::SetPPass(const std::u8string_view& ppass) {
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
  player->Room()->SendDesc(shared_from_this());
  player->Room()->SendContents(shared_from_this());
}

void Mind::SetPlayer(const std::u8string_view& pn) {
  if (player_exists(pn)) {
    pname = pn;
    player = get_player(pname);
    svars = player->Vars();
  }
}

std::u8string Mind::Tactics(int phase) const {
  if (type == mind_t::TBAMOB) {
    return TBAMOBTactics(phase);
  }
  return u8"attack";
}

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
    // Currently Fighting!
    if (body->IsAct(act_t::FIGHT)) {
      if (body->HasTag(crc32c(u8"soldier"))) {
        handle_command(body, u8"call TO ARMS");
      } else if (body->HasTag(crc32c(u8"guard"))) {
        handle_command(body, u8"call ALARM");
      } else if (body->HasTag(crc32c(u8"citizen"))) {
        handle_command(body, u8"call GUARDS");
      } else if (body->HasTag(crc32c(u8"civilian"))) {
        handle_command(body, u8"call HELP");
      } else {
        handle_command(body, u8"call AAAAAAAAHHHHHH");
      }
      return true;
    }

    // Soldiers and Guards On The Job Wield Weapons
    if ((body->HasTag(crc32c(u8"guard")) || body->HasTag(crc32c(u8"soldier"))) &&
        body->IsAct(act_t::WORK)) {
      if (!body->IsAct(act_t::WIELD)) {
        handle_command(body, u8"wield");
        return true;
      }
    } else if (body->IsAct(act_t::WIELD) && (!body->IsAct(act_t::FIGHT))) {
      // Otherwise, If Not Fighting, Put Weapon Away
      handle_command(body, u8"unwield");
      return true;
    }

    // Currently Travelling
    if (svars.contains(u8"path")) {
      if (svars[u8"path"].length() < 1) {
        svars.erase(u8"path");
      } else if (body->Room() != body->Parent()) { // Get out of bed?
        handle_command(body, u8"leave");
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
      return true;
    }

    // Work-Schedule NPCs
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
        if (body->Room() != body->Parent()) { // Get out of bed?
          handle_command(body, u8"wake;stand;leave");
        } else if (body->Pos() == pos_t::LIE) {
          handle_command(body, u8"wake;stand");
        } else if (body->ActTarg(act_t::WORK)) {
          auto project = body->ActTarg(act_t::WORK);
          if (project->HasSkill(prhash(u8"Incomplete"))) {
            ContinueWorkOn(project);
            return true;
          }
        } else if (body->Room() != body->ActTarg(act_t::SPECIAL_WORK)->Room()) {
          if (!svars.contains(u8"path")) {
            auto path =
                body->Room()->DirectionsTo(body->ActTarg(act_t::SPECIAL_WORK)->Room(), body);
            svars[u8"path"] = path;
          }
        } else if (!body->IsAct(act_t::WORK)) {
          body->AddAct(act_t::WORK);
          body->Parent()->SendOut(ALL, 0, u8";s starts on the daily work.\n", u8"", body, nullptr);
        }

        // Night (10PM-4AM)
      } else if (time > 11 * day / 12 - early || time < day / 6 + late) {
        if (body->Room() != body->ActTarg(act_t::SPECIAL_HOME)->Room()) {
          if (!svars.contains(u8"path")) {
            auto path =
                body->Room()->DirectionsTo(body->ActTarg(act_t::SPECIAL_HOME)->Room(), body);
            svars[u8"path"] = path;
          }
        } else if (body->Pos() != pos_t::LIE) {
          if (body->Room() != body->ActTarg(act_t::SPECIAL_HOME)) { // Need to get in bed?
            handle_command(
                body,
                fmt::format(
                    u8"enter obj:{};lie;sleep",
                    reinterpret_cast<void*>(body->ActTarg(act_t::SPECIAL_HOME))));
          } else {
            handle_command(body, u8"lie;sleep");
          }
        }

        // Evening (6PM-10PM)
      } else if (time > day / 2) {
        if (body->IsAct(act_t::WORK)) {
          body->StopAct(act_t::WORK);
          body->Parent()->SendOut(
              ALL, 0, u8";s finishes up with the daily work.\n", u8"", body, nullptr);
        }

        // Morning (4AM-6AM)
      } else {
        if (body->Room() != body->Parent()) { // Get out of bed?
          handle_command(body, u8"wake;stand;leave");
        } else if (body->Pos() != pos_t::STAND) {
          handle_command(body, u8"wake;stand");
        }
      }
    }

    if (body->IsAct(act_t::WORK)) {
      // Artisans Make Stuff
      if (body->HasTag(crc32c(u8"master"))) {
        StartNewProject();

        // Guards and Soldiers Patrol
      } else if (body->HasTag(crc32c(u8"guard")) || body->HasTag(crc32c(u8"soldier"))) {
        if (std::uniform_int_distribution(0, 999)(gen) < 55) { // 5.5% Chance
          std::vector<std::u8string_view> options;
          auto conns = body->Room()->Connections(body);
          std::u8string_view area = u8"";
          if (body->ActTarg(act_t::SPECIAL_WORK)
                  ->Room()) { // Only patrol the street you patrol, etc.
            area = body->ActTarg(act_t::SPECIAL_WORK)->Room()->ShortDesc();
          }
          if (area.length() > 0 && body->Room()->ShortDesc() != area) { // Out of position, go back.
            svars[u8"path"] =
                body->Room()->DirectionsTo(body->ActTarg(act_t::SPECIAL_WORK)->Room(), body);
          } else {
            for (int d = 0; d < 6; ++d) {
              if (conns[d] && (area.length() == 0 || conns[d]->ShortDesc() == area)) {
                options.push_back(dirnames[d]);
              }
            }
            if (options.size() > 1) {
              shuffle(options.begin(), options.end(), gen);
            }
            if (options.size() > 0) {
              svars[u8"path"] = options.front().substr(0, 1);
            }
          }
        }
      }
    }

  } else if (type == mind_t::MOB) {
    if (body->Skill(prhash(u8"Personality")) & 1) { // Group Mind
      //      body->TryCombine();	// I AM a group, after all.
      int qty = body->Quantity(), req = -1;
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

        std::shuffle(dirs.begin(), dirs.end(), gen);

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
    return TBATriggerThink(istick);
  } else if (type == mind_t::TBAMOB) {
    return TBAMOBThink(istick);
  }
  return true;
}

void Mind::SetSpecialPrompt(const std::u8string& newp) {
  prompt = newp;
  UpdatePrompt();
}

std::u8string Mind::SpecialPrompt() const {
  return prompt;
}

std::vector<std::pair<int64_t, std::shared_ptr<Mind>>> Mind::waiting;
void Mind::Suspend(int msec) {
  int64_t when = current_time + int64_t(msec) * int64_t(1000);

  auto itr = waiting.begin();
  for (; itr != waiting.end() && itr->second.get() != this; ++itr) {
  }
  if (itr != waiting.end()) {
    itr->first = when;
  } else {
    waiting.emplace_back(std::make_pair(when, shared_from_this()));
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
      if (itr->second->Body()) {
        itr->second->Body()->Detach(itr->second);
      }
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

static std::shared_ptr<Mind> mob_mind = std::make_shared<Mind>(mind_t::MOB);
std::shared_ptr<Mind> get_mob_mind() {
  return mob_mind;
}
