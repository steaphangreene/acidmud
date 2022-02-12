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

#include <fcntl.h>
#include <unistd.h>
#include <algorithm>
#include <cctype>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

// Replace with C++20 std::format, when widely available
#include <fmt/format.h>

#include "color.hpp"
#include "commands.hpp"
#include "mind.hpp"
#include "net.hpp"
#include "object.hpp"
#include "utils.hpp"

const std::string pos_str[POS_MAX] = {
    "is here",
    "is lying here",
    "is sitting here",
    "is standing here",
    "is using a skill",
};

const std::string act_str[ACT_SPECIAL_MAX] = {
    "doing nothing",
    "dead",
    "bleeding and dying",
    "unconscious",
    "fast asleep",
    "resting",
    "healing %1$s",
    "pointing %2$s%3$sat %1$s",
    "following %1$s",
    "fighting %1$s",
    "offering something to %1$s",
    "holding %1$s",
    "wielding %1$s",
    "wearing %1$s",
    "wearing %1$s",
    "wearing %1$s",
    "wearing %1$s",
    "wearing %1$s",
    "wearing %1$s",
    "wearing %1$s",
    "wearing %1$s",
    "wearing %1$s",
    "wearing %1$s",
    "wearing %1$s",
    "wearing %1$s",
    "wearing %1$s",
    "wearing %1$s",
    "wearing %1$s",
    "wearing %1$s",
    "wearing %1$s",
    "wearing %1$s",
    "wearing %1$s",
    "wearing %1$s",
    "wearing %1$s",
    "wearing %1$s",
    "wearing %1$s",
    "wearing_%1$s",
    "ACT_SPECIAL_MONITOR",
    "ACT_SPECIAL_MASTER",
    "ACT_SPECIAL_LINKED",
    "ACT_SPECIAL_HOME",
    //"ACT_SPECIAL_MAX"
};

static Object* universe = nullptr;
static Object* trash_bin = nullptr;

static std::set<Object*> busylist;
extern int64_t current_time; // From main.cpp

Object* Object::Universe() {
  return universe;
}

Object* Object::TrashBin() {
  return trash_bin;
}

Object* Object::World() {
  Object* world = this;
  if (world->Parent()) {
    while (world->Parent()->Parent())
      world = world->Parent();
  }
  return world;
}

int matches(const std::string& name, const std::string& seek) {
  if (seek.empty())
    return 0;

  auto stok = crc32c(seek);
  if (stok == crc32c("all"))
    return 1;

  if (phrase_match(name, seek))
    return 1;

  static bool dont_recur = false;
  if (dont_recur)
    return 0;
  dont_recur = true;

  int ret = 0;

  // suffix-word searches
  if (stok == crc32c("guard") || stok == crc32c("smith") || stok == crc32c("master") ||
      stok == crc32c("sword") || stok == crc32c("hammer") || stok == crc32c("axe") ||
      stok == crc32c("bow") || stok == crc32c("staff") || stok == crc32c("keeper")) {
    auto part = seek;
    std::transform(part.begin(), part.end(), part.begin(), ascii_tolower);
    auto word = name;
    std::transform(word.begin(), word.end(), word.begin(), ascii_tolower);

    auto cont = word.find(part);
    while (cont != std::string::npos) {
      if (word.length() == cont + part.length() || word[cont + part.length()] == ' ') {
        ret = 1;
        break;
      }
      cont = word.find(part, cont + 1);
    }
  }

  // special SMART[TM] searches
  if ((stok == crc32c("guard")) && matches(name, "guardian"))
    ret = 1;
  else if ((stok == crc32c("guard")) && matches(name, "guardsman"))
    ret = 1;
  else if ((stok == crc32c("guard")) && matches(name, "guardswoman"))
    ret = 1;
  else if ((stok == crc32c("merc")) && matches(name, "mercenary"))
    ret = 1;
  else if ((stok == crc32c("bolt")) && matches(name, "thunderbolt"))
    ret = 1;
  else if ((stok == crc32c("battle hammer")) && matches(name, "battlehammer"))
    ret = 1;
  else if ((stok == crc32c("battlehammer")) && matches(name, "battle hammer"))
    ret = 1;
  else if ((stok == crc32c("war hammer")) && matches(name, "warhammer"))
    ret = 1;
  else if ((stok == crc32c("warhammer")) && matches(name, "war hammer"))
    ret = 1;
  else if ((stok == crc32c("battle axe")) && matches(name, "battleaxe"))
    ret = 1;
  else if ((stok == crc32c("battleaxe")) && matches(name, "battle axe"))
    ret = 1;
  else if ((stok == crc32c("war axe")) && matches(name, "waraxe"))
    ret = 1;
  else if ((stok == crc32c("waraxe")) && matches(name, "war axe"))
    ret = 1;
  else if ((stok == crc32c("morning star")) && matches(name, "morningstar"))
    ret = 1;
  else if ((stok == crc32c("morningstar")) && matches(name, "morning star"))
    ret = 1;
  else if ((stok == crc32c("bisarme")) && matches(name, "gisarme"))
    ret = 1;
  else if ((stok == crc32c("bisarme")) && matches(name, "guisarme"))
    ret = 1;
  else if ((stok == crc32c("gisarme")) && matches(name, "bisarme"))
    ret = 1;
  else if ((stok == crc32c("gisarme")) && matches(name, "guisarme"))
    ret = 1;
  else if ((stok == crc32c("guisarme")) && matches(name, "bisarme"))
    ret = 1;
  else if ((stok == crc32c("guisarme")) && matches(name, "gisarme"))
    ret = 1;
  else if ((stok == crc32c("bill-bisarme")) && matches(name, "bill-gisarme"))
    ret = 1;
  else if ((stok == crc32c("bill-bisarme")) && matches(name, "bill-guisarme"))
    ret = 1;
  else if ((stok == crc32c("bill-gisarme")) && matches(name, "bill-bisarme"))
    ret = 1;
  else if ((stok == crc32c("bill-gisarme")) && matches(name, "bill-guisarme"))
    ret = 1;
  else if ((stok == crc32c("bill-guisarme")) && matches(name, "bill-bisarme"))
    ret = 1;
  else if ((stok == crc32c("bill-guisarme")) && matches(name, "bill-gisarme"))
    ret = 1;
  else if ((stok == crc32c("grey")) && matches(name, "gray"))
    ret = 1;
  else if ((stok == crc32c("gray")) && matches(name, "grey"))
    ret = 1;
  else if ((stok == crc32c("bread")) && matches(name, "waybread"))
    ret = 1;

  // One-way purposeful mis-spellings to silence some extra labelling from TBA
  else if ((stok == crc32c("potatoe")) && matches(name, "potato"))
    ret = 1;

  dont_recur = false;
  return ret;
}

int Object::Matches(std::string targ) {
  trim_string(targ);

  // Pointer Matches
  if (!strncmp(targ.c_str(), "OBJ:", 4)) {
    Object* tofind = nullptr;
    sscanf(targ.c_str() + 4, "%p", &tofind);
    return (this == tofind);
  }

  auto ttok = crc32c(targ);

  // Keywords Only
  if (ttok == crc32c("everyone")) {
    return (IsAnimate());
  }
  if (ttok == crc32c("someone")) {
    return (IsAnimate());
  }
  if (ttok == crc32c("anyone")) {
    return (IsAnimate());
  }
  if (ttok == crc32c("everything")) {
    if (IsAct(ACT_SPECIAL_LINKED))
      return 0;
    return (!IsAnimate());
  }
  if (ttok == crc32c("something")) {
    if (IsAct(ACT_SPECIAL_LINKED))
      return 0;
    return (!IsAnimate());
  }
  if (ttok == crc32c("anything")) {
    if (IsAct(ACT_SPECIAL_LINKED))
      return 0;
    return (!IsAnimate());
  }
  if (ttok == crc32c("everywhere")) {
    if (!IsAct(ACT_SPECIAL_LINKED))
      return 0;
    return (!IsAnimate());
  }
  if (ttok == crc32c("somewhere")) {
    if (!IsAct(ACT_SPECIAL_LINKED))
      return 0;
    return (!IsAnimate());
  }
  if (ttok == crc32c("anywhere")) {
    if (!IsAct(ACT_SPECIAL_LINKED))
      return 0;
    return (!IsAnimate());
  }

  // Matches for sexist TBA aliases :)
  if (ttok == crc32c("man") || ttok == crc32c("boy")) {
    if (IsAnimate() && Gender() == 'M')
      return 1;
  }
  if (ttok == crc32c("woman") || ttok == crc32c("girl")) {
    if (IsAnimate() && Gender() == 'F')
      return 1;
  }

  // Keywords which can also be things
  if ((ttok == crc32c("Corpse")) && IsAct(ACT_DEAD))
    return 1;
  if ((ttok == crc32c("Money")) && Skill(crc32c("Money")))
    return 1;

  return matches(ShortDesc(), targ);
}

Object* new_body() {
  Object* body = new Object();
  body->SetAttribute(0, 3);
  body->SetAttribute(1, 3);
  body->SetAttribute(2, 3);
  body->SetAttribute(3, 3);
  body->SetAttribute(4, 3);
  body->SetAttribute(5, 3);

  body->SetShortDesc("an amorphous blob");

  Object* start = universe->ActTarg(ACT_SPECIAL_HOME);
  if (!start)
    start = universe;
  body->SetParent(start);

  body->SetWeight(body->NormAttribute(0) * 20000);
  body->SetSize(1000 + body->NormAttribute(0) * 200);
  body->SetVolume(100);
  body->SetValue(-1);
  body->SetWeight(80000);
  body->SetGender('M');

  body->SetSkill(crc32c("Attribute Points"), 12);
  body->SetSkill(crc32c("Skill Points"), 64);
  body->SetSkill(crc32c("Status Points"), 6);

  body->SetPos(POS_STAND);

  body->Activate();

  Object* bag = new Object(body);
  bag->SetSkill(crc32c("Capacity"), 40);
  bag->SetSkill(crc32c("Closeable"), 1);
  bag->SetSkill(crc32c("Container"), 20000);
  bag->SetSkill(crc32c("Open"), 1000);
  bag->SetSkill(crc32c("Wearable on Left Hip"), 1);
  bag->SetSkill(crc32c("Wearable on Right Hip"), 2);

  bag->SetShortDesc("a small bag");
  bag->SetDesc("A small bag is here.");

  bag->SetWeight(800);
  bag->SetSize(1);
  bag->SetVolume(2);
  bag->SetValue(10);

  bag->SetPos(POS_LIE);

  body->AddAct(ACT_WEAR_RHIP, bag);

  return body;
}

#define TICKSPLIT 32 // ~3 seconds
static std::set<Object*> ticklist[TICKSPLIT];
int Object::IsActive() const {
  return (tickstep >= 0);
}

void Object::Activate() {
  if (tickstep >= 0)
    return;
  static int tickstage = 0;
  ticklist[tickstage].insert(this);
  tickstep = tickstage++;
  if (tickstage >= TICKSPLIT)
    tickstage = 0;
}

void Object::Deactivate() {
  if (tickstep < 0)
    return;
  ticklist[tickstep].erase(this);
  tickstep = -1;
}

void tick_world() {
  static int tickstage = 0;
  std::set<Object*> todel, todeact;

  auto tkl = ticklist[tickstage];
  for (auto ind : tkl) {
    int res = ind->Tick();
    if (res != 0) {
      if (res == 1) {
        todel.insert(ind);
      } else if (res == -1) {
        todeact.insert(ind);
      }
    }
  }
  for (auto ind : todel) {
    todeact.erase(ind);
    ind->Recycle();
  }
  for (auto ind : todeact) {
    ind->Deactivate();
  }
  ++tickstage;
  if (tickstage >= TICKSPLIT)
    tickstage = 0;
  Mind::Resume(); // Tell suspended minds to resume if their time is up
}

int Object::Tick() {
  auto mnds = minds;
  for (auto m : mnds) {
    m->Attach(this);
    m->Think(1);
  }

  if (phys > (10 + ModAttribute(2))) {
    // You are already dead.
  } else if (phys >= 10) {
    int rec = RollNoWounds(crc32c("Strength"), phys - 4, 0);
    if (!rec)
      ++phys;
    UpdateDamage();
  } else if (phys > 0) {
    int rec = 0;
    if (IsAct(ACT_SLEEP))
      rec = Roll(crc32c("Body"), 2);
    else if (IsAct(ACT_REST))
      rec = Roll(crc32c("Body"), 4);
    else if (!IsAct(ACT_FIGHT))
      rec = Roll(crc32c("Body"), 6);
    if (phys >= 6 && (!rec))
      ++phys;
    else
      phys -= rec / 2;
    phys = std::max(int8_t(0), phys);
    UpdateDamage();
  }
  if (phys < 10 && stun >= 10) {
    int rec = 0;
    rec = RollNoWounds(crc32c("Willpower"), 12, 0);
    stun -= rec;
    stun = std::max(int8_t(0), stun);
    UpdateDamage();
  } else if (phys < 6 && stun > 0) {
    int rec = 0;
    if (IsAct(ACT_SLEEP))
      rec = Roll(crc32c("Willpower"), 2);
    else if (IsAct(ACT_REST))
      rec = Roll(crc32c("Willpower"), 4);
    else if (!IsAct(ACT_FIGHT))
      rec = Roll(crc32c("Willpower"), 6);
    stun -= rec;
    stun = std::max(int8_t(0), stun);
    UpdateDamage();
  }

  if (parent && Skill(crc32c("TBAPopper")) > 0 && contents.size() > 0) {
    if (!ActTarg(ACT_SPECIAL_MONITOR)) {
      Object* obj = new Object(*(contents.front()));
      obj->SetParent(this);
      obj->Travel(parent);
      AddAct(ACT_SPECIAL_MONITOR, obj);
      obj->Attach(get_tba_mob_mind());
      obj->Activate();
      parent->SendOut(ALL, -1, ";s arrives.\n", "", obj, nullptr);
      for (auto trg : obj->Contents()) { // Enable any untriggered triggers
        if (trg->HasSkill(crc32c("TBAScript")) &&
            (trg->Skill(crc32c("TBAScriptType")) & 0x0000002)) {
          trg->Activate();
          new_trigger(13000 + (rand() % 13000), trg, nullptr, nullptr, "");
        }
      }
    }
  }

  // Grow Trees (Silently)
  if (HasSkill(crc32c("Mature Trees")) && Skill(crc32c("Mature Trees")) < 100) {
    SetSkill(crc32c("Mature Trees"), Skill(crc32c("Mature Trees")) + 1);
  }

  if (IsAct(ACT_DEAD)) { // Rotting corpses
    ++stru;
    if (stru == 1) {
      parent->SendOut(ALL, 0, ";s's corpse starts to smell.\n", "", this, nullptr);
    } else if (stru == 3) {
      parent->SendOut(ALL, 0, ";s's corpse starts to rot.\n", "", this, nullptr);
    } else if (stru == 6) {
      parent->SendOut(ALL, 0, ";s's corpse starts to fall apart.\n", "", this, nullptr);
    } else if (stru >= 10) {
      Object* corpse = new Object(parent);

      corpse->SetShortDesc("an unidentifiable corpse");
      corpse->SetDesc("A pile of rotting remains.");
      corpse->SetPos(POS_LIE);

      corpse->SetSkill(crc32c("Perishable"), 1);
      corpse->SetSkill(crc32c("Rot"), 1);
      corpse->Activate();

      corpse->SetWeight(Weight());
      corpse->SetSize(Size());
      corpse->SetVolume(Volume());

      std::set<Object*> todrop;
      MinVec<1, Object*> todropfrom;
      todropfrom.push_back(this);

      for (auto con : todropfrom) {
        for (auto item : con->contents) {
          if (item->contents.size() > 0 && item->Pos() == POS_NONE) {
            todropfrom.push_back(item);
          } else if (item->Pos() != POS_NONE) { // Fixed items can't be dropped!
            todrop.insert(item);
          }
        }
      }

      for (auto td : todrop) {
        if (Parent())
          Drop(td, 0, 1);
        else
          td->Recycle();
      }

      auto cont = contents;
      for (auto todel : cont) {
        todel->Recycle();
      }

      parent->SendOut(ALL, 0, ";s's corpse completely falls apart.\n", "", this, nullptr);

      if (is_pc(this)) { // Hide me in the VOID!
        Object* dest = this;
        while ((!dest->ActTarg(ACT_SPECIAL_HOME)) && dest->Parent()) {
          dest = dest->Parent();
        }
        if (dest->ActTarg(ACT_SPECIAL_HOME)) {
          dest = dest->ActTarg(ACT_SPECIAL_HOME);
        }
        Travel(dest);
        SetSkill(crc32c("Hidden"), 65535);
        return -1; // Deactivate Me!
      } else {
        return 1; // Delete Me!
      }
    }
  }

  if (HasSkill(crc32c("Perishable"))) { // Degrading Items
    SetSkill(crc32c("Rot"), Skill(crc32c("Rot")) - 1);
    if (Skill(crc32c("Rot")) < 1) {
      ++stru;
      if (stru < 10) {
        SetSkill(crc32c("Rot"), Skill(crc32c("Perishable")));
      } else {
        return 1; // Delete Me!
      }
    }
  }

  if (HasSkill(crc32c("Temporary"))) { // Temporary Items
    SetSkill(crc32c("Temporary"), Skill(crc32c("Temporary")) - 1);
    if (Skill(crc32c("Temporary")) < 1) {
      if (Owner() && Owner()->Parent()) {
        Owner()->Parent()->SendOut(0, 0, ";s vanishes in a flash of light.", "", this, nullptr);
      }
      return 1; // Delete Me!
    }
  }

  if (BaseAttribute(2) > 0 // Needs Food & Water
      && (!HasSkill(crc32c("TBAAction"))) // MOBs don't
  ) {
    int level;

    // Get Hungrier
    level = Skill(crc32c("Hungry"));
    if (level < 1)
      level = ModAttribute(2);
    else
      level += ModAttribute(2); // Base Strength Scales Food Req
    if (level > 29999)
      level = 29999;
    SetSkill(crc32c("Hungry"), level);

    if (level == 500)
      Send(ALL, -1, "You could use a snack.\n");
    else if (level == 1000)
      Send(ALL, -1, "You officially have the munchies.\n");
    else if (level == 1500)
      Send(ALL, -1, "You really could go for a snack.\n");
    else if (level == 2000)
      Send(ALL, -1, "You are getting hungry.\n");
    else if (level == 2500)
      Send(ALL, -1, "You are getting very hungry.\n");
    else if (level == 3000)
      Send(ALL, -1, "You are really quite hungry.\n");
    else if (level == 3500)
      Send(ALL, -1, "You are really dying for food.\n");
    else if (level == 4000)
      Send(ALL, -1, "You need to get some food soon!\n");
    else if (level == 4500)
      Send(ALL, -1, "You are starting to starve!\n");
    else if (level >= 5000) {
      if (level % 10 == 0) {
        Send(ALL, -1, "You are starving!\n");
      }
      UpdateDamage();
    }

    // Get Thurstier
    level = Skill(crc32c("Thirsty"));
    if (level < 1)
      level = ModAttribute(0);
    else
      level += ModAttribute(0); // Body Scales Water Req
    if (level > 29999)
      level = 29999;
    SetSkill(crc32c("Thirsty"), level);

    if (level == 500)
      Send(ALL, -1, "You could use a drink.\n");
    else if (level == 1000)
      Send(ALL, -1, "Your mouth is getting dry.\n");
    else if (level == 1500)
      Send(ALL, -1, "You really could go for a drink.\n");
    else if (level == 2000)
      Send(ALL, -1, "You are getting thirsty.\n");
    else if (level == 2500)
      Send(ALL, -1, "You are getting very thirsty.\n");
    else if (level == 3000)
      Send(ALL, -1, "You are really quite thirsty.\n");
    else if (level == 3500)
      Send(ALL, -1, "You are really dying for water.\n");
    else if (level == 4000)
      Send(ALL, -1, "You need to get some water soon!\n");
    else if (level == 4500)
      Send(ALL, -1, "You are starting to dehydrate!\n");
    else if (level >= 5000) {
      if (level % 10 == 0) {
        Send(ALL, -1, "You are dehydrated!\n");
      }
      UpdateDamage();
    }
  }

  if (HasSkill(crc32c("Liquid Source"))) { // Refills Itself
    if (contents.size() > 0) {
      int qty = 1;
      if (contents.front()->Skill(crc32c("Quantity")) > 1) {
        qty = contents.front()->Skill(crc32c("Quantity"));
      }
      if (qty < Skill(crc32c("Liquid Container"))) {
        contents.front()->SetSkill(crc32c("Quantity"), qty + Skill(crc32c("Liquid Source")));
        if (contents.front()->Skill(crc32c("Quantity")) > Skill(crc32c("Liquid Container"))) {
          contents.front()->SetSkill(crc32c("Quantity"), Skill(crc32c("Liquid Container")));
        }
      }
    } else {
      fprintf(stderr, "Warning: Fountain completely out of liquid!\n");
    }
  }

  // Lit Torches/Lanterns
  if (HasSkill(crc32c("Lightable")) && HasSkill(crc32c("Light Source"))) {
    SetSkill(crc32c("Lightable"), Skill(crc32c("Lightable")) - 1);
    bool goesout = false;
    if (Skill(crc32c("Lightable")) < 1) {
      goesout = true;
    } else if (!parent->IsAnimate()) {
      int chances = Skill(crc32c("Resilience"));
      goesout = ((rand() % 1000) >= chances);
    }
    if (goesout) {
      SetSkill(crc32c("Light Source"), 0);
      parent->SendOut(ALL, -1, ";s goes out.\n", "", this, nullptr);
      return -1; // Deactivate Me!
    }
  }

  // Skys
  if (Skill(crc32c("Day Length")) > 1) { // Must be > 1 (divide by it/2 below!)
    SetSkill(crc32c("Day Time"), Skill(crc32c("Day Time")) + 1);
    if (Skill(crc32c("Day Time")) >= Skill(crc32c("Day Length"))) {
      SetSkill(crc32c("Day Time"), 0);
    }
    int light = Skill(crc32c("Day Time")) - (Skill(crc32c("Day Length")) / 2);
    if (light < 0)
      light = -light;
    light *= 900;
    light /= (Skill(crc32c("Day Length")) / 2);
    SetSkill(crc32c("Light Source"), 1000 - light);
  }

  // Poisoned
  if (Skill(crc32c("Poisoned")) > 0) {
    int succ = Roll(crc32c("Strength"), Skill(crc32c("Poisoned")));
    SetSkill(crc32c("Poisoned"), Skill(crc32c("Poisoned")) - succ);
    Parent()->SendOut(
        0,
        0,
        ";s chokes and writhes in pain.\n",
        CRED "You choke and writhe in pain.  POISON!!!!\n" CNRM,
        this,
        nullptr);
    if (succ < 2)
      phys += 6;
    else if (succ < 4)
      phys += 3;
    else if (succ < 6)
      phys += 1;
    UpdateDamage();
  }

  return 0;
}

Object::Object() {
  busy_until = 0;
  short_desc = "new object";
  parent = nullptr;
  pos = POS_NONE;
  cur_skill = crc32c("None");

  weight = 0;
  volume = 0;
  size = 0;
  value = 0;
  gender = 'N';

  sexp = 0;

  stun = 0;
  phys = 0;
  stru = 0;

  att[0] = {0, 0, 0};
  att[1] = {0, 0, 0};
  att[2] = {0, 0, 0};
  att[3] = {0, 0, 0};
  att[4] = {0, 0, 0};
  att[5] = {0, 0, 0};

  no_seek = false;
  no_hear = false;
  tickstep = -1;

  InitSkillsData();
}

Object::Object(Object* o) {
  busy_until = 0;
  short_desc = "new object";
  parent = nullptr;
  SetParent(o);
  pos = POS_NONE;
  cur_skill = crc32c("None");

  weight = 0;
  volume = 0;
  size = 0;
  value = 0;
  gender = 'N';

  sexp = 0;

  stun = 0;
  phys = 0;
  stru = 0;

  att[0] = {0, 0, 0};
  att[1] = {0, 0, 0};
  att[2] = {0, 0, 0};
  att[3] = {0, 0, 0};
  att[4] = {0, 0, 0};
  att[5] = {0, 0, 0};

  no_seek = false;
  no_hear = false;
  tickstep = -1;
}

Object::Object(const Object& o) {
  busy_until = 0;
  short_desc = o.short_desc;
  desc = o.desc;
  long_desc = o.long_desc;
  act.clear();
  pos = o.pos;
  cur_skill = o.cur_skill;

  weight = o.weight;
  volume = o.volume;
  size = o.size;
  value = o.value;
  gender = o.gender;

  sexp = o.sexp;

  stun = o.stun;
  phys = o.phys;
  stru = o.stru;

  att[0] = o.att[0];
  att[1] = o.att[1];
  att[2] = o.att[2];
  att[3] = o.att[3];
  att[4] = o.att[4];
  att[5] = o.att[5];

  skills = o.skills;

  contents.clear();
  for (auto ind : o.contents) {
    Object* nobj = new Object(*ind);
    nobj->SetParent(this);
    if (o.ActTarg(ACT_HOLD) == ind)
      AddAct(ACT_HOLD, nobj);
    if (o.ActTarg(ACT_WIELD) == ind)
      AddAct(ACT_WIELD, nobj);
    for (act_t actt = ACT_WEAR_BACK; actt < ACT_MAX; actt = act_t(int(actt) + 1))
      if (o.ActTarg(actt) == ind)
        AddAct(actt, nobj);
  }
  if (o.IsAct(ACT_DEAD))
    AddAct(ACT_DEAD);
  if (o.IsAct(ACT_DYING))
    AddAct(ACT_DYING);
  if (o.IsAct(ACT_UNCONSCIOUS))
    AddAct(ACT_UNCONSCIOUS);
  if (o.IsAct(ACT_SLEEP))
    AddAct(ACT_SLEEP);
  if (o.IsAct(ACT_REST))
    AddAct(ACT_REST);

  parent = nullptr;

  if (o.IsActive())
    Activate();

  minds = o.minds; // Transmit Attached Minds

  no_seek = false;
  no_hear = false;
  tickstep = -1;
}

std::string Object::Pron() const {
  std::string ret;
  if (Gender() == 'M') {
    ret = "he";
  } else if (Gender() == 'F') {
    ret = "she";
  } else {
    ret = "it";
  }
  return ret;
}

std::string Object::Poss() const {
  std::string ret;
  if (Gender() == 'M') {
    ret = "his";
  } else if (Gender() == 'F') {
    ret = "her";
  } else {
    ret = "its";
  }
  return ret;
}

std::string Object::Obje() const {
  std::string ret;
  if (Gender() == 'M') {
    ret = "him";
  } else if (Gender() == 'F') {
    ret = "her";
  } else {
    ret = "it";
  }
  return ret;
}

// Generate truly-formatted name
std::string Object::Name(int definite, Object* rel, Object* sub) const {
  static std::string local;
  int need_an = 0;
  int proper = 0;
  std::string ret;

  if (rel == this && sub == this)
    return "yourself";
  else if (rel == this)
    return "you";

  // FIXME: Hack!  Really detect/specify reflexives?
  else if (rel == nullptr && sub == this && sub->Gender() == 'F')
    return "her";
  else if (rel == nullptr && sub == this && sub->Gender() == 'M')
    return "him";
  else if (rel == nullptr && sub == this)
    return "it";

  else if (sub == this && sub->Gender() == 'F')
    return "herself";
  else if (sub == this && sub->Gender() == 'M')
    return "himself";
  else if (sub == this)
    return "itself";

  if (!strncmp(short_desc.c_str(), "a ", 2)) {
    ret = (short_desc.c_str() + 2);
    need_an = 0;
  } else if (!strncmp(short_desc.c_str(), "an ", 3)) {
    ret = (short_desc.c_str() + 3);
    need_an = 1;
  } else if (!strncmp(short_desc.c_str(), "the ", 4)) {
    ret = (short_desc.c_str() + 4);
    definite = 1;
  } else {
    ret = short_desc.c_str();
    proper = 1;
  }

  if (!IsAnimate()) {
    Object* own = Owner();
    if (own && own == rel) {
      ret = std::string("your ") + ret;
    } else if (own && own == sub && own->Gender() == 'F') {
      ret = std::string("her ") + ret;
    } else if (own && own == sub && own->Gender() == 'M') {
      ret = std::string("his ") + ret;
    } else if (own && own == sub) {
      ret = std::string("its ") + ret;
    } else if (own) {
      ret = own->Name() + "'s " + ret;
    } else if (definite && (!proper)) {
      ret = std::string("the ") + ret;
    } else if ((!proper) && need_an) {
      ret = std::string("an ") + ret;
    } else if (!proper) {
      ret = std::string("a ") + ret;
    }
  } else if (definite && (!proper)) {
    ret = std::string("the ") + ret;
  } else if ((!proper) && need_an) {
    ret = std::string("an ") + ret;
  } else if (!proper) {
    ret = std::string("a ") + ret;
  }

  local = ret;
  return local;
}

std::string Object::ShortDesc() const {
  return short_desc;
}

std::string Object::Desc() const {
  if (desc.empty())
    return ShortDesc();
  return desc;
}

std::string Object::LongDesc() const {
  if (long_desc.empty())
    return Desc();
  return long_desc;
}

static void trim(std::string& s) {
  trim_string(s);

  // Also remove N00bScript tags
  size_t n00b = s.find('@');
  while (n00b != std::string::npos) {
    // fprintf(stderr, "Step: %s\n", s.c_str());
    if (s[n00b + 1] == '@') { //@@ -> @
      s = s.substr(0, n00b) + "@" + s.substr(n00b + 2);
      n00b = s.find('@', n00b + 1);
    } else { // FIXME: Actually use ANSI colors?
      s = s.substr(0, n00b) + s.substr(n00b + 2);
      n00b = s.find('@', n00b);
    }
    // if(n00b == std::string::npos) fprintf(stderr, "Done: %s\n\n", s.c_str());
  }
}

void Object::SetShortDesc(const std::string& d) {
  std::string s = d;
  trim(s);
  short_desc = s;
}

void Object::SetDesc(const std::string& d) {
  std::string s = d;
  trim(s);
  desc = s;
}

void Object::SetLongDesc(const std::string& d) {
  std::string s = d;
  trim(s);
  long_desc = s;
}

void Object::SetParent(Object* o) {
  parent = o;
  if (o)
    o->AddLink(this);
}

void Object::SendContents(Object* targ, Object* o, int vmode, std::string b) {
  for (auto m : targ->minds) {
    SendContents(m, o, vmode, b);
  }
}

void Object::SendShortDesc(Object* targ, Object* o) {
  for (auto m : targ->minds) {
    SendShortDesc(m, o);
  }
}

void Object::SendDesc(Object* targ, Object* o) {
  for (auto m : targ->minds) {
    SendDesc(m, o);
  }
}

void Object::SendDescSurround(Object* targ, Object* o, int vmode) {
  for (auto m : targ->minds) {
    SendDescSurround(m, o, vmode);
  }
}

void Object::SendLongDesc(Object* targ, Object* o) {
  for (auto m : targ->minds) {
    SendLongDesc(m, o);
  }
}

static std::string base = "";
static char buf[65536];

void Object::SendActions(Mind* m) {
  for (auto cur : act) {
    if (cur.act() < ACT_WEAR_BACK) {
      std::string targ;
      std::string dirn = "";
      std::string dirp = "";

      if (!cur.obj())
        targ = "";
      else
        targ = cur.obj()->Name(0, m->Body(), this);

      //      //FIXME: Busted!  This should be the "pointing north to bob"
      //      thingy.
      //      for(auto dir : connections) {
      //	if(dir.second == cur.obj()) {
      //	  dirn = dir.first;
      //	  dirp = " ";
      //	  break;
      //	  }
      //	}
      m->Send(", ");
      m->SendF(act_str[cur.act()].c_str(), targ.c_str(), dirn.c_str(), dirp.c_str());
    }
  }
  if (HasSkill(crc32c("Invisible"))) {
    m->Send(", invisible");
  }
  if (HasSkill(crc32c("Light Source"))) {
    if (Skill(crc32c("Light Source")) < 20)
      m->Send(", glowing");
    else if (HasSkill(crc32c("Lightable")))
      m->Send(", burning");
    else if (Skill(crc32c("Light Source")) < 200)
      m->Send(", lighting the area");
    else
      m->Send(", shining");
  }
  if (HasSkill(crc32c("Noise Source"))) {
    if (Skill(crc32c("Noise Source")) < 20)
      m->Send(", humming");
    else if (Skill(crc32c("Noise Source")) < 200)
      m->Send(", buzzing");
    else
      m->Send(", roaring");
  }
  m->Send(".\n");
}

void Object::SendExtendedActions(Mind* m, int vmode) {
  std::map<Object*, std::string> shown;
  for (auto cur : act) {
    if ((vmode & (LOC_TOUCH | LOC_HEAT | LOC_NINJA)) == 0 // Can't See/Feel Invis
        && cur.obj() && cur.obj()->Skill(crc32c("Invisible")) > 0) {
      continue; // Don't show invisible equip
    }
    if (cur.act() == ACT_HOLD)
      m->SendF("%24s", "Held: ");
    else if (cur.act() == ACT_WIELD)
      m->SendF("%24s", "Wielded: ");
    else if (cur.act() == ACT_WEAR_BACK)
      m->SendF("%24s", "Worn on back: ");
    else if (cur.act() == ACT_WEAR_CHEST)
      m->SendF("%24s", "Worn on chest: ");
    else if (cur.act() == ACT_WEAR_HEAD)
      m->SendF("%24s", "Worn on head: ");
    else if (cur.act() == ACT_WEAR_FACE)
      m->SendF("%24s", "Worn on face: ");
    else if (cur.act() == ACT_WEAR_NECK)
      m->SendF("%24s", "Worn on neck: ");
    else if (cur.act() == ACT_WEAR_COLLAR)
      m->SendF("%24s", "Worn on collar: ");
    else if (cur.act() == ACT_WEAR_WAIST)
      m->SendF("%24s", "Worn on waist: ");
    else if (cur.act() == ACT_WEAR_SHIELD)
      m->SendF("%24s", "Worn as shield: ");
    else if (cur.act() == ACT_WEAR_LARM)
      m->SendF("%24s", "Worn on left arm: ");
    else if (cur.act() == ACT_WEAR_RARM)
      m->SendF("%24s", "Worn on right arm: ");
    else if (cur.act() == ACT_WEAR_LFINGER)
      m->SendF("%24s", "Worn on left finger: ");
    else if (cur.act() == ACT_WEAR_RFINGER)
      m->SendF("%24s", "Worn on right finger: ");
    else if (cur.act() == ACT_WEAR_LFOOT)
      m->SendF("%24s", "Worn on left foot: ");
    else if (cur.act() == ACT_WEAR_RFOOT)
      m->SendF("%24s", "Worn on right foot: ");
    else if (cur.act() == ACT_WEAR_LHAND)
      m->SendF("%24s", "Worn on left hand: ");
    else if (cur.act() == ACT_WEAR_RHAND)
      m->SendF("%24s", "Worn on right hand: ");
    else if (cur.act() == ACT_WEAR_LLEG)
      m->SendF("%24s", "Worn on left leg: ");
    else if (cur.act() == ACT_WEAR_RLEG)
      m->SendF("%24s", "Worn on right leg: ");
    else if (cur.act() == ACT_WEAR_LWRIST)
      m->SendF("%24s", "Worn on left wrist: ");
    else if (cur.act() == ACT_WEAR_RWRIST)
      m->SendF("%24s", "Worn on right wrist: ");
    else if (cur.act() == ACT_WEAR_LSHOULDER)
      m->SendF("%24s", "Worn on left shoulder: ");
    else if (cur.act() == ACT_WEAR_RSHOULDER)
      m->SendF("%24s", "Worn on right shoulder: ");
    else if (cur.act() == ACT_WEAR_LHIP)
      m->SendF("%24s", "Worn on left hip: ");
    else if (cur.act() == ACT_WEAR_RHIP)
      m->SendF("%24s", "Worn on right hip: ");
    else
      continue;

    if ((vmode & (LOC_HEAT | LOC_NINJA)) == 0 // Can't see (but can touch)
        && cur.obj() && cur.obj()->Skill(crc32c("Invisible")) > 0) {
      m->Send(CGRN "Something invisible.\n" CNRM);
      continue; // Don't show details of invisible equip
    }

    std::string targ;
    if (!cur.obj())
      targ = "";
    else
      targ = cur.obj()->Name(0, m->Body(), this);

    char qty[256] = {0};
    if (cur.obj()->Skill(crc32c("Quantity")) > 1)
      sprintf(qty, "(x%d) ", cur.obj()->Skill(crc32c("Quantity")));

    if (shown.count(cur.obj()) > 0) {
      m->SendF("%s%s (%s).\n", qty, targ.c_str(), shown[cur.obj()].c_str());
    } else {
      m->SendF(CGRN "%s%s.\n" CNRM, qty, targ.c_str());
      if (cur.obj()->Skill(crc32c("Open")) || cur.obj()->Skill(crc32c("Transparent"))) {
        sprintf(buf, "%16s  %c", " ", 0);
        base = buf;
        cur.obj()->SendContents(m, nullptr, vmode);
        base = "";
        m->Send(CNRM);
      } else if (cur.obj()->Skill(crc32c("Container"))) {
        if ((vmode & 1) && cur.obj()->Skill(crc32c("Locked"))) {
          std::string mes =
              base + CNRM + "                " + "  It is closed and locked.\n" + CGRN;
          m->Send(mes.c_str());
        } else if (vmode & 1) {
          sprintf(buf, "%16s  %c", " ", 0);
          base = buf;
          cur.obj()->SendContents(m, nullptr, vmode);
          base = "";
          m->Send(CNRM);
        }
      }
    }
    shown[cur.obj()] = "already listed";
  }
}

void Object::SendContents(Mind* m, Object* o, int vmode, std::string b) {
  auto cont = contents;

  if (!b.empty())
    base += b;

  std::set<Object*> master;
  master.insert(cont.begin(), cont.end());

  for (act_t actt = ACT_HOLD; actt < ACT_MAX; actt = act_t(int(actt) + 1)) {
    master.erase(ActTarg(actt)); // Don't show worn/wielded stuff.
  }

  int tlines = 0, total = 0;
  for (auto ind : cont)
    if (master.count(ind)) {
      if ((vmode & LOC_NINJA) == 0 && Parent() != nullptr) { // NinjaMode/CharRoom
        if (ind->Skill(crc32c("Invisible")) > 999)
          continue;
        if (ind->HasSkill(crc32c("Invisible"))) {
          // Can't detect it at all
          if ((vmode & (LOC_TOUCH | LOC_HEAT)) == 0)
            continue;
          // Can't see it, and it's mobile, so can't feel it
          if ((vmode & LOC_TOUCH) == 0 && ind->Pos() > POS_SIT)
            continue;
        }
        if (ind->Skill(crc32c("Hidden")) > 0)
          continue;
      }

      if (ind->HasSkill(crc32c("Invisible")) && (vmode & (LOC_HEAT | LOC_NINJA)) == 0) {
        if (base != "")
          m->SendF("%s%sInside: ", base.c_str(), CNRM);
        m->Send(CGRN "Something invisible is here.\n" CNRM);
        continue; // Can feel, but can't see
      }

      if (ind->IsAct(ACT_SPECIAL_LINKED)) {
        if (ind->ActTarg(ACT_SPECIAL_LINKED) && ind->ActTarg(ACT_SPECIAL_LINKED)->Parent()) {
          if (base != "")
            m->SendF("%s%sInside: ", base.c_str(), CNRM);
          m->Send(CCYN);
          std::string send = ind->ShortDesc();
          if (!(ind->Skill(crc32c("Open")) || ind->Skill(crc32c("Transparent")))) {
            send += ", the door is closed.\n";
          } else {
            if (ind->Skill(crc32c("Closeable")))
              send += ", through an open door,";
            send += " you see ";
            send += ind->ActTarg(ACT_SPECIAL_LINKED)->Parent()->ShortDesc();
            send += ".\n";
          }
          send[0] = ascii_toupper(send[0]);
          m->Send(send.c_str());
          m->Send(CNRM);
        }
        continue;
      }

      master.erase(ind);

      if (ind != o) {
        /* Comment out this block to disable 20-item limit in view */
        if (tlines >= 20) {
          int ignore = 0;
          if (o && o->Parent() == this)
            ignore = 1;
          m->Send(base.c_str());
          m->SendF(
              CGRN "...and %d more things are here too.\n" CNRM,
              ((int)(cont.size())) - tlines - ignore);
          break;
        }

        if (base != "")
          m->SendF("%sInside: ", base.c_str());

        /*	Uncomment this and comment the block below to disable
           auto-pluralizing.
              int qty = std::max(1, ind->Skill(crc32c("Quantity")));
        */
        int qty = 1; // Even animate objects can have higher quantities.
        auto oth = std::find(cont.begin(), cont.end(), ind);
        for (qty = 0; oth != cont.end(); ++oth) {
          if (ind->LooksLike(*oth, vmode)) {
            master.erase(*oth);
            qty += std::max(1, (*oth)->Skill(crc32c("Quantity")));
          }
        }

        if (ind->IsAnimate())
          m->Send(CYEL);
        else
          m->Send(CGRN);

        if (qty > 1)
          m->SendF("(x%d) ", qty);
        total += qty;
        ++tlines;

        if (ind->parent && ind->parent->Skill(crc32c("Container")))
          sprintf(buf, "%s%c", ind->ShortDesc().c_str(), 0);
        else
          sprintf(buf, "%s %s%c", ind->ShortDesc().c_str(), ind->PosString().c_str(), 0);
        buf[0] = ascii_toupper(buf[0]);
        m->Send(buf);

        ind->SendActions(m);

        m->Send(CNRM);
        if (ind->Skill(crc32c("Open")) || ind->Skill(crc32c("Transparent"))) {
          std::string tmp = base;
          base += "  ";
          ind->SendContents(m, o, vmode);
          base = tmp;
        } else if (ind->Skill(crc32c("Container")) || ind->Skill(crc32c("Liquid Container"))) {
          if ((vmode & 1) && ind->Skill(crc32c("Locked"))) {
            std::string mes = base + "  It is closed and locked, you can't see inside.\n";
            m->Send(mes.c_str());
          } else if (vmode & 1) {
            std::string tmp = base;
            base += "  ";
            ind->SendContents(m, o, vmode);
            base = tmp;
          }
        }
      }
    }
  if (!b.empty())
    base = "";
}

void Object::SendShortDesc(Mind* m, Object* o) {
  memset(buf, 0, 65536);
  sprintf(buf, "%s\n", ShortDesc().c_str());
  m->Send(buf);
}

void Object::SendFullSituation(Mind* m, Object* o) {
  std::string pname = "its";
  if (parent && parent->Gender() == 'M')
    pname = "his";
  else if (parent && parent->Gender() == 'F')
    pname = "her";

  if (Skill(crc32c("Quantity")) > 1) {
    sprintf(buf, "(x%d) ", Skill(crc32c("Quantity")));
    m->Send(buf);
  }

  if (!parent)
    sprintf(buf, "%s is here%c", Name().c_str(), 0);

  else if (parent->ActTarg(ACT_HOLD) == this)
    sprintf(buf, "%s is here in %s off-hand%c", Name().c_str(), pname.c_str(), 0);

  else if (parent->ActTarg(ACT_WIELD) == this)
    sprintf(buf, "%s is here in %s hand%c", Name().c_str(), pname.c_str(), 0);

  else if (parent->ActTarg(ACT_WEAR_BACK) == this)
    sprintf(buf, "%s is here on %s back%c", Name().c_str(), pname.c_str(), 0);

  else if (parent->ActTarg(ACT_WEAR_CHEST) == this)
    sprintf(buf, "%s is here on %s chest%c", Name().c_str(), pname.c_str(), 0);

  else if (parent->ActTarg(ACT_WEAR_HEAD) == this)
    sprintf(buf, "%s is here on %s head%c", Name().c_str(), pname.c_str(), 0);

  else if (parent->ActTarg(ACT_WEAR_FACE) == this)
    sprintf(buf, "%s is here on %s face%c", Name().c_str(), pname.c_str(), 0);

  else if (parent->ActTarg(ACT_WEAR_NECK) == this)
    sprintf(buf, "%s is here around %s neck%c", Name().c_str(), pname.c_str(), 0);

  else if (parent->ActTarg(ACT_WEAR_COLLAR) == this)
    sprintf(buf, "%s is here on %s neck%c", Name().c_str(), pname.c_str(), 0);

  else if (parent->ActTarg(ACT_WEAR_WAIST) == this)
    sprintf(buf, "%s is here around %s waist%c", Name().c_str(), pname.c_str(), 0);

  else if (parent->ActTarg(ACT_WEAR_SHIELD) == this)
    sprintf(buf, "%s is here on %s shield-arm%c", Name().c_str(), pname.c_str(), 0);

  else if (parent->ActTarg(ACT_WEAR_LARM) == this)
    sprintf(buf, "%s is here on %s left arm%c", Name().c_str(), pname.c_str(), 0);

  else if (parent->ActTarg(ACT_WEAR_RARM) == this)
    sprintf(buf, "%s is here on %s right arm%c", Name().c_str(), pname.c_str(), 0);

  else if (parent->ActTarg(ACT_WEAR_LFINGER) == this)
    sprintf(buf, "%s is here on %s left finger%c", Name().c_str(), pname.c_str(), 0);

  else if (parent->ActTarg(ACT_WEAR_RFINGER) == this)
    sprintf(buf, "%s is here on %s right finger%c", Name().c_str(), pname.c_str(), 0);

  else if (parent->ActTarg(ACT_WEAR_LFOOT) == this)
    sprintf(buf, "%s is here on %s left foot%c", Name().c_str(), pname.c_str(), 0);

  else if (parent->ActTarg(ACT_WEAR_RFOOT) == this)
    sprintf(buf, "%s is here on %s right foot%c", Name().c_str(), pname.c_str(), 0);

  else if (parent->ActTarg(ACT_WEAR_LHAND) == this)
    sprintf(buf, "%s is here on %s left hand%c", Name().c_str(), pname.c_str(), 0);

  else if (parent->ActTarg(ACT_WEAR_RHAND) == this)
    sprintf(buf, "%s is here on %s right hand%c", Name().c_str(), pname.c_str(), 0);

  else if (parent->ActTarg(ACT_WEAR_LLEG) == this)
    sprintf(buf, "%s is here on %s left leg%c", Name().c_str(), pname.c_str(), 0);

  else if (parent->ActTarg(ACT_WEAR_RLEG) == this)
    sprintf(buf, "%s is here on %s right leg%c", Name().c_str(), pname.c_str(), 0);

  else if (parent->ActTarg(ACT_WEAR_LWRIST) == this)
    sprintf(buf, "%s is here on %s left wrist%c", Name().c_str(), pname.c_str(), 0);

  else if (parent->ActTarg(ACT_WEAR_RWRIST) == this)
    sprintf(buf, "%s is here on %s right wrist%c", Name().c_str(), pname.c_str(), 0);

  else if (parent->ActTarg(ACT_WEAR_LSHOULDER) == this)
    sprintf(buf, "%s is here on %s left shoulder%c", Name().c_str(), pname.c_str(), 0);

  else if (parent->ActTarg(ACT_WEAR_RSHOULDER) == this)
    sprintf(buf, "%s is here on %s right shoulder%c", Name().c_str(), pname.c_str(), 0);

  else if (parent->ActTarg(ACT_WEAR_LHIP) == this)
    sprintf(buf, "%s is here on %s left hip%c", Name().c_str(), pname.c_str(), 0);

  else if (parent->ActTarg(ACT_WEAR_RHIP) == this)
    sprintf(buf, "%s is here on %s right hip%c", Name().c_str(), pname.c_str(), 0);

  else {
    pname = parent->Name();
    sprintf(buf, "%s %s in %s%c", Name().c_str(), PosString().c_str(), pname.c_str(), 0);
  }

  buf[0] = ascii_toupper(buf[0]);
  m->Send(buf);
}

void Object::SendDesc(Mind* m, Object* o) {
  memset(buf, 0, 65536);

  if (pos != POS_NONE) {
    m->Send(CCYN);
    SendFullSituation(m, o);
    SendActions(m);
  } else {
    m->Send(CCYN);
    sprintf(buf, "%s\n%c", ShortDesc().c_str(), 0);
    buf[0] = ascii_toupper(buf[0]);
    m->Send(buf);
  }

  m->SendF("%s   ", CNRM);
  sprintf(buf, "%s\n%c", Desc().c_str(), 0);
  buf[0] = ascii_toupper(buf[0]);
  m->Send(buf);
  m->Send(CNRM);
}

void Object::SendDescSurround(Mind* m, Object* o, int vmode) {
  if (no_seek)
    return;
  memset(buf, 0, 65536);

  if (pos != POS_NONE) {
    m->Send(CCYN);
    SendFullSituation(m, o);
    SendActions(m);
  } else {
    m->Send(CCYN);
    sprintf(buf, "%s\n%c", ShortDesc().c_str(), 0);
    buf[0] = ascii_toupper(buf[0]);
    m->Send(buf);
  }

  m->SendF("%s   ", CNRM);
  sprintf(buf, "%s\n%c", Desc().c_str(), 0);
  buf[0] = ascii_toupper(buf[0]);
  m->Send(buf);

  m->Send(CNRM);
  SendExtendedActions(m, vmode);

  if ((!parent) || Contains(o) || Skill(crc32c("Open")) || Skill(crc32c("Transparent"))) {
    SendContents(m, o, vmode);
  }

  if (parent && (Skill(crc32c("Open")) || Skill(crc32c("Transparent")))) {
    m->Send(CCYN);
    m->Send("Outside you see: ");
    no_seek = true;
    parent->SendDescSurround(m, this, vmode);
    no_seek = false;
  }

  m->Send(CNRM);
}

void Object::SendLongDesc(Mind* m, Object* o) {
  if (pos != POS_NONE) {
    m->Send(CCYN);
    SendFullSituation(m, o);
    SendActions(m);
  } else {
    m->Send(CCYN);
    sprintf(buf, "%s\n%c", ShortDesc().c_str(), 0);
    buf[0] = ascii_toupper(buf[0]);
    m->Send(buf);
  }

  m->SendF("%s   ", CNRM);
  sprintf(buf, "%s\n%c", LongDesc().c_str(), 0);
  buf[0] = ascii_toupper(buf[0]);
  m->Send(buf);
  m->Send(CNRM);
}

static const std::string atnames[] = {"Bod", "Qui", "Str", "Cha", "Int", "Wil"};
void Object::SendScore(Mind* m, Object* o) {
  if (!m)
    return;
  m->SendF("\n%s", CNRM);
  for (int ctr = 0; ctr < 6; ++ctr) {
    if (std::min(NormAttribute(ctr), 99) == std::min(ModAttribute(ctr), 99)) {
      m->SendF("%s: %2d     ", atnames[ctr].c_str(), std::min(ModAttribute(ctr), 99));
    } else if (ModAttribute(ctr) > 9) { // 2-Digits!
      m->SendF(
          "%s: %2d (%d)",
          atnames[ctr].c_str(),
          std::min(NormAttribute(ctr), 99),
          std::min(ModAttribute(ctr), 99));
    } else { // 1 Digit!
      m->SendF(
          "%s: %2d (%d) ",
          atnames[ctr].c_str(),
          std::min(NormAttribute(ctr), 99),
          std::min(ModAttribute(ctr), 99));
    }
    if (ctr == 0) {
      m->Send("         L     M        S           D");
    } else if (ctr == 1) {
      m->SendF(
          "  Stun: [%c][%c][%c][%c][%c][%c][%c][%c][%c][%c]",
          stun <= 0 ? ' ' : 'X',
          stun <= 1 ? ' ' : 'X',
          stun <= 2 ? ' ' : 'X',
          stun <= 3 ? ' ' : 'X',
          stun <= 4 ? ' ' : 'X',
          stun <= 5 ? ' ' : 'X',
          stun <= 6 ? ' ' : 'X',
          stun <= 7 ? ' ' : 'X',
          stun <= 8 ? ' ' : 'X',
          stun <= 9 ? ' ' : 'X');
    } else if (ctr == 2) {
      m->SendF(
          "  Phys: [%c][%c][%c][%c][%c][%c][%c][%c][%c][%c]",
          phys <= 0 ? ' ' : 'X',
          phys <= 1 ? ' ' : 'X',
          phys <= 2 ? ' ' : 'X',
          phys <= 3 ? ' ' : 'X',
          phys <= 4 ? ' ' : 'X',
          phys <= 5 ? ' ' : 'X',
          phys <= 6 ? ' ' : 'X',
          phys <= 7 ? ' ' : 'X',
          phys <= 8 ? ' ' : 'X',
          phys <= 9 ? ' ' : 'X');
      if (phys > 10) {
        m->SendF(" Overflow: %d", phys - 10);
      }
    } else if (ctr == 3) {
      m->SendF(
          "  Stru: [%c][%c][%c][%c][%c][%c][%c][%c][%c][%c]",
          stru <= 0 ? ' ' : 'X',
          stru <= 1 ? ' ' : 'X',
          stru <= 2 ? ' ' : 'X',
          stru <= 3 ? ' ' : 'X',
          stru <= 4 ? ' ' : 'X',
          stru <= 5 ? ' ' : 'X',
          stru <= 6 ? ' ' : 'X',
          stru <= 7 ? ' ' : 'X',
          stru <= 8 ? ' ' : 'X',
          stru <= 9 ? ' ' : 'X');
    } else if (ctr == 5) {
      m->SendF(
          "  Sex: %c, %d.%.3dkg, %d.%.3dm, %dv, %dY\n",
          gender,
          weight / 1000,
          weight % 1000,
          size / 1000,
          size % 1000,
          volume,
          value);
    }
    m->Send("\n");
  }

  std::vector<std::string> col1;
  auto skls = GetSkills();

  if (!IsAnimate()) { // Inanimate
    col1 = FormatStats(skls);
  } else {
    col1 = FormatSkills(skls);
  }

  auto c1 = col1.begin();
  auto c2 = std::find_if(skls.begin(), skls.end(), [](auto skl) { return !is_skill(skl.first); });
  while (c1 != col1.end() || c2 != skls.end()) {
    if (c1 != col1.end()) {
      m->SendF("%41s ", c1->c_str()); // Note: 41 is 32 (2 Color Escape Codes)
      ++c1;
    } else {
      m->SendF("%32s ", "");
    }

    if (c2 != skls.end()) {
      m->SendF("%28s: %8d", SkillName(c2->first).c_str(), c2->second);
      c2++;
      if (c2 != skls.end()) {
        c2 = std::find_if(c2, skls.end(), [](auto skl) { return !is_skill(skl.first); });
      }
    }

    m->Send("\n");
  }

  for (act_t actt = ACT_MAX; actt < ACT_SPECIAL_MAX; actt = act_t(int(actt) + 1)) {
    if (ActTarg(actt)) {
      m->SendF(CGRN "  %s -> %s\n" CNRM, act_str[actt].c_str(), ActTarg(actt)->Name().c_str());
    } else if (IsAct(actt)) {
      m->SendF(CGRN "  %s\n" CNRM, act_str[actt].c_str());
    }
  }

  if (IsActive())
    m->Send(CCYN "  ACTIVE\n" CNRM);

  for (auto mind : minds) {
    if (mind->Owner()) {
      m->SendF(CBLU "->Player Connected: %s\n" CNRM, mind->Owner()->Name().c_str());
    } else if (mind == get_mob_mind()) {
      m->Send(CBLU "->MOB_MIND\n" CNRM);
    } else if (mind == get_tba_mob_mind()) {
      m->Send(CBLU "->TBA_MOB_MIND\n" CNRM);
    } else if (mind->Type() == MIND_TBATRIG) {
      m->Send(CBLU "->TBA_TRIGGER\n" CNRM);
    }
  }

  // Other Misc Stats
  if (LightLevel() > 0) {
    m->SendF(CYEL "  Light Level: %d (%d)\n" CNRM, Skill(crc32c("Light Source")), LightLevel());
  }
  if (Power("Cursed")) {
    m->SendF(CRED "  Cursed: %d\n" CNRM, Power("Cursed"));
  }

  // Experience Summary
  if (IsAnimate()) {
    m->Send(CYEL);
    m->SendF("\nEarned Exp: %4d  Unspent Exp: %4d\n", Exp(), TotalExp());
    if (Power("Heat Vision") || Power("Dark Vision")) {
      m->SendF("Heat/Dark Vision: %d/%d\n", Power("Heat Vision"), Power("Dark Vision"));
    }
    m->Send(CNRM);
  }
}

std::vector<std::string> Object::FormatSkills(const MinVec<3, skill_pair>& skls) {
  std::vector<std::string> ret;

  auto save = skls;
  for (auto skl : save) {
    if (is_skill(skl.first)) {
      char buf2[256];
      sprintf(
          buf2, "%28s: " CYEL "%2d" CNRM, SkillName(skl.first).c_str(), std::min(99, skl.second));
      ret.push_back(buf2);
    }
  }
  return ret;
}

static void stick_on(
    std::vector<std::string>& out,
    const MinVec<3, skill_pair>& skls,
    uint32_t stok,
    const std::string label) {
  char buf2[256];

  auto itr =
      std::find_if(skls.begin(), skls.end(), [stok](auto skl) { return (skl.first == stok); });
  if (itr != skls.end()) {
    if (itr->second > 0) {
      sprintf(
          buf2,
          "  %18s: " CYEL "%d.%.3d" CNRM,
          label.c_str(),
          itr->second / 1000,
          itr->second % 1000);
      out.push_back(buf2);
    }
  }
}

std::vector<std::string> Object::FormatStats(const MinVec<3, skill_pair>& skls) {
  std::vector<std::string> ret;

  if (HasSkill(crc32c("TBAScriptType"))) { // It's a TBA script
    char buf2[256] = {};
    auto type_name = CRED "BAD-TYPE" CNRM;
    switch (Skill(crc32c("TBAScriptType"))) {
      case (0x1000001): {
        type_name = CYEL "MOB-GLOBAL" CNRM;
        break;
      }
      case (0x1000002): {
        type_name = CGRN "MOB-RANDOM" CNRM;
        break;
      }
      case (0x1000004): {
        type_name = CGRN "MOB-COMMAND" CNRM;
        break;
      }
      case (0x1000008): {
        type_name = CGRN "MOB-SPEECH" CNRM;
        break;
      }
      case (0x1000010): {
        type_name = CGRN "MOB-ACT" CNRM;
        break;
      }
      case (0x1000020): {
        type_name = CYEL "MOB-DEATH" CNRM;
        break;
      }
      case (0x1000040): {
        type_name = CGRN "MOB-GREET" CNRM;
        break;
      }
      case (0x1000080): {
        type_name = CYEL "MOB-GREET-ALL" CNRM;
        break;
      }
      case (0x1000100): {
        type_name = CYEL "MOB-ENTRY" CNRM;
        break;
      }
      case (0x1000200): {
        type_name = CGRN "MOB-RECEIVE" CNRM;
        break;
      }
      case (0x1000400): {
        type_name = CYEL "MOB-FIGHT" CNRM;
        break;
      }
      case (0x1000800): {
        type_name = CYEL "MOB-HITPRCNT" CNRM;
        break;
      }
      case (0x1001000): {
        type_name = CYEL "MOB-BRIBE" CNRM;
        break;
      }
      case (0x1002000): {
        type_name = CYEL "MOB-LOAD" CNRM;
        break;
      }
      case (0x1004000): {
        type_name = CYEL "MOB-MEMORY" CNRM;
        break;
      }
      case (0x1008000): {
        type_name = CYEL "MOB-CAST" CNRM;
        break;
      }
      case (0x1010000): {
        type_name = CYEL "MOB-LEAVE" CNRM;
        break;
      }
      case (0x1020000): {
        type_name = CYEL "MOB-DOOR" CNRM;
        break;
      }
      case (0x1040000): {
        type_name = CYEL "MOB-TIME" CNRM;
        break;
      }

      case (0x2000001): {
        type_name = CYEL "OBJ-GLOBAL" CNRM;
        break;
      }
      case (0x2000002): {
        type_name = CGRN "OBJ-RANDOM" CNRM;
        break;
      }
      case (0x2000004): {
        type_name = CGRN "OBJ-COMMAND" CNRM;
        break;
      }
      case (0x2000008): {
        type_name = CYEL "OBJ-TIMER" CNRM;
        break;
      }
      case (0x2000010): {
        type_name = CGRN "OBJ-GET" CNRM;
        break;
      }
      case (0x2000020): {
        type_name = CGRN "OBJ-DROP" CNRM;
        break;
      }
      case (0x2000040): {
        type_name = CYEL "OBJ-GIVE" CNRM;
        break;
      }
      case (0x2000080): {
        type_name = CGRN "OBJ-WEAR" CNRM;
        break;
      }
      case (0x2000100): {
        type_name = CGRN "OBJ-REMOVE" CNRM;
        break;
      }
      case (0x2000200): {
        type_name = CYEL "OBJ-LOAD" CNRM;
        break;
      }
      case (0x2000400): {
        type_name = CYEL "OBJ-CAST" CNRM;
        break;
      }
      case (0x2000800): {
        type_name = CGRN "OBJ-LEAVE" CNRM;
        break;
      }
      case (0x2001000): {
        type_name = CYEL "OBJ-CONSUME" CNRM;
        break;
      }

      case (0x4000001): {
        type_name = CYEL "ROOM-GLOBAL" CNRM;
        break;
      }
      case (0x4000002): {
        type_name = CGRN "ROOM-RANDOM" CNRM;
        break;
      }
      case (0x4000004): {
        type_name = CGRN "ROOM-COMMAND" CNRM;
        break;
      }
      case (0x4000008): {
        type_name = CGRN "ROOM-SPEECH" CNRM;
        break;
      }
      case (0x4000010): {
        type_name = CYEL "ROOM-ZONE" CNRM;
        break;
      }
      case (0x4000020): {
        type_name = CGRN "ROOM-ENTER" CNRM;
        break;
      }
      case (0x4000040): {
        type_name = CGRN "ROOM-DROP" CNRM;
        break;
      }
      case (0x4000080): {
        type_name = CYEL "ROOM-CAST" CNRM;
        break;
      }
      case (0x4000100): {
        type_name = CGRN "ROOM-LEAVE" CNRM;
        break;
      }
      case (0x4000200): {
        type_name = CYEL "ROOM-DOOR" CNRM;
        break;
      }
      case (0x4000400): {
        type_name = CYEL "ROOM-TIME" CNRM;
        break;
      }

      default: {
        type_name = CRED "BAD-TYPE" CNRM;
      }
    }
    sprintf(buf2, "TBAScriptType: %s", type_name);
    ret.push_back(buf2);
  }
  if (HasSkill(crc32c("WeaponType"))) { // It's a Weapon
    // Detailed Weapon Stats
    ret.push_back(
        "Weapon: " CYEL + SkillName(get_weapon_skill(Skill(crc32c("WeaponType")))) + CNRM);
    stick_on(ret, skls, crc32c("Durability"), "Durability");
    stick_on(ret, skls, crc32c("Hardness"), "Hardness");
    stick_on(ret, skls, crc32c("Flexibility"), "Flexibility");
    stick_on(ret, skls, crc32c("Sharpness"), "Sharpness");
    stick_on(ret, skls, crc32c("Distance"), "Pen. Dist");
    stick_on(ret, skls, crc32c("Width"), "Pen. Width");
    stick_on(ret, skls, crc32c("Ratio"), "Pen. Ratio");
    stick_on(ret, skls, crc32c("Hit Weight"), "Hit Weight");
    stick_on(ret, skls, crc32c("Velocity"), "Velocity");
    stick_on(ret, skls, crc32c("Leverage"), "Leverage");
    stick_on(ret, skls, crc32c("Burn"), "Burn");
    stick_on(ret, skls, crc32c("Chill"), "Chill");
    stick_on(ret, skls, crc32c("Zap"), "Zap");
    stick_on(ret, skls, crc32c("Concuss"), "Concuss");
    stick_on(ret, skls, crc32c("Flash"), "Flash");
    stick_on(ret, skls, crc32c("Bang"), "Bang");
    stick_on(ret, skls, crc32c("Irradiate"), "Irradiate");
    stick_on(ret, skls, crc32c("Reach"), "Reach");
    stick_on(ret, skls, crc32c("Range"), "Range");
    stick_on(ret, skls, crc32c("Strength Required"), "Str Req");
    stick_on(ret, skls, crc32c("Multiple"), "Multiple");

    ret.push_back(CYEL CNRM); // Leave a blank line between new and old
    // Must include color escapes for formatting

    // Old-Style (Shadowrun) Weapon Stats
    static char sevs[] = {'-', 'L', 'M', 'S', 'D'};
    ret.push_back(
        " Old Weapon: " CYEL + SkillName(get_weapon_skill(Skill(crc32c("WeaponType")))) + CNRM);

    char buf2[256];
    sprintf(
        buf2,
        "  Damage: " CYEL "(Str+%d)%c",
        Skill(crc32c("WeaponForce")),
        sevs[std::min(4, Skill(crc32c("WeaponSeverity")))]);
    if (Skill(crc32c("WeaponSeverity")) > 4) {
      sprintf(buf2 + strlen(buf2), "%d", (Skill(crc32c("WeaponSeverity")) - 4) * 2);
    }
    strcat(buf2, CNRM);
    ret.push_back(buf2);

    if (Skill(crc32c("WeaponReach")) > 4) {
      sprintf(buf2, "  Range: " CYEL "%d" CNRM, Skill(crc32c("WeaponReach")));
      ret.push_back(buf2);
    } else if (Skill(crc32c("WeaponReach")) >= 0) {
      sprintf(buf2, "  Reach: " CYEL "%d" CNRM, Skill(crc32c("WeaponReach")));
      ret.push_back(buf2);
    }
  }
  if (HasSkill(crc32c("Thickness"))) { // It's Armor (or just Clothing)
    // stick_on(ret, skls, crc32c("Coverage"), "Coverage");
    stick_on(ret, skls, crc32c("Durability"), "Durability");
    stick_on(ret, skls, crc32c("Hardness"), "Hardness");
    stick_on(ret, skls, crc32c("Flexibility"), "Flexibility");
    stick_on(ret, skls, crc32c("Sharpness"), "Sharpness");
    stick_on(ret, skls, crc32c("Thickness"), "Thickness");
    stick_on(ret, skls, crc32c("Max Gap"), "Max Gap");
    stick_on(ret, skls, crc32c("Min Gap"), "Min Gap");
    stick_on(ret, skls, crc32c("Hit Weight"), "Hit Weight");
    stick_on(ret, skls, crc32c("Ballistic"), "Ballistic");
    stick_on(ret, skls, crc32c("Bulk"), "Bulk");
    stick_on(ret, skls, crc32c("Warm"), "Warm");
    stick_on(ret, skls, crc32c("Reflect"), "Reflect");
    stick_on(ret, skls, crc32c("Insulate"), "Insulate");
    stick_on(ret, skls, crc32c("Padding"), "Padding");
    stick_on(ret, skls, crc32c("Shade"), "Shade");
    stick_on(ret, skls, crc32c("Muffle"), "Muffle");
    stick_on(ret, skls, crc32c("Shielding"), "Shielding");
    stick_on(ret, skls, crc32c("Defense Range"), "Def Range");
    stick_on(ret, skls, crc32c("Strength Required"), "Str Req");
  }

  ret.push_back(CYEL CNRM); // Leave a blank line between weap/arm and gen

  return ret;
}

void Object::AddLink(Object* ob) {
  auto ind = std::find(contents.begin(), contents.end(), ob);
  if (ind == contents.end()) {
    contents.push_back(ob);
    //    auto place = contents.end();
    //    for(ind = contents.begin(); ind != contents.end(); ++ind) {
    //      if(ind == ob) return;				//Already there!
    //      if((*ind) == (*ob)) { place = ind; ++place; }	//Likes by likes
    //      }
    //    contents.insert(place, ob);
  }
}

void Object::RemoveLink(Object* ob) {
  auto ind = std::find(contents.begin(), contents.end(), ob);
  while (ind != contents.end()) {
    contents.erase(ind);
    ind = std::find(contents.begin(), contents.end(), ob);
  }
}

void Object::Link(
    Object* other,
    const std::string& name,
    const std::string& dsc,
    const std::string& oname,
    const std::string& odsc) {
  Object* door1 = new Object(this);
  Object* door2 = new Object(other);
  door1->SetShortDesc(name.c_str());
  door2->SetShortDesc(oname.c_str());
  door1->SetDesc(dsc.c_str());
  door2->SetDesc(odsc.c_str());
  door1->AddAct(ACT_SPECIAL_LINKED, door2);
  door1->AddAct(ACT_SPECIAL_MASTER, door2);
  door1->SetSkill(crc32c("Open"), 1000);
  door1->SetSkill(crc32c("Enterable"), 1);
  door2->AddAct(ACT_SPECIAL_LINKED, door1);
  door2->AddAct(ACT_SPECIAL_MASTER, door1);
  door2->SetSkill(crc32c("Open"), 1000);
  door2->SetSkill(crc32c("Enterable"), 1);
}

void Object::LinkClosed(
    Object* other,
    const std::string& name,
    const std::string& dsc,
    const std::string& oname,
    const std::string& odsc) {
  Object* door1 = new Object(this);
  Object* door2 = new Object(other);
  door1->SetShortDesc(name.c_str());
  door2->SetShortDesc(oname.c_str());
  door1->SetDesc(dsc.c_str());
  door2->SetDesc(odsc.c_str());
  door1->AddAct(ACT_SPECIAL_LINKED, door2);
  door1->AddAct(ACT_SPECIAL_MASTER, door2);
  door1->SetSkill(crc32c("Closeable"), 1);
  door1->SetSkill(crc32c("Enterable"), 1);
  door1->SetSkill(crc32c("Transparent"), 1000);
  door2->AddAct(ACT_SPECIAL_LINKED, door1);
  door2->AddAct(ACT_SPECIAL_MASTER, door1);
  door2->SetSkill(crc32c("Closeable"), 1);
  door2->SetSkill(crc32c("Enterable"), 1);
  door2->SetSkill(crc32c("Transparent"), 1000);
}

void Object::TryCombine() {
  if (!parent)
    return;
  const auto todo = parent->contents;
  for (auto obj : todo) {
    if (obj == this)
      continue; // Skip self

    // Never combine with an actee.
    bool actee = false;
    for (auto a : parent->act) {
      if (a.obj() == obj) {
        actee = true;
        break;
      }
    }
    if (actee)
      continue;

    if (IsSameAs(*obj)) {
      // fprintf(stderr, "Combining '%s'\n", Name().c_str());
      int val;

      val = std::max(1, Skill(crc32c("Quantity"))) + std::max(1, obj->Skill(crc32c("Quantity")));
      SetSkill(crc32c("Quantity"), val);

      val = Skill(crc32c("Hungry")) + obj->Skill(crc32c("Hungry"));
      SetSkill(crc32c("Hungry"), val);

      val = Skill(crc32c("Bored")) + obj->Skill(crc32c("Bored"));
      SetSkill(crc32c("Bored"), val);

      val = Skill(crc32c("Needy")) + obj->Skill(crc32c("Needy"));
      SetSkill(crc32c("Needy"), val);

      val = Skill(crc32c("Tired")) + obj->Skill(crc32c("Tired"));
      SetSkill(crc32c("Tired"), val);

      obj->Recycle();
      break;
    }
  }
}

int Object::Travel(Object* dest, int try_combine) {
  if ((!parent) || (!dest))
    return -1;

  for (Object* chec_recr = dest; chec_recr; chec_recr = chec_recr->parent) {
    if (chec_recr == this)
      return -1; // Check for Recursive Loops
  }

  int cap = dest->Skill(crc32c("Capacity"));
  if (cap > 0) {
    cap -= dest->ContainedVolume();
    if (Volume() > cap)
      return -2;
  }

  int con = dest->Skill(crc32c("Container"));
  if (con > 0) {
    con -= dest->ContainedWeight();
    if (Weight() > con)
      return -3;
  }

  std::string dir = "";
  std::string rdir = "";
  {
    const std::string dirs[6] = {"north", "south", "east", "west", "up", "down"};
    for (int dnum = 0; dnum < 6 && dir[0] == 0; ++dnum) {
      Object* door = parent->PickObject(dirs[dnum], LOC_INTERNAL);
      if (door && door->ActTarg(ACT_SPECIAL_LINKED) &&
          door->ActTarg(ACT_SPECIAL_LINKED)->Parent() &&
          door->ActTarg(ACT_SPECIAL_LINKED)->Parent() == dest) {
        dir = dirs[dnum];
        rdir = dirs[dnum ^ 1]; // Opposite Dir
      }
    }
  }

  if (IsAnimate()) {
    auto trigs = parent->contents;
    for (auto src : parent->contents) {
      trigs.insert(trigs.end(), src->contents.begin(), src->contents.end());
    }

    // Type 0x0010000 (*-LEAVE)
    for (auto trig : trigs) {
      if (trig->Skill(crc32c("TBAScriptType")) & 0x0010000) {
        if ((rand() % 100) < trig->Skill(crc32c("TBAScriptNArg"))) { // % Chance
          // fprintf(stderr, CBLU "Triggering: %s\n" CNRM, trig->Name().c_str());
          if (new_trigger(0, trig, this, dir))
            return 1;
        }
      }
    }
  }

  Object* oldp = parent;
  parent->RemoveLink(this);
  parent = dest;
  oldp->NotifyGone(this, dest);
  parent->AddLink(this);

  auto touches = Touching();
  for (auto touch : touches) {
    touch->NotifyLeft(this, dest);
  }

  if (try_combine)
    TryCombine();

  StopAct(ACT_POINT);
  StopAct(ACT_FOLLOW);
  if (IsAct(ACT_HOLD) && ActTarg(ACT_HOLD)->Parent() != this) { // Dragging
    if (ActTarg(ACT_HOLD)->Parent() != Parent()) { // Didn't come with me!
      StopAct(ACT_HOLD);
    }
  }
  SetSkill(crc32c("Hidden"), 0);

  if (parent->Skill(crc32c("DynamicInit")) > 0) { // Room is dynamic, but uninitialized
    parent->DynamicInit();
  }

  if (parent->Skill(crc32c("Accomplishment"))) {
    for (auto m : minds) {
      if (m->Owner()) {
        Accomplish(parent->Skill(crc32c("Accomplishment")), "finding a secret");
      }
    }
  }

  if (IsAnimate()) {
    auto trigs = parent->contents;
    for (auto src : parent->contents) {
      trigs.insert(trigs.end(), src->contents.begin(), src->contents.end());
    }

    // Type 0x4000040 or 0x1000040 (ROOM-ENTER or MOB-GREET)
    for (auto trig : trigs) {
      if ((trig->Skill(crc32c("TBAScriptType")) & 0x0000040) &&
          (trig->Skill(crc32c("TBAScriptType")) & 0x5000000)) {
        if (trig != this && trig->Parent() != this) {
          if ((rand() % 100) < 1000 * trig->Skill(crc32c("TBAScriptNArg"))) { // % Chance
            // if (trig->Skill(crc32c("TBAScript")) >= 5034503 && trig->Skill(crc32c("TBAScript"))
            // <= 5034507)
            //  fprintf(stderr, CBLU "Triggering: %s\n" CNRM, trig->Name().c_str());
            new_trigger((rand() % 400) + 300, trig, this, rdir);
          }
        }
      }
    }
  }

  return 0;
}

Object::~Object() {
  Recycle(0);
}

void Object::Recycle(int inbin) {
  Deactivate();

  // fprintf(stderr, "Deleting: %s\n", Name(0));

  std::set<Object*> movers;
  std::set<Object*> killers;
  for (auto ind : contents) {
    if (is_pc(ind)) {
      movers.insert(ind);
    } else {
      killers.insert(ind);
    }
  }

  for (auto indk : killers) {
    if (std::find(contents.begin(), contents.end(), indk) != contents.end()) {
      indk->SetParent(nullptr);
      RemoveLink(indk);
      indk->Recycle();
    }
  }
  killers.clear();

  contents.clear();

  for (auto indm : movers) {
    indm->StopAll();
    for (auto ind2 : indm->contents) {
      ind2->SetParent(nullptr);
      killers.insert(ind2);
    }
    indm->contents.clear();
    Object* dest = indm;
    while ((!dest->ActTarg(ACT_SPECIAL_HOME)) && dest->Parent()) {
      dest = dest->Parent();
    }
    if (dest->ActTarg(ACT_SPECIAL_HOME)) {
      dest = dest->ActTarg(ACT_SPECIAL_HOME);
    }
    if (dest == parent)
      dest = universe; // Already there, bail!
    indm->Travel(dest);
  }

  for (auto indk : killers) {
    indk->Recycle();
  }
  killers.clear();

  player_rooms_erase(this);

  auto todo = minds;
  for (auto mind : todo) {
    Unattach(mind);
  }
  minds.clear();

  if (parent) {
    parent->RemoveLink(this);
    parent->NotifyGone(this);
  }

  // Actions over long distances must be notified!
  std::set<Object*> tonotify;
  if (ActTarg(ACT_SPECIAL_MASTER))
    tonotify.insert(ActTarg(ACT_SPECIAL_MASTER));
  if (ActTarg(ACT_SPECIAL_MONITOR))
    tonotify.insert(ActTarg(ACT_SPECIAL_MONITOR));
  if (ActTarg(ACT_SPECIAL_LINKED))
    tonotify.insert(ActTarg(ACT_SPECIAL_LINKED));

  auto touches = Touching();
  for (auto touch : touches) {
    tonotify.insert(touch);
  }

  StopAct(ACT_SPECIAL_MASTER);
  StopAct(ACT_SPECIAL_MONITOR);
  StopAct(ACT_SPECIAL_LINKED);

  for (auto noti : tonotify) {
    int del = 0;
    if (noti->ActTarg(ACT_SPECIAL_MASTER) == this)
      del = 1;
    else if (noti->ActTarg(ACT_SPECIAL_LINKED) == this)
      del = 1;
    noti->NotifyLeft(this);
    if (del)
      noti->Recycle();
  }

  busylist.erase(this);

  if (inbin && trash_bin) {
    parent = trash_bin;
    parent->AddLink(this);
  }

  // fprintf(stderr, "Done deleting: %s\n", Name(0));
}

void Object::Attach(Mind* m) {
  auto itr = minds.begin();
  for (; itr != minds.end() && (*itr) != m; ++itr) {
  }
  if (itr == minds.end()) {
    minds.push_back(m);
  }
}

void Object::Unattach(Mind* m) {
  auto itr = minds.begin();
  for (; itr != minds.end() && (*itr) != m; ++itr) {
  }
  if (itr != minds.end()) {
    minds.erase(itr);
  }
  if (m->Body() == this) {
    m->Unattach();
  }
}

int Object::ContainedWeight() {
  int ret = 0;
  for (auto ind : contents) {
    ret += ind->weight;
  }
  return ret;
}

int Object::ContainedVolume() {
  int ret = 0;
  for (auto ind : contents) {
    ret += ind->volume;
  }
  return ret;
}

static int get_ordinal(const std::string& t) {
  int ret = 0;

  std::string_view text = t;
  if (isdigit(text[0])) {
    text.remove_prefix(std::min(text.find_first_not_of("0123456789"), text.size()));
    if (text[0] == '.') {
      ret = atoi(t.c_str());
    } else if (!isgraph(text[0])) {
      ret = -atoi(t.c_str());
    } else if (text.length() >= 3 && isgraph(text[2])) {
      if (text.substr(0, 2) == "st") {
        ret = atoi(t.c_str());
      } else if (text.substr(0, 2) == "nd") {
        ret = atoi(t.c_str());
      } else if (text.substr(0, 2) == "rd") {
        ret = atoi(t.c_str());
      } else if (text.substr(0, 2) == "th") {
        ret = atoi(t.c_str());
      }
    }
  } else if (text.substr(0, 6) == "first ") {
    ret = 1;
  } else if (text.substr(0, 7) == "second ") {
    ret = 2;
  } else if (text.substr(0, 6) == "third ") {
    ret = 3;
  } else if (text.substr(0, 7) == "fourth ") {
    ret = 4;
  } else if (text.substr(0, 6) == "fifth ") {
    ret = 5;
  } else if (text.substr(0, 6) == "sixth ") {
    ret = 6;
  } else if (text.substr(0, 8) == "seventh ") {
    ret = 7;
  } else if (text.substr(0, 7) == "eighth ") {
    ret = 8;
  } else if (text.substr(0, 6) == "ninth ") {
    ret = 9;
  } else if (text.substr(0, 6) == "tenth ") {
    ret = 10;
  } else if (text.substr(0, 4) == "all ") {
    ret = ALL;
  } else if (text.substr(0, 4) == "all.") {
    ret = ALL;
  } else if (text.substr(0, 5) == "some ") {
    ret = SOME;
  } else if (text.substr(0, 5) == "some.") {
    ret = SOME;
  }
  return ret;
}

static int strip_ordinal(std::string& text) {
  trim_string(text);
  int ret = get_ordinal(text);
  if (ret) {
    while (!text.empty() && isgraph(text[0]) && text[0] != '.')
      text = text.substr(1);
    while (!text.empty() && (!isgraph(text[0]) || text[0] == '.'))
      text = text.substr(1);
  }
  return ret;
}

Object* Object::PickObject(const std::string& name, int loc, int* ordinal) const {
  auto ret = PickObjects(name, loc, ordinal);
  if (ret.size() != 1) {
    return nullptr;
  }
  return ret.front();
}

uint32_t splits[4] = {crc32c("Hungry"), crc32c("Bored"), crc32c("Tired"), crc32c("Needy")};
Object* Object::Split(int nqty) {
  if (nqty < 1)
    nqty = 1;
  int qty = Skill(crc32c("Quantity")) - nqty;
  if (qty < 1)
    qty = 1;

  Object* nobj = new Object(*this);
  nobj->SetParent(Parent());
  nobj->SetSkill(crc32c("Quantity"), (nqty <= 1) ? 0 : nqty);

  SetSkill(crc32c("Quantity"), (qty <= 1) ? 0 : qty);

  for (int ctr = 0; ctr < 4; ++ctr) {
    int val = Skill(splits[ctr]);
    int nval = val / (qty + nqty) * nqty;
    val -= nval;
    SetSkill(splits[ctr], val);
    nobj->SetSkill(splits[ctr], nval);
  }

  return nobj;
}

static int tag(Object* obj, MinVec<1, Object*>& ret, int* ordinal, int vmode = 0) {
  // Only Ninjas in Ninja-Mode should detect these
  if (obj->Skill(crc32c("Invisible")) > 999 && (vmode & LOC_NINJA) == 0)
    return 0;

  // Need Heat Vision to see these
  if (obj->HasSkill(crc32c("Invisible")) && (vmode & (LOC_NINJA | LOC_HEAT)) == 0)
    return 0;

  // Can't be seen/affected (except in char rooms)
  if (obj->Skill(crc32c("Hidden")) > 0 && (vmode & (LOC_NINJA | 1)) == 0)
    return 0;
  Object* nobj = nullptr;

  int cqty = 1, rqty = 1; // Contains / Requires

  if (obj->Skill(crc32c("Quantity")))
    cqty = obj->Skill(crc32c("Quantity"));

  if (*ordinal == -1)
    (*ordinal) = 1; // Need one - make it the first one!

  if (*ordinal == 0)
    return 0; // They don't want anything.

  else if (*ordinal > 0) {
    if (*ordinal > cqty) { // Have not gotten to my targ yet.
      *ordinal -= cqty;
      return 0;
    } else if (cqty == 1) { // Just this one.
      *ordinal = 0;
      ret.push_back(obj);
      return 1;
    } else { // One of this set.
      *ordinal = 0;
      nobj = obj->Split(1);
      ret.push_back(nobj);
      return 1;
    }
  }

  else if (*ordinal == ALL)
    rqty = cqty + 1;
  else if (*ordinal == SOME)
    rqty = cqty;

  else if (*ordinal < -1)
    rqty = -(*ordinal);

  if (rqty == cqty) { // Exactly this entire thing.
    ret.push_back(obj);
    *ordinal = 0;
    return 1;
  } else if (rqty > cqty) { // This entire thing and more.
    ret.push_back(obj);
    if (*ordinal != ALL)
      *ordinal += cqty;
    return 0;
  } else { // Some of this set.
    nobj = obj->Split(rqty);
    ret.push_back(nobj);
    *ordinal = 0;
    return 1;
  }
  return 0;
}

MinVec<1, Object*> Object::PickObjects(std::string name, int loc, int* ordinal) const {
  MinVec<1, Object*> ret;

  trim_string(name);

  int ordcontainer;
  if (ordinal)
    strip_ordinal(name);
  else {
    ordinal = &ordcontainer;
    (*ordinal) = strip_ordinal(name);
  }
  if (!strcmp(name.c_str(), "all"))
    (*ordinal) = ALL;
  if (!strcmp(name.c_str(), "everyone"))
    (*ordinal) = ALL;
  if (!strcmp(name.c_str(), "everything"))
    (*ordinal) = ALL;
  if (!strcmp(name.c_str(), "everywhere"))
    (*ordinal) = ALL;
  if (!(*ordinal))
    (*ordinal) = 1;

  auto poss = std::min(name.find("'s "), name.find("'S "));
  if (poss != std::string::npos) {
    auto possessor = name.substr(0, poss);
    auto possession = name.substr(3 + poss);
    auto masters = PickObjects(possessor, loc, ordinal);
    if (masters.size() > 0) {
      for (auto master : masters) {
        auto add = master->PickObjects(possession, (loc & LOC_SPECIAL) | LOC_INTERNAL);
        ret.insert(ret.end(), add.begin(), add.end());
      }
    }
    return ret;
  }

  int len = name.length();
  while (!isgraph(name[len - 1]))
    --len;

  if (loc & LOC_SELF) {
    if ((!strcmp(name.c_str(), "self")) || (!strcmp(name.c_str(), "myself")) ||
        (!strcmp(name.c_str(), "me"))) {
      if ((*ordinal) != 1)
        return ret;
      ret.push_back((Object*)this); // Wrecks Const-Ness
      return ret;
    }
  }

  if (loc & LOC_HERE) {
    if (!strcmp(name.c_str(), "here")) {
      if ((*ordinal) == 1 && parent)
        ret.push_back(parent);
      return ret;
    }
  }

  if (loc & LOC_INTERNAL) {
    if (!strncmp(name.c_str(), "my ", 3)) {
      return PickObjects(name.substr(3), loc & (LOC_SPECIAL | LOC_INTERNAL | LOC_SELF));
    }
  }

  if ((loc & LOC_NEARBY) && (parent != nullptr)) {
    auto cont = parent->Contents(loc); //"loc" includes vmode.

    for (auto ind : cont)
      if (!ind->no_seek) {
        if (ind == this)
          continue; // Must use "self" to pick self!
        if (ind->Filter(loc) && ind->Matches(name)) {
          if (tag(ind, ret, ordinal, (parent->Parent() == nullptr) | (loc & LOC_SPECIAL))) {
            return ret;
          }
        }
        if (ind->Skill(crc32c("Open")) || ind->Skill(crc32c("Transparent"))) {
          auto add = ind->PickObjects(name, (loc & LOC_SPECIAL) | LOC_INTERNAL, ordinal);
          ret.insert(ret.end(), add.begin(), add.end());

          if ((*ordinal) == 0)
            return ret;
        }
      }
    if (parent->Skill(crc32c("Open")) || parent->Skill(crc32c("Transparent"))) {
      if (parent->parent) {
        parent->no_seek = true;

        auto add = parent->PickObjects(name, (loc & LOC_SPECIAL) | LOC_NEARBY, ordinal);
        ret.insert(ret.end(), add.begin(), add.end());

        parent->no_seek = false;
        if ((*ordinal) == 0)
          return ret;
      }
    }
  }

  if (loc & LOC_INTERNAL) {
    auto cont(contents);

    for (auto action : act) {
      auto ind = std::find(cont.begin(), cont.end(), action.obj());
      if (ind != cont.end()) { // IE: Is action.obj() within cont
        cont.erase(ind);
        if (action.obj()->Filter(loc) && action.obj()->Matches(name) &&
            ((loc & LOC_NOTWORN) == 0 || action.act() <= ACT_HOLD) &&
            ((loc & LOC_NOTUNWORN) == 0 || action.act() >= ACT_HOLD)) {
          if (tag(action.obj(), ret, ordinal, (Parent() == nullptr) | (loc & LOC_SPECIAL))) {
            return ret;
          }
        }
        if (action.obj()->HasSkill(crc32c("Container"))) {
          auto add = action.obj()->PickObjects(name, (loc & LOC_SPECIAL) | LOC_INTERNAL, ordinal);
          ret.insert(ret.end(), add.begin(), add.end());

          if ((*ordinal) == 0)
            return ret;
        }
      }
    }

    for (auto ind : cont) {
      if (ind == this)
        continue; // Must use "self" to pick self!
      if (ind->Filter(loc) && ind->Matches(name)) {
        if (tag(ind, ret, ordinal, (Parent() == nullptr) | (loc & LOC_SPECIAL))) {
          return ret;
        }
      }
      if (ind->Skill(crc32c("Container")) && (loc & LOC_NOTUNWORN) == 0) {
        auto add = ind->PickObjects(name, (loc & LOC_SPECIAL) | LOC_INTERNAL, ordinal);
        ret.insert(ret.end(), add.begin(), add.end());

        if ((*ordinal) == 0)
          return ret;
      }
    }
  }
  return ret;
}

int Object::HasWithin(const Object* obj) {
  if (no_seek)
    return 0;
  for (auto ind : contents) {
    if (ind == obj)
      return 1;
    if (ind->HasWithin(obj))
      return 1;
  }
  return 0;
}

int Object::SeeWithin(const Object* obj) {
  if (no_seek)
    return 0;
  for (auto ind : contents) {
    if (ind == obj)
      return 1;
    if (ind->Skill(crc32c("Open")) || ind->Skill(crc32c("Transparent"))) {
      if (ind->SeeWithin(obj))
        return 1;
    }
  }
  return 0;
}

int Object::IsNearBy(const Object* obj) {
  if (no_seek || (!parent))
    return 0;
  for (auto ind : parent->contents) {
    if (ind == obj)
      return 1;
    if (ind == this)
      continue; // Not Nearby Self
    if (ind->Skill(crc32c("Open")) || ind->Skill(crc32c("Transparent"))) {
      int ret = ind->SeeWithin(obj);
      if (ret)
        return ret;
    }
  }
  if (parent->parent && (parent->Skill(crc32c("Open")) || parent->Skill(crc32c("Transparent")))) {
    parent->no_seek = true;
    int ret = parent->IsNearBy(obj);
    parent->no_seek = false;
    if (ret)
      return ret;
  }
  return 0;
}

void Object::NotifyLeft(Object* obj, Object* newloc) {
  if (this == obj)
    return;

  std::set<act_t> stops, stops2;
  int following = 0;
  for (auto curact : act) {
    if (curact.obj() && curact.act() < ACT_MAX &&
        (curact.obj() == obj || obj->HasWithin(curact.obj()))) {
      if (curact.act() != ACT_FOLLOW || (!newloc)) {
        stops.insert(curact.act());
      } else if (parent != newloc) { // Do nothing if we didn't leave!
        following = 1; // Run Follow Response AFTER loop!
      }
    }
    if (curact.act() >= ACT_MAX && (!newloc) && curact.obj() == obj) {
      for (auto curact2 : obj->act) {
        if (curact2.act() >= ACT_MAX) {
          if (curact2.obj() == this) {
            stops2.insert(curact2.act());
          }
        }
      }
      stops.insert(curact.act());
    }
  }

  if (following) {
    int stealth_t = 0, stealth_s = 0;
    if (IsUsing(crc32c("Stealth")) && Skill(crc32c("Stealth")) > 0) {
      stealth_t = Skill(crc32c("Stealth"));
      stealth_s = Roll(crc32c("Stealth"), 2);
    }
    parent->SendOut(stealth_t, stealth_s, ";s follows ;s.\n", "You follow ;s.\n", this, obj);
    Travel(newloc);
    parent->SendOut(stealth_t, stealth_s, ";s follows ;s.\n", "", this, obj);
    AddAct(ACT_FOLLOW, obj); // Needed since Travel Kills Follow Act
    if (stealth_t > 0) {
      SetSkill(crc32c("Hidden"), Roll(crc32c("Stealth"), 2) * 2);
    }
  }

  for (auto stop : stops) {
    StopAct(stop);
  }
  for (auto stop : stops2) {
    obj->StopAct(stop);
  }

  if (obj->ActTarg(ACT_HOLD) == this) { // Dragging
    if (parent != newloc) { // Actually went somewhere
      parent->SendOut(ALL, -1, ";s drags ;s along.\n", "You drag ;s along with you.\n", obj, this);
      Travel(newloc, 0);
      parent->SendOut(ALL, -1, ";s drags ;s along.\n", "", obj, this);
    }
  }
}

void Object::NotifyGone(Object* obj, Object* newloc, int up) {
  if (obj == this)
    return; // Never notify self or sub-self objects.

  // Climb to top first!
  if (up == 1 && parent && (Skill(crc32c("Open")) || Skill(crc32c("Transparent")))) {
    parent->NotifyGone(obj, newloc, 1);
    return;
  }

  NotifyLeft(obj, newloc);

  std::map<Object*, int> tonotify;

  for (auto ind : contents) {
    if (up >= 0) {
      tonotify[ind] = -1;
    } else if (ind->Skill(crc32c("Open")) || ind->Skill(crc32c("Transparent"))) {
      tonotify[ind] = 0;
    }
  }

  for (auto noti : tonotify) {
    noti.first->NotifyGone(obj, newloc, noti.second);
  }
}

bool Object::IsAct(act_t a) const {
  auto itr = act.begin();
  for (; itr != act.end() && itr->act() != a; ++itr) {
  }
  return (itr != act.end());
};

Object* Object::ActTarg(act_t a) const {
  auto itr = act.begin();
  for (; itr != act.end() && itr->act() != a; ++itr) {
  }
  if (itr != act.end()) {
    return itr->obj();
  } else {
    return nullptr;
  }
};

void Object::AddAct(act_t a, Object* o) {
  if (o) {
    o->NowTouching(this);
  }
  StopAct(a);
  act.push_back(act_pair(a, o));
}

void Object::StopAct(act_t a) {
  auto itr = act.begin();
  for (; itr != act.end() && itr->act() != a; ++itr) {
  }
  if (itr != act.end()) {
    Object* obj = itr->obj();
    act.erase(itr);
    if (a == ACT_HOLD && IsAct(ACT_OFFER)) {
      // obj->SendOut(0, 0, ";s stops offering.\n", "", obj, nullptr);
      StopAct(ACT_OFFER);
    }
    if (obj) {
      for (auto opt : act) {
        if (opt.obj() == obj) {
          return; // Still touching it, so done
        }
      }
      obj->NotTouching(this); // No longer touching it
    }
  }
}

void Object::StopAll() {
  auto oldact = act;
  for (auto opt : oldact) {
    StopAct(opt.act());
  }
}

void Object::Collapse() {
  StopAct(ACT_DEAD);
  StopAct(ACT_DYING);
  StopAct(ACT_UNCONSCIOUS);
  StopAct(ACT_SLEEP);
  StopAct(ACT_REST);
  StopAct(ACT_FOLLOW);
  StopAct(ACT_POINT);
  StopAct(ACT_FIGHT);
  if (parent) {
    if (pos != POS_LIE) {
      parent->SendOut(ALL, -1, ";s collapses!\n", "You collapse!\n", this, nullptr);
      pos = POS_LIE;
    }
    if (ActTarg(ACT_WIELD)) {
      parent->SendOutF(
          ALL,
          -1,
          ";s drops %s!\n",
          "You drop %s!\n",
          this,
          nullptr,
          ActTarg(ACT_WIELD)->ShortDesc().c_str());
      ActTarg(ACT_WIELD)->Travel(parent);
    }
    if (ActTarg(ACT_HOLD) && ActTarg(ACT_HOLD) != ActTarg(ACT_WEAR_SHIELD)) {
      parent->SendOutF(
          ALL,
          -1,
          ";s drops %s!\n",
          "You drop %s!\n",
          this,
          nullptr,
          ActTarg(ACT_HOLD)->ShortDesc().c_str());
      ActTarg(ACT_HOLD)->Travel(parent);
    } else if (ActTarg(ACT_HOLD)) {
      parent->SendOutF(
          ALL,
          -1,
          ";s stops holding %s.\n",
          "You stop holding %s!\n",
          this,
          nullptr,
          ActTarg(ACT_HOLD)->ShortDesc().c_str());
      StopAct(ACT_HOLD);
    }
  }
}

void Object::UpdateDamage() {
  if (stun > 10) { // Overflow
    phys += stun - 10;
    stun = 10;
  }
  if (stun < Skill(crc32c("Hungry")) / 5000) { // Hungry Stuns
    stun = Skill(crc32c("Hungry")) / 5000;
  }
  if (phys < Skill(crc32c("Thirsty")) / 5000) { // Thirsty Wounds
    phys = Skill(crc32c("Thirsty")) / 5000;
  }
  if (phys > 10 + ModAttribute(2)) {
    phys = 10 + ModAttribute(2) + 1;

    if (IsAct(ACT_DEAD) == 0) {
      parent->SendOut(
          ALL, -1, ";s expires from its wounds.\n", "You expire, sorry.\n", this, nullptr);
      stun = 10;
      Collapse();
      AddAct(ACT_DEAD);
      std::set<Mind*> removals;
      std::set<Mind*>::iterator mind;
      for (auto mnd : minds) {
        if (mnd->Type() == MIND_REMOTE)
          removals.insert(mnd);
      }
      for (auto mnd : removals) {
        Unattach(mnd);
      }
    }
    SetPos(POS_LIE);
  } else if (phys >= 10) {
    if (IsAct(ACT_DYING) + IsAct(ACT_DEAD) == 0) {
      parent->SendOut(
          ALL,
          -1,
          ";s collapses, bleeding and dying!\n",
          "You collapse, bleeding and dying!\n",
          this,
          nullptr);
      stun = 10;
      Collapse();
      AddAct(ACT_DYING);
    } else if (IsAct(ACT_DEAD) == 0) {
      parent->SendOut(
          ALL, -1, ";s isn't quite dead yet!\n", "You aren't quite dead yet!\n", this, nullptr);
      StopAct(ACT_DEAD);
      AddAct(ACT_DYING);
    }
    SetPos(POS_LIE);
  } else if (stun >= 10) {
    if (IsAct(ACT_UNCONSCIOUS) + IsAct(ACT_DYING) + IsAct(ACT_DEAD) == 0) {
      parent->SendOut(ALL, -1, ";s falls unconscious!\n", "You fall unconscious!\n", this, nullptr);
      Collapse();
      AddAct(ACT_UNCONSCIOUS);
    } else if (IsAct(ACT_DEAD) + IsAct(ACT_DYING) != 0) {
      parent->SendOut(
          ALL,
          -1,
          ";s isn't dead, just out cold.\n",
          "You aren't dead, just unconscious.",
          this,
          nullptr);
      StopAct(ACT_DEAD);
      StopAct(ACT_DYING);
      AddAct(ACT_UNCONSCIOUS);
    }
    SetPos(POS_LIE);
  } else if (stun > 0) {
    if (IsAct(ACT_DEAD) + IsAct(ACT_DYING) + IsAct(ACT_UNCONSCIOUS) != 0) {
      parent->SendOut(
          ALL,
          -1,
          ";s wakes up, a little groggy.\n",
          "You wake up, a little groggy.",
          this,
          nullptr);
      StopAct(ACT_DEAD);
      StopAct(ACT_DYING);
      StopAct(ACT_UNCONSCIOUS);
    }
  } else {
    if (IsAct(ACT_DEAD) + IsAct(ACT_DYING) + IsAct(ACT_UNCONSCIOUS) != 0) {
      parent->SendOut(
          ALL, -1, ";s wakes up, feeling fine!\n", "You wake up, feeling fine!\n", this, nullptr);
      StopAct(ACT_DEAD);
      StopAct(ACT_DYING);
      StopAct(ACT_UNCONSCIOUS);
    }
  }
}

int Object::HealStun(int boxes) {
  if (!IsAnimate())
    return 0;
  if (phys >= 10)
    return 0;
  if (boxes > stun)
    boxes = stun;
  stun -= boxes;
  UpdateDamage();
  return boxes;
}

int Object::HealPhys(int boxes) {
  if (!IsAnimate())
    return 0;
  if (boxes > phys)
    boxes = phys;
  phys -= boxes;
  UpdateDamage();
  return boxes;
}

int Object::HealStru(int boxes) {
  if (boxes > stru)
    boxes = stru;
  stru -= boxes;
  UpdateDamage();
  return boxes;
}

int Object::HitMent(int force, int sev, int succ) {
  if (!IsAnimate())
    return 0;
  succ -= roll(ModAttribute(0) + Modifier("Resilience"), force);
  sev *= 2;
  sev += succ;
  for (int ctr = 0; ctr <= (sev / 2) && ctr <= 4; ++ctr)
    stun += ctr;
  if (sev > 8)
    stun += (sev - 8);
  if (stun > 10)
    stun = 10; // No Overflow to Phys from "Ment"
  UpdateDamage();
  return sev;
}

int Object::HitStun(int force, int sev, int succ) {
  if (!IsAnimate())
    return 0;
  succ -= roll(ModAttribute(0) + Modifier("Resilience"), force);
  sev *= 2;
  sev += succ;
  for (int ctr = 0; ctr <= (sev / 2) && ctr <= 4; ++ctr)
    stun += ctr;
  if (sev > 8)
    stun += (sev - 8);
  UpdateDamage();
  return sev;
}

int Object::HitPhys(int force, int sev, int succ) {
  if (!IsAnimate())
    return 0;
  succ -= roll(ModAttribute(0) + Modifier("Resilience"), force);
  sev *= 2;
  sev += succ;
  for (int ctr = 0; ctr <= (sev / 2) && ctr <= 4; ++ctr)
    phys += ctr;
  if (sev > 8)
    phys += (sev - 8);
  UpdateDamage();
  return sev;
}

int Object::HitStru(int force, int sev, int succ) {
  succ -= roll(ModAttribute(0) + Modifier("Resilience"), force);
  sev *= 2;
  sev += succ;
  for (int ctr = 0; ctr <= (sev / 2) && ctr <= 4; ++ctr)
    stru += ctr;
  if (sev > 8)
    stru += (sev - 8);
  UpdateDamage();
  return sev;
}

void Object::Send(int tnum, int rsucc, const std::string& mes) {
  if (no_hear)
    return;
  auto tosend = mes;
  tosend[0] = ascii_toupper(tosend[0]);

  for (auto mind : minds) {
    Object* body = mind->Body();
    mind->Attach(this);
    mind->Send(tosend);
    mind->Attach(body);
  }
}

void Object::SendF(int tnum, int rsucc, const char* mes, ...) {
  if (mes[0] == 0)
    return;

  if (tnum != ALL && rsucc >= 0 && Roll(crc32c("Perception"), tnum) <= rsucc) {
    return;
  }

  char buf2[65536];
  va_list stuff;
  va_start(stuff, mes);
  vsprintf(buf2, mes, stuff);
  va_end(stuff);

  Send(tnum, rsucc, buf2);
}

void Object::Send(channel_t channel, const std::string& mes) {
  auto tosend = mes;
  tosend[0] = ascii_toupper(tosend[0]);

  for (auto mind : minds) {
    if (channel == CHANNEL_ROLLS && mind->IsSVar("combatinfo")) {
      Object* body = mind->Body();
      mind->Attach(this);
      mind->Send(tosend);
      mind->Attach(body);
    }
  }
}

void Object::SendF(channel_t channel, const char* mes, ...) {
  if (mes[0] == 0)
    return;

  char buf2[65536];
  va_list stuff;
  va_start(stuff, mes);
  vsprintf(buf2, mes, stuff);
  va_end(stuff);

  Send(channel, buf2);
}

void Object::SendIn(
    int tnum,
    int rsucc,
    const std::string& mes,
    const std::string& youmes,
    Object* actor,
    Object* targ) {
  if (no_seek)
    return;

  if (this != actor) { // Don't trigger yourself!
    for (auto trig : contents) {
      if (strncmp(mes.c_str(), ";s says '", 9)) { // Type 0x1000010 (MOB + MOB-ACT)
        if ((trig->Skill(crc32c("TBAScriptType")) & 0x1000010) == 0x1000010) {
          if (trig->Desc()[0] == '*') { // All Actions
            new_trigger((rand() % 400) + 300, trig, actor, mes);
          } else if (trig->Skill(crc32c("TBAScriptNArg")) == 0) { // Match Full Phrase
            if (phrase_match(mes, trig->Desc())) {
              new_trigger((rand() % 400) + 300, trig, actor, mes);
            }
          } else { // Match Words
            if (words_match(mes, trig->Desc())) {
              new_trigger((rand() % 400) + 300, trig, actor, mes);
            }
          }
        }
      } else { // Type 0x1000008 (MOB + MOB-SPEECH)
        if ((trig->Skill(crc32c("TBAScriptType")) & 0x1000008) == 0x1000008) {
          // if (trig->Skill(crc32c("TBAScript")) >= 5034503 && trig->Skill(crc32c("TBAScript")) <=
          // 5034507)
          //  fprintf(stderr, CBLU "[#%d] Got message: '%s'\n" CNRM,
          //  trig->Skill(crc32c("TBAScript")), mes);
          std::string speech = (mes.c_str() + 9);
          while (!speech.empty() && speech.back() != '\'') {
            speech.pop_back();
          }
          if (!speech.empty())
            speech.pop_back();

          if (trig->Desc()[0] == '*') { // All Speech
            new_trigger((rand() % 400) + 300, trig, actor, speech);
          } else if (trig->Skill(crc32c("TBAScriptNArg")) == 0) { // Match Full Phrase
            if (phrase_match(speech, trig->Desc())) {
              // if (trig->Skill(crc32c("TBAScript")) >= 5034503 && trig->Skill(crc32c("TBAScript"))
              // <= 5034507)
              //  fprintf(stderr, CBLU "Triggering(f): %s\n" CNRM, trig->Name().c_str());
              new_trigger((rand() % 400) + 300, trig, actor, speech);
            }
          } else { // Match Words
            if (words_match(speech, trig->Desc())) {
              // if (trig->Skill(crc32c("TBAScript")) >= 5034503 && trig->Skill(crc32c("TBAScript"))
              // <= 5034507)
              //  fprintf(stderr, CBLU "Triggering(w): %s\n" CNRM, trig->Name().c_str());
              new_trigger((rand() % 400) + 300, trig, actor, speech);
            }
          }
        }
      }
    }
  }

  std::string tstr = "";
  if (targ)
    tstr = targ->Name(0, this, actor);
  std::string astr = "";
  if (actor)
    astr = actor->Name(0, this);

  auto str = mes;
  auto youstr = youmes;

  for (auto& ctr : str) {
    if (ctr == ';')
      ctr = '%';
    else if (ctr == '%')
      ctr = ';';
  }
  for (auto& ctr : youstr) {
    if (ctr == ';')
      ctr = '%';
    else if (ctr == '%')
      ctr = ';';
  }

  if (youstr[0] == '*' && this == actor) {
    Send(ALL, -1, CYEL);
    if (targ)
      SendF(ALL, -1, youstr.c_str() + 1, tstr.c_str());
    else
      Send(ALL, -1, youstr.c_str() + 1);
    Send(ALL, -1, CNRM);
  } else if (this == actor) {
    if (targ)
      SendF(ALL, -1, youstr.c_str(), tstr.c_str());
    else
      Send(ALL, -1, youstr);
  } else if (str[0] == '*' && targ == this) {
    Send(ALL, -1, CRED);
    if (targ || actor)
      SendF(tnum, rsucc, str.c_str() + 1, astr.c_str(), tstr.c_str());
    else
      Send(tnum, rsucc, str.c_str() + 1);
    Send(ALL, -1, CNRM);
  } else if (str[0] == '*') {
    Send(ALL, -1, CMAG);
    if (targ || actor)
      SendF(tnum, rsucc, str.c_str() + 1, astr.c_str(), tstr.c_str());
    else
      Send(tnum, rsucc, str.c_str() + 1);
    Send(ALL, -1, CNRM);
  } else {
    if (targ || actor)
      SendF(tnum, rsucc, str.c_str(), astr.c_str(), tstr.c_str());
    else
      Send(tnum, rsucc, str);
  }

  for (auto ind : contents) {
    if (ind->Skill(crc32c("Open")) || ind->Skill(crc32c("Transparent")))
      ind->SendIn(tnum, rsucc, mes, youmes, actor, targ);
    else if (ind->Pos() != POS_NONE) // FIXME - Understand Transparency
      ind->SendIn(tnum, rsucc, mes, youmes, actor, targ);
  }
}

void Object::SendInF(
    int tnum,
    int rsucc,
    const char* mes,
    const char* youmes,
    Object* actor,
    Object* targ,
    ...) {
  if (no_seek)
    return;

  char buf2[65536];
  char youbuf[65536];
  va_list stuff;
  va_start(stuff, targ);
  vsprintf(buf2, mes, stuff);
  va_end(stuff);
  va_start(stuff, targ);
  vsprintf(youbuf, youmes, stuff);
  va_end(stuff);

  SendIn(tnum, rsucc, buf2, youbuf, actor, targ);
}

void Object::SendOut(
    int tnum,
    int rsucc,
    const std::string& mes,
    const std::string& youmes,
    Object* actor,
    Object* targ) {
  if (no_seek)
    return;

  if (!strncmp(mes.c_str(), ";s says '", 9)) { // Type 0x4000008 (ROOM + ROOM-SPEECH)
    for (auto trig : contents) {
      if ((trig->Skill(crc32c("TBAScriptType")) & 0x4000008) == 0x4000008) {
        std::string speech = (mes.c_str() + 9);
        while (!speech.empty() && speech.back() != '\'') {
          speech.pop_back();
        }
        if (!speech.empty())
          speech.pop_back();

        if (trig->Desc()[0] == '*') { // All Speech
          new_trigger((rand() % 400) + 300, trig, actor, speech);
        } else if (trig->Skill(crc32c("TBAScriptNArg")) == 0) { // Match Full Phrase
          if (phrase_match(speech, trig->Desc())) {
            new_trigger((rand() % 400) + 300, trig, actor, speech);
          }
        } else { // Match Words
          if (words_match(speech, trig->Desc())) {
            new_trigger((rand() % 400) + 300, trig, actor, speech);
          }
        }
      }
    }
  }

  std::string tstr = "";
  if (targ)
    tstr = targ->Name(0, this, actor);
  std::string astr = "";
  if (actor)
    astr = actor->Name(0, this);

  auto str = mes;
  auto youstr = youmes;

  for (auto& ctr : str) {
    if (ctr == ';')
      ctr = '%';
    else if (ctr == '%')
      ctr = ';';
  }
  for (auto& ctr : youstr) {
    if (ctr == ';')
      ctr = '%';
    else if (ctr == '%')
      ctr = ';';
  }

  if (youstr[0] == '*' && this == actor) {
    Send(ALL, -1, CGRN);
    if (targ)
      SendF(ALL, -1, youstr.c_str() + 1, tstr.c_str());
    else
      Send(ALL, -1, youstr.c_str() + 1);
    Send(ALL, -1, CNRM);
  } else if (this == actor) {
    if (targ)
      SendF(ALL, -1, youstr.c_str(), tstr.c_str());
    else
      Send(ALL, -1, youstr);
  } else if (str[0] == '*') {
    Send(ALL, -1, CRED);
    if (targ || actor)
      SendF(tnum, rsucc, str.c_str() + 1, astr.c_str(), tstr.c_str());
    else
      Send(tnum, rsucc, str.c_str() + 1);
    Send(ALL, -1, CNRM);
  } else {
    if (targ || actor)
      SendF(tnum, rsucc, str.c_str(), astr.c_str(), tstr.c_str());
    else
      Send(tnum, rsucc, str);
  }

  for (auto ind : contents) {
    if (ind->Skill(crc32c("Open")) || ind->Skill(crc32c("Transparent")))
      ind->SendIn(tnum, rsucc, mes, youmes, actor, targ);
    else if (ind->Pos() != POS_NONE) // FIXME - Understand Transparency
      ind->SendIn(tnum, rsucc, mes, youmes, actor, targ);
  }

  if (parent && (Skill(crc32c("Open")) || Skill(crc32c("Transparent")))) {
    no_seek = true;
    parent->SendOut(tnum, rsucc, mes, youmes, actor, targ);
    no_seek = false;
  }
}

void Object::SendOutF(
    int tnum,
    int rsucc,
    const char* mes,
    const char* youmes,
    Object* actor,
    Object* targ,
    ...) {
  if (no_seek)
    return;

  char buf2[65536];
  char youbuf[65536];
  va_list stuff;
  va_start(stuff, targ);
  vsprintf(buf2, mes, stuff);
  va_end(stuff);
  va_start(stuff, targ);
  vsprintf(youbuf, youmes, stuff);
  va_end(stuff);

  SendOut(tnum, rsucc, buf2, youbuf, actor, targ);
}

void Object::Loud(int str, const std::string& mes) {
  std::set<Object*> visited;
  Loud(visited, str, mes);
}

void Object::LoudF(int str, const char* mes, ...) {
  if (mes[0] == 0)
    return;

  char buf2[65536];
  va_list stuff;
  va_start(stuff, mes);
  vsprintf(buf2, mes, stuff);
  va_end(stuff);

  std::set<Object*> visited;
  Loud(visited, str, buf2);
}

void Object::Loud(std::set<Object*>& visited, int str, const std::string& mes) {
  visited.insert(this);
  auto targs = PickObjects("all", LOC_INTERNAL);
  for (auto dest : targs) {
    if (dest->HasSkill(crc32c("Enterable"))) {
      int ostr = str;
      --str;
      if (dest->Skill(crc32c("Open")) < 1) {
        --str;
      }
      if (str >= 0) {
        if (dest->ActTarg(ACT_SPECIAL_LINKED) && dest->ActTarg(ACT_SPECIAL_LINKED)->Parent()) {
          dest = dest->ActTarg(ACT_SPECIAL_LINKED);
          if (visited.count(dest->Parent()) < 1) {
            dest->Parent()->SendOutF(ALL, 0, "From ;s you hear %s\n", "", dest, dest, mes.c_str());
            dest->Parent()->Loud(visited, str, mes.c_str());
          }
        }
      }
      str = ostr;
    }
  }
}

void init_world() {
  universe = new Object;
  universe->SetShortDesc("The Universe");
  universe->SetDesc("An Infinite Universe within which to play.");
  universe->SetLongDesc("A Really Big Infinite Universe within which to play.");
  trash_bin = new Object;
  trash_bin->SetShortDesc("The Trash Bin");
  trash_bin->SetDesc("The place objects come to die.");

  if (!universe->Load("acid/current.wld")) {
    load_players("acid/current.plr");
  } else {
    is_skill(0); // Force runtime string initialization

    int fd = open("acid/startup.conf", O_RDONLY);
    if (fd >= 0) {
      Object* autoninja = new Object(universe);
      autoninja->SetShortDesc("The AutoNinja");
      autoninja->SetDesc("The AutoNinja - you should NEVER see this!");
      autoninja->SetPos(POS_STAND);

      Player* anp = new Player("AutoNinja", "AutoNinja");
      anp->Set(PLAYER_SUPERNINJA);
      anp->Set(PLAYER_NINJA);
      anp->Set(PLAYER_NINJAMODE);

      Mind* automind = new Mind();
      automind->SetPlayer("AutoNinja");
      automind->SetSystem();
      automind->Attach(autoninja);

      int len = lseek(fd, 0, SEEK_END);
      lseek(fd, 0, SEEK_SET);
      char buf2[len + 1];
      read(fd, buf2, len);
      buf2[len] = 0;

      handle_command(autoninja, buf2, automind);
      close(fd);

      delete automind;
      delete anp;
      autoninja->Recycle();
    }
  }
  universe->SetSkill(crc32c("Light Source"), 1000); // Ninjas need to see too.
}

void save_world(int with_net) {
  std::string fn = "acid/current";
  std::string wfn = fn + ".wld.tmp";
  if (!universe->Save(wfn.c_str())) {
    std::string pfn = fn + ".plr.tmp";
    if (!save_players(pfn.c_str())) {
      std::string nfn = fn + ".nst";
      if ((!with_net) || (!save_net(nfn.c_str()))) {
        std::string dfn = fn + ".wld";
        unlink(dfn.c_str());
        rename(wfn.c_str(), dfn.c_str());

        dfn = fn + ".plr";
        unlink(dfn.c_str());
        rename(pfn.c_str(), dfn.c_str());
      } else {
        fprintf(stderr, "Unable to save network status!\n");
        perror("save_world");
      }
    } else {
      fprintf(stderr, "Unable to save players!\n");
      perror("save_world");
    }
  } else {
    fprintf(stderr, "Unable to save world!\n");
    perror("save_world");
  }
}

int Object::WriteContentsTo(FILE* fl) {
  for (auto cind : contents) {
    fprintf(fl, ":%d", getnum(cind));
  }
  return 0;
}

void Object::BusyFor(long msec, const std::string& default_next) {
  // fprintf(stderr, "Holding %p, will default do '%s'!\n", this, default_next);
  busy_until = current_time;
  busy_until += (msec * 1000);
  defact = default_next;
  busylist.insert(this);
}

void Object::BusyWith(Object* other, const std::string& default_next) {
  // fprintf(stderr, "Holding %p, will default do '%s'!\n", this, default_next);
  busy_until = other->busy_until;
  defact = default_next;
  busylist.insert(this);
}

int Object::StillBusy() {
  return (current_time <= busy_until);
}

void Object::DoWhenFree(const std::string& action) {
  // fprintf(stderr, "Adding busyact for %p of '%s'!\n", this, action);
  dowhenfree += ";";
  dowhenfree += action;
  busylist.insert(this);
}

int Object::BusyAct() {
  // fprintf(stderr, "Taking busyact %p!\n", this);
  busylist.erase(this);

  std::string comm = dowhenfree;
  std::string def = defact;
  dowhenfree = "";
  defact = "";

  // fprintf(stderr, "Act is %s [%s]!\n", comm.c_str(), def.c_str());

  int ret;
  if (minds.size()) {
    ret = handle_command(this, comm.c_str(), (*(minds.begin())));
    if (ret != 2 && (!StillBusy()))
      ret = handle_command(this, def.c_str(), (*(minds.begin())));
  } else {
    ret = handle_command(this, comm.c_str());
    if (ret != 2 && (!StillBusy()))
      ret = handle_command(this, def.c_str());
  }
  return (ret == 2); // Return 1 if no more actions this round!
}

void Object::FreeActions() {
  int maxinit = 0;
  std::map<Object*, int> initlist;
  for (auto busy : busylist) {
    if (!busy->StillBusy()) {
      initlist[busy] = busy->RollInitiative();
      if (maxinit < initlist[busy]) {
        maxinit = initlist[busy];
      }
    }
  }
  for (int phase = maxinit; phase > 0; --phase) {
    for (auto init : initlist) {
      // Make sure it's still in busylist
      // (hasn't been deleted by another's BusyAct)!
      if (init.second == phase && busylist.count(init.first)) {
        //	if(init.first->IsAct(ACT_FIGHT))
        //	  fprintf(stderr, "Going at %d (%s)\n", phase,
        // init.first->Name().c_str());
        init.first->BusyAct();
      }
    }
  }
  for (auto init : initlist) {
    if (init.first->IsAct(ACT_FIGHT)) { // Still in combat!
      if (!init.first->StillBusy()) { // Make Sure!
        std::string ret = init.first->Tactics();
        init.first->BusyFor(3000, ret.c_str());
      }

      // Type 0x1000400 (MOB + MOB-FIGHT)
      for (auto trig : init.first->contents) {
        if ((trig->Skill(crc32c("TBAScriptType")) & 0x1000400) == 0x1000400) {
          if ((rand() % 100) < trig->Skill(crc32c("TBAScriptNArg"))) { // % Chance
            new_trigger(0, trig, init.first->ActTarg(ACT_FIGHT));
          }
        }
      }
    }
  }
}

std::string Object::Tactics(int phase) {
  if (minds.size() < 1)
    return "attack";
  Mind* mind = (*(minds.begin())); // FIXME: Handle Multiple Minds
  Object* body = mind->Body();
  mind->Attach(this);
  std::string ret = mind->Tactics();
  mind->Attach(body);
  return ret;
}

bool Object::IsSameAs(const Object& in) const {
  if (short_desc != in.short_desc)
    return 0;
  if (desc != in.desc)
    return 0;
  if (long_desc != in.long_desc)
    return 0;
  if (weight != in.weight)
    return 0;
  if (volume != in.volume)
    return 0;
  if (size != in.size)
    return 0;
  if (value != in.value)
    return 0;
  if (gender != in.gender)
    return 0;

  if (att[0].base != in.att[0].base)
    return 0;
  if (att[0].cur != in.att[0].cur)
    return 0;
  if (att[0].mod != in.att[0].mod)
    return 0;
  if (att[1].base != in.att[1].base)
    return 0;
  if (att[1].cur != in.att[1].cur)
    return 0;
  if (att[1].mod != in.att[1].mod)
    return 0;
  if (att[2].base != in.att[2].base)
    return 0;
  if (att[2].cur != in.att[2].cur)
    return 0;
  if (att[2].mod != in.att[2].mod)
    return 0;
  if (att[3].base != in.att[3].base)
    return 0;
  if (att[3].cur != in.att[3].cur)
    return 0;
  if (att[3].mod != in.att[3].mod)
    return 0;
  if (att[4].base != in.att[4].base)
    return 0;
  if (att[4].cur != in.att[4].cur)
    return 0;
  if (att[4].mod != in.att[4].mod)
    return 0;
  if (att[5].base != in.att[5].base)
    return 0;
  if (att[5].cur != in.att[5].cur)
    return 0;
  if (att[5].mod != in.att[5].mod)
    return 0;

  if (phys != in.phys)
    return 0;
  if (stun != in.stun)
    return 0;
  if (stru != in.stru)
    return 0;

  if (pos != in.pos)
    return 0;

  if (minds != in.minds)
    return 0; // Only no-mind or same-group-minds

  if (act.size() != 0 || in.act.size() != 0)
    return 0;

  if (contents.size() != 0 || in.contents.size() != 0)
    return 0;

  auto sk1 = skills;
  auto sk2 = in.skills;
  for (auto sk = sk1.begin(); sk != sk1.end();) {
    if (sk->first == crc32c("Quantity") || sk->first == crc32c("Hungry") ||
        sk->first == crc32c("Bored") || sk->first == crc32c("Needy") ||
        sk->first == crc32c("Tired")) {
      sk1.erase(sk);
    } else {
      ++sk;
    }
  }
  for (auto sk = sk2.begin(); sk != sk2.end();) {
    if (sk->first == crc32c("Quantity") || sk->first == crc32c("Hungry") ||
        sk->first == crc32c("Bored") || sk->first == crc32c("Needy") ||
        sk->first == crc32c("Tired")) {
      sk2.erase(sk);
    } else {
      ++sk;
    }
  }
  std::sort(sk1.begin(), sk1.end());
  std::sort(sk2.begin(), sk2.end());
  if (sk1 != sk2)
    return 0;

  return 1;
}

void Object::operator=(const Object& in) {
  short_desc = in.short_desc;
  desc = in.desc;
  long_desc = in.long_desc;
  weight = in.weight;
  volume = in.volume;
  size = in.size;
  value = in.value;
  gender = in.gender;

  att[0] = in.att[0];
  att[1] = in.att[1];
  att[2] = in.att[2];
  att[3] = in.att[3];
  att[4] = in.att[4];
  att[5] = in.att[5];

  skills = in.skills;

  pos = in.pos;

  if (in.IsActive())
    Activate();
  else
    Deactivate();

  //  contents = in.contents;	// NOT doing this for deep/shallow reasons.
  //  act = in.act;
}

MinVec<1, Object*> Object::Contents(int vmode) {
  MinVec<1, Object*> ret;
  if (vmode & LOC_NINJA) {
    ret = contents;
  } else {
    for (auto item : contents) {
      if (item->Skill(crc32c("Invisible")) >= 1000)
        continue; // Not Really There
      if ((vmode & (LOC_HEAT | LOC_TOUCH)) == 0 && item->Skill(crc32c("Invisible"))) {
        continue;
      }
      if ((vmode & LOC_FIXED) && item->Pos() != POS_NONE)
        continue;
      if ((vmode & LOC_NOTFIXED) && item->Pos() == POS_NONE)
        continue;
      ret.push_back(item);
    }
  }
  return ret;
}

MinVec<1, Object*> Object::Contents() {
  return contents;
}

int Object::Contains(const Object* obj) {
  return (std::find(contents.begin(), contents.end(), obj) != contents.end());
}

void Object::SpendExp(int e) {
  sexp += e;
}

bool Object::HasAccomplished(uint64_t acc) {
  for (auto comp : completed) {
    if (comp == acc) {
      return true;
    }
  }
  return false;
}

bool Object::Accomplish(uint64_t acc, const std::string& why) {
  if (HasAccomplished(acc)) {
    return false;
  }
  completed.push_back(acc);
  for (auto m : minds) {
    if (m->Owner()) {
      m->SendF(CYEL "You gain an experience point for %s!\n" CNRM, why.c_str());
    }
  }
  return true;
}

int two_handed(int wtype) {
  static std::set<int> thsks;
  if (thsks.size() == 0) {
    thsks.insert(get_weapon_type("Two-Handed Blades"));
    thsks.insert(get_weapon_type("Two-Handed Cleaves"));
    thsks.insert(get_weapon_type("Two-Handed Crushing"));
    thsks.insert(get_weapon_type("Two-Handed Flails"));
    thsks.insert(get_weapon_type("Two-Handed Piercing"));
    thsks.insert(get_weapon_type("Two-Handed Staves"));
  }
  return int(thsks.count(wtype));
}

std::string Object::PosString() {
  std::string ret;
  if (pos == POS_USE) {
    ret = fmt::format("is {} here", UsingString());
  } else {
    ret = pos_str[pos];
  }
  return ret;
}

std::string Object::UsingString() {
  std::string ret;
  if (pos == POS_USE) {
    if (cur_skill == crc32c("Stealth")) {
      ret = "sneaking around";
    } else if (cur_skill == crc32c("Perception")) {
      ret = "keeping an eye out";
    } else if (cur_skill == crc32c("Healing")) {
      ret = "caring for others' wounds";
    } else if (cur_skill == crc32c("First Aid")) {
      ret = "giving first-aid";
    } else if (cur_skill == crc32c("Treatment")) {
      ret = "treating others' wounds";
    } else if (cur_skill == crc32c("Lumberjack")) {
      ret = "chopping down trees";
    } else if (cur_skill == crc32c("Sprinting")) {
      ret = "running as fast as possible";
    } else {
      ret = fmt::format("using the {} skill", SkillName(Using()));
    }
  } else {
    ret = "doing nothing";
  }
  return ret;
}

void Object::StartUsing(uint32_t skill) {
  cur_skill = skill;
  pos = POS_USE;
}

void Object::StopUsing() {
  if (pos == POS_USE)
    pos = POS_STAND;
  cur_skill = crc32c("None");
}

uint32_t Object::Using() {
  return cur_skill;
}

int Object::IsUsing(uint32_t skill) {
  return (skill == cur_skill);
}

pos_t Object::Pos() {
  return pos;
}

void Object::SetPos(pos_t p) {
  if (pos == POS_USE && p != POS_USE)
    StopUsing();
  pos = p;
}

int Object::Filter(int loc) {
  if (loc & (LOC_ALIVE | LOC_CONSCIOUS)) {
    if (!IsAnimate() || IsAct(ACT_DEAD))
      return 0;
  }
  if (loc & LOC_CONSCIOUS) {
    if (IsAct(ACT_DYING) || IsAct(ACT_UNCONSCIOUS))
      return 0;
  }
  return 1;
}

int Object::LooksLike(Object* other, int vmode) {
  if (Name() != other->Name())
    return 0;
  if (Pos() != other->Pos())
    return 0;
  if (Using() != other->Using())
    return 0;

  // Neither open/trans/seen inside (if either contain anything)
  if (Contents(vmode).size() > 0 || other->Contents(vmode).size() > 0) {
    if (Skill(crc32c("Open")) || Skill(crc32c("Transparent")))
      return 0;
    if (other->Skill(crc32c("Open")) || other->Skill(crc32c("Transparent")))
      return 0;
    if (Skill(crc32c("Container")) || Skill(crc32c("Liquid Container"))) {
      if (vmode && (!Skill(crc32c("Locked"))))
        return 0;
    }
    if (other->Skill(crc32c("Container")) || other->Skill(crc32c("Liquid Container"))) {
      if (vmode && (!other->Skill(crc32c("Locked"))))
        return 0;
    }
  }

  for (act_t actt = ACT_NONE; actt < ACT_WIELD;) { // Off-By-One!
    actt = act_t(int(actt) + 1); // Increments First!
    if (IsAct(actt) != other->IsAct(actt))
      return 0;
    if (ActTarg(actt) != other->ActTarg(actt)) {
      std::string s1 = "";
      if (ActTarg(actt))
        s1 = ActTarg(actt)->Name(0, this);
      std::string s2 = "";
      if (ActTarg(actt))
        s2 = other->ActTarg(actt)->Name(0, other);
      if (s1 != s2)
        return 0;
    }
  }
  return 1;
}

void Object::Consume(const Object* item) {
  // Standard food/drink effects
  int hung = Skill(crc32c("Hungry"));
  SetSkill(crc32c("Hungry"), Skill(crc32c("Hungry")) - item->Skill(crc32c("Food")));
  int thir = Skill(crc32c("Thirsty"));
  SetSkill(crc32c("Thirsty"), Skill(crc32c("Thirsty")) - item->Skill(crc32c("Drink")));
  SetSkill(crc32c("Thirsty"), Skill(crc32c("Thirsty")) + item->Skill(crc32c("Dehydrate Effect")));
  // Heal back dehydrate/hunger wounds
  if ((hung / 5000) > (Skill(crc32c("Hungry")) / 5000)) {
    HealStun((hung / 5000) - (Skill(crc32c("Hungry")) / 5000));
  }
  if ((thir / 5000) > (Skill(crc32c("Thirsty")) / 5000)) {
    HealPhys((thir / 5000) - (Skill(crc32c("Thirsty")) / 5000));
  }

  // Special effect: Poisonous
  if (item->Skill(crc32c("Poisonous")) > 0) {
    SetSkill(crc32c("Poisoned"), Skill(crc32c("Poisoned")) + item->Skill(crc32c("Poisonous")));
  }

  // Special effect: Cure Poison
  if (item->Skill(crc32c("Cure Poison Spell")) > 0 && Skill(crc32c("Poisoned")) > 0) {
    if (item->Skill(crc32c("Cure Poison Spell")) >= Skill(crc32c("Poisoned"))) {
      SetSkill(crc32c("Poisoned"), 0);
      Send(ALL, 0, "You feel better.\n");
    }
  }

  // Special effect: Heal
  if (item->Skill(crc32c("Heal Spell")) > 0) {
    int succ = Roll(crc32c("Strength"), 12 - item->Skill(crc32c("Heal Spell")));
    if (phys > 0 && succ > 0)
      Send(ALL, 0, "You feel a bit less pain.\n");
    phys -= succ;
    if (phys < 0)
      phys = 0;
    UpdateDamage();
  }

  // Special effect: Energize
  if (item->Skill(crc32c("Energize Spell")) > 0) {
    int succ = Roll(crc32c("Strength"), 12 - item->Skill(crc32c("Energize Spell")));
    if (stun > 0 && succ > 0)
      Send(ALL, 0, "You feel a bit more rested.\n");
    stun -= succ;
    if (stun < 0)
      stun = 0;
    UpdateDamage();
  }

  // Special effect: Remove Curse - Note: Can't remove curse from cursed items
  if (item->Skill(crc32c("Remove Curse Spell")) > 0 && (!HasSkill(crc32c("Cursed")))) {
    Object* cursed = NextHasSkill(crc32c("Cursed"));
    while (cursed) {
      if (cursed->Skill(crc32c("Cursed")) <= item->Skill(crc32c("Remove Curse Spell"))) {
        Drop(cursed, 1, 1);
        cursed = NextHasSkill(crc32c("Cursed"));
      } else {
        cursed = NextHasSkill(crc32c("Cursed"), cursed);
      }
    }
  }

  // Special effect: Recall
  if (item->Skill(crc32c("Recall Spell")) > 0) {
    if (parent) {
      parent->SendOut(
          0, 0, "BAMF! ;s teleports away.\n", "BAMF! You teleport home.\n", this, nullptr);
    }
    Object* dest = this;
    while ((!dest->ActTarg(ACT_SPECIAL_HOME)) && dest->Parent()) {
      dest = dest->Parent();
    }
    if (dest->ActTarg(ACT_SPECIAL_HOME)) {
      dest = dest->ActTarg(ACT_SPECIAL_HOME);
    }
    Travel(dest, 0);
    if (parent) {
      parent->SendOut(0, 0, "BAMF! ;s teleports here.\n", "", this, nullptr);
      parent->SendDescSurround(this, this);
    }
  }

  // Special effect: Heat Vision Spell (Grants Ability)

  if (item->Skill(crc32c("Heat Vision Spell"))) {
    int force = item->Skill(crc32c("Heat Vision Spell"));
    Object* spell = new Object(this);
    spell->SetSkill(crc32c("Heat Vision"), std::min(100, force));
    spell->SetShortDesc("a spell");
    spell->SetSkill(crc32c("Magical"), force);
    spell->SetSkill(crc32c("Magical Spell"), force);
    spell->SetSkill(crc32c("Temporary"), force);
    spell->SetSkill(crc32c("Invisible"), 1000);
    spell->Activate();
    Send(ALL, 0, "You can now see better!\n");
  }

  // Special effect: Dark Vision Spell (Grants Ability)
  if (item->Skill(crc32c("Dark Vision Spell"))) {
    int force = item->Skill(crc32c("Dark Vision Spell"));
    Object* spell = new Object(this);
    spell->SetSkill(crc32c("Dark Vision"), std::min(100, force));
    spell->SetShortDesc("a spell");
    spell->SetSkill(crc32c("Magical"), force);
    spell->SetSkill(crc32c("Magical Spell"), force);
    spell->SetSkill(crc32c("Temporary"), force);
    spell->SetSkill(crc32c("Invisible"), 1000);
    spell->Activate();
    Send(ALL, 0, "You can now see better!\n");
  }

  // Special effect: Teleport Spell (Grants Ability)
  if (item->Skill(crc32c("Teleport Spell"))) {
    SetSkill(crc32c("Teleport"), 1); // Can use once!
    Send(ALL, 0, "You can now teleport once!\n");
  }

  // Special effect: Resurrect Spell (Grants Ability)
  if (item->Skill(crc32c("Resurrect Spell"))) {
    SetSkill(crc32c("Resurrect"), 1); // Can use once!
    Send(ALL, 0, "You can now resurrect a character once!\n");
  }

  // Special effect: Sleep Other
  if (item->Skill(crc32c("Sleep Other Spell")) > 0) {
    int succ = Roll(crc32c("Willpower"), item->Skill(crc32c("Sleep Other Spell")));
    if (succ > 0) {
      if (parent) {
        parent->SendOut(
            0,
            0,
            ";s looks groggy for a moment, but recovers.\n",
            "You feel groggy for a moment, but recover.\n",
            this,
            nullptr);
      }
    } else {
      handle_command(this, "sleep");
    }
  }
}

int Object::LightLevel(int updown) {
  int level = 0;

  if (updown != -1 && Parent()) { // Go Up
    int fac = Skill(crc32c("Open")) + Skill(crc32c("Transparent")) + Skill(crc32c("Translucent"));
    if (fac > 1000)
      fac = 1000;
    if (fac > 0) {
      level += (fac * Parent()->LightLevel(1));
    }
  }
  if (updown != 1) { // Go Down
    for (auto item : contents) {
      if (!Wearing(item)) { // Containing it (internal)
        int fac = item->Skill(crc32c("Open")) + item->Skill(crc32c("Transparent")) +
            item->Skill(crc32c("Translucent"));
        if (fac > 1000) {
          fac = 1000;
        }
        if (fac > 0) {
          level += (fac * item->LightLevel(-1));
        }
        level += 1000 * item->Skill(crc32c("Light Source"));
      }

      auto subitem = item->contents.begin();
      for (; subitem != item->contents.end(); ++subitem) {
        // Wearing it (external - so reaching one level further)
        if (item->Wearing(*subitem)) {
          level += (1000 * (*subitem)->LightLevel(-1));
        }
      }
    }
  }
  level /= 1000;
  level += Skill(crc32c("Light Source"));
  if (level > 1000)
    level = 1000;
  return level;
}

int Object::BaseAttribute(int a) const {
  return att[a].base;
}

int Object::NormAttribute(int a) const {
  if (a >= 6) { // Reaction = (Q+I)/2
    return (att[1].cur + att[4].cur) / 2;
  }
  return att[a].cur;
}

int Object::ModAttribute(int a) const {
  if (a >= 6) { // Reaction
    return (ModAttribute(1) + ModAttribute(4)) / 2 + Modifier("Reaction");
  }
  if (att[a].cur == 0) {
    return 0; // Can't Enhance Nothing
  }
  return att[a].cur + Modifier(a);
}

int Object::Modifier(int a) const {
  int ret = 0;
  for (auto item : contents) {
    if (Wearing(item) || item->Skill(crc32c("Magical Spell"))) {
      ret += item->att[a].mod;
    }
  }
  ret += att[a].mod;
  if (ret < 0)
    return (ret - 999) / 1000;
  return (ret / 1000);
}

int Object::Modifier(const std::string& m) const {
  int ret = 0;
  for (auto item : contents) {
    if (Wearing(item) || item->Skill(crc32c("Magical Spell"))) {
      ret += item->Skill(crc32c(m + " Bonus"));
      ret -= item->Skill(crc32c(m + " Penalty"));
    }
  }
  ret += Skill(crc32c(m + " Bonus"));
  ret -= Skill(crc32c(m + " Penalty"));
  if (ret < 0)
    return (ret - 999) / 1000;
  return (ret / 1000);
}

int Object::Power(const std::string& m) const {
  int ret = 0;
  ret = Skill(crc32c(m));
  for (auto item : contents) {
    if (ActTarg(ACT_WIELD) == item || Wearing(item) || item->Skill(crc32c("Magical Spell"))) {
      int val = item->Skill(crc32c(m));
      if (val > ret)
        ret = val;
    }
  }
  return ret;
}

int Object::Wearing(const Object* obj) const {
  for (act_t actt = ACT_HOLD; actt < ACT_MAX; actt = act_t(actt + 1)) {
    if (ActTarg(actt) == obj)
      return 1;
  }
  return 0;
}

int Object::WearMask() const {
  return (
      Skill(crc32c("Wearable on Back")) | Skill(crc32c("Wearable on Chest")) |
      Skill(crc32c("Wearable on Head")) | Skill(crc32c("Wearable on Neck")) |
      Skill(crc32c("Wearable on Collar")) | Skill(crc32c("Wearable on Waist")) |
      Skill(crc32c("Wearable on Shield")) | Skill(crc32c("Wearable on Left Arm")) |
      Skill(crc32c("Wearable on Right Arm")) | Skill(crc32c("Wearable on Left Finger")) |
      Skill(crc32c("Wearable on Right Finger")) | Skill(crc32c("Wearable on Left Foot")) |
      Skill(crc32c("Wearable on Right Foot")) | Skill(crc32c("Wearable on Left Hand")) |
      Skill(crc32c("Wearable on Right Hand")) | Skill(crc32c("Wearable on Left Leg")) |
      Skill(crc32c("Wearable on Right Leg")) | Skill(crc32c("Wearable on Left Wrist")) |
      Skill(crc32c("Wearable on Right Wrist")) | Skill(crc32c("Wearable on Left Shoulder")) |
      Skill(crc32c("Wearable on Right Shoulder")) | Skill(crc32c("Wearable on Left Hip")) |
      Skill(crc32c("Wearable on Right Hip")));
}

std::set<act_t> Object::WearSlots(int m) const {
  std::set<act_t> locs;
  if (Skill(crc32c("Wearable on Back")) & m)
    locs.insert(ACT_WEAR_BACK);
  if (Skill(crc32c("Wearable on Chest")) & m)
    locs.insert(ACT_WEAR_CHEST);
  if (Skill(crc32c("Wearable on Head")) & m)
    locs.insert(ACT_WEAR_HEAD);
  if (Skill(crc32c("Wearable on Face")) & m)
    locs.insert(ACT_WEAR_FACE);
  if (Skill(crc32c("Wearable on Neck")) & m)
    locs.insert(ACT_WEAR_NECK);
  if (Skill(crc32c("Wearable on Collar")) & m)
    locs.insert(ACT_WEAR_COLLAR);
  if (Skill(crc32c("Wearable on Waist")) & m)
    locs.insert(ACT_WEAR_WAIST);
  if (Skill(crc32c("Wearable on Shield")) & m)
    locs.insert(ACT_WEAR_SHIELD);
  if (Skill(crc32c("Wearable on Left Arm")) & m)
    locs.insert(ACT_WEAR_LARM);
  if (Skill(crc32c("Wearable on Right Arm")) & m)
    locs.insert(ACT_WEAR_RARM);
  if (Skill(crc32c("Wearable on Left Finger")) & m)
    locs.insert(ACT_WEAR_LFINGER);
  if (Skill(crc32c("Wearable on Right Finger")) & m)
    locs.insert(ACT_WEAR_RFINGER);
  if (Skill(crc32c("Wearable on Left Foot")) & m)
    locs.insert(ACT_WEAR_LFOOT);
  if (Skill(crc32c("Wearable on Right Foot")) & m)
    locs.insert(ACT_WEAR_RFOOT);
  if (Skill(crc32c("Wearable on Left Hand")) & m)
    locs.insert(ACT_WEAR_LHAND);
  if (Skill(crc32c("Wearable on Right Hand")) & m)
    locs.insert(ACT_WEAR_RHAND);
  if (Skill(crc32c("Wearable on Left Leg")) & m)
    locs.insert(ACT_WEAR_LLEG);
  if (Skill(crc32c("Wearable on Right Leg")) & m)
    locs.insert(ACT_WEAR_RLEG);
  if (Skill(crc32c("Wearable on Left Wrist")) & m)
    locs.insert(ACT_WEAR_LWRIST);
  if (Skill(crc32c("Wearable on Right Wrist")) & m)
    locs.insert(ACT_WEAR_RWRIST);
  if (Skill(crc32c("Wearable on Left Shoulder")) & m)
    locs.insert(ACT_WEAR_LSHOULDER);
  if (Skill(crc32c("Wearable on Right Shoulder")) & m)
    locs.insert(ACT_WEAR_RSHOULDER);
  if (Skill(crc32c("Wearable on Left Hip")) & m)
    locs.insert(ACT_WEAR_LHIP);
  if (Skill(crc32c("Wearable on Right Hip")) & m)
    locs.insert(ACT_WEAR_RHIP);
  return locs;
}

std::string Object::WearNames(const std::set<act_t>& locs) const {
  std::string ret = "";
  for (auto loc = locs.begin(); loc != locs.end(); ++loc) {
    if (loc != locs.begin()) {
      auto tmp = loc;
      ++tmp;
      if (tmp == locs.end()) {
        if (locs.size() == 2) {
          ret += " and ";
        } else {
          ret += ", and ";
        }
      } else {
        ret += ", ";
      }
    }
    if (*loc == ACT_WEAR_BACK)
      ret += "back";
    else if (*loc == ACT_WEAR_CHEST)
      ret += "chest";
    else if (*loc == ACT_WEAR_HEAD)
      ret += "head";
    else if (*loc == ACT_WEAR_FACE)
      ret += "face";
    else if (*loc == ACT_WEAR_NECK)
      ret += "neck";
    else if (*loc == ACT_WEAR_COLLAR)
      ret += "collar";
    else if (*loc == ACT_WEAR_WAIST)
      ret += "waist";
    else if (*loc == ACT_WEAR_SHIELD)
      ret += "shield";
    else if (*loc == ACT_WEAR_LARM)
      ret += "left arm";
    else if (*loc == ACT_WEAR_RARM)
      ret += "right arm";
    else if (*loc == ACT_WEAR_LFINGER)
      ret += "left finger";
    else if (*loc == ACT_WEAR_RFINGER)
      ret += "right finger";
    else if (*loc == ACT_WEAR_LFOOT)
      ret += "left foot";
    else if (*loc == ACT_WEAR_RFOOT)
      ret += "right foot";
    else if (*loc == ACT_WEAR_LHAND)
      ret += "left hand";
    else if (*loc == ACT_WEAR_RHAND)
      ret += "right hand";
    else if (*loc == ACT_WEAR_LLEG)
      ret += "left leg";
    else if (*loc == ACT_WEAR_RLEG)
      ret += "right leg";
    else if (*loc == ACT_WEAR_LWRIST)
      ret += "left wrist";
    else if (*loc == ACT_WEAR_RWRIST)
      ret += "right wrist";
    else if (*loc == ACT_WEAR_LSHOULDER)
      ret += "left shoulder";
    else if (*loc == ACT_WEAR_RSHOULDER)
      ret += "right shoulder";
    else if (*loc == ACT_WEAR_LHIP)
      ret += "left hip";
    else if (*loc == ACT_WEAR_RHIP)
      ret += "right hip";
  }
  return ret;
}

std::string Object::WearNames(int m) const {
  return WearNames(WearSlots(m));
}

Object* Object::Stash(Object* item, int message, int force, int try_combine) {
  MinVec<1, Object*> containers;
  auto my_cont = PickObjects("all", LOC_INTERNAL);
  for (auto ind : my_cont) {
    if (ind->Skill(crc32c("Container")) &&
        ((!ind->Skill(crc32c("Locked"))) || ind->Skill(crc32c("Open")))) {
      containers.push_back(ind);
    }
  }

  Object* dest = nullptr;
  for (auto con : containers) {
    if (con->Skill(crc32c("Capacity")) - con->ContainedVolume() < item->Volume())
      continue;
    if (con->Skill(crc32c("Container")) - con->ContainedWeight() < item->Weight())
      continue;
    if (!dest)
      dest = con; // It CAN go here....
    for (auto ind : con->contents) {
      if (item->IsSameAs(*ind)) {
        dest = con;
        break;
      }
    }
  }

  // See if it actually makes it!
  if (dest && (item->Travel(dest, try_combine)))
    dest = nullptr;

  if (message && dest) {
    int openclose = 0;
    if (dest->Skill(crc32c("Open")) < 1)
      openclose = 1;

    if (openclose)
      parent->SendOut(0, 0, ";s opens ;s.\n", "You open ;s.\n", this, dest);
    parent->SendOut(0, 0, ";s stashes ;s.\n", "You stash ;s.\n", this, item);
    if (openclose)
      parent->SendOut(0, 0, ";s closes ;s.\n", "You close ;s.\n", this, dest);
  }

  return dest;
}

int Object::Drop(Object* item, int message, int force, int try_combine) {
  if (!item)
    return 1;
  if (!parent)
    return 1;

  // Can't drop cursed stuff (unless ninja or otherwise forcing)
  if ((!force) && item->SubHasSkill(crc32c("Cursed"))) {
    return -4;
  }

  int ret = item->Travel(parent, try_combine);
  if (ret)
    return ret;

  // Activate perishable dropped stuff, so it will rot
  if (item->HasSkill(crc32c("Perishable"))) {
    item->Activate();
  }

  if (message) {
    parent->SendOut(0, 0, ";s drops ;s.\n", "You drop ;s.\n", this, item);
  }
  return 0;
}

int Object::DropOrStash(Object* item, int message, int force, int try_combine) {
  int ret = Drop(item, message, force, try_combine);
  if (ret) {
    if (!Stash(item, message, force, try_combine)) {
      return ret;
    }
  }
  return 0;
}

int Object::StashOrDrop(Object* item, int message, int force, int try_combine) {
  if (!Stash(item, message, force, try_combine)) {
    return Drop(item, message, force, try_combine);
  }
  return 0;
}

int Object::SubMaxSkill(uint32_t stok) const {
  int ret = Skill(stok);
  for (auto item : contents) {
    int sub = item->SubMaxSkill(stok);
    if (sub > ret)
      ret = sub;
  }
  return ret;
}

int Object::SubHasSkill(uint32_t stok) const {
  if (HasSkill(stok))
    return 1;
  for (auto item : contents) {
    if (item->SubHasSkill(stok))
      return 1;
  }
  return 0;
}

Object* Object::NextHasSkill(uint32_t stok, const Object* last) {
  if (HasSkill(stok) && (!last))
    return this;
  if (last == this)
    last = nullptr; // I was last one
  for (auto item : contents) {
    Object* found = item->NextHasSkill(stok, last);
    if (found)
      return found;
    if (last && (last == item || item->HasWithin(last))) {
      last = nullptr; // Was last item in sub-item
    }
  }
  return nullptr;
}

Object* Object::Owner() const {
  Object* owner = parent;
  while (owner && (!owner->IsAnimate()))
    owner = owner->Parent();
  return owner;
}

int Object::Quantity() const {
  if (!HasSkill(crc32c("Quantity")))
    return 1;
  return Skill(crc32c("Quantity"));
}

void Object::Deafen(bool deaf) {
  no_hear = deaf;
}

int Object::Wear(Object* targ, unsigned long masks, int mes) {
  unsigned long mask = 1;
  while ((mask & masks) == 0 && mask != 0)
    mask <<= 1;
  int success = 0;
  while (!success && mask != 0) {
    std::set<act_t> locations;

    if (targ->Skill(crc32c("Wearable on Back")) & mask)
      locations.insert(ACT_WEAR_BACK);

    if (targ->Skill(crc32c("Wearable on Chest")) & mask)
      locations.insert(ACT_WEAR_CHEST);

    if (targ->Skill(crc32c("Wearable on Head")) & mask)
      locations.insert(ACT_WEAR_HEAD);

    if (targ->Skill(crc32c("Wearable on Face")) & mask)
      locations.insert(ACT_WEAR_FACE);

    if (targ->Skill(crc32c("Wearable on Neck")) & mask)
      locations.insert(ACT_WEAR_NECK);

    if (targ->Skill(crc32c("Wearable on Collar")) & mask)
      locations.insert(ACT_WEAR_COLLAR);

    if (targ->Skill(crc32c("Wearable on Waist")) & mask)
      locations.insert(ACT_WEAR_WAIST);

    if (targ->Skill(crc32c("Wearable on Shield")) & mask)
      locations.insert(ACT_WEAR_SHIELD);

    if (targ->Skill(crc32c("Wearable on Left Arm")) & mask)
      locations.insert(ACT_WEAR_LARM);

    if (targ->Skill(crc32c("Wearable on Right Arm")) & mask)
      locations.insert(ACT_WEAR_RARM);

    if (targ->Skill(crc32c("Wearable on Left Finger")) & mask)
      locations.insert(ACT_WEAR_LFINGER);

    if (targ->Skill(crc32c("Wearable on Right Finger")) & mask)
      locations.insert(ACT_WEAR_RFINGER);

    if (targ->Skill(crc32c("Wearable on Left Foot")) & mask)
      locations.insert(ACT_WEAR_LFOOT);

    if (targ->Skill(crc32c("Wearable on Right Foot")) & mask)
      locations.insert(ACT_WEAR_RFOOT);

    if (targ->Skill(crc32c("Wearable on Left Hand")) & mask)
      locations.insert(ACT_WEAR_LHAND);

    if (targ->Skill(crc32c("Wearable on Right Hand")) & mask)
      locations.insert(ACT_WEAR_RHAND);

    if (targ->Skill(crc32c("Wearable on Left Leg")) & mask)
      locations.insert(ACT_WEAR_LLEG);

    if (targ->Skill(crc32c("Wearable on Right Leg")) & mask)
      locations.insert(ACT_WEAR_RLEG);

    if (targ->Skill(crc32c("Wearable on Left Wrist")) & mask)
      locations.insert(ACT_WEAR_LWRIST);

    if (targ->Skill(crc32c("Wearable on Right Wrist")) & mask)
      locations.insert(ACT_WEAR_RWRIST);

    if (targ->Skill(crc32c("Wearable on Left Shoulder")) & mask)
      locations.insert(ACT_WEAR_LSHOULDER);

    if (targ->Skill(crc32c("Wearable on Right Shoulder")) & mask)
      locations.insert(ACT_WEAR_RSHOULDER);

    if (targ->Skill(crc32c("Wearable on Left Hip")) & mask)
      locations.insert(ACT_WEAR_LHIP);

    if (targ->Skill(crc32c("Wearable on Right Hip")) & mask)
      locations.insert(ACT_WEAR_RHIP);

    if (locations.size() < 1) {
      if (mask == 1) {
        if (mes)
          targ->SendF(
              ALL, -1, "You can't wear %s - it's not wearable!\n", targ->Name(0, this).c_str());
      } else {
        if (mes)
          targ->SendF(
              ALL,
              -1,
              "You can't wear %s with what you are already wearing!\n",
              targ->Name(0, this).c_str());
      }
      break;
    }
    success = 1;
    mask <<= 1;
    while ((mask & masks) == 0 && mask != 0)
      mask <<= 1;

    if (targ->Skill(crc32c("Quantity")) > 1) { // One at a time!
      targ = targ->Split(1);
    }

    for (auto loc : locations) {
      if (IsAct(loc)) {
        success = 0;
        break;
      }
    }
    if (success) {
      targ->Travel(this, 0); // Kills Holds and Wields on "targ"
      for (auto loc : locations) {
        AddAct(loc, targ);
      }
      Parent()->SendOut(
          0,
          0, // FIXME: stealth_t, stealth_s,
          ";s puts on ;s.\n",
          "You put on ;s.\n",
          this,
          targ);
    }
  }
  return success;
}

Object* new_obj() {
  return new Object();
}

Object* new_obj(Object* o) {
  return new Object(o);
}

Object* new_obj(const Object& o) {
  return new Object(o);
}

MinVec<3, skill_pair> Object::GetSkills() const {
  auto ret = skills;

  std::sort(ret.begin(), ret.end(), [](auto& s1, auto& s2) {
    return (SkillName(s1.first) < SkillName(s2.first));
  });

  return ret;
}
