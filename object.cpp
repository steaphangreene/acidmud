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
#include <cctype>
#include <cstdarg>
#include <cstdio>
#include <filesystem>
#include <string>
#include <vector>

#include "cchar8.hpp"
#include "color.hpp"
#include "commands.hpp"
#include "mind.hpp"
#include "net.hpp"
#include "object.hpp"
#include "properties.hpp"
#include "utils.hpp"

const std::u8string pos_str[static_cast<uint8_t>(pos_t::MAX)] = {
    u8"is here",
    u8"is lying here",
    u8"is sitting here",
    u8"is standing here",
    u8"is using a skill",
};

const std::u8string act_str[static_cast<uint8_t>(act_t::SPECIAL_MAX)] = {
    u8"doing nothing",
    u8"dead",
    u8"bleeding and dying",
    u8"unconscious",
    u8"fast asleep",
    u8"resting",
    u8"healing {0}",
    u8"pointing {1}{2}at {0}",
    u8"following {0}",
    u8"fighting {0}",
    u8"offering something to {0}",
    u8"holding {0}",
    u8"wielding {0}",
    u8"wearing {0}",
    u8"wearing {0}",
    u8"wearing {0}",
    u8"wearing {0}",
    u8"wearing {0}",
    u8"wearing {0}",
    u8"wearing {0}",
    u8"wearing {0}",
    u8"wearing {0}",
    u8"wearing {0}",
    u8"wearing {0}",
    u8"wearing {0}",
    u8"wearing {0}",
    u8"wearing {0}",
    u8"wearing {0}",
    u8"wearing {0}",
    u8"wearing {0}",
    u8"wearing {0}",
    u8"wearing {0}",
    u8"wearing {0}",
    u8"wearing {0}",
    u8"wearing {0}",
    u8"wearing {0}",
    u8"wearing_{0}",
    u8"SPECIAL_MONITOR",
    u8"SPECIAL_MASTER",
    u8"SPECIAL_LINKED",
    u8"SPECIAL_HOME",
    u8"SPECIAL_WORK",
    u8"SPECIAL_ACTEE",
    //"SPECIAL_MAX"
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

Object* Object::Zone() {
  Object* zone = this;
  if (zone->Parent()) {
    if (zone->Parent()->Parent()) {
      while (zone->Parent()->Parent()->Parent())
        zone = zone->Parent();
    }
  }
  return zone;
}

int matches(const std::u8string_view& name, const std::u8string_view& seek) {
  if (seek.empty())
    return 0;

  auto stok = crc32c(seek);
  if (stok == crc32c(u8"all"))
    return 1;

  if (phrase_match(name, seek))
    return 1;

  static bool dont_recur = false;
  if (dont_recur)
    return 0;
  dont_recur = true;

  int ret = 0;

  // suffix-word searches
  if (stok == crc32c(u8"guard") || stok == crc32c(u8"smith") || stok == crc32c(u8"master") ||
      stok == crc32c(u8"sword") || stok == crc32c(u8"hammer") || stok == crc32c(u8"axe") ||
      stok == crc32c(u8"bow") || stok == crc32c(u8"staff") || stok == crc32c(u8"keeper")) {
    std::u8string part(seek);
    std::transform(part.begin(), part.end(), part.begin(), ascii_tolower);
    std::u8string word(name);
    std::transform(word.begin(), word.end(), word.begin(), ascii_tolower);

    auto cont = word.find(part);
    while (cont != std::u8string::npos) {
      if (word.length() == cont + part.length() || word[cont + part.length()] == ' ') {
        ret = 1;
        break;
      }
      cont = word.find(part, cont + 1);
    }
  }

  // special SMART[TM] searches
  if ((stok == crc32c(u8"guard")) && matches(name, u8"guardian"))
    ret = 1;
  else if ((stok == crc32c(u8"guard")) && matches(name, u8"guardsman"))
    ret = 1;
  else if ((stok == crc32c(u8"guard")) && matches(name, u8"guardswoman"))
    ret = 1;
  else if ((stok == crc32c(u8"merc")) && matches(name, u8"mercenary"))
    ret = 1;
  else if ((stok == crc32c(u8"bolt")) && matches(name, u8"thunderbolt"))
    ret = 1;
  else if ((stok == crc32c(u8"battle hammer")) && matches(name, u8"battlehammer"))
    ret = 1;
  else if ((stok == crc32c(u8"battlehammer")) && matches(name, u8"battle hammer"))
    ret = 1;
  else if ((stok == crc32c(u8"war hammer")) && matches(name, u8"warhammer"))
    ret = 1;
  else if ((stok == crc32c(u8"warhammer")) && matches(name, u8"war hammer"))
    ret = 1;
  else if ((stok == crc32c(u8"battle axe")) && matches(name, u8"battleaxe"))
    ret = 1;
  else if ((stok == crc32c(u8"battleaxe")) && matches(name, u8"battle axe"))
    ret = 1;
  else if ((stok == crc32c(u8"war axe")) && matches(name, u8"waraxe"))
    ret = 1;
  else if ((stok == crc32c(u8"waraxe")) && matches(name, u8"war axe"))
    ret = 1;
  else if ((stok == crc32c(u8"morning star")) && matches(name, u8"morningstar"))
    ret = 1;
  else if ((stok == crc32c(u8"morningstar")) && matches(name, u8"morning star"))
    ret = 1;
  else if ((stok == crc32c(u8"bisarme")) && matches(name, u8"gisarme"))
    ret = 1;
  else if ((stok == crc32c(u8"bisarme")) && matches(name, u8"guisarme"))
    ret = 1;
  else if ((stok == crc32c(u8"gisarme")) && matches(name, u8"bisarme"))
    ret = 1;
  else if ((stok == crc32c(u8"gisarme")) && matches(name, u8"guisarme"))
    ret = 1;
  else if ((stok == crc32c(u8"guisarme")) && matches(name, u8"bisarme"))
    ret = 1;
  else if ((stok == crc32c(u8"guisarme")) && matches(name, u8"gisarme"))
    ret = 1;
  else if ((stok == crc32c(u8"bill-bisarme")) && matches(name, u8"bill-gisarme"))
    ret = 1;
  else if ((stok == crc32c(u8"bill-bisarme")) && matches(name, u8"bill-guisarme"))
    ret = 1;
  else if ((stok == crc32c(u8"bill-gisarme")) && matches(name, u8"bill-bisarme"))
    ret = 1;
  else if ((stok == crc32c(u8"bill-gisarme")) && matches(name, u8"bill-guisarme"))
    ret = 1;
  else if ((stok == crc32c(u8"bill-guisarme")) && matches(name, u8"bill-bisarme"))
    ret = 1;
  else if ((stok == crc32c(u8"bill-guisarme")) && matches(name, u8"bill-gisarme"))
    ret = 1;
  else if ((stok == crc32c(u8"grey")) && matches(name, u8"gray"))
    ret = 1;
  else if ((stok == crc32c(u8"gray")) && matches(name, u8"grey"))
    ret = 1;
  else if ((stok == crc32c(u8"bread")) && matches(name, u8"waybread"))
    ret = 1;

  // One-way purposeful mis-spellings to silence some extra labelling from TBA
  else if ((stok == crc32c(u8"potatoe")) && matches(name, u8"potato"))
    ret = 1;

  dont_recur = false;
  return ret;
}

int Object::Matches(std::u8string targ, bool knows) const {
  trim_string(targ);

  // Pointer Matches
  if (!strncmp(targ.c_str(), u8"OBJ:", 4) || !strncmp(targ.c_str(), u8"obj:", 4)) {
    Object* tofind = nullptr;
    sscanf(targ.c_str() + 4, u8"%p", &tofind);
    return (this == tofind);
  }

  auto ttok = crc32c(targ);

  // Keywords Only
  if (ttok == crc32c(u8"everyone")) {
    return (IsAnimate());
  }
  if (ttok == crc32c(u8"someone")) {
    return (IsAnimate());
  }
  if (ttok == crc32c(u8"anyone")) {
    return (IsAnimate());
  }
  if (ttok == crc32c(u8"everything")) {
    if (IsAct(act_t::SPECIAL_LINKED))
      return 0;
    return (!IsAnimate());
  }
  if (ttok == crc32c(u8"something")) {
    if (IsAct(act_t::SPECIAL_LINKED))
      return 0;
    return (!IsAnimate());
  }
  if (ttok == crc32c(u8"anything")) {
    if (IsAct(act_t::SPECIAL_LINKED))
      return 0;
    return (!IsAnimate());
  }
  if (ttok == crc32c(u8"everywhere")) {
    if (!IsAct(act_t::SPECIAL_LINKED))
      return 0;
    return (!IsAnimate());
  }
  if (ttok == crc32c(u8"somewhere")) {
    if (!IsAct(act_t::SPECIAL_LINKED))
      return 0;
    return (!IsAnimate());
  }
  if (ttok == crc32c(u8"anywhere")) {
    if (!IsAct(act_t::SPECIAL_LINKED))
      return 0;
    return (!IsAnimate());
  }

  // Matches for sexist TBA aliases :)
  if (ttok == crc32c(u8"man") || ttok == crc32c(u8"boy")) {
    if (IsAnimate() && Gender() == 'M')
      return 1;
  }
  if (ttok == crc32c(u8"woman") || ttok == crc32c(u8"girl")) {
    if (IsAnimate() && Gender() == 'F')
      return 1;
  }

  // Keywords which can also be things
  if ((ttok == crc32c(u8"Corpse")) && IsAct(act_t::DEAD))
    return 1;
  if ((ttok == crc32c(u8"Money")) && Skill(prhash(u8"Money")))
    return 1;

  return (knows && matches(Name(), targ)) || matches(ShortDesc(), targ);
}

Object* new_body(Object* world) {
  Object* body = new Object();
  body->SetAttribute(0, 3);
  body->SetAttribute(1, 3);
  body->SetAttribute(2, 3);
  body->SetAttribute(3, 3);
  body->SetAttribute(4, 3);
  body->SetAttribute(5, 3);

  body->SetShortDesc(u8"an amorphous blob");

  Object* start = world->ActTarg(act_t::SPECIAL_HOME);
  if (!start)
    start = world;
  body->SetParent(start);

  body->SetWeight(body->NormAttribute(0) * 20000);
  body->SetSize(1000 + body->NormAttribute(0) * 200);
  body->SetVolume(100);
  body->SetValue(-1);
  body->SetWeight(80000);
  body->SetGender('M');

  body->SetSkill(prhash(u8"Attribute Points"), 12);
  body->SetSkill(prhash(u8"Skill Points"), 64);
  body->SetSkill(prhash(u8"Status Points"), 6);
  body->SetSkill(prhash(u8"Invisible"), 1000);

  body->SetPos(pos_t::STAND);

  body->Activate();

  Object* bag = new Object(body);
  bag->SetSkill(prhash(u8"Capacity"), 40);
  bag->SetSkill(prhash(u8"Closeable"), 1);
  bag->SetSkill(prhash(u8"Container"), 20000);
  bag->SetSkill(prhash(u8"Open"), 1000);
  bag->SetSkill(prhash(u8"Wearable on Left Hip"), 1);
  bag->SetSkill(prhash(u8"Wearable on Right Hip"), 2);

  bag->SetShortDesc(u8"a small bag");
  bag->SetDesc(u8"A small bag is here.");

  bag->SetWeight(800);
  bag->SetSize(1);
  bag->SetVolume(2);
  bag->SetValue(10);

  bag->SetPos(pos_t::LIE);

  body->AddAct(act_t::WEAR_RHIP, bag);

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
    if (!m->Think(1)) {
      delete m;
    }
  }

  if (phys > (10 + ModAttribute(2))) {
    // You are already dead.
  } else if (phys >= 10) {
    int rec = RollNoWounds(prhash(u8"Strength"), phys - 4, 0);
    if (!rec)
      ++phys;
    UpdateDamage();
  } else if (phys > 0) {
    int rec = 0;
    if (IsAct(act_t::SLEEP))
      rec = Roll(prhash(u8"Body"), 2);
    else if (IsAct(act_t::REST))
      rec = Roll(prhash(u8"Body"), 4);
    else if (!IsAct(act_t::FIGHT))
      rec = Roll(prhash(u8"Body"), 6);
    if (phys >= 6 && (!rec))
      ++phys;
    else
      phys -= rec / 2;
    phys = std::max(int8_t(0), phys);
    UpdateDamage();
  }
  if (phys < 10 && stun >= 10) {
    int rec = 0;
    rec = RollNoWounds(prhash(u8"Willpower"), 12, 0);
    stun -= rec;
    stun = std::max(int8_t(0), stun);
    UpdateDamage();
  } else if (phys < 6 && stun > 0) {
    int rec = 0;
    if (IsAct(act_t::SLEEP))
      rec = Roll(prhash(u8"Willpower"), 2);
    else if (IsAct(act_t::REST))
      rec = Roll(prhash(u8"Willpower"), 4);
    else if (!IsAct(act_t::FIGHT))
      rec = Roll(prhash(u8"Willpower"), 6);
    stun -= rec;
    stun = std::max(int8_t(0), stun);
    UpdateDamage();
  }

  if (parent && Skill(prhash(u8"TBAPopper")) > 0 && contents.size() > 0) {
    if (!ActTarg(act_t::SPECIAL_MONITOR)) {
      Object* obj = new Object(*(contents.front()));
      obj->SetParent(this);
      obj->Travel(parent);
      AddAct(act_t::SPECIAL_MONITOR, obj);
      obj->Attach(get_tba_mob_mind());
      obj->Activate();
      parent->SendOut(ALL, -1, u8";s arrives.\n", u8"", obj, nullptr);
      for (auto trg : obj->Contents()) { // Enable any untriggered triggers
        if (trg->HasSkill(prhash(u8"TBAScript")) &&
            (trg->Skill(prhash(u8"TBAScriptType")) & 0x0000002)) {
          trg->Activate();
          new_trigger(13000 + (rand() % 13000), trg, nullptr, nullptr, u8"");
        }
      }
    }
  }

  // Grow Trees (Silently)
  if (HasSkill(prhash(u8"Mature Trees")) && Skill(prhash(u8"Mature Trees")) < 100) {
    SetSkill(prhash(u8"Mature Trees"), Skill(prhash(u8"Mature Trees")) + 1);
  }

  if (IsAct(act_t::DEAD)) { // Rotting corpses
    ++stru;
    if (stru == 1) {
      parent->SendOut(ALL, 0, u8";s's corpse starts to smell.\n", u8"", this, nullptr);
    } else if (stru == 3) {
      parent->SendOut(ALL, 0, u8";s's corpse starts to rot.\n", u8"", this, nullptr);
    } else if (stru == 6) {
      parent->SendOut(ALL, 0, u8";s's corpse starts to fall apart.\n", u8"", this, nullptr);
    } else if (stru >= 10) {
      Object* corpse = new Object(parent);

      corpse->SetShortDesc(u8"an unidentifiable corpse");
      corpse->SetDesc(u8"A pile of rotting remains.");
      corpse->SetPos(pos_t::LIE);

      corpse->SetSkill(prhash(u8"Perishable"), 1);
      corpse->SetSkill(prhash(u8"Rot"), 1);
      corpse->Activate();

      corpse->SetWeight(Weight());
      corpse->SetSize(Size());
      corpse->SetVolume(Volume());

      std::set<Object*> todrop;
      MinVec<1, Object*> todropfrom;
      todropfrom.push_back(this);

      for (auto con : todropfrom) {
        for (auto item : con->contents) {
          if (item->contents.size() > 0 && item->Pos() == pos_t::NONE) {
            todropfrom.push_back(item);
          } else if (item->Pos() != pos_t::NONE) { // Fixed items can't be dropped!
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

      parent->SendOut(ALL, 0, u8";s's corpse completely falls apart.\n", u8"", this, nullptr);

      if (is_pc(this)) { // Hide me in the VOID!
        Object* dest = this;
        while ((!dest->ActTarg(act_t::SPECIAL_HOME)) && dest->Parent()) {
          dest = dest->Parent();
        }
        if (dest->ActTarg(act_t::SPECIAL_HOME)) {
          dest = dest->ActTarg(act_t::SPECIAL_HOME);
        }
        Travel(dest);
        SetSkill(prhash(u8"Hidden"), 65535);
        return -1; // Deactivate Me!
      } else {
        return 1; // Delete Me!
      }
    }
  }

  if (HasSkill(prhash(u8"Perishable"))) { // Degrading Items
    SetSkill(prhash(u8"Rot"), Skill(prhash(u8"Rot")) - 1);
    if (Skill(prhash(u8"Rot")) < 1) {
      ++stru;
      if (stru < 10) {
        SetSkill(prhash(u8"Rot"), Skill(prhash(u8"Perishable")));
      } else {
        return 1; // Delete Me!
      }
    }
  }

  if (HasSkill(prhash(u8"Temporary"))) { // Temporary Items
    SetSkill(prhash(u8"Temporary"), Skill(prhash(u8"Temporary")) - 1);
    if (Skill(prhash(u8"Temporary")) < 1) {
      if (Owner() && Owner()->Parent()) {
        Owner()->Parent()->SendOut(0, 0, u8";s vanishes in a flash of light.", u8"", this, nullptr);
      }
      return 1; // Delete Me!
    }
  }

  if (NormAttribute(2) > 0 // Needs Food & Water
      && (HasSkill(prhash(u8"Object ID"))) // Active PC/NPC
  ) {
    int level;

    // Get Hungrier
    level = Skill(prhash(u8"Hungry"));
    if (level < 1)
      level = ModAttribute(2);
    else
      level += ModAttribute(2); // Base Strength Scales Food Req
    if (level > 2999999)
      level = 2999999;
    SetSkill(prhash(u8"Hungry"), level);

    if (level == 50000)
      Send(ALL, -1, u8"You could use a snack.\n");
    else if (level == 100000)
      Send(ALL, -1, u8"You officially have the munchies.\n");
    else if (level == 150000)
      Send(ALL, -1, u8"You really could go for a snack.\n");
    else if (level == 200000)
      Send(ALL, -1, u8"You are getting hungry.\n");
    else if (level == 250000)
      Send(ALL, -1, u8"You are getting very hungry.\n");
    else if (level == 300000)
      Send(ALL, -1, u8"You are really quite hungry.\n");
    else if (level == 350000)
      Send(ALL, -1, u8"You are really dying for food.\n");
    else if (level == 400000)
      Send(ALL, -1, u8"You need to get some food soon!\n");
    else if (level == 450000)
      Send(ALL, -1, u8"You are starting to starve!\n");
    else if (level >= 500000) {
      if (level % 10 == 0) {
        Send(ALL, -1, u8"You are starving!\n");
      }
      UpdateDamage();
    }

    // Get Thurstier
    level = Skill(prhash(u8"Thirsty"));
    if (level < 1)
      level = ModAttribute(0);
    else
      level += ModAttribute(0); // Body Scales Water Req
    if (level > 299999)
      level = 299999;
    SetSkill(prhash(u8"Thirsty"), level);

    if (level == 5000)
      Send(ALL, -1, u8"You could use a drink.\n");
    else if (level == 10000)
      Send(ALL, -1, u8"Your mouth is getting dry.\n");
    else if (level == 15000)
      Send(ALL, -1, u8"You really could go for a drink.\n");
    else if (level == 20000)
      Send(ALL, -1, u8"You are getting thirsty.\n");
    else if (level == 25000)
      Send(ALL, -1, u8"You are getting very thirsty.\n");
    else if (level == 30000)
      Send(ALL, -1, u8"You are really quite thirsty.\n");
    else if (level == 35000)
      Send(ALL, -1, u8"You are really dying for water.\n");
    else if (level == 40000)
      Send(ALL, -1, u8"You need to get some water soon!\n");
    else if (level == 45000)
      Send(ALL, -1, u8"You are starting to dehydrate!\n");
    else if (level >= 50000) {
      if (level % 10 == 0) {
        Send(ALL, -1, u8"You are dehydrated!\n");
      }
      UpdateDamage();
    }
  }

  if (HasSkill(prhash(u8"Liquid Source"))) { // Refills Itself
    if (contents.size() > 0) {
      int qty = 1;
      if (contents.front()->Skill(prhash(u8"Quantity")) > 1) {
        qty = contents.front()->Skill(prhash(u8"Quantity"));
      }
      if (qty < Skill(prhash(u8"Liquid Container"))) {
        contents.front()->SetSkill(prhash(u8"Quantity"), qty + Skill(prhash(u8"Liquid Source")));
        if (contents.front()->Skill(prhash(u8"Quantity")) > Skill(prhash(u8"Liquid Container"))) {
          contents.front()->SetSkill(prhash(u8"Quantity"), Skill(prhash(u8"Liquid Container")));
        }
      }
    } else {
      fprintf(stderr, u8"Warning: Fountain completely out of liquid!\n");
    }
  }

  // Lit Torches/Lanterns
  if (HasSkill(prhash(u8"Lightable")) && HasSkill(prhash(u8"Light Source"))) {
    SetSkill(prhash(u8"Lightable"), Skill(prhash(u8"Lightable")) - 1);
    bool goesout = false;
    if (Skill(prhash(u8"Lightable")) < 1) {
      goesout = true;
    } else if (!parent->IsAnimate()) {
      int chances = Skill(prhash(u8"Resilience"));
      goesout = ((rand() % 1000) >= chances);
    }
    if (goesout) {
      SetSkill(prhash(u8"Light Source"), 0);
      parent->SendOut(ALL, -1, u8";s goes out.\n", u8"", this, nullptr);
      return -1; // Deactivate Me!
    }
  }

  // Skys
  if (Skill(prhash(u8"Day Length")) > 1) { // Must be > 1 (divide by it/2 below!)
    SetSkill(prhash(u8"Day Time"), Skill(prhash(u8"Day Time")) + 1);
    if (Skill(prhash(u8"Day Time")) >= Skill(prhash(u8"Day Length"))) {
      SetSkill(prhash(u8"Day Time"), 0);
    }
    int light = Skill(prhash(u8"Day Time")) - (Skill(prhash(u8"Day Length")) / 2);
    if (light < 0)
      light = -light;
    light *= 900;
    light /= (Skill(prhash(u8"Day Length")) / 2);
    SetSkill(prhash(u8"Light Source"), 1000 - light);
  }

  // Poisoned
  if (Skill(prhash(u8"Poisoned")) > 0) {
    int succ = Roll(prhash(u8"Strength"), Skill(prhash(u8"Poisoned")));
    SetSkill(prhash(u8"Poisoned"), Skill(prhash(u8"Poisoned")) - succ);
    Parent()->SendOut(
        0,
        0,
        u8";s chokes and writhes in pain.\n",
        CRED u8"You choke and writhe in pain.  POISON!!!!\n" CNRM,
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
  parent = nullptr;
  pos = pos_t::NONE;
  cur_skill = prhash(u8"None");

  weight = 0;
  volume = 0;
  size = 0;
  value = 0;
  gender = 'N';

  sexp = 0;

  stun = 0;
  phys = 0;
  stru = 0;

  attr[0] = 0;
  attr[1] = 0;
  attr[2] = 0;
  attr[3] = 0;
  attr[4] = 0;
  attr[5] = 0;

  no_seek = false;
  no_hear = false;
  tickstep = -1;
}

Object::Object(Object* o) {
  parent = nullptr;
  SetParent(o);
  pos = pos_t::NONE;
  cur_skill = prhash(u8"None");

  weight = 0;
  volume = 0;
  size = 0;
  value = 0;
  gender = 'N';

  sexp = 0;

  stun = 0;
  phys = 0;
  stru = 0;

  attr[0] = 0;
  attr[1] = 0;
  attr[2] = 0;
  attr[3] = 0;
  attr[4] = 0;
  attr[5] = 0;

  no_seek = false;
  no_hear = false;
  tickstep = -1;
}

Object::Object(const Object& o) {
  dlens = o.dlens;
  if (o.descriptions == default_descriptions) {
    descriptions = default_descriptions;
  } else {
    const size_t len = dlens.sd + dlens.n + dlens.d + dlens.ld + 4;
    char8_t* new_descs = new char8_t[len];
    std::memcpy(new_descs, o.descriptions, len);
    descriptions = new_descs;
  }

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

  attr[0] = o.attr[0];
  attr[1] = o.attr[1];
  attr[2] = o.attr[2];
  attr[3] = o.attr[3];
  attr[4] = o.attr[4];
  attr[5] = o.attr[5];

  skills = o.skills;

  contents.clear();
  for (auto ind : o.contents) {
    Object* nobj = new Object(*ind);
    nobj->SetParent(this);
    if (o.ActTarg(act_t::HOLD) == ind)
      AddAct(act_t::HOLD, nobj);
    if (o.ActTarg(act_t::WIELD) == ind)
      AddAct(act_t::WIELD, nobj);
    for (act_t actt = act_t::WEAR_BACK; actt < act_t::MAX; actt = act_t(int(actt) + 1))
      if (o.ActTarg(actt) == ind)
        AddAct(actt, nobj);
  }
  if (o.IsAct(act_t::DEAD))
    AddAct(act_t::DEAD);
  if (o.IsAct(act_t::DYING))
    AddAct(act_t::DYING);
  if (o.IsAct(act_t::UNCONSCIOUS))
    AddAct(act_t::UNCONSCIOUS);
  if (o.IsAct(act_t::SLEEP))
    AddAct(act_t::SLEEP);
  if (o.IsAct(act_t::REST))
    AddAct(act_t::REST);

  parent = nullptr;

  if (o.IsActive())
    Activate();

  minds = o.minds; // Transmit Attached Minds

  no_seek = false;
  no_hear = false;
  tickstep = -1;
}

std::u8string Object::Pron() const {
  std::u8string ret;
  if (Gender() == 'M') {
    ret = u8"he";
  } else if (Gender() == 'F') {
    ret = u8"she";
  } else {
    ret = u8"it";
  }
  return ret;
}

std::u8string Object::Poss() const {
  std::u8string ret;
  if (Gender() == 'M') {
    ret = u8"his";
  } else if (Gender() == 'F') {
    ret = u8"her";
  } else {
    ret = u8"its";
  }
  return ret;
}

std::u8string Object::Obje() const {
  std::u8string ret;
  if (Gender() == 'M') {
    ret = u8"him";
  } else if (Gender() == 'F') {
    ret = u8"her";
  } else {
    ret = u8"it";
  }
  return ret;
}

// Generate truly-formatted name/noun/pronoun/possessive....
std::u8string Object::Noun(bool definite, bool verbose, const Object* rel, const Object* sub)
    const {
  bool need_an = false;
  bool proper = false;
  std::u8string ret;

  if (rel == this && sub == this)
    return u8"yourself";
  else if (rel == this)
    return u8"you";

  // FIXME: Hack!  Really detect/specify reflexives?
  else if (rel == nullptr && sub == this && sub->Gender() == 'F')
    return u8"her";
  else if (rel == nullptr && sub == this && sub->Gender() == 'M')
    return u8"him";
  else if (rel == nullptr && sub == this)
    return u8"it";

  else if (sub == this && sub->Gender() == 'F')
    return u8"herself";
  else if (sub == this && sub->Gender() == 'M')
    return u8"himself";
  else if (sub == this)
    return u8"itself";

  if (!strncmp(ShortDescC(), u8"a ", 2)) {
    ret = ShortDesc().substr(2);
    need_an = false;
  } else if (!strncmp(ShortDescC(), u8"an ", 3)) {
    ret = ShortDesc().substr(3);
    need_an = true;
  } else if (!strncmp(ShortDescC(), u8"the ", 4)) {
    ret = ShortDesc().substr(4);
    definite = true;
  } else {
    ret = ShortDesc();
    proper = true;
  }

  if (!IsAnimate()) {
    Object* own = Owner();
    if (own && own == rel) {
      ret = fmt::format(u8"your {}", ret);
    } else if (own && own == sub && own->Gender() == 'F') {
      ret = fmt::format(u8"her {}", ret);
    } else if (own && own == sub && own->Gender() == 'M') {
      ret = fmt::format(u8"his {}", ret);
    } else if (own && own == sub) {
      ret = fmt::format(u8"its {}", ret);
    } else if (own) {
      ret = own->Noun() + u8"'s " + ret;
    } else if (definite && (!proper)) {
      ret = fmt::format(u8"the {}", ret);
    } else if ((!proper) && need_an) {
      ret = fmt::format(u8"an {}", ret);
    } else if (!proper) {
      ret = fmt::format(u8"a {}", ret);
    }
  } else if (definite && (!proper)) {
    ret = fmt::format(u8"the {}", ret);
  } else if ((!proper) && need_an) {
    ret = fmt::format(u8"an {}", ret);
  } else if (!proper) {
    ret = fmt::format(u8"a {}", ret);
  }

  if (HasName() && (rel == nullptr || rel->Knows(this))) {
    if (verbose) {
      ret = fmt::format(u8"{}, {},", Name(), ret);
    } else {
      ret = Name();
    }
  }

  return ret;
}

void Object::SetDescs(
    std::u8string_view sd,
    std::u8string_view n,
    std::u8string_view d,
    std::u8string_view ld) {
  trim_string(sd);
  trim_string(n);
  trim_string(d);
  trim_string(ld);
  if (sd.length() > 80) { // No longer than one traditional line of text
    sd = sd.substr(0, 80);
    trim_string(sd);
  }
  if (n.length() > 80) { // No longer than one traditional line of text
    n = n.substr(0, 80);
    trim_string(n);
  }
  if (d.length() > 8000) { // No longer than 100 traditional lines of text
    d = d.substr(0, 8000);
    trim_string(d);
  }
  if (ld.length() > 0xFFFFUL) { // Max size this field can store
    ld = ld.substr(0, 0xFFFFUL);
    trim_string(ld);
  }
  dlens.sd = sd.length();
  dlens.n = n.length();
  dlens.d = d.length();
  dlens.ld = ld.length();
  const size_t len = dlens.sd + dlens.n + dlens.d + dlens.ld + 4;

  char8_t* new_descs = new char8_t[len];
  std::memcpy(new_descs, sd.data(), dlens.sd);
  new_descs[dlens.sd] = 0;
  std::memcpy(new_descs + dlens.sd + 1, n.data(), dlens.n);
  new_descs[dlens.sd + dlens.n + 1] = 0;
  std::memcpy(new_descs + dlens.sd + dlens.n + 2, d.data(), dlens.d);
  new_descs[dlens.sd + dlens.n + dlens.d + 2] = 0;
  std::memcpy(new_descs + dlens.sd + dlens.n + dlens.d + 3, ld.data(), dlens.ld);
  new_descs[dlens.sd + dlens.n + dlens.d + dlens.ld + 3] = 0;

  if (descriptions != default_descriptions) {
    delete[] descriptions;
  }
  descriptions = new_descs;
}

void Object::SetShortDesc(const std::u8string_view& sd) {
  SetDescs(sd, Name(), Desc(), LongDesc());
}

void Object::SetName(const std::u8string_view& n) {
  SetDescs(ShortDesc(), n, Desc(), LongDesc());
}

void Object::SetDesc(const std::u8string_view& d) {
  SetDescs(ShortDesc(), Name(), d, LongDesc());
}

void Object::SetLongDesc(const std::u8string_view& ld) {
  SetDescs(ShortDesc(), Name(), Desc(), ld);
}

void Object::SetParent(Object* o) {
  parent = o;
  if (o)
    o->AddLink(this);
}

void Object::SendContents(Object* targ, Object* o, int vmode, std::u8string b) {
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

static std::u8string base = u8"";
static char8_t buf[65536];

void Object::SendActions(Mind* m) {
  for (auto cur : act) {
    if (cur.act() < act_t::WEAR_BACK) {
      std::u8string targ;
      std::u8string dirn = u8"";
      std::u8string dirp = u8"";

      if (!cur.obj())
        targ = u8"";
      else
        targ = cur.obj()->Noun(0, 0, m->Body(), this);

      //      //FIXME: Busted!  This should be the u8"pointing north to bob"
      //      thingy.
      //      for(auto dir : connections) {
      //	if(dir.second == cur.obj()) {
      //	  dirn = dir.first;
      //	  dirp = u8" ";
      //	  break;
      //	  }
      //	}
      m->Send(u8", ");
      m->Send(act_str[static_cast<uint8_t>(cur.act())], targ, dirn, dirp);
    }
  }
  if (HasSkill(prhash(u8"Invisible"))) {
    m->Send(u8", invisible");
  }
  if (HasSkill(prhash(u8"Light Source"))) {
    if (Skill(prhash(u8"Light Source")) < 20)
      m->Send(u8", glowing");
    else if (HasSkill(prhash(u8"Lightable")))
      m->Send(u8", burning");
    else if (Skill(prhash(u8"Light Source")) < 200)
      m->Send(u8", lighting the area");
    else
      m->Send(u8", shining");
  }
  if (HasSkill(prhash(u8"Noise Source"))) {
    if (Skill(prhash(u8"Noise Source")) < 20)
      m->Send(u8", humming");
    else if (Skill(prhash(u8"Noise Source")) < 200)
      m->Send(u8", buzzing");
    else
      m->Send(u8", roaring");
  }
  m->Send(u8".\n");
}

void Object::SendExtendedActions(Mind* m, int vmode) {
  std::map<Object*, std::u8string> shown;
  for (auto cur : act) {
    if ((vmode & (LOC_TOUCH | LOC_HEAT | LOC_NINJA)) == 0 // Can't See/Feel Invis
        && cur.obj() && cur.obj()->Skill(prhash(u8"Invisible")) > 0) {
      continue; // Don't show invisible equip
    }
    if (cur.act() == act_t::HOLD)
      m->Send(u8"{:>24}", u8"Held: ");
    else if (cur.act() == act_t::WIELD)
      m->Send(u8"{:>24}", u8"Wielded: ");
    else if (cur.act() == act_t::WEAR_BACK)
      m->Send(u8"{:>24}", u8"Worn on back: ");
    else if (cur.act() == act_t::WEAR_CHEST)
      m->Send(u8"{:>24}", u8"Worn on chest: ");
    else if (cur.act() == act_t::WEAR_HEAD)
      m->Send(u8"{:>24}", u8"Worn on head: ");
    else if (cur.act() == act_t::WEAR_FACE)
      m->Send(u8"{:>24}", u8"Worn on face: ");
    else if (cur.act() == act_t::WEAR_NECK)
      m->Send(u8"{:>24}", u8"Worn on neck: ");
    else if (cur.act() == act_t::WEAR_COLLAR)
      m->Send(u8"{:>24}", u8"Worn on collar: ");
    else if (cur.act() == act_t::WEAR_WAIST)
      m->Send(u8"{:>24}", u8"Worn on waist: ");
    else if (cur.act() == act_t::WEAR_SHIELD)
      m->Send(u8"{:>24}", u8"Worn as shield: ");
    else if (cur.act() == act_t::WEAR_LARM)
      m->Send(u8"{:>24}", u8"Worn on left arm: ");
    else if (cur.act() == act_t::WEAR_RARM)
      m->Send(u8"{:>24}", u8"Worn on right arm: ");
    else if (cur.act() == act_t::WEAR_LFINGER)
      m->Send(u8"{:>24}", u8"Worn on left finger: ");
    else if (cur.act() == act_t::WEAR_RFINGER)
      m->Send(u8"{:>24}", u8"Worn on right finger: ");
    else if (cur.act() == act_t::WEAR_LFOOT)
      m->Send(u8"{:>24}", u8"Worn on left foot: ");
    else if (cur.act() == act_t::WEAR_RFOOT)
      m->Send(u8"{:>24}", u8"Worn on right foot: ");
    else if (cur.act() == act_t::WEAR_LHAND)
      m->Send(u8"{:>24}", u8"Worn on left hand: ");
    else if (cur.act() == act_t::WEAR_RHAND)
      m->Send(u8"{:>24}", u8"Worn on right hand: ");
    else if (cur.act() == act_t::WEAR_LLEG)
      m->Send(u8"{:>24}", u8"Worn on left leg: ");
    else if (cur.act() == act_t::WEAR_RLEG)
      m->Send(u8"{:>24}", u8"Worn on right leg: ");
    else if (cur.act() == act_t::WEAR_LWRIST)
      m->Send(u8"{:>24}", u8"Worn on left wrist: ");
    else if (cur.act() == act_t::WEAR_RWRIST)
      m->Send(u8"{:>24}", u8"Worn on right wrist: ");
    else if (cur.act() == act_t::WEAR_LSHOULDER)
      m->Send(u8"{:>24}", u8"Worn on left shoulder: ");
    else if (cur.act() == act_t::WEAR_RSHOULDER)
      m->Send(u8"{:>24}", u8"Worn on right shoulder: ");
    else if (cur.act() == act_t::WEAR_LHIP)
      m->Send(u8"{:>24}", u8"Worn on left hip: ");
    else if (cur.act() == act_t::WEAR_RHIP)
      m->Send(u8"{:>24}", u8"Worn on right hip: ");
    else
      continue;

    if ((vmode & (LOC_HEAT | LOC_NINJA)) == 0 // Can't see (but can touch)
        && cur.obj() && cur.obj()->Skill(prhash(u8"Invisible")) > 0) {
      m->Send(CGRN u8"Something invisible.\n" CNRM);
      continue; // Don't show details of invisible equip
    }

    std::u8string targ;
    if (!cur.obj())
      targ = u8"";
    else
      targ = cur.obj()->Noun(0, 0, m->Body(), this);

    std::u8string qty = u8"";
    if (cur.obj()->Skill(prhash(u8"Quantity")) > 1)
      qty = fmt::format(u8"(x{}) ", cur.obj()->Skill(prhash(u8"Quantity")));

    if (shown.count(cur.obj()) > 0) {
      m->Send(u8"{}{} ({}).\n", qty, targ, shown[cur.obj()]);
    } else {
      m->Send(CGRN u8"{}{}.\n" CNRM, qty, targ);
      if (cur.obj()->Skill(prhash(u8"Open")) || cur.obj()->Skill(prhash(u8"Transparent"))) {
        sprintf(buf, u8"%16s  %c", u8" ", 0);
        base = buf;
        cur.obj()->SendContents(m, nullptr, vmode);
        base = u8"";
        m->Send(CNRM);
      } else if (cur.obj()->Skill(prhash(u8"Container"))) {
        if ((vmode & 1) && cur.obj()->Skill(prhash(u8"Locked"))) {
          std::u8string mes =
              base + CNRM + u8"                " + u8"  It is closed and locked.\n" + CGRN;
          m->Send(mes.c_str());
        } else if (vmode & 1) {
          sprintf(buf, u8"%16s  %c", u8" ", 0);
          base = buf;
          cur.obj()->SendContents(m, nullptr, vmode);
          base = u8"";
          m->Send(CNRM);
        }
      }
    }
    shown[cur.obj()] = u8"already listed";
  }
}

void Object::SendContents(Mind* m, Object* o, int vmode, std::u8string b) {
  auto cont = contents;

  if (!b.empty())
    base += b;

  std::set<Object*> master;
  master.insert(cont.begin(), cont.end());

  for (act_t actt = act_t::HOLD; actt < act_t::MAX; actt = act_t(int(actt) + 1)) {
    master.erase(ActTarg(actt)); // Don't show worn/wielded stuff.
  }

  size_t tlines = 0;
  for (auto ind : cont)
    if (master.count(ind)) {
      if ((vmode & LOC_NINJA) == 0 && Parent() != nullptr) { // NinjaMode/CharRoom
        if (ind->Skill(prhash(u8"Invisible")) > 999)
          continue;
        if (ind->HasSkill(prhash(u8"Invisible"))) {
          // Can't detect it at all
          if ((vmode & (LOC_TOUCH | LOC_HEAT)) == 0)
            continue;
          // Can't see it, and it's mobile, so can't feel it
          if ((vmode & LOC_TOUCH) == 0 && ind->Pos() > pos_t::SIT)
            continue;
        }
        if (ind->Skill(prhash(u8"Hidden")) > 0)
          continue;
      }

      if (ind->HasSkill(prhash(u8"Invisible")) && (vmode & (LOC_HEAT | LOC_NINJA)) == 0 &&
          Parent() != nullptr) {
        if (base != u8"")
          m->Send(u8"{}{}Inside: ", base, CNRM);
        m->Send(CGRN u8"Something invisible is here.\n" CNRM);
        continue; // Can feel, but can't see
      }

      if (ind->IsAct(act_t::SPECIAL_LINKED)) {
        if (ind->ActTarg(act_t::SPECIAL_LINKED) && ind->ActTarg(act_t::SPECIAL_LINKED)->Parent()) {
          if (base != u8"")
            m->Send(u8"{}{}Inside: ", base, CNRM);
          m->Send(CCYN);
          std::u8string send = ind->ShortDescS();
          if (!(ind->Skill(prhash(u8"Open")) || ind->Skill(prhash(u8"Transparent")))) {
            send += u8", the door is closed.\n";
          } else {
            if (ind->Skill(prhash(u8"Closeable")))
              send += u8", through an open door,";
            send += u8" you see ";
            send += ind->ActTarg(act_t::SPECIAL_LINKED)->Parent()->ShortDesc();
            send += u8".\n";
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
          size_t ignore = 0;
          if (o && o->Parent() == this)
            ignore = 1;
          m->Send(base.c_str());
          m->Send(
              CGRN u8"...and {} more things are here too.\n" CNRM, cont.size() - tlines - ignore);
          break;
        }

        if (base != u8"")
          m->Send(u8"{}Inside: ", base);

        /*	Uncomment this and comment the block below to disable
           auto-pluralizing.
              int qty = std::max(1, ind->Skill(prhash(u8"Quantity")));
        */
        int qty = 1; // Even animate objects can have higher quantities.
        auto oth = std::find(cont.begin(), cont.end(), ind);
        for (qty = 0; oth != cont.end(); ++oth) {
          if (ind->LooksLike(*oth, vmode, o)) {
            master.erase(*oth);
            qty += std::max(1, (*oth)->Skill(prhash(u8"Quantity")));
          }
        }

        if (ind->IsAnimate())
          m->Send(CYEL);
        else
          m->Send(CGRN);

        if (qty > 1)
          m->Send(u8"(x{}) ", qty);
        ++tlines;

        if (ind->parent && ind->parent->Skill(prhash(u8"Container"))) {
          sprintf(buf, u8"%s%c", ind->Noun().c_str(), 0);
        } else if (vmode & LOC_NINJA) {
          sprintf(buf, u8"%s %s%c", ind->Noun(false).c_str(), ind->PosString().c_str(), 0);
        } else {
          sprintf(buf, u8"%s %s%c", ind->Noun(false, true, o).c_str(), ind->PosString().c_str(), 0);
        }
        buf[0] = ascii_toupper(buf[0]);
        m->Send(buf);

        ind->SendActions(m);

        m->Send(CNRM);
        if (ind->Skill(prhash(u8"Open")) || ind->Skill(prhash(u8"Transparent"))) {
          std::u8string tmp = base;
          base += u8"  ";
          ind->SendContents(m, o, vmode);
          base = tmp;
        } else if (ind->Skill(prhash(u8"Container")) || ind->Skill(prhash(u8"Liquid Container"))) {
          if ((vmode & 1) && ind->Skill(prhash(u8"Locked"))) {
            std::u8string mes = base + u8"  It is closed and locked, you can't see inside.\n";
            m->Send(mes.c_str());
          } else if (vmode & 1) {
            std::u8string tmp = base;
            base += u8"  ";
            ind->SendContents(m, o, vmode);
            base = tmp;
          }
        }
      }
    }
  if (!b.empty())
    base = u8"";
}

void Object::SendShortDesc(Mind* m, Object* o) {
  memset(buf, 0, 65536);
  sprintf(buf, u8"%s\n", ShortDescC());
  m->Send(buf);
}

void Object::SendFullSituation(Mind* m, Object* o) {
  std::u8string pname = u8"its";
  if (parent && parent->Gender() == 'M')
    pname = u8"his";
  else if (parent && parent->Gender() == 'F')
    pname = u8"her";

  if (Skill(prhash(u8"Quantity")) > 1) {
    m->Send(u8"(x{}) ", Skill(prhash(u8"Quantity")));
  }

  if (!parent)
    sprintf(buf, u8"%s is here%c", Noun().c_str(), 0);

  else if (parent->ActTarg(act_t::HOLD) == this)
    sprintf(buf, u8"%s is here in %s off-hand%c", Noun().c_str(), pname.c_str(), 0);

  else if (parent->ActTarg(act_t::WIELD) == this)
    sprintf(buf, u8"%s is here in %s hand%c", Noun().c_str(), pname.c_str(), 0);

  else if (parent->ActTarg(act_t::WEAR_BACK) == this)
    sprintf(buf, u8"%s is here on %s back%c", Noun().c_str(), pname.c_str(), 0);

  else if (parent->ActTarg(act_t::WEAR_CHEST) == this)
    sprintf(buf, u8"%s is here on %s chest%c", Noun().c_str(), pname.c_str(), 0);

  else if (parent->ActTarg(act_t::WEAR_HEAD) == this)
    sprintf(buf, u8"%s is here on %s head%c", Noun().c_str(), pname.c_str(), 0);

  else if (parent->ActTarg(act_t::WEAR_FACE) == this)
    sprintf(buf, u8"%s is here on %s face%c", Noun().c_str(), pname.c_str(), 0);

  else if (parent->ActTarg(act_t::WEAR_NECK) == this)
    sprintf(buf, u8"%s is here around %s neck%c", Noun().c_str(), pname.c_str(), 0);

  else if (parent->ActTarg(act_t::WEAR_COLLAR) == this)
    sprintf(buf, u8"%s is here on %s neck%c", Noun().c_str(), pname.c_str(), 0);

  else if (parent->ActTarg(act_t::WEAR_WAIST) == this)
    sprintf(buf, u8"%s is here around %s waist%c", Noun().c_str(), pname.c_str(), 0);

  else if (parent->ActTarg(act_t::WEAR_SHIELD) == this)
    sprintf(buf, u8"%s is here on %s shield-arm%c", Noun().c_str(), pname.c_str(), 0);

  else if (parent->ActTarg(act_t::WEAR_LARM) == this)
    sprintf(buf, u8"%s is here on %s left arm%c", Noun().c_str(), pname.c_str(), 0);

  else if (parent->ActTarg(act_t::WEAR_RARM) == this)
    sprintf(buf, u8"%s is here on %s right arm%c", Noun().c_str(), pname.c_str(), 0);

  else if (parent->ActTarg(act_t::WEAR_LFINGER) == this)
    sprintf(buf, u8"%s is here on %s left finger%c", Noun().c_str(), pname.c_str(), 0);

  else if (parent->ActTarg(act_t::WEAR_RFINGER) == this)
    sprintf(buf, u8"%s is here on %s right finger%c", Noun().c_str(), pname.c_str(), 0);

  else if (parent->ActTarg(act_t::WEAR_LFOOT) == this)
    sprintf(buf, u8"%s is here on %s left foot%c", Noun().c_str(), pname.c_str(), 0);

  else if (parent->ActTarg(act_t::WEAR_RFOOT) == this)
    sprintf(buf, u8"%s is here on %s right foot%c", Noun().c_str(), pname.c_str(), 0);

  else if (parent->ActTarg(act_t::WEAR_LHAND) == this)
    sprintf(buf, u8"%s is here on %s left hand%c", Noun().c_str(), pname.c_str(), 0);

  else if (parent->ActTarg(act_t::WEAR_RHAND) == this)
    sprintf(buf, u8"%s is here on %s right hand%c", Noun().c_str(), pname.c_str(), 0);

  else if (parent->ActTarg(act_t::WEAR_LLEG) == this)
    sprintf(buf, u8"%s is here on %s left leg%c", Noun().c_str(), pname.c_str(), 0);

  else if (parent->ActTarg(act_t::WEAR_RLEG) == this)
    sprintf(buf, u8"%s is here on %s right leg%c", Noun().c_str(), pname.c_str(), 0);

  else if (parent->ActTarg(act_t::WEAR_LWRIST) == this)
    sprintf(buf, u8"%s is here on %s left wrist%c", Noun().c_str(), pname.c_str(), 0);

  else if (parent->ActTarg(act_t::WEAR_RWRIST) == this)
    sprintf(buf, u8"%s is here on %s right wrist%c", Noun().c_str(), pname.c_str(), 0);

  else if (parent->ActTarg(act_t::WEAR_LSHOULDER) == this)
    sprintf(buf, u8"%s is here on %s left shoulder%c", Noun().c_str(), pname.c_str(), 0);

  else if (parent->ActTarg(act_t::WEAR_RSHOULDER) == this)
    sprintf(buf, u8"%s is here on %s right shoulder%c", Noun().c_str(), pname.c_str(), 0);

  else if (parent->ActTarg(act_t::WEAR_LHIP) == this)
    sprintf(buf, u8"%s is here on %s left hip%c", Noun().c_str(), pname.c_str(), 0);

  else if (parent->ActTarg(act_t::WEAR_RHIP) == this)
    sprintf(buf, u8"%s is here on %s right hip%c", Noun().c_str(), pname.c_str(), 0);

  else {
    pname = parent->Noun();
    sprintf(
        buf,
        u8"%s %s in %s%c",
        Noun(false, true, m->Body()).c_str(),
        PosString().c_str(),
        pname.c_str(),
        0);
  }

  buf[0] = ascii_toupper(buf[0]);
  m->Send(buf);
}

void Object::SendDesc(Mind* m, Object* o) {
  memset(buf, 0, 65536);

  if (pos != pos_t::NONE) {
    m->Send(CCYN);
    SendFullSituation(m, o);
    SendActions(m);
  } else {
    m->Send(CCYN);
    sprintf(buf, u8"%s\n%c", ShortDescC(), 0);
    buf[0] = ascii_toupper(buf[0]);
    m->Send(buf);
  }

  m->Send(u8"{}   ", CNRM);
  sprintf(buf, u8"%s\n%c", DescC(), 0);
  buf[0] = ascii_toupper(buf[0]);
  m->Send(buf);
  m->Send(CNRM);
}

void Object::SendDescSurround(Mind* m, Object* o, int vmode) {
  if (no_seek)
    return;
  memset(buf, 0, 65536);

  if (pos != pos_t::NONE) {
    m->Send(CCYN);
    SendFullSituation(m, o);
    SendActions(m);
  } else {
    m->Send(CCYN);
    sprintf(buf, u8"%s\n%c", ShortDescC(), 0);
    buf[0] = ascii_toupper(buf[0]);
    m->Send(buf);
  }

  m->Send(u8"{}   ", CNRM);
  sprintf(buf, u8"%s\n%c", DescC(), 0);
  buf[0] = ascii_toupper(buf[0]);
  m->Send(buf);

  m->Send(CNRM);
  SendExtendedActions(m, vmode);

  if ((!parent) || Contains(o) || Skill(prhash(u8"Open")) || Skill(prhash(u8"Transparent"))) {
    SendContents(m, o, vmode);
  }

  if (parent && (Skill(prhash(u8"Open")) || Skill(prhash(u8"Transparent")))) {
    m->Send(CCYN);
    m->Send(u8"Outside you see: ");
    no_seek = true;
    parent->SendDescSurround(m, this, vmode);
    no_seek = false;
  }

  m->Send(CNRM);
}

void Object::SendLongDesc(Mind* m, Object* o) {
  if (pos != pos_t::NONE) {
    m->Send(CCYN);
    SendFullSituation(m, o);
    SendActions(m);
  } else {
    m->Send(CCYN);
    sprintf(buf, u8"%s\n%c", ShortDescC(), 0);
    buf[0] = ascii_toupper(buf[0]);
    m->Send(buf);
  }

  m->Send(u8"{}   ", CNRM);
  sprintf(buf, u8"%s\n%c", LongDescC(), 0);
  buf[0] = ascii_toupper(buf[0]);
  m->Send(buf);
  m->Send(CNRM);
}

static const std::u8string atnames[] = {u8"Bod", u8"Qui", u8"Str", u8"Cha", u8"Int", u8"Wil"};
void Object::SendScore(Mind* m, Object* o) {
  if (!m)
    return;
  m->Send(u8"\n{}", CNRM);
  for (int ctr = 0; ctr < 6; ++ctr) {
    if (std::min(NormAttribute(ctr), 99) == std::min(ModAttribute(ctr), 99)) {
      m->Send(u8"{}: {:>2}     ", atnames[ctr], std::min(ModAttribute(ctr), 99));
    } else if (ModAttribute(ctr) > 9) { // 2-Digits!
      m->Send(
          u8"{}: {:>2} ({})",
          atnames[ctr],
          std::min(NormAttribute(ctr), 99),
          std::min(ModAttribute(ctr), 99));
    } else { // 1 Digit!
      m->Send(
          u8"{}: {:>2} ({}) ",
          atnames[ctr],
          std::min(NormAttribute(ctr), 99),
          std::min(ModAttribute(ctr), 99));
    }
    if (ctr == 0) {
      m->Send(u8"         L     M        S           D");
    } else if (ctr == 1) {
      m->Send(
          u8"  Stun: [{}][{}][{}][{}][{}][{}][{}][{}][{}][{}]",
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
      m->Send(
          u8"  Phys: [{}][{}][{}][{}][{}][{}][{}][{}][{}][{}]",
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
        m->Send(u8" Overflow: {}", phys - 10);
      }
    } else if (ctr == 3) {
      m->Send(
          u8"  Stru: [{}][{}][{}][{}][{}][{}][{}][{}][{}][{}]",
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
      if (Pos() == pos_t::NONE) {
        m->Send(u8"  Zone Coords: ({},{},{})  Value: {}Y\n", X(), Y(), Z(), value);
      } else {
        m->Send(
            u8"  Sex: {}, {}.{:03}kg, {}.{:03}m, {}v, {}Y\n",
            static_cast<char8_t>(gender),
            weight / 1000,
            weight % 1000,
            size / 1000,
            size % 1000,
            volume,
            value);
      }
    }
    m->Send(u8"\n");
  }

  std::vector<std::u8string> col1;
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
      m->Send(u8"{:>41} ", *c1); // Note: 41 is 32 (2 Color Escape Codes)
      ++c1;
    } else {
      m->Send(u8"{:>32} ", u8"");
    }

    if (c2 != skls.end()) {
      m->Send(u8"{:>28}: {:>8}", SkillName(c2->first), c2->second);
      c2++;
      if (c2 != skls.end()) {
        c2 = std::find_if(c2, skls.end(), [](auto skl) { return !is_skill(skl.first); });
      }
    }

    m->Send(u8"\n");
  }

  for (act_t actt = act_t::MAX; actt < act_t::SPECIAL_MAX; actt = act_t(int(actt) + 1)) {
    if (ActTarg(actt)) {
      m->Send(
          CGRN u8"  {} -> {}\n" CNRM, act_str[static_cast<uint8_t>(actt)], ActTarg(actt)->Noun());
    } else if (IsAct(actt)) {
      m->Send(CGRN u8"  {}\n" CNRM, act_str[static_cast<uint8_t>(actt)]);
    }
  }

  if (IsActive())
    m->Send(CCYN u8"  ACTIVE\n" CNRM);

  for (auto mind : minds) {
    if (mind->Owner()) {
      m->Send(CBLU u8"->Player Connected: {}\n" CNRM, mind->Owner()->Name());
    } else if (mind == get_mob_mind()) {
      m->Send(CBLU u8"->MOB_MIND\n" CNRM);
    } else if (mind == get_tba_mob_mind()) {
      m->Send(CBLU u8"->TBA_MOB_MIND\n" CNRM);
    } else if (mind->Type() == MIND_TBATRIG) {
      m->Send(CBLU u8"->TBA_TRIGGER\n" CNRM);
    }
  }

  // Other Misc Stats
  if (LightLevel() > 0) {
    m->Send(CYEL u8"  Light Level: {} ({})\n" CNRM, Skill(prhash(u8"Light Source")), LightLevel());
  }
  if (Power(prhash(u8"Cursed"))) {
    m->Send(CRED u8"  Cursed: {}\n" CNRM, Power(prhash(u8"Cursed")));
  }

  // Experience Summary
  if (IsAnimate()) {
    m->Send(CYEL);
    m->Send(
        u8"\nEarned Exp: {:>4}  Unspent Exp: {:>4}  Knowledge: {:>4}\n",
        Exp(),
        TotalExp(),
        known.size());
    if (Power(prhash(u8"Heat Vision")) || Power(prhash(u8"Dark Vision"))) {
      m->Send(
          u8"Heat/Dark Vision: {}/{}\n",
          Power(prhash(u8"Heat Vision")),
          Power(prhash(u8"Dark Vision")));
    }
    m->Send(CNRM);
  }
}

std::vector<std::u8string> Object::FormatSkills(const MinVec<7, skill_pair>& skls) {
  std::vector<std::u8string> ret;

  auto save = skls;
  for (auto skl : save) {
    if (is_skill(skl.first)) {
      char8_t buf2[256];
      sprintf(
          buf2,
          u8"%28s: " CYEL u8"%2d" CNRM,
          SkillName(skl.first).c_str(),
          std::min(99, skl.second));
      ret.push_back(buf2);
    }
  }
  return ret;
}

static void stick_on(
    std::vector<std::u8string>& out,
    const MinVec<7, skill_pair>& skls,
    uint32_t stok,
    const std::u8string label) {
  char8_t buf2[256];

  auto itr =
      std::find_if(skls.begin(), skls.end(), [stok](auto skl) { return (skl.first == stok); });
  if (itr != skls.end()) {
    if (itr->second > 0) {
      sprintf(
          buf2,
          u8"  %18s: " CYEL u8"%d.%.3d" CNRM,
          label.c_str(),
          itr->second / 1000,
          itr->second % 1000);
      out.push_back(buf2);
    }
  }
}

std::vector<std::u8string> Object::FormatStats(const MinVec<7, skill_pair>& skls) {
  std::vector<std::u8string> ret;

  if (HasSkill(prhash(u8"TBAScriptType"))) { // It's a TBA script
    char8_t buf2[256] = {};
    auto type_name = CRED u8"BAD-TYPE" CNRM;
    switch (Skill(prhash(u8"TBAScriptType"))) {
      case (0x1000001): {
        type_name = CYEL u8"MOB-GLOBAL" CNRM;
        break;
      }
      case (0x1000002): {
        type_name = CGRN u8"MOB-RANDOM" CNRM;
        break;
      }
      case (0x1000004): {
        type_name = CGRN u8"MOB-COMMAND" CNRM;
        break;
      }
      case (0x1000008): {
        type_name = CGRN u8"MOB-SPEECH" CNRM;
        break;
      }
      case (0x1000010): {
        type_name = CGRN u8"MOB-ACT" CNRM;
        break;
      }
      case (0x1000020): {
        type_name = CYEL u8"MOB-DEATH" CNRM;
        break;
      }
      case (0x1000040): {
        type_name = CGRN u8"MOB-GREET" CNRM;
        break;
      }
      case (0x1000080): {
        type_name = CYEL u8"MOB-GREET-ALL" CNRM;
        break;
      }
      case (0x1000100): {
        type_name = CYEL u8"MOB-ENTRY" CNRM;
        break;
      }
      case (0x1000200): {
        type_name = CGRN u8"MOB-RECEIVE" CNRM;
        break;
      }
      case (0x1000400): {
        type_name = CYEL u8"MOB-FIGHT" CNRM;
        break;
      }
      case (0x1000800): {
        type_name = CYEL u8"MOB-HITPRCNT" CNRM;
        break;
      }
      case (0x1001000): {
        type_name = CYEL u8"MOB-BRIBE" CNRM;
        break;
      }
      case (0x1002000): {
        type_name = CYEL u8"MOB-LOAD" CNRM;
        break;
      }
      case (0x1004000): {
        type_name = CYEL u8"MOB-MEMORY" CNRM;
        break;
      }
      case (0x1008000): {
        type_name = CYEL u8"MOB-CAST" CNRM;
        break;
      }
      case (0x1010000): {
        type_name = CYEL u8"MOB-LEAVE" CNRM;
        break;
      }
      case (0x1020000): {
        type_name = CYEL u8"MOB-DOOR" CNRM;
        break;
      }
      case (0x1040000): {
        type_name = CYEL u8"MOB-TIME" CNRM;
        break;
      }

      case (0x2000001): {
        type_name = CYEL u8"OBJ-GLOBAL" CNRM;
        break;
      }
      case (0x2000002): {
        type_name = CGRN u8"OBJ-RANDOM" CNRM;
        break;
      }
      case (0x2000004): {
        type_name = CGRN u8"OBJ-COMMAND" CNRM;
        break;
      }
      case (0x2000008): {
        type_name = CYEL u8"OBJ-TIMER" CNRM;
        break;
      }
      case (0x2000010): {
        type_name = CGRN u8"OBJ-GET" CNRM;
        break;
      }
      case (0x2000020): {
        type_name = CGRN u8"OBJ-DROP" CNRM;
        break;
      }
      case (0x2000040): {
        type_name = CYEL u8"OBJ-GIVE" CNRM;
        break;
      }
      case (0x2000080): {
        type_name = CGRN u8"OBJ-WEAR" CNRM;
        break;
      }
      case (0x2000100): {
        type_name = CGRN u8"OBJ-REMOVE" CNRM;
        break;
      }
      case (0x2000200): {
        type_name = CYEL u8"OBJ-LOAD" CNRM;
        break;
      }
      case (0x2000400): {
        type_name = CYEL u8"OBJ-CAST" CNRM;
        break;
      }
      case (0x2000800): {
        type_name = CGRN u8"OBJ-LEAVE" CNRM;
        break;
      }
      case (0x2001000): {
        type_name = CYEL u8"OBJ-CONSUME" CNRM;
        break;
      }

      case (0x4000001): {
        type_name = CYEL u8"ROOM-GLOBAL" CNRM;
        break;
      }
      case (0x4000002): {
        type_name = CGRN u8"ROOM-RANDOM" CNRM;
        break;
      }
      case (0x4000004): {
        type_name = CGRN u8"ROOM-COMMAND" CNRM;
        break;
      }
      case (0x4000008): {
        type_name = CGRN u8"ROOM-SPEECH" CNRM;
        break;
      }
      case (0x4000010): {
        type_name = CYEL u8"ROOM-ZONE" CNRM;
        break;
      }
      case (0x4000020): {
        type_name = CGRN u8"ROOM-ENTER" CNRM;
        break;
      }
      case (0x4000040): {
        type_name = CGRN u8"ROOM-DROP" CNRM;
        break;
      }
      case (0x4000080): {
        type_name = CYEL u8"ROOM-CAST" CNRM;
        break;
      }
      case (0x4000100): {
        type_name = CGRN u8"ROOM-LEAVE" CNRM;
        break;
      }
      case (0x4000200): {
        type_name = CYEL u8"ROOM-DOOR" CNRM;
        break;
      }
      case (0x4000400): {
        type_name = CYEL u8"ROOM-TIME" CNRM;
        break;
      }

      default: {
        type_name = CRED u8"BAD-TYPE" CNRM;
      }
    }
    sprintf(buf2, u8"TBAScriptType: %s", type_name);
    ret.push_back(buf2);
  }
  if (HasSkill(prhash(u8"WeaponType"))) { // It's a Weapon
    // Detailed Weapon Stats
    ret.push_back(
        u8"Weapon: " CYEL + SkillName(get_weapon_skill(Skill(prhash(u8"WeaponType")))) + CNRM);
    stick_on(ret, skls, prhash(u8"Durability"), u8"Durability");
    stick_on(ret, skls, prhash(u8"Hardness"), u8"Hardness");
    stick_on(ret, skls, prhash(u8"Flexibility"), u8"Flexibility");
    stick_on(ret, skls, prhash(u8"Sharpness"), u8"Sharpness");
    stick_on(ret, skls, prhash(u8"Distance"), u8"Pen. Dist");
    stick_on(ret, skls, prhash(u8"Width"), u8"Pen. Width");
    stick_on(ret, skls, prhash(u8"Ratio"), u8"Pen. Ratio");
    stick_on(ret, skls, prhash(u8"Hit Weight"), u8"Hit Weight");
    stick_on(ret, skls, prhash(u8"Velocity"), u8"Velocity");
    stick_on(ret, skls, prhash(u8"Leverage"), u8"Leverage");
    stick_on(ret, skls, prhash(u8"Burn"), u8"Burn");
    stick_on(ret, skls, prhash(u8"Chill"), u8"Chill");
    stick_on(ret, skls, prhash(u8"Zap"), u8"Zap");
    stick_on(ret, skls, prhash(u8"Concuss"), u8"Concuss");
    stick_on(ret, skls, prhash(u8"Flash"), u8"Flash");
    stick_on(ret, skls, prhash(u8"Bang"), u8"Bang");
    stick_on(ret, skls, prhash(u8"Irradiate"), u8"Irradiate");
    stick_on(ret, skls, prhash(u8"Reach"), u8"Reach");
    stick_on(ret, skls, prhash(u8"Range"), u8"Range");
    stick_on(ret, skls, prhash(u8"Strength Required"), u8"Str Req");
    stick_on(ret, skls, prhash(u8"Multiple"), u8"Multiple");

    ret.push_back(CYEL CNRM); // Leave a blank line between new and old
    // Must include color escapes for formatting

    // Old-Style (Shadowrun) Weapon Stats
    static const char8_t* sevs[] = {
        u8"-",
        u8"L",
        u8"M",
        u8"S",
        u8"D",
        u8"D2",
        u8"D4",
        u8"D6",
        u8"D8",
        u8"D10",
        u8"D12",
        u8"D14",
        u8"D16",
        u8"D18",
        u8"D20",
        u8"D20+",
    };
    ret.push_back(
        u8" Old Weapon: " CYEL + SkillName(get_weapon_skill(Skill(prhash(u8"WeaponType")))) + CNRM);

    char8_t buf2[256];
    sprintf(
        buf2,
        u8"  Damage: " CYEL u8"(Str+%d)%s" CNRM,
        Skill(prhash(u8"WeaponForce")),
        sevs[std::max(0, std::min(15, Skill(prhash(u8"WeaponSeverity"))))]);
    ret.push_back(buf2);

    if (Skill(prhash(u8"WeaponReach")) > 4) {
      sprintf(buf2, u8"  Range: " CYEL u8"%d" CNRM, Skill(prhash(u8"WeaponReach")));
      ret.push_back(buf2);
    } else if (Skill(prhash(u8"WeaponReach")) >= 0) {
      sprintf(buf2, u8"  Reach: " CYEL u8"%d" CNRM, Skill(prhash(u8"WeaponReach")));
      ret.push_back(buf2);
    }
  }
  if (HasSkill(prhash(u8"Thickness"))) { // It's Armor (or just Clothing)
    // stick_on(ret, skls, prhash(u8"Coverage"), u8"Coverage");
    stick_on(ret, skls, prhash(u8"Durability"), u8"Durability");
    stick_on(ret, skls, prhash(u8"Hardness"), u8"Hardness");
    stick_on(ret, skls, prhash(u8"Flexibility"), u8"Flexibility");
    stick_on(ret, skls, prhash(u8"Sharpness"), u8"Sharpness");
    stick_on(ret, skls, prhash(u8"Thickness"), u8"Thickness");
    stick_on(ret, skls, prhash(u8"Max Gap"), u8"Max Gap");
    stick_on(ret, skls, prhash(u8"Min Gap"), u8"Min Gap");
    stick_on(ret, skls, prhash(u8"Hit Weight"), u8"Hit Weight");
    stick_on(ret, skls, prhash(u8"Ballistic"), u8"Ballistic");
    stick_on(ret, skls, prhash(u8"Bulk"), u8"Bulk");
    stick_on(ret, skls, prhash(u8"Warm"), u8"Warm");
    stick_on(ret, skls, prhash(u8"Reflect"), u8"Reflect");
    stick_on(ret, skls, prhash(u8"Insulate"), u8"Insulate");
    stick_on(ret, skls, prhash(u8"Padding"), u8"Padding");
    stick_on(ret, skls, prhash(u8"Shade"), u8"Shade");
    stick_on(ret, skls, prhash(u8"Muffle"), u8"Muffle");
    stick_on(ret, skls, prhash(u8"Shielding"), u8"Shielding");
    stick_on(ret, skls, prhash(u8"Defense Range"), u8"Def Range");
    stick_on(ret, skls, prhash(u8"Strength Required"), u8"Str Req");
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
    const std::u8string& name,
    const std::u8string& dsc,
    const std::u8string& oname,
    const std::u8string& odsc) {
  Object* door1 = new Object(this);
  Object* door2 = new Object(other);
  door1->SetShortDesc(name.c_str());
  door2->SetShortDesc(oname.c_str());
  door1->SetDesc(dsc.c_str());
  door2->SetDesc(odsc.c_str());
  door1->AddAct(act_t::SPECIAL_LINKED, door2);
  door1->AddAct(act_t::SPECIAL_MASTER, door2);
  door1->SetSkill(prhash(u8"Open"), 1000);
  door1->SetSkill(prhash(u8"Enterable"), 1);
  door2->AddAct(act_t::SPECIAL_LINKED, door1);
  door2->AddAct(act_t::SPECIAL_MASTER, door1);
  door2->SetSkill(prhash(u8"Open"), 1000);
  door2->SetSkill(prhash(u8"Enterable"), 1);
}

void Object::LinkClosed(
    Object* other,
    const std::u8string& name,
    const std::u8string& dsc,
    const std::u8string& oname,
    const std::u8string& odsc) {
  Object* door1 = new Object(this);
  Object* door2 = new Object(other);
  door1->SetShortDesc(name.c_str());
  door2->SetShortDesc(oname.c_str());
  door1->SetDesc(dsc.c_str());
  door2->SetDesc(odsc.c_str());
  door1->AddAct(act_t::SPECIAL_LINKED, door2);
  door1->AddAct(act_t::SPECIAL_MASTER, door2);
  door1->SetSkill(prhash(u8"Closeable"), 1);
  door1->SetSkill(prhash(u8"Enterable"), 1);
  door1->SetSkill(prhash(u8"Transparent"), 1000);
  door2->AddAct(act_t::SPECIAL_LINKED, door1);
  door2->AddAct(act_t::SPECIAL_MASTER, door1);
  door2->SetSkill(prhash(u8"Closeable"), 1);
  door2->SetSkill(prhash(u8"Enterable"), 1);
  door2->SetSkill(prhash(u8"Transparent"), 1000);
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
      // fprintf(stderr, u8"Combining '%s'\n", Noun().c_str());
      int val;

      val =
          std::max(1, Skill(prhash(u8"Quantity"))) + std::max(1, obj->Skill(prhash(u8"Quantity")));
      SetSkill(prhash(u8"Quantity"), val);

      val = Skill(prhash(u8"Hungry")) + obj->Skill(prhash(u8"Hungry"));
      SetSkill(prhash(u8"Hungry"), val);

      val = Skill(prhash(u8"Bored")) + obj->Skill(prhash(u8"Bored"));
      SetSkill(prhash(u8"Bored"), val);

      val = Skill(prhash(u8"Needy")) + obj->Skill(prhash(u8"Needy"));
      SetSkill(prhash(u8"Needy"), val);

      val = Skill(prhash(u8"Tired")) + obj->Skill(prhash(u8"Tired"));
      SetSkill(prhash(u8"Tired"), val);

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

  int cap = dest->Skill(prhash(u8"Capacity"));
  if (cap > 0) {
    cap -= dest->ContainedVolume();
    if (Volume() > cap)
      return -2;
  }

  int con = dest->Skill(prhash(u8"Container"));
  if (con > 0) {
    con -= dest->ContainedWeight();
    if (Weight() > con)
      return -3;
  }

  std::u8string dir = u8"";
  std::u8string rdir = u8"";
  {
    const std::u8string dirs[6] = {u8"north", u8"south", u8"east", u8"west", u8"up", u8"down"};
    for (int dnum = 0; dnum < 6 && dir[0] == 0; ++dnum) {
      Object* door = parent->PickObject(dirs[dnum], LOC_INTERNAL);
      if (door && door->ActTarg(act_t::SPECIAL_LINKED) &&
          door->ActTarg(act_t::SPECIAL_LINKED)->Parent() &&
          door->ActTarg(act_t::SPECIAL_LINKED)->Parent() == dest) {
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
      if (trig->Skill(prhash(u8"TBAScriptType")) & 0x0010000) {
        if ((rand() % 100) < trig->Skill(prhash(u8"TBAScriptNArg"))) { // % Chance
          // fprintf(stderr, CBLU u8"Triggering: %s\n" CNRM, trig->Noun().c_str());
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

  StopAct(act_t::POINT);
  StopAct(act_t::FOLLOW);
  if (IsAct(act_t::HOLD) && ActTarg(act_t::HOLD)->Parent() != this) { // Dragging
    if (ActTarg(act_t::HOLD)->Parent() != Parent()) { // Didn't come with me!
      StopAct(act_t::HOLD);
    }
  }
  SetSkill(prhash(u8"Hidden"), 0);

  if (parent->Skill(prhash(u8"DynamicInit")) > 0) { // Room is dynamic, but uninitialized
    parent->DynamicInit();
  }

  if (parent->Skill(prhash(u8"Accomplishment"))) {
    for (auto m : minds) {
      if (m->Owner()) {
        Accomplish(parent->Skill(prhash(u8"Accomplishment")), u8"finding a secret");
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
      if ((trig->Skill(prhash(u8"TBAScriptType")) & 0x0000040) &&
          (trig->Skill(prhash(u8"TBAScriptType")) & 0x5000000)) {
        if (trig != this && trig->Parent() != this) {
          if ((rand() % 100) < 1000 * trig->Skill(prhash(u8"TBAScriptNArg"))) { // % Chance
            // if (trig->Skill(prhash(u8"TBAScript")) >= 5034503 &&
            // trig->Skill(prhash(u8"TBAScript"))
            // <= 5034507)
            //  fprintf(stderr, CBLU u8"Triggering: %s\n" CNRM, trig->Noun().c_str());
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

  if (descriptions != default_descriptions) {
    delete[] descriptions;
  }
  if (dowhenfree[0] != '\0') {
    delete[] dowhenfree;
  }
  if (defact[0] != '\0') {
    delete[] defact;
  }
}

void Object::Recycle(int inbin) {
  Deactivate();

  // fprintf(stderr, u8"Deleting: %s\n", Noun(0));

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
    while ((!dest->ActTarg(act_t::SPECIAL_HOME)) && dest->Parent()) {
      dest = dest->Parent();
    }
    if (dest->ActTarg(act_t::SPECIAL_HOME)) {
      dest = dest->ActTarg(act_t::SPECIAL_HOME);
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
  if (ActTarg(act_t::SPECIAL_MASTER))
    tonotify.insert(ActTarg(act_t::SPECIAL_MASTER));
  if (ActTarg(act_t::SPECIAL_MONITOR))
    tonotify.insert(ActTarg(act_t::SPECIAL_MONITOR));
  if (ActTarg(act_t::SPECIAL_LINKED))
    tonotify.insert(ActTarg(act_t::SPECIAL_LINKED));

  auto touches = Touching();
  for (auto touch : touches) {
    tonotify.insert(touch);
  }

  StopAct(act_t::SPECIAL_MASTER);
  StopAct(act_t::SPECIAL_MONITOR);
  StopAct(act_t::SPECIAL_LINKED);

  for (auto noti : tonotify) {
    int del = 0;
    if (noti->ActTarg(act_t::SPECIAL_MASTER) == this)
      del = 1;
    else if (noti->ActTarg(act_t::SPECIAL_LINKED) == this)
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

  // fprintf(stderr, u8"Done deleting: %s\n", Noun(0));
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

static int get_ordinal(const std::u8string& t) {
  int ret = 0;

  std::u8string_view text = t;
  if (isdigit(text[0])) {
    text.remove_prefix(std::min(text.find_first_not_of(u8"0123456789"), text.size()));
    if (text[0] == '.') {
      ret = atoi(t.c_str());
    } else if (!isgraph(text[0])) {
      ret = -atoi(t.c_str());
    } else if (text.length() >= 3 && isgraph(text[2])) {
      if (text.substr(0, 2) == u8"st") {
        ret = atoi(t.c_str());
      } else if (text.substr(0, 2) == u8"nd") {
        ret = atoi(t.c_str());
      } else if (text.substr(0, 2) == u8"rd") {
        ret = atoi(t.c_str());
      } else if (text.substr(0, 2) == u8"th") {
        ret = atoi(t.c_str());
      }
    }
  } else if (text.substr(0, 6) == u8"first ") {
    ret = 1;
  } else if (text.substr(0, 7) == u8"second ") {
    ret = 2;
  } else if (text.substr(0, 6) == u8"third ") {
    ret = 3;
  } else if (text.substr(0, 7) == u8"fourth ") {
    ret = 4;
  } else if (text.substr(0, 6) == u8"fifth ") {
    ret = 5;
  } else if (text.substr(0, 6) == u8"sixth ") {
    ret = 6;
  } else if (text.substr(0, 8) == u8"seventh ") {
    ret = 7;
  } else if (text.substr(0, 7) == u8"eighth ") {
    ret = 8;
  } else if (text.substr(0, 6) == u8"ninth ") {
    ret = 9;
  } else if (text.substr(0, 6) == u8"tenth ") {
    ret = 10;
  } else if (text.substr(0, 4) == u8"all ") {
    ret = ALL;
  } else if (text.substr(0, 4) == u8"all.") {
    ret = ALL;
  } else if (text.substr(0, 5) == u8"some ") {
    ret = SOME;
  } else if (text.substr(0, 5) == u8"some.") {
    ret = SOME;
  }
  return ret;
}

static int strip_ordinal(std::u8string& text) {
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

Object* Object::PickObject(const std::u8string& name, int loc, int* ordinal) const {
  auto ret = PickObjects(name, loc, ordinal);
  if (ret.size() != 1) {
    return nullptr;
  }
  return ret.front();
}

uint32_t splits[4] = {prhash(u8"Hungry"), prhash(u8"Bored"), prhash(u8"Tired"), prhash(u8"Needy")};
Object* Object::Split(int nqty) {
  if (nqty < 1)
    nqty = 1;
  int qty = Skill(prhash(u8"Quantity")) - nqty;
  if (qty < 1)
    qty = 1;

  Object* nobj = new Object(*this);
  nobj->SetParent(Parent());
  nobj->SetSkill(prhash(u8"Quantity"), (nqty <= 1) ? 0 : nqty);

  SetSkill(prhash(u8"Quantity"), (qty <= 1) ? 0 : qty);

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
  if (obj->Skill(prhash(u8"Invisible")) > 999 && (vmode & LOC_NINJA) == 0)
    return 0;

  // Need Heat Vision to see these
  if (obj->HasSkill(prhash(u8"Invisible")) && (vmode & (LOC_NINJA | LOC_HEAT)) == 0)
    return 0;

  // Can't be seen/affected (except in char8_t rooms)
  if (obj->Skill(prhash(u8"Hidden")) > 0 && (vmode & (LOC_NINJA | 1)) == 0)
    return 0;
  Object* nobj = nullptr;

  int cqty = 1, rqty = 1; // Contains / Requires

  if (obj->Skill(prhash(u8"Quantity")))
    cqty = obj->Skill(prhash(u8"Quantity"));

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

MinVec<1, Object*> Object::PickObjects(std::u8string name, int loc, int* ordinal) const {
  MinVec<1, Object*> ret;

  trim_string(name);
  uint32_t ntok = crc32c(name);

  int ordcontainer;
  if (ordinal)
    strip_ordinal(name);
  else {
    ordinal = &ordcontainer;
    (*ordinal) = strip_ordinal(name);
  }
  if (ntok == crc32c(u8"all"))
    (*ordinal) = ALL;
  if (ntok == crc32c(u8"everyone"))
    (*ordinal) = ALL;
  if (ntok == crc32c(u8"everything"))
    (*ordinal) = ALL;
  if (ntok == crc32c(u8"everywhere"))
    (*ordinal) = ALL;
  if (!(*ordinal))
    (*ordinal) = 1;

  auto poss = std::min(name.find(u8"'s "), name.find(u8"'S "));
  if (poss != std::u8string::npos) {
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
    if ((ntok == crc32c(u8"self")) || (ntok == crc32c(u8"myself")) || (ntok == crc32c(u8"me"))) {
      if ((*ordinal) != 1)
        return ret;
      ret.push_back(const_cast<Object*>(this)); // Wrecks Const-Ness
      return ret;
    }
  }

  if (loc & LOC_HERE) {
    if (ntok == crc32c(u8"here")) {
      if ((*ordinal) == 1 && parent)
        ret.push_back(parent);
      return ret;
    }
  }

  if (loc & LOC_INTERNAL) {
    if (!strncmp(name.c_str(), u8"my ", 3)) {
      return PickObjects(name.substr(3), loc & (LOC_SPECIAL | LOC_INTERNAL | LOC_SELF));
    }
  }

  if ((loc & LOC_NEARBY) && (parent != nullptr)) {
    auto cont = parent->Contents(loc); //"loc" includes vmode.

    for (auto ind : cont)
      if (!ind->no_seek) {
        if (ind == this)
          continue; // Must use u8"self" to pick self!
        if (ind->Filter(loc) && ind->Matches(name, (loc & LOC_NINJA) || Knows(ind))) {
          if (tag(ind, ret, ordinal, (parent->Parent() == nullptr) | (loc & LOC_SPECIAL))) {
            return ret;
          }
        }
        if (ind->Skill(prhash(u8"Open")) || ind->Skill(prhash(u8"Transparent"))) {
          auto add = ind->PickObjects(name, (loc & LOC_SPECIAL) | LOC_INTERNAL, ordinal);
          ret.insert(ret.end(), add.begin(), add.end());

          if ((*ordinal) == 0)
            return ret;
        }
      }
    if (parent->Skill(prhash(u8"Open")) || parent->Skill(prhash(u8"Transparent"))) {
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
        if (action.obj()->Filter(loc) &&
            action.obj()->Matches(name, (loc & LOC_NINJA) || Knows(action.obj())) &&
            ((loc & LOC_NOTWORN) == 0 || action.act() <= act_t::HOLD) &&
            ((loc & LOC_NOTUNWORN) == 0 || action.act() >= act_t::HOLD)) {
          if (tag(action.obj(), ret, ordinal, (Parent() == nullptr) | (loc & LOC_SPECIAL))) {
            return ret;
          }
        }
        if (action.obj()->HasSkill(prhash(u8"Container"))) {
          auto add = action.obj()->PickObjects(name, (loc & LOC_SPECIAL) | LOC_INTERNAL, ordinal);
          ret.insert(ret.end(), add.begin(), add.end());

          if ((*ordinal) == 0)
            return ret;
        }
      }
    }

    for (auto ind : cont) {
      if (ind == this)
        continue; // Must use u8"self" to pick self!
      if (ind->Filter(loc) && ind->Matches(name, (loc & LOC_NINJA) || Knows(ind))) {
        if (tag(ind, ret, ordinal, (Parent() == nullptr) | (loc & LOC_SPECIAL))) {
          return ret;
        }
      }
      if (ind->Skill(prhash(u8"Container")) && (loc & LOC_NOTUNWORN) == 0) {
        auto add = ind->PickObjects(name, (loc & LOC_SPECIAL) | LOC_INTERNAL, ordinal);
        ret.insert(ret.end(), add.begin(), add.end());

        if ((*ordinal) == 0)
          return ret;
      }
    }
  }
  return ret;
}

int Object::HasWithin(const Object* obj) const {
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

int Object::SeeWithin(const Object* obj) const {
  if (no_seek)
    return 0;
  for (auto ind : contents) {
    if (ind == obj)
      return 1;
    if (ind->Skill(prhash(u8"Open")) || ind->Skill(prhash(u8"Transparent"))) {
      if (ind->SeeWithin(obj))
        return 1;
    }
  }
  return 0;
}

int Object::IsNearBy(const Object* obj) const {
  if (no_seek || (!parent))
    return 0;
  for (auto ind : parent->contents) {
    if (ind == obj)
      return 1;
    if (ind == this)
      continue; // Not Nearby Self
    if (ind->Skill(prhash(u8"Open")) || ind->Skill(prhash(u8"Transparent"))) {
      int ret = ind->SeeWithin(obj);
      if (ret)
        return ret;
    }
  }
  if (parent->parent &&
      (parent->Skill(prhash(u8"Open")) || parent->Skill(prhash(u8"Transparent")))) {
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
    if (curact.obj() && curact.act() < act_t::MAX &&
        (curact.obj() == obj || obj->HasWithin(curact.obj()))) {
      if (curact.act() != act_t::FOLLOW || (!newloc)) {
        stops.insert(curact.act());
      } else if (parent != newloc) { // Do nothing if we didn't leave!
        following = 1; // Run Follow Response AFTER loop!
      }
    }
    if (curact.act() >= act_t::MAX && (!newloc) && curact.obj() == obj) {
      for (auto curact2 : obj->act) {
        if (curact2.act() >= act_t::MAX) {
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
    if (IsUsing(prhash(u8"Stealth")) && Skill(prhash(u8"Stealth")) > 0) {
      stealth_t = Skill(prhash(u8"Stealth"));
      stealth_s = Roll(prhash(u8"Stealth"), 2);
    }
    parent->SendOut(stealth_t, stealth_s, u8";s follows ;s.\n", u8"You follow ;s.\n", this, obj);
    Travel(newloc);
    parent->SendOut(stealth_t, stealth_s, u8";s follows ;s.\n", u8"", this, obj);
    AddAct(act_t::FOLLOW, obj); // Needed since Travel Kills Follow Act
    if (stealth_t > 0) {
      SetSkill(prhash(u8"Hidden"), Roll(prhash(u8"Stealth"), 2) * 2);
    }
  }

  for (const auto& stop : stops) {
    StopAct(stop);
  }
  for (const auto& stop : stops2) {
    obj->StopAct(stop);
  }

  if (obj->ActTarg(act_t::HOLD) == this) { // Dragging
    if (parent != newloc) { // Actually went somewhere
      parent->SendOut(
          ALL, -1, u8";s drags ;s along.\n", u8"You drag ;s along with you.\n", obj, this);
      Travel(newloc, 0);
      parent->SendOut(ALL, -1, u8";s drags ;s along.\n", u8"", obj, this);
    }
  }
}

void Object::NotifyGone(Object* obj, Object* newloc, int up) {
  if (obj == this)
    return; // Never notify self or sub-self objects.

  // Climb to top first!
  if (up == 1 && parent && (Skill(prhash(u8"Open")) || Skill(prhash(u8"Transparent")))) {
    parent->NotifyGone(obj, newloc, 1);
    return;
  }

  NotifyLeft(obj, newloc);

  std::map<Object*, int> tonotify;

  for (const auto& ind : contents) {
    if (up >= 0) {
      tonotify[ind] = -1;
    } else if (ind->Skill(prhash(u8"Open")) || ind->Skill(prhash(u8"Transparent"))) {
      tonotify[ind] = 0;
    }
  }

  for (auto noti : tonotify) {
    noti.first->NotifyGone(obj, newloc, noti.second);
  }
}

void Object::AddAct(act_t a, Object* o) {
  StopAct(a);
  act.push_back(act_pair(a, o));
  if (o) {
    o->NowTouching(this);
  }
}

void Object::StopAct(act_t a) {
  auto itr = act.begin();
  for (; itr != act.end() && itr->act() != a; ++itr) {
  }
  if (itr != act.end()) {
    Object* obj = itr->obj();
    act.erase(itr);
    if (a == act_t::HOLD && IsAct(act_t::OFFER)) {
      // obj->SendOut(0, 0, u8";s stops offering.\n", u8"", obj, nullptr);
      StopAct(act_t::OFFER);
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
    if (opt.act() != act_t::SPECIAL_ACTEE) {
      StopAct(opt.act());
    }
  }
}

void Object::Collapse() {
  StopAct(act_t::DEAD);
  StopAct(act_t::DYING);
  StopAct(act_t::UNCONSCIOUS);
  StopAct(act_t::SLEEP);
  StopAct(act_t::REST);
  StopAct(act_t::FOLLOW);
  StopAct(act_t::POINT);
  StopAct(act_t::FIGHT);
  if (parent) {
    if (pos != pos_t::LIE) {
      parent->SendOut(ALL, -1, u8";s collapses!\n", u8"You collapse!\n", this, nullptr);
      pos = pos_t::LIE;
    }
    if (ActTarg(act_t::WIELD)) {
      parent->SendOut(
          ALL,
          -1,
          u8";s drops {}!\n",
          u8"You drop {}!\n",
          this,
          nullptr,
          ActTarg(act_t::WIELD)->ShortDescC());
      ActTarg(act_t::WIELD)->Travel(parent);
    }
    if (ActTarg(act_t::HOLD) && ActTarg(act_t::HOLD) != ActTarg(act_t::WEAR_SHIELD)) {
      parent->SendOut(
          ALL,
          -1,
          u8";s drops {}!\n",
          u8"You drop {}!\n",
          this,
          nullptr,
          ActTarg(act_t::HOLD)->ShortDescC());
      ActTarg(act_t::HOLD)->Travel(parent);
    } else if (ActTarg(act_t::HOLD)) {
      parent->SendOut(
          ALL,
          -1,
          u8";s stops holding {}.\n",
          u8"You stop holding {}!\n",
          this,
          nullptr,
          ActTarg(act_t::HOLD)->ShortDescC());
      StopAct(act_t::HOLD);
    }
  }
}

void Object::UpdateDamage() {
  if (stun > 10) { // Overflow
    phys += stun - 10;
    stun = 10;
  }
  if (stun < Skill(prhash(u8"Hungry")) / 500000) { // Hungry Stuns
    stun = Skill(prhash(u8"Hungry")) / 500000;
  }
  if (phys < Skill(prhash(u8"Thirsty")) / 50000) { // Thirsty Wounds
    phys = Skill(prhash(u8"Thirsty")) / 50000;
  }
  if (phys > 10 + ModAttribute(2)) {
    phys = 10 + ModAttribute(2) + 1;

    if (IsAct(act_t::DEAD) == 0) {
      parent->SendOut(
          ALL, -1, u8";s expires from its wounds.\n", u8"You expire, sorry.\n", this, nullptr);
      stun = 10;
      Collapse();
      AddAct(act_t::DEAD);
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
    SetPos(pos_t::LIE);
  } else if (phys >= 10) {
    if (IsAct(act_t::DYING) + IsAct(act_t::DEAD) == 0) {
      parent->SendOut(
          ALL,
          -1,
          u8";s collapses, bleeding and dying!\n",
          u8"You collapse, bleeding and dying!\n",
          this,
          nullptr);
      stun = 10;
      Collapse();
      AddAct(act_t::DYING);
    } else if (IsAct(act_t::DEAD) == 0) {
      parent->SendOut(
          ALL, -1, u8";s isn't quite dead yet!\n", u8"You aren't quite dead yet!\n", this, nullptr);
      StopAct(act_t::DEAD);
      AddAct(act_t::DYING);
    }
    SetPos(pos_t::LIE);
  } else if (stun >= 10) {
    if (IsAct(act_t::UNCONSCIOUS) + IsAct(act_t::DYING) + IsAct(act_t::DEAD) == 0) {
      parent->SendOut(
          ALL, -1, u8";s falls unconscious!\n", u8"You fall unconscious!\n", this, nullptr);
      Collapse();
      AddAct(act_t::UNCONSCIOUS);
    } else if (IsAct(act_t::DEAD) + IsAct(act_t::DYING) != 0) {
      parent->SendOut(
          ALL,
          -1,
          u8";s isn't dead, just out cold.\n",
          u8"You aren't dead, just unconscious.",
          this,
          nullptr);
      StopAct(act_t::DEAD);
      StopAct(act_t::DYING);
      AddAct(act_t::UNCONSCIOUS);
    }
    SetPos(pos_t::LIE);
  } else if (stun > 0) {
    if (IsAct(act_t::DEAD) + IsAct(act_t::DYING) + IsAct(act_t::UNCONSCIOUS) != 0) {
      parent->SendOut(
          ALL,
          -1,
          u8";s wakes up, a little groggy.\n",
          u8"You wake up, a little groggy.",
          this,
          nullptr);
      StopAct(act_t::DEAD);
      StopAct(act_t::DYING);
      StopAct(act_t::UNCONSCIOUS);
    }
  } else {
    if (IsAct(act_t::DEAD) + IsAct(act_t::DYING) + IsAct(act_t::UNCONSCIOUS) != 0) {
      parent->SendOut(
          ALL,
          -1,
          u8";s wakes up, feeling fine!\n",
          u8"You wake up, feeling fine!\n",
          this,
          nullptr);
      StopAct(act_t::DEAD);
      StopAct(act_t::DYING);
      StopAct(act_t::UNCONSCIOUS);
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
  succ -= roll(ModAttribute(0) + Modifier(u8"Resilience"), force);
  sev *= 2;
  sev += succ;
  for (int ctr = 0; ctr <= (sev / 2) && ctr <= 4; ++ctr)
    stun += ctr;
  if (sev > 8)
    stun += (sev - 8);
  if (stun > 10)
    stun = 10; // No Overflow to Phys from u8"Ment"
  UpdateDamage();
  return sev;
}

int Object::HitStun(int force, int sev, int succ) {
  if (!IsAnimate())
    return 0;
  succ -= roll(ModAttribute(0) + Modifier(u8"Resilience"), force);
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
  succ -= roll(ModAttribute(0) + Modifier(u8"Resilience"), force);
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
  succ -= roll(ModAttribute(0) + Modifier(u8"Resilience"), force);
  sev *= 2;
  sev += succ;
  for (int ctr = 0; ctr <= (sev / 2) && ctr <= 4; ++ctr)
    stru += ctr;
  if (sev > 8)
    stru += (sev - 8);
  UpdateDamage();
  return sev;
}

void Object::Send(int tnum, int rsucc, const std::u8string& mes) {
  if (no_hear)
    return;

  if (tnum != ALL && rsucc >= 0 && Roll(prhash(u8"Perception"), tnum) <= rsucc) {
    return;
  }

  auto tosend = mes;
  tosend[0] = ascii_toupper(tosend[0]);

  for (auto mind : minds) {
    Object* body = mind->Body();
    mind->Attach(this);
    mind->Send(tosend);
    mind->Attach(body);
  }
}

void Object::Send(channel_t channel, const std::u8string& mes) {
  auto tosend = mes;
  tosend[0] = ascii_toupper(tosend[0]);

  for (auto mind : minds) {
    if (channel == CHANNEL_ROLLS && mind->IsSVar(u8"combatinfo")) {
      Object* body = mind->Body();
      mind->Attach(this);
      mind->Send(tosend);
      mind->Attach(body);
    }
  }
}

void Object::SendOut(
    int tnum,
    int rsucc,
    const std::u8string& mes,
    const std::u8string& youmes,
    Object* actor,
    Object* targ,
    bool expanding) {
  if (no_seek)
    return;

  if (this != actor) { // Don't trigger yourself!
    for (auto trig : contents) {
      if (strncmp(mes.c_str(), u8";s says '", 9)) {
        // Type 0x1000010 (MOB + MOB-ACT)
        if ((trig->Skill(prhash(u8"TBAScriptType")) & 0x1000010) == 0x1000010) {
          if (trig->Desc()[0] == '*') { // All Actions
            new_trigger((rand() % 400) + 300, trig, actor, mes);
          } else if (trig->Skill(prhash(u8"TBAScriptNArg")) == 0) { // Match Full Phrase
            if (phrase_match(mes, trig->Desc())) {
              new_trigger((rand() % 400) + 300, trig, actor, mes);
            }
          } else { // Match Words
            if (words_match(mes, trig->Desc())) {
              new_trigger((rand() % 400) + 300, trig, actor, mes);
            }
          }
        }
      } else {
        // Type 0x1000008 (MOB + MOB-SPEECH)
        if ((trig->Skill(prhash(u8"TBAScriptType")) & 0x1000008) == 0x1000008) {
          // if (trig->Skill(prhash(u8"TBAScript")) >= 5034503 && trig->Skill(prhash(u8"TBAScript"))
          // <= 5034507)
          //  fprintf(stderr, CBLU u8"[#%d] Got message: '%s'\n" CNRM,
          //  trig->Skill(prhash(u8"TBAScript")), mes);
          std::u8string speech = (mes.c_str() + 9);
          while (!speech.empty() && speech.back() != '\'') {
            speech.pop_back();
          }
          if (!speech.empty())
            speech.pop_back();

          if (trig->Desc()[0] == '*') { // All Speech
            new_trigger((rand() % 400) + 300, trig, actor, speech);
          } else if (trig->Skill(prhash(u8"TBAScriptNArg")) == 0) { // Match Full Phrase
            if (phrase_match(speech, trig->Desc())) {
              // if (trig->Skill(prhash(u8"TBAScript")) >= 5034503 &&
              // trig->Skill(prhash(u8"TBAScript"))
              // <= 5034507)
              //  fprintf(stderr, CBLU u8"Triggering(f): %s\n" CNRM, trig->Noun().c_str());
              new_trigger((rand() % 400) + 300, trig, actor, speech);
            }
          } else { // Match Words
            if (words_match(speech, trig->Desc())) {
              // if (trig->Skill(prhash(u8"TBAScript")) >= 5034503 &&
              // trig->Skill(prhash(u8"TBAScript"))
              // <= 5034507)
              //  fprintf(stderr, CBLU u8"Triggering(w): %s\n" CNRM, trig->Noun().c_str());
              new_trigger((rand() % 400) + 300, trig, actor, speech);
            }
          }

          // Type 0x4000008 (ROOM + ROOM-SPEECH)
        } else if ((trig->Skill(prhash(u8"TBAScriptType")) & 0x4000008) == 0x4000008) {
          std::u8string speech = (mes.c_str() + 9);
          while (!speech.empty() && speech.back() != '\'') {
            speech.pop_back();
          }
          if (!speech.empty())
            speech.pop_back();

          if (trig->Desc()[0] == '*') { // All Speech
            new_trigger((rand() % 400) + 300, trig, actor, speech);
          } else if (trig->Skill(prhash(u8"TBAScriptNArg")) == 0) { // Match Full Phrase
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
  }

  std::u8string tstr = u8"";
  if (targ) {
    tstr = targ->Noun(0, 0, this, actor);
  }
  std::u8string astr = u8"";
  if (actor) {
    astr = actor->Noun(0, 1, this);
  }

  auto str = mes;
  auto youstr = youmes;

  bool just_swapped = false;
  int num_braces[] = {0, 0};
  for (auto& ctr : str) {
    if (just_swapped) {
      ctr = '}';
      just_swapped = false;
      ++num_braces[1];
    } else if (ctr == ';') {
      ctr = '{';
      just_swapped = true;
      ++num_braces[0];
    } else if (ctr == '{' || ctr == '}') {
      ctr = ';';
      just_swapped = false;
    }
  }
  if (num_braces[0] != num_braces[1]) {
    fprintf(stderr, CRED u8"ERROR: Mismatched braces in SendOut() str: '%s'!\n" CNRM, str.c_str());
    return; // Abort sending, this may throw an exception!
  } else if (num_braces[0] > 2) {
    fprintf(
        stderr,
        CRED u8"ERROR: Too many opening braces in SendOut() str: '%s'!\n" CNRM,
        str.c_str());
    return; // Abort sending, this may throw an exception!
  }

  just_swapped = false;
  num_braces[0] = 0;
  num_braces[1] = 0;
  for (auto& ctr : youstr) {
    if (just_swapped) {
      ctr = '}';
      just_swapped = false;
      ++num_braces[1];

    } else if (ctr == ';') {
      ctr = '{';
      just_swapped = true;
      ++num_braces[0];
    } else if (ctr == '{' || ctr == '}') {
      ctr = ';';
      just_swapped = false;
    }
  }
  if (num_braces[0] != num_braces[1]) {
    fprintf(
        stderr,
        CRED u8"ERROR: Mismatched braces in SendOut() youstr: '%s'!\n" CNRM,
        youstr.c_str());
    return; // Abort sending, this may throw an exception!
  } else if (num_braces[0] > 1) {
    fprintf(
        stderr,
        CRED u8"ERROR: Too many opening braces in SendOut() youstr: '%s'!\n" CNRM,
        youstr.c_str());
    return; // Abort sending, this may throw an exception!
  }

  if (youstr[0] == '*' && this == actor) {
    Send(ALL, -1, CGRN);
    Send(ALL, -1, youstr.substr(1), tstr);
    Send(ALL, -1, CNRM);
  } else if (this == actor) {
    Send(ALL, -1, youstr, tstr);
  } else if (str[0] == '*' && targ == this) {
    Send(ALL, -1, CRED);
    Send(tnum, rsucc, str.substr(1), astr, tstr);
    Send(ALL, -1, CNRM);
  } else if (str[0] == '*') {
    Send(ALL, -1, CMAG);
    Send(tnum, rsucc, str.substr(1), astr, tstr);
    Send(ALL, -1, CNRM);
  } else {
    Send(tnum, rsucc, str, astr, tstr);
  }

  for (auto ind : contents) {
    if (ind->Skill(prhash(u8"Open")) || ind->Skill(prhash(u8"Transparent")))
      ind->SendOut(tnum, rsucc, mes, youmes, actor, targ, false);
    else if (ind->Pos() != pos_t::NONE) // FIXME - Understand Transparency
      ind->SendOut(tnum, rsucc, mes, youmes, actor, targ, false);
  }

  if (expanding && parent && (Skill(prhash(u8"Open")) || Skill(prhash(u8"Transparent")))) {
    no_seek = true;
    parent->SendOut(tnum, rsucc, mes, youmes, actor, targ, true);
    no_seek = false;
  }

  if (targ && targ != this && minds.size() > 0 && targ->HasSkill(prhash(u8"Object ID")) &&
      mes.starts_with(u8";s introduces ;s as")) {
    Learn(targ->Skill(prhash(u8"Object ID")), targ->NameS());
  }
}

void Object::Loud(int str, const std::u8string& mes) {
  std::set<Object*> visited;
  Loud(visited, str, mes);
}

void Object::Loud(std::set<Object*>& visited, int str, const std::u8string& mes) {
  visited.insert(this);
  auto targs = Connections(true);
  for (auto dest : targs) {
    if (dest && dest->Parent() && str > 0) {
      if (!visited.contains(dest->Parent())) {
        dest->Parent()->SendOut(
            ALL, 0, u8"From ;s you hear someone shout '{}'\n", u8"", dest, dest, mes);
        dest->Parent()->Loud(visited, str - 1, mes);
      }
    }
  }
}

void init_world() {
  Object::InitSkillsData();

  universe = new Object;
  universe->SetShortDesc(u8"The Universe");
  universe->SetDesc(u8"An Infinite Universe within which to play.");
  universe->SetLongDesc(u8"A Really Big Infinite Universe within which to play.");
  trash_bin = new Object;
  trash_bin->SetShortDesc(u8"The Trash Bin");
  trash_bin->SetDesc(u8"The place objects come to die.");

  if (!universe->Load(u8"acid/current.wld")) {
    load_players(u8"acid/current.plr");
  } else {
    FILE* conf = fopen(u8"acid/startup.conf", u8"r");
    if (conf) {
      Object* autoninja = new Object(universe);
      autoninja->SetShortDesc(u8"The AutoNinja");
      autoninja->SetDesc(u8"The AutoNinja - you should NEVER see this!");
      autoninja->SetPos(pos_t::STAND);

      Player* anp = new Player(u8"AutoNinja", u8"AutoNinja");
      anp->Set(PLAYER_SUPERNINJA);
      anp->Set(PLAYER_NINJA);
      anp->Set(PLAYER_NINJAMODE);

      Mind* automind = new Mind();
      automind->SetPlayer(u8"AutoNinja");
      automind->SetSystem();
      automind->Attach(autoninja);

      fseek(conf, 0, SEEK_END);
      size_t len = ftell(conf);
      fseek(conf, 0, SEEK_SET);
      char8_t buf2[len + 1];
      fread(buf2, 1, len, conf);
      buf2[len] = 0;

      handle_command(autoninja, buf2, automind);
      fclose(conf);

      delete automind;
      delete anp;
      autoninja->Recycle();
    }
  }
  universe->SetSkill(prhash(u8"Light Source"), 1000); // Ninjas need to see too.
}

void save_world(int with_net) {
  std::u8string fn = u8"acid/current";
  std::u8string wfn = fn + u8".wld.tmp";
  if (!universe->Save(wfn.c_str())) {
    std::u8string pfn = fn + u8".plr.tmp";
    if (!save_players(pfn.c_str())) {
      std::u8string nfn = fn + u8".nst";
      if ((!with_net) || (!save_net(nfn.c_str()))) {
        std::u8string dfn = fn + u8".wld";
        std::filesystem::remove(dfn);
        std::filesystem::rename(wfn, dfn);

        dfn = fn + u8".plr";
        std::filesystem::remove(dfn);
        std::filesystem::rename(pfn, dfn);
      } else {
        fprintf(stderr, u8"Unable to save network status!\n");
        perror("save_world");
      }
    } else {
      fprintf(stderr, u8"Unable to save players!\n");
      perror("save_world");
    }
  } else {
    fprintf(stderr, u8"Unable to save world!\n");
    perror("save_world");
  }
}

int Object::WriteContentsTo(FILE* fl) {
  for (auto cind : contents) {
    fprintf(fl, u8":%d", getnum(cind));
  }
  return 0;
}

void Object::BusyFor(long msec, const std::u8string& default_next) {
  // fprintf(stderr, u8"Holding %p, will default do '%s'!\n", this, default_next);
  busy_until = (current_time + (msec * 1000)) & 0xFFFFFFFFU; // Save Only Lower 32-Bits
  if (busy_until == 0) {
    busy_until = 1; // Avoid Special Case
  }
  if (default_next.length() == 0) {
    if (defact[0] != '\0') {
      delete[] defact;
    }
    defact = u8"";
  } else {
    if (defact[0] != '\0') {
      delete[] defact;
    }
    auto new_d = new char8_t[default_next.length() + 1];
    std::memcpy(new_d, default_next.c_str(), default_next.length() + 1);
    defact = new_d;
  }
  busylist.insert(this);
}

void Object::BusyWith(Object* other, const std::u8string& default_next) {
  // fprintf(stderr, u8"Holding %p, will default do '%s'!\n", this, default_next);
  busy_until = other->busy_until;
  if (default_next.length() == 0) {
    if (defact[0] != '\0') {
      delete[] defact;
    }
    defact = u8"";
  } else {
    if (defact[0] != '\0') {
      delete[] defact;
    }
    auto new_d = new char8_t[default_next.length() + 1];
    std::memcpy(new_d, default_next.c_str(), default_next.length() + 1);
    defact = new_d;
  }
  busylist.insert(this);
}

bool Object::StillBusy() const {
  if (busy_until == 0) {
    return false;
  }
  int64_t ct = current_time & 0xFFFFFFFFL;
  int64_t bu = busy_until;
  if (bu + 0x40000000L < ct) {
    bu += 0x100000000L; // Handle wrapped busy_util
  }
  return (ct <= bu);
}

void Object::DoWhenFree(const std::u8string& action) {
  // fprintf(stderr, u8"Adding busyact for %p of '%s'!\n", this, action);
  std::u8string dwf = dowhenfree;
  dwf += u8";";
  dwf += action;
  if (dwf.length() == 0) {
    if (dowhenfree[0] != '\0') {
      delete[] dowhenfree;
    }
    dowhenfree = u8"";
  } else {
    if (dowhenfree[0] != '\0') {
      delete[] dowhenfree;
    }
    auto new_d = new char8_t[dwf.length() + 1];
    std::memcpy(new_d, dwf.c_str(), dwf.length() + 1);
    dowhenfree = new_d;
  }
  busylist.insert(this);
}

bool Object::BusyAct() {
  // fprintf(stderr, u8"Taking busyact %p!\n", this);
  busylist.erase(this);

  std::u8string comm = dowhenfree;
  std::u8string def = defact;
  if (dowhenfree[0] != '\0') {
    delete[] dowhenfree;
    dowhenfree = u8"";
  }
  if (defact[0] != '\0') {
    delete[] defact;
    defact = u8"";
  }

  // fprintf(stderr, u8"Act is %s [%s]!\n", comm.c_str(), def.c_str());

  int ret;
  if (minds.size()) {
    ret = handle_command(this, comm.c_str(), (minds.front()));
    if (ret != 2 && (!StillBusy()))
      ret = handle_command(this, def.c_str(), (minds.front()));
  } else {
    ret = handle_command(this, comm.c_str());
    if (ret != 2 && (!StillBusy()))
      ret = handle_command(this, def.c_str());
  }

  if (!StillBusy()) {
    busy_until = 0; // Use special, permanent value, so it doesn't wrap.
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
        //	if(init.first->IsAct(act_t::FIGHT))
        //	  fprintf(stderr, u8"Going at %d (%s)\n", phase,
        // init.first->Noun().c_str());
        init.first->BusyAct();
      }
    }
  }
  for (auto init : initlist) {
    if (init.first->IsAct(act_t::FIGHT)) { // Still in combat!
      if (!init.first->StillBusy()) { // Make Sure!
        std::u8string ret = init.first->Tactics();
        init.first->BusyFor(3000, ret.c_str());
      }

      // Type 0x1000400 (MOB + MOB-FIGHT)
      for (auto trig : init.first->contents) {
        if ((trig->Skill(prhash(u8"TBAScriptType")) & 0x1000400) == 0x1000400) {
          if ((rand() % 100) < trig->Skill(prhash(u8"TBAScriptNArg"))) { // % Chance
            new_trigger(0, trig, init.first->ActTarg(act_t::FIGHT));
          }
        }
      }
    }
  }
}

std::u8string Object::Tactics(int phase) {
  if (minds.size() < 1)
    return u8"attack";
  Mind* mind = (*(minds.begin())); // FIXME: Handle Multiple Minds
  Object* body = mind->Body();
  mind->Attach(this);
  std::u8string ret = mind->Tactics();
  mind->Attach(body);
  return ret;
}

bool Object::IsSameAs(const Object& in) const {
  if (dlens.sd != in.dlens.sd)
    return false;
  if (dlens.n != in.dlens.n)
    return false;
  if (dlens.d != in.dlens.d)
    return false;
  if (dlens.ld != in.dlens.ld)
    return false;
  if (std::memcmp(descriptions, in.descriptions, dlens.sd + dlens.n + dlens.d + dlens.ld + 4) !=
      0) {
    return false;
  }
  if (weight != in.weight)
    return false;
  if (volume != in.volume)
    return false;
  if (size != in.size)
    return false;
  if (value != in.value)
    return false;
  if (gender != in.gender)
    return false;

  if (attr[0] != in.attr[0])
    return false;
  if (attr[1] != in.attr[1])
    return false;
  if (attr[2] != in.attr[2])
    return false;
  if (attr[3] != in.attr[3])
    return false;
  if (attr[4] != in.attr[4])
    return false;
  if (attr[5] != in.attr[5])
    return false;

  if (phys != in.phys)
    return false;
  if (stun != in.stun)
    return false;
  if (stru != in.stru)
    return false;

  if (pos != in.pos)
    return false;

  if (minds != in.minds)
    return false; // Only no-mind or same-group-minds

  if (act.size() != 0 || in.act.size() != 0)
    return false;

  if (contents.size() != 0 || in.contents.size() != 0)
    return false;

  auto sk1 = skills;
  auto sk2 = in.skills;
  for (auto sk = sk1.begin(); sk != sk1.end();) {
    if (sk->first == prhash(u8"Quantity") || sk->first == prhash(u8"Hungry") ||
        sk->first == prhash(u8"Bored") || sk->first == prhash(u8"Needy") ||
        sk->first == prhash(u8"Tired")) {
      sk1.erase(sk);
    } else {
      ++sk;
    }
  }
  for (auto sk = sk2.begin(); sk != sk2.end();) {
    if (sk->first == prhash(u8"Quantity") || sk->first == prhash(u8"Hungry") ||
        sk->first == prhash(u8"Bored") || sk->first == prhash(u8"Needy") ||
        sk->first == prhash(u8"Tired")) {
      sk2.erase(sk);
    } else {
      ++sk;
    }
  }
  std::sort(sk1.begin(), sk1.end());
  std::sort(sk2.begin(), sk2.end());
  if (sk1 != sk2)
    return false;

  return true;
}

void Object::operator=(const Object& in) {
  dlens = in.dlens;
  if (descriptions != default_descriptions) {
    delete[] descriptions;
  }
  if (in.descriptions == default_descriptions) {
    descriptions = default_descriptions;
  } else {
    const size_t len = dlens.sd + dlens.n + dlens.d + dlens.ld + 4;
    char8_t* new_descs = new char8_t[len];
    std::memcpy(new_descs, in.descriptions, len);
    descriptions = new_descs;
  }

  weight = in.weight;
  volume = in.volume;
  size = in.size;
  value = in.value;
  gender = in.gender;

  attr[0] = in.attr[0];
  attr[1] = in.attr[1];
  attr[2] = in.attr[2];
  attr[3] = in.attr[3];
  attr[4] = in.attr[4];
  attr[5] = in.attr[5];

  skills = in.skills;

  pos = in.pos;

  if (in.IsActive())
    Activate();
  else
    Deactivate();

  //  contents = in.contents;	// NOT doing this for deep/shallow reasons.
  //  act = in.act;
}

MinVec<3, Object*> Object::Contents(int vmode) const {
  MinVec<3, Object*> ret;
  if (vmode & LOC_NINJA) {
    ret = contents;
  } else {
    for (auto item : contents) {
      if (item->Skill(prhash(u8"Invisible")) >= 1000)
        continue; // Not Really There
      if ((vmode & (LOC_HEAT | LOC_TOUCH)) == 0 && item->Skill(prhash(u8"Invisible"))) {
        continue;
      }
      if ((vmode & LOC_FIXED) && item->Pos() != pos_t::NONE)
        continue;
      if ((vmode & LOC_NOTFIXED) && item->Pos() == pos_t::NONE)
        continue;
      ret.push_back(item);
    }
  }
  return ret;
}

MinVec<3, Object*> Object::Contents() const {
  return contents;
}

MinVec<7, Object*> Object::Connections(int vmode) const {
  MinVec<7, Object*> ret; // Includes nulls for unconnected dirs
  for (std::u8string dir : {u8"north", u8"south", u8"east", u8"west", u8"up", u8"down"}) {
    Object* conn = nullptr;
    Object* door = PickObject(dir, vmode | LOC_INTERNAL);
    if (door) {
      Object* odoor = door->ActTarg(act_t::SPECIAL_LINKED);
      if (odoor) {
        conn = odoor->Parent();
      }
    }
    ret.push_back(conn);
  }
  return ret;
}

MinVec<7, Object*> Object::Connections(bool exits) const {
  MinVec<7, Object*> ret; // Includes nulls for unconnected dirs
  for (std::u8string dir : {u8"north", u8"south", u8"east", u8"west", u8"up", u8"down"}) {
    Object* door = PickObject(dir, LOC_NINJA | LOC_INTERNAL);
    Object* odoor = nullptr;
    Object* conn = nullptr;
    if (door) {
      odoor = door->ActTarg(act_t::SPECIAL_LINKED);
      if (odoor) {
        conn = odoor->Parent();
      }
    }
    ret.push_back((conn && exits) ? odoor : conn);
  }
  return ret;
}

int Object::Contains(const Object* obj) const {
  return (std::find(contents.begin(), contents.end(), obj) != contents.end());
}

void Object::SpendExp(int e) {
  sexp += e;
}

bool Object::HasAccomplished(uint64_t acc) const {
  for (auto comp : completed) {
    if (comp == acc) {
      return true;
    }
  }
  return false;
}

bool Object::Accomplish(uint64_t acc, const std::u8string& why) {
  if (HasAccomplished(acc)) {
    return false;
  }
  completed.push_back(acc);
  for (auto m : minds) {
    if (m->Owner()) {
      m->Send(CYEL u8"You gain an experience point for {}!\n" CNRM, why);
    }
  }
  return true;
}

bool Object::Knows(const Object* o) const {
  if (o == this) {
    return true;
  }
  if (o->HasName()) {
    if (o->HasSkill(prhash(u8"Object ID"))) {
      return Knows(o->Skill(prhash(u8"Object ID")));
    }
  }
  return false;
}

bool Object::Knows(uint64_t k) const {
  for (auto know : known) {
    if (know == k) {
      return true;
    }
  }
  return false;
}

bool Object::Learn(uint64_t k, const std::u8string& what) {
  if (Knows(k)) {
    return false;
  }
  known.push_back(k);
  for (auto m : minds) {
    if (m->Owner()) {
      m->Send(CYEL u8"You now know {}!\n" CNRM, what);
    }
  }
  return true;
}

int two_handed(int wtype) {
  static std::set<int> thsks;
  if (thsks.size() == 0) {
    thsks.insert(get_weapon_type(u8"Two-Handed Blades"));
    thsks.insert(get_weapon_type(u8"Two-Handed Cleaves"));
    thsks.insert(get_weapon_type(u8"Two-Handed Crushing"));
    thsks.insert(get_weapon_type(u8"Two-Handed Flails"));
    thsks.insert(get_weapon_type(u8"Two-Handed Piercing"));
    thsks.insert(get_weapon_type(u8"Two-Handed Staves"));
  }
  return int(thsks.count(wtype));
}

std::u8string Object::PosString() const {
  std::u8string ret;
  if (pos == pos_t::USE) {
    ret = fmt::format(u8"is {} here", UsingString());
  } else {
    ret = pos_str[static_cast<uint8_t>(pos)];
  }
  return ret;
}

std::u8string Object::UsingString() const {
  std::u8string ret;
  if (pos == pos_t::USE) {
    if (cur_skill == prhash(u8"Stealth")) {
      ret = u8"sneaking around";
    } else if (cur_skill == prhash(u8"Perception")) {
      ret = u8"keeping an eye out";
    } else if (cur_skill == prhash(u8"Healing")) {
      ret = u8"caring for others' wounds";
    } else if (cur_skill == prhash(u8"First Aid")) {
      ret = u8"giving first-aid";
    } else if (cur_skill == prhash(u8"Treatment")) {
      ret = u8"treating others' wounds";
    } else if (cur_skill == prhash(u8"Lumberjack")) {
      ret = u8"chopping down trees";
    } else if (cur_skill == prhash(u8"Sprinting")) {
      ret = u8"running as fast as possible";
    } else {
      ret = fmt::format(u8"using the {} skill", SkillName(Using()));
    }
  } else {
    ret = u8"doing nothing";
  }
  return ret;
}

void Object::StartUsing(uint32_t skill) {
  cur_skill = skill;
  pos = pos_t::USE;
}

void Object::StopUsing() {
  if (pos == pos_t::USE)
    pos = pos_t::STAND;
  cur_skill = prhash(u8"None");
}

uint32_t Object::Using() const {
  return cur_skill;
}

int Object::IsUsing(uint32_t skill) const {
  return (skill == cur_skill);
}

pos_t Object::Pos() const {
  return pos;
}

void Object::SetPos(pos_t p) {
  if (pos == pos_t::USE && p != pos_t::USE)
    StopUsing();
  pos = p;
}

bool Object::Filter(int loc) const {
  if (loc & (LOC_ALIVE | LOC_CONSCIOUS)) {
    if (!IsAnimate() || IsAct(act_t::DEAD))
      return false;
  }
  if (loc & LOC_CONSCIOUS) {
    if (IsAct(act_t::DYING) || IsAct(act_t::UNCONSCIOUS))
      return false;
  }
  return true;
}

int Object::LooksLike(Object* other, int vmode, Object* viewer) const {
  if (Noun(false, true, viewer) != other->Noun(false, true, viewer))
    return 0;
  if (Pos() != other->Pos())
    return 0;
  if (Using() != other->Using())
    return 0;

  // Neither open/trans/seen inside (if either contain anything)
  if (Contents(vmode).size() > 0 || other->Contents(vmode).size() > 0) {
    if (Skill(prhash(u8"Open")) || Skill(prhash(u8"Transparent")))
      return 0;
    if (other->Skill(prhash(u8"Open")) || other->Skill(prhash(u8"Transparent")))
      return 0;
    if (Skill(prhash(u8"Container")) || Skill(prhash(u8"Liquid Container"))) {
      if (vmode && (!Skill(prhash(u8"Locked"))))
        return 0;
    }
    if (other->Skill(prhash(u8"Container")) || other->Skill(prhash(u8"Liquid Container"))) {
      if (vmode && (!other->Skill(prhash(u8"Locked"))))
        return 0;
    }
  }

  for (act_t actt = act_t::NONE; actt < act_t::WIELD;) { // Off-By-One!
    actt = act_t(int(actt) + 1); // Increments First!
    if (IsAct(actt) != other->IsAct(actt))
      return 0;
    if (ActTarg(actt) != other->ActTarg(actt)) {
      std::u8string s1 = u8"";
      if (ActTarg(actt))
        s1 = ActTarg(actt)->Noun(0, 0, this);
      std::u8string s2 = u8"";
      if (ActTarg(actt))
        s2 = other->ActTarg(actt)->Noun(0, 0, other);
      if (s1 != s2)
        return 0;
    }
  }
  return 1;
}

void Object::Consume(const Object* item) {
  // Standard food/drink effects
  int hung = Skill(prhash(u8"Hungry"));
  SetSkill(prhash(u8"Hungry"), Skill(prhash(u8"Hungry")) - item->Skill(prhash(u8"Food")));
  int thir = Skill(prhash(u8"Thirsty"));
  SetSkill(prhash(u8"Thirsty"), Skill(prhash(u8"Thirsty")) - item->Skill(prhash(u8"Drink")));
  SetSkill(
      prhash(u8"Thirsty"), Skill(prhash(u8"Thirsty")) + item->Skill(prhash(u8"Dehydrate Effect")));
  // Heal back dehydrate/hunger wounds
  if ((hung / 500000) > (Skill(prhash(u8"Hungry")) / 500000)) {
    HealStun((hung / 500000) - (Skill(prhash(u8"Hungry")) / 500000));
  }
  if ((thir / 50000) > (Skill(prhash(u8"Thirsty")) / 50000)) {
    HealPhys((thir / 50000) - (Skill(prhash(u8"Thirsty")) / 50000));
  }

  // Special effect: Poisonous
  if (item->Skill(prhash(u8"Poisonous")) > 0) {
    SetSkill(
        prhash(u8"Poisoned"), Skill(prhash(u8"Poisoned")) + item->Skill(prhash(u8"Poisonous")));
  }

  // Special effect: Cure Poison
  if (item->Skill(prhash(u8"Cure Poison Spell")) > 0 && Skill(prhash(u8"Poisoned")) > 0) {
    if (item->Skill(prhash(u8"Cure Poison Spell")) >= Skill(prhash(u8"Poisoned"))) {
      SetSkill(prhash(u8"Poisoned"), 0);
      Send(ALL, 0, u8"You feel better.\n");
    }
  }

  // Special effect: Heal
  if (item->Skill(prhash(u8"Heal Spell")) > 0) {
    int succ = Roll(prhash(u8"Strength"), 12 - item->Skill(prhash(u8"Heal Spell")));
    if (phys > 0 && succ > 0)
      Send(ALL, 0, u8"You feel a bit less pain.\n");
    phys -= succ;
    if (phys < 0)
      phys = 0;
    UpdateDamage();
  }

  // Special effect: Energize
  if (item->Skill(prhash(u8"Energize Spell")) > 0) {
    int succ = Roll(prhash(u8"Strength"), 12 - item->Skill(prhash(u8"Energize Spell")));
    if (stun > 0 && succ > 0)
      Send(ALL, 0, u8"You feel a bit more rested.\n");
    stun -= succ;
    if (stun < 0)
      stun = 0;
    UpdateDamage();
  }

  // Special effect: Remove Curse - Note: Can't remove curse from cursed items
  if (item->Skill(prhash(u8"Remove Curse Spell")) > 0 && (!HasSkill(prhash(u8"Cursed")))) {
    Object* cursed = NextHasSkill(prhash(u8"Cursed"));
    while (cursed) {
      if (cursed->Skill(prhash(u8"Cursed")) <= item->Skill(prhash(u8"Remove Curse Spell"))) {
        Drop(cursed, 1, 1);
        cursed = NextHasSkill(prhash(u8"Cursed"));
      } else {
        cursed = NextHasSkill(prhash(u8"Cursed"), cursed);
      }
    }
  }

  // Special effect: Recall
  if (item->Skill(prhash(u8"Recall Spell")) > 0) {
    if (parent) {
      parent->SendOut(
          0, 0, u8"BAMF! ;s teleports away.\n", u8"BAMF! You teleport home.\n", this, nullptr);
    }
    Object* dest = this;
    while ((!dest->ActTarg(act_t::SPECIAL_HOME)) && dest->Parent()) {
      dest = dest->Parent();
    }
    if (dest->ActTarg(act_t::SPECIAL_HOME)) {
      dest = dest->ActTarg(act_t::SPECIAL_HOME);
    }
    Travel(dest, 0);
    if (parent) {
      parent->SendOut(0, 0, u8"BAMF! ;s teleports here.\n", u8"", this, nullptr);
      parent->SendDescSurround(this, this);
    }
  }

  // Special effect: Heat Vision Spell (Grants Ability)

  if (item->Skill(prhash(u8"Heat Vision Spell"))) {
    int force = item->Skill(prhash(u8"Heat Vision Spell"));
    Object* spell = new Object(this);
    spell->SetSkill(prhash(u8"Heat Vision"), std::min(100, force));
    spell->SetShortDesc(u8"a spell");
    spell->SetSkill(prhash(u8"Magical"), force);
    spell->SetSkill(prhash(u8"Magical Spell"), force);
    spell->SetSkill(prhash(u8"Temporary"), force);
    spell->SetSkill(prhash(u8"Invisible"), 1000);
    spell->Activate();
    Send(ALL, 0, u8"You can now see better!\n");
  }

  // Special effect: Dark Vision Spell (Grants Ability)
  if (item->Skill(prhash(u8"Dark Vision Spell"))) {
    int force = item->Skill(prhash(u8"Dark Vision Spell"));
    Object* spell = new Object(this);
    spell->SetSkill(prhash(u8"Dark Vision"), std::min(100, force));
    spell->SetShortDesc(u8"a spell");
    spell->SetSkill(prhash(u8"Magical"), force);
    spell->SetSkill(prhash(u8"Magical Spell"), force);
    spell->SetSkill(prhash(u8"Temporary"), force);
    spell->SetSkill(prhash(u8"Invisible"), 1000);
    spell->Activate();
    Send(ALL, 0, u8"You can now see better!\n");
  }

  // Special effect: Teleport Spell (Grants Ability)
  if (item->Skill(prhash(u8"Teleport Spell"))) {
    SetSkill(prhash(u8"Teleport"), 1); // Can use once!
    Send(ALL, 0, u8"You can now teleport once!\n");
  }

  // Special effect: Resurrect Spell (Grants Ability)
  if (item->Skill(prhash(u8"Resurrect Spell"))) {
    SetSkill(prhash(u8"Resurrect"), 1); // Can use once!
    Send(ALL, 0, u8"You can now resurrect a character once!\n");
  }

  // Special effect: Sleep Other
  if (item->Skill(prhash(u8"Sleep Other Spell")) > 0) {
    int succ = Roll(prhash(u8"Willpower"), item->Skill(prhash(u8"Sleep Other Spell")));
    if (succ > 0) {
      if (parent) {
        parent->SendOut(
            0,
            0,
            u8";s looks groggy for a moment, but recovers.\n",
            u8"You feel groggy for a moment, but recover.\n",
            this,
            nullptr);
      }
    } else {
      handle_command(this, u8"sleep");
    }
  }
}

int Object::LightLevel(int updown) {
  int level = 0;

  if (updown != -1 && Parent()) { // Go Up
    int fac =
        Skill(prhash(u8"Open")) + Skill(prhash(u8"Transparent")) + Skill(prhash(u8"Translucent"));
    if (fac > 1000)
      fac = 1000;
    if (fac > 0) {
      level += (fac * Parent()->LightLevel(1));
    }
  }
  if (updown != 1) { // Go Down
    for (auto item : contents) {
      if (!Wearing(item)) { // Containing it (internal)
        int fac = item->Skill(prhash(u8"Open")) + item->Skill(prhash(u8"Transparent")) +
            item->Skill(prhash(u8"Translucent"));
        if (fac > 1000) {
          fac = 1000;
        }
        if (fac > 0) {
          level += (fac * item->LightLevel(-1));
        }
        level += 1000 * item->Skill(prhash(u8"Light Source"));
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
  level += Skill(prhash(u8"Light Source"));
  if (level > 1000)
    level = 1000;
  return level;
}

int Object::NormAttribute(int a) const {
  if (a >= 6) { // Reaction = (Q+I)/2
    return (attr[1] + attr[4]) / 2;
  }
  return attr[a];
}

int Object::ModAttribute(int a) const {
  if (a >= 6) { // Reaction
    return (ModAttribute(1) + ModAttribute(4)) / 2 + Modifier(u8"Reaction");
  }
  if (attr[a] == 0) {
    return 0; // Can't Enhance Nothing
  }
  return attr[a] + Modifier(a);
}

static const uint32_t attr_bonus[6] = {
    prhash(u8"Body Bonus"),
    prhash(u8"Quickness Bonus"),
    prhash(u8"Strength Bonus"),
    prhash(u8"Charisma Bonus"),
    prhash(u8"Intelligence Bonus"),
    prhash(u8"Willpower Bonus"),
};
static const uint32_t attr_penalty[6] = {
    prhash(u8"Body Penalty"),
    prhash(u8"Quickness Penalty"),
    prhash(u8"Strength Penalty"),
    prhash(u8"Charisma Penalty"),
    prhash(u8"Intelligence Penalty"),
    prhash(u8"Willpower Penalty"),
};
int Object::Modifier(int a) const {
  int ret = 0;
  for (auto item : contents) {
    if (Wearing(item) || item->Skill(prhash(u8"Magical Spell"))) {
      ret += item->Skill(attr_bonus[a]);
      ret -= item->Skill(attr_penalty[a]);
    }
  }
  ret += Skill(attr_bonus[a]);
  ret -= Skill(attr_penalty[a]);
  if (ret < 0)
    return (ret - 999) / 1000;
  return (ret / 1000);
}

int Object::Modifier(const std::u8string& m) const {
  int ret = 0;
  for (auto item : contents) {
    if (Wearing(item) || item->Skill(prhash(u8"Magical Spell"))) {
      ret += item->Skill(crc32c(m + u8" Bonus"));
      ret -= item->Skill(crc32c(m + u8" Penalty"));
    }
  }
  ret += Skill(crc32c(m + u8" Bonus"));
  ret -= Skill(crc32c(m + u8" Penalty"));
  if (ret < 0)
    return (ret - 999) / 1000;
  return (ret / 1000);
}

int Object::Power(uint32_t ptok) const {
  int ret = 0;
  ret = Skill(ptok);
  for (auto item : contents) {
    if (ActTarg(act_t::WIELD) == item || Wearing(item) || item->Skill(prhash(u8"Magical Spell"))) {
      int val = item->Skill(ptok);
      if (val > ret)
        ret = val;
    }
  }
  return ret;
}

int Object::Wearing(const Object* obj) const {
  for (act_t actt = act_t::HOLD; actt < act_t::MAX; ++actt) {
    if (ActTarg(actt) == obj)
      return 1;
  }
  return 0;
}

int Object::WearMask() const {
  return (
      Skill(prhash(u8"Wearable on Back")) | Skill(prhash(u8"Wearable on Chest")) |
      Skill(prhash(u8"Wearable on Head")) | Skill(prhash(u8"Wearable on Neck")) |
      Skill(prhash(u8"Wearable on Collar")) | Skill(prhash(u8"Wearable on Waist")) |
      Skill(prhash(u8"Wearable on Shield")) | Skill(prhash(u8"Wearable on Left Arm")) |
      Skill(prhash(u8"Wearable on Right Arm")) | Skill(prhash(u8"Wearable on Left Finger")) |
      Skill(prhash(u8"Wearable on Right Finger")) | Skill(prhash(u8"Wearable on Left Foot")) |
      Skill(prhash(u8"Wearable on Right Foot")) | Skill(prhash(u8"Wearable on Left Hand")) |
      Skill(prhash(u8"Wearable on Right Hand")) | Skill(prhash(u8"Wearable on Left Leg")) |
      Skill(prhash(u8"Wearable on Right Leg")) | Skill(prhash(u8"Wearable on Left Wrist")) |
      Skill(prhash(u8"Wearable on Right Wrist")) | Skill(prhash(u8"Wearable on Left Shoulder")) |
      Skill(prhash(u8"Wearable on Right Shoulder")) | Skill(prhash(u8"Wearable on Left Hip")) |
      Skill(prhash(u8"Wearable on Right Hip")));
}

std::set<act_t> Object::WearSlots(int m) const {
  std::set<act_t> locs;
  if (Skill(prhash(u8"Wearable on Back")) & m)
    locs.insert(act_t::WEAR_BACK);
  if (Skill(prhash(u8"Wearable on Chest")) & m)
    locs.insert(act_t::WEAR_CHEST);
  if (Skill(prhash(u8"Wearable on Head")) & m)
    locs.insert(act_t::WEAR_HEAD);
  if (Skill(prhash(u8"Wearable on Face")) & m)
    locs.insert(act_t::WEAR_FACE);
  if (Skill(prhash(u8"Wearable on Neck")) & m)
    locs.insert(act_t::WEAR_NECK);
  if (Skill(prhash(u8"Wearable on Collar")) & m)
    locs.insert(act_t::WEAR_COLLAR);
  if (Skill(prhash(u8"Wearable on Waist")) & m)
    locs.insert(act_t::WEAR_WAIST);
  if (Skill(prhash(u8"Wearable on Shield")) & m)
    locs.insert(act_t::WEAR_SHIELD);
  if (Skill(prhash(u8"Wearable on Left Arm")) & m)
    locs.insert(act_t::WEAR_LARM);
  if (Skill(prhash(u8"Wearable on Right Arm")) & m)
    locs.insert(act_t::WEAR_RARM);
  if (Skill(prhash(u8"Wearable on Left Finger")) & m)
    locs.insert(act_t::WEAR_LFINGER);
  if (Skill(prhash(u8"Wearable on Right Finger")) & m)
    locs.insert(act_t::WEAR_RFINGER);
  if (Skill(prhash(u8"Wearable on Left Foot")) & m)
    locs.insert(act_t::WEAR_LFOOT);
  if (Skill(prhash(u8"Wearable on Right Foot")) & m)
    locs.insert(act_t::WEAR_RFOOT);
  if (Skill(prhash(u8"Wearable on Left Hand")) & m)
    locs.insert(act_t::WEAR_LHAND);
  if (Skill(prhash(u8"Wearable on Right Hand")) & m)
    locs.insert(act_t::WEAR_RHAND);
  if (Skill(prhash(u8"Wearable on Left Leg")) & m)
    locs.insert(act_t::WEAR_LLEG);
  if (Skill(prhash(u8"Wearable on Right Leg")) & m)
    locs.insert(act_t::WEAR_RLEG);
  if (Skill(prhash(u8"Wearable on Left Wrist")) & m)
    locs.insert(act_t::WEAR_LWRIST);
  if (Skill(prhash(u8"Wearable on Right Wrist")) & m)
    locs.insert(act_t::WEAR_RWRIST);
  if (Skill(prhash(u8"Wearable on Left Shoulder")) & m)
    locs.insert(act_t::WEAR_LSHOULDER);
  if (Skill(prhash(u8"Wearable on Right Shoulder")) & m)
    locs.insert(act_t::WEAR_RSHOULDER);
  if (Skill(prhash(u8"Wearable on Left Hip")) & m)
    locs.insert(act_t::WEAR_LHIP);
  if (Skill(prhash(u8"Wearable on Right Hip")) & m)
    locs.insert(act_t::WEAR_RHIP);
  return locs;
}

std::u8string Object::WearNames(const std::set<act_t>& locs) const {
  std::u8string ret = u8"";
  for (auto loc = locs.begin(); loc != locs.end(); ++loc) {
    if (loc != locs.begin()) {
      auto tmp = loc;
      ++tmp;
      if (tmp == locs.end()) {
        if (locs.size() == 2) {
          ret += u8" and ";
        } else {
          ret += u8", and ";
        }
      } else {
        ret += u8", ";
      }
    }
    if (*loc == act_t::WEAR_BACK)
      ret += u8"back";
    else if (*loc == act_t::WEAR_CHEST)
      ret += u8"chest";
    else if (*loc == act_t::WEAR_HEAD)
      ret += u8"head";
    else if (*loc == act_t::WEAR_FACE)
      ret += u8"face";
    else if (*loc == act_t::WEAR_NECK)
      ret += u8"neck";
    else if (*loc == act_t::WEAR_COLLAR)
      ret += u8"collar";
    else if (*loc == act_t::WEAR_WAIST)
      ret += u8"waist";
    else if (*loc == act_t::WEAR_SHIELD)
      ret += u8"shield";
    else if (*loc == act_t::WEAR_LARM)
      ret += u8"left arm";
    else if (*loc == act_t::WEAR_RARM)
      ret += u8"right arm";
    else if (*loc == act_t::WEAR_LFINGER)
      ret += u8"left finger";
    else if (*loc == act_t::WEAR_RFINGER)
      ret += u8"right finger";
    else if (*loc == act_t::WEAR_LFOOT)
      ret += u8"left foot";
    else if (*loc == act_t::WEAR_RFOOT)
      ret += u8"right foot";
    else if (*loc == act_t::WEAR_LHAND)
      ret += u8"left hand";
    else if (*loc == act_t::WEAR_RHAND)
      ret += u8"right hand";
    else if (*loc == act_t::WEAR_LLEG)
      ret += u8"left leg";
    else if (*loc == act_t::WEAR_RLEG)
      ret += u8"right leg";
    else if (*loc == act_t::WEAR_LWRIST)
      ret += u8"left wrist";
    else if (*loc == act_t::WEAR_RWRIST)
      ret += u8"right wrist";
    else if (*loc == act_t::WEAR_LSHOULDER)
      ret += u8"left shoulder";
    else if (*loc == act_t::WEAR_RSHOULDER)
      ret += u8"right shoulder";
    else if (*loc == act_t::WEAR_LHIP)
      ret += u8"left hip";
    else if (*loc == act_t::WEAR_RHIP)
      ret += u8"right hip";
  }
  return ret;
}

std::u8string Object::WearNames(int m) const {
  return WearNames(WearSlots(m));
}

Object* Object::Stash(Object* item, int message, int force, int try_combine) {
  MinVec<1, Object*> containers;
  auto my_cont = PickObjects(u8"all", LOC_INTERNAL);
  for (auto ind : my_cont) {
    if (ind->Skill(prhash(u8"Container")) &&
        ((!ind->Skill(prhash(u8"Locked"))) || ind->Skill(prhash(u8"Open")))) {
      containers.push_back(ind);
    }
  }

  Object* dest = nullptr;
  for (auto con : containers) {
    if (con->Skill(prhash(u8"Capacity")) - con->ContainedVolume() < item->Volume())
      continue;
    if (con->Skill(prhash(u8"Container")) - con->ContainedWeight() < item->Weight())
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
    if (dest->Skill(prhash(u8"Open")) < 1)
      openclose = 1;

    if (openclose)
      parent->SendOut(0, 0, u8";s opens ;s.\n", u8"You open ;s.\n", this, dest);
    parent->SendOut(0, 0, u8";s stashes ;s.\n", u8"You stash ;s.\n", this, item);
    if (openclose)
      parent->SendOut(0, 0, u8";s closes ;s.\n", u8"You close ;s.\n", this, dest);
  }

  return dest;
}

int Object::Drop(Object* item, int message, int force, int try_combine) {
  if (!item)
    return 1;
  if (!parent)
    return 1;

  // Can't drop cursed stuff (unless ninja or otherwise forcing)
  if ((!force) && item->SubHasSkill(prhash(u8"Cursed"))) {
    return -4;
  }

  int ret = item->Travel(parent, try_combine);
  if (ret)
    return ret;

  // Activate perishable dropped stuff, so it will rot
  if (item->HasSkill(prhash(u8"Perishable"))) {
    item->Activate();
  }

  if (message) {
    parent->SendOut(0, 0, u8";s drops ;s.\n", u8"You drop ;s.\n", this, item);
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
  if (!HasSkill(prhash(u8"Quantity")))
    return 1;
  return Skill(prhash(u8"Quantity"));
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

    if (targ->Skill(prhash(u8"Wearable on Back")) & mask)
      locations.insert(act_t::WEAR_BACK);

    if (targ->Skill(prhash(u8"Wearable on Chest")) & mask)
      locations.insert(act_t::WEAR_CHEST);

    if (targ->Skill(prhash(u8"Wearable on Head")) & mask)
      locations.insert(act_t::WEAR_HEAD);

    if (targ->Skill(prhash(u8"Wearable on Face")) & mask)
      locations.insert(act_t::WEAR_FACE);

    if (targ->Skill(prhash(u8"Wearable on Neck")) & mask)
      locations.insert(act_t::WEAR_NECK);

    if (targ->Skill(prhash(u8"Wearable on Collar")) & mask)
      locations.insert(act_t::WEAR_COLLAR);

    if (targ->Skill(prhash(u8"Wearable on Waist")) & mask)
      locations.insert(act_t::WEAR_WAIST);

    if (targ->Skill(prhash(u8"Wearable on Shield")) & mask)
      locations.insert(act_t::WEAR_SHIELD);

    if (targ->Skill(prhash(u8"Wearable on Left Arm")) & mask)
      locations.insert(act_t::WEAR_LARM);

    if (targ->Skill(prhash(u8"Wearable on Right Arm")) & mask)
      locations.insert(act_t::WEAR_RARM);

    if (targ->Skill(prhash(u8"Wearable on Left Finger")) & mask)
      locations.insert(act_t::WEAR_LFINGER);

    if (targ->Skill(prhash(u8"Wearable on Right Finger")) & mask)
      locations.insert(act_t::WEAR_RFINGER);

    if (targ->Skill(prhash(u8"Wearable on Left Foot")) & mask)
      locations.insert(act_t::WEAR_LFOOT);

    if (targ->Skill(prhash(u8"Wearable on Right Foot")) & mask)
      locations.insert(act_t::WEAR_RFOOT);

    if (targ->Skill(prhash(u8"Wearable on Left Hand")) & mask)
      locations.insert(act_t::WEAR_LHAND);

    if (targ->Skill(prhash(u8"Wearable on Right Hand")) & mask)
      locations.insert(act_t::WEAR_RHAND);

    if (targ->Skill(prhash(u8"Wearable on Left Leg")) & mask)
      locations.insert(act_t::WEAR_LLEG);

    if (targ->Skill(prhash(u8"Wearable on Right Leg")) & mask)
      locations.insert(act_t::WEAR_RLEG);

    if (targ->Skill(prhash(u8"Wearable on Left Wrist")) & mask)
      locations.insert(act_t::WEAR_LWRIST);

    if (targ->Skill(prhash(u8"Wearable on Right Wrist")) & mask)
      locations.insert(act_t::WEAR_RWRIST);

    if (targ->Skill(prhash(u8"Wearable on Left Shoulder")) & mask)
      locations.insert(act_t::WEAR_LSHOULDER);

    if (targ->Skill(prhash(u8"Wearable on Right Shoulder")) & mask)
      locations.insert(act_t::WEAR_RSHOULDER);

    if (targ->Skill(prhash(u8"Wearable on Left Hip")) & mask)
      locations.insert(act_t::WEAR_LHIP);

    if (targ->Skill(prhash(u8"Wearable on Right Hip")) & mask)
      locations.insert(act_t::WEAR_RHIP);

    if (locations.size() < 1) {
      if (mask == 1) {
        if (mes)
          targ->Send(ALL, -1, u8"You can't wear {} - it's not wearable!\n", targ->Noun(0, 0, this));
      } else {
        if (mes)
          targ->Send(
              ALL,
              -1,
              u8"You can't wear {} with what you are already wearing!\n",
              targ->Noun(0, 0, this));
      }
      break;
    }
    success = 1;
    mask <<= 1;
    while ((mask & masks) == 0 && mask != 0)
      mask <<= 1;

    if (targ->Skill(prhash(u8"Quantity")) > 1) { // One at a time!
      targ = targ->Split(1);
    }

    for (auto loc : locations) {
      if (IsAct(loc)) {
        success = 0;
        break;
      }
    }
    if (success) {
      targ->Travel(this, 0); // Kills Holds and Wields on u8"targ"
      for (auto loc : locations) {
        AddAct(loc, targ);
      }
      Parent()->SendOut(
          0,
          0, // FIXME: stealth_t, stealth_s,
          u8";s puts on ;s.\n",
          u8"You put on ;s.\n",
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

MinVec<7, skill_pair> Object::GetSkills() const {
  auto ret = skills;

  std::sort(ret.begin(), ret.end(), [](auto& s1, auto& s2) {
    return (SkillName(s1.first) < SkillName(s2.first));
  });

  return ret;
}
