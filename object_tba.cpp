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
#include <vector>

#include <fcntl.h>
#include <unistd.h>
#include <cctype>
#include <cmath>

#include "cchar8.hpp"
#include "color.hpp"
#include "commands.hpp"
#include "mind.hpp"
#include "object.hpp"
#include "properties.hpp"
#include "utils.hpp"

static int obj_aliases = 0;
static int mob_aliases = 0;
static const std::u8string target_chars =
    u8"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_-'";

static uint32_t tba_bitvec(const std::u8string& val) {
  uint32_t ret = atoi(val.c_str());
  if (ret == 0) {
    for (size_t idx = 0; idx < val.length(); ++idx) {
      ret |= 1 << ((val[idx] & 31) - 1);
    }
  }
  return ret;
}

static const std::u8string dirname[6] =
    {u8"north", u8"east", u8"south", u8"west", u8"up", u8"down"};

static int fline(FILE* f) {
  long pos = ftell(f);
  int lnum = 0;
  rewind(f);
  fscanf(f, u8"%*[^\n]");
  while ((!feof(f)) && pos > ftell(f)) {
    ++lnum;
    fscanf(f, u8"%*c%*[^\n]");
  }
  fseek(f, pos, SEEK_SET);
  return lnum;
}

static int untrans_trig = 0;
static char8_t buf[65536];
void Object::TBALoadAll() {
  FILE* mudt = fopen(u8"tba/trg/index", u8"r");
  if (mudt) {
    sprintf(buf, u8"tba/trg/%c", 0);
    memset(buf + strlen(buf), 0, 256);
    fscanf(mudt, u8"%255[^\n\r]\n", buf + strlen(buf));
    while (strlen(buf) > 10) {
      TBALoadTRG(buf);
      sprintf(buf, u8"tba/trg/%c", 0);
      memset(buf + strlen(buf), 0, 256);
      fscanf(mudt, u8"%255[^\n\r]\n", buf + strlen(buf));
    }
    fclose(mudt);
  }
  FILE* mudw = fopen(u8"tba/wld/index", u8"r");
  if (mudw) {
    sprintf(buf, u8"tba/wld/%c", 0);
    memset(buf + strlen(buf), 0, 256);
    fscanf(mudw, u8"%255[^\n\r]\n", buf + strlen(buf));
    while (strlen(buf) > 10) {
      TBALoadWLD(buf);
      sprintf(buf, u8"tba/wld/%c", 0);
      memset(buf + strlen(buf), 0, 256);
      fscanf(mudw, u8"%255[^\n\r]\n", buf + strlen(buf));
    }
    fclose(mudw);
  }
  FILE* mudo = fopen(u8"tba/obj/index", u8"r");
  if (mudo) {
    sprintf(buf, u8"tba/obj/%c", 0);
    memset(buf + strlen(buf), 0, 256);
    fscanf(mudo, u8"%255[^\n\r]\n", buf + strlen(buf));
    while (strlen(buf) > 10) {
      TBALoadOBJ(buf);
      sprintf(buf, u8"tba/obj/%c", 0);
      memset(buf + strlen(buf), 0, 256);
      fscanf(mudo, u8"%255[^\n\r]\n", buf + strlen(buf));
    }
    fclose(mudo);
  }
  FILE* mudm = fopen(u8"tba/mob/index", u8"r");
  if (mudm) {
    sprintf(buf, u8"tba/mob/%c", 0);
    memset(buf + strlen(buf), 0, 256);
    fscanf(mudm, u8"%255[^\n\r]\n", buf + strlen(buf));
    while (strlen(buf) > 10) {
      TBALoadMOB(buf);
      sprintf(buf, u8"tba/mob/%c", 0);
      memset(buf + strlen(buf), 0, 256);
      fscanf(mudm, u8"%255[^\n\r]\n", buf + strlen(buf));
    }
    fclose(mudm);
  }
  FILE* mudz = fopen(u8"tba/zon/index", u8"r");
  if (mudz) {
    sprintf(buf, u8"tba/zon/%c", 0);
    memset(buf + strlen(buf), 0, 256);
    fscanf(mudz, u8"%255[^\n\r]\n", buf + strlen(buf));
    while (strlen(buf) > 10) {
      TBALoadZON(buf);
      sprintf(buf, u8"tba/zon/%c", 0);
      memset(buf + strlen(buf), 0, 256);
      fscanf(mudz, u8"%255[^\n\r]\n", buf + strlen(buf));
    }
    fclose(mudz);
  }
  TBAFinalizeTriggers();
  FILE* muds = fopen(u8"tba/shp/index", u8"r");
  if (muds) {
    sprintf(buf, u8"tba/shp/%c", 0);
    memset(buf + strlen(buf), 0, 256);
    fscanf(muds, u8"%255[^\n\r]\n", buf + strlen(buf));
    while (strlen(buf) > 10) {
      TBALoadSHP(buf);
      sprintf(buf, u8"tba/shp/%c", 0);
      memset(buf + strlen(buf), 0, 256);
      fscanf(muds, u8"%255[^\n\r]\n", buf + strlen(buf));
    }
    fclose(muds);
  }
  TBACleanup();
  fprintf(stderr, CBYL u8"Warning: %d untranslated triggers!\n" CNRM, untrans_trig);
  fprintf(stderr, CBYL u8"Warning: %d tacked-on object aliases!\n" CNRM, obj_aliases);
  fprintf(stderr, CBYL u8"Warning: %d tacked-on mob aliases!\n" CNRM, mob_aliases);
}

static std::vector<Object*> todotrg;
static std::map<int, Object*> bynumtrg;
static std::map<int, Object*> bynumwld;
static std::map<int, Object*> bynumobj;
static std::map<int, Object*> bynummob;
static std::map<int, Object*> bynummobinst;
static std::map<Object*, int> tonum[6];
static std::map<Object*, int> tynum[6];
static std::map<Object*, int> knum[6];
static std::map<Object*, std::u8string> nmnum[6];
static std::vector<Object*> olist;
static Object* objroom = nullptr;
static Object* mobroom = nullptr;

void Object::TBACleanup() {
  //  for(ind : bynumobj) {
  //    ind.second->Recycle();
  //    }
  //  for(ind : bynummob) {
  //    ind.second->Recycle();
  //    }
  for (auto ind : bynumtrg) {
    ind.second->Recycle();
  }

  bynumwld.clear();
  bynumobj.clear();
  bynummob.clear();
  bynumtrg.clear();
  bynummobinst.clear();
  tonum[0].clear();
  tonum[1].clear();
  tonum[2].clear();
  tonum[3].clear();
  tonum[4].clear();
  tonum[5].clear();
  tynum[0].clear();
  tynum[1].clear();
  tynum[2].clear();
  tynum[3].clear();
  tynum[4].clear();
  tonum[5].clear();
  knum[0].clear();
  knum[1].clear();
  knum[2].clear();
  knum[3].clear();
  knum[4].clear();
  knum[5].clear();
  nmnum[0].clear();
  nmnum[1].clear();
  nmnum[2].clear();
  nmnum[3].clear();
  nmnum[4].clear();
  nmnum[5].clear();
  olist.clear();
  todotrg.clear();
}

void Object::TBAFinalizeTriggers() {
  for (auto trg : todotrg) {
    std::u8string newtext = u8"Powers List:\n";
    auto cur = trg->LongDesc().find(u8"teleport [");
    while (cur != std::u8string::npos) {
      int rnum;
      trg->Parent()->SetSkill(prhash(u8"Teleport"), 10);
      trg->Parent()->SetSkill(prhash(u8"Restricted Item"), 1);
      sscanf(trg->LongDescC() + cur, u8"teleport [%d]\n", &rnum);
      if (bynumwld.count(rnum) > 0) {
        newtext += std::u8string(u8"teleport ") + bynumwld[rnum]->Noun() + u8"\n";
      } else {
        fprintf(stderr, u8"Error: Can't find teleport dest: %d\n", rnum);
      }
      cur = trg->LongDesc().find(u8"teleport [", cur + 9);
    }
    if (newtext != u8"Powers List:\n") {
      trg->Parent()->SetLongDesc(newtext.c_str());
      trg->Recycle();
      // fprintf(stderr, u8"%s", newtext.c_str());
    } else if (trg->Skill(prhash(u8"TBAScriptType")) & 0x1000000) { // Room or Obj
      trg->Activate();
      new_trigger(13000 + (rand() % 13000), trg, nullptr, nullptr, u8"");
    }
  }
  todotrg.clear();
}

static Mind* tba_mob_mind = nullptr;

static Object* gold = nullptr;
static void init_gold() {
  gold = new Object();
  gold->SetShortDesc(u8"a gold piece");
  gold->SetDesc(u8"A standard one-ounce gold piece.");
  gold->SetWeight(454 / 16);
  gold->SetVolume(0);
  gold->SetValue(1);
  gold->SetSize(0);
  gold->SetPos(pos_t::LIE);
  gold->SetSkill(prhash(u8"Money"), 1);
}

Mind* get_tba_mob_mind() {
  if (!tba_mob_mind) {
    tba_mob_mind = new Mind();
    tba_mob_mind->SetTBAMob();
  }
  return tba_mob_mind;
}

// Format: Strings, terminated by only an EoL '~'.
// ...other '~' characters are valid components.
std::u8string load_tba_field(FILE* fd) {
  std::u8string ret = u8"";
  {
    char8_t buffer[65536] = {};
    if (!fscanf(fd, u8"%65535[^\n]%*c", buffer)) {
      fscanf(fd, u8"%*c");
    } else {
      ret = buffer;
    }
  }
  while (ret.length() == 0 || ret.back() != '~') {
    char8_t buffer[65536] = {};
    ret += u8"\n";
    if (!fscanf(fd, u8"%65535[^\n]%*c", buffer)) {
      fscanf(fd, u8"%*c");
    } else {
      ret += buffer;
    }
  }

  ret.pop_back(); // Remove the terminating '~'

  std::transform(ret.begin(), ret.end(), ret.begin(), [](auto c) { return (c == ';') ? ',' : c; });

  return ret;
}

Object* dup_tba_obj(Object* obj) {
  Object* obj2 = nullptr;
  if (obj->Skill(prhash(u8"Wearable on Left Hand")) !=
      obj->Skill(prhash(u8"Wearable on Right Hand"))) {
    obj2 = new Object(*obj);
    obj2->SetSkill(prhash(u8"Wearable on Left Hand"), 0);
    obj2->SetSkill(prhash(u8"Wearable on Right Hand"), 1);
    obj->SetShortDesc((std::u8string(obj->ShortDesc()) + u8" (left)").c_str());
    obj2->SetShortDesc((std::u8string(obj2->ShortDesc()) + u8" (right)").c_str());
    //    fprintf(stderr, u8"Duped: '%s'\n", obj2->ShortDesc());
  } else if (
      obj->Skill(prhash(u8"Wearable on Left Foot")) !=
      obj->Skill(prhash(u8"Wearable on Right Foot"))) {
    obj2 = new Object(*obj);
    obj2->SetSkill(prhash(u8"Wearable on Left Foot"), 0);
    obj2->SetSkill(prhash(u8"Wearable on Right Foot"), 1);
    obj->SetShortDesc((std::u8string(obj->ShortDesc()) + u8" (left)").c_str());
    obj2->SetShortDesc((std::u8string(obj2->ShortDesc()) + u8" (right)").c_str());
    //    fprintf(stderr, u8"Duped: '%s'\n", obj2->ShortDesc());
  } else if (
      obj->Skill(prhash(u8"Wearable on Left Leg")) !=
      obj->Skill(prhash(u8"Wearable on Right Leg"))) {
    obj2 = new Object(*obj);
    //    fprintf(stderr, u8"Duped: '%s'\n", obj2->ShortDesc());
  } else if (
      obj->Skill(prhash(u8"Wearable on Left Arm")) !=
      obj->Skill(prhash(u8"Wearable on Right Arm"))) {
    obj2 = new Object(*obj);
    //    fprintf(stderr, u8"Duped: '%s'\n", obj2->ShortDesc());
  }
  return obj2;
}

void Object::TBAFinishMOB(Object* mob) {
  if (mob->Skill(prhash(u8"TBAGold"))) {
    Object* bag = new Object(mob);
    bag->SetShortDesc(u8"a TBAMUD purse");
    bag->SetDesc(u8"A mysterious purse that didn't seem to need to exist before.");

    bag->SetSkill(prhash(u8"Wearable on Left Hip"), 1);
    bag->SetSkill(prhash(u8"Wearable on Right Hip"), 2);
    bag->SetSkill(prhash(u8"Container"), 5 * 454);
    bag->SetSkill(prhash(u8"Capacity"), 5);
    bag->SetSkill(prhash(u8"Closeable"), 1);

    bag->SetWeight(1 * 454);
    bag->SetSize(2);
    bag->SetVolume(1);
    bag->SetValue(100);

    bag->SetSkill(prhash(u8"Perishable"), 1);
    mob->AddAct(act_t::WEAR_LHIP, bag);

    if (!gold)
      init_gold();
    Object* g = new Object(*gold);
    g->SetParent(bag);
    g->SetSkill(prhash(u8"Quantity"), mob->Skill(prhash(u8"TBAGold")));
    mob->SetSkill(prhash(u8"TBAGold"), 0);
  }

  if (mob->Skill(prhash(u8"TBAAttack"))) {
    if (mob->IsAct(act_t::WIELD)) {
      // fprintf(stderr, u8"Weapon def: %s\n", mob->ActTarg(act_t::WIELD)->Noun().c_str());
      if (mob->ActTarg(act_t::WIELD)->Skill(prhash(u8"WeaponType")) == 0) {
        if (!mob->ActTarg(act_t::HOLD)) { // Don't wield non-weapons, hold them
          fprintf(
              stderr,
              CYEL u8"Warning: Wielded non-weapon: %s\n" CNRM,
              mob->ActTarg(act_t::WIELD)->Noun().c_str());
          mob->AddAct(act_t::HOLD, mob->ActTarg(act_t::WIELD));
          mob->StopAct(act_t::WIELD);
        } else {
          fprintf(
              stderr,
              u8"Error: Wielded non-weapon with a held item: %s\n",
              mob->ActTarg(act_t::WIELD)->Noun().c_str());
        }
      } else {
        mob->SetSkill(
            get_weapon_skill(mob->ActTarg(act_t::WIELD)->Skill(prhash(u8"WeaponType"))),
            mob->Skill(prhash(u8"TBAAttack")));
      }
      if (mob->Skill(prhash(u8"NaturalWeapon")) == 13) { // Default (hit), but is armed!
        mob->SetSkill(prhash(u8"NaturalWeapon"), 0); // So remove it
      }
    } else {
      mob->SetSkill(prhash(u8"Punching"), mob->Skill(prhash(u8"TBAAttack")));
    }
    mob->SetSkill(prhash(u8"TBAAttack"), 0);
  }
  if (mob->Skill(prhash(u8"TBADefense"))) {
    if (mob->IsAct(act_t::WEAR_SHIELD)) {
      mob->SetSkill(prhash(u8"Shields"), mob->Skill(prhash(u8"TBADefense")));
    } else if (mob->Skill(prhash(u8"Punching"))) {
      mob->SetSkill(prhash(u8"Kicking"), mob->Skill(prhash(u8"TBADefense")));
    } else {
      mob->SetSkill(prhash(u8"Kicking"), mob->Skill(prhash(u8"TBADefense")) / 2);
      mob->SetSkill(
          prhash(u8"Punching"),
          mob->Skill(prhash(u8"TBADefense")) - mob->Skill(prhash(u8"Kicking")));
    }
    mob->SetSkill(prhash(u8"TBADefense"), 0);
  }

  if (Matches(u8"snake") || Matches(u8"spider") || Matches(u8"poisonous")) {
    SetSkill(prhash(u8"Poisonous"), NormAttribute(2));
  }
}

static Object *lastmob = nullptr, *lastbag = nullptr;
static std::map<int, Object*> lastobj;
void Object::TBALoadZON(const std::u8string& fn) {
  FILE* mudz = fopen(fn.c_str(), u8"r");
  if (mudz) {
    // fprintf(stderr, u8"Loading TBA Zone from \"%s\"\n", fn.c_str());
    for (int ctr = 0; ctr < 3; ++ctr) {
      fscanf(mudz, u8"%*[^\n\r]\n");
    }
    int done = 0;
    while (!done) {
      char8_t type;
      fscanf(mudz, u8" %c", &type);
      // fprintf(stderr, u8"Processing %c zone directive.\n", type);
      switch (type) {
        case ('S'): {
          done = 1;
        } break;
        case ('D'): { // Door state
          int dnum, room, state;
          fscanf(mudz, u8" %*d %d %d %d\n", &room, &dnum, &state);
          Object* door = nullptr;
          if (bynumwld.count(room) > 0)
            door = bynumwld[room]->PickObject(dirname[dnum], LOC_INTERNAL);
          if (door && state == 0) {
            door->SetSkill(prhash(u8"Open"), 1000);
            door->SetSkill(prhash(u8"Locked"), 0);
          } else if (door && state == 1) {
            door->SetSkill(prhash(u8"Open"), 0);
            door->SetSkill(prhash(u8"Locked"), 0);
          } else if (door && state == 2) {
            door->SetSkill(prhash(u8"Open"), 0);
            door->SetSkill(prhash(u8"Locked"), 1);
          }
        } break;
        case ('M'): {
          int num, room;
          fscanf(mudz, u8" %*d %d %*d %d %*[^\n\r]\n", &num, &room);
          if (bynumwld.count(room) && bynummob.count(num)) {
            Object* obj = new Object(bynumwld[room]);
            obj->SetShortDesc(u8"a TBAMUD MOB Popper");
            obj->SetDesc(u8"This thing just pops out MOBs.");

            // fprintf(stderr, u8"Put Mob \"%s\" in Room \"%s\"\n",
            // obj->ShortDescC(), bynumwld[room]->ShortDescC());

            if (lastmob)
              TBAFinishMOB(lastmob);
            lastmob = new Object(*(bynummob[num]));
            bynummobinst[num] = lastmob;
            lastmob->SetParent(obj);
            lastmob->AddAct(act_t::SPECIAL_MASTER, obj);
            obj->SetSkill(prhash(u8"TBAPopper"), 1);
            obj->SetSkill(prhash(u8"Invisible"), 1000);
            obj->Activate();
            lastbag = nullptr;
          }
        } break;
        case ('O'): {
          int num, room;
          fscanf(mudz, u8" %*d %d %*d %d %*[^\n\r]\n", &num, &room);
          if (bynumwld.count(room) && bynumobj.count(num)) {
            Object* obj = new Object(*(bynumobj[num]));
            obj->SetParent(bynumwld[room]);
            // fprintf(stderr, u8"Put Obj \"%s\" in Room \"%s\"\n",
            // obj->ShortDescC(), bynumwld[room]->ShortDescC());
            if (obj->HasSkill(prhash(u8"Liquid Source"))) {
              obj->Activate();
            }
            lastobj[num] = obj;
          }
        } break;
        case ('G'):
        case ('E'): {
          int num, posit = -1;
          if (type == 'E')
            fscanf(mudz, u8" %*d %d %*d %d%*[^\n\r]\n", &num, &posit);
          if (type == 'G')
            fscanf(mudz, u8" %*d %d %*d%*[^\n\r]\n", &num);
          if (lastmob && bynumobj.count(num)) {
            Object* obj = new Object(*(bynumobj[num]));
            Object* obj2 = dup_tba_obj(obj);
            obj->SetParent(lastmob);
            if (obj2)
              obj2->SetParent(lastmob);
            lastobj[num] = obj;

            int bagit = 0;
            switch (posit) {
              case (1): { // Worn
                lastmob->AddAct(act_t::WEAR_RFINGER, obj);
                if (obj->Skill(prhash(u8"Wearable on Right Finger")) == 0) {
                  fprintf(
                      stderr,
                      CYEL u8"%s:%d: Warning: Wear item wrong: %s\n" CNRM,
                      fn.c_str(),
                      fline(mudz),
                      obj->ShortDescC());
                }
              } break;
              case (2): { // Worn
                lastmob->AddAct(act_t::WEAR_LFINGER, obj);
                if (obj->Skill(prhash(u8"Wearable on Left Finger")) == 0) {
                  fprintf(
                      stderr,
                      CYEL u8"%s:%d: Warning: Wear item wrong: %s\n" CNRM,
                      fn.c_str(),
                      fline(mudz),
                      obj->ShortDescC());
                }
              } break;
              case (3): { // TBA MOBs have two necks (1/2)
                if (obj->Skill(prhash(u8"Wearable on Neck")) == 0) {
                  if (obj->Skill(prhash(u8"Wearable on Face")) == 0) {
                    fprintf(
                        stderr,
                        CYEL u8"%s:%d: Warning: Wear item wrong: %s\n" CNRM,
                        fn.c_str(),
                        fline(mudz),
                        obj->ShortDescC());
                  } else {
                    if (lastmob->IsAct(act_t::WEAR_FACE))
                      bagit = 1;
                    else
                      lastmob->AddAct(act_t::WEAR_FACE, obj);
                  }
                } else {
                  if (lastmob->IsAct(act_t::WEAR_NECK))
                    bagit = 1;
                  else
                    lastmob->AddAct(act_t::WEAR_NECK, obj);
                }
              } break;
              case (4): { // TBA MOBs have two necks (2/2)
                if (obj->Skill(prhash(u8"Wearable on Collar")) == 0) {
                  if (obj->Skill(prhash(u8"Wearable on Face")) == 0) {
                    fprintf(
                        stderr,
                        CYEL u8"%s:%d: Warning: Wear item wrong: %s\n" CNRM,
                        fn.c_str(),
                        fline(mudz),
                        obj->ShortDescC());
                  } else {
                    if (lastmob->IsAct(act_t::WEAR_FACE))
                      bagit = 1;
                    else
                      lastmob->AddAct(act_t::WEAR_FACE, obj);
                  }
                } else {
                  if (lastmob->IsAct(act_t::WEAR_COLLAR))
                    bagit = 1;
                  else
                    lastmob->AddAct(act_t::WEAR_COLLAR, obj);
                }
              } break;
              case (5): { // Worn
                lastmob->AddAct(act_t::WEAR_CHEST, obj);
                lastmob->AddAct(act_t::WEAR_BACK, obj);
                if (obj->Skill(prhash(u8"Wearable on Chest")) == 0) {
                  fprintf(
                      stderr,
                      CYEL u8"%s:%d: Warning: Wear item wrong: %s\n" CNRM,
                      fn.c_str(),
                      fline(mudz),
                      obj->ShortDescC());
                }
              } break;
              case (6): { // Worn
                if (obj->Skill(prhash(u8"Wearable on Head")) == 0) {
                  if (obj->Skill(prhash(u8"Wearable on Face")) == 0) {
                    fprintf(
                        stderr,
                        CYEL u8"%s:%d: Warning: Wear item wrong: %s\n" CNRM,
                        fn.c_str(),
                        fline(mudz),
                        obj->ShortDescC());
                  } else {
                    if (lastmob->IsAct(act_t::WEAR_FACE))
                      bagit = 1;
                    else
                      lastmob->AddAct(act_t::WEAR_FACE, obj);
                  }
                } else {
                  lastmob->AddAct(act_t::WEAR_HEAD, obj);
                }
              } break;
              case (7): { // Worn
                lastmob->AddAct(act_t::WEAR_LLEG, obj);
                if (obj2)
                  lastmob->AddAct(act_t::WEAR_RLEG, obj2);
                else
                  lastmob->AddAct(act_t::WEAR_RLEG, obj);
                if (obj->Skill(prhash(u8"Wearable on Left Leg")) == 0) {
                  fprintf(
                      stderr,
                      CYEL u8"%s:%d: Warning: Wear item wrong: %s\n" CNRM,
                      fn.c_str(),
                      fline(mudz),
                      obj->ShortDescC());
                }
              } break;
              case (8): { // Worn
                lastmob->AddAct(act_t::WEAR_LFOOT, obj);
                if (obj2)
                  lastmob->AddAct(act_t::WEAR_RFOOT, obj2);
                else
                  lastmob->AddAct(act_t::WEAR_RFOOT, obj);
                if (obj->Skill(prhash(u8"Wearable on Left Foot")) == 0) {
                  fprintf(
                      stderr,
                      CYEL u8"%s:%d: Warning: Wear item wrong: %s\n" CNRM,
                      fn.c_str(),
                      fline(mudz),
                      obj->ShortDescC());
                }
              } break;
              case (9): { // Worn
                lastmob->AddAct(act_t::WEAR_LHAND, obj);
                if (obj2)
                  lastmob->AddAct(act_t::WEAR_RHAND, obj2);
                else
                  lastmob->AddAct(act_t::WEAR_RHAND, obj);
                if (obj->Skill(prhash(u8"Wearable on Left Hand")) == 0) {
                  fprintf(
                      stderr,
                      CYEL u8"%s:%d: Warning: Wear item wrong: %s\n" CNRM,
                      fn.c_str(),
                      fline(mudz),
                      obj->ShortDescC());
                }
              } break;
              case (10): { // Worn
                lastmob->AddAct(act_t::WEAR_LARM, obj);
                if (obj2)
                  lastmob->AddAct(act_t::WEAR_RARM, obj2);
                else
                  lastmob->AddAct(act_t::WEAR_RARM, obj);
                if (obj->Skill(prhash(u8"Wearable on Left Arm")) == 0) {
                  fprintf(
                      stderr,
                      CYEL u8"%s:%d: Warning: Wear item wrong: %s\n" CNRM,
                      fn.c_str(),
                      fline(mudz),
                      obj->ShortDescC());
                }
              } break;
              case (11): { // Worn
                lastmob->AddAct(act_t::WEAR_SHIELD, obj);
                if (obj->Skill(prhash(u8"Wearable on Shield")) == 0) {
                  fprintf(
                      stderr,
                      CYEL u8"%s:%d: Warning: Wear item wrong: %s\n" CNRM,
                      fn.c_str(),
                      fline(mudz),
                      obj->ShortDescC());
                }
              } break;
              case (12): { // Worn
                lastmob->AddAct(act_t::WEAR_LSHOULDER, obj);
                lastmob->AddAct(act_t::WEAR_RSHOULDER, obj);
                if (obj->Skill(prhash(u8"Wearable on Left Shoulder")) == 0) {
                  fprintf(
                      stderr,
                      CYEL u8"%s:%d: Warning: Wear item wrong: %s\n" CNRM,
                      fn.c_str(),
                      fline(mudz),
                      obj->ShortDescC());
                }
              } break;
              case (13): { // Worn
                lastmob->AddAct(act_t::WEAR_WAIST, obj);
                if (obj->Skill(prhash(u8"Wearable on Waist")) == 0) {
                  fprintf(
                      stderr,
                      CYEL u8"%s:%d: Warning: Wear item wrong: %s\n" CNRM,
                      fn.c_str(),
                      fline(mudz),
                      obj->ShortDescC());
                }
              } break;
              case (14): { // Worn
                lastmob->AddAct(act_t::WEAR_RWRIST, obj);
                if (obj->Skill(prhash(u8"Wearable on Right Wrist")) == 0) {
                  fprintf(
                      stderr,
                      CYEL u8"%s:%d: Warning: Wear item wrong: %s\n" CNRM,
                      fn.c_str(),
                      fline(mudz),
                      obj->ShortDescC());
                }
              } break;
              case (15): { // Worn
                lastmob->AddAct(act_t::WEAR_LWRIST, obj);
                if (obj->Skill(prhash(u8"Wearable on Left Wrist")) == 0) {
                  fprintf(
                      stderr,
                      CYEL u8"%s:%d: Warning: Wear item wrong: %s\n" CNRM,
                      fn.c_str(),
                      fline(mudz),
                      obj->ShortDescC());
                }
              } break;
              case (16): { // Wielded
                lastmob->AddAct(act_t::WIELD, obj);
                if (obj->Skill(prhash(u8"WeaponType")) == 0) {
                  fprintf(
                      stderr,
                      CYEL u8"%s:%d: Warning: Wield non-weapon: %s\n" CNRM,
                      fn.c_str(),
                      fline(mudz),
                      obj->ShortDescC());
                }
              } break;
              case (17): { // Held
                lastmob->AddAct(act_t::HOLD, obj);
              } break;
              default: {
                bagit = 1;
              } break;
            }
            if (bagit) {
              if (!lastbag) {
                lastbag = new Object(lastmob);
                lastbag->SetShortDesc(u8"a TBAMUD bag");
                lastbag->SetDesc(u8"A mysterious bag that didn't seem to need to exist before.");

                lastbag->SetSkill(prhash(u8"Wearable on Right Hip"), 1);
                lastbag->SetSkill(prhash(u8"Wearable on Left Hip"), 2);
                lastbag->SetSkill(prhash(u8"Container"), 1000 * 454);
                lastbag->SetSkill(prhash(u8"Capacity"), 1000);
                lastbag->SetSkill(prhash(u8"Closeable"), 1);

                lastbag->SetWeight(5 * 454);
                lastbag->SetSize(1000);
                lastbag->SetVolume(5);
                lastbag->SetValue(200);

                lastbag->SetSkill(prhash(u8"Perishable"), 1);
                lastmob->AddAct(act_t::WEAR_RHIP, lastbag);
              }
              obj->Travel(lastbag);
              if (obj2)
                obj2->Travel(lastbag);
            }
          }
        } break;
        case ('P'): {
          int num, innum;
          fscanf(mudz, u8" %*d %d %*d %d %*[^\n\r]\n", &num, &innum);
          if (lastobj.count(innum) && bynumobj.count(num)) {
            Object* obj = new Object(*(bynumobj[num]));
            Object* obj2 = dup_tba_obj(obj);
            obj->SetParent(lastobj[innum]);
            if (obj2)
              obj2->SetParent(lastobj[innum]);
            // fprintf(stderr, u8"Put Obj \"%s\" in Obj \"%s\"\n", obj->ShortDescC(),
            // lastobj[innum]->ShortDescC());
            lastobj[num] = obj;
          }
        } break;
        default: {
          fscanf(mudz, u8"%*[^\n\r]\n");
        } break;
      }
    }
    if (lastmob)
      TBAFinishMOB(lastmob);
    fclose(mudz);
  }
}

void Object::TBALoadMOB(const std::u8string& fn) {
  if (mobroom == nullptr) {
    mobroom = new Object(this->World());
    mobroom->SetSkill(prhash(u8"Invisible"), 1000);
    mobroom->SetShortDesc(u8"The TBAMUD MOB Room");
  }
  FILE* mudm = fopen(fn.c_str(), u8"r");
  if (mudm) {
    // fprintf(stderr, u8"Loading TBA Mobiles from \"%s\"\n", fn.c_str());
    while (1) {
      int onum;
      if (fscanf(mudm, u8" #%d\n", &onum) < 1)
        break;
      // fprintf(stderr, u8"Loaded MOB #%d\n", onum);

      Object* obj = new Object(mobroom);
      obj->SetSkill(prhash(u8"TBAMOB"), 1000000 + onum);
      bynummob[onum] = obj;

      std::vector<std::u8string_view> aliases;
      memset(buf, 0, 65536); // Alias List
      fscanf(mudm, u8"%65535[^~\n]~\n", buf);
      std::u8string buffer(buf, strlen(buf));
      std::transform(buffer.begin(), buffer.end(), buffer.begin(), ascii_tolower);
      std::u8string_view line = buffer;

      size_t lbeg = 0;
      size_t lend = 0;
      do {
        lbeg = line.find_first_not_of(u8" \t\r", lend);
        if (lbeg != std::u8string::npos) {
          lend = line.find_first_of(u8" \t\r", lbeg);
          if (lend != std::u8string::npos) {
            aliases.emplace_back(line.substr(lbeg, lend - lbeg));
          } else {
            aliases.emplace_back(line.substr(lbeg));
          }
        }
      } while (lbeg != std::u8string::npos && lend != std::u8string::npos);

      obj->SetShortDesc(load_tba_field(mudm));
      // fprintf(stderr, u8"Loaded TBA Mobile with Name = %s\n", buf);

      std::u8string label = u8"";
      for (unsigned int actr = 0; actr < aliases.size(); ++actr) {
        if (!obj->Matches(std::u8string(aliases[actr]))) {
          if (aliases[actr].find_first_not_of(target_chars) != std::u8string::npos) {
            fprintf(
                stderr,
                CYEL u8"Warning: Ignoring non-alpha alias [%s] in #%d ('%s')\n" CNRM,
                std::u8string(aliases[actr]).c_str(),
                obj->Skill(prhash(u8"TBAMOB")),
                obj->ShortDescC());
          } else if (aliases[actr] == u8"woman" || aliases[actr] == u8"girl") {
            obj->SetGender('F');
          } else if (aliases[actr] == u8"man" || aliases[actr] == u8"boy") {
            obj->SetGender('M');
          } else if (aliases[actr] == u8"wyv" || aliases[actr] == u8"ghenna") {
            // Ignore these, they're just typing short-cuts.
          } else if (
              aliases[actr] == u8"guard" && aliases.size() > (actr + 1) &&
              aliases[actr + 1] == u8"royal") {
            // Auto-fix royal guards who just show as u8"Jim" etc.
            obj->SetShortDesc(obj->ShortDescS() + u8" the Royal Guard");
          } else if (aliases[actr] == u8"guildguard") {
            // Auto-fix guildguards who don't show they are that
            if (obj->ShortDesc().substr(0, 4) == u8"the ")
              obj->SetShortDesc(obj->ShortDescS() + u8" guildguard");
            else
              obj->SetShortDesc(obj->ShortDescS() + u8" the guildguard");
          } else {
            // fprintf(
            //    stderr,
            //    CYEL u8"Warning: Adding [%s] to #%d ('%s')\n" CNRM,
            //    std::u8string(aliases[actr]).c_str(),
            //    obj->Skill(prhash(u8"TBAMOB")),
            //    obj->ShortDescC());
            label += u8" ";
            label += aliases[actr];
            ++mob_aliases;
          }
        }
      }
      if (!label.empty()) {
        label[0] = '(';
        label += ')';
        obj->SetShortDesc(obj->ShortDescS() + u8" " + label);
      }

      obj->SetDesc(load_tba_field(mudm));
      // fprintf(stderr, u8"Loaded TBA Mobile with Desc = %s\n", buf);

      auto field = load_tba_field(mudm);
      if (field.length() > 0) {
        if (field[0] != '.') {
          obj->SetLongDesc(field);
        } else { // Hidden MOBs
          obj->SetSkill(prhash(u8"Hidden"), 10);
          obj->SetLongDesc(field.substr(1));
        }
      }
      // fprintf(stderr, u8"Loaded TBA Mobile with LongDesc = %s\n", buf);

      obj->SetPos(pos_t::STAND);
      obj->SetAttribute(0, 3);
      obj->SetAttribute(1, 3);
      obj->SetAttribute(2, 3);
      obj->SetAttribute(3, 3);
      obj->SetAttribute(4, 3);
      obj->SetAttribute(5, 3);

      int aware = 0, hidden = 0, sneak = 0;
      int val, val2, val3;
      char8_t tp;
      memset(buf, 0, 65536);
      fscanf(mudm, u8"%65535[^ \t\n]", buf); // Rest of line read below...

      obj->SetSkill(prhash(u8"TBAAction"), 8); // IS_NPC - I'll use it to see if(MOB)
      if (strcasestr8(buf, u8"b") || (atoi(buf) & 2)) { // SENTINEL
        obj->SetSkill(prhash(u8"TBAAction"), obj->Skill(prhash(u8"TBAAction")) | 2);
      }
      if (strcasestr8(buf, u8"c") || (atoi(buf) & 4)) { // SCAVENGER
        obj->SetSkill(prhash(u8"TBAAction"), obj->Skill(prhash(u8"TBAAction")) | 4);
      }
      if (strcasestr8(buf, u8"e") || (atoi(buf) & 16)) { // AWARE
        aware = 1;
      }
      if (strcasestr8(buf, u8"f") || (atoi(buf) & 32)) { // AGGRESSIVE
        obj->SetSkill(prhash(u8"TBAAction"), obj->Skill(prhash(u8"TBAAction")) | 32);
      }
      if (strcasestr8(buf, u8"g") || (atoi(buf) & 64)) { // STAY_ZONE
        obj->SetSkill(prhash(u8"TBAAction"), obj->Skill(prhash(u8"TBAAction")) | 64);
      }
      if (strcasestr8(buf, u8"h") || (atoi(buf) & 128)) { // WIMPY
        obj->SetSkill(prhash(u8"TBAAction"), obj->Skill(prhash(u8"TBAAction")) | 128);
      }
      if (strcasestr8(buf, u8"l") || (atoi(buf) & 2048)) { // MEMORY
        obj->SetSkill(prhash(u8"TBAAction"), obj->Skill(prhash(u8"TBAAction")) | 2048);
      }
      if (strcasestr8(buf, u8"m") || (atoi(buf) & 4096)) { // HELPER
        obj->SetSkill(prhash(u8"TBAAction"), obj->Skill(prhash(u8"TBAAction")) | 4096);
      }
      // FIXME: Add others here.

      memset(buf, 0, 65536);
      fscanf(mudm, u8" %*s %*s %*s %65535[^ \t\n]", buf); // Rest of line read below...
      if (strcasestr8(buf, u8"g") || (atoi(buf) & 64)) { // WATERWALK
        obj->SetSkill(prhash(u8"TBAAffection"), obj->Skill(prhash(u8"TBAAffection")) | 64);
      }
      if (strcasestr8(buf, u8"s") || (atoi(buf) & 262144)) { // SNEAK
        sneak = 1;
      }
      if (strcasestr8(buf, u8"t") || (atoi(buf) & 524288)) { // HIDE
        hidden = 1;
      }
      // FIXME: Implement special powers of MOBs here.

      memset(buf, 0, 65536);
      fscanf(mudm, u8" %*s %*s %*s %d %c\n", &val, &tp);
      if (val > 0)
        obj->SetSkill(prhash(u8"Honor"), val);
      else
        obj->SetSkill(prhash(u8"Dishonor"), -val);

      obj->SetSkill(prhash(u8"Accomplishment"), 1000000 + onum);

      if (tp == 'E' || tp == 'S') {
        fscanf(mudm, u8"%d %d %d", &val, &val2, &val3);
        for (int ctr = 0; ctr < val; ++ctr)
          obj->SetAttribute(ctr % 6, obj->NormAttribute(ctr % 6) + 1); // val1 = Level
        obj->SetSkill(prhash(u8"TBAAttack"), ((20 - val2) / 3) + 3); // val2 = THAC0
        obj->SetSkill(prhash(u8"TBADefense"), ((10 - val3) / 3) + 3); // val2 = AC

        fscanf(mudm, u8" %dd%d+%d", &val, &val2, &val3); // Hit Points
        val = (val * (val2 + 1) + 1) / 2 + val3;
        obj->SetAttribute(0, (val + 49) / 50); // Becomes Body

        fscanf(mudm, u8" %dd%d+%d\n", &val, &val2, &val3); // Barehand Damage
        val = (val * (val2 + 1) + 1) / 2 + val3;
        obj->SetAttribute(2, (val / 3) + 3); // Becomes Strength

        fscanf(mudm, u8"%d", &val); // Gold
        obj->SetSkill(prhash(u8"TBAGold"), val);

        fscanf(mudm, u8"%*[^\n\r]\n"); // XP //FIXME: Worth Karma?

        fscanf(mudm, u8"%d %d %d\n", &val, &val2, &val3);

        if (val == 4) { // Mob Starts off Sleeping
          obj->SetPos(pos_t::LIE);
          obj->AddAct(act_t::SLEEP);
        } else if (val == 5) { // Mob Starts off Resting
          obj->SetPos(pos_t::SIT);
          obj->AddAct(act_t::REST);
        } else if (val == 6) { // Mob Starts off Sitting
          obj->SetPos(pos_t::SIT);
        }

        static char8_t genderlist[] = {'N', 'M', 'F'};
        obj->gender = genderlist[val3];
      }

      obj->SetSkill(prhash(u8"NaturalWeapon"), 13); //"Hits" (is default in TBA)
      memset(buf, 0, 65536);
      while (tp == 'E') { // Basically an if with an infinite loop ;)
        if (fscanf(mudm, u8"Con: %d\n", &val))
          obj->SetAttribute(0, std::max(obj->NormAttribute(0), (val / 3) + 3));

        else if (fscanf(mudm, u8"Dex: %d\n", &val))
          obj->SetAttribute(1, std::max(obj->NormAttribute(1), (val / 3) + 3));

        else if (fscanf(mudm, u8"Str: %d\n", &val))
          obj->SetAttribute(2, std::max(obj->NormAttribute(2), (val / 3) + 3));

        else if (fscanf(mudm, u8"ha: %d\n", &val)) //'Cha' minus 'Con' Conflict!
          obj->SetAttribute(3, std::max(obj->NormAttribute(3), (val / 3) + 3));

        else if (fscanf(mudm, u8"Int: %d\n", &val))
          obj->SetAttribute(4, std::max(obj->NormAttribute(4), (val / 3) + 3));

        else if (fscanf(mudm, u8"Wis: %d\n", &val))
          obj->SetAttribute(5, std::max(obj->NormAttribute(5), (val / 3) + 3));

        else if (fscanf(mudm, u8"Add: %d\n", &val))
          ; //'StrAdd' - Do Nothing

        else if (fscanf(mudm, u8"BareHandAttack: %d\n", &val)) {
          if (val == 13)
            val = 0; // Punches (is the Default in Acid)
          obj->SetSkill(prhash(u8"NaturalWeapon"), val);
        }

        else
          break;
      }
      fscanf(mudm, u8" E"); // Nuke the terminating u8"E", if present.

      obj->SetWeight(obj->NormAttribute(0) * 20000);
      obj->SetSize(1000 + obj->NormAttribute(0) * 200);
      obj->SetVolume(100);
      obj->SetValue(-1);

      if (aware) { // Perception = Int
        obj->SetSkill(prhash(u8"Perception"), obj->NormAttribute(4));
      }

      if (sneak && hidden) { // Stealth = 3Q/2
        obj->SetSkill(prhash(u8"Stealth"), (3 * obj->NormAttribute(1) + 1) / 2);
      } else if (hidden) { // Stealth = Q
        obj->SetSkill(prhash(u8"Stealth"), obj->NormAttribute(1));
      } else if (sneak) { // Stealth = Q/2
        obj->SetSkill(prhash(u8"Stealth"), (obj->NormAttribute(1) + 1) / 2);
      }

      if (hidden) {
        obj->SetSkill(prhash(u8"Hidden"), obj->Skill(prhash(u8"Stealth")) * 2);
      }
      if (sneak) {
        obj->StartUsing(prhash(u8"Stealth"));
      }

      int tnum;
      while (fscanf(mudm, u8" T %d\n", &tnum) > 0) {
        if (tnum > 0 && bynumtrg.count(tnum) > 0) {
          Object* trg = new Object(*(bynumtrg[tnum]));
          trg->SetParent(obj);
          todotrg.push_back(trg);
          //  fprintf(stderr, u8"Put Trg \"%s\" on MOB \"%s\"\n",
          //	trg->DescC(), obj->ShortDescC()
          //	);
        }
      }

      fscanf(mudm, u8" %*[^#$]");
    }
    fclose(mudm);
  }
}

static void add_tba_spell(Object* obj, int spell, int power) {
  switch (spell) {
    case (-1): { // No Effect
    } break;
    case (1): { // ARMOR
      obj->SetSkill(prhash(u8"Resilience Spell"), power);
    } break;
    case (2): { // TELEPORT
      obj->SetSkill(prhash(u8"Teleport Spell"), power);
    } break;
    case (3): { // BLESS
      obj->SetSkill(prhash(u8"Luck Spell"), power);
    } break;
    case (4): { // BLINDNESS
      obj->SetSkill(prhash(u8"Blind Spell"), power);
    } break;
    case (5): { // BURNING HANDS
      obj->SetSkill(prhash(u8"Fire Dart Spell"), power);
    } break;
    case (6): { // CALL LIGHTNING
      obj->SetSkill(prhash(u8"Lightning Bolt Spell"), power);
    } break;
    case (7): { // CHARM
      obj->SetSkill(prhash(u8"Influence Spell"), power);
    } break;
    case (8): { // CHILL TOUCH
      obj->SetSkill(prhash(u8"Injure Spell"), power);
    } break;
    case (9): { // CLONE
      obj->SetSkill(prhash(u8"Copy Book Spell"), power);
    } break;
    case (10): { // COLOR SPRAY
      obj->SetSkill(prhash(u8"Distract Spell"), power);
    } break;
    case (11): { // CONTROL WEATHER
      obj->SetSkill(prhash(u8"Clear Weather Spell"), power);
    } break;
    case (12): { // CREATE FOOD
      obj->SetSkill(prhash(u8"Create Food Spell"), power);
    } break;
    case (13): { // CREATE WATER
      obj->SetSkill(prhash(u8"Create Water Spell"), power);
    } break;
    case (14): { // CURE BLIND
      obj->SetSkill(prhash(u8"Cure Blindness Spell"), power);
    } break;
    case (15): { // CURE_CRITIC
      obj->SetSkill(prhash(u8"Heal Spell"), power);
    } break;
    case (16): { // CURE_LIGHT
      obj->SetSkill(prhash(u8"Energize Spell"), power);
    } break;
    case (17): { // CURSE
      obj->SetSkill(prhash(u8"Misfortune Spell"), power);
    } break;
    case (18): { // DETECT ALIGN
      obj->SetSkill(prhash(u8"Identify Character Spell"), power);
    } break;
    case (19): { // DETECT INVIS
      obj->SetSkill(prhash(u8"Heat Vision Spell"), power);
    } break;
    case (20): { // DETECT MAGIC
      obj->SetSkill(prhash(u8"Detect Cursed Items Spell"), power);
    } break;
    case (21): { // DETECT POISON
      obj->SetSkill(prhash(u8"Detect Poison Spell"), power);
    } break;
    case (22): { // DETECT EVIL
      obj->SetSkill(prhash(u8"Identify Person Spell"), power);
    } break;
    case (23): { // EARTHQUAKE
      obj->SetSkill(prhash(u8"Earthquake Spell"), power);
    } break;
    case (24): { // ENCHANT WEAPON
      obj->SetSkill(prhash(u8"Force Sword Spell"), power);
    } break;
    case (25): { // ENERGY DRAIN
      obj->SetSkill(prhash(u8"Weaken Subject Spell"), power);
    } break;
    case (26): { // FIREBALL
      obj->SetSkill(prhash(u8"Fireball Spell"), power);
    } break;
    case (27): { // HARM
      obj->SetSkill(prhash(u8"Harm Spell"), power);
    } break;
    case (28): { // HEAL
      obj->SetSkill(prhash(u8"Heal Spell"), power);
      obj->SetSkill(prhash(u8"Energize Spell"), power);
    } break;
    case (29): { // INVISIBLE
      obj->SetSkill(prhash(u8"Invisibility Spell"), power);
    } break;
    case (30): { // LIGHTNING BOLT
      obj->SetSkill(prhash(u8"Fire Burst Spell"), power);
    } break;
    case (31): { // LOCATE OBJECT
      obj->SetSkill(prhash(u8"Locate Object Spell"), power);
    } break;
    case (32): { // MAGIC MISSILE
      obj->SetSkill(prhash(u8"Force Arrow Spell"), power);
    } break;
    case (33): { // POISON
      obj->SetSkill(prhash(u8"Poisonous"), power);
    } break;
    case (34): { // PROT FROM EVIL
      obj->SetSkill(prhash(u8"Personal Shield Spell"), power);
    } break;
    case (35): { // REMOVE CURSE
      obj->SetSkill(prhash(u8"Remove Curse Spell"), power);
    } break;
    case (36): { // SANCTUARY
      obj->SetSkill(prhash(u8"Treatment Spell"), power);
    } break;
    case (37): { // SHOCKING GRASP
      obj->SetSkill(prhash(u8"Spark Spell"), power);
    } break;
    case (38): { // SLEEP
      obj->SetSkill(prhash(u8"Sleep Other Spell"), power);
    } break;
    case (39): { // STRENGTH
      obj->SetSkill(prhash(u8"Strength Spell"), power);
    } break;
    case (40): { // SUMMON
      obj->SetSkill(prhash(u8"Summon Creature Spell"), power);
    } break;
    case (41): { // VENTRILOQUATE
      obj->SetSkill(prhash(u8"Translate Spell"), power);
    } break;
    case (42): { // WORD OF RECALL
      obj->SetSkill(prhash(u8"Recall Spell"), power);
    } break;
    case (43): { // REMOVE_POISON
      obj->SetSkill(prhash(u8"Cure Poison Spell"), power);
    } break;
    case (44): { // SENSE LIFE
      obj->SetSkill(prhash(u8"Light Spell"), power);
    } break;
    case (45): { // ANIMATE DEAD
      obj->SetSkill(prhash(u8"Create Zombie Spell"), power);
    } break;
    case (46): { // DISPEL GOOD??
      obj->SetSkill(prhash(u8"Protection Spell"), power);
    } break;
    case (47): { // GROUP ARMOR
      obj->SetSkill(prhash(u8"Group Resilience Spell"), power);
    } break;
    case (48): { // GROUP HEAL
      obj->SetSkill(prhash(u8"Heal Group Spell"), power);
    } break;
    case (49): { // GROUP RECALL
      obj->SetSkill(prhash(u8"Recall Group Spell"), power);
    } break;
    case (50): { // INFRAVISION
      obj->SetSkill(prhash(u8"Dark Vision Spell"), power);
    } break;
    case (51): { // WATERWALK
      obj->SetSkill(prhash(u8"Float Spell"), power);
    } break;
    case (52):
    case (201): { // IDENTIFY
      obj->SetSkill(prhash(u8"Identify Spell"), power);
    } break;
    case (53): { // FLY
      obj->SetSkill(prhash(u8"Fly Spell"), power);
    } break;
    case (54): { // DARKNESS
      obj->SetSkill(prhash(u8"Darkness Spell"), power);
    } break;
    default: {
      fprintf(stderr, CYEL u8"Warning: Unhandled CicleMUD Spell: %d\n" CNRM, spell);
    }
  }
}

void Object::TBALoadOBJ(const std::u8string& fn) {
  if (objroom == nullptr) {
    objroom = new Object(this->World());
    objroom->SetSkill(prhash(u8"Invisible"), 1000);
    objroom->SetShortDesc(u8"The TBAMUD Object Room");
  }
  FILE* mudo = fopen(fn.c_str(), u8"r");
  if (mudo) {
    // fprintf(stderr, u8"Loading TBA Objects from \"%s\"\n", fn.c_str());
    while (1) {
      int onum;
      int valmod = 1000, powmod = 1;
      if (fscanf(mudo, u8" #%d\n", &onum) < 1)
        break;
      // fprintf(stderr, u8"Loaded object #%d\n", onum);

      Object* obj = new Object(objroom);
      obj->SetSkill(prhash(u8"TBAObject"), 1000000 + onum);
      bynumobj[onum] = obj;

      std::vector<std::u8string_view> aliases;
      memset(buf, 0, 65536); // Alias List
      fscanf(mudo, u8"%65535[^~\n]~\n", buf);
      std::u8string buffer(buf, strlen(buf));
      std::transform(buffer.begin(), buffer.end(), buffer.begin(), ascii_tolower);
      std::u8string_view line = buffer;

      size_t lbeg = 0;
      size_t lend = 0;
      do {
        lbeg = line.find_first_not_of(u8" \t\r", lend);
        if (lbeg != std::u8string::npos) {
          lend = line.find_first_of(u8" \t\r", lbeg);
          if (lend != std::u8string::npos) {
            aliases.emplace_back(line.substr(lbeg, lend - lbeg));
          } else {
            aliases.emplace_back(line.substr(lbeg));
          }
        }
      } while (lbeg != std::u8string::npos && lend != std::u8string::npos);

      obj->SetShortDesc(load_tba_field(mudo));
      // fprintf(stderr, u8"Loaded TBA Object with Name = %s\n", buf);

      std::u8string label = u8"";
      for (unsigned int actr = 0; actr < aliases.size(); ++actr) {
        if (!obj->Matches(std::u8string(aliases[actr]))) {
          if (aliases[actr].find_first_not_of(target_chars) != std::u8string::npos) {
            fprintf(
                stderr,
                CYEL u8"Warning: Ignoring non-alpha alias [%s] in #%d ('%s')\n" CNRM,
                std::u8string(aliases[actr]).c_str(),
                obj->Skill(prhash(u8"TBAObject")),
                obj->ShortDescC());
          } else if (aliases[actr] == u8"wyv" || aliases[actr] == u8"ghenna") {
            // Ignore these, they're just typing short-cuts.
          } else if (
              aliases[actr] == u8"water" || aliases[actr] == u8"beer" || aliases[actr] == u8"ale" ||
              aliases[actr] == u8"wine" || aliases[actr] == u8"whisky" ||
              aliases[actr] == u8"milk") {
            // Ignore these, they're usually referring to the item that *should* be inside them.
          } else {
            // fprintf(
            //    stderr,
            //    CYEL u8"Warning: Adding [%s] to #%d ('%s')\n" CNRM,
            //    std::u8string(aliases[actr]).c_str(),
            //    obj->Skill(prhash(u8"TBAObject")),
            //    obj->ShortDescC());
            label += u8" ";
            label += aliases[actr];
            ++obj_aliases;
          }
        }
      }
      if (!label.empty()) {
        label[0] = '(';
        label += ')';
        obj->SetShortDesc(obj->ShortDescS() + u8" " + label);
      }

      auto field = load_tba_field(mudo);
      if (field.length() > 0) {
        if (field[0] != '.') {
          obj->SetLongDesc(field);
        } else { // Hidden Objects
          obj->SetSkill(prhash(u8"Hidden"), 10);
          obj->SetLongDesc(field.substr(1));
        }
      }
      // fprintf(stderr, u8"Loaded TBA Object with Desc = %s\n", buf);

      fscanf(mudo, u8"%*[^\n\r]\n");

      int tp = 0, val[4];
      memset(buf, 0, 65536);
      fscanf(mudo, u8"%d %65535[^ \n\t]", &tp, buf); // Effects Bitvector
      if (strcasestr8(buf, u8"a") || (atoi(buf) & 1)) { // GLOW
        obj->SetSkill(prhash(u8"Light Source"), 10);
      }
      if (strcasestr8(buf, u8"b") || (atoi(buf) & 2)) { // HUM
        obj->SetSkill(prhash(u8"Noise Source"), 10);
      }
      //      if(strcasestr8(buf, u8"c") || (atoi(buf) & 4)) { //NORENT
      //	}
      //      if(strcasestr8(buf, u8"d") || (atoi(buf) & 8)) { //NODONATE
      //	}
      if (strcasestr8(buf, u8"e") || (atoi(buf) & 16)) { // NOINVIS
        obj->SetSkill(prhash(u8"Obvious"), 1000);
      }
      if (strcasestr8(buf, u8"f") || (atoi(buf) & 32)) { // INVISIBLE
        obj->SetSkill(prhash(u8"Invisible"), 10);
      }
      if (strcasestr8(buf, u8"g") || (atoi(buf) & 64)) { // MAGIC
        obj->SetSkill(prhash(u8"Magical"), 10);
      }
      if (strcasestr8(buf, u8"h") || (atoi(buf) & 128)) { // NODROP
        obj->SetSkill(prhash(u8"Cursed"), 10);
      }
      if (strcasestr8(buf, u8"i") || (atoi(buf) & 256)) { // BLESS
        obj->SetSkill(prhash(u8"Blessed"), 10);
      }
      //      if(strcasestr8(buf, u8"j") || (atoi(buf) & 512)) { //ANTI_GOOD
      //	}
      //      if(strcasestr8(buf, u8"k") || (atoi(buf) & 1024)) { //ANTI_EVIL
      //	}
      //      if(strcasestr8(buf, u8"l") || (atoi(buf) & 2048)) { //ANTI_NEUTRAL
      //	}
      //      if(strcasestr8(buf, u8"m") || (atoi(buf) & 4096)) { //ANTI_MAGIC_USER
      //	}
      //      if(strcasestr8(buf, u8"n") || (atoi(buf) & 8192)) { //ANTI_CLERIC
      //	}
      //      if(strcasestr8(buf, u8"o") || (atoi(buf) & 16384)) { //ANTI_THIEF
      //	}
      //      if(strcasestr8(buf, u8"p") || (atoi(buf) & 32768)) { //ANTI_WARRIOR
      //	}
      if (strcasestr8(buf, u8"q") || (atoi(buf) & 65536)) { // NOSELL
        obj->SetSkill(prhash(u8"Priceless"), 1);
      }

      // Wear Bitvector
      memset(buf, 0, 65536);
      fscanf(mudo, u8"%*s %*s %*s %65535[^ \n\t]%*[^\n\r]\n", buf);
      if (strcasestr8(buf, u8"a") || (atoi(buf) & 1)) { // TAKE
        obj->SetPos(pos_t::LIE);
      }

      int sf = 0;
      if (!strncmp(obj->ShortDescC(), u8"a pair of ", 10))
        sf = 9;
      else if (!strncmp(obj->ShortDescC(), u8"some ", 5))
        sf = 4;
      else if (!strncmp(obj->ShortDescC(), u8"a set of ", 9))
        sf = 8;

      std::u8string name = obj->ShortDescS();
      if (strcasestr8(buf, u8"b") || (atoi(buf) & 2)) {
        obj->SetSkill(prhash(u8"Wearable on Left Finger"), 1); // Two Alternatives
        obj->SetSkill(prhash(u8"Wearable on Right Finger"), 2);
      }
      if (strcasestr8(buf, u8"c") || (atoi(buf) & 4)) {
        if (matches(name.c_str(), u8"mask") || matches(name.c_str(), u8"sunglasses") ||
            matches(name.c_str(), u8"eyeglasses") || matches(name.c_str(), u8"spectacles") ||
            matches(name.c_str(), u8"glasses") || matches(name.c_str(), u8"goggles") ||
            matches(name.c_str(), u8"visor") || matches(name.c_str(), u8"eyelets")) {
          obj->SetSkill(prhash(u8"Wearable on Face"), 1);
        } else {
          obj->SetSkill(prhash(u8"Wearable on Neck"), 1);
          obj->SetSkill(prhash(u8"Wearable on Collar"), 2);
        }
      }
      if (strcasestr8(buf, u8"d") || (atoi(buf) & 8)) {
        obj->SetSkill(prhash(u8"Wearable on Chest"), 1);
        obj->SetSkill(prhash(u8"Wearable on Back"), 1);
        if (matches(name.c_str(), u8"suit of")) {
          obj->SetSkill(prhash(u8"Wearable on Right Leg"), 1);
          obj->SetSkill(prhash(u8"Wearable on Left Leg"), 1);
          obj->SetSkill(prhash(u8"Wearable on Right Arm"), 1);
          obj->SetSkill(prhash(u8"Wearable on Left Arm"), 1);
          valmod *= 5;
        }
      }
      if (strcasestr8(buf, u8"e") || (atoi(buf) & 16)) {
        if (matches(name.c_str(), u8"mask") || matches(name.c_str(), u8"sunglasses") ||
            matches(name.c_str(), u8"eyeglasses") || matches(name.c_str(), u8"spectacles") ||
            matches(name.c_str(), u8"glasses") || matches(name.c_str(), u8"goggles") ||
            matches(name.c_str(), u8"visor") || matches(name.c_str(), u8"eyelets")) {
          obj->SetSkill(prhash(u8"Wearable on Face"), 1);
        } else {
          obj->SetSkill(prhash(u8"Wearable on Head"), 1);
        }
      }
      if (strcasestr8(buf, u8"f") || (atoi(buf) & 32)) {
        obj->SetSkill(prhash(u8"Wearable on Left Leg"), 1);
        if (sf) {
          if (!strcmp(name.c_str() + (name.length() - 9), u8" leggings"))
            name = std::u8string(u8"a") + name.substr(sf, name.length() - (sf + 1));
          else if (!strcmp(name.c_str() + (name.length() - 7), u8" plates"))
            name = std::u8string(u8"a") + name.substr(sf, name.length() - (sf + 1));
          else
            obj->SetSkill(prhash(u8"Wearable on Right Leg"), 1);
        } else
          obj->SetSkill(prhash(u8"Wearable on Right Leg"), 1);
        if (!obj->Skill(prhash(u8"Wearable on Right Leg"))) { // Reversable
          obj->SetSkill(prhash(u8"Wearable on Right Leg"), 2);
          valmod /= 2;
          powmod = 2;
        }
      }
      if (strcasestr8(buf, u8"g") || (atoi(buf) & 64)) {
        obj->SetSkill(prhash(u8"Wearable on Left Foot"), 1);
        if (sf) {
          if (!strcmp(name.c_str() + (name.length() - 8), u8" sandals"))
            name = std::u8string(u8"a") + name.substr(sf, name.length() - (sf + 1));
          else if (!strcmp(name.c_str() + (name.length() - 6), u8" boots"))
            name = std::u8string(u8"a") + name.substr(sf, name.length() - (sf + 1));
          else if (!strcmp(name.c_str() + (name.length() - 6), u8" shoes"))
            name = std::u8string(u8"a") + name.substr(sf, name.length() - (sf + 1));
          else
            obj->SetSkill(prhash(u8"Wearable on Right Foot"), 1);
        } else
          obj->SetSkill(prhash(u8"Wearable on Right Foot"), 1);
        if (!obj->Skill(prhash(u8"Wearable on Right Foot"))) {
          valmod /= 2;
          powmod = 2;
        }
      }
      if (strcasestr8(buf, u8"h") || (atoi(buf) & 128)) {
        obj->SetSkill(prhash(u8"Wearable on Left Hand"), 1);
        if (sf) {
          if (!strcmp(name.c_str() + (name.length() - 10), u8" gauntlets"))
            name = std::u8string(u8"a") + name.substr(sf, name.length() - (sf + 1));
          else if (!strcmp(name.c_str() + (name.length() - 7), u8" gloves"))
            name = std::u8string(u8"a") + name.substr(sf, name.length() - (sf + 1));
          else
            obj->SetSkill(prhash(u8"Wearable on Right Hand"), 1);
        } else
          obj->SetSkill(prhash(u8"Wearable on Right Hand"), 1);
        if (!obj->Skill(prhash(u8"Wearable on Right Hand"))) {
          valmod /= 2;
          powmod = 2;
        }
      }
      if (strcasestr8(buf, u8"i") || (atoi(buf) & 256)) {
        obj->SetSkill(prhash(u8"Wearable on Left Arm"), 1);
        if (sf) {
          if (!strcmp(name.c_str() + (name.length() - 8), u8" sleeves"))
            name = std::u8string(u8"a") + name.substr(sf, name.length() - (sf + 1));
          else if (!strcmp(name.c_str() + (name.length() - 8), u8" bracers"))
            name = std::u8string(u8"a") + name.substr(sf, name.length() - (sf + 1));
          else if (!strcmp(name.c_str() + (name.length() - 7), u8" plates"))
            name = std::u8string(u8"a") + name.substr(sf, name.length() - (sf + 1));
          else
            obj->SetSkill(prhash(u8"Wearable on Right Arm"), 1);
        } else
          obj->SetSkill(prhash(u8"Wearable on Right Arm"), 1);
        if (!obj->Skill(prhash(u8"Wearable on Right Arm"))) { // Reversable
          obj->SetSkill(prhash(u8"Wearable on Right Arm"), 2);
          valmod /= 2;
          powmod = 2;
        }
      }
      if (strcasestr8(buf, u8"j") || (atoi(buf) & 512)) {
        obj->SetSkill(prhash(u8"Wearable on Shield"), 1); // FIXME: Wear Shield?
      }
      if (strcasestr8(buf, u8"k") || (atoi(buf) & 1024)) {
        obj->SetSkill(prhash(u8"Wearable on Left Shoulder"), 1);
        obj->SetSkill(prhash(u8"Wearable on Right Shoulder"), 1);
      }
      if (strcasestr8(buf, u8"l") || (atoi(buf) & 2048)) {
        obj->SetSkill(prhash(u8"Wearable on Waist"), 1);
      }
      if (strcasestr8(buf, u8"m") || (atoi(buf) & 4096)) {
        obj->SetSkill(prhash(u8"Wearable on Left Wrist"), 1);
        obj->SetSkill(prhash(u8"Wearable on Right Wrist"), 2);
      }
      obj->SetShortDesc(name.c_str());

      fscanf(mudo, u8"%d %d %d %d\n", val + 0, val + 1, val + 2, val + 3);

      if (tp == 1) { // LIGHTS
        if (val[2] > 1) {
          obj->SetSkill(prhash(u8"Lightable"), val[2] * 60); // Total Lit Minutes
          obj->SetSkill(prhash(u8"Brightness"), 100); // All TBAMUD Lights
          if (matches(name.c_str(), u8"lantern")) {
            obj->SetSkill(prhash(u8"Resilience"), 800); // May last when dropped
          } else if (matches(name.c_str(), u8"torch")) {
            obj->SetSkill(prhash(u8"Resilience"), 200); // Won't last when dropped
          } else if (matches(name.c_str(), u8"candle")) {
            // obj->SetSkill(prhash(u8"Resilience"), 0); // Goes out when dropped
          } else {
            obj->SetSkill(prhash(u8"Resilience"), 100); // Won't last when dropped
          }
        } else {
          obj->SetSkill(prhash(u8"Light Source"), 100); // All TBAMUD Lights
        }
      } else if (tp == 9) { // ARMOR
        if (val[0] < 0) { // Cursed Armor
          obj->SetAttribute(0, -val[0]);
          obj->SetModifier(0, val[0] * 100);
          obj->SetModifier(1, val[0] * 100);
        } else {
          obj->SetAttribute(0, val[0]);
          obj->SetModifier(0, val[0] * 200);
        }
      } else if (tp == 20) { // MONEY
        if (!gold)
          init_gold();
        (*obj) = (*gold);
        obj->SetSkill(prhash(u8"Quantity"), val[0]);
      } else if (tp == 18) { // KEY
        obj->SetSkill(prhash(u8"Key"), 1000000 + onum); // Key's u8"code"
      } else if (tp == 15) { // CONTAINER
        obj->SetSkill(prhash(u8"Container"), val[0] * 454);
        obj->SetSkill(prhash(u8"Capacity"), val[0]);

        if (!(val[1] & 4))
          obj->SetSkill(prhash(u8"Open"), 1000); // Start open?
        if (val[1] & 8) {
          obj->SetSkill(prhash(u8"Locked"), 1); // Start locked?
          obj->SetSkill(prhash(u8"Lockable"), 1); // Can it be locked?
        }
        if (val[1] & 1)
          obj->SetSkill(prhash(u8"Closeable"), 1); // Can it be closed?
        if (val[2] > 0) {
          obj->SetSkill(prhash(u8"Lock"), 1000000 + val[2]); // Unlocking key's code
          obj->SetSkill(prhash(u8"Accomplishment"), 1300000 + val[2]);
          obj->SetSkill(prhash(u8"Lockable"), 1); // Can it be locked?
        }

        if (obj->ShortDesc().find(u8"bag") < obj->ShortDesc().length()) {
          obj->SetSkill(prhash(u8"Closeable"), 1); // Bags CAN be closed
          obj->SetSkill(prhash(u8"Wearable on Left Hip"), 1); // Bags CAN be belted
          obj->SetSkill(prhash(u8"Wearable on Right Hip"), 2);
        }

        if (obj->ShortDesc().find(u8"pouch") < obj->ShortDesc().length()) {
          obj->SetSkill(prhash(u8"Closeable"), 1); // Pouches CAN be closed
          obj->SetSkill(prhash(u8"Wearable on Left Hip"), 1); // Pouches CAN be belted
          obj->SetSkill(prhash(u8"Wearable on Right Hip"), 2);
        }

      } else if (tp == 2) { // SCROLL
        obj->SetSkill(prhash(u8"Magical"), val[0]);
        obj->SetSkill(prhash(u8"Magical Scroll"), val[0]);
        obj->SetSkill(prhash(u8"Magical Charges"), 1);
        for (int idx = 1; idx < 4; ++idx) {
          add_tba_spell(obj, val[idx], val[0]);
        }
      } else if (tp == 3) { // WAND
        obj->SetSkill(prhash(u8"Magical"), val[0]);
        obj->SetSkill(prhash(u8"Magical Wand"), val[1]);
        obj->SetSkill(prhash(u8"Magical Charges"), val[2]);
        add_tba_spell(obj, val[3], val[0]);
      } else if (tp == 4) { // STAFF
        obj->SetSkill(prhash(u8"Magical"), val[0]);
        obj->SetSkill(prhash(u8"Magical Staff"), val[1]);
        obj->SetSkill(prhash(u8"Magical Charges"), val[2]);
        add_tba_spell(obj, val[3], val[0]);
      } else if (tp == 10) { // POTION
        obj->SetSkill(prhash(u8"Liquid Container"), 1);
        obj->SetSkill(prhash(u8"Closeable"), 1);
        obj->SetSkill(prhash(u8"Perishable"), 1);

        Object* liq = new Object(obj);
        liq->SetSkill(prhash(u8"Liquid"), 1);
        liq->SetSkill(prhash(u8"Ingestible"), 1);
        liq->SetWeight(10);
        liq->SetVolume(1);
        liq->SetSkill(prhash(u8"Quantity"), 1);
        liq->SetSkill(prhash(u8"Magical"), val[0]);
        liq->SetShortDesc(u8"some liquid");
        for (int idx = 1; idx < 4; ++idx) {
          add_tba_spell(liq, val[idx], val[0]);
        }
      } else if (tp == 17 || tp == 23) { // DRINKCON/FOUNTAIN
        if (val[0] < 0)
          val[0] = 1 << 30; // Unlimited
        if (tp == 23) { // FOUNTAIN only
          obj->SetSkill(prhash(u8"Open"), 1000);
          obj->SetSkill(prhash(u8"Liquid Source"), 1);
          obj->SetSkill(prhash(u8"Liquid Container"), val[0] + 1);
        } else { // DRINKCON only
          obj->SetSkill(prhash(u8"Closeable"), 1);
          obj->SetSkill(prhash(u8"Liquid Container"), val[0]);
        }
        if (val[1] > 0 || tp == 23) {
          Object* liq = new Object(obj);
          liq->SetSkill(prhash(u8"Liquid"), 1);
          liq->SetSkill(prhash(u8"Ingestible"), 1);
          liq->SetWeight(20);
          liq->SetVolume(2);
          switch (val[2]) {
            // u8"* 900" = TBAMUD's hours/4 * Acid's Hours * 6 (x10 Drink, x100 Food)
            case (0): { // WATER
              liq->SetShortDesc(u8"water");
              liq->SetSkill(prhash(u8"Drink"), 10 * 900);
              liq->SetSkill(prhash(u8"Food"), 1 * 9000);
              // liq->SetSkill(prhash(u8"Alcohol"), 0 * 900);
            } break;
            case (1): { // BEER
              liq->SetShortDesc(u8"beer");
              liq->SetSkill(prhash(u8"Drink"), 5 * 900);
              liq->SetSkill(prhash(u8"Food"), 2 * 9000);
              liq->SetSkill(prhash(u8"Alcohol"), 3 * 900);
              liq->SetSkill(prhash(u8"Perishable"), 32);
            } break;
            case (2): { // WINE
              liq->SetShortDesc(u8"wine");
              liq->SetSkill(prhash(u8"Drink"), 5 * 900);
              liq->SetSkill(prhash(u8"Food"), 2 * 9000);
              liq->SetSkill(prhash(u8"Alcohol"), 5 * 900);
            } break;
            case (3): { // ALE
              liq->SetShortDesc(u8"ale");
              liq->SetSkill(prhash(u8"Drink"), 5 * 900);
              liq->SetSkill(prhash(u8"Food"), 2 * 9000);
              liq->SetSkill(prhash(u8"Alcohol"), 2 * 900);
              liq->SetSkill(prhash(u8"Perishable"), 16);
            } break;
            case (4): { // DARKALE
              liq->SetShortDesc(u8"dark ale");
              liq->SetSkill(prhash(u8"Drink"), 5 * 900);
              liq->SetSkill(prhash(u8"Food"), 2 * 9000);
              liq->SetSkill(prhash(u8"Alcohol"), 1 * 900);
              liq->SetSkill(prhash(u8"Perishable"), 8);
            } break;
            case (5): { // WHISKY
              liq->SetShortDesc(u8"whisky");
              liq->SetSkill(prhash(u8"Drink"), 4 * 900);
              liq->SetSkill(prhash(u8"Food"), 1 * 9000);
              liq->SetSkill(prhash(u8"Alcohol"), 6 * 900);
            } break;
            case (6): { // LEMONADE
              liq->SetShortDesc(u8"lemonaid");
              liq->SetSkill(prhash(u8"Drink"), 8 * 900);
              liq->SetSkill(prhash(u8"Food"), 1 * 9000);
              // liq->SetSkill(prhash(u8"Alcohol"), 0 * 900);
              liq->SetSkill(prhash(u8"Perishable"), 4);
            } break;
            case (7): { // FIREBRT
              liq->SetShortDesc(u8"firebreather");
              // liq->SetSkill(prhash(u8"Drink"), 0 * 900);
              // liq->SetSkill(prhash(u8"Food"), 0 * 9000);
              liq->SetSkill(prhash(u8"Alcohol"), 10 * 900);
            } break;
            case (8): { // LOCALSPC
              liq->SetShortDesc(u8"local brew");
              liq->SetSkill(prhash(u8"Drink"), 3 * 900);
              liq->SetSkill(prhash(u8"Food"), 3 * 9000);
              liq->SetSkill(prhash(u8"Alcohol"), 3 * 900);
            } break;
            case (9): { // SLIME
              liq->SetShortDesc(u8"slime");
              liq->SetSkill(prhash(u8"Dehydrate Effect"), 8 * 900);
              liq->SetSkill(prhash(u8"Food"), 4 * 9000);
              // liq->SetSkill(prhash(u8"Alcohol"), 0 * 900);
            } break;
            case (10): { // MILK
              liq->SetShortDesc(u8"milk");
              liq->SetSkill(prhash(u8"Drink"), 6 * 900);
              liq->SetSkill(prhash(u8"Food"), 3 * 9000);
              // liq->SetSkill(prhash(u8"Alcohol"), 0 * 900);
              liq->SetSkill(prhash(u8"Perishable"), val[0]);
            } break;
            case (11): { // TEA
              liq->SetShortDesc(u8"tea");
              liq->SetSkill(prhash(u8"Drink"), 6 * 900);
              liq->SetSkill(prhash(u8"Food"), 1 * 9000);
              // liq->SetSkill(prhash(u8"Alcohol"), 0 * 900);
            } break;
            case (12): { // COFFE
              liq->SetShortDesc(u8"coffee");
              liq->SetSkill(prhash(u8"Drink"), 6 * 900);
              liq->SetSkill(prhash(u8"Food"), 1 * 9000);
              // liq->SetSkill(prhash(u8"Alcohol"), 0 * 900);
            } break;
            case (13): { // BLOOD
              liq->SetShortDesc(u8"blood");
              liq->SetSkill(prhash(u8"Dehydrate Effect"), 1 * 900);
              liq->SetSkill(prhash(u8"Food"), 2 * 9000);
              // liq->SetSkill(prhash(u8"Alcohol"), 0 * 900);
              liq->SetSkill(prhash(u8"Perishable"), 2);
            } break;
            case (14): { // SALTWATER
              liq->SetShortDesc(u8"salt water");
              liq->SetSkill(prhash(u8"Dehydrate Effect"), 2 * 900);
              liq->SetSkill(prhash(u8"Food"), 1 * 9000);
              // liq->SetSkill(prhash(u8"Alcohol"), 0 * 900);
            } break;
            case (15): { // CLEARWATER
              liq->SetShortDesc(u8"clear water");
              liq->SetSkill(prhash(u8"Drink"), 13 * 900);
              // liq->SetSkill(prhash(u8"Food"), 0 * 9000);
              // liq->SetSkill(prhash(u8"Alcohol"), 0 * 900);
            } break;
          }
          if (val[3] != 0) {
            liq->SetSkill(prhash(u8"Poisionous"), val[3]);
          }
          liq->SetSkill(prhash(u8"Quantity"), val[1]);
        }
      } else if (tp == 19) { // FOOD
        obj->SetSkill(prhash(u8"Ingestible"), 1);
        obj->SetSkill(prhash(u8"Perishable"), val[0]);
        obj->SetSkill(prhash(u8"Food"), val[0] * 36000); // 60 Mins & 6 Acid Hours/Hour (x100)
        if (val[3] != 0) {
          obj->SetSkill(prhash(u8"Poisionous"), val[3]);
        }
      } else if (tp == 22) { // BOAT
        obj->SetSkill(prhash(u8"Enterable"), 1);
        obj->SetSkill(prhash(u8"Open"), 1000);
        obj->SetSkill(prhash(u8"Vehicle"), 4); // Unpowered (1=0), Calm Water (4=1).
      } else if (tp == 5) { // WEAPON
        int wreach = 0; // default

        int skmatch = get_weapon_type(u8"Short Crushing"); // default
        if (val[3] == 1) // u8"stings"
          skmatch = get_weapon_type(u8"Short Piercing");
        else if (val[3] == 2) // u8"whips"
          skmatch = get_weapon_type(u8"Whips");
        else if (val[3] == 3) // u8"slashes"
          skmatch = get_weapon_type(u8"Short Blades");
        else if (val[3] == 4) // u8"bites"
          skmatch = get_weapon_type(u8"Short Piercing");
        else if (val[3] == 8) // u8"claws"
          skmatch = get_weapon_type(u8"Short Blades");
        else if (val[3] == 9) // u8"mauls"
          skmatch = get_weapon_type(u8"Short Cleaves");
        else if (val[3] == 11) // u8"pierces"
          skmatch = get_weapon_type(u8"Short Piercing");
        else if (val[3] == 12) // u8"blasts"
          skmatch = get_weapon_type(u8"Shotguns");
        else if (val[3] == 13) // u8"punches"
          skmatch = get_weapon_type(u8"Punching");
        else if (val[3] == 14) // u8"stabs"
          skmatch = get_weapon_type(u8"Short Piercing");

        if (matches(obj->ShortDesc(), u8"lance")) {
          skmatch = get_weapon_type(u8"Two-Handed Piercing");
        } else if (matches(obj->ShortDesc(), u8"spear")) {
          skmatch = get_weapon_type(u8"Two-Handed Piercing");
        } else if (matches(obj->ShortDesc(), u8"pitchfork")) {
          skmatch = get_weapon_type(u8"Two-Handed Piercing");
        } else if (matches(obj->ShortDesc(), u8"pitch fork")) {
          skmatch = get_weapon_type(u8"Two-Handed Piercing");
        } else if (matches(obj->ShortDesc(), u8"bolg")) {
          skmatch = get_weapon_type(u8"Two-Handed Piercing");
        } else if (matches(obj->ShortDesc(), u8"bulg")) {
          skmatch = get_weapon_type(u8"Two-Handed Piercing");
        } else if (matches(obj->ShortDesc(), u8"pike")) {
          skmatch = get_weapon_type(u8"Two-Handed Piercing");
        } else if (matches(obj->ShortDesc(), u8"trident")) {
          skmatch = get_weapon_type(u8"Two-Handed Piercing");
        } else if (matches(obj->ShortDesc(), u8"sickle")) {
          skmatch = get_weapon_type(u8"Two-Handed Piercing");
        } else if (matches(obj->ShortDesc(), u8"sap")) {
          skmatch = get_weapon_type(u8"Short Flails");
        } else if (matches(obj->ShortDesc(), u8"flail")) {
          skmatch = get_weapon_type(u8"Long Flails");
        } else if (matches(obj->ShortDesc(), u8"ball and chain")) {
          skmatch = get_weapon_type(u8"Two-Handed Flails");
        } else if (matches(obj->ShortDesc(), u8"whip")) {
          skmatch = get_weapon_type(u8"Whips");
        } else if (matches(obj->ShortDesc(), u8"bullwhip")) {
          skmatch = get_weapon_type(u8"Whips");
        } else if (matches(obj->ShortDesc(), u8"scourge")) {
          skmatch = get_weapon_type(u8"Whips");
        } else if (matches(obj->ShortDesc(), u8"crop")) {
          skmatch = get_weapon_type(u8"Whips");
        } else if (matches(obj->ShortDesc(), u8"chain")) {
          skmatch = get_weapon_type(u8"Whips");
        } else if (matches(obj->ShortDesc(), u8"batleth")) {
          skmatch = get_weapon_type(u8"Two-Handed Blades");
        } else if (matches(obj->ShortDesc(), u8"lochaber")) {
          skmatch = get_weapon_type(u8"Two-Handed Cleaves");
        } else if (matches(obj->ShortDesc(), u8"claymore")) {
          skmatch = get_weapon_type(u8"Two-Handed Blades");
        } else if (matches(obj->ShortDesc(), u8"knife")) {
          skmatch = get_weapon_type(u8"Short Blades");
        } else if (matches(obj->ShortDesc(), u8"shard")) {
          skmatch = get_weapon_type(u8"Short Blades");
        } else if (matches(obj->ShortDesc(), u8"hook")) {
          skmatch = get_weapon_type(u8"Short Piercing");
        } else if (matches(obj->ShortDesc(), u8"stake")) {
          skmatch = get_weapon_type(u8"Short Piercing");
        } else if (matches(obj->ShortDesc(), u8"spike")) {
          skmatch = get_weapon_type(u8"Short Piercing");
        } else if (matches(obj->ShortDesc(), u8"nail")) {
          skmatch = get_weapon_type(u8"Short Piercing");
        } else if (matches(obj->ShortDesc(), u8"dagger")) {
          skmatch = get_weapon_type(u8"Short Piercing");
        } else if (matches(obj->ShortDesc(), u8"needles")) {
          skmatch = get_weapon_type(u8"Short Piercing");
        } else if (matches(obj->ShortDesc(), u8"awl")) {
          skmatch = get_weapon_type(u8"Short Piercing");
        } else if (matches(obj->ShortDesc(), u8"screwdriver")) {
          skmatch = get_weapon_type(u8"Short Piercing");
        } else if (matches(obj->ShortDesc(), u8"stiletto")) {
          skmatch = get_weapon_type(u8"Short Piercing");
        } else if (matches(obj->ShortDesc(), u8"kris")) {
          skmatch = get_weapon_type(u8"Short Piercing");
        } else if (matches(obj->ShortDesc(), u8"chisel")) {
          skmatch = get_weapon_type(u8"Short Piercing");
        } else if (matches(obj->ShortDesc(), u8"dirk")) {
          skmatch = get_weapon_type(u8"Short Piercing");
        } else if (matches(obj->ShortDesc(), u8"rapier")) {
          skmatch = get_weapon_type(u8"Long Piercing");
        } else if (matches(obj->ShortDesc(), u8"epee")) {
          skmatch = get_weapon_type(u8"Long Piercing");
        } else if (matches(obj->ShortDesc(), u8"glaive")) {
          skmatch = get_weapon_type(u8"Two-Handed Blades");
        } else if (matches(obj->ShortDesc(), u8"scimitar")) {
          skmatch = get_weapon_type(u8"Long Blades");
        } else if (matches(obj->ShortDesc(), u8"katana")) {
          skmatch = get_weapon_type(u8"Long Blades");
        } else if (matches(obj->ShortDesc(), u8"sword")) {
          skmatch = get_weapon_type(u8"Long Blades");
        } else if (matches(obj->ShortDesc(), u8"longsword")) {
          skmatch = get_weapon_type(u8"Long Blades");
        } else if (matches(obj->ShortDesc(), u8"straightsword")) {
          skmatch = get_weapon_type(u8"Long Blades");
        } else if (matches(obj->ShortDesc(), u8"saber")) {
          skmatch = get_weapon_type(u8"Long Blades");
        } else if (matches(obj->ShortDesc(), u8"sabre")) {
          skmatch = get_weapon_type(u8"Long Blades");
        } else if (matches(obj->ShortDesc(), u8"cutlass")) {
          skmatch = get_weapon_type(u8"Long Blades");
        } else if (matches(obj->ShortDesc(), u8"sabre")) {
          skmatch = get_weapon_type(u8"Long Blades");
        } else if (matches(obj->ShortDesc(), u8"cleaver")) {
          skmatch = get_weapon_type(u8"Long Cleaves");
        } else if (matches(obj->ShortDesc(), u8"pick")) {
          skmatch = get_weapon_type(u8"Long Cleaves");
        } else if (matches(obj->ShortDesc(), u8"axe")) {
          skmatch = get_weapon_type(u8"Long Cleaves");
        } else if (matches(obj->ShortDesc(), u8"hatchet")) {
          skmatch = get_weapon_type(u8"Long Cleaves");
        } else if (matches(obj->ShortDesc(), u8"pickaxe")) {
          skmatch = get_weapon_type(u8"Long Cleaves");
        } else if (matches(obj->ShortDesc(), u8"hand axe")) {
          skmatch = get_weapon_type(u8"Short Cleaves");
        } else if (matches(obj->ShortDesc(), u8"handaxe")) {
          skmatch = get_weapon_type(u8"Short Cleaves");
        } else if (matches(obj->ShortDesc(), u8"club")) {
          skmatch = get_weapon_type(u8"Long Crushing");
        } else if (matches(obj->ShortDesc(), u8"peg leg")) {
          skmatch = get_weapon_type(u8"Long Crushing");
        } else if (matches(obj->ShortDesc(), u8"stick")) {
          skmatch = get_weapon_type(u8"Long Crushing");
        } else if (matches(obj->ShortDesc(), u8"bat")) {
          skmatch = get_weapon_type(u8"Long Crushing");
        } else if (matches(obj->ShortDesc(), u8"rod")) {
          skmatch = get_weapon_type(u8"Long Crushing");
        } else if (matches(obj->ShortDesc(), u8"scepter")) {
          skmatch = get_weapon_type(u8"Long Crushing");
        } else if (matches(obj->ShortDesc(), u8"sceptre")) {
          skmatch = get_weapon_type(u8"Long Crushing");
        } else if (matches(obj->ShortDesc(), u8"mace")) {
          skmatch = get_weapon_type(u8"Long Crushing");
        } else if (matches(obj->ShortDesc(), u8"morning star")) {
          skmatch = get_weapon_type(u8"Long Crushing");
        } else if (matches(obj->ShortDesc(), u8"cudgel")) {
          skmatch = get_weapon_type(u8"Long Crushing");
        } else if (matches(obj->ShortDesc(), u8"scythe")) {
          skmatch = get_weapon_type(u8"Two-Handed Blades");
        } else if (matches(obj->ShortDesc(), u8"bardiche")) {
          skmatch = get_weapon_type(u8"Two-Handed Cleaves");
        } else if (matches(obj->ShortDesc(), u8"fauchard")) {
          skmatch = get_weapon_type(u8"Two-Handed Cleaves");
        } else if (matches(obj->ShortDesc(), u8"bec de corbin")) {
          skmatch = get_weapon_type(u8"Two-Handed Cleaves");
        } else if (matches(obj->ShortDesc(), u8"bill-gisarme")) {
          skmatch = get_weapon_type(u8"Two-Handed Cleaves");
        } else if (matches(obj->ShortDesc(), u8"gisarme")) {
          skmatch = get_weapon_type(u8"Two-Handed Cleaves");
        } else if (matches(obj->ShortDesc(), u8"halberd")) {
          skmatch = get_weapon_type(u8"Two-Handed Cleaves");
        } else if (matches(obj->ShortDesc(), u8"poleaxe")) {
          skmatch = get_weapon_type(u8"Two-Handed Cleaves");
        } else if (matches(obj->ShortDesc(), u8"polearm")) {
          skmatch = get_weapon_type(u8"Two-Handed Cleaves");
        } else if (matches(obj->ShortDesc(), u8"pole axe")) {
          skmatch = get_weapon_type(u8"Two-Handed Cleaves");
        } else if (matches(obj->ShortDesc(), u8"pole arm")) {
          skmatch = get_weapon_type(u8"Two-Handed Cleaves");
        } else if (matches(obj->ShortDesc(), u8"fishing pole")) {
          skmatch = get_weapon_type(u8"Whips");
        } else if (matches(obj->ShortDesc(), u8"hammer")) {
          skmatch = get_weapon_type(u8"Long Crushing");
        } else if (matches(obj->ShortDesc(), u8"maul")) {
          skmatch = get_weapon_type(u8"Long Crushing");
        } else if (matches(obj->ShortDesc(), u8"mallet")) {
          skmatch = get_weapon_type(u8"Long Crushing");
        } else if (matches(obj->ShortDesc(), u8"gavel")) {
          skmatch = get_weapon_type(u8"Short Crushing");
        } else if (matches(obj->ShortDesc(), u8"baton")) {
          skmatch = get_weapon_type(u8"Short Staves");
        } else if (matches(obj->ShortDesc(), u8"quarterstaff")) {
          skmatch = get_weapon_type(u8"Long Staves");
        } else if (matches(obj->ShortDesc(), u8"shovel")) {
          skmatch = get_weapon_type(u8"Long Crushing");
        } else if (matches(obj->ShortDesc(), u8"hoe")) {
          skmatch = get_weapon_type(u8"Long Crushing");
        } else if (matches(obj->ShortDesc(), u8"broom")) {
          skmatch = get_weapon_type(u8"Two-Handed Staves");
        } else if (matches(obj->ShortDesc(), u8"rake")) {
          skmatch = get_weapon_type(u8"Two-Handed Staves");
        } else if (matches(obj->ShortDesc(), u8"mop")) {
          skmatch = get_weapon_type(u8"Two-Handed Staves");
        } else if (matches(obj->ShortDesc(), u8"staff")) {
          skmatch = get_weapon_type(u8"Two-Handed Staves");
        } else if (matches(obj->ShortDesc(), u8"bow")) {
          skmatch = get_weapon_type(u8"Archery");
        } else if (matches(obj->ShortDesc(), u8"longbow")) {
          skmatch = get_weapon_type(u8"Archery");
        } else if (matches(obj->ShortDesc(), u8"arrow")) {
          skmatch = get_weapon_type(u8"Archery");
        } else if (matches(obj->ShortDesc(), u8"arrows")) {
          skmatch = get_weapon_type(u8"Archery");
        } else if (matches(obj->ShortDesc(), u8"slingshot")) {
          skmatch = get_weapon_type(u8"Crossbow");
        } else if (matches(obj->ShortDesc(), u8"sling shot")) {
          skmatch = get_weapon_type(u8"Crossbow");
        } else if (matches(obj->ShortDesc(), u8"sling")) {
          skmatch = get_weapon_type(u8"Slings");
        } else if (matches(obj->ShortDesc(), u8"dart")) {
          skmatch = get_weapon_type(u8"Throwing, Aero");
        } else if (matches(obj->ShortDesc(), u8"darts")) {
          skmatch = get_weapon_type(u8"Throwing, Aero");
        } else if (matches(obj->ShortDesc(), u8"blade")) {
          skmatch = get_weapon_type(u8"Long Blades");
        } else if (matches(obj->ShortDesc(), u8"bone")) {
          skmatch = get_weapon_type(u8"Long Crushing");
        } else if (matches(obj->ShortDesc(), u8"branch")) {
          skmatch = get_weapon_type(u8"Long Crushing");
        } else if (matches(obj->ShortDesc(), u8"pen")) {
          skmatch = get_weapon_type(u8"Short Piercing");
        } else if (matches(obj->ShortDesc(), u8"shears")) {
          skmatch = get_weapon_type(u8"Short Piercing");
        } else if (matches(obj->ShortDesc(), u8"stakes")) {
          skmatch = get_weapon_type(u8"Short Piercing");
        } else if (matches(obj->ShortDesc(), u8"scalpel")) {
          skmatch = get_weapon_type(u8"Short Blades");
        } else if (matches(obj->ShortDesc(), u8"crook")) {
          skmatch = get_weapon_type(u8"Two-Handed Staves");
        } else if (matches(obj->ShortDesc(), u8"knuckles")) {
          skmatch = get_weapon_type(u8"Punching");
        } else if (matches(obj->ShortDesc(), u8"laser rifle")) {
          skmatch = get_weapon_type(u8"Laser Rifles");
        } else if (matches(obj->ShortDesc(), u8"assault rifle")) {
          skmatch = get_weapon_type(u8"Assault Rifles");
        } else if (matches(obj->ShortDesc(), u8"rifle")) {
          skmatch = get_weapon_type(u8"Rifles");
        } else if (matches(obj->ShortDesc(), u8"phaser")) {
          skmatch = get_weapon_type(u8"Plasma Pistols");
        } else if (matches(obj->ShortDesc(), u8"plasma gun")) {
          skmatch = get_weapon_type(u8"Plasma Pistols");
        } else if (matches(obj->ShortDesc(), u8"laser pistol")) {
          skmatch = get_weapon_type(u8"Laser Pistols");
        } else if (matches(obj->ShortDesc(), u8"pistol")) {
          skmatch = get_weapon_type(u8"Pistols");
        } else if (matches(obj->ShortDesc(), u8"handgun")) {
          skmatch = get_weapon_type(u8"Pistols");
        } else if (matches(obj->ShortDesc(), u8"hand gun")) {
          skmatch = get_weapon_type(u8"Pistols");
        } else if (matches(obj->ShortDesc(), u8"gun")) {
          skmatch = get_weapon_type(u8"Shotguns");
        } else if (matches(obj->ShortDesc(), u8"thunderbolt")) {
          skmatch = get_weapon_type(u8"Throwing, Aero");
        } else if (matches(obj->ShortDesc(), u8"bola")) {
          skmatch = get_weapon_type(u8"Throwing, Aero");
        } else if (matches(obj->ShortDesc(), u8"bolas")) {
          skmatch = get_weapon_type(u8"Throwing, Aero");
        } else if (matches(obj->ShortDesc(), u8"pole")) {
          skmatch = get_weapon_type(u8"Hurling");
        } else if (matches(obj->ShortDesc(), u8"boulder")) {
          skmatch = get_weapon_type(u8"Hurling");
        } else if (matches(obj->ShortDesc(), u8"rock")) {
          skmatch = get_weapon_type(u8"Throwing, Non-Aero");
        } else if (matches(obj->ShortDesc(), u8"stone")) {
          skmatch = get_weapon_type(u8"Throwing, Non-Aero");
        } else if (matches(obj->ShortDesc(), u8"boomerang")) {
          skmatch = get_weapon_type(u8"Throwing, Aero");
        } else if (matches(obj->ShortDesc(), u8"blowgun")) {
          skmatch = get_weapon_type(u8"Blowgun");
        } else if (matches(obj->ShortDesc(), u8"crossbow")) {
          skmatch = get_weapon_type(u8"Crossbow");
        } else if (matches(obj->ShortDesc(), u8"arbalest")) {
          skmatch = get_weapon_type(u8"Crossbow");
        } else if (matches(obj->ShortDesc(), u8"net")) {
          skmatch = get_weapon_type(u8"Nets");
        } else {
          fprintf(
              stderr,
              CYEL u8"Warning: Using Default of '%s' for '%s'!\n" CNRM,
              SkillName(get_weapon_skill(skmatch)).c_str(),
              obj->ShortDescC());
        }

        if (matches(obj->ShortDesc(), u8"two-handed")) {
          if (skmatch == get_weapon_type(u8"Short Blades"))
            skmatch = get_weapon_type(u8"Two-Handed Blades");

          else if (skmatch == get_weapon_type(u8"Long Blades"))
            skmatch = get_weapon_type(u8"Two-Handed Blades");

          else if (skmatch == get_weapon_type(u8"Short Piercing"))
            skmatch = get_weapon_type(u8"Two-Handed Piercing");

          else if (skmatch == get_weapon_type(u8"Long Piercing"))
            skmatch = get_weapon_type(u8"Two-Handed Piercing");

          else if (skmatch == get_weapon_type(u8"Short Flails"))
            skmatch = get_weapon_type(u8"Two-Handed Flails");

          else if (skmatch == get_weapon_type(u8"Long Flails"))
            skmatch = get_weapon_type(u8"Two-Handed Flails");

          else if (skmatch == get_weapon_type(u8"Short Staves"))
            skmatch = get_weapon_type(u8"Two-Handed Staves");

          else if (skmatch == get_weapon_type(u8"Long Staves"))
            skmatch = get_weapon_type(u8"Two-Handed Staves");

          else if (skmatch == get_weapon_type(u8"Long Crushing"))
            skmatch = get_weapon_type(u8"Two-Handed Crushing");

          else if (skmatch == get_weapon_type(u8"Short Crushing"))
            skmatch = get_weapon_type(u8"Two-Handed Crushing");

          else if (skmatch == get_weapon_type(u8"Short Cleaves"))
            skmatch = get_weapon_type(u8"Two-Handed Cleaves");

          else if (skmatch == get_weapon_type(u8"Long Cleaves"))
            skmatch = get_weapon_type(u8"Two-Handed Cleaves");
        }

        if (wreach == 0) {
          if (skmatch == get_weapon_type(u8"Short Blades"))
            wreach = 0;
          else if (skmatch == get_weapon_type(u8"Short Piercing"))
            wreach = 0;
          else if (skmatch == get_weapon_type(u8"Long Blades"))
            wreach = 1;
          else if (skmatch == get_weapon_type(u8"Two-Handed Blades"))
            wreach = 2;
          else if (skmatch == get_weapon_type(u8"Long Piercing"))
            wreach = 1;
          else if (skmatch == get_weapon_type(u8"Two-Handed Piercing"))
            wreach = 2;
          else if (skmatch == get_weapon_type(u8"Short Crushing"))
            wreach = 0;
          else if (skmatch == get_weapon_type(u8"Long Crushing"))
            wreach = 1;
          else if (skmatch == get_weapon_type(u8"Two-Handed Crushing"))
            wreach = 2;
          else if (skmatch == get_weapon_type(u8"Short Cleaves"))
            wreach = 0;
          else if (skmatch == get_weapon_type(u8"Long Cleaves"))
            wreach = 1;
          else if (skmatch == get_weapon_type(u8"Two-Handed Cleaves"))
            wreach = 2;
          else if (skmatch == get_weapon_type(u8"Short Flails"))
            wreach = 0;
          else if (skmatch == get_weapon_type(u8"Long Flails"))
            wreach = 1;
          else if (skmatch == get_weapon_type(u8"Two-Handed Flails"))
            wreach = 2;
          else if (skmatch == get_weapon_type(u8"Short Staves"))
            wreach = 0;
          else if (skmatch == get_weapon_type(u8"Long Staves"))
            wreach = 1;
          else if (skmatch == get_weapon_type(u8"Two-Handed Staves"))
            wreach = 2;
          else if (skmatch == get_weapon_type(u8"Whips"))
            wreach = 4;
          else if (skmatch == get_weapon_type(u8"Nets"))
            wreach = 4;
          else if (skmatch == get_weapon_type(u8"Archery"))
            wreach = 100;
          else if (skmatch == get_weapon_type(u8"Crossbow"))
            wreach = 100;
          else if (skmatch == get_weapon_type(u8"Slings"))
            wreach = 50;
          else if (skmatch == get_weapon_type(u8"Blowgun"))
            wreach = 20;
          else if (skmatch == get_weapon_type(u8"Throwing, Aero"))
            wreach = 20;
          else if (skmatch == get_weapon_type(u8"Throwing, Non-Aero"))
            wreach = 10;
          else if (skmatch == get_weapon_type(u8"Hurling"))
            wreach = 5;
          else if (skmatch == get_weapon_type(u8"Assault Rifles"))
            wreach = 1000;
          else if (skmatch == get_weapon_type(u8"Laser Rifles"))
            wreach = 1000;
          else if (skmatch == get_weapon_type(u8"Rifles"))
            wreach = 1000;
          else if (skmatch == get_weapon_type(u8"Shotguns"))
            wreach = 500;
          else if (skmatch == get_weapon_type(u8"Pistols"))
            wreach = 200;
          else if (skmatch == get_weapon_type(u8"Laser Pistols"))
            wreach = 200;
          else if (skmatch == get_weapon_type(u8"Plasma Pistols"))
            wreach = 200;
          else if (skmatch == get_weapon_type(u8"Punching"))
            wreach = 0;
          else {
            fprintf(
                stderr,
                CYEL u8"Warning: Using Default reach of zero for '%s'!\n" CNRM,
                obj->ShortDescC());
          }
        }

        obj->SetSkill(prhash(u8"WeaponType"), skmatch);
        //	obj->SetSkill(prhash(u8"WeaponDamage"), val[1]*val[2]);
        int sev = 0;
        int tot = (val[1] * (val[2] + 1) + 1) / 2; // Avg. TBA Dam. Rounded Up
        while (tot > sev) {
          ++sev;
          tot -= sev;
        }
        obj->SetSkill(prhash(u8"WeaponForce"), tot);
        obj->SetSkill(prhash(u8"WeaponSeverity"), sev);
        obj->SetSkill(prhash(u8"WeaponReach"), wreach);
      }

      int wt, vl;
      fscanf(mudo, u8"%d %d %*[^\n\r]\n", &wt, &vl);

      if (tp != 20) { // MONEY DOESN'T WORK THIS WAY
        obj->SetWeight((wt >= 1000000) ? 1000000 : wt * 454);
        obj->SetVolume(wt); // FIXME: Better guess within units?
        obj->SetSize(1);
        obj->SetValue((vl * valmod) / 1000);
        if (obj->Matches(u8"cashcard")) { // Is Script Now
          obj->SetSkill(prhash(u8"Money"), (vl * valmod) / 1000);
        }
      }

      int magresist = 0;
      while (fscanf(mudo, u8"%1[AET] ", buf) > 0) {
        if (buf[0] == 'A') { // Extra Affects
          int anum, aval;
          fscanf(mudo, u8"%d %d\n", &anum, &aval);
          switch (anum) {
            case (1): { // STR -> Strength
              obj->SetModifier(2, aval * 400 / powmod);
            } break;
            case (2): { // DEX -> Quickness
              obj->SetModifier(1, aval * 400 / powmod);
            } break;
            case (3): { // INT -> Intelligence
              obj->SetModifier(4, aval * 400 / powmod);
            } break;
            case (4): { // WIS -> Willpower
              obj->SetModifier(5, aval * 400 / powmod);
            } break;
            case (5): { // CON -> Body
              obj->SetModifier(0, aval * 400 / powmod);
            } break;
            case (6): { // CHA -> Charisma
              obj->SetModifier(3, aval * 400 / powmod);
            } break;
            //	    case(7): {	// CLASS (Even TBAMUD Doesn't Use This!)
            //	      } break;
            //	    case(8): {	// LEVEL (Even TBAMUD Doesn't Use This!)
            //	      } break;
            case (9): { // AGE
              if (aval > 0)
                obj->SetSkill(prhash(u8"Youth Penalty"), aval / powmod);
              else if (aval < 0)
                obj->SetSkill(prhash(u8"Youth Bonus"), -aval / powmod);
            } break;
            //	    case(10): {	// CHAR_WEIGHT (Unused by main TBAMUD data)
            //	      } break;
            //	    case(11): {	// CHAR_HEIGHT (Unused by main TBAMUD data)
            //	      } break;
            case (12): { // MANA
              if (aval > 0)
                obj->SetSkill(prhash(u8"Magic Force Bonus"), aval * 100 / powmod);
              else if (aval < 0)
                obj->SetSkill(prhash(u8"Magic Force Penalty"), -aval * 100 / powmod);
            } break;
            case (13): { // HIT
              if (aval > 0)
                obj->SetSkill(prhash(u8"Resilience Bonus"), aval * 100 / powmod);
              else if (aval < 0)
                obj->SetSkill(prhash(u8"Resilience Penalty"), -aval * 100 / powmod);
            } break;
            case (14): { // MOVE
              if (aval > 0)
                obj->SetSkill(prhash(u8"Encumbrance Bonus"), aval * 20 / powmod);
              else if (aval < 0)
                obj->SetSkill(prhash(u8"Encumbrance Penalty"), -aval * 20 / powmod);
            } break;
            //	    case(15): {	// GOLD (Even TBAMUD Doesn't Use This!)
            //	      } break;
            //	    case(16): {	// EXP (Even TBAMUD Doesn't Use This!)
            //	      } break;
            case (17): { // AC
              if (aval > 0)
                obj->SetSkill(prhash(u8"Evasion Penalty"), aval * 400 / powmod);
              else if (aval < 0)
                obj->SetSkill(prhash(u8"Evasion Bonus"), -aval * 400 / powmod);
            } break;
            case (18): { // HITROLL
              if (aval > 0)
                obj->SetSkill(prhash(u8"Accuracy Bonus"), aval * 400 / powmod);
              else if (aval < 0)
                obj->SetSkill(prhash(u8"Accuracy Penalty"), -aval * 400 / powmod);
            } break;
            case (19): { // DAMROLL
              if (aval > 0)
                obj->SetSkill(prhash(u8"Damage Bonus"), aval * 400 / powmod);
              else if (aval < 0)
                obj->SetSkill(prhash(u8"Damage Penalty"), -aval * 400 / powmod);
            } break;
            case (20): { // SAVING_PARA
              magresist += (aval * 400 / powmod);
            } break;
            case (21): { // SAVING_ROD
              magresist += (aval * 400 / powmod);
            } break;
            case (22): { // SAVING_PETRI
              magresist += (aval * 400 / powmod);
            } break;
            case (23): { // SAVING_BREATH
              magresist += (aval * 400 / powmod);
            } break;
            case (24): { // SAVING_SPELL
              magresist += (aval * 400 / powmod);
            } break;
          }
        } else if (buf[0] == 'T') { // Triggers
          int tnum = 0;
          fscanf(mudo, u8"%d\n", &tnum);
          if (tnum > 0 && bynumtrg.count(tnum) > 0) {
            Object* trg = new Object(*(bynumtrg[tnum]));
            trg->SetParent(obj);
            todotrg.push_back(trg);
            //    fprintf(stderr, u8"Put Trg \"%s\" on Obj \"%s\"\n",
            //	trg->Desc(), obj->ShortDesc()
            //	);
          }
        } else if (buf[0] == 'E') { // Extra Affects
          while (fscanf(mudo, u8" %65535[^~ ]", buf) > 0) {
            for (auto chr = buf; *chr != 0; ++chr) {
              if (ascii_isalpha(*chr)) {
                *chr = ascii_tolower(*chr);
              }
            }
            if (std::u8string(buf).find_first_not_of(target_chars) != std::u8string::npos) {
              fprintf(
                  stderr,
                  CYEL u8"Warning: Ignoring non-alpha extra (%s) for '%s'!\n" CNRM,
                  buf,
                  obj->ShortDescC());
            } else if (words_match(obj->ShortDesc(), buf)) {
              // fprintf(stderr, CYEL u8"Warning: Duplicate (%s) extra for '%s'!\n" CNRM, buf,
              // obj->ShortDescC());
            } else {
              std::u8string sd = obj->ShortDescS();
              if (sd.back() == ')') {
                sd.back() = ' ';
              } else {
                sd += u8" (";
              }
              sd += buf;
              sd += ')';
              obj->SetShortDesc(sd);
              // fprintf(stderr, CYEL u8"Warning: Non-matching (%s) extra for '%s'!\n" CNRM, buf,
              // obj->ShortDescC());
            }
          }
          fscanf(mudo, u8"%*[^~]");
          fscanf(mudo, u8"~%*[\n\r]");
          fscanf(mudo, u8"%65535[^~]", buf);
          if (!obj->HasLongDesc()) {
            obj->SetLongDesc(buf);
          } else {
            std::u8string ld = obj->LongDescS();
            ld += u8"\n\n";
            ld += buf;
            obj->SetLongDesc(ld);
            // fprintf(stderr, CYEL u8"Warning: Had to merge long descriptions of (%s) extra for
            // '%s'!\n" CNRM, obj->LongDescC(), obj->ShortDescC());
          }
          fscanf(mudo, u8"%*[^~]");
          fscanf(mudo, u8"~%*[\n\r]");
        } else { // Extra Descriptions FIXME: Handle!
          fprintf(stderr, u8"ERROR: Unknown tag!\n");
        }
      }
      if (magresist > 0)
        obj->SetSkill(prhash(u8"Magic Resistance"), magresist);
      else if (magresist < 0)
        obj->SetSkill(prhash(u8"Magic Vulnerability"), -magresist);

      fscanf(mudo, u8"%*[^#$]");
    }
    fclose(mudo);
  }
}

void Object::TBALoadWLD(const std::u8string& fn) {
  FILE* mud = fopen(fn.c_str(), u8"r");
  int znum = 0, offset = fn.length() - 5; // Chop off the .wld
  while (isdigit(fn[offset]))
    --offset;
  znum = atoi(fn.c_str() + offset + 1);
  if (mud) {
    Object* zone = new Object(this);
    zone->SetShortDesc(
        std::u8string(u8"TBA Zone-") + fn.substr(0, fn.length() - 4).substr(offset + 1));
    zone->SetSkill(prhash(u8"Light Source"), 1000);
    zone->SetSkill(prhash(u8"Day Length"), 240);
    zone->SetSkill(prhash(u8"Day Time"), 120);

    // fprintf(stderr, u8"Loading TBA Realm from \"%s\"\n", fn.c_str());
    while (1) {
      int onum;
      if (fscanf(mud, u8" #%d\n", &onum) < 1)
        break;
      // fprintf(stderr, u8"Loading room #%d\n", onum);

      Object* obj = new Object(zone);
      olist.push_back(obj);
      obj->SetSkill(prhash(u8"TBARoom"), 1000000 + onum);
      bynumwld[onum] = obj;
      if (onum == 0) { // Player Start Room (Void) Hard-Coded Here
        Object* world = obj->World();
        world->AddAct(act_t::SPECIAL_HOME, obj);
        if (!world->parent->IsAct(act_t::SPECIAL_HOME)) { // If is first world
          world->parent->AddAct(act_t::SPECIAL_HOME, obj);
        }
      }

      obj->SetWeight(-1);
      obj->SetVolume(-1);
      obj->SetSize(-1);
      obj->SetValue(-1);

      obj->SetShortDesc(load_tba_field(mud));
      // fprintf(stderr, u8"Loaded TBA Room with Name = %s\n", buf);

      obj->SetDesc(load_tba_field(mud));
      // fprintf(stderr, u8"Loaded TBA Room with Desc = %s\n", buf);

      int val;
      fscanf(mud, u8"%*d %65535[^ \t\n] %*d %*d %*d %d\n", buf, &val);
      // FIXME: TBA's extra 3 flags variables (ignored now)?
      if (val == 6)
        obj->SetSkill(prhash(u8"WaterDepth"), 1); // WATER_SWIM
      else if (val == 7)
        obj->SetSkill(prhash(u8"WaterDepth"), 2); // WATER_NOSWIM
      else if (val == 8)
        obj->SetSkill(prhash(u8"WaterDepth"), 3); // UNDERWATER

      std::u8string name = obj->ShortDescS();
      if (name.find(u8"Secret") >= 0 && name.find(u8"Secret") < name.length()) {
        obj->SetSkill(prhash(u8"Accomplishment"), 1100000 + onum);
      }

      obj->SetSkill(prhash(u8"Translucent"), 1000); // Full sky, by default
      if (strcasestr8(buf, u8"d") || (atoi(buf) & 8)) { // INDOORS
        obj->SetSkill(prhash(u8"Translucent"), 200); // Windows (unless DARK)
        obj->SetSkill(prhash(u8"Light Source"), 100); // Torches (unless DARK)
      }
      if (strcasestr8(buf, u8"a") || (atoi(buf) & 1)) { // DARK
        obj->SetSkill(prhash(u8"Translucent"), 0); // No sky, no windows
        obj->SetSkill(prhash(u8"Light Source"), 0); // No torches
      }
      if (strcasestr8(buf, u8"b") || (atoi(buf) & 2)) { // DEATH
        obj->SetSkill(prhash(u8"Accomplishment"), 1100000 + onum);
        // obj->SetSkill(prhash(u8"Hazardous"), 2); //FIXME: Actually Dangerous?
      }
      if (strcasestr8(buf, u8"c") || (atoi(buf) & 4)) { // NOMOB
        obj->SetSkill(prhash(u8"TBAZone"), 999999);
      } else {
        obj->SetSkill(prhash(u8"TBAZone"), 1000000 + znum);
      }
      if (strcasestr8(buf, u8"e") || (atoi(buf) & 16)) { // PEACEFUL
        obj->SetSkill(prhash(u8"Peaceful"), 1000);
      }
      if (strcasestr8(buf, u8"f") || (atoi(buf) & 32)) { // SOUNDPROOF
        obj->SetSkill(prhash(u8"Soundproof"), 1000);
      }
      //      if(strcasestr8(buf, u8"g") || (atoi(buf) & 64)) { //NOTRACK
      //	//FIXME: Implement
      //	}
      if (strcasestr8(buf, u8"h") || (atoi(buf) & 128)) { // NOMAGIC
        obj->SetSkill(prhash(u8"Magic Dead"), 1000);
      }
      //      if(strcasestr8(buf, u8"i") || (atoi(buf) & 256)) { //TUNNEL
      //	//FIXME: Implement
      //	}
      //      if(strcasestr8(buf, u8"j") || (atoi(buf) & 512)) { //PRIVATE
      //	//FIXME: Implement
      //	}
      //      if(strcasestr8(buf, u8"k") || (atoi(buf) & 1024)) { //GODROOM
      //	//FIXME: Implement
      //	}

      while (1) {
        int dnum, tnum, tmp, tmp2;
        fscanf(mud, u8"%c", buf);
        if (buf[0] == 'D') {
          fscanf(mud, u8"%d\n", &dnum);

          memset(buf, 0, 65536);
          fscanf(mud, u8"%65535[^~]", buf);
          fscanf(mud, u8"~%*[\n\r]");
          memset(buf, 0, 65536);
          fscanf(mud, u8"%65535[^~]", buf);
          fscanf(mud, u8"~%*[\n\r]");
          nmnum[dnum][obj] = buf;

          fscanf(mud, u8"%d %d %d\n", &tmp, &tmp2, &tnum);

          tonum[dnum][obj] = tnum;
          tynum[dnum][obj] = tmp;
          knum[dnum][obj] = tmp2;
        } else if (buf[0] == 'E') {
          fscanf(mud, u8"%*[^~]"); // FIXME: Load These!
          fscanf(mud, u8"~%*[\n\r]");
          fscanf(mud, u8"%*[^~]"); // FIXME: Load These!
          fscanf(mud, u8"~%*[\n\r]");
        } else if (buf[0] != 'S') {
          fprintf(stderr, u8"#%d: Warning, didn't see an ending S!\n", onum);
        } else {
          break;
        }
      }
      {
        int tnum;
        while (fscanf(mud, u8" T %d\n", &tnum) > 0) {
          if (tnum > 0 && bynumtrg.count(tnum) > 0) {
            Object* trg = new Object(*(bynumtrg[tnum]));
            trg->SetParent(obj);
            todotrg.push_back(trg);
            //    fprintf(stderr, u8"Put Trg \"%s\" on Room \"%s\"\n",
            //	trg->Desc(), obj->ShortDesc()
            //	);
          }
        }
      }
    }

    for (auto ob : olist) {
      for (int dir = 0; dir < 6; ++dir) {
        if (tonum[dir].count(ob)) {
          int tnum = tonum[dir][ob];
          if (bynumwld.count(tnum)) {
            Object* nobj = nullptr;
            Object* nobj2 = nullptr;
            std::u8string des, nm = dirname[dir];

            auto cont = ob->Contents();
            for (auto cind : cont) {
              if (cind->ShortDesc() == u8"a passage exit") {
                if (cind->ActTarg(act_t::SPECIAL_MASTER)->Parent() == bynumwld[tnum]) {
                  nobj = cind;
                  nobj2 = cind->ActTarg(act_t::SPECIAL_MASTER);
                }
              } else if (cind->ActTarg(act_t::SPECIAL_LINKED)) {
                if (cind->ActTarg(act_t::SPECIAL_LINKED)->Parent() == bynumwld[tnum]) {
                  nobj = cind;
                  nobj2 = cind->ActTarg(act_t::SPECIAL_LINKED);
                  nm = nobj->ShortDescS() + u8" and " + dirname[dir];
                }
              }
            }
            if (!nobj) {
              nobj = new Object;
              nobj2 = new Object;
              nobj->SetParent(ob);
              nobj2->SetParent(bynumwld[tnum]);
              nobj2->SetShortDesc(u8"a passage exit");
              nobj2->SetDesc(u8"A passage exit.");
              nobj2->SetSkill(prhash(u8"Invisible"), 1000);
            } else {
              nobj->SetSkill(prhash(u8"Invisible"), 0);
            }

            if (nmnum[dir][ob] != u8"") {
              nm += u8" (";
              nm += nmnum[dir][ob];
              nm += u8")";
            }
            if (tynum[dir][ob] != 0) { // FIXME: Respond to u8"door"?
              des = std::u8string(u8"A door to the ") + dirname[dir] + u8" is here.";
              nobj->SetSkill(prhash(u8"Closeable"), 1);
              nobj->SetSkill(prhash(u8"Lockable"), 1);
              if (tynum[dir][ob] == 1)
                nobj->SetSkill(prhash(u8"Pickable"), 4);
              if (tynum[dir][ob] == 2)
                nobj->SetSkill(prhash(u8"Pickable"), 1000);
              if (knum[dir][ob] > 0) {
                nobj->SetSkill(prhash(u8"Lock"), 1000000 + knum[dir][ob]);
                nobj->SetSkill(prhash(u8"Accomplishment"), 1300000 + tnum);
              }
            } else {
              des = std::u8string(u8"A passage ") + dirname[dir] + u8" is here.";
            }
            nobj->SetShortDesc(nm.c_str());
            nobj->SetDesc(des.c_str());
            nobj->SetSkill(prhash(u8"Open"), 1000);
            nobj->SetSkill(prhash(u8"Enterable"), 1);
            nobj->AddAct(act_t::SPECIAL_LINKED, nobj2);
            nobj2->AddAct(act_t::SPECIAL_MASTER, nobj);

            nmnum[dir].erase(ob);
            tonum[dir].erase(ob);
            tynum[dir].erase(ob);
            knum[dir].erase(ob);
          }
        }
      }
    }
    fclose(mud);
  } else {
    fprintf(stderr, u8"Error: No TBA Realm \"%s\"\n", fn.c_str());
  }
}

static const std::u8string base = u8"'^&*abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ ";
static std::set<std::u8string> parse_tba_shop_rules(std::u8string rules) {
  std::set<std::u8string> ret;
  if (rules[0]) {
    //    fprintf(stderr, u8"Initial: '%s'\n", rules.c_str());
    size_t done = rules.find_first_not_of(base);
    while (done != std::u8string::npos) {
      if (rules[done] == '|' || rules[done] == '+') {
        std::u8string first = rules.substr(0, done);
        trim_string(first);
        ret.insert(first);
        //	fprintf(stderr, u8"  Done: '%s'\n", first.c_str());
        rules = rules.substr(done + 1);
        trim_string(rules);
        done = rules.find_first_not_of(base);
      } else if (rules[done] == '(' || rules[done] == '[') {
        size_t end = rules.find_first_of(u8")]");
        if (end == std::u8string::npos)
          end = rules.length();
        std::set<std::u8string> tmp = parse_tba_shop_rules(rules.substr(done + 1));
        for (auto next : tmp) {
          ret.insert(next + rules.substr(end + 1));
          //	  fprintf(stderr, u8"  Built: '%s'\n",
          //		((*next) + rules.substr(end+1)).c_str()
          //		);
        }
        return ret; // FIXME: Handled multiple ()()()....
      } else if (rules[done] == ')' || rules[done] == ']') {
        std::u8string first = rules.substr(0, done);
        trim_string(first);
        //	fprintf(stderr, u8"  Done: '%s'\n", first.c_str());
        ret.insert(first);
        return ret; // End of sub-call
      } else {
        fprintf(
            stderr, CYEL u8"Warning: Can't handle shop rule fragment: '%s'\n" CNRM, rules.c_str());
        done = std::u8string::npos;
      }
    }
    //    fprintf(stderr, u8"  Done: '%s'\n", rules.c_str());
    ret.insert(rules);
  }
  return ret;
}

void Object::TBALoadSHP(const std::u8string& fn) {
  FILE* mud = fopen(fn.c_str(), u8"r");
  if (mud) {
    Object* vortex = nullptr;
    if (fscanf(mud, u8"CircleMUD v3.0 Shop File~%65535[\n\r]", buf) > 0) {
      while (1) {
        int val, kpr;
        if (!fscanf(mud, u8"#%d~\n", &val))
          break; // Shop Number
        // fprintf(stderr, u8"Loaded shop #%d\n", val);

        vortex = new Object;
        vortex->SetShortDesc(u8"a shopkeeper vortex");
        vortex->SetDesc(u8"An advanced wormhole that shopkeeper's use.");
        vortex->SetSkill(prhash(u8"Vortex"), 1); // Mark it as a shopkeeper Vortex.
        vortex->SetSkill(prhash(u8"Invisible"), 1000);
        vortex->SetSkill(prhash(u8"Perishable"), 1);
        vortex->SetSkill(prhash(u8"Wearable on Right Shoulder"), 1);
        vortex->SetSkill(prhash(u8"Wearable on Left Shoulder"), 2);

        fscanf(mud, u8"%d\n", &val); // Item sold
        while (val >= 0) {
          if (val != 0 && bynumobj.count(val) == 0) {
            fprintf(stderr, u8"Error: Shop's item #%d does not exist!\n", val);
          } else if (val != 0) {
            Object* item = new Object(*(bynumobj[val]));
            Object* item2 = dup_tba_obj(item);
            item->SetParent(vortex);
            item->SetSkill(prhash(u8"Quantity"), 1000);
            if (item2) {
              item2->SetParent(vortex);
              item2->SetSkill(prhash(u8"Quantity"), 1000);
            }
          }
          fscanf(mud, u8"%d\n", &val); // Item sold
        }

        double num, num2;
        fscanf(mud, u8"%lf\n", &num); // Profit when Sell
        fscanf(mud, u8"%lf\n", &num2); // Profit when Buy

        memset(buf, 0, 65536);
        fscanf(mud, u8"%65535[^\n\r]\n", buf); // Item types bought
        val = atoi(buf);
        std::vector<std::u8string> types;
        while (val >= 0) {
          types.push_back(std::u8string(buf));
          memset(buf, 0, 65536);
          fscanf(mud, u8"%65535[^\n\r]\n", buf); // Item types bought
          val = atoi(buf);
        }

        memset(buf, 0, 65536);
        for (int ctr = 0; ctr < 8; ++ctr) {
          fscanf(mud, u8"%255[^\n\r]\n", buf + strlen(buf));
        }

        memset(buf, 0, 65536);
        fscanf(mud, u8"%65535[^\n\r]\n", buf); // Shop Bitvectors

        fscanf(mud, u8"%d\n", &kpr); // Shopkeeper!
        Object* keeper = nullptr;
        if (bynummobinst.count(kpr))
          keeper = bynummobinst[kpr];

        memset(buf, 0, 65536);
        fscanf(mud, u8"%65535[^\n\r]\n", buf); // With Bitvectors

        fscanf(mud, u8"%d\n", &val); // Shop rooms
        while (val >= 0) {
          fscanf(mud, u8"%d\n", &val); // Shop rooms
        }

        fscanf(mud, u8"%*d\n"); // Open time
        fscanf(mud, u8"%*d\n"); // Close time
        fscanf(mud, u8"%*d\n"); // Open time
        fscanf(mud, u8"%*d\n"); // Close time

        if (keeper) {
          std::u8string picky = u8"";
          keeper->SetSkill(prhash(u8"Sell Profit"), (int)(num * 1000.0 + 0.5));

          for (auto type : types) { // Buy Types
            for (unsigned int ctr = 1; ascii_isalpha(type[ctr]); ++ctr) {
              type[ctr] = ascii_tolower(type[ctr]);
            }

            std::u8string extra = type;
            int itnum = atoi(extra.c_str());
            if (itnum > 0) {
              while (isdigit(extra[0]))
                extra = extra.substr(1);
            } else {
              while (isgraph(extra[0]))
                extra = extra.substr(1);
            }
            while (extra.length() > 0 && isspace(extra[0]))
              extra = extra.substr(1);

            if (itnum == 1)
              type = u8"Light";
            else if (itnum == 2)
              type = u8"Scroll";
            else if (itnum == 3)
              type = u8"Wand";
            else if (itnum == 4)
              type = u8"Staff";
            else if (itnum == 5)
              type = u8"Weapon";
            else if (itnum == 6)
              type = u8"Fire Weapon";
            else if (itnum == 7)
              type = u8"Missile";
            else if (itnum == 8)
              type = u8"Treasure";
            else if (itnum == 9)
              type = u8"Armor";
            else if (itnum == 10)
              type = u8"Potion";
            else if (itnum == 11)
              type = u8"Worn";
            else if (itnum == 12)
              type = u8"Other";
            else if (itnum == 13)
              type = u8"Trash";
            else if (itnum == 14)
              type = u8"Trap";
            else if (itnum == 15)
              type = u8"Container";
            else if (itnum == 16)
              type = u8"Note";
            else if (itnum == 17)
              type = u8"Liquid Container";
            else if (itnum == 18)
              type = u8"Key";
            else if (itnum == 19)
              type = u8"Food";
            else if (itnum == 20)
              type = u8"Money";
            else if (itnum == 21)
              type = u8"Pen";
            else if (itnum == 22)
              type = u8"Boat";
            else if (itnum == 23)
              type = u8"Fountain";
            else if (itnum == 55)
              type = u8"Cursed"; // According To: Rumble

            if (extra[0]) {
              // fprintf(stderr, u8"Rule: '%s'\n", extra.c_str());
              std::set<std::u8string> extras = parse_tba_shop_rules(extra);
              for (auto ex : extras) {
                // fprintf(stderr, u8"Adding: 'Accept %s'\n", ex.c_str());
                // keeper->SetSkill(prhash(u8"Accept ") + ex, 1);
                picky += (type + u8": " + ex + u8"\n");
              }
            } else {
              picky += type + u8": all\n";
            }

            if (type != u8"0" && type != u8"Light" && type != u8"Scroll" && type != u8"Wand" &&
                type != u8"Staff" && type != u8"Weapon" && type != u8"Fire Weapon" &&
                type != u8"Missile" && type != u8"Treasure" && type != u8"Armor" &&
                type != u8"Potion" && type != u8"Worn" && type != u8"Other" && type != u8"Trash" &&
                type != u8"Trap" && type != u8"Container" && type != u8"Note" &&
                type != u8"Liquid Container" && type != u8"Key" && type != u8"Food" &&
                type != u8"Money" && type != u8"Pen" && type != u8"Boat" && type != u8"Fountain") {
              fprintf(
                  stderr,
                  CYEL u8"Warning: Can't handle %s's buy target: '%s'\n" CNRM,
                  keeper->Noun().c_str(),
                  type.c_str());
            } else if (type != u8"0") { // Apparently 0 used for u8"Ignore This"
              keeper->SetSkill(std::u8string(u8"Buy ") + type, (int)(num2 * 1000.0 + 0.5));
            }
          }
          vortex->SetLongDesc(picky);
          vortex->SetParent(keeper);
          keeper->AddAct(act_t::WEAR_RSHOULDER, vortex);
        } else {
          vortex->Recycle();
          fprintf(stderr, CYEL u8"Warning: Can't find shopkeeper #%d!\n" CNRM, kpr);
        }
      }
    } else if (fscanf(mud, u8"%1[$]", buf) < 1) { // Not a Null Shop File!
      fprintf(stderr, u8"Error: '%s' is not a CircleMUD v3.0 Shop File!\n", fn.c_str());
    }
    fclose(mud);
  } else {
    fprintf(stderr, u8"Error: '%s' does not exist!\n", fn.c_str());
  }
}

void Object::TBALoadTRG(const std::u8string& fn) { // Triggers
  FILE* mud = fopen(fn.c_str(), u8"r");
  if (mud) {
    int tnum = -1;
    while (fscanf(mud, u8" #%d", &tnum) > 0) {
      Object* script = nullptr;
      script = new Object();
      bynumtrg[tnum] = script;
      script->SetSkill(prhash(u8"Invisible"), 1000);
      script->SetSkill(prhash(u8"TBAScript"), 1000000 + tnum);
      script->SetSkill(prhash(u8"Accomplishment"), 1200000 + tnum);
      script->SetShortDesc(u8"A tbaMUD trigger script");
      // fprintf(stderr, u8"Loading #%d\n", tnum);
      fscanf(mud, u8" %65535[^~]", buf); // Trigger Name - Discarded!
      // script->SetDesc(buf);
      fscanf(mud, u8"~");

      int atype, ttype, narg;
      fscanf(mud, u8" %d %65535s %d", &atype, buf, &narg);
      ttype = tba_bitvec(buf); // Trigger Types
      atype = 1 << (atype + 24); // Attach Type
      script->SetSkill(prhash(u8"TBAScriptType"), atype | ttype); // Combined
      script->SetSkill(prhash(u8"TBAScriptNArg"), narg); // Numeric Arg

      fscanf(mud, u8" %65535[^~]", buf); // Text argument!
      script->SetDesc(buf);

      fscanf(mud, u8"~");
      fscanf(mud, u8"%*[\n\r]"); // Go to next Line, don't eat spaces.
      fscanf(mud, u8"%[^~]~", buf); // Command List (Multi-Line)
      while (buf[strlen(buf) - 1] != '\n' && (!feof(mud))) { //~ in Middle
        buf[strlen(buf) + 1] = 0;
        buf[strlen(buf)] = '~';
        fscanf(mud, u8"%[^~]~", buf + strlen(buf));
      }
      script->SetLongDesc(buf);
      if (strstr(buf, u8"* Check the direction the player must go to enter the guild.")) {
        // char8_t dir[16];
        // char8_t* dirp = strstr(buf, u8"if %direction% == ");
        // if (dirp)
        //  sscanf(dirp + strlen(u8"if %direction% == "), u8"%s", dir);

        // char8_t cls[16];
        // char8_t* clsp = strstr(buf, u8"if %actor.class% != ");
        // if (clsp)
        //  sscanf(clsp + strlen(u8"if %actor.class% != "), u8"%s", cls);

        // if (dirp) {
        //  if (clsp) {
        //    fprintf(stderr, u8"%d appears to be a '%s' %s-guild guard trigger.\n", tnum, dir,
        //    cls);
        //  } else {
        //    fprintf(stderr, u8"%d appears to be a '%s' direction guard trigger.\n", tnum, dir);
        //  }
        //}
        ++untrans_trig; // This is NOT really handled yet.
      } else if (strstr(buf, u8"if %direction% == ")) {
        // fprintf(stderr, u8"%d appears to be a direction trigger.\n", tnum);
        ++untrans_trig; // This is NOT really handled yet.
      } else if (0) {
      } else {
        ++untrans_trig; // FIXME: Handle some more!
      }
    }
    fclose(mud);
  } else {
    fprintf(stderr, u8"Error: '%s' does not exist!\n", fn.c_str());
  }
}
