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

#include "color.hpp"
#include "log.hpp"
#include "net.hpp"
#include "player.hpp"
#include "utils.hpp"
#include "version.hpp"

static std::map<std::u8string, Player*> player_list;
static std::set<Player*> non_init;

static const std::u8string salt_char =
    u8"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./";

static std::u8string md5_crypt(const std::u8string_view& pass, const std::u8string_view& insalt) {
  Chocobo1::MD5 md5;

  std::u8string_view salt = insalt;
  if (!salt.starts_with(u8"$1$") || salt.length() < 11 ||
      (salt.length() > 11 && salt[11] != u8'$')) {
    loger(u8"Invalid magic in salt submitted to md5_crypt!\n");
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
  std::u8string hash = fmt::format(u8"{}$", salt);
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

Player::Player(const std::u8string_view& nm, const std::u8string_view& ps) {
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

void Player::SetName(const std::u8string_view& nm) {
  player_list.erase(name);
  name = nm;
  player_list[name] = this;
  room->SetShortDesc(fmt::format(u8"{}'s character room", nm));
  std::u8string desc = u8"Available commands:\n";
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
  room->SetDesc(desc);
}

void Player::Link(Object* obj) {}

void Player::AddChar(Object* ch) {
  room->AddLink(ch);
  creator = ch;
  world = ch->World();
}

int player_exists(const std::u8string_view& name) {
  return player_list.count(std::u8string(name));
}

Player* get_player(const std::u8string_view& name) {
  if (!player_list.contains(std::u8string(name))) {
    return nullptr;
  }
  Player* pl = player_list[std::u8string(name)];
  non_init.erase(pl);

  return pl;
}

Player* player_login(const std::u8string_view& name, const std::u8string_view& pass) {
  if (!player_list.contains(std::u8string(name))) {
    return nullptr;
  }
  Player* pl = player_list[std::u8string(name)];
  std::u8string enpass = md5_crypt(pass, pl->pass);

  // loge(u8"Trying {}:\n{}\n{}\n", name, enpass, pl->pass);

  if (enpass != pl->pass) {
    if (non_init.count(pl)) {
      player_list.erase(std::u8string(name));
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

int Player::LoadFrom(std::u8string_view& fl) {
  SetName(std::u8string(getuntil(fl, '\n')));
  skipspace(fl);
  pass = getuntil(fl, '\n');
  skipspace(fl);

  // Player experience - obsolete, ignored
  nextchar(fl); // Skip Colon
  nextnum(fl); // Skip Number

  char8_t delim = nextchar(fl);
  while (delim == ';') {
    nextnum(fl);
    // Do nothing - ignore these
    delim = nextchar(fl);
  }
  if (delim == ':') {
    flags = nexthex(fl);
  }

  while (nextchar(fl) == ':') {
    AddChar(getbynum(nextnum(fl)));
  }

  // Won't be present previous to v0x02, but that's ok, will work fine anyway
  while (nextchar(fl) == ';') {
    auto varname = getuntil(fl, ':');
    nextchar(fl); // Skip the ':'
    vars[std::u8string(varname)] = getuntil(fl, ';');
  }
  return 0;
}

int load_players(const std::u8string_view& fn) {
  loge(u8"Loading Players.\n");

  infile file(fn);
  if (!file)
    return -1;
  std::u8string_view fl = file.all();

  unsigned int ver = nexthex(fl);
  skipspace(fl);
  int num = nextnum(fl);
  skipspace(fl);

  loge(u8"Loading Players: {},{}\n", ver, num);

  for (int ctr = 0; ctr < num; ++ctr) {
    Player* pl = new Player(u8";;TEMP;;", u8";;TEMP;;");
    non_init.erase(pl);
    pl->LoadFrom(fl);
    // loge(u8"Loaded Player: {}\n", pl->Name());
  }
  return 0;
}

int Player::SaveTo(outfile& fl) {
  fl.append(u8"{}\n{}\n", name, pass);

  fl.append(u8":0"); // Player experience, obsolete, always zero

  fl.append(u8":{:X}", flags);
  fl.append(u8"{}\n", room->WriteContents());

  for (auto var : vars) {
    fl.append(u8";{}:{}", var.first, var.second);
  }
  fl.append(u8"\n");
  return 0;
}

int save_players(const std::u8string_view& fn) {
  outfile fl(fn);
  if (!fl)
    return -1;

  fl.append(u8"{:08X}\n", CurrentVersion.savefile_version_player);

  fl.append(u8"{}\n", player_list.size() - non_init.size());

  for (auto pl : player_list) {
    if (non_init.count(pl.second) == 0) {
      if (pl.second->SaveTo(fl))
        return -1;
    }
  }
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

std::vector<Player*> get_all_players() {
  std::vector<Player*> ret;
  for (auto pl : player_list) {
    if (pl.second->Room()) {
      ret.push_back(pl.second);
    }
  }
  return ret;
}
