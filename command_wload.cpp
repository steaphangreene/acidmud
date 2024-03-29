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

#include "color.hpp"
#include "commands.hpp"
#include "dice.hpp"
#include "infile.hpp"
#include "log.hpp"
#include "mind.hpp"
#include "object.hpp"
#include "properties.hpp"
#include "tags.hpp"
#include "utils.hpp"

static Object* add_oid(Object* obj) {
  auto obj_id = obj->World()->Skill(prhash(u8"Last Object ID")) + 1;
  obj->World()->SetSkill(prhash(u8"Last Object ID"), obj_id);
  obj->SetSkill(prhash(u8"Object ID"), obj_id);
  return obj;
}

static Object* new_object(Object* parent) {
  Object* obj = add_oid(new Object(parent));
  return obj;
}
std::multimap<std::u8string, std::pair<std::u8string, Object*>> zone_links;

bool tag_superset(const ObjectTag npcdef, const std::u8string_view& requirements) {
  std::u8string_view required_tags = requirements;
  std::u8string_view tag = getuntil(required_tags, ',');
  while (tag.length() > 0 || required_tags.length() > 0) {
    if (tag.length() > 0) {
      bool found = false;
      for (auto t : npcdef.tags_) {
        if (t == crc32c(tag)) {
          found = true;
          break;
        }
      }
      if (!found) {
        return false;
      }
    }
    tag = getuntil(required_tags, ',');
  }
  return true;
}

static std::set<int32_t> world_has_key;
static ObjectTag make_key(Object* world, const std::u8string_view& keyname, int32_t keynum) {
  if (!world_has_key.contains(keynum)) {
    world_has_key.insert(keynum);
    world->LoadTagsFrom(fmt::format(
        u8"tag:item:key_{0}\n"
        u8"short:{1}\n"
        u8"desc:A rather well-made key.\n"
        u8"weight:30\n"
        u8"value:1\n"
        u8"prop:Key:{0}\n",
        keynum,
        keyname));
  }
  return ObjectTag(fmt::format(
      u8"has_key_{0}\n"
      u8"type:none\n"
      u8"itag:key_{0}\n",
      keynum));
}

static bool
load_map(Object* world, std::shared_ptr<Mind> mind, const std::filesystem::directory_entry& ent) {
  auto filename = ent.path().u8string();

  if (filename.empty()) {
    mind->Send(u8"You need to specify the filename of the datafile!\n");
    return false;
  }

  infile file(ent);
  if (!file) {
    mind->Send(u8"Can't find definition file '{}'!\n", filename);
    return false;
  }

  std::u8string name = u8"Unknown Land";
  uint8_t lower_level = 0;
  uint8_t upper_level = 1;

  std::map<char8_t, std::vector<std::u8string>> rooms;
  std::map<char8_t, std::vector<std::u8string>> roomtags;
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
  std::map<char8_t, bool> inorder;
  std::map<char8_t, bool> indoors;
  std::map<char8_t, bool> trees;
  std::map<char8_t, uint8_t> levels;
  std::map<char8_t, std::vector<bool>> stairblocked;
  std::map<char8_t, std::vector<std::u8string>> stairnames;
  std::map<char8_t, std::vector<std::u8string>> emptags;
  std::map<char8_t, std::vector<Dice::Die>> empnums;
  std::map<char8_t, std::vector<int>> empfloors;
  std::map<char8_t, std::vector<bool>> empnight;
  std::map<char8_t, std::vector<std::u8string>> restags;
  std::map<char8_t, std::vector<Dice::Die>> resnums;
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
        roomtags[sym].emplace_back(getuntil(line, ':'));
        rooms[sym].emplace_back(line);
        indoors[sym] = true;
      }
    } else if (process(line, u8"place:")) {
      char8_t sym = nextchar(line);
      if (process(line, u8":")) {
        roomtags[sym].emplace_back(getuntil(line, ':'));
        rooms[sym].emplace_back(line);
      }
    } else if (process(line, u8"level:")) {
      char8_t sym = nextchar(line);
      if (process(line, u8":")) {
        levels[sym] = nextnum(line);
      }
    } else if (process(line, u8"trees:")) {
      char8_t sym = nextchar(line);
      trees[sym] = true;
    } else if (process(line, u8"inorder:")) {
      char8_t sym = nextchar(line);
      inorder[sym] = true;
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
        if (process(line, u8"-")) {
          hnum = nextnum(line);
        } else {
          hnum = lnum;
        }
      } else {
        parse_error = true;
      }
      if (process(line, u8":")) {
        emptags[sym].emplace_back(line);
      } else {
        parse_error = true;
      }
      if (!parse_error) {
        empnums[sym].push_back(Dice::GetDie(lnum, hnum));
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
        if (process(line, u8"-")) {
          hnum = nextnum(line);
        } else {
          hnum = lnum;
        }
      } else {
        parse_error = true;
      }
      if (process(line, u8":")) {
        emptags[sym].emplace_back(line);
      } else {
        parse_error = true;
      }
      if (!parse_error) {
        empnums[sym].push_back(Dice::GetDie(lnum, hnum));
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
        if (process(line, u8"-")) {
          hnum = nextnum(line);
        } else {
          hnum = lnum;
        }
      } else {
        parse_error = true;
      }
      if (process(line, u8":")) {
        restags[sym].emplace_back(line);
      } else {
        parse_error = true;
      }
      if (!parse_error) {
        resnums[sym].push_back(Dice::GetDie(lnum, hnum));
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
    } else if (process(line, u8"product:")) {
    } else if (process(line, u8"ascii_map:")) {
      in_main_def = true;
    } else if (line.front() == '#') {
      // This is a comment, so just ignore this line.
    } else {
      parse_error = true;
    }

    if (parse_error) {
      mind->Send(CRED u8"Bad definition file '{}'!\n" CNRM, filename);
      mind->Send(CRED u8"Read: '{}'!\n" CNRM, line);
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

        std::u8string roomname = rooms[room].front();
        std::u8string roomtag = roomtags[room].front();
        if (rooms[room].size() > 1) { // FIXME: Same number means one of each.
          std::vector<std::u8string> out = {u8""};
          if (inorder[room]) {
            std::rotate(rooms[room].begin(), rooms[room].begin() + 1, rooms[room].end());
            std::rotate(roomtags[room].begin(), roomtags[room].begin() + 1, roomtags[room].end());
          } else {
            int idx = Dice::Rand(0, rooms[room].size() - 1);
            roomname = rooms[room].at(idx);
            roomtag = roomtags[room].at(idx);
          }
        }

        for (int f = 0; f < num_floors; ++f) {
          objs[coord{x, y}].push_back(add_oid(zone->AddRoom(roomtag)));
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
          if (trees[room]) { // FIXME: Convert to a normal tag
            objs[coord{x, y}].back()->SetSkill(prhash(u8"Mature Trees"), 100);
          }
          if (start_symbol == room) {
            world->AddAct(act_t::SPECIAL_HOME, objs[coord{x, y}].back());
          }
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
        if (restags.count(room) > 0) {
          auto loc = coord{x, y};
          for (size_t f = 0; f < restags.at(room).size(); ++f) {
            int resfl = resfloors.at(room)[f];
            if (beds.count(std::make_pair(objs[loc][resfl], restags[room][f])) == 0) {
              beds[std::make_pair(objs[loc][resfl], restags[room][f])] = resnums.at(room)[f]();
            }
          }
        }

      } else if (ascii_isdigit(room)) { // Default Room
        int num_floors = room - '0';
        if (num_floors < 1) {
          num_floors = 1;
        }
        for (int f = 0; f < num_floors; ++f) {
          objs[coord{x, y}].push_back(add_oid(zone->AddRoom(u8"")));
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
        if (restags.count(room) > 0) {
          auto loc = coord{x, y};
          for (size_t f = 0; f < restags.at(room).size(); ++f) {
            int resfl = resfloors.at(room)[f];
            if (beds.count(std::make_pair(objs[loc][resfl], restags[room][f])) == 0) {
              beds[std::make_pair(objs[loc][resfl], restags[room][f])] = resnums.at(room)[f]();
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
          entrance = add_oid(zone->AddRoom(u8""));
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
        if (door_char != ' ' && !ascii_isdigit(door_char)) {
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
        if (locktags.contains(door_char)) {
          std::u8string_view tags = locktags.at(door_char);
          std::u8string_view tag = getuntil(tags, ',');
          while (tag.length() > 0 || tags.length() > 0) {
            if (tag.length() > 0) {
              if (tag == u8"common") {
                if (lockid.count(door_char) > 0) {
                  lid = lockid.at(door_char);
                } else {
                  lockid[door_char] = lid;
                }
              } else if (tag == u8"resident") {
                for (auto loc : {door1->Parent(), door2->Parent()}) {
                  if (loc->Skill(prhash(u8"Translucent")) <
                      1000) { // Only for those *inside* the place.
                    loc_keys[loc].push_back(std::make_pair(door_char, lid));
                  }
                }
              } else {
                asp_keys[std::u8string(tag)].push_back(std::make_pair(door_char, lid));
              }
            }
            tag = getuntil(tags, ',');
          }
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
  world_has_key.clear();
  for (auto obj : objs) {
    auto x = obj.first.x;
    auto y = obj.first.y;
    auto room = ascii_map[y][x];
    if (emptags.count(room) > 0) {
      for (size_t n = 0; n < emptags[room].size(); ++n) {
        int num = empnums[room][n]();
        int floor = empfloors[room][n];
        bool night = empnight[room][n];
        for (int m = 0; m < num; ++m) {
          // Build the definition of this NPC
          ObjectTag npcdef = objs[coord{x, y}][floor]->BuildNPC(emptags[room][n]);
          Object* employed = objs[coord{x, y}][floor];

          std::set<int32_t> npc_has_key;

          // Grant them all keys needed for their workplace
          if (loc_keys.count(objs[coord{x, y}][floor]) > 0) {
            for (auto keydef : loc_keys[objs[coord{x, y}][floor]]) {
              if (!npc_has_key.contains(keydef.second)) {
                npc_has_key.insert(keydef.second);
                npcdef += make_key(world, keynames[keydef.first], keydef.second);
              }
            }
          }
          if (floor != 0 && loc_keys.count(objs[coord{x, y}][0]) > 0) {
            for (auto keydef : loc_keys[objs[coord{x, y}][0]]) {
              if (!npc_has_key.contains(keydef.second)) {
                npc_has_key.insert(keydef.second);
                npcdef += make_key(world, keynames[keydef.first], keydef.second);
              }
            }
          }

          // Grant them all keys they deserve for their title(s)
          for (const auto& asp : asp_keys) {
            if (tag_superset(npcdef, asp.first)) {
              for (const auto& keydef : asp.second) {
                if (!npc_has_key.contains(keydef.second)) {
                  npc_has_key.insert(keydef.second);
                  npcdef += make_key(world, keynames[keydef.first], keydef.second);
                }
              }
            }
          }

          // Now find them a home.
          Object* housed = nullptr;

          // Get a list of all places which can house them.
          std::vector<decltype(beds)::key_type> places;
          places.reserve(beds.size() + 1);
          for (const auto& bed : beds) {
            if (tag_superset(npcdef, bed.first.second)) {
              if (employed->ManhattanDistance(bed.first.first) < 25) { // Max commute distance.
                // TODO: Different NPCs have different commute distance preferences.
                places.emplace_back(bed.first);
              }
            }
          }
          if (places.size() > 0) { // FIXME: This just selects one randomly, do better.
            Dice::Sample(places, std::back_inserter(places));
            housed = places.back().first;
            --beds.at(places.back());

            // Grant them all keys needed for their new home
            if (loc_keys.count(housed) > 0) {
              for (auto keydef : loc_keys[housed]) {
                if (!npc_has_key.contains(keydef.second)) {
                  npc_has_key.insert(keydef.second);
                  npcdef += make_key(world, keynames[keydef.first], keydef.second);
                }
              }
            }
            if (housed->Z() != 0) { // Ground Floor Key Too
              uint32_t loc_x = housed->X();
              uint32_t loc_y = housed->Y();
              auto loc = coord{loc_x, loc_y};
              if (loc_keys.count(objs[loc][0]) > 0) {
                for (auto keydef : loc_keys[objs[loc][0]]) {
                  if (!npc_has_key.contains(keydef.second)) {
                    npc_has_key.insert(keydef.second);
                    npcdef += make_key(world, keynames[keydef.first], keydef.second);
                  }
                }
              }
            }
          }
          // Now, create the actual NPC.
          Object* npc = objs[coord{x, y}][floor]->MakeNPC(npcdef);
          npc->AddAct(act_t::SPECIAL_WORK, employed);

          Object* bed = new Object(housed);
          bed->SetDescs(
              u8"a bed",
              u8"",
              u8"a nice comfortable bed",
              fmt::format(u8"This is {}'s bed.  It looks vert comfortable.", npc->Name()));
          bed->SetSkill(prhash(u8"Open"), 1000);
          bed->SetSkill(prhash(u8"Enterable"), 1000);
          npc->AddAct(act_t::SPECIAL_HOME, bed);

          int timeliness = Dice::Rand(-5, 20);
          if (night) {
            npc->SetSkill(prhash(u8"Night Worker"), timeliness);
          } else {
            npc->SetSkill(prhash(u8"Day Worker"), timeliness);
          }

          if (!housed) {
            logey(
                u8"Warning: Homeless NPC in {}/{}: {} [{}]\n",
                zone->ShortDesc(),
                npc->Room()->ShortDesc(),
                npc->ShortDesc(),
                emptags[room][n]);
            ++homeless;
          }
        }
      }
    }
  }

  if (homeless > 0) {
    mind->Send(CYEL u8"Warning: {} Homeless NPCs in {}\n" CNRM, homeless, zone->ShortDesc());
  }

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

  mind->Send(u8"Loaded {} From: '{}'!\n", name, filename);
  return true;
}

int handle_command_wload(
    Object* body,
    std::shared_ptr<Mind>& mind,
    const std::u8string_view& args,
    int stealth_t,
    int stealth_s) {
  if (args.empty()) {
    mind->Send(u8"You need to specify the directory of the datafiles!\n");
    return 1;
  }

  std::vector<std::filesystem::directory_entry> tag_files;
  std::vector<std::filesystem::directory_entry> map_files;

  std::error_code err[4];
  auto directory = std::filesystem::directory_entry(args, err[0]);
  for (const auto& fl : std::filesystem::directory_iterator(args, err[3])) {
    if (fl.path().u8string().contains(u8"/.")) { // Ignore hidden files
    } else if (fl.path().u8string().ends_with(u8".map")) { // World Map Files
      map_files.emplace_back(fl);
    } else if (fl.path().u8string().ends_with(u8".tags")) { // Tag Definition Files
      tag_files.emplace_back(fl);
    }
  }

  for (auto e : err) {
    if (e) {
      auto rawmes = e.message(); // Available in old-school char string only. :(
      std::u8string mes(rawmes.length() + 1, 0); // Make char8_t string of same size.
      std::copy(rawmes.begin(), rawmes.end(), mes.begin()); // Copy chars to char8_ts.
      mind->Send(CRED u8"Error loading '{}': {}\n" CNRM, args, mes);
      mind->Send(CYEL u8"You need to specify the correct directory of the datafiles!\n" CNRM);
      return 1;
    }
  }

  if (map_files.size() == 0) {
    mind->Send(CRED u8"No map files found in '{}'.\n" CNRM, args);
    mind->Send(CYEL u8"You need to specify the correct directory of the datafiles!\n" CNRM);
    return 1;
  }

  // Create New World
  std::u8string world_name(args);
  Object* world = new Object(body->Parent());
  world->SetShortDesc(world_name);
  world->SetSkill(prhash(u8"Light Source"), 1000);
  world->SetSkill(prhash(u8"Day Length"), 240);
  world->SetSkill(prhash(u8"Day Time"), 120);
  if (!world->Parent()->IsAct(act_t::SPECIAL_HOME)) { // If is first world
    world->Parent()->AddAct(act_t::SPECIAL_HOME, world);
  }

  for (const auto& fl : tag_files) {
    infile tags(fl);
    if (tags && world->LoadTagsFrom(tags)) {
      mind->Send(u8"Loaded Tags From: '{}'!\n", fl.path().u8string());
    } else {
      mind->Send(CRED u8"Failed Loading Tags From: '{}'!\n" CNRM, fl.path().u8string());
      delete world;
      return 0;
    }
  }

  zone_links.clear();
  for (const auto& fl : map_files) {
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
