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

#include <unistd.h>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "color.hpp"
#include "commands.hpp"
#include "mind.hpp"
#include "mob.hpp"
#include "object.hpp"
#include "utils.hpp"

void Object::DynamicInit9() {}

void Object::DynamicInit8() {}

void Object::DynamicInit7() {}

void Object::DynamicInit6() {}

void Object::DynamicInit5() {}

void Object::DynamicInit4() {}

void Object::DynamicInit3() {}

void Object::DynamicInit2() { // Cyberpunk City
}

void Object::DynamicInit1() { // Dwarven mine
  const std::string names[] = {
      "An Entrance to a Large Mining Tunnel",
      "A Large Mining Tunnel",
      "A Large Mining Tunnel with a Small Tunnel to One Side",
      "A Large Mining Tunnel",
      "A Large Open Chamber",
      "A Small Mining Tunnel",
      "A Bend in a Small Mining Tunnel",
      "A Fork in a Small Mining Tunnel",
      "A Small Alcove"};
  const std::string descs[] = {
      "This tunnel looks to have been carved centuries ago.  It is so well crafted\n"
      "that you think it will stand as-is for another millenia.\n",
      "This tunnel looks to have been carved centuries ago.  It is so well crafted\n"
      "that you think it will stand as-is for another millenia.\n",
      "This tunnel looks to have been carved centuries ago.  It is so well crafted\n"
      "that you think it will stand as-is for another millenia.  Another less\n"
      "well-supported tunnel leads off in another direction.\n",
      "This tunnel looks to have been carved centuries ago.  It is so well crafted\n"
      "that you think it will stand as-is for another millenia.\n",
      "This is a large natural-looking opening.  Perhaps this is where some of the\n"
      "miners removed a large vein of ore, or maybe this cavern was just here due\n"
      "to natural forces before they even got here.  It seems the dwarves have used\n"
      "this tunnel for more than just mining work - a large cage here looks well\n"
      "used.\n",
      "This tunnel looks to have been carved quickly.  It looks like it might\n"
      "collapse at any moment.\n",
      "This bend in the tunnel looks to have been carved quickly.  It looks like it\n"
      "might collapse at any moment.\n",
      "This tunnel fork looks to have been carved quickly.  It looks like it\n"
      "might collapse at any moment.\n",
      "The tunnel comes to an end in a rough opening here.\n"};

  static MOBType* dwarf_miner;
  static MOBType* dwarf_engineer;
  static MOBType* dwarf_guard;
  static MOBType* dwarf_explorer;
  static MOBType* elf_prisoner;
  if (!dwarf_miner) {
    WeaponType* weap;
    ArmorType* arm;
    ItemType* item;

    dwarf_miner = new MOBType(
        "a dwarf miner",
        "{He} looks pissed.",
        "",
        "M",
        7,
        7,
        4,
        5,
        6,
        7,
        2,
        2,
        4,
        3,
        8,
        7,
        500,
        2001);
    dwarf_miner->Skill(crc32c("Two-Handed Cleaves"), 100, 4);
    dwarf_miner->Skill(crc32c("Accomplishment"), 1300000);
    weap = new WeaponType(
        "a dwarven mining pickaxe",
        "A super-strong, super-sharp, super-heavy pickaxe.",
        "",
        "Two-Handed Cleaves",
        2,
        2,
        7,
        3,
        3,
        20000,
        50,
        2000);
    dwarf_miner->Arm(weap);
    arm = new ArmorType(
        "a dwarven leather jerkin",
        "A heavy dwarven leather jerkin.  It'll probably stop an arrow.",
        "",
        3,
        2,
        2,
        2,
        0,
        0,
        1,
        0,
        10000,
        10,
        150,
        act_t::WEAR_CHEST,
        act_t::WEAR_BACK);
    dwarf_miner->Armor(arm);

    dwarf_engineer = new MOBType(
        "a dwarf engineer",
        "She looks pissed.",
        "",
        "F",
        5,
        7,
        4,
        5,
        5,
        7,
        3,
        2,
        5,
        3,
        8,
        7,
        2000,
        8001);
    dwarf_engineer->Skill(crc32c("Long Cleaves"), 100, 2);
    dwarf_engineer->Skill(crc32c("Accomplishment"), 1300001);
    weap = new WeaponType(
        "a dwarven combat axe",
        "A super-strong, super-sharp combat axe.",
        "",
        "Long Cleaves",
        1,
        2,
        7,
        2,
        2,
        4000,
        10,
        1000);
    dwarf_engineer->Arm(weap);
    arm = new ArmorType(
        "a dwarven leather jerkin",
        "A heavy dwarven leather jerkin.  It'll probably stop an arrow.",
        "",
        3,
        2,
        2,
        2,
        0,
        0,
        1,
        0,
        10000,
        10,
        150,
        act_t::WEAR_CHEST,
        act_t::WEAR_BACK);
    dwarf_engineer->Armor(arm);

    dwarf_guard = new MOBType(
        "a dwarf guard",
        "{He} looks pissed.",
        "",
        "MF",
        9,
        4,
        6,
        4,
        9,
        4,
        1,
        3,
        5,
        4,
        9,
        4,
        100,
        401);
    dwarf_guard->Skill(crc32c("Two-Handed Cleaves"), 100, 4);
    dwarf_guard->Skill(crc32c("Accomplishment"), 1300002);
    weap = new WeaponType(
        "a dwarven war axe",
        "A super-strong, super-sharp, super-heavy, high-quality war axe.",
        "",
        "Two-Handed Cleaves",
        2,
        4,
        6,
        4,
        2,
        20000,
        40,
        5000);
    dwarf_guard->Arm(weap);
    arm = new ArmorType(
        "a dwarven heavy breastplate",
        "A heavy dwarven breastplate.  It'll probably stop a warhammer.",
        "",
        8,
        4,
        6,
        2,
        4,
        2,
        10,
        5,
        200000,
        100,
        15000,
        act_t::WEAR_CHEST,
        act_t::WEAR_BACK);
    dwarf_guard->Armor(arm);
    arm = new ArmorType(
        "a dwarven great helm",
        "A heavy dwarven great helm.  It'll probably stop a warhammer.",
        "",
        8,
        4,
        6,
        2,
        4,
        2,
        10,
        5,
        50000,
        70,
        5000,
        act_t::WEAR_HEAD);
    dwarf_guard->Armor(arm);
    arm = new ArmorType(
        "a dwarven arm plate (right)",
        "A heavy dwarven arm plate.  It'll probably stop a warhammer.",
        "",
        8,
        4,
        6,
        2,
        4,
        2,
        10,
        5,
        50000,
        60,
        5000,
        act_t::WEAR_RARM);
    dwarf_guard->Armor(arm);
    arm = new ArmorType(
        "a dwarven arm plate (left)",
        "A heavy dwarven arm plate.  It'll probably stop a warhammer.",
        "",
        8,
        4,
        6,
        2,
        4,
        2,
        10,
        5,
        50000,
        60,
        5000,
        act_t::WEAR_LARM);
    dwarf_guard->Armor(arm);
    arm = new ArmorType(
        "a dwarven battle skirt",
        "A heavy dwarven battle skirt.  It'll probably stop a warhammer.",
        "",
        8,
        4,
        6,
        2,
        4,
        2,
        10,
        5,
        100000,
        80,
        10000,
        act_t::WEAR_RLEG,
        act_t::WEAR_LLEG);
    dwarf_guard->Armor(arm);
    item = new ItemType(
        "a key",
        "A heavy steel key.  It looks dwarven, and very old.",
        "",
        {skill_pair{crc32c("Key"), 1310003}},
        100,
        10,
        20);
    dwarf_guard->Carry(item);

    dwarf_explorer = new MOBType(
        "a dwarf explorer",
        "{He} looks pissed.",
        "",
        "MF",
        5,
        4,
        5,
        4,
        6,
        4,
        3,
        2,
        6,
        4,
        9,
        5,
        1000,
        4001);
    dwarf_explorer->Skill(crc32c("Long Cleaves"), 100, 4);
    dwarf_explorer->Skill(crc32c("Accomplishment"), 1300003);
    weap = new WeaponType(
        "a dwarven climbing pick",
        "A super-sharp, lightweight pick.",
        "",
        "Long Cleaves",
        1,
        1,
        3,
        3,
        1,
        2000,
        10,
        500);
    dwarf_explorer->Arm(weap);
    arm = new ArmorType(
        "a dwarven leather jerkin",
        "A heavy dwarven leather jerkin.  It'll probably stop an arrow.",
        "",
        3,
        2,
        2,
        2,
        0,
        0,
        1,
        0,
        10000,
        10,
        150,
        act_t::WEAR_CHEST,
        act_t::WEAR_BACK);
    dwarf_explorer->Armor(arm);

    elf_prisoner = new MOBType(
        "an elf slave",
        "This elf looks like {he}'s been a "
        "prisoner longer than you've been alive.",
        "",
        "MF",
        4,
        2,
        8,
        4,
        5,
        2,
        8,
        2,
        7,
        4,
        4,
        4,
        0,
        0);
    elf_prisoner->Skill(crc32c("Carromeleg - Tier I"), 2);
    elf_prisoner->Skill(crc32c("Carromeleg - Tier II"), 2);
    elf_prisoner->Skill(crc32c("Carromeleg - Tier III"), 2);
    elf_prisoner->Skill(crc32c("Carromeleg - Tier IV"), 2);
    elf_prisoner->Skill(crc32c("Carromeleg - Tier V"), 2);
    elf_prisoner->Skill(crc32c("Carromeleg - Tier VI"), 2);
    elf_prisoner->Skill(crc32c("Accomplishment"), 1300004);
  }

  int mojo = Skill(crc32c("DynamicMojo"));
  SetSkill(crc32c("DynamicMojo"), 0);

  // Tree structure, start with one door - dir of travel is opposite it.
  std::string dirb = "south";
  std::string dir = "north";
  std::string dir2 = "west";
  std::string dir3 = "east";

  if (PickObject("north", LOC_INTERNAL) != nullptr) {
    dirb = "north";
    dir = "south";
    dir2 = "east";
    dir3 = "west";
  } else if (PickObject("east", LOC_INTERNAL) != nullptr) {
    dirb = "east";
    dir = "west";
    dir2 = "south";
    dir3 = "north";
  } else if (PickObject("west", LOC_INTERNAL) != nullptr) {
    dirb = "west";
    dir = "east";
    dir2 = "north";
    dir3 = "south";
  }
  // Up and Down shafts are handled specially.

  switch (Skill(crc32c("DynamicPhase"))) {
    case (0): { // Entrance
      if (mojo == 0)
        mojo = 100000;

      SetSkill(crc32c("Accomplishment"), 1310000);

      Object* next = new Object(parent);
      next->SetShortDesc(names[1]);
      next->SetDesc(descs[1]);
      next->SetSkill(crc32c("DynamicInit"), 1);
      next->SetSkill(crc32c("DynamicPhase"), 1); // Major Shaft
      next->SetSkill(crc32c("DynamicMojo"), mojo - 1000);

      Object* door1 = new Object(this);
      Object* door2 = new Object(next);
      door1->SetShortDesc(dir);
      door2->SetShortDesc(dirb);
      door1->SetDesc((std::string("You see a solid passage leading ") + dir + ".\n").c_str());
      door2->SetDesc((std::string("You see a solid passage leading ") + dirb + ".\n").c_str());
      door1->AddAct(act_t::SPECIAL_LINKED, door2);
      door1->AddAct(act_t::SPECIAL_MASTER, door2);
      door1->SetSkill(crc32c("Open"), 1000);
      door1->SetSkill(crc32c("Enterable"), 1);
      door2->AddAct(act_t::SPECIAL_LINKED, door1);
      door2->AddAct(act_t::SPECIAL_MASTER, door1);
      door2->SetSkill(crc32c("Open"), 1000);
      door2->SetSkill(crc32c("Enterable"), 1);

    } break;
    case (1): { // Major Shaft
      if ((rand() % 100) < 30) {
        mojo -= 500;
        AddMOB(dwarf_miner);
      }
      if ((rand() % 100) < 10) {
        mojo -= 500;
        AddMOB(dwarf_engineer);
      }
      if ((rand() % 100) < 20) {
        mojo -= 500;
        AddMOB(dwarf_guard);
      }
      if ((rand() % 100) < 20) {
        mojo -= 500;
        AddMOB(dwarf_guard);
      }

      if (mojo <= 0)
        break; // End of Tunnel

      int ntypes[] = {1, 1, 1, 1, 1, 1, 1, 2, 2, 3, 4};
      int ntype = ntypes[rand() % (sizeof(ntypes) / sizeof(int))];

      Object* next = new Object(parent);
      next->SetShortDesc(names[ntype]);
      next->SetDesc(descs[ntype]);
      next->SetSkill(crc32c("DynamicInit"), 1);
      next->SetSkill(crc32c("DynamicPhase"), ntype);
      next->SetSkill(crc32c("DynamicMojo"), mojo - 1000);

      Object* door1 = new Object(this);
      Object* door2 = new Object(next);
      door1->SetShortDesc(dir);
      door2->SetShortDesc(dirb);
      door1->SetDesc((std::string("You see a solid passage leading ") + dir + ".\n").c_str());
      door2->SetDesc((std::string("You see a solid passage leading ") + dirb + ".\n").c_str());
      door1->AddAct(act_t::SPECIAL_LINKED, door2);
      door1->AddAct(act_t::SPECIAL_MASTER, door2);
      door1->SetSkill(crc32c("Open"), 1000);
      door1->SetSkill(crc32c("Enterable"), 1);
      door2->AddAct(act_t::SPECIAL_LINKED, door1);
      door2->AddAct(act_t::SPECIAL_MASTER, door1);
      door2->SetSkill(crc32c("Open"), 1000);
      door2->SetSkill(crc32c("Enterable"), 1);
    } break;
    case (2): { // Major Shaft w/ Minor Offshoot
      if ((rand() % 100) < 30) {
        mojo -= 500;
        AddMOB(dwarf_miner);
      }
      if ((rand() % 100) < 10) {
        mojo -= 500;
        AddMOB(dwarf_engineer);
      }
      if ((rand() % 100) < 20) {
        mojo -= 500;
        AddMOB(dwarf_guard);
      }
      if ((rand() % 100) < 20) {
        mojo -= 500;
        AddMOB(dwarf_guard);
      }

      if (mojo <= 0)
        break; // End of Tunnel

      int ntypes[] = {1, 1, 1, 1, 1, 1, 1, 2, 2, 3, 4};
      int ntype = ntypes[rand() % (sizeof(ntypes) / sizeof(int))];

      Object* next = new Object(parent);
      next->SetShortDesc(names[ntype]);
      next->SetDesc(descs[ntype]);
      next->SetSkill(crc32c("DynamicInit"), 1);
      next->SetSkill(crc32c("DynamicPhase"), ntype);
      next->SetSkill(crc32c("DynamicMojo"), (mojo - 1000) * 9 / 10);

      Object* door1 = new Object(this);
      Object* door2 = new Object(next);
      door1->SetShortDesc(dir);
      door2->SetShortDesc(dirb);
      door1->SetDesc((std::string("You see a solid passage leading ") + dir + ".\n").c_str());
      door2->SetDesc((std::string("You see a solid passage leading ") + dirb + ".\n").c_str());
      door1->AddAct(act_t::SPECIAL_LINKED, door2);
      door1->AddAct(act_t::SPECIAL_MASTER, door2);
      door1->SetSkill(crc32c("Open"), 1000);
      door1->SetSkill(crc32c("Enterable"), 1);
      door2->AddAct(act_t::SPECIAL_LINKED, door1);
      door2->AddAct(act_t::SPECIAL_MASTER, door1);
      door2->SetSkill(crc32c("Open"), 1000);
      door2->SetSkill(crc32c("Enterable"), 1);

      if (rand() % 2)
        std::swap(dir2, dir3); // Half left, half right
      next = new Object(parent);
      next->SetShortDesc(names[5]);
      next->SetDesc(descs[5]);
      next->SetSkill(crc32c("DynamicInit"), 1);
      next->SetSkill(crc32c("DynamicPhase"), 5); // Minor Shaft
      next->SetSkill(crc32c("DynamicMojo"), (mojo - 1000) / 10);

      door1 = new Object(this);
      door2 = new Object(next);
      door1->SetShortDesc(dir2);
      door2->SetShortDesc(dir3);
      door1->SetDesc((std::string("You see a crumbling passage leading ") + dir2 + ".\n").c_str());
      door2->SetDesc((std::string("You see a crumbling passage leading ") + dir3 + ".\n").c_str());
      door1->AddAct(act_t::SPECIAL_LINKED, door2);
      door1->AddAct(act_t::SPECIAL_MASTER, door2);
      door1->SetSkill(crc32c("Open"), 1000);
      door1->SetSkill(crc32c("Enterable"), 1);
      door2->AddAct(act_t::SPECIAL_LINKED, door1);
      door2->AddAct(act_t::SPECIAL_MASTER, door1);
      door2->SetSkill(crc32c("Open"), 1000);
      door2->SetSkill(crc32c("Enterable"), 1);
    } break;
    case (3): { // Major Shaft w/ Secret Minor Offshoot
      if ((rand() % 100) < 30) {
        mojo -= 500;
        AddMOB(dwarf_miner);
      }
      if ((rand() % 100) < 10) {
        mojo -= 500;
        AddMOB(dwarf_engineer);
      }
      if ((rand() % 100) < 20) {
        mojo -= 500;
        AddMOB(dwarf_guard);
      }
      if ((rand() % 100) < 20) {
        mojo -= 500;
        AddMOB(dwarf_guard);
      }

      if (mojo <= 0)
        break; // End of Tunnel

      int ntypes[] = {1, 1, 1, 1, 1, 1, 1, 2, 2, 3, 4};
      int ntype = ntypes[rand() % (sizeof(ntypes) / sizeof(int))];

      Object* next = new Object(parent);
      next->SetShortDesc(names[ntype]);
      next->SetDesc(descs[ntype]);
      next->SetSkill(crc32c("DynamicInit"), 1);
      next->SetSkill(crc32c("DynamicPhase"), ntype);
      next->SetSkill(crc32c("DynamicMojo"), (mojo - 1000) * 3 / 4);

      Object* door1 = new Object(this);
      Object* door2 = new Object(next);
      door1->SetShortDesc(dir);
      door2->SetShortDesc(dirb);
      door1->SetDesc((std::string("You see a solid passage leading ") + dir + ".\n").c_str());
      door2->SetDesc((std::string("You see a solid passage leading ") + dirb + ".\n").c_str());
      door1->AddAct(act_t::SPECIAL_LINKED, door2);
      door1->AddAct(act_t::SPECIAL_MASTER, door2);
      door1->SetSkill(crc32c("Open"), 1000);
      door1->SetSkill(crc32c("Enterable"), 1);
      door2->AddAct(act_t::SPECIAL_LINKED, door1);
      door2->AddAct(act_t::SPECIAL_MASTER, door1);
      door2->SetSkill(crc32c("Open"), 1000);
      door2->SetSkill(crc32c("Enterable"), 1);

      if (rand() % 2)
        std::swap(dir2, dir3); // Half left, half right
      next = new Object(parent);
      next->SetShortDesc(names[5]);
      next->SetDesc(descs[5]);
      next->SetSkill(crc32c("DynamicInit"), 1);
      next->SetSkill(crc32c("DynamicPhase"), 5); // Minor Shaft
      next->SetSkill(crc32c("DynamicMojo"), (mojo - 1000) / 4);
      next->SetSkill(crc32c("Accomplishment"), 1310002);

      door1 = new Object(this);
      door2 = new Object(next);
      door1->SetShortDesc(dir2);
      door2->SetShortDesc(dir3);
      door1->SetDesc((std::string("You see a crumbling passage leading ") + dir2 + ".\n").c_str());
      door2->SetDesc((std::string("You see a crumbling passage leading ") + dir3 + ".\n").c_str());
      door1->AddAct(act_t::SPECIAL_LINKED, door2);
      door1->AddAct(act_t::SPECIAL_MASTER, door2);
      door1->SetSkill(crc32c("Open"), 1000);
      door1->SetSkill(crc32c("Enterable"), 1);
      door2->AddAct(act_t::SPECIAL_LINKED, door1);
      door2->AddAct(act_t::SPECIAL_MASTER, door1);
      door2->SetSkill(crc32c("Open"), 1000);
      door2->SetSkill(crc32c("Enterable"), 1);
      door1->SetSkill(crc32c("Hidden"), 4 + rand() % 13);
    } break;
    case (4): { // Major Chamber
      if ((rand() % 100) < 30) {
        mojo -= 500;
        AddMOB(dwarf_miner);
      }
      if ((rand() % 100) < 30) {
        mojo -= 500;
        AddMOB(dwarf_miner);
      }
      if ((rand() % 100) < 30) {
        mojo -= 500;
        AddMOB(dwarf_miner);
      }
      if ((rand() % 100) < 30) {
        mojo -= 500;
        AddMOB(dwarf_miner);
      }
      if ((rand() % 100) < 10) {
        mojo -= 500;
        AddMOB(dwarf_engineer);
      }
      if ((rand() % 100) < 10) {
        mojo -= 500;
        AddMOB(dwarf_engineer);
      }
      if ((rand() % 100) < 20) {
        mojo -= 500;
        AddMOB(dwarf_guard);
      }
      if ((rand() % 100) < 20) {
        mojo -= 500;
        AddMOB(dwarf_guard);
      }
      if ((rand() % 100) < 20) {
        mojo -= 500;
        AddMOB(dwarf_guard);
      }
      if ((rand() % 100) < 20) {
        mojo -= 500;
        AddMOB(dwarf_guard);
      }

      Object* cage = new Object(this);
      cage->SetShortDesc("an old strong cage");
      cage->SetDesc(
          "This is a very old, large, rugged cage - probably built by "
          "dwarves.\n");
      cage->SetSkill(crc32c("Transparent"), 900);
      cage->SetSkill(crc32c("Container"), 100000);
      cage->SetSkill(crc32c("Locked"), 1);
      cage->SetSkill(crc32c("Lock"), 1310003);
      cage->SetSkill(crc32c("Closeable"), 1);
      cage->SetSkill(crc32c("Accomplishment"), 1310003);

      if ((rand() % 100) < 20) {
        mojo -= 500;
        cage->AddMOB(elf_prisoner);
      }
      if ((rand() % 100) < 20) {
        mojo -= 500;
        cage->AddMOB(elf_prisoner);
      }

      if (mojo <= 0)
        break; // End of Tunnel

      int ntypes[] = {1, 1, 1, 1, 1, 1, 1, 2, 2, 3};
      int ntype = ntypes[rand() % (sizeof(ntypes) / sizeof(int))];

      Object* next = new Object(parent);
      next->SetShortDesc(names[ntype]);
      next->SetDesc(descs[ntype]);
      next->SetSkill(crc32c("DynamicInit"), 1);
      next->SetSkill(crc32c("DynamicPhase"), ntype);
      next->SetSkill(crc32c("DynamicMojo"), mojo - 1000);

      Object* door1 = new Object(this);
      Object* door2 = new Object(next);
      door1->SetShortDesc(dir);
      door2->SetShortDesc(dirb);
      door1->SetDesc((std::string("You see a solid passage leading ") + dir + ".\n").c_str());
      door2->SetDesc((std::string("You see a solid passage leading ") + dirb + ".\n").c_str());
      door1->AddAct(act_t::SPECIAL_LINKED, door2);
      door1->AddAct(act_t::SPECIAL_MASTER, door2);
      door1->SetSkill(crc32c("Open"), 1000);
      door1->SetSkill(crc32c("Enterable"), 1);
      door2->AddAct(act_t::SPECIAL_LINKED, door1);
      door2->AddAct(act_t::SPECIAL_MASTER, door1);
      door2->SetSkill(crc32c("Open"), 1000);
      door2->SetSkill(crc32c("Enterable"), 1);
    } break;
    case (5): { // Minor Shaft
      if ((rand() % 100) < 30) {
        mojo -= 500;
        AddMOB(dwarf_miner);
      }
      if ((rand() % 100) < 30) {
        mojo -= 500;
        AddMOB(dwarf_miner);
      }
      if ((rand() % 100) < 10) {
        mojo -= 500;
        AddMOB(dwarf_engineer);
      }
      if ((rand() % 100) < 20) {
        mojo -= 500;
        AddMOB(dwarf_explorer);
      }

      if (mojo <= 0)
        break; // End of Tunnel

      //      int ntypes[] = { 5, 5, 5, 5, 6, 6, 6, 7, 7, 8, 9 };
      int ntypes[] = {5, 5, 5, 5, 5, 6, 6, 7, 7, 8};
      int ntype = ntypes[rand() % (sizeof(ntypes) / sizeof(int))];

      Object* next = new Object(parent);
      next->SetShortDesc(names[ntype]);
      next->SetDesc(descs[ntype]);
      next->SetSkill(crc32c("DynamicInit"), 1);
      next->SetSkill(crc32c("DynamicPhase"), ntype);
      next->SetSkill(crc32c("DynamicMojo"), mojo - 100);

      Object* door1 = new Object(this);
      Object* door2 = new Object(next);
      door1->SetShortDesc(dir);
      door2->SetShortDesc(dirb);
      door1->SetDesc((std::string("You see a crumbling passage leading ") + dir + ".\n").c_str());
      door2->SetDesc((std::string("You see a crumbling passage leading ") + dirb + ".\n").c_str());
      door1->AddAct(act_t::SPECIAL_LINKED, door2);
      door1->AddAct(act_t::SPECIAL_MASTER, door2);
      door1->SetSkill(crc32c("Open"), 1000);
      door1->SetSkill(crc32c("Enterable"), 1);
      door2->AddAct(act_t::SPECIAL_LINKED, door1);
      door2->AddAct(act_t::SPECIAL_MASTER, door1);
      door2->SetSkill(crc32c("Open"), 1000);
      door2->SetSkill(crc32c("Enterable"), 1);
    } break;
    case (6): { // Minor Shaft (Bend)
      if ((rand() % 100) < 30) {
        mojo -= 500;
        AddMOB(dwarf_miner);
      }
      if ((rand() % 100) < 30) {
        mojo -= 500;
        AddMOB(dwarf_miner);
      }
      if ((rand() % 100) < 10) {
        mojo -= 500;
        AddMOB(dwarf_engineer);
      }
      if ((rand() % 100) < 20) {
        mojo -= 500;
        AddMOB(dwarf_explorer);
      }

      //      int ntypes[] = { 5, 5, 5, 5, 5, 6, 6, 7, 7, 8, 9 };
      int ntypes[] = {5, 5, 5, 5, 5, 6, 6, 7, 7, 8};
      int ntype = ntypes[rand() % (sizeof(ntypes) / sizeof(int))];

      if (rand() % 2)
        std::swap(dir2, dir3); // Half left, half right
      Object* next = new Object(parent);
      next->SetShortDesc(names[ntype]);
      next->SetDesc(descs[ntype]);
      next->SetSkill(crc32c("DynamicInit"), 1);
      next->SetSkill(crc32c("DynamicPhase"), ntype);
      next->SetSkill(crc32c("DynamicMojo"), mojo - 100);

      Object* door1 = new Object(this);
      Object* door2 = new Object(next);
      door1->SetShortDesc(dir2);
      door2->SetShortDesc(dir3);
      door1->SetDesc((std::string("You see a crumbling passage leading ") + dir2 + ".\n").c_str());
      door2->SetDesc((std::string("You see a crumbling passage leading ") + dir3 + ".\n").c_str());
      door1->AddAct(act_t::SPECIAL_LINKED, door2);
      door1->AddAct(act_t::SPECIAL_MASTER, door2);
      door1->SetSkill(crc32c("Open"), 1000);
      door1->SetSkill(crc32c("Enterable"), 1);
      door2->AddAct(act_t::SPECIAL_LINKED, door1);
      door2->AddAct(act_t::SPECIAL_MASTER, door1);
      door2->SetSkill(crc32c("Open"), 1000);
      door2->SetSkill(crc32c("Enterable"), 1);
    } break;
    case (7): { // Minor Shaft Fork
      if ((rand() % 100) < 30) {
        mojo -= 500;
        AddMOB(dwarf_miner);
      }
      if ((rand() % 100) < 30) {
        mojo -= 500;
        AddMOB(dwarf_miner);
      }
      if ((rand() % 100) < 10) {
        mojo -= 500;
        AddMOB(dwarf_engineer);
      }
      if ((rand() % 100) < 20) {
        mojo -= 500;
        AddMOB(dwarf_explorer);
      }

      //      int ntypes[] = { 5, 5, 5, 5, 5, 6, 6, 7, 7, 8, 9 };
      int ntypes[] = {5, 5, 5, 5, 5, 6, 6, 7, 7, 8};
      int ntype = ntypes[rand() % (sizeof(ntypes) / sizeof(int))];

      Object* next = new Object(parent);
      next->SetShortDesc(names[ntype]);
      next->SetDesc(descs[ntype]);
      next->SetSkill(crc32c("DynamicInit"), 1);
      next->SetSkill(crc32c("DynamicPhase"), ntype);
      next->SetSkill(crc32c("DynamicMojo"), (mojo - 100) / 2);

      Object* door1 = new Object(this);
      Object* door2 = new Object(next);
      door1->SetShortDesc(dir2);
      door2->SetShortDesc(dir3);
      door1->SetDesc((std::string("You see a crumbling passage leading ") + dir2 + ".\n").c_str());
      door2->SetDesc((std::string("You see a crumbling passage leading ") + dir3 + ".\n").c_str());
      door1->AddAct(act_t::SPECIAL_LINKED, door2);
      door1->AddAct(act_t::SPECIAL_MASTER, door2);
      door1->SetSkill(crc32c("Open"), 1000);
      door1->SetSkill(crc32c("Enterable"), 1);
      door2->AddAct(act_t::SPECIAL_LINKED, door1);
      door2->AddAct(act_t::SPECIAL_MASTER, door1);
      door2->SetSkill(crc32c("Open"), 1000);
      door2->SetSkill(crc32c("Enterable"), 1);

      ntype = ntypes[rand() % (sizeof(ntypes) / sizeof(int))];
      std::swap(dir2, dir3); // Same code, opposite dir.
      next = new Object(parent);
      next->SetShortDesc(names[ntype]);
      next->SetDesc(descs[ntype]);
      next->SetSkill(crc32c("DynamicInit"), 1);
      next->SetSkill(crc32c("DynamicPhase"), ntype);
      next->SetSkill(crc32c("DynamicMojo"), (mojo - 100) / 2);

      door1 = new Object(this);
      door2 = new Object(next);
      door1->SetShortDesc(dir2);
      door2->SetShortDesc(dir3);
      door1->SetDesc((std::string("You see a crumbling passage leading ") + dir2 + ".\n").c_str());
      door2->SetDesc((std::string("You see a crumbling passage leading ") + dir3 + ".\n").c_str());
      door1->AddAct(act_t::SPECIAL_LINKED, door2);
      door1->AddAct(act_t::SPECIAL_MASTER, door2);
      door1->SetSkill(crc32c("Open"), 1000);
      door1->SetSkill(crc32c("Enterable"), 1);
      door2->AddAct(act_t::SPECIAL_LINKED, door1);
      door2->AddAct(act_t::SPECIAL_MASTER, door1);
      door2->SetSkill(crc32c("Open"), 1000);
      door2->SetSkill(crc32c("Enterable"), 1);
    } break;
    case (8): { // Minor Shaft Alcove
      if ((rand() % 100) < 30) {
        mojo -= 500;
        AddMOB(dwarf_miner);
      }
      if ((rand() % 100) < 30) {
        mojo -= 500;
        AddMOB(dwarf_miner);
      }
      if ((rand() % 100) < 10) {
        mojo -= 500;
        AddMOB(dwarf_engineer);
      }
      if ((rand() % 100) < 20) {
        mojo -= 500;
        AddMOB(dwarf_explorer);
      }

      if (mojo <= 0)
        break; // End of Tunnel

      if (mojo > 10000 && (rand() % 100) < 20) {
        mojo -= 5000;
        int option = rand() % 100;
        if (option >= 70) {
          std::swap(dir, dir2);
          std::swap(dirb, dir3);
        } else if (option >= 70) {
          std::swap(dir, dir3);
          std::swap(dirb, dir2);
        }
      } else { // No secret tunnels here!
        break;
      }

      int ntype = 5;

      Object* next = new Object(parent);
      next->SetShortDesc(names[ntype]);
      next->SetDesc(descs[ntype]);
      next->SetSkill(crc32c("DynamicInit"), 1);
      next->SetSkill(crc32c("DynamicPhase"), ntype);
      next->SetSkill(crc32c("DynamicMojo"), mojo - 500);
      next->SetSkill(crc32c("Accomplishment"), 1310001);

      Object* door1 = new Object(this);
      Object* door2 = new Object(next);
      door1->SetShortDesc(dir);
      door2->SetShortDesc(dirb);
      door1->SetDesc((std::string("You see a crumbling passage leading ") + dir + ".\n").c_str());
      door2->SetDesc((std::string("You see a crumbling passage leading ") + dirb + ".\n").c_str());
      door1->AddAct(act_t::SPECIAL_LINKED, door2);
      door1->AddAct(act_t::SPECIAL_MASTER, door2);
      door1->SetSkill(crc32c("Open"), 1000);
      door1->SetSkill(crc32c("Enterable"), 1);
      door2->AddAct(act_t::SPECIAL_LINKED, door1);
      door2->AddAct(act_t::SPECIAL_MASTER, door1);
      door2->SetSkill(crc32c("Open"), 1000);
      door2->SetSkill(crc32c("Enterable"), 1);
      door1->SetSkill(crc32c("Hidden"), 4 + rand() % 13);
    } break;
    default: {
      fprintf(
          stderr,
          "Unknown dynamic-phase-type (%d-%d) init requested!\n",
          Skill(crc32c("DynamicInit")),
          Skill(crc32c("DynamicPhase")));
    } break;
  }
  SetSkill(crc32c("DynamicPhase"), 0);
}

void Object::DynamicInit() {
  switch (Skill(crc32c("DynamicInit"))) {
    case (2): { // Cyberpunk City
      DynamicInit2();
    } break;
    case (1): { // Dwarven Mine
      DynamicInit1();
    } break;
    default: {
      fprintf(stderr, "Unknown dynamic-type (%d) init requested!\n", Skill(crc32c("DynamicInit")));
    } break;
  }
  SetSkill(crc32c("DynamicInit"), 0);
}
