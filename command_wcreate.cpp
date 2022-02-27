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

#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <random>

#include "commands.hpp"
#include "mind.hpp"
#include "mob.hpp"
#include "object.hpp"
#include "properties.hpp"
#include "utils.hpp"

// From: https://gist.github.com/Brennall/b9c3a0202eb11c5cfd54868c5752012a
static const std::vector<std::string> dwarf_first_names[2] = {
    {
        "Anbera",   "Artin",    "Audhild", "Balifra",  "Barbena", "Bardryn",  "Bolhild",
        "Dagnal",   "Dafifi",   "Delre",   "Diesa",    "Hdeth",   "Eridred",  "Falkrunn",
        "Fallthra", "Finelien", "Gillydd", "Gunnloda", "Gurdis",  "Helgret",  "Helja",
        "Hlin",     "llde",     "Jarana",  "Kathra",   "Kilia",   "Kristryd", "Liftrasa",
        "Marastyr", "Mardred",  "Morana",  "Nalaed",   "Nora",    "Nurkara",  "Orifi",
        "Ovina",    "Riswynn",  "Sannl",   "Therlin",  "Thodris", "Torbera",  "Tordrid",
        "Torgga",   "Urshar",   "Valida",  "Vistra",   "Vonana",  "Werydd",   "Whurd red",
        "Yurgunn",
    },
    {
        "Adrik",   "Alberich", "Baern",    "Barendd", "Beloril", "Brottor", "Dain",     "Dalgal",
        "Darrak",  "Delg",     "Duergath", "Dworic",  "Eberk",   "Einkil",  "Elaim",    "Erias",
        "Fallond", "Fargrim",  "Gardain",  "Gilthur", "Gimgen",  "Gimurt",  "Harbek",   "Kildrak",
        "Kilvar",  "Morgran",  "Morkral",  "Nalral",  "Nordak",  "Nuraval", "Oloric",   "Olunt",
        "Osrik",   "Oskar",    "Rangrim",  "Reirak",  "Rurik",   "Taklinn", "Thoradin", "Thorin",
        "Thradal", "Tordek",   "Traubon",  "Travok",  "Ulfgar",  "Uraim",   "Veit",     "Vonbin",
        "Vondal",  "Whurbin",
    }};

// From: https://gist.github.com/Brennall/b9c3a0202eb11c5cfd54868c5752012a
static const std::vector<std::string> dwarf_last_names = {
    "Aranore",     "Balderk",     "Battlehammer", "Bigtoe",      "Bloodkith",    "Bofdarm",
    "Brawnanvil",  "Brazzik",     "Broodfist",    "Burrowfound", "Caebrek",      "Daerdahk",
    "Dankil",      "Daraln",      "Deepdelver",   "Durthane",    "Eversharp",    "FaHack",
    "Fire-forge",  "Foamtankard", "Frostbeard",   "Glanhig",     "Goblinbane",   "Goldfinder",
    "Gorunn",      "Graybeard",   "Hammerstone",  "Helcral",     "Holderhek",    "Ironfist",
    "Loderr",      "Lutgehr",     "Morigak",      "Orcfoe",      "Rakankrak",    "Ruby-Eye",
    "Rumnaheim",   "Silveraxe",   "Silverstone",  "Steelfist",   "Stoutale",     "Strakeln",
    "Strongheart", "Thrahak",     "Torevir",      "Torunn",      "Trollbleeder", "Trueanvil",
    "Trueblood",   "Ungart",
};

static std::random_device rd;
static std::mt19937 gen(rd());

static Object* new_object(Object* parent) {
  Object* body = new Object(parent);
  auto obj_id = body->World()->Skill(prhash("Last Object ID")) + 1;
  body->World()->SetSkill(prhash("Last Object ID"), obj_id);
  body->SetSkill(prhash("Object ID"), obj_id);
  return body;
}
std::multimap<std::string, std::pair<std::string, Object*>> zone_links;

static MOBType mob_dwarf(
    "a dwarf",
    "{He} looks pissed.",
    "",
    "MF",
    {9, 4, 6, 4, 9, 4},
    {10, 7, 11, 8, 18, 8},
    100,
    500);

static int load_map(Object* world, Mind* mind, const std::string_view fn) {
  if (fn.empty()) {
    mind->Send("You need to specify the filename of the datafile!\n");
    return 1;
  }

  std::string filename(fn);
  FILE* def_file = fopen(filename.c_str(), "r");
  if (def_file == nullptr) {
    mind->SendF("Can't find definition file '%s'!\n", filename.c_str());
    return 1;
  }

  std::string name = "Unknown Land";
  uint8_t lower_level = 0;
  uint8_t upper_level = 1;

  std::map<char, std::vector<std::string>> rooms;
  std::map<char, std::string> doors;
  std::map<char, std::string> doorterms;
  std::map<char, bool> remote;
  std::map<char, bool> closed;
  std::map<char, bool> clear;
  std::map<char, bool> locks;
  std::map<char, bool> locked;
  std::map<char, bool> indoors;
  std::map<char, uint8_t> levels;
  std::map<char, std::vector<std::string>> empnames;
  std::map<char, std::vector<int>> empnums;
  std::map<char, std::vector<int>> empfloors;
  std::map<char, std::vector<std::string>> resnames;
  std::map<char, std::vector<int>> resnums;
  std::map<char, std::vector<int>> resfloors;
  std::vector<std::string> en_links;
  std::vector<std::string> en_rooms;
  std::vector<bool> en_indoors;
  char start_symbol = '\0';

  bool in_main_def = false;
  char line_buf[65536] = "";
  while (!in_main_def) {
    bool parse_error = false;
    fscanf(def_file, " %65535[^\n]", line_buf);
    if (!strncmp(line_buf, "name:", 5)) {
      char namebuf[256];
      sscanf(line_buf + 5, "%255[^\n]", namebuf);
      name = namebuf;
    } else if (!strncmp(line_buf, "world:", 6)) {
      char namebuf[256];
      sscanf(line_buf + 6, "%255[^\n]", namebuf);
      world->SetShortDesc(namebuf);
    } else if (!strncmp(line_buf, "lower_level:", 12)) {
      sscanf(line_buf + 12, "%hhu[^\n]", &lower_level);
    } else if (!strncmp(line_buf, "upper_level:", 12)) {
      sscanf(line_buf + 12, "%hhu[^\n]", &upper_level);
    } else if (!strncmp(line_buf, "building:", 9)) {
      char sym = '0';
      sscanf(line_buf + 9, "%c", &sym);
      char namebuf[256];
      sscanf(line_buf + 10, ":%255[^\n]", namebuf);
      rooms[sym].emplace_back(namebuf);
      indoors[sym] = true;
    } else if (!strncmp(line_buf, "place:", 6)) {
      char sym = '0';
      sscanf(line_buf + 6, "%c", &sym);
      char namebuf[256];
      sscanf(line_buf + 7, ":%255[^\n]", namebuf);
      rooms[sym].emplace_back(namebuf);
    } else if (!strncmp(line_buf, "level:", 6)) {
      char sym = '0';
      sscanf(line_buf + 6, "%c", &sym);
      uint8_t lev;
      sscanf(line_buf + 7, ":%hhu", &lev);
      levels[sym] = lev;
    } else if (!strncmp(line_buf, "employ:", 7)) {
      char sym = '0';
      sscanf(line_buf + 7, "%c", &sym);
      int floor = 0;
      int lnum, hnum;
      char namebuf[256];
      int off = 8;
      if (sscanf(line_buf + off, ".%d", &floor) == 1) {
        off = 10;
      }
      if (sscanf(line_buf + off, ":%d", &lnum) < 1) {
        parse_error = true;
      }
      if (sscanf(line_buf + off, ":%*d-%d", &hnum) < 1) {
        hnum = lnum;
        if (sscanf(line_buf + off, ":%*d:%255[^\n]", namebuf) < 1) {
          parse_error = true;
        }
      } else {
        if (sscanf(line_buf + off, ":%*d-%*d:%255[^\n]", namebuf) < 1) {
          parse_error = true;
        }
      }
      if (!parse_error) {
        empnames[sym].emplace_back(namebuf);
        empnums[sym].push_back(std::uniform_int_distribution<int>(lnum, hnum)(gen));
        empfloors[sym].push_back(floor);
      }
    } else if (!strncmp(line_buf, "house:", 6)) {
      char sym = '0';
      sscanf(line_buf + 6, "%c", &sym);
      int floor = 0;
      int lnum, hnum;
      char namebuf[256];
      int off = 7;
      if (sscanf(line_buf + off, ".%d", &floor) == 1) {
        off = 9;
      }
      if (sscanf(line_buf + off, ":%d", &lnum) < 1) {
        parse_error = true;
      }
      if (sscanf(line_buf + off, ":%*d-%d", &hnum) < 1) {
        hnum = lnum;
        if (sscanf(line_buf + off, ":%*d:%255[^\n]", namebuf) < 1) {
          parse_error = true;
        }
      } else {
        if (sscanf(line_buf + off, ":%*d-%*d:%255[^\n]", namebuf) < 1) {
          parse_error = true;
        }
      }
      if (!parse_error) {
        resnames[sym].emplace_back(namebuf);
        resnums[sym].push_back(std::uniform_int_distribution<int>(lnum, hnum)(gen));
        resfloors[sym].push_back(floor);
      }
    } else if (!strncmp(line_buf, "door:", 5)) {
      char sym = 'd';
      sscanf(line_buf + 5, "%c", &sym);
      char namebuf[256];
      sscanf(line_buf + 6, ":%255[^\n]", namebuf);
      doors[sym] = namebuf;
    } else if (!strncmp(line_buf, "doorterm:", 9)) {
      char sym = 'd';
      sscanf(line_buf + 9, "%c", &sym);
      char namebuf[256];
      sscanf(line_buf + 10, ":%255[^\n]", namebuf);
      doorterms[sym] = namebuf;
    } else if (!strncmp(line_buf, "lock:", 5)) {
      char sym = 'd';
      sscanf(line_buf + 5, "%c", &sym);
      locks[sym] = true;
    } else if (!strncmp(line_buf, "locked:", 7)) {
      char sym = 'd';
      sscanf(line_buf + 7, "%c", &sym);
      locked[sym] = true;
    } else if (!strncmp(line_buf, "closed:", 7)) {
      char sym = 'd';
      sscanf(line_buf + 7, "%c", &sym);
      closed[sym] = true;
    } else if (!strncmp(line_buf, "clear:", 6)) {
      char sym = 'd';
      sscanf(line_buf + 6, "%c", &sym);
      clear[sym] = true;
    } else if (!strncmp(line_buf, "remote:", 7)) {
      char sym = 'd';
      sscanf(line_buf + 7, "%c", &sym);
      remote[sym] = true;
    } else if (!strncmp(line_buf, "en_link:", 8)) {
      en_links.push_back(line_buf + 8);
    } else if (!strncmp(line_buf, "en_place:", 9)) {
      en_rooms.push_back(line_buf + 9);
      en_indoors.push_back(false);
    } else if (!strncmp(line_buf, "en_building:", 12)) {
      en_rooms.push_back(line_buf + 12);
      en_indoors.push_back(true);
    } else if (!strncmp(line_buf, "start:", 6)) {
      sscanf(line_buf + 6, "%c", &start_symbol);
    } else if (!strncmp(line_buf, "ascii_map:", 10)) {
      in_main_def = true;
    } else if (line_buf[0] == '#') {
      // This is a comment, so just ignore this line.
    } else {
      parse_error = true;
    }

    if (parse_error) {
      mind->SendF("Bad definition file '%s'!\n", filename.c_str());
      mind->SendF("Read: '%s'!\n", line_buf);
      fclose(def_file);
      return 0;
    }
  }

  // Load ASCII Map Into A vector<string>, Padding All Sides with Spaces
  std::vector<std::string> ascii_map = {""};
  fscanf(def_file, "%*[\n]");
  line_buf[0] = ' '; // Padding
  size_t max_line_len = 0;
  while (fscanf(def_file, "%65534[^\n]", line_buf + 1) > 0) {
    ascii_map.emplace_back(line_buf);
    max_line_len = std::max(max_line_len, ascii_map.back().length() + 1);
    fscanf(def_file, "%*[\n]");
  }
  ascii_map.emplace_back("");
  for (auto& line : ascii_map) {
    for (size_t pad = line.length(); pad < max_line_len; ++pad) {
      line += " ";
    }
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
        mind->SendF("Bad definition file '%s'!\n", filename.c_str());
        mind->SendF("Read Unknown Character '%c' at ascii_map (%u,%u)!\n", room, x, y);
        fclose(def_file);
        return 0;
      }
    }
  }

  // Create New Zone
  Object* zone = new_object(world);
  zone->SetShortDesc(name);
  zone->SetSkill(prhash("Light Source"), 1000);
  zone->SetSkill(prhash("Day Length"), 240);
  zone->SetSkill(prhash("Day Time"), 120);

  // Convert ASCII Map Into AcidMUD Rooms
  struct coord {
    uint32_t x;
    uint32_t y;
    auto operator<=>(const coord&) const = default;
  };
  static const std::string floornames[] = {
      "first floor of the ",
      "second floor of the ",
      "third floor of the ",
      "fourth floor of the ",
      "fifth floor of the ",
      "sixth floor of the ",
      "seventh floor of the ",
      "eighth floor of the ",
      "ninth floor of the ",
      "tenth floor of the ",
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
          if (indoors[room]) {
            objs[coord{x, y}].back()->SetSkill(prhash("Translucent"), 200);
            objs[coord{x, y}].back()->SetSkill(prhash("Light Source"), 100);
          } else {
            objs[coord{x, y}].back()->SetSkill(prhash("Translucent"), 1000);
          }
          if (start_symbol == room) {
            world->AddAct(act_t::SPECIAL_HOME, objs[coord{x, y}].back());
          }
        }
        std::string roomname = rooms[room][0];
        if (rooms[room].size() > 1) {
          std::vector<std::string> out = {""};
          std::sample(rooms[room].begin(), rooms[room].end(), out.begin(), 1, gen);
          roomname = out.front();
        }
        if (objs[coord{x, y}].size() > 1) {
          for (uint32_t f = 0; f < objs[coord{x, y}].size(); ++f) {
            objs[coord{x, y}][f]->SetShortDesc(floornames[f] + roomname);
            objs[coord{x, y}][f]->SetDesc(
                std::string("This is a building in ") + zone->ShortDesc() + ", on " +
                world->ShortDesc() + ".  " + zone->ShortDesc() + " is nice.");
            if (f > 0) {
              objs[coord{x, y}][f]->Link(
                  objs[coord{x, y}][f - 1],
                  "down",
                  "The floor below.\n",
                  "up",
                  "The floor above.\n");
            }
          }
        } else {
          objs[coord{x, y}].back()->SetShortDesc(roomname);
          objs[coord{x, y}].back()->SetDesc(
              std::string("This is ") + zone->ShortDesc() + ", on " + world->ShortDesc() + ".  " +
              zone->ShortDesc() + " is nice.");
        }
      } else if (ascii_isdigit(room)) { // Default Room
        int num_floors = room - '0';
        if (num_floors < 1) {
          num_floors = 1;
        }
        for (int f = 0; f < num_floors; ++f) {
          objs[coord{x, y}].push_back(new_object(zone));
          if (indoors[room]) {
            objs[coord{x, y}].back()->SetSkill(prhash("Translucent"), 200);
            objs[coord{x, y}].back()->SetSkill(prhash("Light Source"), 100);
          } else {
            objs[coord{x, y}].back()->SetSkill(prhash("Translucent"), 1000);
          }
          if (start_symbol == room) {
            world->AddAct(act_t::SPECIAL_HOME, objs[coord{x, y}].back());
          }
        }
        if (objs[coord{x, y}].size() > 1) {
          for (uint32_t f = 0; f < objs[coord{x, y}].size(); ++f) {
            objs[coord{x, y}][f]->SetShortDesc(floornames[f] + "building");
            if (f > 0) {
              objs[coord{x, y}][f]->Link(
                  objs[coord{x, y}][f - 1],
                  "down",
                  "The floor below.\n",
                  "up",
                  "The floor above.\n");
            }
          }
        } else {
          objs[coord{x, y}].back()->SetShortDesc("a room");
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
          entrance->SetShortDesc("a generic zone entrance");
          objs[coord{x, y}].push_back(entrance);
          if (!en_links.empty()) {
            auto ozone = en_links.front();
            zone_links.insert(std::make_pair(ozone, std::make_pair(name, entrance)));
          }
        } else { // Object already exists, well-defined or not - use it.
          entrance = objs[coord{x, y}].back();
        }

        if (entrance->ShortDesc() == "a generic zone entrance") { // Not well-defined yet
          if (!en_indoors.empty() && en_indoors.front()) {
            objs[coord{x, y}].back()->SetSkill(prhash("Translucent"), 200);
            objs[coord{x, y}].back()->SetSkill(prhash("Light Source"), 100);
          } else {
            objs[coord{x, y}].back()->SetSkill(prhash("Translucent"), 1000);
          }
          if (!en_rooms.empty()) {
            objs[coord{x, y}].back()->SetShortDesc(en_rooms.front());
            objs[coord{x, y}].back()->SetDesc(
                std::string("This is ") + zone->ShortDesc() + ", on " + world->ShortDesc() + ".  " +
                zone->ShortDesc() + " is nice.");
          }
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

    const std::string dirnames[] = {
        "south and east",
        "south and west",
        "north and east",
        "north and west",
        "south",
        "east",
        "west",
        "north",
    };
    const int32_t off_x[] = {1, -1, 1, -1, 0, 1, -1, 0};
    const int32_t off_y[] = {1, 1, -1, -1, 1, 0, 0, -1};
    const uint8_t dmask[] = {3, 5, 10, 12, 1, 2, 4, 8};
    for (uint8_t dir = 0; dir < 8; ++dir) {
      uint32_t dx = x + off_x[dir];
      uint32_t dy = y + off_y[dir];
      bool is_upper = false;
      bool is_lower = false;
      char door_char = ascii_map[dy][dx];
      if (ascii_islower(door_char)) {
        is_lower = true;
      } else if (ascii_isupper(door_char)) {
        is_upper = true;
      } else if (door_char == '-' || door_char == '/' || door_char == '|' || door_char == '\\') {
        // Connections with no level forcing
      } else {
        if (door_char != ' ') {
          mind->SendF("Unrecognized passage symbol '%c'!\n", door_char);
        }
        continue;
      }

      do {
        dx += off_x[dir];
        dy += off_y[dir];
      } while (ascii_isalpha(ascii_map[dy][dx]) || ascii_map[dy][dx] == '-' ||
               ascii_map[dy][dx] == '/' || ascii_map[dy][dx] == '|' || ascii_map[dy][dx] == '\\');

      if (objs.count(coord{dx, dy}) <= 0) {
        mind->SendF("Path to nowhere (%u,%u)->(%u,%u)!\n", x, y, dx, dy);
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
          // mind->SendF("Lower link incompatible with origin point '%c'->'%c'!\n", schr, dchr);
          continue;
        }

        if (levels[dchr] <= lower_level && dst.size() + levels[dchr] > lower_level) {
          l2 = lower_level - levels[dchr];
        } else {
          // mind->SendF("Lower link incompatible with dest point '%c'->'%c'!\n", schr, dchr);
          continue;
        }

      } else if (is_upper) {
        if (levels[schr] <= upper_level && obj.second.size() + levels[schr] > upper_level) {
          l1 = upper_level - levels[schr];
        } else {
          // mind->SendF("Upper link incompatible with origin point '%c'->'%c'!\n", schr, dchr);
          continue;
        }

        if (levels[dchr] <= upper_level && dst.size() + levels[dchr] > upper_level) {
          l2 = upper_level - levels[dchr];
        } else {
          // mind->SendF("Upper link incompatible with dest point '%c'->'%c'!\n", schr, dchr);
          continue;
        }
      }

      if (connected[obj.second[l1]] & dmask[dir]) {
        // mind->Send("Direction already (at least partly) defined from here.\n");
        continue;
      }
      if (connected[objs[coord{dx, dy}][l2]] & dmask[dir ^ 3]) {
        // mind->Send("Direction already (at least partly) defined from there.\n");
        continue;
      }

      std::string start = "A passage ";
      if (doors.count(door_char) > 0) {
        start = std::string("A ") + doors[door_char] + " to the ";
      }
      std::string term = "door";
      if (doorterms.count(door_char) > 0) {
        term = doorterms[door_char];
      }

      connected[obj.second[l1]] |= dmask[dir];
      connected[objs[coord{dx, dy}][l2]] |= dmask[dir ^ 3];

      // Create the linking doors
      Object* door1 = new_object(obj.second[l1]);
      Object* door2 = new_object(objs[coord{dx, dy}][l2]);
      if (doors.count(door_char) > 0) {
        door1->SetShortDesc(dirnames[dir] + " (" + term + ")");
        door2->SetShortDesc(dirnames[dir ^ 3] + " (" + term + ")");
      } else {
        door1->SetShortDesc(dirnames[dir]);
        door2->SetShortDesc(dirnames[dir ^ 3]);
      }
      door1->SetDesc(start + dirnames[dir] + " is here.\n");
      door2->SetDesc(start + dirnames[dir ^ 3] + " is here.\n");
      door1->AddAct(act_t::SPECIAL_LINKED, door2);
      door2->AddAct(act_t::SPECIAL_LINKED, door1);
      door1->AddAct(act_t::SPECIAL_MASTER, door2);
      door2->AddAct(act_t::SPECIAL_MASTER, door1);
      door1->SetSkill(prhash("Enterable"), 1);
      door2->SetSkill(prhash("Enterable"), 1);
      if (doors.count(door_char) > 0) {
        door1->SetSkill(prhash("Closeable"), 1);
        door2->SetSkill(prhash("Closeable"), 1);
      }
      if (closed.count(door_char) == 0) {
        door1->SetSkill(prhash("Open"), 1000);
        door2->SetSkill(prhash("Open"), 1000);
      }
      if (clear.count(door_char) > 0) {
        door1->SetSkill(prhash("Transparent"), 1000);
        door2->SetSkill(prhash("Transparent"), 1000);
      }
      if (locks.count(door_char) > 0) {
        door1->SetSkill(prhash("Lock"), door1->Skill(prhash("Object ID")));
        door2->SetSkill(prhash("Lock"), door1->Skill(prhash("Object ID")));
      }
      if (locked.count(door_char) > 0) {
        door1->SetSkill(prhash("Locked"), 1);
        door2->SetSkill(prhash("Locked"), 1);
      }
    }
  }

  // Now, populate this new zone.
  for (auto obj : objs) {
    auto x = obj.first.x;
    auto y = obj.first.y;
    auto room = ascii_map[y][x];
    if (empnames.count(room) > 0) {
      for (size_t n = 0; n < empnames[room].size(); ++n) {
        mob_dwarf.SetName(empnames[room][n]);
        int num = empnums[room][n];
        int floor = empfloors[room][n];
        for (int m = 0; m < num; ++m) {
          objs[coord{x, y}][floor]->AddMOB(gen, &mob_dwarf);
          Object* mob = objs[coord{x, y}][floor]->Contents().back();
          int gender = (mob->Gender() == 'F') ? 0 : 1;

          std::vector<std::string> first = {""};
          std::sample(
              dwarf_first_names[gender].begin(),
              dwarf_first_names[gender].end(),
              first.begin(),
              1,
              gen);

          std::vector<std::string> last = {""};
          std::sample(dwarf_last_names.begin(), dwarf_last_names.end(), last.begin(), 1, gen);

          mob->SetName(first.front() + " " + last.front());

          mob->AddAct(act_t::SPECIAL_WORK, objs[coord{x, y}][floor]);
        }
      }
    }
  }

  mind->SendF("Loaded %s From: '%s'!\n", name.c_str(), filename.c_str());
  fclose(def_file);
  return 0;
}

int handle_command_wcreate(
    Object* body,
    Mind* mind,
    const std::string_view args,
    int stealth_t,
    int stealth_s) {
  if (args.empty()) {
    mind->Send("You need to specify the directory of the datafiles!\n");
    return 1;
  }
  std::vector<std::string> filenames;
  try {
    for (const auto& fl : std::filesystem::directory_iterator(args)) {
      filenames.emplace_back(fl.path());
    }
  } catch (...) {
    mind->Send("You need to specify the correct directory of the datafiles!\n");
    return 1;
  }

  // Create New World
  std::string world_name(args);
  Object* world = new Object(body->Parent());
  world->SetShortDesc(world_name);
  world->SetSkill(prhash("Light Source"), 1000);
  world->SetSkill(prhash("Day Length"), 240);
  world->SetSkill(prhash("Day Time"), 120);

  zone_links.clear();
  for (const auto& fn : filenames) {
    int ret = load_map(world, mind, fn);
    if (ret != 0) {
      delete world;
      return ret;
    }
  }

  body->Parent()->SendOutF(
      stealth_t,
      stealth_s,
      ";s creates a new world '%s' with Ninja Powers[TM].\n",
      "You create a new world '%s'.\n",
      body,
      nullptr,
      world->ShortDescC());

  return 0;
}
