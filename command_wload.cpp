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

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <random>

#include "cchar8.hpp"
#include "color.hpp"
#include "commands.hpp"
#include "log.hpp"
#include "mind.hpp"
#include "npc.hpp"
#include "object.hpp"
#include "properties.hpp"
#include "utils.hpp"

// From: https://gist.github.com/Brennall/b9c3a0202eb11c5cfd54868c5752012a
static const std::vector<std::u8string> dwarf_first_names[2] = {
    {
        u8"Anbera",    u8"Artin",    u8"Audhild",  u8"Balifra",  u8"Barbena",  u8"Bardryn",
        u8"Bolhild",   u8"Dagnal",   u8"Dafifi",   u8"Delre",    u8"Diesa",    u8"Hdeth",
        u8"Eridred",   u8"Falkrunn", u8"Fallthra", u8"Finelien", u8"Gillydd",  u8"Gunnloda",
        u8"Gurdis",    u8"Helgret",  u8"Helja",    u8"Hlin",     u8"llde",     u8"Jarana",
        u8"Kathra",    u8"Kilia",    u8"Kristryd", u8"Liftrasa", u8"Marastyr", u8"Mardred",
        u8"Morana",    u8"Nalaed",   u8"Nora",     u8"Nurkara",  u8"Orifi",    u8"Ovina",
        u8"Riswynn",   u8"Sannl",    u8"Therlin",  u8"Thodris",  u8"Torbera",  u8"Tordrid",
        u8"Torgga",    u8"Urshar",   u8"Valida",   u8"Vistra",   u8"Vonana",   u8"Werydd",
        u8"Whurd red", u8"Yurgunn",
    },
    {
        u8"Adrik",   u8"Alberich", u8"Baern",   u8"Barendd",  u8"Beloril", u8"Brottor", u8"Dain",
        u8"Dalgal",  u8"Darrak",   u8"Delg",    u8"Duergath", u8"Dworic",  u8"Eberk",   u8"Einkil",
        u8"Elaim",   u8"Erias",    u8"Fallond", u8"Fargrim",  u8"Gardain", u8"Gilthur", u8"Gimgen",
        u8"Gimurt",  u8"Harbek",   u8"Kildrak", u8"Kilvar",   u8"Morgran", u8"Morkral", u8"Nalral",
        u8"Nordak",  u8"Nuraval",  u8"Oloric",  u8"Olunt",    u8"Osrik",   u8"Oskar",   u8"Rangrim",
        u8"Reirak",  u8"Rurik",    u8"Taklinn", u8"Thoradin", u8"Thorin",  u8"Thradal", u8"Tordek",
        u8"Traubon", u8"Travok",   u8"Ulfgar",  u8"Uraim",    u8"Veit",    u8"Vonbin",  u8"Vondal",
        u8"Whurbin",
    }};

// From: https://gist.github.com/Brennall/b9c3a0202eb11c5cfd54868c5752012a
static const std::vector<std::u8string> dwarf_last_names = {
    u8"Aranore",    u8"Balderk",      u8"Battlehammer", u8"Bigtoe",      u8"Bloodkith",
    u8"Bofdarm",    u8"Brawnanvil",   u8"Brazzik",      u8"Broodfist",   u8"Burrowfound",
    u8"Caebrek",    u8"Daerdahk",     u8"Dankil",       u8"Daraln",      u8"Deepdelver",
    u8"Durthane",   u8"Eversharp",    u8"FaHack",       u8"Fire-forge",  u8"Foamtankard",
    u8"Frostbeard", u8"Glanhig",      u8"Goblinbane",   u8"Goldfinder",  u8"Gorunn",
    u8"Graybeard",  u8"Hammerstone",  u8"Helcral",      u8"Holderhek",   u8"Ironfist",
    u8"Loderr",     u8"Lutgehr",      u8"Morigak",      u8"Orcfoe",      u8"Rakankrak",
    u8"Ruby-Eye",   u8"Rumnaheim",    u8"Silveraxe",    u8"Silverstone", u8"Steelfist",
    u8"Stoutale",   u8"Strakeln",     u8"Strongheart",  u8"Thrahak",     u8"Torevir",
    u8"Torunn",     u8"Trollbleeder", u8"Trueanvil",    u8"Trueblood",   u8"Ungart",
};

static std::random_device rd;
static std::mt19937 gen(rd());

static Object* new_object(Object* parent) {
  Object* body = new Object(parent);
  auto obj_id = body->World()->Skill(prhash(u8"Last Object ID")) + 1;
  body->World()->SetSkill(prhash(u8"Last Object ID"), obj_id);
  body->SetSkill(prhash(u8"Object ID"), obj_id);
  return body;
}
std::multimap<std::u8string, std::pair<std::u8string, Object*>> zone_links;

static NPCType npc_dwarf(
    u8"a dwarf",
    u8"{He} looks pissed.",
    u8"",
    u8"MF",
    {9, 4, 6, 4, 9, 4},
    {10, 7, 11, 8, 18, 8},
    100,
    500);

static bool load_map(Object* world, Mind* mind, const std::filesystem::directory_entry& ent) {
  auto filename = ent.path().u8string();

  if (filename.empty()) {
    mind->Send(u8"You need to specify the filename of the datafile!\n");
    return false;
  }

  std::u8string file(ent.file_size(), 0);
  std::ifstream def_file(ent.path(), std::ios::in | std::ios::binary);
  if (!def_file.is_open()) {
    mind->Send(u8"Can't find definition file '{}'!\n", filename);
    return false;
  }
  def_file.read(reinterpret_cast<char*>(file.data()), ent.file_size());
  def_file.close();

  std::u8string name = u8"Unknown Land";
  uint8_t lower_level = 0;
  uint8_t upper_level = 1;

  std::map<char8_t, std::vector<std::u8string>> rooms;
  std::map<char8_t, std::u8string> doors;
  std::map<char8_t, std::u8string> doorterms;
  std::map<char8_t, bool> from_above;
  std::map<char8_t, bool> closed;
  std::map<char8_t, bool> clear;
  std::map<char8_t, std::u8string> keynames;
  std::map<char8_t, bool> locks;
  std::map<char8_t, bool> locked;
  std::map<char8_t, std::u8string> locktags;
  std::map<char8_t, int32_t> lockid;
  std::map<char8_t, bool> indoors;
  std::map<char8_t, uint8_t> levels;
  std::map<char8_t, std::vector<bool>> stairblocked;
  std::map<char8_t, std::vector<std::u8string>> stairnames;
  std::map<char8_t, std::vector<std::u8string>> empnames;
  std::map<char8_t, std::vector<std::uniform_int_distribution<int>>> empnums;
  std::map<char8_t, std::vector<int>> empfloors;
  std::map<char8_t, std::vector<bool>> empnight;
  std::map<char8_t, std::vector<std::u8string>> resnames;
  std::map<char8_t, std::vector<std::uniform_int_distribution<int>>> resnums;
  std::map<char8_t, std::vector<int>> resfloors;
  std::map<std::pair<Object*, std::u8string>, int> beds;
  std::map<Object*, std::vector<std::pair<char8_t, int32_t>>> loc_keys;
  std::map<std::u8string, std::vector<std::pair<char8_t, int32_t>>> asp_keys;
  std::vector<std::u8string> en_links;
  std::vector<std::u8string> en_rooms;
  std::vector<bool> en_indoors;
  char8_t start_symbol = '\0';

  size_t cursor = 0;
  bool in_main_def = false;
  while (!in_main_def) {
    bool parse_error = false;
    std::u8string_view line = next_line(file, cursor);
    if (process(line, u8"name:")) {
      name = line;
    } else if (process(line, u8"world:")) {
      world->SetShortDesc(line);
    } else if (process(line, u8"lower_level:")) {
      lower_level = nextnum(line);
    } else if (process(line, u8"upper_level:")) {
      upper_level = nextnum(line);
    } else if (process(line, u8"building:")) {
      char8_t sym = nextchar(line);
      if (process(line, u8":")) {
        rooms[sym].emplace_back(line);
        indoors[sym] = true;
      }
    } else if (process(line, u8"place:")) {
      char8_t sym = nextchar(line);
      if (process(line, u8":")) {
        rooms[sym].emplace_back(line);
      }
    } else if (process(line, u8"level:")) {
      char8_t sym = nextchar(line);
      if (process(line, u8":")) {
        levels[sym] = nextnum(line);
      }
    } else if (process(line, u8"stair:")) {
      char8_t sym = nextchar(line);
      int floor = 0;
      std::u8string_view stair = u8"a stairway";
      if (process(line, u8".")) {
        floor = nextnum(line);
      } else {
        parse_error = true;
      }
      if (process(line, u8":")) {
        stair = line;
      } else {
        parse_error = true;
      }
      if (!parse_error) {
        if (!stairnames.contains(sym)) {
          stairnames.emplace( // FIXME: This is hard-coded to a max of 10 floors!
              std::make_pair(sym, std::vector<std::u8string>(10, std::u8string(u8"a stairway"))));
          stairblocked.emplace(std::make_pair(sym, std::vector<bool>(10, false)));
        }
        stairnames[sym][floor] = std::u8string(stair);
        stairblocked[sym][floor] = false;
      }
    } else if (process(line, u8"blockedstair:")) {
      char8_t sym = nextchar(line);
      int floor = 0;
      std::u8string_view stair = u8"a stairway";
      if (process(line, u8".")) {
        floor = nextnum(line);
      } else {
        parse_error = true;
      }
      if (process(line, u8":")) {
        stair = line;
      } else {
        parse_error = true;
      }
      if (!parse_error) {
        if (!stairnames.contains(sym)) {
          stairnames.emplace(
              std::make_pair(sym, std::vector<std::u8string>(10, std::u8string(u8"a stairway"))));
          stairblocked.emplace(std::make_pair(sym, std::vector<bool>(10, false)));
        }
        stairnames[sym][floor] = std::u8string(stair);
        stairblocked[sym][floor] = true;
      }
    } else if (process(line, u8"employ:")) {
      char8_t sym = nextchar(line);
      int floor = 0;
      int lnum, hnum;
      if (process(line, u8".")) {
        floor = nextnum(line);
      }
      if (process(line, u8":")) {
        lnum = nextnum(line);
      } else {
        parse_error = true;
      }
      if (process(line, u8"-")) {
        hnum = nextnum(line);
      } else {
        hnum = lnum;
      }
      if (process(line, u8":")) {
        empnames[sym].emplace_back(line);
      } else {
        parse_error = true;
      }
      if (!parse_error) {
        empnums[sym].push_back(std::uniform_int_distribution<int>(lnum, hnum));
        empfloors[sym].push_back(floor);
        empnight[sym].push_back(false);
      }
    } else if (process(line, u8"night:")) {
      char8_t sym = nextchar(line);
      int floor = 0;
      int lnum, hnum;
      if (process(line, u8".")) {
        floor = nextnum(line);
      }
      if (process(line, u8":")) {
        lnum = nextnum(line);
      } else {
        parse_error = true;
      }
      if (process(line, u8"-")) {
        hnum = nextnum(line);
      } else {
        hnum = lnum;
      }
      if (process(line, u8":")) {
        empnames[sym].emplace_back(line);
      } else {
        parse_error = true;
      }
      if (!parse_error) {
        empnums[sym].push_back(std::uniform_int_distribution<int>(lnum, hnum));
        empfloors[sym].push_back(floor);
        empnight[sym].push_back(true);
      }
    } else if (process(line, u8"house:")) {
      char8_t sym = nextchar(line);
      int floor = 0;
      int lnum, hnum;
      if (process(line, u8".")) {
        floor = nextnum(line);
      }
      if (process(line, u8":")) {
        lnum = nextnum(line);
      } else {
        parse_error = true;
      }
      if (process(line, u8"-")) {
        hnum = nextnum(line);
      } else {
        hnum = lnum;
      }
      if (process(line, u8":")) {
        resnames[sym].emplace_back(line);
      } else {
        parse_error = true;
      }
      if (!parse_error) {
        resnums[sym].push_back(std::uniform_int_distribution<int>(lnum, hnum));
        resfloors[sym].push_back(floor);
      }
    } else if (process(line, u8"door:")) {
      char8_t sym = nextchar(line);
      if (process(line, u8":")) {
        doors[sym] = line;
      }
    } else if (process(line, u8"doorterm:")) {
      char8_t sym = nextchar(line);
      if (process(line, u8":")) {
        doorterms[sym] = line;
      }
    } else if (process(line, u8"lock:")) {
      char8_t sym = nextchar(line);
      locks[sym] = true;
      if (process(line, u8":")) {
        locktags[sym] = line;
      }
    } else if (process(line, u8"key:")) {
      char8_t sym = nextchar(line);
      if (process(line, u8":")) {
        keynames[sym] = line;
      }
    } else if (process(line, u8"locked:")) {
      char8_t sym = nextchar(line);
      locked[sym] = true;
    } else if (process(line, u8"closed:")) {
      char8_t sym = nextchar(line);
      closed[sym] = true;
    } else if (process(line, u8"clear:")) {
      char8_t sym = nextchar(line);
      clear[sym] = true;
    } else if (process(line, u8"fromabove:")) {
      char8_t sym = nextchar(line);
      from_above[sym] = true;
    } else if (process(line, u8"en_link:")) {
      en_links.emplace_back(line);
    } else if (process(line, u8"en_place:")) {
      en_rooms.emplace_back(line);
      en_indoors.push_back(false);
    } else if (process(line, u8"en_building:")) {
      en_rooms.emplace_back(line);
      en_indoors.push_back(true);
    } else if (process(line, u8"start:")) {
      start_symbol = nextchar(line);
    } else if (process(line, u8"ascii_map:")) {
      in_main_def = true;
    } else if (line.front() == '#') {
      // This is a comment, so just ignore this line.
    } else {
      parse_error = true;
    }

    if (parse_error) {
      mind->Send(u8"Bad definition file '{}'!\n", filename);
      mind->Send(u8"Read: '{}'!\n", line);
      return false;
    }
  }

  // Load ASCII Map Into A vector<string>, Padding All Sides with Spaces
  std::vector<std::u8string> ascii_map = {u8""};
  size_t max_line_len = 0;
  {
    std::u8string_view line;
    while ((line = next_line(file, cursor)) != u8"") {
      ascii_map.emplace_back(line);
      max_line_len = std::max(max_line_len, ascii_map.back().length());
    }
  }
  ascii_map.emplace_back(u8"");
  for (auto& line : ascii_map) {
    line = fmt::format(u8" {:<{}}", line, max_line_len + 1);
  }

  // Preview ASCII Map To Confirm It Will Not Fail
  for (uint32_t y = 0; y < ascii_map.size(); ++y) {
    for (uint32_t x = 0; x < ascii_map[y].size(); ++x) {
      auto room = ascii_map[y][x];
      if (room == ' ') {
      } else if (rooms.count(room) > 0) { // Defined Room
      } else if (ascii_isdigit(room)) { // Default Room
      } else if (room == '@') { // Zone Entrance
      } else if (
          ascii_isalpha(room) || room == '|' || room == '/' || room == '-' ||
          room == '\\') { // Default Connections
      } else {
        mind->Send(u8"Bad definition file '{}'!\n", filename);
        mind->Send(u8"Read Unknown Character '{}' at ascii_map ({},{})!\n", room, x, y);
        return false;
      }
    }
  }

  // Create New Zone
  Object* zone = new_object(world);
  zone->SetShortDesc(name);
  zone->SetSkill(prhash(u8"Light Source"), 1000);
  zone->SetSkill(prhash(u8"Day Length"), 240);
  zone->SetSkill(prhash(u8"Day Time"), 120);

  // Convert ASCII Map Into AcidMUD Rooms
  struct coord {
    uint32_t x;
    uint32_t y;
    auto operator<=>(const coord&) const = default;
  };
  static const std::u8string floornames[] = {
      u8"the first floor of the ",
      u8"the second floor of the ",
      u8"the third floor of the ",
      u8"the fourth floor of the ",
      u8"the fifth floor of the ",
      u8"the sixth floor of the ",
      u8"the seventh floor of the ",
      u8"the eighth floor of the ",
      u8"the ninth floor of the ",
      u8"the tenth floor of the ",
  };
  std::map<coord, std::vector<Object*>> objs;
  for (uint32_t y = 0; y < ascii_map.size(); ++y) {
    for (uint32_t x = 0; x < ascii_map[y].size(); ++x) {
      auto room = ascii_map[y][x];
      if (room == ' ') {
      } else if (rooms.count(room) > 0) { // Defined Room
        int num_floors = 1;
        if (ascii_isdigit(room)) {
          num_floors = room - '0';
        }
        if (num_floors < 1) {
          num_floors = 1;
        }
        for (int f = 0; f < num_floors; ++f) {
          objs[coord{x, y}].push_back(new_object(zone));
          if (levels.count(room) > 0) {
            objs[coord{x, y}].back()->SetCoords(x, y, levels.at(room) + f);
          } else {
            objs[coord{x, y}].back()->SetCoords(x, y, f);
          }
          if (indoors[room]) {
            objs[coord{x, y}].back()->SetSkill(prhash(u8"Translucent"), 200);
            objs[coord{x, y}].back()->SetSkill(prhash(u8"Light Source"), 100);
          } else {
            objs[coord{x, y}].back()->SetSkill(prhash(u8"Translucent"), 1000);
          }
          if (start_symbol == room) {
            world->AddAct(act_t::SPECIAL_HOME, objs[coord{x, y}].back());
          }
        }
        std::u8string roomname = rooms[room][0];
        if (rooms[room].size() > 1) {
          std::vector<std::u8string> out = {u8""};
          std::sample(rooms[room].begin(), rooms[room].end(), out.begin(), 1, gen);
          roomname = out.front();
        }
        if (objs[coord{x, y}].size() > 1) {
          for (uint32_t f = 0; f < objs[coord{x, y}].size(); ++f) {
            objs[coord{x, y}][f]->SetShortDesc(floornames[f] + roomname);
            objs[coord{x, y}][f]->SetDesc(fmt::format(
                u8"This is a building in {0}, on {1}.  {0} is nice.",
                zone->ShortDesc(),
                world->ShortDesc()));
            if (f > 0) {
              std::u8string_view stairname = u8"a stairway";
              if (stairnames.contains(room)) {
                stairname = stairnames[room][f];
              }
              bool blocked = false;
              if (stairblocked.contains(room)) {
                blocked = stairblocked[room][f];
              }

              // Create the linking stairs
              Object* door1 = new_object(objs[coord{x, y}][f]);
              Object* door2 = new_object(objs[coord{x, y}][f - 1]);
              door1->SetShortDesc(fmt::format(u8"down {}", stairname));
              door2->SetShortDesc(fmt::format(u8"up {}", stairname));
              door1->SetDesc(u8"The floor below.\n");
              door2->SetDesc(u8"The floor above.\n");
              door1->AddAct(act_t::SPECIAL_LINKED, door2);
              door2->AddAct(act_t::SPECIAL_LINKED, door1);
              door1->AddAct(act_t::SPECIAL_MASTER, door2);
              door2->AddAct(act_t::SPECIAL_MASTER, door1);
              door1->SetSkill(prhash(u8"Enterable"), 1);
              door2->SetSkill(prhash(u8"Enterable"), 1);
              if (blocked) {
                door1->SetSkill(prhash(u8"Transparent"), 1000);
                door2->SetSkill(prhash(u8"Transparent"), 1000);
              } else {
                door1->SetSkill(prhash(u8"Open"), 1000);
                door2->SetSkill(prhash(u8"Open"), 1000);
                //  TODO: Closeable, Lockable, Etc., Stairs
                //  door1->SetSkill(prhash(u8"Closeable"), 1);
                //  door2->SetSkill(prhash(u8"Closeable"), 1);
              }
            }
          }
        } else {
          objs[coord{x, y}].back()->SetShortDesc(roomname);
          objs[coord{x, y}].back()->SetDesc(fmt::format(
              u8"This is {0}, on {1}.  {0} is nice.", zone->ShortDesc(), world->ShortDesc()));
        }

        // Load data for housing capacities for these new objects
        if (resnames.count(room) > 0) {
          auto loc = coord{x, y};
          for (size_t f = 0; f < resnames.at(room).size(); ++f) {
            int resfl = resfloors.at(room)[f];
            if (beds.count(std::make_pair(objs[loc][resfl], resnames[room][f])) == 0) {
              beds[std::make_pair(objs[loc][resfl], resnames[room][f])] = resnums.at(room)[f](gen);
            }
          }
        }

      } else if (ascii_isdigit(room)) { // Default Room
        int num_floors = room - '0';
        if (num_floors < 1) {
          num_floors = 1;
        }
        for (int f = 0; f < num_floors; ++f) {
          objs[coord{x, y}].push_back(new_object(zone));
          objs[coord{x, y}].back()->SetCoords(x, y);
          if (indoors[room]) {
            objs[coord{x, y}].back()->SetSkill(prhash(u8"Translucent"), 200);
            objs[coord{x, y}].back()->SetSkill(prhash(u8"Light Source"), 100);
          } else {
            objs[coord{x, y}].back()->SetSkill(prhash(u8"Translucent"), 1000);
          }
          if (start_symbol == room) {
            world->AddAct(act_t::SPECIAL_HOME, objs[coord{x, y}].back());
          }
        }
        if (objs[coord{x, y}].size() > 1) {
          for (uint32_t f = 0; f < objs[coord{x, y}].size(); ++f) {
            objs[coord{x, y}][f]->SetShortDesc(floornames[f] + u8"building");
            if (f > 0) {
              objs[coord{x, y}][f]->Link(
                  objs[coord{x, y}][f - 1],
                  u8"down a stairway",
                  u8"The floor below.\n",
                  u8"up a stairway",
                  u8"The floor above.\n");
            }
          }
        } else {
          objs[coord{x, y}].back()->SetShortDesc(u8"a room");
        }

        // Load data for housing capacities for these new objects
        if (resnames.count(room) > 0) {
          auto loc = coord{x, y};
          for (size_t f = 0; f < resnames.at(room).size(); ++f) {
            int resfl = resfloors.at(room)[f];
            if (beds.count(std::make_pair(objs[loc][resfl], resnames[room][f])) == 0) {
              beds[std::make_pair(objs[loc][resfl], resnames[room][f])] = resnums.at(room)[f](gen);
            }
          }
        }

      } else if (room == '@') { // Zone Entrance
        bool linked = false;
        if (!en_links.empty()) {
          auto ozone = en_links.front();
          if (zone_links.count(name) > 0) {
            for (auto link = zone_links.find(name); link != zone_links.end() && link->first == name;
                 ++link) {
              if (link->second.first == ozone) {
                objs[coord{x, y}].push_back(link->second.second);
                linked = true;
                break;
              }
            }
          }
        }

        Object* entrance;
        if (!linked) { // No object yet, so make one.
          entrance = new_object(zone);
          entrance->SetShortDesc(u8"a generic zone entrance");
          objs[coord{x, y}].push_back(entrance);
          objs[coord{x, y}].back()->SetCoords(x, y);
          if (!en_links.empty()) {
            auto ozone = en_links.front();
            zone_links.insert(std::make_pair(ozone, std::make_pair(name, entrance)));
          }
        } else { // Object already exists, well-defined or not - use it.
          entrance = objs[coord{x, y}].back();
        }

        if (entrance->ShortDesc() == u8"a generic zone entrance") { // Not well-defined yet
          if (!en_indoors.empty() && en_indoors.front()) {
            objs[coord{x, y}].back()->SetSkill(prhash(u8"Translucent"), 200);
            objs[coord{x, y}].back()->SetSkill(prhash(u8"Light Source"), 100);
          } else {
            objs[coord{x, y}].back()->SetSkill(prhash(u8"Translucent"), 1000);
          }
          if (!en_rooms.empty()) {
            objs[coord{x, y}].back()->SetShortDesc(en_rooms.front());
            objs[coord{x, y}].back()->SetDesc(fmt::format(
                u8"This is {0}, on {1}.  {0} is nice.", zone->ShortDesc(), world->ShortDesc()));
          }
          objs[coord{x, y}].back()->SetCoords(x, y);
        }
        if (!en_links.empty()) {
          en_links.erase(en_links.begin());
        }
        if (!en_rooms.empty()) {
          en_rooms.erase(en_rooms.begin());
        }
        if (!en_indoors.empty()) {
          en_indoors.erase(en_indoors.begin());
        }
      }
    }
  }

  // Interconnect rooms in this zone, according to the ASCII map.
  std::map<Object*, uint8_t> connected;
  for (auto obj : objs) {
    auto x = obj.first.x;
    auto y = obj.first.y;

    const std::u8string dirnames[] = {
        u8"south and east",
        u8"south and west",
        u8"north and east",
        u8"north and west",
        u8"south",
        u8"east",
        u8"west",
        u8"north",
    };
    const int32_t off_x[] = {1, -1, 1, -1, 0, 1, -1, 0};
    const int32_t off_y[] = {1, 1, -1, -1, 1, 0, 0, -1};
    const uint8_t dmask[] = {3, 5, 10, 12, 1, 2, 4, 8};
    for (uint8_t dir = 0; dir < 8; ++dir) {
      uint32_t dx = x + off_x[dir];
      uint32_t dy = y + off_y[dir];
      bool is_upper = false;
      bool is_lower = false;
      char8_t door_char = ascii_map[dy][dx];
      if (ascii_islower(door_char)) {
        is_lower = true;
      } else if (ascii_isupper(door_char)) {
        is_upper = true;
      } else if (door_char == '-' || door_char == '/' || door_char == '|' || door_char == '\\') {
        // Connections with no level forcing
      } else {
        if (door_char != ' ') {
          mind->Send(u8"Unrecognized passage symbol '{}'!\n", door_char);
        }
        continue;
      }

      do {
        dx += off_x[dir];
        dy += off_y[dir];
      } while (ascii_isalpha(ascii_map[dy][dx]) || ascii_map[dy][dx] == '-' ||
               ascii_map[dy][dx] == '/' || ascii_map[dy][dx] == '|' || ascii_map[dy][dx] == '\\');

      if (objs.count(coord{dx, dy}) <= 0) {
        mind->Send(u8"Path to nowhere ({},{})->({},{})!\n", x, y, dx, dy);
        continue;
      }

      uint32_t l1 = 0;
      uint32_t l2 = 0;
      auto schr = ascii_map[y][x];
      auto dchr = ascii_map[dy][dx];
      auto& dst = objs[coord{dx, dy}];
      if (is_lower) {
        if (levels[schr] <= lower_level && obj.second.size() + levels[schr] > lower_level) {
          l1 = lower_level - levels[schr];
        } else {
          // mind->Send(u8"Lower link incompatible with origin point '{}'->'{}'!\n", schr, dchr);
          continue;
        }

        if (levels[dchr] <= lower_level && dst.size() + levels[dchr] > lower_level) {
          l2 = lower_level - levels[dchr];
        } else {
          // mind->Send(u8"Lower link incompatible with dest point '{}'->'{}'!\n", schr, dchr);
          continue;
        }

      } else if (is_upper) {
        if (levels[schr] <= upper_level && obj.second.size() + levels[schr] > upper_level) {
          l1 = upper_level - levels[schr];
        } else {
          // mind->Send(u8"Upper link incompatible with origin point '{}'->'{}'!\n", schr, dchr);
          continue;
        }

        if (levels[dchr] <= upper_level && dst.size() + levels[dchr] > upper_level) {
          l2 = upper_level - levels[dchr];
        } else {
          // mind->Send(u8"Upper link incompatible with dest point '{}'->'{}'!\n", schr, dchr);
          continue;
        }
      }

      if (connected[obj.second[l1]] & dmask[dir]) {
        // mind->Send(u8"Direction already (at least partly) defined from here.\n");
        continue;
      }
      if (connected[objs[coord{dx, dy}][l2]] & dmask[dir ^ 3]) {
        // mind->Send(u8"Direction already (at least partly) defined from there.\n");
        continue;
      }

      std::u8string start = u8"A passage ";
      if (doors.count(door_char) > 0) {
        start = fmt::format(u8"A {} to the ", doors[door_char]);
      }
      std::u8string term = u8"door";
      if (doorterms.count(door_char) > 0) {
        term = doorterms[door_char];
      }

      connected[obj.second[l1]] |= dmask[dir];
      connected[objs[coord{dx, dy}][l2]] |= dmask[dir ^ 3];

      // Create the linking doors
      Object* door1 = new_object(obj.second[l1]);
      Object* door2 = new_object(objs[coord{dx, dy}][l2]);
      if (doors.count(door_char) > 0) {
        door1->SetShortDesc(dirnames[dir] + u8" (" + term + u8")");
        door2->SetShortDesc(dirnames[dir ^ 3] + u8" (" + term + u8")");
      } else {
        door1->SetShortDesc(dirnames[dir]);
        door2->SetShortDesc(dirnames[dir ^ 3]);
      }
      door1->SetDesc(start + dirnames[dir] + u8" is here.\n");
      door2->SetDesc(start + dirnames[dir ^ 3] + u8" is here.\n");
      door1->AddAct(act_t::SPECIAL_LINKED, door2);
      door2->AddAct(act_t::SPECIAL_LINKED, door1);
      door1->AddAct(act_t::SPECIAL_MASTER, door2);
      door2->AddAct(act_t::SPECIAL_MASTER, door1);
      door1->SetSkill(prhash(u8"Enterable"), 1);
      door2->SetSkill(prhash(u8"Enterable"), 1);
      if (doors.count(door_char) > 0) {
        door1->SetSkill(prhash(u8"Closeable"), 1);
        door2->SetSkill(prhash(u8"Closeable"), 1);
      }
      if (closed.count(door_char) == 0) {
        door1->SetSkill(prhash(u8"Open"), 1000);
        door2->SetSkill(prhash(u8"Open"), 1000);
      }
      if (clear.count(door_char) > 0) {
        door1->SetSkill(prhash(u8"Transparent"), 1000);
        door2->SetSkill(prhash(u8"Transparent"), 1000);
      }
      if (locks.count(door_char) > 0) {
        int32_t lid = door1->Skill(prhash(u8"Object ID"));
        if (locktags.count(door_char) > 0 && locktags.at(door_char).contains(u8"common")) {
          if (lockid.count(door_char) > 0) {
            lid = lockid.at(door_char);
          } else {
            lockid[door_char] = lid;
          }
        }
        if (locktags.count(door_char) > 0 && locktags.at(door_char).contains(u8"resident")) {
          for (auto loc : {door1->Parent(), door2->Parent()}) {
            if (loc->Skill(prhash(u8"Translucent")) < 1000) { // Only for those *inside* the place.
              loc_keys[loc].push_back(std::make_pair(door_char, lid));
            }
          }
        }
        if (locktags.count(door_char) > 0 && // FIXME: Make generic, for any keyword
            locktags.at(door_char).contains(u8"guard")) {
          asp_keys[u8"guard"].push_back(std::make_pair(door_char, lid));
        }
        if (locktags.count(door_char) > 0 && // FIXME: Make generic, for any keyword
            locktags.at(door_char).contains(u8"soldier")) {
          asp_keys[u8"soldier"].push_back(std::make_pair(door_char, lid));
        }
        if (locktags.count(door_char) > 0 && // FIXME: Make generic, for any keyword
            locktags.at(door_char).contains(u8"dwarven")) {
          asp_keys[u8"dwarven"].push_back(std::make_pair(door_char, lid));
        }
        door1->SetSkill(prhash(u8"Lock"), lid);
        door2->SetSkill(prhash(u8"Lock"), lid);
      }
      if (locked.count(door_char) > 0) {
        door1->SetSkill(prhash(u8"Locked"), 1);
        door2->SetSkill(prhash(u8"Locked"), 1);
      }
    }
  }

  // Now, populate this new zone.
  int homeless = 0;
  for (auto obj : objs) {
    auto x = obj.first.x;
    auto y = obj.first.y;
    auto room = ascii_map[y][x];
    if (empnames.count(room) > 0) {
      for (size_t n = 0; n < empnames[room].size(); ++n) {
        npc_dwarf.SetName(empnames[room][n]);
        int num = empnums[room][n](gen);
        int floor = empfloors[room][n];
        bool night = empnight[room][n];
        for (int m = 0; m < num; ++m) {
          objs[coord{x, y}][floor]->AddNPC(gen, &npc_dwarf);
          Object* npc = objs[coord{x, y}][floor]->Contents().back();
          int gender = (npc->Gender() == 'F') ? 0 : 1;

          std::vector<std::u8string> first = {u8""};
          std::sample(
              dwarf_first_names[gender].begin(),
              dwarf_first_names[gender].end(),
              first.begin(),
              1,
              gen);

          std::vector<std::u8string> last = {u8""};
          std::sample(dwarf_last_names.begin(), dwarf_last_names.end(), last.begin(), 1, gen);

          npc->SetName(first.front() + u8" " + last.front());

          npc->AddAct(act_t::SPECIAL_WORK, objs[coord{x, y}][floor]);

          std::set<int32_t> have_keys;
          Object* bag = npc->ActTarg(act_t::WEAR_RHIP);
          if (!bag) {
            bag = npc; // FIXME: Dimensional storage.  Create bag instead.
          }

          // Grant them all keys needed for their workplace
          if (loc_keys.count(objs[coord{x, y}][floor]) > 0) {
            for (auto keydef : loc_keys[objs[coord{x, y}][floor]]) {
              if (!have_keys.contains(keydef.second)) {
                Object* key = new Object(bag);
                key->SetDescs(keynames[keydef.first], u8"A rather well-made key.", u8"", u8"");
                key->SetWeight(454 / 16);
                key->SetVolume(0);
                key->SetValue(1);
                key->SetSize(0);
                key->SetPos(pos_t::LIE);
                key->SetSkill(prhash(u8"Key"), keydef.second);
                have_keys.insert(keydef.second);
              }
            }
          }
          if (floor != 0 && loc_keys.count(objs[coord{x, y}][0]) > 0) {
            for (auto keydef : loc_keys[objs[coord{x, y}][0]]) {
              if (!have_keys.contains(keydef.second)) {
                Object* key = new Object(bag);
                key->SetDescs(keynames[keydef.first], u8"A rather well-made key.", u8"", u8"");
                key->SetWeight(454 / 16);
                key->SetVolume(0);
                key->SetValue(1);
                key->SetSize(0);
                key->SetPos(pos_t::LIE);
                key->SetSkill(prhash(u8"Key"), keydef.second);
                have_keys.insert(keydef.second);
              }
            }
          }

          // Grant them all keys they deserve for their title(s)
          for (const auto& asp : asp_keys) {
            if (npc->Matches(asp.first)) {
              for (const auto& keydef : asp.second) {
                if (!have_keys.contains(keydef.second)) {
                  Object* key = new Object(bag);
                  key->SetDescs(keynames[keydef.first], u8"A rather well-made key.", u8"", u8"");
                  key->SetWeight(454 / 16);
                  key->SetVolume(0);
                  key->SetValue(1);
                  key->SetSize(0);
                  key->SetPos(pos_t::LIE);
                  key->SetSkill(prhash(u8"Key"), keydef.second);
                  have_keys.insert(keydef.second);
                }
              }
            }
          }

          int timeliness = std::uniform_int_distribution<int>(-5, 20)(gen);
          if (night) {
            npc->SetSkill(prhash(u8"Night Worker"), timeliness);
          } else {
            npc->SetSkill(prhash(u8"Day Worker"), timeliness);
          }

          // Now find them a home.
          bool housed = false;
          uint32_t rng = ascii_map[0].size() + ascii_map.size();
          for (uint32_t d = 0; !housed && d < rng; ++d) { // Commute Distance, Limited by Map Size
            for (uint32_t s = 0; !housed && s < std::max(1U, d * 4); ++s) { // Num Samples To Check
              uint32_t loc_x = x - d + ((s <= (d * 2)) ? s : (d * 4) - s);
              uint32_t t = (d == 0) ? 0 : (s + d) % (d * 4);
              uint32_t loc_y = y - d + ((t <= (d * 2)) ? t : (d * 4) - t);
              if (loc_x < ascii_map[0].size() && loc_y < ascii_map.size()) { // >= 0 is implied
                auto loc = coord{loc_x, loc_y};
                char8_t type = ascii_map[loc_y][loc_x];
                if (resnames.count(type) > 0) {
                  for (size_t f = 0; !housed && f < resnames.at(type).size(); ++f) {
                    if (resnames.at(type)[f] == empnames[room][n]) {
                      int resfl = resfloors.at(type)[f];
                      if (beds.at(std::make_pair(objs[loc][resfl], empnames[room][n])) > 0) {
                        npc->AddAct(act_t::SPECIAL_HOME, objs[loc][resfl]);
                        --beds.at(std::make_pair(objs[loc][resfl], empnames[room][n]));
                        housed = true;

                        // Grant them all keys needed for their new home
                        if (loc_keys.count(objs[loc][resfl]) > 0) {
                          for (auto keydef : loc_keys[objs[loc][resfl]]) {
                            if (!have_keys.contains(keydef.second)) {
                              Object* key = new Object(bag);
                              key->SetDescs(
                                  keynames[keydef.first], u8"A rather well-made key.", u8"", u8"");
                              key->SetWeight(454 / 16);
                              key->SetVolume(0);
                              key->SetValue(1);
                              key->SetSize(0);
                              key->SetPos(pos_t::LIE);
                              key->SetSkill(prhash(u8"Key"), keydef.second);
                              have_keys.insert(keydef.second);
                            }
                          }
                        }
                        if (resfl != 0 && loc_keys.count(objs[loc][0]) > 0) {
                          for (auto keydef : loc_keys[objs[loc][0]]) {
                            if (!have_keys.contains(keydef.second)) {
                              Object* key = new Object(bag);
                              key->SetDescs(
                                  keynames[keydef.first], u8"A rather well-made key.", u8"", u8"");
                              key->SetWeight(454 / 16);
                              key->SetVolume(0);
                              key->SetValue(1);
                              key->SetSize(0);
                              key->SetPos(pos_t::LIE);
                              key->SetSkill(prhash(u8"Key"), keydef.second);
                              have_keys.insert(keydef.second);
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
          if (!housed) {
            logey(u8"Warning: Homeless NPC in {}: {}\n", zone->ShortDesc(), empnames[room][n]);
            ++homeless;
          }
        }
      }
    }
  }

  if (homeless > 0) {
    mind->Send(CYEL u8"Warning: {} Homeless NPCs in {}\n" CNRM, homeless, zone->ShortDesc());
  } else {
    std::map<std::u8string, int> housing;
    for (auto bed : beds) {
      if (bed.second > 0) {
        housing[bed.first.second] += bed.second;
      }
    }
    for (auto house : housing) {
      mind->Send(
          CGRN u8"TODO: Available Housing in {} for {}: {}\n" CNRM,
          zone->ShortDesc(),
          house.first,
          house.second);
    }
  }

  mind->Send(u8"Loaded {} From: '{}'!\n", name, filename);
  return true;
}

int handle_command_wload(
    Object* body,
    Mind* mind,
    const std::u8string_view args,
    int stealth_t,
    int stealth_s) {
  if (args.empty()) {
    mind->Send(u8"You need to specify the directory of the datafiles!\n");
    return 1;
  }
  std::vector<std::filesystem::directory_entry> files;
  try {
    for (const auto& fl : std::filesystem::directory_iterator(args)) {
      if (!fl.path().u8string().contains(u8"/.")) { // Ignore hidden files
        files.emplace_back(fl);
      }
    }
  } catch (...) {
    mind->Send(u8"You need to specify the correct directory of the datafiles!\n");
    return 1;
  }

  // Create New World
  std::u8string world_name(args);
  Object* world = new Object(body->Parent());
  world->SetShortDesc(world_name);
  world->SetSkill(prhash(u8"Light Source"), 1000);
  world->SetSkill(prhash(u8"Day Length"), 240);
  world->SetSkill(prhash(u8"Day Time"), 120);

  zone_links.clear();
  for (const auto& fl : files) {
    if (!load_map(world, mind, fl)) {
      delete world;
      return 0;
    }
  }

  world->Activate();

  body->Parent()->SendOut(
      stealth_t,
      stealth_s,
      u8";s creates a new world '{}' with Ninja Powers[TM].\n",
      u8"You create a new world '{}'.\n",
      body,
      nullptr,
      world->ShortDesc());

  return 0;
}