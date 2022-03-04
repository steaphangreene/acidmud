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

#include <array>
#include <set>
#include <vector>

#include "md5.hpp"

#include "cchar8.hpp"
#include "color.hpp"
#include "net.hpp"
#include "player.hpp"
#include "version.hpp"

static std::map<std::u8string, Player*> player_list;
static std::set<Player*> non_init;

static const std::u8string salt_char =
    u8"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./";

static std::u8string md5_crypt(std::u8string pass, std::u8string salt) {
  Chocobo1::MD5 md5;

  if (!salt.starts_with(u8"$1$") || salt.length() < 11 ||
      (salt.length() > 11 && salt[11] != u8'$')) {
    fprintf(stderr, CRED u8"Invalid magic in salt submitted to md5_crypt!\n" CNRM);
    return u8"";
  }
  salt = salt.substr(0, 11);

  // Initial Hash
  md5.addData(pass);
  md5.addData(salt.substr(3));
  md5.addData(pass);
  md5.finalize();
  auto alternate = md5.toArray();

  { // Shake
    md5.reset();
    md5.addData(pass);
    md5.addData(salt);
    std::vector<char8_t> sub(alternate.begin(), alternate.begin() + pass.length());
    md5.addData(sub);
    for (auto bit = pass.length(); bit != 0; bit >>= 1) {
      if (bit & 1) {
        md5.addData(u8"\0", 1);
      } else {
        md5.addData(pass.substr(0, 1));
      }
    }
  }
  md5.finalize();
  auto intermediate = md5.toArray();

  // Rattle
  for (int i = 0; i < 1000; ++i) {
    md5.reset();
    if (i % 2 == 0) {
      md5.addData(intermediate);
    }
    if (i % 2 != 0) {
      md5.addData(pass);
    }
    if (i % 3 != 0) {
      md5.addData(salt.substr(3));
    }
    if (i % 7 != 0) {
      md5.addData(pass);
    }
    if (i % 2 == 0) {
      md5.addData(pass);
    }
    if (i % 2 != 0) {
      md5.addData(intermediate);
    }
    md5.finalize();
    intermediate = md5.toArray();
  }

  // Roll
  std::array<char8_t, 18> final = {{
      0,
      0,
      intermediate[11],
      intermediate[4],
      intermediate[10],
      intermediate[5],
      intermediate[3],
      intermediate[9],
      intermediate[15],
      intermediate[2],
      intermediate[8],
      intermediate[14],
      intermediate[1],
      intermediate[7],
      intermediate[13],
      intermediate[0],
      intermediate[6],
      intermediate[12],
  }};

  // Final Encoding
  std::u8string hash = salt + u8"$";
  static const char8_t* cb64 = u8"./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
  for (int off = 15; off >= 0; off -= 3) {
    hash.push_back(cb64[final[off + 2] & 0x3F]);
    hash.push_back(cb64[(final[off + 1] << 2 | final[off + 2] >> 6) & 0x3F]);
    if (off != 0) {
      hash.push_back(cb64[(final[off] << 4 | final[off + 1] >> 4) & 0x3F]);
      hash.push_back(cb64[final[off] >> 2]);
    }
  }

  return hash;
}

Player::Player(std::u8string nm, std::u8string ps) {
  flags = 0;

  room = new Object();
  creator = nullptr;
  world = nullptr;

  SetName(nm);
  if (ps[0] == '$' && ps[1] == '1' && ps[2] == '$') {
    pass = ps;
  } else {
    std::u8string salt = u8"$1$", app = u8" ";
    for (int ctr = 0; ctr < 8; ++ctr) {
      app[0] = salt_char[rand() & 63];
      salt += app;
    }
    pass = md5_crypt(ps, salt);
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

void Player::SetName(std::u8string nm) {
  player_list.erase(name);
  name = nm;
  player_list[name] = this;
  std::u8string desc = nm + u8"'s character room";
  room->SetShortDesc(desc.c_str());
  desc = u8"Available commands:\n";
  desc += u8"     " CMAG u8"look" CNRM u8": Show all your existing characters.\n";
  desc += u8"     " CMAG u8"world" CNRM u8": Show list of currently available worlds.\n";
  desc +=
      u8"     " CMAG u8"world" CNRM u8" " CGRN u8"<world>" CNRM u8": Select a world to make a new character in.\n";
  desc +=
      u8"     " CMAG u8"enter" CNRM u8" " CGRN u8"<character_name>" CNRM u8": Enter a finished character.\n";
  desc +=
      u8"     " CMAG u8"select" CNRM u8" " CGRN u8"<character_name>" CNRM u8": Select an unfinished character.\n";
  desc +=
      u8"     " CMAG u8"newcharacter" CNRM u8" " CGRN u8"<character_name>" CNRM u8": Create a new character.\n";
  desc +=
      u8"     " CMAG u8"raise" CNRM u8" " CGRN u8"<stat>" CNRM u8": Raise a stat of the currently selected character.\n";
  desc += u8"     " CMAG u8"randomize" CNRM
          u8": Randomly spend all remaining points of current "
          u8"character.\n";
  desc +=
      u8"     " CMAG u8"help" CNRM u8" " CGRN u8"<topic>" CNRM u8": Get more info (try 'help commands').\n";
  desc += u8"   Here are all of your current characters:";
  room->SetDesc(desc.c_str());
}

void Player::Link(Object* obj) {}

void Player::AddChar(Object* ch) {
  room->AddLink(ch);
  creator = ch;
  world = ch->World();
}

int player_exists(std::u8string name) {
  return player_list.count(name);
}

Player* get_player(std::u8string name) {
  if (!player_list.count(name))
    return nullptr;

  Player* pl = player_list[name];
  non_init.erase(pl);

  return pl;
}

Player* player_login(std::u8string name, std::u8string pass) {
  if (!player_list.count(name))
    return nullptr;

  Player* pl = player_list[name];
  std::u8string enpass = md5_crypt(pass, pl->pass);

  // fprintf(stderr, u8"Trying %s:\n%s\n%s\n", name.c_str(), enpass.c_str(), pl->pass.c_str());

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

static char8_t buf[65536];
int Player::LoadFrom(FILE* fl) {
  memset(buf, 0, 65536);
  fscanf(fl, u8"%s\n", buf);
  SetName(buf);
  memset(buf, 0, 65536);
  fscanf(fl, u8"%s\n", buf);
  pass = buf;

  // Player experience - obsolete, ignored
  fscanf(fl, u8":%*d");
  while (fscanf(fl, u8";%*d") > 0) {
    // Do nothing - ignore these
  }

  fscanf(fl, u8":%lX\n", &flags);

  int num;
  while (fscanf(fl, u8":%d\n", &num) > 0) {
    AddChar(getbynum(num));
  }

  // Won't be present previous to v0x02, but that's ok, will work fine anyway
  while (fscanf(fl, u8"\n;%32767[^:]:%32767[^; \t\n\r]", buf, buf + 32768) >= 2) {
    vars[buf] = (buf + 32768);
  }
  return 0;
}

int load_players(const std::u8string& fn) {
  fprintf(stderr, u8"Loading Players.\n");

  FILE* fl = fopen(fn.c_str(), u8"r");
  if (!fl)
    return -1;

  int num;
  unsigned int ver;
  fscanf(fl, u8"%X\n%d\n", &ver, &num);

  fprintf(stderr, u8"Loading Players: %d,%d\n", ver, num);

  for (int ctr = 0; ctr < num; ++ctr) {
    Player* pl = new Player(u8";;TEMP;;", u8";;TEMP;;");
    non_init.erase(pl);
    pl->LoadFrom(fl);
    // fprintf(stderr, u8"Loaded Player: %s\n", pl->Name());
  }

  fclose(fl);
  return 0;
}

int Player::SaveTo(FILE* fl) {
  fprintf(fl, u8"%s\n%s\n", name.c_str(), pass.c_str());

  fprintf(fl, u8":0"); // Player experience, obsolete, always zero

  fprintf(fl, u8":%lX", flags);
  room->WriteContentsTo(fl);
  fprintf(fl, u8"\n");

  for (auto var : vars) {
    fprintf(fl, u8";%s:%s", var.first.c_str(), var.second.c_str());
  }
  fprintf(fl, u8"\n");
  return 0;
}

int save_players(const std::u8string& fn) {
  FILE* fl = fopen(fn.c_str(), u8"w");
  if (!fl)
    return -1;

  fprintf(fl, u8"%.8X\n", CurrentVersion.savefile_version_player);

  fprintf(fl, u8"%d\n", (int)(player_list.size() - non_init.size()));

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
  fprintf(stderr, u8"Warning: %s called, but not implemented yet!\n", __PRETTY_FUNCTION__);
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
