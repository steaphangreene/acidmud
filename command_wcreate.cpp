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
#include <cstdlib>
#include <cstring>
#include <random>

#include "commands.hpp"
#include "mind.hpp"
#include "object.hpp"
#include "utils.hpp"

#define NUM_AVS 5 // 28 Max!
#define NUM_STS 5 // No Max
int handle_command_wcreate(
    Object* body,
    Mind* mind,
    const std::string_view args,
    int stealth_t,
    int stealth_s) {
  if (args.empty()) {
    mind->Send("You need to specify the filename of the datafile!\n");
    return 0;
  }

  std::string filename(args);
  FILE* def_file = fopen(filename.c_str(), "r");
  if (def_file == nullptr) {
    mind->SendF("Can't find definition file '%s'!\n", filename.c_str());
    return 0;
  }

  std::string name = "Unknown Land";
  int lower_level = 0;
  int upper_level = 1;

  std::map<char, std::vector<std::string>> rooms;
  std::map<char, bool> indoors;
  std::map<char, uint8_t> levels;
  bool in_main_def = false;
  char line_buf[65536] = "";
  while (!in_main_def) {
    fscanf(def_file, " %65535[^\n]", line_buf);
    if (!strncmp(line_buf, "name:", 5)) {
      char namebuf[256];
      sscanf(line_buf + 5, "%255[^\n]", namebuf);
      name = namebuf;
    } else if (!strncmp(line_buf, "lower_level:", 12)) {
      sscanf(line_buf + 12, "%d[^\n]", &lower_level);
    } else if (!strncmp(line_buf, "upper_level:", 12)) {
      sscanf(line_buf + 12, "%d[^\n]", &upper_level);
    } else if (!strncmp(line_buf, "building:", 9)) {
      char sym = '0';
      sscanf(line_buf + 9, "%c", &sym);
      char namebuf[256];
      sscanf(line_buf + 11, "%255[^\n]", namebuf);
      rooms[sym].emplace_back(namebuf);
      indoors[sym] = true;
    } else if (!strncmp(line_buf, "place:", 6)) {
      char sym = '0';
      sscanf(line_buf + 6, "%c", &sym);
      char namebuf[256];
      sscanf(line_buf + 8, "%255[^\n]", namebuf);
      rooms[sym].emplace_back(namebuf);
    } else if (!strncmp(line_buf, "level:", 6)) {
      char sym = '0';
      sscanf(line_buf + 6, "%c", &sym);
      uint8_t lev;
      sscanf(line_buf + 8, "%hhu", &lev);
      levels[sym] = lev;
    } else if (!strncmp(line_buf, "entrance:", 9)) {
      // TODO: Connect Entrances!
    } else if (!strncmp(line_buf, "ascii_map:", 10)) {
      in_main_def = true;
    } else if (line_buf[0] == '#') {
      // This is a comment, so just ignore this line.
    } else {
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
  while (fscanf(def_file, "%65535[^\n]", line_buf + 1) > 0) {
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
      } else if (room == '@') { // Default Entrance
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

  std::random_device rd;
  std::mt19937 g(rd());

  // Create New World
  Object* world = new Object(body->Parent());
  world->SetShortDesc(name);
  world->SetSkill(crc32c("Light Source"), 1000);
  world->SetSkill(crc32c("Day Length"), 240);
  world->SetSkill(crc32c("Day Time"), 120);

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
          objs[coord{x, y}].push_back(new Object(world));
          if (indoors[room]) {
            objs[coord{x, y}].back()->SetSkill(crc32c("Translucent"), 200);
            objs[coord{x, y}].back()->SetSkill(crc32c("Light Source"), 100);
          } else {
            objs[coord{x, y}].back()->SetSkill(crc32c("Translucent"), 1000);
          }
        }
        std::string roomname = rooms[room][0];
        if (rooms[room].size() > 1) {
          std::vector<std::string> out = {""};
          std::sample(rooms[room].begin(), rooms[room].end(), out.begin(), 1, g);
          roomname = out.front();
        }
        if (objs[coord{x, y}].size() > 1) {
          for (uint32_t f = 0; f < objs[coord{x, y}].size(); ++f) {
            objs[coord{x, y}][f]->SetShortDesc(floornames[f] + roomname);
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
        }
      } else if (ascii_isdigit(room)) { // Default Room
        int num_floors = room - '0';
        if (num_floors < 1) {
          num_floors = 1;
        }
        for (int f = 0; f < num_floors; ++f) {
          objs[coord{x, y}].push_back(new Object(world));
          if (indoors[room]) {
            objs[coord{x, y}].back()->SetSkill(crc32c("Translucent"), 200);
            objs[coord{x, y}].back()->SetSkill(crc32c("Light Source"), 100);
          } else {
            objs[coord{x, y}].back()->SetSkill(crc32c("Translucent"), 1000);
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
      } else if (room == '@') { // Default Entrance
        objs[coord{x, y}].push_back(new Object(world));
        objs[coord{x, y}].back()->SetSkill(crc32c("Translucent"), 1000);
        objs[coord{x, y}].back()->SetShortDesc("a zone entrance");
        // TODO: Connect Entrances!
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
    const std::string start = "A passage ";
    for (uint8_t dir = 0; dir < 8; ++dir) {
      uint32_t dx = x + off_x[dir];
      uint32_t dy = y + off_y[dir];
      bool is_upper = false;
      bool is_lower = false;
      if (ascii_islower(ascii_map[dy][dx])) {
        is_lower = true;
      } else if (ascii_isupper(ascii_map[dy][dx])) {
        is_upper = true;
      } else if (
          ascii_map[dy][dx] == '-' || ascii_map[dy][dx] == '/' || ascii_map[dy][dx] == '|' ||
          ascii_map[dy][dx] == '\\') {
        // Connections with no level forcing
      } else {
        if (ascii_map[dy][dx] != ' ') {
          mind->SendF("Unrecognized passage symbol '%c'!\n", ascii_map[dy][dx]);
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
      if(connected[objs[coord{dx, dy}][l2]] & dmask[dir ^ 3]) {
        // mind->Send("Direction already (at least partly) defined from there.\n");
        continue;
      }

      connected[obj.second[l1]] |= dmask[dir];
      connected[objs[coord{dx, dy}][l2]] |= dmask[dir ^ 3];
      obj.second[l1]->Link(
          objs[coord{dx, dy}][l2],
          dirnames[dir],
          start + dirnames[dir] + " is here.\n",
          dirnames[dir ^ 3],
          start + dirnames[dir ^ 3] + " is here.\n");
    }
  }

  mind->SendF("Loaded %s From: '%s'!\n", name.c_str(), filename.c_str());
  fclose(def_file);
  return 0;
}
