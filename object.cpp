#include <vector>
#include <map>
#include <set>

using namespace std;

#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>

#include "commands.h"
#include "object.h"
#include "stats.h"
#include "color.h"
#include "mind.h"
#include "net.h"

extern stats_t zero_stats;

const char *pos_str[POS_MAX] = {
        "is here",
        "is lying here",
        "is sitting here",
        "is standing here",
        };

const char *act_str[ACT_MAX] = {
        "doing nothing",
        "dead",
        "bleeding and dying",
        "unconscious",
        "fast asleep",
        "resting",
        "pointing %2$s%3$sat %1$s",
        "fighting %1$s",
        "wielding %1$s",
        "holding %1$s",
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
        };

static Object *universe;
static Object *default_initial;

static set<Object *> busylist;
extern timeval current_time; // From main.cpp

void set_start_room(Object *o) {
  default_initial = o;
  }

int matches(const char *name, const char *seek) {
  int len = strlen(seek);
  char *desc = (char*)name;
  while(*desc) {
    if((!strncasecmp(desc, seek, len)) && (!isalnum(desc[len]))) {
      return 1;
      }
    while(isalnum(*desc)) ++desc;
    while((!isalnum(*desc)) && (*desc)) ++desc;
    }

  static int dont_recur;
  if(dont_recur) return 0;
  dont_recur = 1;

  int ret = 0;

  //special SMART[TM] searches
  if((!strcasecmp(seek, "guard")) && matches(name, "cityguard")) ret=1;
  else if((!strcasecmp(seek, "guard")) && matches(name, "guildguard")) ret=1;
  else if((!strcasecmp(seek, "guard")) && matches(name, "gateguard")) ret=1;
  else if((!strcasecmp(seek, "guard")) && matches(name, "bodyguard")) ret=1;
  else if((!strcasecmp(seek, "merc")) && matches(name, "mercenary")) ret=1;
  else if((!strcasecmp(seek, "smith")) && matches(name, "weaponsmith")) ret=1;
  else if((!strcasecmp(seek, "smith")) && matches(name, "swordsmith")) ret=1;
  else if((!strcasecmp(seek, "smith")) && matches(name, "blacksmith")) ret=1;
  else if((!strcasecmp(seek, "bolt")) && matches(name, "thunderbolt")) ret=1;
  else if((!strcasecmp(seek, "sword")) && matches(name, "longsword")) ret=1;
  else if((!strcasecmp(seek, "sword")) && matches(name, "broadsword")) ret=1;
  else if((!strcasecmp(seek, "sword")) && matches(name, "shortsword")) ret=1;
  else if((!strcasecmp(seek, "hammer")) && matches(name, "warhammer")) ret=1;
  else if((!strcasecmp(seek, "hammer")) && matches(name, "battlehammer")) ret=1;
  else if((!strcasecmp(seek, "battle hammer")) && matches(name, "battlehammer")) ret=1;
  else if((!strcasecmp(seek, "battlehammer")) && matches(name, "battle hammer")) ret=1;
  else if((!strcasecmp(seek, "war hammer")) && matches(name, "warhammer")) ret=1;
  else if((!strcasecmp(seek, "warhammer")) && matches(name, "war hammer")) ret=1;
  else if((!strcasecmp(seek, "axe")) && matches(name, "waraxe")) ret=1;
  else if((!strcasecmp(seek, "axe")) && matches(name, "battleaxe")) ret=1;
  else if((!strcasecmp(seek, "battle axe")) && matches(name, "battleaxe")) ret=1;
  else if((!strcasecmp(seek, "battleaxe")) && matches(name, "battle axe")) ret=1;
  else if((!strcasecmp(seek, "war axe")) && matches(name, "waraxe")) ret=1;
  else if((!strcasecmp(seek, "waraxe")) && matches(name, "war axe")) ret=1;
  else if((!strcasecmp(seek, "morning star")) && matches(name, "morningstar")) ret=1;
  else if((!strcasecmp(seek, "morningstar")) && matches(name, "morning star")) ret=1;
  else if((!strcasecmp(seek, "bisarme")) && matches(name, "gisarme")) ret=1;
  else if((!strcasecmp(seek, "bisarme")) && matches(name, "guisarme")) ret=1;
  else if((!strcasecmp(seek, "gisarme")) && matches(name, "bisarme")) ret=1;
  else if((!strcasecmp(seek, "gisarme")) && matches(name, "guisarme")) ret=1;
  else if((!strcasecmp(seek, "guisarme")) && matches(name, "bisarme")) ret=1;
  else if((!strcasecmp(seek, "guisarme")) && matches(name, "gisarme")) ret=1;
  else if((!strcasecmp(seek, "bill-bisarme")) && matches(name, "bill-gisarme")) ret=1;
  else if((!strcasecmp(seek, "bill-bisarme")) && matches(name, "bill-guisarme")) ret=1;
  else if((!strcasecmp(seek, "bill-gisarme")) && matches(name, "bill-bisarme")) ret=1;
  else if((!strcasecmp(seek, "bill-gisarme")) && matches(name, "bill-guisarme")) ret=1;
  else if((!strcasecmp(seek, "bill-guisarme")) && matches(name, "bill-bisarme")) ret=1;
  else if((!strcasecmp(seek, "bill-guisarme")) && matches(name, "bill-gisarme")) ret=1;
  else if((!strcasecmp(seek, "staff")) && matches(name, "quarterstaff")) ret=1;
  else if((!strcasecmp(seek, "bow")) && matches(name, "longbow")) ret=1;
  else if((!strcasecmp(seek, "bow")) && matches(name, "shortbow")) ret=1;

  dont_recur = 0;
  return ret;
  }

Object *get_start_room() {
  return default_initial;
  }

Object *new_body() {
  static stats_t bod_stats;
  bod_stats.SetAttribute(0, 3);
  bod_stats.SetAttribute(1, 3);
  bod_stats.SetAttribute(2, 3);
  bod_stats.SetAttribute(3, 3);
  bod_stats.SetAttribute(4, 3);
  bod_stats.SetAttribute(5, 3);

  Object *body = new Object();
  body->SetShortDesc("an amorphous blob");
  body->SetParent(default_initial);
  body->SetStats(bod_stats);
  body->SetPos(POS_STAND);
  return body;
  }

Object::Object() {
  busytill.tv_sec = 0;
  busytill.tv_usec = 0;
  short_desc = "new object";
  parent = NULL;
  pos = POS_NONE;
  stats = zero_stats;
  }

Object::Object(Object *o) {
  busytill.tv_sec = 0;
  busytill.tv_usec = 0;
  short_desc = "new object";
  parent = NULL;
  SetParent(o);
  pos = POS_NONE;
  stats = zero_stats;
  }

Object::Object(const Object &o) {
  busytill.tv_sec = 0;
  busytill.tv_usec = 0;
  short_desc = o.short_desc;
  desc = o.desc;
  long_desc = o.long_desc;
  stats = o.stats;
  act.clear();
  pos = o.pos;

  contents.clear();
  set<Object*>::const_iterator ind;
  for(ind = o.contents.begin(); ind != o.contents.end(); ++ind) {
    Object *nobj = new Object(*(*ind));
    nobj->SetParent(this);
    if(o.ActTarg(ACT_WIELD) == (*ind)) AddAct(ACT_WIELD, nobj);
    if(o.ActTarg(ACT_HOLD) == (*ind)) AddAct(ACT_HOLD, nobj);
    for(act_t act=ACT_WEAR_BACK; act <= ACT_WEAR_RWRIST; ++((int&)(act)))
      if(o.ActTarg(act) == (*ind)) AddAct(act, nobj);
    }
  if(o.IsAct(ACT_DEAD)) AddAct(ACT_DEAD);
  if(o.IsAct(ACT_DYING)) AddAct(ACT_DYING);
  if(o.IsAct(ACT_UNCONSCIOUS)) AddAct(ACT_UNCONSCIOUS);
  if(o.IsAct(ACT_ASLEEP)) AddAct(ACT_ASLEEP);
  if(o.IsAct(ACT_REST)) AddAct(ACT_REST);

  parent = NULL;
  }

const char *Object::ShortDesc() {
  return short_desc.c_str();
  }

const char *Object::Desc() {
  if(desc.length() <= 0) return ShortDesc();
  return desc.c_str();
  }

const char *Object::LongDesc() {
  if(long_desc.length() <= 0) return Desc();
  return long_desc.c_str();
  }

static void trim(string &s) {
  while(!isgraph(s[0])) s = s.substr(1);
  while(!isgraph(s[s.length()-1])) s = s.substr(0, s.length()-1);
  }

void Object::SetShortDesc(const char *d) {
  short_desc = d;
  trim(short_desc);
  }

void Object::SetDesc(const char *d) {
  desc = d;
  trim(desc);
  }

void Object::SetLongDesc(const char *d) {
  long_desc = d;
  trim(long_desc);
  }

void Object::LinkTo(const char *n, Object *o) {
  connections[n] = o;
  if(!strcmp(n, "north")) o->connections["south"] = this;
  if(!strcmp(n, "south")) o->connections["north"] = this;
  if(!strcmp(n, "east")) o->connections["west"] = this;
  if(!strcmp(n, "west")) o->connections["east"] = this;
  if(!strcmp(n, "up")) o->connections["down"] = this;
  if(!strcmp(n, "down")) o->connections["up"] = this;
  }

void Object::LinkToNew(const char *n) {
  Object *o = new Object(parent);
  LinkTo(n, o);
  }

void Object::SetParent(Object *o) {
  parent = o;
  if(o) o->contents.insert(this);
  }

void Object::SendContents(Object *targ, Object *o, int seeinside) {
  set<Mind*>::iterator m = targ->minds.begin();
  for(; m != targ->minds.end(); ++m) {
    SendContents(*m, o, seeinside);
    }
  }

void Object::SendShortDesc(Object *targ, Object *o) {
  set<Mind*>::iterator m = targ->minds.begin();
  for(; m != targ->minds.end(); ++m) {
    SendShortDesc(*m, o);
    }
  }

void Object::SendDesc(Object *targ, Object *o) {
  set<Mind*>::iterator m = targ->minds.begin();
  for(; m != targ->minds.end(); ++m) {
    SendDesc(*m, o);
    }
  }

void Object::SendLongDesc(Object *targ, Object *o) {
  set<Mind*>::iterator m = targ->minds.begin();
  for(; m != targ->minds.end(); ++m) {
    SendLongDesc(*m, o);
    }
  }

static string base = "";
static char buf[65536];

void Object::SendActions(Mind *m) {
  map<act_t,Object*>::iterator cur;
  for(cur = act.begin(); cur != act.end(); ++cur) {
    if(cur->first < ACT_WEAR_BACK) {
      char *targ, *dirn = "", *dirp = "";

      if(!cur->second) targ = "";
      else if((cur->second) == this) targ = "itself";
      else if((cur->second) == m->Body()) targ = "you";
      else targ = (char*) cur->second->ShortDesc();

      map<string,Object*>::iterator dir = connections.begin();
      for(; dir != connections.end(); ++dir) {
	if((*dir).second == cur->second) {
	  dirn = (char*) (*dir).first.c_str();
	  dirp = " ";
	  break;
	  }
	}
      m->Send(", ");
      m->Send(act_str[cur->first], targ, dirn, dirp);
      }
    }
  m->Send(".\n");
  }

void Object::SendExtendedActions(Mind *m, int seeinside) {
  map<act_t,Object*>::iterator cur;
  for(cur = act.begin(); cur != act.end(); ++cur) {
    if(cur->first == ACT_WIELD) m->Send("%20s", "Wielded: ");
    else if(cur->first == ACT_HOLD) m->Send("%20s", "Held: ");
    else if(cur->first == ACT_WEAR_BACK) m->Send("%20s", "Worn on back: ");
    else if(cur->first == ACT_WEAR_CHEST) m->Send("%20s", "Worn on chest: ");
    else if(cur->first == ACT_WEAR_HEAD) m->Send("%20s", "Worn on head: ");
    else if(cur->first == ACT_WEAR_NECK) m->Send("%20s", "Worn on neck: ");
    else if(cur->first == ACT_WEAR_WAIST) m->Send("%20s", "Worn on waist: ");
    else if(cur->first == ACT_WEAR_SHIELD) m->Send("%20s", "Worn as shield: ");
    else if(cur->first == ACT_WEAR_LARM) m->Send("%20s", "Worn on left arm: ");
    else if(cur->first == ACT_WEAR_RARM) m->Send("%20s", "Worn on right arm: ");
    else if(cur->first == ACT_WEAR_LFINGER) m->Send("%20s", "Worn on left finger: ");
    else if(cur->first == ACT_WEAR_RFINGER) m->Send("%20s", "Worn on right finger: ");
    else if(cur->first == ACT_WEAR_LFOOT) m->Send("%20s", "Worn on left foot: ");
    else if(cur->first == ACT_WEAR_RFOOT) m->Send("%20s", "Worn on right foot: ");
    else if(cur->first == ACT_WEAR_LHAND) m->Send("%20s", "Worn on left hand: ");
    else if(cur->first == ACT_WEAR_RHAND) m->Send("%20s", "Worn on right hand: ");
    else if(cur->first == ACT_WEAR_LLEG) m->Send("%20s", "Worn on left leg: ");
    else if(cur->first == ACT_WEAR_RLEG) m->Send("%20s", "Worn on right leg: ");
    else if(cur->first == ACT_WEAR_LWRIST) m->Send("%20s", "Worn on left wrist: ");
    else if(cur->first == ACT_WEAR_RWRIST) m->Send("%20s", "Worn on right wrist: ");
    else continue;

    char *targ;
    if(!cur->second) targ = "";
    else if((cur->second) == this) targ = "itself";
    else if((cur->second) == m->Body()) targ = "you";
    else targ = (char*) cur->second->ShortDesc();

    m->Send("%s%s.\n%s", CGRN, targ, CNRM);

    if(seeinside || cur->second->Stats()->GetSkill("Transparent")) {
      sprintf(buf, "%20s  %c", " ", 0);
      base = buf;
      cur->second->SendContents(m, NULL, seeinside);
      base = "";
      m->Send("%s", CNRM);
      }
    }
  }

void Object::SendContents(Mind *m, Object *o, int seeinside) {
  set<Object*> cont = contents;

  if(pos != POS_NONE) {
    for(act_t act = ACT_WIELD; act < ACT_MAX; ++((int&)(act))) {
      cont.erase(ActTarg(act));
      }
    }

  int total = 0;
  m->Send("%s", CGRN);
  set<Object*>::iterator ind;
  for(ind = cont.begin(); ind != cont.end(); ++ind) {
    if(total >= 20) {
      m->Send(base.c_str());
      m->Send("...and %d more things are here too.\n",
	cont.size() - total);
      break;
      }
    ++total;
    if((*ind) != o) {
      if(base != "") m->Send("%s%sInside:%s ", base.c_str(), CNRM, CGRN);

      sprintf(buf, "%s %s%c", (*ind)->ShortDesc(), (*ind)->PosString(), 0);
      buf[0] = toupper(buf[0]);
      m->Send(buf);

      (*ind)->SendActions(m);

      if(seeinside || (*ind)->Stats()->GetSkill("Transparent")) {
	string tmp = base;
	base += "  ";
	(*ind)->SendContents(m, o, seeinside);
	base = tmp;
	}
      }
    }
  }

void Object::SendShortDesc(Mind *m, Object *o) {
  memset(buf, 0, 65536);
  sprintf(buf, "%s\n", ShortDesc());
  m->Send(buf);
  }

void Object::SendDesc(Mind *m, Object *o) {
  memset(buf, 0, 65536);

  if(pos != POS_NONE) {
    m->Send("%s", CCYN);
    sprintf(buf, "%s %s%c", ShortDesc(), PosString(), 0);
    buf[0] = toupper(buf[0]);
    m->Send(buf);

    SendActions(m);
    }
  else {
    m->Send("%s", CCYN);
    sprintf(buf, "%s\n%c", ShortDesc(), 0);
    buf[0] = toupper(buf[0]);
    m->Send(buf);
    }

  m->Send("%s   ", CNRM);
  sprintf(buf, "%s\n%c", Desc(), 0);
  buf[0] = toupper(buf[0]);
  m->Send(buf);

  if(pos == POS_NONE) {
    m->Send("%s", CCYN);
    map<string, Object*>::iterator mind;
    for(mind = connections.begin(); mind != connections.end(); ++mind) {
      sprintf(buf, "%s you see %s.\n%c",
	(*mind).first.c_str(), (*mind).second->ShortDesc(), 0);
      buf[0] = toupper(buf[0]);
      m->Send(buf);
      }
    }

  m->Send("%s", CNRM);
  if(pos != POS_NONE) {
    SendExtendedActions(m, 0);
    }

//  if(m->Body() == this) {
//    SendContents(m, o, 1);
//    }
//  else 
  if(pos == POS_NONE || Stats()->GetSkill("Transparent")) {
    SendContents(m, o);
    }

  if(parent && Stats()->GetSkill("Transparent")) {
    m->Send("%s", CCYN);
    m->Send("Outside you see: ");
    parent->SendDesc(m, this);
    }

//  m->Send("%s", CMAG);
  m->Send("%s", CNRM);
  }

void Object::SendLongDesc(Mind *m, Object *o) {
  if(pos != POS_NONE) {
    m->Send("%s", CCYN);
    sprintf(buf, "%s %s%c", ShortDesc(), PosString(), 0);
    buf[0] = toupper(buf[0]);
    m->Send(buf);

    SendActions(m);
    }
  else {
    m->Send("%s", CCYN);
    sprintf(buf, "%s\n%c", ShortDesc(), 0);
    buf[0] = toupper(buf[0]);
    m->Send(buf);
    }

  m->Send("%s   ", CNRM);
  sprintf(buf, "%s\n%c", Desc(), 0);
  buf[0] = toupper(buf[0]);
  m->Send(buf);

  if(pos == POS_NONE) {
    m->Send("%s", CCYN);
    map<string, Object*>::iterator mind;
    for(mind = connections.begin(); mind != connections.end(); ++mind) {
      sprintf(buf, "%s you see %s.\n%c",
	(*mind).first.c_str(), (*mind).second->ShortDesc(), 0);
      buf[0] = toupper(buf[0]);
      m->Send(buf);
      }
    }

  m->Send("%s", CCYN);
  map<string, Object*>::iterator mind;
  for(mind = connections.begin(); mind != connections.end(); ++mind) {
    sprintf(buf, "%s you see %s.\n%c",
	(*mind).first.c_str(), (*mind).second->ShortDesc(), 0);
    buf[0] = toupper(buf[0]);
    m->Send(buf);
    }

  m->Send("%s", CNRM);
  if(pos != POS_NONE) {
    SendExtendedActions(m, 1);
    }

//  if(m->Body() == this) {
//    SendContents(m, o, 1);
//    }
//  else if(pos == POS_NONE || Stats()->GetSkill("Transparent")) {
//    SendContents(m, o);
//    }

  SendContents(m, o, 1);

  m->Send("%s\n", CNRM);
  m->Send("Bod: %2d", stats.GetAttribute(0));
  m->Send("           L     M        S           D\n");
  m->Send("Qui: %2d", stats.GetAttribute(1));
  m->Send("    Stun: [%c][%c][%c][%c][%c][%c][%c][%c][%c][%c]",
	stats.stun <= 0 ? ' ' : 'X',
	stats.stun <= 1 ? ' ' : 'X',
	stats.stun <= 2 ? ' ' : 'X',
	stats.stun <= 3 ? ' ' : 'X',
	stats.stun <= 4 ? ' ' : 'X',
	stats.stun <= 5 ? ' ' : 'X',
	stats.stun <= 6 ? ' ' : 'X',
	stats.stun <= 7 ? ' ' : 'X',
	stats.stun <= 8 ? ' ' : 'X',
	stats.stun <= 9 ? ' ' : 'X'
	);
  m->Send("\n");
  m->Send("Str: %2d", stats.GetAttribute(2));
  m->Send("    Phys: [%c][%c][%c][%c][%c][%c][%c][%c][%c][%c]",
	stats.phys <= 0 ? ' ' : 'X',
	stats.phys <= 1 ? ' ' : 'X',
	stats.phys <= 2 ? ' ' : 'X',
	stats.phys <= 3 ? ' ' : 'X',
	stats.phys <= 4 ? ' ' : 'X',
	stats.phys <= 5 ? ' ' : 'X',
	stats.phys <= 6 ? ' ' : 'X',
	stats.phys <= 7 ? ' ' : 'X',
	stats.phys <= 8 ? ' ' : 'X',
	stats.phys <= 9 ? ' ' : 'X'
	);
  if(stats.phys > 10) {
    m->Send(" Overflow: %d", stats.phys-10);
    }
  m->Send("\n");
  m->Send("Cha: %2d", stats.GetAttribute(3));
  m->Send("    Stru: [%c][%c][%c][%c][%c][%c][%c][%c][%c][%c]",
	stats.stru <= 0 ? ' ' : 'X',
	stats.stru <= 1 ? ' ' : 'X',
	stats.stru <= 2 ? ' ' : 'X',
	stats.stru <= 3 ? ' ' : 'X',
	stats.stru <= 4 ? ' ' : 'X',
	stats.stru <= 5 ? ' ' : 'X',
	stats.stru <= 6 ? ' ' : 'X',
	stats.stru <= 7 ? ' ' : 'X',
	stats.stru <= 8 ? ' ' : 'X',
	stats.stru <= 9 ? ' ' : 'X'
	);
  m->Send("\n");
  m->Send("Int: %2d", stats.GetAttribute(4));
  m->Send("\n");
  m->Send("Wil: %2d", stats.GetAttribute(5));

  map<string,int> skills = stats.GetSkills();
  map<string,int>::iterator skl;
  if(skills.count("WeaponType")) {
    static char sevs[] = { '-', 'L', 'M', 'S', 'D' };
    m->Send("    %s: (Str+%d)%c",
	get_weapon_skill(skills["WeaponType"]).c_str(),
	skills["WeaponForce"], sevs[4 <? skills["WeaponSeverity"]]);
    if(skills["WeaponSeverity"] > 4)
      m->Send("%d", (skills["WeaponSeverity"]-4)*2);
    if(skills["WeaponReach"] > 4)
      m->Send("  Range: %d", skills["WeaponReach"]);
    else if(skills["WeaponReach"] >= 0)
      m->Send("  Reach: %d", skills["WeaponReach"]);
    }
  else {
    for(skl = skills.begin(); skl != skills.end(); ++skl) {
      if(skl != skills.begin()) m->Send("\n       ");
      m->Send("    %16s: %2d", skl->first.c_str(),skl->second);
      }
    }

  m->Send("\n");
  m->Send("%s", CNRM);
  }

void Object::AddLink(Object *ob) {
  contents.insert(ob);
  }

int Object::Travel(Object *dest) {
  if((!parent) || (!dest)) return -1;
  parent->contents.erase(this);
  if(parent->ActTarg(ACT_WIELD) == this) parent->StopAct(ACT_WIELD);
  if(parent->ActTarg(ACT_HOLD) == this) parent->StopAct(ACT_HOLD);
  for(act_t act=ACT_WEAR_BACK; act <= ACT_WEAR_RWRIST; ++((int&)(act)))
    if(parent->ActTarg(act) == this) parent->StopAct(act);
  parent = dest;
  parent->contents.insert(this);
  StopAct(ACT_POINT);
  return 0;
  }

Object::~Object() {
  if(default_initial == this) default_initial = universe;
  if(parent->ActTarg(ACT_WIELD) == this) parent->StopAct(ACT_WIELD);
  if(parent->ActTarg(ACT_HOLD) == this) parent->StopAct(ACT_HOLD);
  for(act_t act=ACT_WEAR_BACK; act <= ACT_WEAR_RWRIST; ++((int&)(act)))
    if(parent->ActTarg(act) == this) parent->StopAct(act);

  set<Object*> movers;
  set<Object*>::iterator ind;
  for(ind = contents.begin(); ind != contents.end(); ++ind) {
    if(is_pc(*ind)) movers.insert(*ind);
    else delete(*ind);
    }
  contents.clear();
  for(ind = movers.begin(); ind != movers.end(); ++ind) {
    (*ind)->StopAll();
    set<Object*>::iterator ind2 = (*ind)->contents.begin();
    for(; ind2 != (*ind)->contents.end(); ++ind2) {
      delete(*ind2);
      }
    (*ind)->Travel(default_initial);
    }

  player_rooms_erase(this);

  set<Mind*>::iterator mind;
  for(mind = minds.begin(); mind != minds.end(); ++mind) {
    Unattach(*mind);
    }
  minds.clear();

  parent->contents.erase(this);
  busylist.erase(this);
  }

void Object::Attach(Mind *m) {
  minds.insert(m);
  }

void Object::Unattach(Mind *m) {
  minds.erase(m);
  if(m->Body() == this) {
    m->Unattach();
    }
  }

int get_ordinal(char *text) {
  int ret = 0, len = 0;
  while((!isgraph(*text)) && (*text)) ++text;
  while(isgraph(text[len])) ++len;
  if(isdigit(*text)) {
    char *suf = text;
    while(isdigit(*suf)) ++suf;
    if(!strncasecmp(suf, "st", 2)) ret = atoi(text);
    else if(!strncasecmp(suf, "nd", 2)) ret = atoi(text);
    else if(!strncasecmp(suf, "rd", 2)) ret = atoi(text);
    else if(!strncasecmp(suf, "th", 2)) ret = atoi(text);
    if(ret && isgraph(suf[2])) ret = 0;
    if(suf[0] == '.') ret = atoi(text);
    }
  else if(!strncasecmp(text, "first",  len>?5)) ret = 1;
  else if(!strncasecmp(text, "second", len>?6)) ret = 2;
  else if(!strncasecmp(text, "third",  len>?5)) ret = 3;
  else if(!strncasecmp(text, "fourth", len>?6)) ret = 4;
  else if(!strncasecmp(text, "fifth",  len>?5)) ret = 5;
  else if(!strncasecmp(text, "sixth",  len>?5)) ret = 6;
  else if(!strncasecmp(text, "seventh",len>?7)) ret = 7;
  else if(!strncasecmp(text, "eighth", len>?6)) ret = 8;
  else if(!strncasecmp(text, "ninth",  len>?5)) ret = 9;
  else if(!strncasecmp(text, "tenth",  len>?5)) ret = 10;
  return ret;
  }

int strip_ordinal(char **text) {
  int ret = get_ordinal(*text);
  if(ret) {
    while((!isgraph(**text)) && (**text)) ++(*text);
    while((isgraph(**text)) && (**text) != '.') ++(*text);
    while(((!isgraph(**text)) || (**text) == '.') && (**text)) ++(*text);
    }
  return ret;
  }

Object *Object::PickObject(char *name, int loc, int *ordinal) {
  while((!isgraph(*name)) && (*name)) ++name;

  if(!(*name)) return NULL;

  char *keyword = NULL;
  char *keyword2 = NULL;
  if((keyword = strstr(name, "'s ")) || (keyword2 = strstr(name, "'S "))) {
    if(keyword && keyword2) keyword = keyword <? keyword2;
    else if(!keyword) keyword = keyword2;
    keyword2 = strdup(name);
    keyword2[keyword-name] = 0;
    Object *master = PickObject(keyword2, loc);
    if(!master) { free(keyword2); return NULL; }
    Object *ret = master->PickObject(keyword2 + (keyword-name)+3, LOC_INTERNAL);
    free(keyword2);
    return ret;
    }

  int ordcontainer;
  if(ordinal) strip_ordinal(&name);
  else { ordinal = &ordcontainer; (*ordinal) = strip_ordinal(&name); }
  if(!(*ordinal)) (*ordinal) = 1;

  int len = strlen(name);
  while(!isgraph(name[len-1])) --len;

  if(loc & LOC_SELF) {
    if((!strcasecmp(name, "self")) || (!strcasecmp(name, "myself"))) {
      if((*ordinal) != 1) return NULL;
      return this;
      }
    }

  if(loc & LOC_INTERNAL) {
    if(!strncasecmp(name, "my ", 3)) {
      name += 3;
      return PickObject(name, LOC_INTERNAL|LOC_SELF);
      }
    }

  if(loc & LOC_ADJACENT) {
    char *dir = name;
    if(!strcasecmp(dir, "n")) dir = "north";
    if(!strcasecmp(dir, "s")) dir = "south";
    if(!strcasecmp(dir, "w")) dir = "west";
    if(!strcasecmp(dir, "e")) dir = "east";
    if(!strcasecmp(dir, "u")) dir = "up";
    if(!strcasecmp(dir, "d")) dir = "down";

    if(parent->connections.count(dir) > 0) {
      (*ordinal)--;
      if((*ordinal)==0) return parent->connections[dir];
      }
    }

  if(loc & LOC_INTERNAL) {
    set<Object*>::iterator ind;
    for(ind = contents.begin(); ind != contents.end(); ++ind) {
      if((*ind) == this) continue;  // Must use "self" to pick self!
      if(matches((*ind)->Name(), name)) {
	(*ordinal)--;
	if((*ordinal) == 0) { return (*ind); }
	}
      }
    }

  if(loc & LOC_NEARBY) {
    set<Object*>::iterator ind;
    for(ind = parent->contents.begin(); ind != parent->contents.end(); ++ind) {
      if((*ind) == this) continue;  // Must use "self" to pick self!
      if(matches((*ind)->Name(), name)) {
	(*ordinal)--;
	if((*ordinal) == 0) { return (*ind); }
	}
      if((*ind)->Stats()->GetSkill("Transparent")) {
	Object *ret = (*ind)->PickObject(name, LOC_INTERNAL, ordinal);
	if(ret) return ret;
	}
      }
    if(parent->Stats()->GetSkill("Transparent")) {
      if(parent->parent) {
	parent->parent->contents.erase(parent);
	Object *ret = parent->PickObject(name, LOC_NEARBY, ordinal);
	parent->parent->contents.insert(parent);
	if(ret) return ret;
	}
      }
    }

  return NULL;
  }

int Object::IsWithin(Object *obj) {
  set<Object*>::iterator ind;
  for(ind = contents.begin(); ind != contents.end(); ++ind) {
    if((*ind) == obj) return 1;
    if((*ind)->Stats()->GetSkill("Transparent")) {
      int ret = (*ind)->IsWithin(obj);
      if(ret) return ret;
      }
    }
  return 0;
  }

int Object::IsNearBy(Object *obj) {
  if(!parent) return 0;
  set<Object*>::iterator ind;
  for(ind = parent->contents.begin(); ind != parent->contents.end(); ++ind) {
    if((*ind) == obj) return 1;
    if((*ind) == this) continue;  // Not Nearby Self
    if((*ind)->Stats()->GetSkill("Transparent")) {
      int ret = (*ind)->IsWithin(obj);
      if(ret) return ret;
      }
    }
  if(parent->parent && parent->Stats()->GetSkill("Transparent")) {
    parent->parent->contents.erase(parent);
    int ret = parent->IsNearBy(obj);
    parent->parent->contents.insert(parent);
    if(ret) return ret;
    }
  return 0;
  }

Object *Object::ActTarg(act_t a) const {
  if(act.count(a)) return (act.find(a))->second;
  return NULL;
  };

void Object::StopAll() {
  if(parent) {
    if(IsAct(ACT_WIELD)) {
      parent->SendOut(";s drops %s!\n", "You drop %s!\n",
		this, NULL, ActTarg(ACT_WIELD)->ShortDesc());
      ActTarg(ACT_WIELD)->Travel(parent);
      }
    if(IsAct(ACT_HOLD)) {
      parent->SendOut(";s drops %s!\n", "You drop %s!\n",
		this, NULL, ActTarg(ACT_HOLD)->ShortDesc());
      ActTarg(ACT_HOLD)->Travel(parent);
      }
    }
  act.clear();
  }

void Object::Collapse() {
  StopAct(ACT_DEAD);
  StopAct(ACT_DYING);
  StopAct(ACT_UNCONSCIOUS);
  StopAct(ACT_ASLEEP);
  StopAct(ACT_REST);
  StopAct(ACT_POINT);
  StopAct(ACT_FIGHT);
  if(parent) {
    if(pos != POS_LIE) {
      parent->SendOut(";s collapses!\n", "You collapse!\n", this, NULL);
      pos = POS_LIE;
      }
    if(IsAct(ACT_WIELD)) {
      parent->SendOut(";s drops %s!\n", "You drop %s!\n",
		this, NULL, ActTarg(ACT_WIELD)->ShortDesc());
      ActTarg(ACT_WIELD)->Travel(parent);
      }
    if(IsAct(ACT_HOLD)) {
      parent->SendOut(";s drops %s!\n", "You drop %s!\n",
		this, NULL, ActTarg(ACT_HOLD)->ShortDesc());
      ActTarg(ACT_HOLD)->Travel(parent);
      }
    }
  }

void Object::UpdateDamage() {
  if(stats.stun > 10) {
    stats.phys += stats.stun-10;
    stats.stun = 10;
    }
  if(stats.phys >= 10+stats.GetAttribute(0)) {
    if(IsAct(ACT_DEAD) == 0) {
      parent->SendOut(
	";s expires from its wounds.\n", "You expire, sorry.\n", this, NULL);
      stats.stun = 10;
      Collapse();
      AddAct(ACT_DEAD);
      set<Mind*>::iterator mind;
      for(mind = minds.begin(); mind != minds.end(); ++mind) {
	Unattach(*mind);
	}
      minds.clear();
      }
    SetPos(POS_LIE);
    }
  else if(stats.phys >= 10) {
    if(IsAct(ACT_DYING)+IsAct(ACT_DEAD) == 0) {
      parent->SendOut(
	";s collapses, bleeding and dying!\n",
	"You collapse, bleeding and dying!\n",
	this, NULL);
      stats.stun = 10;
      Collapse();
      AddAct(ACT_DYING);
      }
    else if(IsAct(ACT_DEAD) == 0) {
      parent->SendOut(
	";s isn't quite dead yet!\n", "You aren't quite dead yet!\n",
	this, NULL);
      StopAct(ACT_DEAD);
      AddAct(ACT_DYING);
      }
    SetPos(POS_LIE);
    }
  else if(stats.stun >= 10) {
    if(IsAct(ACT_UNCONSCIOUS)+IsAct(ACT_DYING)+IsAct(ACT_DEAD)==0) {
      parent->SendOut(
	";s falls unconscious!\n", "You fall unconscious!\n", this, NULL);
      Collapse();
      AddAct(ACT_UNCONSCIOUS);
      }
    else if(IsAct(ACT_DEAD)+IsAct(ACT_DYING) != 0) {
      parent->SendOut(
	";s isn't dead, just out cold.\n", "You aren't dead, just unconscious.",
	this, NULL);
      StopAct(ACT_DEAD);
      StopAct(ACT_DYING);
      AddAct(ACT_UNCONSCIOUS);
      }
    SetPos(POS_LIE);
    }
  else if(stats.stun > 0) {
    if(IsAct(ACT_DEAD)+IsAct(ACT_DYING)+IsAct(ACT_UNCONSCIOUS) != 0) {
      parent->SendOut(
	";s wakes up, a little groggy.\n", "You wake up, a little groggy.",
	this, NULL);
      StopAct(ACT_DEAD);
      StopAct(ACT_DYING);
      StopAct(ACT_UNCONSCIOUS);
      }
    }
  else {
    if(IsAct(ACT_DEAD)+IsAct(ACT_DYING)+IsAct(ACT_UNCONSCIOUS) != 0) {
      parent->SendOut(
	";s wakes up, feeling fine!\n", "You wake up, feeling fine!",
	this, NULL);
      StopAct(ACT_DEAD);
      StopAct(ACT_DYING);
      StopAct(ACT_UNCONSCIOUS);
      }
    }
  }

int Object::HealStun(int boxes) {
  if(stats.GetAttribute(1) <= 0) return 0;
  if(stats.phys >= 10) return 0;
  if(boxes > stats.stun) boxes = stats.stun;
  stats.stun -= boxes;
  UpdateDamage();
  return boxes;
  }

int Object::HealPhys(int boxes) {
  if(stats.GetAttribute(1) <= 0) return 0;
  if(boxes > stats.phys) boxes = stats.phys;
  stats.phys -= boxes;
  UpdateDamage();
  return boxes;
  }

int Object::HealStru(int boxes) {
  if(boxes > stats.stru) boxes = stats.stru;
  stats.stru -= boxes;
  UpdateDamage();
  return boxes;
  }

int Object::HitMent(int force, int sev, int succ) {
  if(stats.GetAttribute(1) <= 0) return 0;
  succ -= roll(stats.GetAttribute(0), force);
  sev *= 2;  sev += succ;
  for(int ctr=0; ctr<=(sev/2) && ctr<=4; ++ctr) stats.stun += ctr;
  if(sev > 8) stats.stun += (sev-8);
  if(stats.stun > 10) stats.stun = 10;  // No Overflow to Phys from "Ment"
  UpdateDamage();
  return sev;
  }

int Object::HitStun(int force, int sev, int succ) {
  if(stats.GetAttribute(1) <= 0) return 0;
  succ -= roll(stats.GetAttribute(0), force);
  sev *= 2;  sev += succ;
  for(int ctr=0; ctr<=(sev/2) && ctr<=4; ++ctr) stats.stun += ctr;
  if(sev > 8) stats.stun += (sev-8);
  UpdateDamage();
  return sev;
  }

int Object::HitPhys(int force, int sev, int succ) {
  if(stats.GetAttribute(1) <= 0) return 0;
  succ -= roll(stats.GetAttribute(0), force);
  sev *= 2;  sev += succ;
  for(int ctr=0; ctr<=(sev/2) && ctr<=4; ++ctr) stats.phys += ctr;
  if(sev > 8) stats.phys += (sev-8);
  UpdateDamage();
  return sev;
  }

int Object::HitStru(int force, int sev, int succ) {
  succ -= roll(stats.GetAttribute(0), force);
  sev *= 2;  sev += succ;
  for(int ctr=0; ctr<=(sev/2) && ctr<=4; ++ctr) stats.stru += ctr;
  if(sev > 8) stats.stru += (sev-8);
  UpdateDamage();
  return sev;
  }

void Object::Send(const char *mes, ...) {
  static char buf[65536];

  if(mes[0] == 0) return;

  memset(buf, 0, 65536);
  va_list stuff;  
  va_start(stuff, mes);
  vsprintf(buf, mes, stuff);
  va_end(stuff);

  char *tosend = strdup(buf);
  tosend[0] = toupper(tosend[0]);

  set<Mind*>::iterator mind;
  for(mind = minds.begin(); mind != minds.end(); ++mind) {
    (*mind)->Send(tosend);
    }
  free(tosend);
  }

void Object::SendAll(const set<Object*> &excl, const char *mes, ...) {
  static char buf[65536];

  if(mes[0] == 0) return;

  memset(buf, 0, 65536);
  va_list stuff;  
  va_start(stuff, mes);
  vsprintf(buf, mes, stuff);
  va_end(stuff);

  char *tosend = strdup(buf);

  if(excl.count(this) == 0) Send(tosend);
  set<Object*>::iterator ind;
  for(ind = contents.begin(); ind != contents.end(); ++ind) {
    (*ind)->SendAll(excl, tosend);
    }
  free(tosend);
  }

void Object::SendIn(const char *mes, const char *youmes,
	Object *actor, Object *targ, ...) {
  static char buf[65536];
  static char youbuf[65536];

  char *tstr = "";  if(targ) tstr = (char*)targ->ShortDesc();
  char *astr = "";  if(actor) astr = (char*)actor->ShortDesc();

  memset(buf, 0, 65536);
  memset(youbuf, 0, 65536);
  va_list stuff;  
  va_start(stuff, targ);
  vsprintf(buf, mes, stuff);
  va_end(stuff);
  va_start(stuff, targ);
  vsprintf(youbuf, youmes, stuff);
  va_end(stuff);

  char *str = strdup(buf);
  char *youstr = strdup(youbuf);

  for(char *ctr=buf; *ctr; ++ctr) if((*ctr) == ';') (*ctr) = '%';
  for(char *ctr=youbuf; *ctr; ++ctr) if((*ctr) == ';') (*ctr) = '%';

  if(youmes && this == actor && this == targ) Send(youbuf, "yourself");
  else if(youmes && this == actor) Send(youbuf, tstr);
  else if(this == targ) Send(buf, astr, "you");
  else Send(buf, astr, tstr);

  set<Object*>::const_iterator ind;
  for(ind = contents.begin(); ind != contents.end(); ++ind) {
    if((*ind)->Stats()->GetSkill("Transparent"))
      (*ind)->SendIn(str, youstr, actor, targ);
    else if((*ind)->Pos() != POS_NONE)	//FIXME - Understand Transparency
      (*ind)->SendIn(str, youstr, actor, targ);
    }

  free(str);
  free(youstr);
  }

void Object::SendOut(const char *mes, const char *youmes,
	Object *actor, Object *targ, ...) {
  static char buf[65536];
  static char youbuf[65536];

  char *tstr = "";  if(targ) tstr = (char*)targ->ShortDesc();
  char *astr = "";  if(actor) astr = (char*)actor->ShortDesc();

  memset(buf, 0, 65536);
  memset(youbuf, 0, 65536);
  va_list stuff;  
  va_start(stuff, targ);
  vsprintf(buf, mes, stuff);
  va_end(stuff);
  va_start(stuff, targ);
  vsprintf(youbuf, youmes, stuff);
  va_end(stuff);

  char *str = strdup(buf);
  char *youstr = strdup(youbuf);

  for(char *ctr=buf; *ctr; ++ctr) if((*ctr) == ';') (*ctr) = '%';
  for(char *ctr=youbuf; *ctr; ++ctr) if((*ctr) == ';') (*ctr) = '%';

  if(youmes && this == actor && this == targ) Send(youbuf, "yourself");
  else if(youmes && this == actor) Send(youbuf, tstr);
  else if(this == targ) Send(buf, astr, "you");
  else Send(buf, astr, tstr);

  set<Object*>::const_iterator ind;
  for(ind = contents.begin(); ind != contents.end(); ++ind) {
    if((*ind)->Stats()->GetSkill("Transparent"))
      (*ind)->SendIn(str, youstr, actor, targ);
    else if((*ind)->Pos() != POS_NONE)	//FIXME - Understand Transparency
      (*ind)->SendIn(str, youstr, actor, targ);
    }

  if(parent && Stats()->GetSkill("Transparent")) {
    parent->contents.erase(this);
    parent->SendOut(str, youstr, actor, targ);
    parent->contents.insert(this);
    }

  free(str);
  free(youstr);
  }

void init_world() {
  universe = new Object;
  universe->SetShortDesc("The Universe");
  universe->SetDesc("An Infinite Universe within which to play.");
  universe->SetLongDesc("An Really Big Infinite Universe within which to play.");
  default_initial = universe;

  if(!universe->Load("acid/current.wld")) {
    load_players("acid/current.plr");
    return;
    }

  int fd = open("startup.conf", O_RDONLY);
  if(fd >= 0) {
    Object *autoninja = new Object(universe);
    autoninja->SetShortDesc("The AutoNinja");
    autoninja->SetDesc("The AutoNinja - you should NEVER see this!");
    autoninja->SetPos(POS_STAND);

    Player *anp = new Player("AutoNinja", "AutoNinja");
    anp->Set(PLAYER_SUPERNINJA);
    anp->Set(PLAYER_NINJA);
    anp->Set(PLAYER_NINJAMODE);

    Mind *mind = new Mind();
    mind->SetPlayer("AutoNinja");
    mind->SetSystem();
    mind->Attach(autoninja);

    int len = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    char *buf = new char[len+1];
    read(fd, buf, len);
    buf[len] = 0;


    handle_command(mind, buf);
//    handle_command(mind, "quit");
//    handle_command(mind, "quit");
    close(fd);

    delete mind; //DON'T NEED TO!!! DONE BY "quit" ABOVE!!!!!
    delete anp;
    delete autoninja;

    delete buf;
    }


  }

void save_world(int with_net) {
  string fn = "acid/current";
  string wfn = fn + ".wld";
  if(!universe->Save(wfn.c_str())) {
    string pfn = fn + ".plr";
    if(!save_players(pfn.c_str())) {
      string pfn = fn + ".nst";
      if((!with_net) || (!save_net(pfn.c_str()))) return;
      fprintf(stderr, "Unable to save network status!\n");
      }
    fprintf(stderr, "Unable to save players!\n");
    }
  fprintf(stderr, "Unable to save world!\n");
  perror("save_world");
  }

int Object::WriteContentsTo(FILE *fl) {
  set<Object*>::iterator cind;
  for(cind = contents.begin(); cind != contents.end(); ++cind) {
    fprintf(fl, ":%d", getnum(*cind));
    }
  return 0;
  }

void Object::SetStats(const stats_t &st) {
  stats = st;
  UpdateDamage();
  }

void Object::BusyFor(long msec, const char *default_next) {
//  fprintf(stderr, "Holding %p, will default do '%s'!\n", this, default_next);
  busytill = current_time;
  busytill.tv_sec += msec/1000;
  busytill.tv_usec += (msec%1000)*1000;
  if(busytill.tv_usec >= 1000000) {
    ++busytill.tv_sec;
    busytill.tv_usec -= 1000000;
    }
  defact = default_next;
  busylist.insert(this);
  }

void Object::BusyWith(Object *other, const char *default_next) {
//  fprintf(stderr, "Holding %p, will default do '%s'!\n", this, default_next);
  busytill = other->busytill;
  defact = default_next;
  busylist.insert(this);
  }

int Object::StillBusy() {
  int ret = 1;
  if(current_time.tv_sec > busytill.tv_sec) ret = 0;
  else if(current_time.tv_sec < busytill.tv_sec) ret = 1;
  else if(current_time.tv_usec >= busytill.tv_usec) ret = 0;
  return ret;
  }

void Object::DoWhenFree(const char *act) {
//  fprintf(stderr, "Adding busyact for %p of '%s'!\n", this, act);
  dowhenfree += ";";
  dowhenfree += act;
  busylist.insert(this);
  }

void Object::BusyAct() {
//  fprintf(stderr, "Taking busyact %p!\n", this);
  busylist.erase(this);

  busytill = current_time;  //FIXME: THIS IS A HACK TO GET MULTI-ACTION ROUNDS

  string comm = dowhenfree;
  string def = defact;
  dowhenfree = "";
  defact = "";

//  fprintf(stderr, "Act is %s!\n", comm.c_str());
  if(minds.size()) {
    handle_command(*(minds.begin()), comm.c_str());
    if(!StillBusy()) handle_command(*(minds.begin()), def.c_str());
    }
  else {
    handle_command(this, comm.c_str());
    if(!StillBusy()) handle_command(this, def.c_str());
    }
  }

void FreeActions() {
  int maxinit = 0;
  map<Object *,int> initlist;
  for(set<Object *>::iterator busy = busylist.begin();
		busy != busylist.end(); ++busy) {
    if(!(*busy)->StillBusy()) {
      initlist[*busy] = (*busy)->Stats()->RollInitiative();
//      fprintf(stderr, "Initiative = %d\n", initlist[*busy]);
      maxinit = maxinit >? initlist[*busy];
      }
    }
  for(int phase = maxinit; phase > 0; --phase) {
    for(map<Object *,int>::iterator init = initlist.begin();
		init != initlist.end(); ++init) {
				// Make sure it's still in busylist
				// (hasn't been deleted by another's BusyAct)!
      if(init->second == phase && busylist.count(init->first)) {
//	fprintf(stderr, "Going at %d\n", phase);
	(init->first)->BusyAct();
	init->second -= 10;
	}
      }
    }
  }
