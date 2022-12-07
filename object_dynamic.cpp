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
#include <string>

#include "color.hpp"
#include "commands.hpp"
#include "dice.hpp"
#include "log.hpp"
#include "mind.hpp"
#include "object.hpp"
#include "properties.hpp"
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
      if (Dice::Percent(30)) {
        mojo -= 500;
        AddNPC(u8"dwarven,miner")->SetSkill(prhash(u8"Accomplishment"), 1500000);
      }
      if (Dice::Percent(10)) {
        mojo -= 500;
        AddNPC(u8"dwarven,engineer")->SetSkill(prhash(u8"Accomplishment"), 1500001);
      }
      if (Dice::Percent(20)) {
        mojo -= 500;
        AddNPC(u8"dwarven,guard,dungeon_cage_key")->SetSkill(prhash(u8"Accomplishment"), 1500002);
      }
      if (Dice::Percent(20)) {
        mojo -= 500;
        AddNPC(u8"dwarven,guard,dungeon_cage_key")->SetSkill(prhash(u8"Accomplishment"), 1500002);
      }

      if (mojo <= 0)
        break; // End of Tunnel

      int ntypes[] = {1, 1, 1, 1, 1, 1, 1, 2, 2, 3, 4};
      int ntype = Dice::Sample(ntypes);

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
      if (Dice::Percent(30)) {
        mojo -= 500;
        AddNPC(u8"dwarven,miner")->SetSkill(prhash(u8"Accomplishment"), 1500000);
      }
      if (Dice::Percent(10)) {
        mojo -= 500;
        AddNPC(u8"dwarven,engineer")->SetSkill(prhash(u8"Accomplishment"), 1500001);
      }
      if (Dice::Percent(20)) {
        mojo -= 500;
        AddNPC(u8"dwarven,guard,dungeon_cage_key")->SetSkill(prhash(u8"Accomplishment"), 1500002);
      }
      if (Dice::Percent(20)) {
        mojo -= 500;
        AddNPC(u8"dwarven,guard,dungeon_cage_key")->SetSkill(prhash(u8"Accomplishment"), 1500002);
      }

      if (mojo <= 0)
        break; // End of Tunnel

      int ntypes[] = {1, 1, 1, 1, 1, 1, 1, 2, 2, 3, 4};
      int ntype = Dice::Sample(ntypes);

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

      if (Dice::Rand(0, 1))
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
      if (Dice::Percent(30)) {
        mojo -= 500;
        AddNPC(u8"dwarven,miner")->SetSkill(prhash(u8"Accomplishment"), 1500000);
      }
      if (Dice::Percent(10)) {
        mojo -= 500;
        AddNPC(u8"dwarven,engineer")->SetSkill(prhash(u8"Accomplishment"), 1500001);
      }
      if (Dice::Percent(20)) {
        mojo -= 500;
        AddNPC(u8"dwarven,guard,dungeon_cage_key")->SetSkill(prhash(u8"Accomplishment"), 1500002);
      }
      if (Dice::Percent(20)) {
        mojo -= 500;
        AddNPC(u8"dwarven,guard,dungeon_cage_key")->SetSkill(prhash(u8"Accomplishment"), 1500002);
      }

      if (mojo <= 0)
        break; // End of Tunnel

      int ntypes[] = {1, 1, 1, 1, 1, 1, 1, 2, 2, 3, 4};
      int ntype = Dice::Sample(ntypes);

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

      if (Dice::Rand(0, 1)) {
        std::swap(dir2, dir3); // Half left, half right
      }
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
      door1->SetSkill(prhash(u8"Hidden"), 4 + Dice::Rand(0, 12));
    } break;
    case (4): { // Major Chamber
      if (Dice::Percent(30)) {
        mojo -= 500;
        AddNPC(u8"dwarven,miner")->SetSkill(prhash(u8"Accomplishment"), 1500000);
      }
      if (Dice::Percent(30)) {
        mojo -= 500;
        AddNPC(u8"dwarven,miner")->SetSkill(prhash(u8"Accomplishment"), 1500000);
      }
      if (Dice::Percent(30)) {
        mojo -= 500;
        AddNPC(u8"dwarven,miner")->SetSkill(prhash(u8"Accomplishment"), 1500000);
      }
      if (Dice::Percent(30)) {
        mojo -= 500;
        AddNPC(u8"dwarven,miner")->SetSkill(prhash(u8"Accomplishment"), 1500000);
      }
      if (Dice::Percent(10)) {
        mojo -= 500;
        AddNPC(u8"dwarven,engineer")->SetSkill(prhash(u8"Accomplishment"), 1500001);
      }
      if (Dice::Percent(10)) {
        mojo -= 500;
        AddNPC(u8"dwarven,engineer")->SetSkill(prhash(u8"Accomplishment"), 1500001);
      }
      if (Dice::Percent(20)) {
        mojo -= 500;
        AddNPC(u8"dwarven,guard,dungeon_cage_key")->SetSkill(prhash(u8"Accomplishment"), 1500002);
      }
      if (Dice::Percent(20)) {
        mojo -= 500;
        AddNPC(u8"dwarven,guard,dungeon_cage_key")->SetSkill(prhash(u8"Accomplishment"), 1500002);
      }
      if (Dice::Percent(20)) {
        mojo -= 500;
        AddNPC(u8"dwarven,guard,dungeon_cage_key")->SetSkill(prhash(u8"Accomplishment"), 1500002);
      }
      if (Dice::Percent(20)) {
        mojo -= 500;
        AddNPC(u8"dwarven,guard,dungeon_cage_key")->SetSkill(prhash(u8"Accomplishment"), 1500002);
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

      if (Dice::Percent(20)) {
        mojo -= 500;
        cage->AddNPC(u8"elven,prisoner")->SetSkill(prhash(u8"Accomplishment"), 1500004);
      }
      if (Dice::Percent(20)) {
        mojo -= 500;
        cage->AddNPC(u8"elven,prisoner")->SetSkill(prhash(u8"Accomplishment"), 1500004);
      }

      if (mojo <= 0)
        break; // End of Tunnel

      int ntypes[] = {1, 1, 1, 1, 1, 1, 1, 2, 2, 3};
      int ntype = Dice::Sample(ntypes);

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
      if (Dice::Percent(30)) {
        mojo -= 500;
        AddNPC(u8"dwarven,miner")->SetSkill(prhash(u8"Accomplishment"), 1500000);
      }
      if (Dice::Percent(30)) {
        mojo -= 500;
        AddNPC(u8"dwarven,miner")->SetSkill(prhash(u8"Accomplishment"), 1500000);
      }
      if (Dice::Percent(10)) {
        mojo -= 500;
        AddNPC(u8"dwarven,engineer")->SetSkill(prhash(u8"Accomplishment"), 1500001);
      }
      if (Dice::Percent(20)) {
        mojo -= 500;
        AddNPC(u8"dwarven,explorer")->SetSkill(prhash(u8"Accomplishment"), 1500003);
      }

      if (mojo <= 0)
        break; // End of Tunnel

      //      int ntypes[] = { 5, 5, 5, 5, 6, 6, 6, 7, 7, 8, 9 };
      int ntypes[] = {5, 5, 5, 5, 5, 6, 6, 7, 7, 8};
      int ntype = Dice::Sample(ntypes);

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
      if (Dice::Percent(30)) {
        mojo -= 500;
        AddNPC(u8"dwarven,miner")->SetSkill(prhash(u8"Accomplishment"), 1500000);
      }
      if (Dice::Percent(30)) {
        mojo -= 500;
        AddNPC(u8"dwarven,miner")->SetSkill(prhash(u8"Accomplishment"), 1500000);
      }
      if (Dice::Percent(10)) {
        mojo -= 500;
        AddNPC(u8"dwarven,engineer")->SetSkill(prhash(u8"Accomplishment"), 1500001);
      }
      if (Dice::Percent(20)) {
        mojo -= 500;
        AddNPC(u8"dwarven,explorer")->SetSkill(prhash(u8"Accomplishment"), 1500003);
      }

      //      int ntypes[] = { 5, 5, 5, 5, 5, 6, 6, 7, 7, 8, 9 };
      int ntypes[] = {5, 5, 5, 5, 5, 6, 6, 7, 7, 8};
      int ntype = Dice::Sample(ntypes);

      if (Dice::Rand(0, 1)) {
        std::swap(dir2, dir3); // Half left, half right
      }
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
      if (Dice::Percent(30)) {
        mojo -= 500;
        AddNPC(u8"dwarven,miner")->SetSkill(prhash(u8"Accomplishment"), 1500000);
      }
      if (Dice::Percent(30)) {
        mojo -= 500;
        AddNPC(u8"dwarven,miner")->SetSkill(prhash(u8"Accomplishment"), 1500000);
      }
      if (Dice::Percent(10)) {
        mojo -= 500;
        AddNPC(u8"dwarven,engineer")->SetSkill(prhash(u8"Accomplishment"), 1500001);
      }
      if (Dice::Percent(20)) {
        mojo -= 500;
        AddNPC(u8"dwarven,explorer")->SetSkill(prhash(u8"Accomplishment"), 1500003);
      }

      //      int ntypes[] = { 5, 5, 5, 5, 5, 6, 6, 7, 7, 8, 9 };
      int ntypes[] = {5, 5, 5, 5, 5, 6, 6, 7, 7, 8};
      int ntype = Dice::Sample(ntypes);

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

      ntype = Dice::Sample(ntypes);
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
      if (Dice::Percent(30)) {
        mojo -= 500;
        AddNPC(u8"dwarven,miner")->SetSkill(prhash(u8"Accomplishment"), 1500000);
      }
      if (Dice::Percent(30)) {
        mojo -= 500;
        AddNPC(u8"dwarven,miner")->SetSkill(prhash(u8"Accomplishment"), 1500000);
      }
      if (Dice::Percent(10)) {
        mojo -= 500;
        AddNPC(u8"dwarven,engineer")->SetSkill(prhash(u8"Accomplishment"), 1500001);
      }
      if (Dice::Percent(20)) {
        mojo -= 500;
        AddNPC(u8"dwarven,explorer")->SetSkill(prhash(u8"Accomplishment"), 1500003);
      }

      if (mojo <= 0)
        break; // End of Tunnel

      if (mojo > 10000 && Dice::Percent(20)) {
        mojo -= 5000;
        auto roll = d10();
        if (roll <= 3) {
          std::swap(dir, dir2);
          std::swap(dirb, dir3);
        } else if (roll > 7) {
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
      door1->SetSkill(prhash(u8"Hidden"), 4 + Dice::Rand(0, 12));
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
      loge(u8"Unknown dynamic-type ({}) init requested!", Skill(prhash(u8"DynamicInit")));
    } break;
  }
  ClearSkill(prhash(u8"DynamicInit"));
}
