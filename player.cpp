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

#include <set>

#include <crypt.h>
#include <cstdlib>
#include <cstring>

#include "color.hpp"
#include "net.hpp"
#include "player.hpp"
#include "version.hpp"

static std::map<std::string, Player*> player_list;
static std::set<Player*> non_init;

static const std::string salt_char =
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./";

Player::Player(std::string nm, std::string ps) {
  flags = 0;

  room = new Object();
  creator = nullptr;
  world = nullptr;

  SetName(nm);
  if (ps[0] == '$' && ps[1] == '1' && ps[2] == '$') {
    pass = ps;
  } else {
    std::string salt = "$1$", app = " ";
    for (int ctr = 0; ctr < 8; ++ctr) {
      app[0] = salt_char[rand() & 63];
      salt += app;
    }
    pass = crypt(ps.c_str(), salt.c_str());
  }
  player_list[name] = this;
  non_init.insert(this);
}

Player::~Player() {
  notify_player_deleted(this);
  player_list.erase(name);
  non_init.erase(this);
  creator = nullptr;
  world = nullptr;
  if (room)
    delete room;
}

void Player::SetName(std::string nm) {
  player_list.erase(name);
  name = nm;
  player_list[name] = this;
  std::string desc = nm + "'s character room";
  room->SetShortDesc(desc.c_str());
  desc = "Available commands:\n";
  desc += "     " CMAG "look" CNRM ": Show all your existing characters.\n";
  desc += "     " CMAG "world" CNRM ": Show list of currently available worlds.\n";
  desc += "     " CMAG "world" CNRM " " CGRN "<world>" CNRM
          ": Select a world to make a new character in.\n";
  desc +=
      "     " CMAG "enter" CNRM " " CGRN "<character_name>" CNRM ": Enter a finished character.\n";
  desc += "     " CMAG "select" CNRM " " CGRN "<character_name>" CNRM
          ": Select an unfinished character.\n";
  desc += "     " CMAG "newcharacter" CNRM " " CGRN "<character_name>" CNRM
          ": Create a new character.\n";
  desc += "     " CMAG "raise" CNRM " " CGRN "<stat>" CNRM
          ": Raise a stat of the currently selected character.\n";
  desc += "     " CMAG "randomize" CNRM
          ": Randomly spend all remaining points of current "
          "character.\n";
  desc +=
      "     " CMAG "help" CNRM " " CGRN "<topic>" CNRM ": Get more info (try 'help commands').\n";
  desc += "   Here are all of your current characters:";
  room->SetDesc(desc.c_str());
}

void Player::Link(Object* obj) {}

void Player::AddChar(Object* ch) {
  room->AddLink(ch);
  creator = ch;
  world = ch->World();
}

int player_exists(std::string name) {
  return player_list.count(name);
}

Player* get_player(std::string name) {
  if (!player_list.count(name))
    return nullptr;

  Player* pl = player_list[name];
  non_init.erase(pl);

  return pl;
}

Player* player_login(std::string name, std::string pass) {
  if (!player_list.count(name))
    return nullptr;

  Player* pl = player_list[name];
  std::string enpass = crypt(pass.c_str(), pl->pass.c_str());

  //  fprintf(stderr, "Trying %s:%s\n", name.c_str(), enpass.c_str());

  if (enpass != pl->pass) {
    if (non_init.count(pl)) {
      player_list.erase(name);
      non_init.erase(pl);
    }
    return nullptr;
  }

  non_init.erase(pl);
  if ((player_list.size() - non_init.size()) == 1) {
    pl->Set(PLAYER_NINJA);
    pl->Set(PLAYER_SUPERNINJA);
  }
  return pl;
}

static char buf[65536];
int Player::LoadFrom(FILE* fl) {
  memset(buf, 0, 65536);
  fscanf(fl, "%s\n", buf);
  SetName(buf);
  memset(buf, 0, 65536);
  fscanf(fl, "%s\n", buf);
  pass = buf;

  // Player experience - obsolete, ignored
  fscanf(fl, ":%*d");
  while (fscanf(fl, ";%*d") > 0) {
    // Do nothing - ignore these
  }

  fscanf(fl, ":%lX\n", &flags);

  int num;
  while (fscanf(fl, ":%d\n", &num) > 0) {
    AddChar(getbynum(num));
  }

  // Won't be present previous to v0x02, but that's ok, will work fine anyway
  while (fscanf(fl, "\n;%32767[^:]:%32767[^; \t\n\r]", buf, buf + 32768) >= 2) {
    vars[buf] = (buf + 32768);
  }
  return 0;
}

int load_players(const std::string& fn) {
  fprintf(stderr, "Loading Players.\n");

  FILE* fl = fopen(fn.c_str(), "r");
  if (!fl)
    return -1;

  int num;
  unsigned int ver;
  fscanf(fl, "%X\n%d\n", &ver, &num);

  fprintf(stderr, "Loading Players: %d,%d\n", ver, num);

  for (int ctr = 0; ctr < num; ++ctr) {
    Player* pl = new Player(";;TEMP;;", ";;TEMP;;");
    non_init.erase(pl);
    pl->LoadFrom(fl);
    // fprintf(stderr, "Loaded Player: %s\n", pl->Name());
  }

  fclose(fl);
  return 0;
}

int Player::SaveTo(FILE* fl) {
  fprintf(fl, "%s\n%s\n", name.c_str(), pass.c_str());

  fprintf(fl, ":0"); // Player experience, obsolete, always zero

  fprintf(fl, ":%lX", flags);
  room->WriteContentsTo(fl);
  fprintf(fl, "\n");

  for (auto var : vars) {
    fprintf(fl, ";%s:%s", var.first.c_str(), var.second.c_str());
  }
  fprintf(fl, "\n");
  return 0;
}

int save_players(const std::string& fn) {
  FILE* fl = fopen(fn.c_str(), "w");
  if (!fl)
    return -1;

  fprintf(fl, "%.8X\n", CurrentVersion.savefile_version_player);

  fprintf(fl, "%d\n", (int)(player_list.size() - non_init.size()));

  for (auto pl : player_list) {
    if (non_init.count(pl.second) == 0) {
      if (pl.second->SaveTo(fl))
        return -1;
    }
  }

  fclose(fl);
  return 0;
}

void player_rooms_erase(Object* obj) {
  for (auto pl : player_list) {
    if (pl.second->Creator() == obj)
      pl.second->SetCreator(nullptr);
    pl.second->Room()->RemoveLink(obj);
  }
}

int is_pc(const Object* obj) {
  for (auto pl : player_list) {
    if (pl.second->Room() && pl.second->Room()->Contains(obj)) {
      return 1;
    }
  }
  return 0;
}

std::vector<Player*> get_current_players() {
  fprintf(stderr, "Warning: %s called, but not implemented yet!\n", __PRETTY_FUNCTION__);
  std::vector<Player*> ret;
  return ret;
}

std::vector<Player*> get_all_players() {
  std::vector<Player*> ret;
  for (auto pl : player_list) {
    if (pl.second->Room()) {
      ret.push_back(pl.second);
    }
  }
  return ret;
}
