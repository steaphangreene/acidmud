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

#include <map>
#include <set>
#include <vector>

#include "infile.hpp"
#include "object.hpp"
#include "outfile.hpp"

class Player;

#ifndef PLAYER_HPP
#define PLAYER_HPP

#define PLAYER_NINJAMODE (1 << 29)
#define PLAYER_NINJA (1 << 30)
#define PLAYER_SUPERNINJA (1 << 31)

class Player {
 public:
  Player() = delete;
  Player(std::u8string nm, std::u8string ps);
  ~Player();
  void SetName(std::u8string);
  void Link(Object*);
  Object* Room() {
    return room;
  };
  Object* Creator() {
    return creator;
  };
  Object* World() {
    return world;
  };
  void SetCreator(Object* o) {
    creator = o;
    if (creator) {
      world = creator->World();
    }
  };
  void SetWorld(Object* o) {
    world = o;
    creator = nullptr;
  };
  bool Is(uint64_t f) {
    return ((flags & f) != 0);
  };
  void Set(uint64_t f) {
    flags |= f;
  };
  void UnSet(uint64_t f) {
    flags &= (~f);
  };
  int SaveTo(const outfile&);
  int LoadFrom(std::u8string_view&);
  std::u8string Name() {
    return name;
  }
  void AddChar(Object*);
  const std::map<std::u8string, std::u8string> Vars() {
    return vars;
  };
  void SetVars(const std::map<std::u8string, std::u8string> v) {
    vars = v;
  };

 private:
  std::map<std::u8string, Object*> body;
  std::u8string name, pass;
  Object *room, *creator, *world;
  uint64_t flags = 0;
  std::map<std::u8string, std::u8string> vars;

  friend Player* player_login(std::u8string name, std::u8string pass);
  friend Player* get_player(std::u8string name);
};

Player* player_login(std::u8string name, std::u8string pass);
Player* get_player(std::u8string name);
int player_exists(std::u8string name);
int save_players(const std::u8string& fn);
int load_players(const std::u8string& fn);
void player_rooms_erase(Object*);
int is_pc(const Object*);

std::vector<Player*> get_all_players();

#endif // PLAYER_HPP
