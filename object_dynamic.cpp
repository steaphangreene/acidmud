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
#include <random>
#include <string>

#include "color.hpp"
#include "commands.hpp"
#include "log.hpp"
#include "mind.hpp"
#include "npc.hpp"
#include "object.hpp"
#include "properties.hpp"
#include "utils.hpp"

static std::random_device rd;
static std::mt19937 gen(rd());

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
  const std::u8string names[] = {
      u8"An Entrance to a Large Mining Tunnel",
      u8"A Large Mining Tunnel",
      u8"A Large Mining Tunnel with a Small Tunnel to One Side",
      u8"A Large Mining Tunnel",
      u8"A Large Open Chamber",
      u8"A Small Mining Tunnel",
      u8"A Bend in a Small Mining Tunnel",
      u8"A Fork in a Small Mining Tunnel",
      u8"A Small Alcove"};
  const std::u8string descs[] = {
      u8"This tunnel looks to have been carved centuries ago.  It is so well crafted\n"
      u8"that you think it will stand as-is for another millenia.\n",
      u8"This tunnel looks to have been carved centuries ago.  It is so well crafted\n"
      u8"that you think it will stand as-is for another millenia.\n",
      u8"This tunnel looks to have been carved centuries ago.  It is so well crafted\n"
      u8"that you think it will stand as-is for another millenia.  Another less\n"
      u8"well-supported tunnel leads off in another direction.\n",
      u8"This tunnel looks to have been carved centuries ago.  It is so well crafted\n"
      u8"that you think it will stand as-is for another millenia.\n",
      u8"This is a large natural-looking opening.  Perhaps this is where some of the\n"
      u8"miners removed a large vein of ore, or maybe this cavern was just here due\n"
      u8"to natural forces before they even got here.  It seems the dwarves have used\n"
      u8"this tunnel for more than just mining work - a large cage here looks well\n"
      u8"used.\n",
      u8"This tunnel looks to have been carved quickly.  It looks like it might\n"
      u8"collapse at any moment.\n",
      u8"This bend in the tunnel looks to have been carved quickly.  It looks like it\n"
      u8"might collapse at any moment.\n",
      u8"This tunnel fork looks to have been carved quickly.  It looks like it\n"
      u8"might collapse at any moment.\n",
      u8"The tunnel comes to an end in a rough opening here.\n"};

  static NPCType* dwarf_miner;
  static NPCType* dwarf_engineer;
  static NPCType* dwarf_guard;
  static NPCType* dwarf_explorer;
  static NPCType* elf_prisoner;
  if (!dwarf_miner) {
    WeaponType* weap;
    ArmorType* arm;
    ItemType* item;

    dwarf_miner = new NPCType(
        u8"a dwarf miner",
        u8"{He} looks pissed.",
        u8"",
        {gender_t::MALE, gender_t::FEMALE},
        {7, 7, 4, 5, 6, 7},
        {9, 9, 8, 8, 14, 14},
        500,
        2500);
    dwarf_miner->Skill(prhash(u8"Two-Handed Cleaves"), 100, 4);
    dwarf_miner->Skill(prhash(u8"Accomplishment"), 1500000);
    weap = new WeaponType(
        u8"a dwarven mining pickaxe",
        u8"A super-strong, super-sharp, super-heavy pickaxe.",
        u8"",
        u8"Two-Handed Cleaves",
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
        u8"a dwarven leather jerkin",
        u8"A heavy dwarven leather jerkin.  It'll probably stop an arrow.",
        u8"",
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

    dwarf_engineer = new NPCType(
        u8"a dwarf engineer",
        u8"{He} looks pissed.",
        u8"",
        {gender_t::MALE, gender_t::FEMALE},
        {5, 7, 4, 5, 5, 7},
        {8, 9, 9, 8, 13, 14},
        2000,
        10000);
    dwarf_engineer->Skill(prhash(u8"Long Cleaves"), 100, 2);
    dwarf_engineer->Skill(prhash(u8"Accomplishment"), 1500001);
    weap = new WeaponType(
        u8"a dwarven combat axe",
        u8"A super-strong, super-sharp combat axe.",
        u8"",
        u8"Long Cleaves",
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
        u8"a dwarven leather jerkin",
        u8"A heavy dwarven leather jerkin.  It'll probably stop an arrow.",
        u8"",
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

    dwarf_guard = new NPCType(
        u8"a dwarf guard",
        u8"{He} looks pissed.",
        u8"",
        {gender_t::MALE, gender_t::FEMALE},
        {9, 4, 6, 4, 9, 4},
        {10, 7, 11, 8, 18, 8},
        100,
        500);
    dwarf_guard->Skill(prhash(u8"Two-Handed Cleaves"), 100, 4);
    dwarf_guard->Skill(prhash(u8"Accomplishment"), 1500002);
    weap = new WeaponType(
        u8"a dwarven war axe",
        u8"A super-strong, super-sharp, super-heavy, high-quality war axe.",
        u8"",
        u8"Two-Handed Cleaves",
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
        u8"a dwarven heavy breastplate",
        u8"A heavy dwarven breastplate.  It'll probably stop a warhammer.",
        u8"",
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
        u8"a dwarven great helm",
        u8"A heavy dwarven great helm.  It'll probably stop a warhammer.",
        u8"",
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
        u8"a dwarven arm plate (right)",
        u8"A heavy dwarven arm plate.  It'll probably stop a warhammer.",
        u8"",
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
        u8"a dwarven arm plate (left)",
        u8"A heavy dwarven arm plate.  It'll probably stop a warhammer.",
        u8"",
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
        u8"a dwarven battle skirt",
        u8"A heavy dwarven battle skirt.  It'll probably stop a warhammer.",
        u8"",
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
        u8"a key",
        u8"A heavy steel key.  It looks dwarven, and very old.",
        u8"",
        {skill_pair{prhash(u8"Key"), 1510003}},
        100,
        10,
        20);
    dwarf_guard->Carry(item);

    dwarf_explorer = new NPCType(
        u8"a dwarf explorer",
        u8"{He} looks pissed.",
        u8"",
        {gender_t::MALE, gender_t::FEMALE},
        {5, 4, 5, 4, 6, 4},
        {8, 6, 11, 8, 17, 9},
        1000,
        5000);
    dwarf_explorer->Skill(prhash(u8"Long Cleaves"), 100, 4);
    dwarf_explorer->Skill(prhash(u8"Accomplishment"), 1500003);
    weap = new WeaponType(
        u8"a dwarven climbing pick",
        u8"A super-sharp, lightweight pick.",
        u8"",
        u8"Long Cleaves",
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
        u8"a dwarven leather jerkin",
        u8"A heavy dwarven leather jerkin.  It'll probably stop an arrow.",
        u8"",
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

    elf_prisoner = new NPCType(
        u8"an elf slave",
        u8"This elf looks like {he}'s been a "
        u8"prisoner longer than you've been alive.",
        u8"",
        {gender_t::MALE, gender_t::FEMALE},
        {4, 2, 8, 4, 5, 2},
        {12, 4, 15, 8, 9, 6});
    elf_prisoner->Skill(prhash(u8"Carromeleg - Tier I"), 2);
    elf_prisoner->Skill(prhash(u8"Carromeleg - Tier II"), 2);
    elf_prisoner->Skill(prhash(u8"Carromeleg - Tier III"), 2);
    elf_prisoner->Skill(prhash(u8"Carromeleg - Tier IV"), 2);
    elf_prisoner->Skill(prhash(u8"Carromeleg - Tier V"), 2);
    elf_prisoner->Skill(prhash(u8"Carromeleg - Tier VI"), 2);
    elf_prisoner->Skill(prhash(u8"Accomplishment"), 1500004);
  }

  int mojo = Skill(prhash(u8"DynamicMojo"));
  ClearSkill(prhash(u8"DynamicMojo"));

  // Tree structure, start with one door - dir of travel is opposite it.
  std::u8string dirb = u8"south";
  std::u8string dir = u8"north";
  std::u8string dir2 = u8"west";
  std::u8string dir3 = u8"east";

  if (PickObject(u8"north", LOC_INTERNAL) != nullptr) {
    dirb = u8"north";
    dir = u8"south";
    dir2 = u8"east";
    dir3 = u8"west";
  } else if (PickObject(u8"east", LOC_INTERNAL) != nullptr) {
    dirb = u8"east";
    dir = u8"west";
    dir2 = u8"south";
    dir3 = u8"north";
  } else if (PickObject(u8"west", LOC_INTERNAL) != nullptr) {
    dirb = u8"west";
    dir = u8"east";
    dir2 = u8"north";
    dir3 = u8"south";
  }
  // Up and Down shafts are handled specially.

  switch (Skill(prhash(u8"DynamicPhase"))) {
    case (0): { // Entrance
      if (mojo == 0)
        mojo = 100000;

      SetSkill(prhash(u8"Accomplishment"), 1510000);

      Object* next = new Object(parent);
      next->SetShortDesc(names[1]);
      next->SetDesc(descs[1]);
      next->SetSkill(prhash(u8"DynamicInit"), 1);
      next->SetSkill(prhash(u8"DynamicPhase"), 1); // Major Shaft
      next->SetSkill(prhash(u8"DynamicMojo"), mojo - 1000);

      Object* door1 = new Object(this);
      Object* door2 = new Object(next);
      door1->SetShortDesc(dir);
      door2->SetShortDesc(dirb);
      door1->SetDesc(fmt::format(u8"You see a solid passage leading {}.\n", dir));
      door2->SetDesc(fmt::format(u8"You see a solid passage leading {}.\n", dirb));
      door1->AddAct(act_t::SPECIAL_LINKED, door2);
      door1->SetSkill(prhash(u8"Open"), 1000);
      door1->SetSkill(prhash(u8"Enterable"), 1);
      door2->AddAct(act_t::SPECIAL_LINKED, door1);
      door2->SetSkill(prhash(u8"Open"), 1000);
      door2->SetSkill(prhash(u8"Enterable"), 1);

    } break;
    case (1): { // Major Shaft
      if ((rand() % 100) < 30) {
        mojo -= 500;
        AddNPC(gen, dwarf_miner);
      }
      if ((rand() % 100) < 10) {
        mojo -= 500;
        AddNPC(gen, dwarf_engineer);
      }
      if ((rand() % 100) < 20) {
        mojo -= 500;
        AddNPC(gen, dwarf_guard);
      }
      if ((rand() % 100) < 20) {
        mojo -= 500;
        AddNPC(gen, dwarf_guard);
      }

      if (mojo <= 0)
        break; // End of Tunnel

      int ntypes[] = {1, 1, 1, 1, 1, 1, 1, 2, 2, 3, 4};
      int ntype = ntypes[rand() % (sizeof(ntypes) / sizeof(int))];

      Object* next = new Object(parent);
      next->SetShortDesc(names[ntype]);
      next->SetDesc(descs[ntype]);
      next->SetSkill(prhash(u8"DynamicInit"), 1);
      next->SetSkill(prhash(u8"DynamicPhase"), ntype);
      next->SetSkill(prhash(u8"DynamicMojo"), mojo - 1000);

      Object* door1 = new Object(this);
      Object* door2 = new Object(next);
      door1->SetShortDesc(dir);
      door2->SetShortDesc(dirb);
      door1->SetDesc(fmt::format(u8"You see a solid passage leading {}.\n", dir));
      door2->SetDesc(fmt::format(u8"You see a solid passage leading {}.\n", dirb));
      door1->AddAct(act_t::SPECIAL_LINKED, door2);
      door1->SetSkill(prhash(u8"Open"), 1000);
      door1->SetSkill(prhash(u8"Enterable"), 1);
      door2->AddAct(act_t::SPECIAL_LINKED, door1);
      door2->SetSkill(prhash(u8"Open"), 1000);
      door2->SetSkill(prhash(u8"Enterable"), 1);
    } break;
    case (2): { // Major Shaft w/ Minor Offshoot
      if ((rand() % 100) < 30) {
        mojo -= 500;
        AddNPC(gen, dwarf_miner);
      }
      if ((rand() % 100) < 10) {
        mojo -= 500;
        AddNPC(gen, dwarf_engineer);
      }
      if ((rand() % 100) < 20) {
        mojo -= 500;
        AddNPC(gen, dwarf_guard);
      }
      if ((rand() % 100) < 20) {
        mojo -= 500;
        AddNPC(gen, dwarf_guard);
      }

      if (mojo <= 0)
        break; // End of Tunnel

      int ntypes[] = {1, 1, 1, 1, 1, 1, 1, 2, 2, 3, 4};
      int ntype = ntypes[rand() % (sizeof(ntypes) / sizeof(int))];

      Object* next = new Object(parent);
      next->SetShortDesc(names[ntype]);
      next->SetDesc(descs[ntype]);
      next->SetSkill(prhash(u8"DynamicInit"), 1);
      next->SetSkill(prhash(u8"DynamicPhase"), ntype);
      next->SetSkill(prhash(u8"DynamicMojo"), (mojo - 1000) * 9 / 10);

      Object* door1 = new Object(this);
      Object* door2 = new Object(next);
      door1->SetShortDesc(dir);
      door2->SetShortDesc(dirb);
      door1->SetDesc(fmt::format(u8"You see a solid passage leading {}.\n", dir));
      door2->SetDesc(fmt::format(u8"You see a solid passage leading {}.\n", dirb));
      door1->AddAct(act_t::SPECIAL_LINKED, door2);
      door1->SetSkill(prhash(u8"Open"), 1000);
      door1->SetSkill(prhash(u8"Enterable"), 1);
      door2->AddAct(act_t::SPECIAL_LINKED, door1);
      door2->SetSkill(prhash(u8"Open"), 1000);
      door2->SetSkill(prhash(u8"Enterable"), 1);

      if (rand() % 2)
        std::swap(dir2, dir3); // Half left, half right
      next = new Object(parent);
      next->SetShortDesc(names[5]);
      next->SetDesc(descs[5]);
      next->SetSkill(prhash(u8"DynamicInit"), 1);
      next->SetSkill(prhash(u8"DynamicPhase"), 5); // Minor Shaft
      next->SetSkill(prhash(u8"DynamicMojo"), (mojo - 1000) / 10);

      door1 = new Object(this);
      door2 = new Object(next);
      door1->SetShortDesc(dir2);
      door2->SetShortDesc(dir3);
      door1->SetDesc(fmt::format(u8"You see a crumbling passage leading {}.\n", dir2));
      door2->SetDesc(fmt::format(u8"You see a crumbling passage leading {}.\n", dir3));
      door1->AddAct(act_t::SPECIAL_LINKED, door2);
      door1->SetSkill(prhash(u8"Open"), 1000);
      door1->SetSkill(prhash(u8"Enterable"), 1);
      door2->AddAct(act_t::SPECIAL_LINKED, door1);
      door2->SetSkill(prhash(u8"Open"), 1000);
      door2->SetSkill(prhash(u8"Enterable"), 1);
    } break;
    case (3): { // Major Shaft w/ Secret Minor Offshoot
      if ((rand() % 100) < 30) {
        mojo -= 500;
        AddNPC(gen, dwarf_miner);
      }
      if ((rand() % 100) < 10) {
        mojo -= 500;
        AddNPC(gen, dwarf_engineer);
      }
      if ((rand() % 100) < 20) {
        mojo -= 500;
        AddNPC(gen, dwarf_guard);
      }
      if ((rand() % 100) < 20) {
        mojo -= 500;
        AddNPC(gen, dwarf_guard);
      }

      if (mojo <= 0)
        break; // End of Tunnel

      int ntypes[] = {1, 1, 1, 1, 1, 1, 1, 2, 2, 3, 4};
      int ntype = ntypes[rand() % (sizeof(ntypes) / sizeof(int))];

      Object* next = new Object(parent);
      next->SetShortDesc(names[ntype]);
      next->SetDesc(descs[ntype]);
      next->SetSkill(prhash(u8"DynamicInit"), 1);
      next->SetSkill(prhash(u8"DynamicPhase"), ntype);
      next->SetSkill(prhash(u8"DynamicMojo"), (mojo - 1000) * 3 / 4);

      Object* door1 = new Object(this);
      Object* door2 = new Object(next);
      door1->SetShortDesc(dir);
      door2->SetShortDesc(dirb);
      door1->SetDesc(fmt::format(u8"You see a solid passage leading {}.\n", dir));
      door2->SetDesc(fmt::format(u8"You see a solid passage leading {}.\n", dirb));
      door1->AddAct(act_t::SPECIAL_LINKED, door2);
      door1->SetSkill(prhash(u8"Open"), 1000);
      door1->SetSkill(prhash(u8"Enterable"), 1);
      door2->AddAct(act_t::SPECIAL_LINKED, door1);
      door2->SetSkill(prhash(u8"Open"), 1000);
      door2->SetSkill(prhash(u8"Enterable"), 1);

      if (rand() % 2)
        std::swap(dir2, dir3); // Half left, half right
      next = new Object(parent);
      next->SetShortDesc(names[5]);
      next->SetDesc(descs[5]);
      next->SetSkill(prhash(u8"DynamicInit"), 1);
      next->SetSkill(prhash(u8"DynamicPhase"), 5); // Minor Shaft
      next->SetSkill(prhash(u8"DynamicMojo"), (mojo - 1000) / 4);
      next->SetSkill(prhash(u8"Accomplishment"), 1510002);

      door1 = new Object(this);
      door2 = new Object(next);
      door1->SetShortDesc(dir2);
      door2->SetShortDesc(dir3);
      door1->SetDesc(fmt::format(u8"You see a crumbling passage leading {}.\n", dir2));
      door2->SetDesc(fmt::format(u8"You see a crumbling passage leading {}.\n", dir3));
      door1->AddAct(act_t::SPECIAL_LINKED, door2);
      door1->SetSkill(prhash(u8"Open"), 1000);
      door1->SetSkill(prhash(u8"Enterable"), 1);
      door2->AddAct(act_t::SPECIAL_LINKED, door1);
      door2->SetSkill(prhash(u8"Open"), 1000);
      door2->SetSkill(prhash(u8"Enterable"), 1);
      door1->SetSkill(prhash(u8"Hidden"), 4 + rand() % 13);
    } break;
    case (4): { // Major Chamber
      if ((rand() % 100) < 30) {
        mojo -= 500;
        AddNPC(gen, dwarf_miner);
      }
      if ((rand() % 100) < 30) {
        mojo -= 500;
        AddNPC(gen, dwarf_miner);
      }
      if ((rand() % 100) < 30) {
        mojo -= 500;
        AddNPC(gen, dwarf_miner);
      }
      if ((rand() % 100) < 30) {
        mojo -= 500;
        AddNPC(gen, dwarf_miner);
      }
      if ((rand() % 100) < 10) {
        mojo -= 500;
        AddNPC(gen, dwarf_engineer);
      }
      if ((rand() % 100) < 10) {
        mojo -= 500;
        AddNPC(gen, dwarf_engineer);
      }
      if ((rand() % 100) < 20) {
        mojo -= 500;
        AddNPC(gen, dwarf_guard);
      }
      if ((rand() % 100) < 20) {
        mojo -= 500;
        AddNPC(gen, dwarf_guard);
      }
      if ((rand() % 100) < 20) {
        mojo -= 500;
        AddNPC(gen, dwarf_guard);
      }
      if ((rand() % 100) < 20) {
        mojo -= 500;
        AddNPC(gen, dwarf_guard);
      }

      Object* cage = new Object(this);
      cage->SetShortDesc(u8"an old strong cage");
      cage->SetDesc(
          u8"This is a very old, large, rugged cage - probably built by "
          u8"dwarves.\n");
      cage->SetSkill(prhash(u8"Transparent"), 900);
      cage->SetSkill(prhash(u8"Container"), 100000);
      cage->SetSkill(prhash(u8"Locked"), 1);
      cage->SetSkill(prhash(u8"Lock"), 1510003);
      cage->SetSkill(prhash(u8"Closeable"), 1);
      cage->SetSkill(prhash(u8"Accomplishment"), 1510003);

      if ((rand() % 100) < 20) {
        mojo -= 500;
        cage->AddNPC(gen, elf_prisoner);
      }
      if ((rand() % 100) < 20) {
        mojo -= 500;
        cage->AddNPC(gen, elf_prisoner);
      }

      if (mojo <= 0)
        break; // End of Tunnel

      int ntypes[] = {1, 1, 1, 1, 1, 1, 1, 2, 2, 3};
      int ntype = ntypes[rand() % (sizeof(ntypes) / sizeof(int))];

      Object* next = new Object(parent);
      next->SetShortDesc(names[ntype]);
      next->SetDesc(descs[ntype]);
      next->SetSkill(prhash(u8"DynamicInit"), 1);
      next->SetSkill(prhash(u8"DynamicPhase"), ntype);
      next->SetSkill(prhash(u8"DynamicMojo"), mojo - 1000);

      Object* door1 = new Object(this);
      Object* door2 = new Object(next);
      door1->SetShortDesc(dir);
      door2->SetShortDesc(dirb);
      door1->SetDesc(fmt::format(u8"You see a solid passage leading {}.\n", dir));
      door2->SetDesc(fmt::format(u8"You see a solid passage leading {}.\n", dirb));
      door1->AddAct(act_t::SPECIAL_LINKED, door2);
      door1->SetSkill(prhash(u8"Open"), 1000);
      door1->SetSkill(prhash(u8"Enterable"), 1);
      door2->AddAct(act_t::SPECIAL_LINKED, door1);
      door2->SetSkill(prhash(u8"Open"), 1000);
      door2->SetSkill(prhash(u8"Enterable"), 1);
    } break;
    case (5): { // Minor Shaft
      if ((rand() % 100) < 30) {
        mojo -= 500;
        AddNPC(gen, dwarf_miner);
      }
      if ((rand() % 100) < 30) {
        mojo -= 500;
        AddNPC(gen, dwarf_miner);
      }
      if ((rand() % 100) < 10) {
        mojo -= 500;
        AddNPC(gen, dwarf_engineer);
      }
      if ((rand() % 100) < 20) {
        mojo -= 500;
        AddNPC(gen, dwarf_explorer);
      }

      if (mojo <= 0)
        break; // End of Tunnel

      //      int ntypes[] = { 5, 5, 5, 5, 6, 6, 6, 7, 7, 8, 9 };
      int ntypes[] = {5, 5, 5, 5, 5, 6, 6, 7, 7, 8};
      int ntype = ntypes[rand() % (sizeof(ntypes) / sizeof(int))];

      Object* next = new Object(parent);
      next->SetShortDesc(names[ntype]);
      next->SetDesc(descs[ntype]);
      next->SetSkill(prhash(u8"DynamicInit"), 1);
      next->SetSkill(prhash(u8"DynamicPhase"), ntype);
      next->SetSkill(prhash(u8"DynamicMojo"), mojo - 100);

      Object* door1 = new Object(this);
      Object* door2 = new Object(next);
      door1->SetShortDesc(dir);
      door2->SetShortDesc(dirb);
      door1->SetDesc(fmt::format(u8"You see a crumbling passage leading {}.\n", dir));
      door2->SetDesc(fmt::format(u8"You see a crumbling passage leading {}.\n", dirb));
      door1->AddAct(act_t::SPECIAL_LINKED, door2);
      door1->SetSkill(prhash(u8"Open"), 1000);
      door1->SetSkill(prhash(u8"Enterable"), 1);
      door2->AddAct(act_t::SPECIAL_LINKED, door1);
      door2->SetSkill(prhash(u8"Open"), 1000);
      door2->SetSkill(prhash(u8"Enterable"), 1);
    } break;
    case (6): { // Minor Shaft (Bend)
      if ((rand() % 100) < 30) {
        mojo -= 500;
        AddNPC(gen, dwarf_miner);
      }
      if ((rand() % 100) < 30) {
        mojo -= 500;
        AddNPC(gen, dwarf_miner);
      }
      if ((rand() % 100) < 10) {
        mojo -= 500;
        AddNPC(gen, dwarf_engineer);
      }
      if ((rand() % 100) < 20) {
        mojo -= 500;
        AddNPC(gen, dwarf_explorer);
      }

      //      int ntypes[] = { 5, 5, 5, 5, 5, 6, 6, 7, 7, 8, 9 };
      int ntypes[] = {5, 5, 5, 5, 5, 6, 6, 7, 7, 8};
      int ntype = ntypes[rand() % (sizeof(ntypes) / sizeof(int))];

      if (rand() % 2)
        std::swap(dir2, dir3); // Half left, half right
      Object* next = new Object(parent);
      next->SetShortDesc(names[ntype]);
      next->SetDesc(descs[ntype]);
      next->SetSkill(prhash(u8"DynamicInit"), 1);
      next->SetSkill(prhash(u8"DynamicPhase"), ntype);
      next->SetSkill(prhash(u8"DynamicMojo"), mojo - 100);

      Object* door1 = new Object(this);
      Object* door2 = new Object(next);
      door1->SetShortDesc(dir2);
      door2->SetShortDesc(dir3);
      door1->SetDesc(fmt::format(u8"You see a crumbling passage leading {}.\n", dir2));
      door2->SetDesc(fmt::format(u8"You see a crumbling passage leading {}.\n", dir3));
      door1->AddAct(act_t::SPECIAL_LINKED, door2);
      door1->SetSkill(prhash(u8"Open"), 1000);
      door1->SetSkill(prhash(u8"Enterable"), 1);
      door2->AddAct(act_t::SPECIAL_LINKED, door1);
      door2->SetSkill(prhash(u8"Open"), 1000);
      door2->SetSkill(prhash(u8"Enterable"), 1);
    } break;
    case (7): { // Minor Shaft Fork
      if ((rand() % 100) < 30) {
        mojo -= 500;
        AddNPC(gen, dwarf_miner);
      }
      if ((rand() % 100) < 30) {
        mojo -= 500;
        AddNPC(gen, dwarf_miner);
      }
      if ((rand() % 100) < 10) {
        mojo -= 500;
        AddNPC(gen, dwarf_engineer);
      }
      if ((rand() % 100) < 20) {
        mojo -= 500;
        AddNPC(gen, dwarf_explorer);
      }

      //      int ntypes[] = { 5, 5, 5, 5, 5, 6, 6, 7, 7, 8, 9 };
      int ntypes[] = {5, 5, 5, 5, 5, 6, 6, 7, 7, 8};
      int ntype = ntypes[rand() % (sizeof(ntypes) / sizeof(int))];

      Object* next = new Object(parent);
      next->SetShortDesc(names[ntype]);
      next->SetDesc(descs[ntype]);
      next->SetSkill(prhash(u8"DynamicInit"), 1);
      next->SetSkill(prhash(u8"DynamicPhase"), ntype);
      next->SetSkill(prhash(u8"DynamicMojo"), (mojo - 100) / 2);

      Object* door1 = new Object(this);
      Object* door2 = new Object(next);
      door1->SetShortDesc(dir2);
      door2->SetShortDesc(dir3);
      door1->SetDesc(fmt::format(u8"You see a crumbling passage leading {}.\n", dir2));
      door2->SetDesc(fmt::format(u8"You see a crumbling passage leading {}.\n", dir3));
      door1->AddAct(act_t::SPECIAL_LINKED, door2);
      door1->SetSkill(prhash(u8"Open"), 1000);
      door1->SetSkill(prhash(u8"Enterable"), 1);
      door2->AddAct(act_t::SPECIAL_LINKED, door1);
      door2->SetSkill(prhash(u8"Open"), 1000);
      door2->SetSkill(prhash(u8"Enterable"), 1);

      ntype = ntypes[rand() % (sizeof(ntypes) / sizeof(int))];
      std::swap(dir2, dir3); // Same code, opposite dir.
      next = new Object(parent);
      next->SetShortDesc(names[ntype]);
      next->SetDesc(descs[ntype]);
      next->SetSkill(prhash(u8"DynamicInit"), 1);
      next->SetSkill(prhash(u8"DynamicPhase"), ntype);
      next->SetSkill(prhash(u8"DynamicMojo"), (mojo - 100) / 2);

      door1 = new Object(this);
      door2 = new Object(next);
      door1->SetShortDesc(dir2);
      door2->SetShortDesc(dir3);
      door1->SetDesc(fmt::format(u8"You see a crumbling passage leading {}.\n", dir2));
      door2->SetDesc(fmt::format(u8"You see a crumbling passage leading {}.\n", dir3));
      door1->AddAct(act_t::SPECIAL_LINKED, door2);
      door1->SetSkill(prhash(u8"Open"), 1000);
      door1->SetSkill(prhash(u8"Enterable"), 1);
      door2->AddAct(act_t::SPECIAL_LINKED, door1);
      door2->SetSkill(prhash(u8"Open"), 1000);
      door2->SetSkill(prhash(u8"Enterable"), 1);
    } break;
    case (8): { // Minor Shaft Alcove
      if ((rand() % 100) < 30) {
        mojo -= 500;
        AddNPC(gen, dwarf_miner);
      }
      if ((rand() % 100) < 30) {
        mojo -= 500;
        AddNPC(gen, dwarf_miner);
      }
      if ((rand() % 100) < 10) {
        mojo -= 500;
        AddNPC(gen, dwarf_engineer);
      }
      if ((rand() % 100) < 20) {
        mojo -= 500;
        AddNPC(gen, dwarf_explorer);
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
      next->SetSkill(prhash(u8"DynamicInit"), 1);
      next->SetSkill(prhash(u8"DynamicPhase"), ntype);
      next->SetSkill(prhash(u8"DynamicMojo"), mojo - 500);
      next->SetSkill(prhash(u8"Accomplishment"), 1510001);

      Object* door1 = new Object(this);
      Object* door2 = new Object(next);
      door1->SetShortDesc(dir);
      door2->SetShortDesc(dirb);
      door1->SetDesc(fmt::format(u8"You see a crumbling passage leading {}.\n", dir));
      door2->SetDesc(fmt::format(u8"You see a crumbling passage leading {}.\n", dirb));
      door1->AddAct(act_t::SPECIAL_LINKED, door2);
      door1->SetSkill(prhash(u8"Open"), 1000);
      door1->SetSkill(prhash(u8"Enterable"), 1);
      door2->AddAct(act_t::SPECIAL_LINKED, door1);
      door2->SetSkill(prhash(u8"Open"), 1000);
      door2->SetSkill(prhash(u8"Enterable"), 1);
      door1->SetSkill(prhash(u8"Hidden"), 4 + rand() % 13);
    } break;
    default: {
      loge(
          u8"Unknown dynamic-phase-type ({}-{}) init requested!\n",
          Skill(prhash(u8"DynamicInit")),
          Skill(prhash(u8"DynamicPhase")));
    } break;
  }
  ClearSkill(prhash(u8"DynamicPhase"));
}

void Object::DynamicInit() {
  switch (Skill(prhash(u8"DynamicInit"))) {
    case (2): { // Cyberpunk City
      DynamicInit2();
    } break;
    case (1): { // Dwarven Mine
      DynamicInit1();
    } break;
    default: {
      loge(u8"Unknown dynamic-type ({}) init requested!\n", Skill(prhash(u8"DynamicInit")));
    } break;
  }
  ClearSkill(prhash(u8"DynamicInit"));
}
