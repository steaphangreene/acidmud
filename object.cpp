#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <unistd.h>
#include <cstring>
#include <cctype>
#include <fcntl.h>
#include <algorithm>

using namespace std;

#include "commands.h"
#include "object.h"
#include "color.h"
#include "mind.h"
#include "net.h"

const char *pos_str[POS_MAX] = {
        "is here",
        "is lying here",
        "is sitting here",
        "is standing here",
        };

const char *act_str[ACT_SPECIAL_MAX] = {
        "doing nothing",
        "dead",
        "bleeding and dying",
        "unconscious",
        "fast asleep",
        "resting",
        "pointing %2$s%3$sat %1$s",
        "following %1$s",
        "fighting %1$s",
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
        "ACT_MAX",
        "ACT_SPECIAL_MONITOR",
        "ACT_SPECIAL_OLDMONITOR",
        "ACT_SPECIAL_PREPARE",
        "ACT_SPECIAL_NOTSHOWN",
        "ACT_SPECIAL_MASTER",
        "ACT_SPECIAL_LINKED"
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
  if(len == 0) return 0;
  if(!strcasecmp(seek, "all")) return 1;

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
  else if((!strcasecmp(seek, "armor")) && matches(name, "armour")) ret=1;
  else if((!strcasecmp(seek, "armour")) && matches(name, "armor")) ret=1;
  else if((!strcasecmp(seek, "armorer")) && matches(name, "armourer")) ret=1;
  else if((!strcasecmp(seek, "armourer")) && matches(name, "armorer")) ret=1;
  else if((!strcasecmp(seek, "grey")) && matches(name, "gray")) ret=1;
  else if((!strcasecmp(seek, "gray")) && matches(name, "grey")) ret=1;

  dont_recur = 0;
  return ret;
  }

int Object::Matches(const char *seek) {
  if(!strcasecmp(seek, "everyone")) return (Attribute(1) > 0);
  return matches(ShortDesc(), seek);
  }

Object *get_start_room() {
  return default_initial;
  }

Object *new_body() {
  Object *body = new Object();
  body->SetAttribute(0, 3);
  body->SetAttribute(1, 3);
  body->SetAttribute(2, 3);
  body->SetAttribute(3, 3);
  body->SetAttribute(4, 3);
  body->SetAttribute(5, 3);

  body->SetShortDesc("an amorphous blob");
  body->SetParent(default_initial);

  body->SetWeight(body->Attribute(0) * 20000);
  body->SetSize(1000 + body->Attribute(0) * 200);
  body->SetVolume(100);
  body->SetValue(-1);
  body->SetWeight(80000);
  body->SetGender('M');

  body->SetSkill("Attributes", 12);
  body->SetSkill("Skills", 40);
  body->SetSkill("Resources", 90000);

  body->SetPos(POS_STAND);

  body->Activate();

  return body;
  }

#define TICKSPLIT 1500 //15 seconds
static set<Object*> ticklist[TICKSPLIT];
void add_tick(Object *o) {
  static int tickstage = 0;
  ticklist[tickstage].insert(o);
  ++tickstage;
  if(tickstage >= TICKSPLIT) tickstage = 0;
  }

void remove_tick(Object *o) {
  for(int ctr=0; ctr<TICKSPLIT; ++ctr) ticklist[ctr].erase(o);
  }

int has_tick(const Object *o) {
  int ret=0;
  for(int ctr=0; (!ret) && ctr<TICKSPLIT; ++ctr)
    ret += ticklist[ctr].count((Object *)(o));	//Why do I need to typecast?
  return (ret != 0);
  }

void tick_world() {
  static int tickstage = 0;
  set<Object*>::iterator ind = ticklist[tickstage].begin();
//  fprintf(stderr, "Ticking %d items\n", ticklist[tickstage].size());
  for(; ind != ticklist[tickstage].end(); ++ind) {
    (*ind)->Tick();
    }
  ++tickstage;
  if(tickstage >= TICKSPLIT) tickstage = 0;
  }

void Object::Tick() {
  set<Mind*>::iterator m;
  for(m = minds.begin(); m != minds.end(); ++m) {
    (*m)->Attach(this);
    (*m)->Think(1);
    }

  if(phys > (10+Attribute(0))) {
    // You are already dead.
    }
  else if(phys >= 10) {
    int rec = RollNoWounds("Body", phys - 4);
    if(!rec) ++phys;
    UpdateDamage();
    }
  else if(phys > 0) {
    int rec = 0;
    if(IsAct(ACT_SLEEP)) rec = Roll("Body", 2);
    else if(IsAct(ACT_REST)) rec = Roll("Body", 4);
    else if(!IsAct(ACT_FIGHT)) rec = Roll("Body", 6);
    if(phys >= 6 && (!rec)) ++phys;
    else phys -= rec/2;
    phys = 0 >? phys;
    UpdateDamage();
    }
  if(phys < 10 && stun >= 10) {
    int rec = 0;
    rec = RollNoWounds("Willpower", 12);
    stun -= rec;  stun = 0 >? stun;
    UpdateDamage();
    }
  else if(phys < 6 && stun > 0) {
    int rec = 0;
    if(IsAct(ACT_SLEEP)) rec = Roll("Willpower", 2);
    else if(IsAct(ACT_REST)) rec = Roll("Willpower", 4);
    else if(!IsAct(ACT_FIGHT)) rec = Roll("Willpower", 6);
    stun -= rec;  stun = 0 >? stun;
    UpdateDamage();
    }

  if(parent && Skill("CirclePopper") == 1 && ActTarg(ACT_SPECIAL_PREPARE)) {
    ActTarg(ACT_SPECIAL_PREPARE)->AddAct(ACT_SPECIAL_MASTER, this);
    if(!ActTarg(ACT_SPECIAL_MONITOR)) {
      Object *obj = new Object(*(ActTarg(ACT_SPECIAL_PREPARE)));
      obj->SetParent(this);
      obj->AddAct(ACT_SPECIAL_MASTER, this);
      obj->Travel(parent);
      AddAct(ACT_SPECIAL_MONITOR, obj);
      obj->Attach(get_circle_mob_mind());
      obj->Activate();
      parent->SendOut(ALL, -1, ";s arrives.\n", "", obj, NULL);
      }
    else if(ActTarg(ACT_SPECIAL_MONITOR)->IsAct(ACT_DEAD)) {
      if(ActTarg(ACT_SPECIAL_OLDMONITOR))
	delete ActTarg(ACT_SPECIAL_OLDMONITOR);
      AddAct(ACT_SPECIAL_OLDMONITOR, ActTarg(ACT_SPECIAL_MONITOR));
      Object *obj = new Object(*(ActTarg(ACT_SPECIAL_PREPARE)));
      obj->SetParent(this);
      obj->AddAct(ACT_SPECIAL_MASTER, this);
      obj->Travel(parent);
      AddAct(ACT_SPECIAL_MONITOR, obj);
      obj->Attach(get_circle_mob_mind());
      obj->Activate();
      parent->SendOut(ALL, -1, ";s arrives.\n", "", obj, NULL);
      }
    }
  if(Skill("CirclePopper") > 0) {	// Only pop-check once per minute!
    SetSkill("CirclePopper", (Skill("CirclePopper")%20)+1);
    }

  //FIXME: rot, degrade, etc....
  }

Object::Object() {
  busytill.tv_sec = 0;
  busytill.tv_usec = 0;
  short_desc = "new object";
  parent = NULL;
  pos = POS_NONE;

  weight = 0;
  volume = 0;
  size = 0;
  value = 0;
  gender = 'N';

  exp = 0;
  sexp = 0;

  stun = 0;
  phys = 0;
  stru = 0;
  att[0] = 0;
  att[1] = 0;
  att[2] = 0;
  att[3] = 0;
  att[4] = 0;
  att[5] = 0;
  att[6] = 0;
  att[7] = 0;

  no_seek = 0;
  }

Object::Object(Object *o) {
  busytill.tv_sec = 0;
  busytill.tv_usec = 0;
  short_desc = "new object";
  parent = NULL;
  SetParent(o);
  pos = POS_NONE;

  weight = 0;
  volume = 0;
  size = 0;
  value = 0;
  gender = 'N';

  exp = 0;
  sexp = 0;

  stun = 0;
  phys = 0;
  stru = 0;
  att[0] = 0;
  att[1] = 0;
  att[2] = 0;
  att[3] = 0;
  att[4] = 0;
  att[5] = 0;
  att[6] = 0;
  att[7] = 0;

  no_seek = 0;
  }

Object::Object(const Object &o) {
  busytill.tv_sec = 0;
  busytill.tv_usec = 0;
  short_desc = o.short_desc;
  desc = o.desc;
  long_desc = o.long_desc;
  act.clear();
  pos = o.pos;

  weight = o.weight;
  volume = o.volume;
  size = o.size;
  value = o.value;
  gender = o.gender;

  exp = o.exp;
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
  att[6] = o.att[6];
  att[7] = o.att[7];

  skills = o.skills;

  contents.clear();
  typeof(o.contents.begin()) ind;
  for(ind = o.contents.begin(); ind != o.contents.end(); ++ind) {
    Object *nobj = new Object(*(*ind));
    nobj->SetParent(this);
    if(o.ActTarg(ACT_HOLD) == (*ind)) AddAct(ACT_HOLD, nobj);
    if(o.ActTarg(ACT_WIELD) == (*ind)) AddAct(ACT_WIELD, nobj);
    for(act_t act=ACT_WEAR_BACK; act < ACT_MAX; ++((int&)(act)))
      if(o.ActTarg(act) == (*ind)) AddAct(act, nobj);
    }
  if(o.IsAct(ACT_DEAD)) AddAct(ACT_DEAD);
  if(o.IsAct(ACT_DYING)) AddAct(ACT_DYING);
  if(o.IsAct(ACT_UNCONSCIOUS)) AddAct(ACT_UNCONSCIOUS);
  if(o.IsAct(ACT_SLEEP)) AddAct(ACT_SLEEP);
  if(o.IsAct(ACT_REST)) AddAct(ACT_REST);

  parent = NULL;

  no_seek = 0;

  if(o.IsActive()) Activate();
  }

// Generate truly-formatted name
const char *Object::Name(int definite, Object *rel, Object *sub) {
  static string local;
  int need_an = 0;
  int proper = 0;
  string ret;

  if(rel == this && sub == this) return "yourself";
  else if(rel == this) return "you";

  //FIXME: Hack!  Really detect/specify reflexives?
  else if(rel == NULL && sub == this && sub->Gender() == 'F') return "her";
  else if(rel == NULL && sub == this && sub->Gender() == 'M') return "him";
  else if(rel == NULL && sub == this) return "it";


  else if(sub == this && sub->Gender() == 'F') return "herself";
  else if(sub == this && sub->Gender() == 'M') return "himself";
  else if(sub == this) return "itself";

  if(!strncasecmp(short_desc.c_str(), "a ", 2)) {
    ret = (short_desc.c_str()+2);
    need_an = 0;
    }
  else if(!strncasecmp(short_desc.c_str(), "an ", 3)) {
    ret = (short_desc.c_str()+3);
    need_an = 1;
    }
  else if(!strncasecmp(short_desc.c_str(), "the ", 4)) {
    ret = (short_desc.c_str()+4);
    definite = 1;
    }
  else {
    ret = short_desc.c_str();
    proper = 1;
    }

  if(!Attribute(1)) {
    Object *pos = parent;
    while(pos && (!pos->Attribute(1))) pos = pos->Parent();
    if(pos && pos == rel) {
      ret = string("your ") + ret;
      }
    else if(pos && pos == sub && pos->Gender() == 'F') {
      ret = string("her ") + ret;
      }
    else if(pos && pos == sub && pos->Gender() == 'M') {
      ret = string("his ") + ret;
      }
    else if(pos && pos == sub) {
      ret = string("its ") + ret;
      }
    else if(pos) {
      ret = string(pos->Name()) + "'s " + ret;
      }
    else if(definite && (!proper)) {
      ret = string("the ") + ret;
      }
    else if((!proper) && need_an){
      ret = string("an ") + ret;
      }
    else if(!proper) {
      ret = string("a ") + ret;
      }
    }
  else if(definite && (!proper)) {
    ret = string("the ") + ret;
    }
  else if((!proper) && need_an){
    ret = string("an ") + ret;
    }
  else if(!proper) {
    ret = string("a ") + ret;
    }

  local = ret;
  return local.c_str();
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
  if(s.length() < 1) return;
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

void Object::SetParent(Object *o) {
  parent = o;
  if(o) o->AddLink(this);
  }

void Object::SendContents(Object *targ, Object *o, int seeinside, string b) {
  set<Mind*>::iterator m = targ->minds.begin();
  for(; m != targ->minds.end(); ++m) {
    SendContents(*m, o, seeinside, b);
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

void Object::SendDescSurround(Object *targ, Object *o) {
  set<Mind*>::iterator m = targ->minds.begin();
  for(; m != targ->minds.end(); ++m) {
    SendDescSurround(*m, o);
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
      else targ = (char*) cur->second->Name(0, m->Body(), this);

//      //FIXME: Busted!  This should be the "pointing north to bob" thingy.
//      map<string,Object*>::iterator dir = connections.begin();
//      for(; dir != connections.end(); ++dir) {
//	if((*dir).second == cur->second) {
//	  dirn = (char*) (*dir).first.c_str();
//	  dirp = " ";
//	  break;
//	  }
//	}
      m->Send(", ");
      m->Send(act_str[cur->first], targ, dirn, dirp);
      }
    }
  m->Send(".\n");
  }

void Object::SendExtendedActions(Mind *m, int seeinside) {
  map<act_t,Object*>::iterator cur;
  for(cur = act.begin(); cur != act.end(); ++cur) {
    if(cur->first == ACT_HOLD) m->Send("%24s", "Held: ");
    else if(cur->first == ACT_WIELD) m->Send("%24s", "Wielded: ");
    else if(cur->first == ACT_WEAR_BACK) m->Send("%24s", "Worn on back: ");
    else if(cur->first == ACT_WEAR_CHEST) m->Send("%24s", "Worn on chest: ");
    else if(cur->first == ACT_WEAR_HEAD) m->Send("%24s", "Worn on head: ");
    else if(cur->first == ACT_WEAR_NECK) m->Send("%24s", "Worn on neck: ");
    else if(cur->first == ACT_WEAR_WAIST) m->Send("%24s", "Worn on waist: ");
    else if(cur->first == ACT_WEAR_SHIELD) m->Send("%24s", "Worn as shield: ");
    else if(cur->first == ACT_WEAR_LARM) m->Send("%24s", "Worn on left arm: ");
    else if(cur->first == ACT_WEAR_RARM) m->Send("%24s", "Worn on right arm: ");
    else if(cur->first == ACT_WEAR_LFINGER) m->Send("%24s", "Worn on left finger: ");
    else if(cur->first == ACT_WEAR_RFINGER) m->Send("%24s", "Worn on right finger: ");
    else if(cur->first == ACT_WEAR_LFOOT) m->Send("%24s", "Worn on left foot: ");
    else if(cur->first == ACT_WEAR_RFOOT) m->Send("%24s", "Worn on right foot: ");
    else if(cur->first == ACT_WEAR_LHAND) m->Send("%24s", "Worn on left hand: ");
    else if(cur->first == ACT_WEAR_RHAND) m->Send("%24s", "Worn on right hand: ");
    else if(cur->first == ACT_WEAR_LLEG) m->Send("%24s", "Worn on left leg: ");
    else if(cur->first == ACT_WEAR_RLEG) m->Send("%24s", "Worn on right leg: ");
    else if(cur->first == ACT_WEAR_LWRIST) m->Send("%24s", "Worn on left wrist: ");
    else if(cur->first == ACT_WEAR_RWRIST) m->Send("%24s", "Worn on right wrist: ");
    else if(cur->first == ACT_WEAR_LSHOULDER) m->Send("%24s", "Worn on left shoulder: ");
    else if(cur->first == ACT_WEAR_RSHOULDER) m->Send("%24s", "Worn on right shoulder: ");
    else if(cur->first == ACT_WEAR_LHIP) m->Send("%24s", "Worn on left hip: ");
    else if(cur->first == ACT_WEAR_RHIP) m->Send("%24s", "Worn on right hip: ");
    else continue;

    char *targ;
    if(!cur->second) targ = "";
    else targ = (char*) cur->second->Name(0, m->Body(), this);

    char qty[256] = { 0 };
    if(cur->second->Skill("Quantity") > 1)
      sprintf(qty, "(x%d) ", cur->second->Skill("Quantity"));

    m->Send("%s%s%s.\n%s", CGRN, qty, targ, CNRM);

    if(cur->second->Skill("Open")) {
      sprintf(buf, "%16s  %c", " ", 0);
      base = buf;
      cur->second->SendContents(m, NULL, seeinside);
      base = "";
      m->Send("%s", CNRM);
      }
    else if(cur->second->Skill("Container")) {
      if(seeinside && (!cur->second->Skill("Locked"))) {
	sprintf(buf, "%16s  %c", " ", 0);
	base = buf;
	cur->second->SendContents(m, NULL, seeinside);
	base = "";
	m->Send("%s", CNRM);
	}
      else if(seeinside && cur->second->Skill("Locked")) {
	string mes = base + CNRM + "                "
		+ "  It is closed and locked.\n" + CGRN;
	m->Send(mes.c_str());
	}
      }
    }
  }

void Object::SendContents(Mind *m, Object *o, int seeinside, string b) {
  typeof(contents) cont = contents;

  if(b.length() > 0) base += b;

  set<Object*> master;
  master.insert(cont.begin(), cont.end());

  for(act_t act = ACT_HOLD; act < ACT_MAX; ++((int&)(act))) {
    master.erase(ActTarg(act));  //Don't show worn/wielded stuff.
    }

  int total = 0;
  typeof(cont.begin()) ind;
  for(ind = cont.begin(); ind != cont.end(); ++ind) if(master.count(*ind)) {
    if((*ind)->IsAct(ACT_SPECIAL_NOTSHOWN)) continue;
    if((*ind)->Skill("Hidden") > 0) continue;

    if((*ind)->IsAct(ACT_SPECIAL_LINKED)) {
      if((*ind)->ActTarg(ACT_SPECIAL_LINKED)
		&& (*ind)->ActTarg(ACT_SPECIAL_LINKED)->Parent()) {
	if(base != "") m->Send("%s%sInside: ", base.c_str(), CNRM);
	m->Send("%s", CCYN);
	string send = (*ind)->ShortDesc();
	if(!(*ind)->Skill("Open")) {
	  send += ", the door is closed.\n";
	  }
	else {
	  if((*ind)->Skill("Closeable"))
	    send += ", through an open door,";
	  send += " you see ";
	  send += (*ind)->ActTarg(ACT_SPECIAL_LINKED)->Parent()->ShortDesc();
	  send += ".\n";
	  }
	send[0] = toupper(send[0]);
	m->Send(send.c_str());
	}
      continue;
      }

    m->Send("%s", CGRN);
    master.erase(*ind);

/* Comment out this block to disable 20-item limit in view */
    if(total >= 20) {
      m->Send(base.c_str());
      m->Send("...and %d more things are here too.\n",
	cont.size() - total);
      break;
      }

    ++total;
    if((*ind) != o) {
      if(base != "") m->Send("%s%sInside:%s ", base.c_str(), CNRM, CGRN);

/*	Uncomment this and comment the block below to disable auto-pluralizing.
      int qty = (1 >? (*ind)->Skill("Quantity"));
*/
      int qty = 1;
      if(!(*ind)->Attribute(1)) { // Inanimate objects can have higher qtys
	typeof(cont.begin()) oth = ind;
	for(qty = 0; oth != cont.end(); ++oth) if((*(*oth)) == (*(*ind))) {
	  master.erase(*oth);
	  qty += 1 >? (*oth)->Skill("Quantity");
	  }
	}

      if(qty > 1) m->Send("(x%d) ", qty);

      if((*ind)->parent && (*ind)->parent->Skill("Container"))
	sprintf(buf, "%s%c", (*ind)->ShortDesc(), 0);
      else
	sprintf(buf, "%s %s%c", (*ind)->ShortDesc(), (*ind)->PosString(), 0);
      buf[0] = toupper(buf[0]);
      m->Send(buf);

      (*ind)->SendActions(m);

      if((*ind)->Skill("Open")) {
	string tmp = base;
	base += "  ";
	(*ind)->SendContents(m, o, seeinside);
	base = tmp;
	}
      else if((*ind)->Skill("Container")) {
	if(seeinside && (!(*ind)->Skill("Locked"))) {
	  string tmp = base;
	  base += "  ";
	  (*ind)->SendContents(m, o, seeinside);
	  base = tmp;
	  }
	else if(seeinside && (*ind)->Skill("Locked")) {
	  string mes = base + CNRM
		+ "  It is closed and locked, you can't see inside.\n" + CGRN;
	  m->Send(mes.c_str());
	  }
	}
      }
    m->Send("%s", CNRM);
    }
  if(b.length() > 0) base = "";
  }

void Object::SendShortDesc(Mind *m, Object *o) {
  memset(buf, 0, 65536);
  sprintf(buf, "%s\n", ShortDesc());
  m->Send(buf);
  }

void Object::SendFullSituation(Mind *m, Object *o) {
  string pname = "its";
  if(parent && parent->Gender() == 'M') pname = "his";
  else if(parent && parent->Gender() == 'F') pname = "her";

  if(!parent)
    sprintf(buf, "%s is here%c", Name(), 0);

  else if(parent->ActTarg(ACT_HOLD) == this)
    sprintf(buf, "%s is here in %s off-hand%c", Name(), pname.c_str(), 0);

  else if(parent->ActTarg(ACT_WIELD) == this)
    sprintf(buf, "%s is here in %s hand%c", Name(), pname.c_str(), 0);

  else if(parent->ActTarg(ACT_WEAR_BACK) == this)
    sprintf(buf, "%s is here on %s back%c", Name(), pname.c_str(), 0);

  else if(parent->ActTarg(ACT_WEAR_CHEST) == this)
    sprintf(buf, "%s is here on %s chest%c", Name(), pname.c_str(), 0);

  else if(parent->ActTarg(ACT_WEAR_HEAD) == this)
    sprintf(buf, "%s is here on %s head%c", Name(), pname.c_str(), 0);

  else if(parent->ActTarg(ACT_WEAR_NECK) == this)
    sprintf(buf, "%s is here around %s neck%c", Name(), pname.c_str(), 0);

  else if(parent->ActTarg(ACT_WEAR_WAIST) == this)
    sprintf(buf, "%s is here around %s waist%c", Name(), pname.c_str(), 0);

  else if(parent->ActTarg(ACT_WEAR_SHIELD) == this)
    sprintf(buf, "%s is here on %s shield-arm%c", Name(), pname.c_str(), 0);

  else if(parent->ActTarg(ACT_WEAR_LARM) == this)
    sprintf(buf, "%s is here on %s left arm%c", Name(), pname.c_str(), 0);

  else if(parent->ActTarg(ACT_WEAR_RARM) == this)
    sprintf(buf, "%s is here on %s right arm%c", Name(), pname.c_str(), 0);

  else if(parent->ActTarg(ACT_WEAR_LFINGER) == this)
    sprintf(buf, "%s is here on %s left finger%c", Name(), pname.c_str(), 0);

  else if(parent->ActTarg(ACT_WEAR_RFINGER) == this)
    sprintf(buf, "%s is here on %s right finger%c", Name(), pname.c_str(), 0);

  else if(parent->ActTarg(ACT_WEAR_LFOOT) == this)
    sprintf(buf, "%s is here on %s left foot%c", Name(), pname.c_str(), 0);

  else if(parent->ActTarg(ACT_WEAR_RFOOT) == this)
    sprintf(buf, "%s is here on %s right foot%c", Name(), pname.c_str(), 0);

  else if(parent->ActTarg(ACT_WEAR_LHAND) == this)
    sprintf(buf, "%s is here on %s left hand%c", Name(), pname.c_str(), 0);

  else if(parent->ActTarg(ACT_WEAR_RHAND) == this)
    sprintf(buf, "%s is here on %s right hand%c", Name(), pname.c_str(), 0);

  else if(parent->ActTarg(ACT_WEAR_LLEG) == this)
    sprintf(buf, "%s is here on %s left leg%c", Name(), pname.c_str(), 0);

  else if(parent->ActTarg(ACT_WEAR_RLEG) == this)
    sprintf(buf, "%s is here on %s right leg%c", Name(), pname.c_str(), 0);

  else if(parent->ActTarg(ACT_WEAR_LWRIST) == this)
    sprintf(buf, "%s is here on %s left wrist%c", Name(), pname.c_str(), 0);

  else if(parent->ActTarg(ACT_WEAR_RWRIST) == this)
    sprintf(buf, "%s is here on %s right wrist%c", Name(), pname.c_str(), 0);

  else if(parent->ActTarg(ACT_WEAR_LSHOULDER) == this)
    sprintf(buf, "%s is here on %s left shoulder%c", Name(), pname.c_str(), 0);

  else if(parent->ActTarg(ACT_WEAR_RSHOULDER) == this)
    sprintf(buf, "%s is here on %s right shoulder%c", Name(), pname.c_str(), 0);

  else if(parent->ActTarg(ACT_WEAR_LHIP) == this)
    sprintf(buf, "%s is here on %s left hip%c", Name(), pname.c_str(), 0);

  else if(parent->ActTarg(ACT_WEAR_RHIP) == this)
    sprintf(buf, "%s is here on %s right hip%c", Name(), pname.c_str(), 0);

  else {
    pname = parent->Name();
    sprintf(buf, "%s %s in %s%c", Name(), PosString(), pname.c_str(), 0);
    }

  buf[0] = toupper(buf[0]);
  m->Send(buf);
  }

void Object::SendDesc(Mind *m, Object *o) {
  memset(buf, 0, 65536);

  if(pos != POS_NONE) {
    m->Send("%s", CCYN);
    SendFullSituation(m, o);
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
  m->Send("%s", CNRM);
  }

void Object::SendDescSurround(Mind *m, Object *o) {
  if(no_seek) return;
  memset(buf, 0, 65536);

  if(pos != POS_NONE) {
    m->Send("%s", CCYN);
    SendFullSituation(m, o);
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

  m->Send("%s", CNRM);
  SendExtendedActions(m, 0);

  if((!parent) || Contains(o) || Skill("Open")) {
    SendContents(m, o);
    }

  if(parent && Skill("Open")) {
    m->Send("%s", CCYN);
    m->Send("Outside you see: ");
    no_seek = 1;
    parent->SendDescSurround(m, this);
    no_seek = 0;
    }

  m->Send("%s", CNRM);
  }

void Object::SendLongDesc(Mind *m, Object *o) {
  if(pos != POS_NONE) {
    m->Send("%s", CCYN);
    SendFullSituation(m, o);
    SendActions(m);
    }
  else {
    m->Send("%s", CCYN);
    sprintf(buf, "%s\n%c", ShortDesc(), 0);
    buf[0] = toupper(buf[0]);
    m->Send(buf);
    }

  m->Send("%s   ", CNRM);
  sprintf(buf, "%s\n%c", LongDesc(), 0);
  buf[0] = toupper(buf[0]);
  m->Send(buf);
  m->Send("%s", CNRM);
  }

void Object::SendScore(Mind *m, Object *o) {
  if(!m) return;
  m->Send("\n%s", CNRM);
  m->Send("Bod: %2d", Attribute(0) <? 99);
  m->Send("           L     M        S           D\n");
  m->Send("Qui: %2d", Attribute(1) <? 99);
  m->Send("    Stun: [%c][%c][%c][%c][%c][%c][%c][%c][%c][%c]",
	stun <= 0 ? ' ' : 'X',
	stun <= 1 ? ' ' : 'X',
	stun <= 2 ? ' ' : 'X',
	stun <= 3 ? ' ' : 'X',
	stun <= 4 ? ' ' : 'X',
	stun <= 5 ? ' ' : 'X',
	stun <= 6 ? ' ' : 'X',
	stun <= 7 ? ' ' : 'X',
	stun <= 8 ? ' ' : 'X',
	stun <= 9 ? ' ' : 'X'
	);
  m->Send("\n");
  m->Send("Str: %2d", Attribute(2) <? 99);
  m->Send("    Phys: [%c][%c][%c][%c][%c][%c][%c][%c][%c][%c]",
	phys <= 0 ? ' ' : 'X',
	phys <= 1 ? ' ' : 'X',
	phys <= 2 ? ' ' : 'X',
	phys <= 3 ? ' ' : 'X',
	phys <= 4 ? ' ' : 'X',
	phys <= 5 ? ' ' : 'X',
	phys <= 6 ? ' ' : 'X',
	phys <= 7 ? ' ' : 'X',
	phys <= 8 ? ' ' : 'X',
	phys <= 9 ? ' ' : 'X'
	);
  if(phys > 10) {
    m->Send(" Overflow: %d", phys-10);
    }
  m->Send("\n");
  m->Send("Cha: %2d", Attribute(3) <? 99);
  m->Send("    Stru: [%c][%c][%c][%c][%c][%c][%c][%c][%c][%c]",
	stru <= 0 ? ' ' : 'X',
	stru <= 1 ? ' ' : 'X',
	stru <= 2 ? ' ' : 'X',
	stru <= 3 ? ' ' : 'X',
	stru <= 4 ? ' ' : 'X',
	stru <= 5 ? ' ' : 'X',
	stru <= 6 ? ' ' : 'X',
	stru <= 7 ? ' ' : 'X',
	stru <= 8 ? ' ' : 'X',
	stru <= 9 ? ' ' : 'X'
	);
  m->Send("\n");
  m->Send("Int: %2d", Attribute(4) <? 99);
  m->Send("\n");
  m->Send("Wil: %2d", Attribute(5) <? 99);

  m->Send("    Sex: %c, %d.%.3dkg, %d.%.3dm, %dv, %dY\n\n",
	gender, weight / 1000, weight % 1000,
	size / 1000, size % 1000, volume, value);

  map<string,int> skills = GetSkills();
  map<string,int> sks;
  map<string,int>::iterator skl;
  map<string,int> nsks;
  map<string,int>::iterator nskl;

  if(skills.count("WeaponType") == 0) {
    for(skl = skills.begin(); skl != skills.end(); ++skl) {
      if(is_skill(skl->first)) sks[skl->first] = skl->second;
      else nsks[skl->first] = skl->second;
      }
    skl = sks.begin();
    nskl = nsks.begin();
    while(nskl != nsks.end() || skl != sks.end()) {
      if(skl != sks.end()) {
	m->Send("%25s: %2d ", skl->first.c_str(), (99 <? skl->second));
	++skl;
	}
      else {
	m->Send("%25s     ", " ");
	}

      if(nskl != nsks.end()) {
	m->Send("%30s: %7d ", nskl->first.c_str(), nskl->second);
	++nskl;
	}

      m->Send("\n");
      }
    }

  m->Send("%s", CYEL);
  m->Send("\nEarned Exp: %4d  Player Exp: %4d  Unspent Exp: %4d\n", exp,
	(minds.count(m) && m->Owner()) ? m->Owner()->Exp() : -1,
	(minds.count(m) && m->Owner()) ? Exp(m->Owner()) : 0);

  m->Send("%s", CNRM);
  }

void Object::SendStats(Mind *m, Object *o) {
  if(!m) return;

  m->Send("\n");

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
    m->Send("\n");
    }

  for(act_t act = ACT_MAX; act < ACT_SPECIAL_MAX; ++((int&)(act))) {
    if(ActTarg(act)) m->Send("  %s -> %s\n", act_str[act], ActTarg(act)->Name());
    else if(IsAct(act)) m->Send("  %s\n", act_str[act]);
    }

  if(IsActive()) m->Send("  ACTIVE\n");

  set<Mind*>::iterator mind;
  for(mind = minds.begin(); mind != minds.end(); ++mind) {
    if((*mind)->Owner()) {
      m->Send("->Player Connected: %s (%d exp)\n",
	(*mind)->Owner()->Name(), (*mind)->Owner()->Exp());
      }
    }
  m->Send("%s", CNRM);
  }

void Object::AddLink(Object *ob) {
  typeof(contents.begin()) ind;
  ind = find(contents.begin(), contents.end(), ob);
  if(ind == contents.end()) {
    typeof(contents.begin()) place = contents.end();
    for(ind = contents.begin(); ind != contents.end(); ++ind) {
      if((*ind) == ob) return;				//Already there!
      if((*(*ind)) == (*ob)) { place = ind; ++place; }	//Likes by likes
      }
    contents.insert(place, ob);
    }
  }

void Object::RemoveLink(Object *ob) {
  typeof(contents.begin()) ind;
  ind = find(contents.begin(), contents.end(), ob);
  while(ind != contents.end()) {
    contents.erase(ind);
    ind = find(contents.begin(), contents.end(), ob);
    }
  }

int Object::Travel(Object *dest, int try_combine) {
  if((!parent) || (!dest)) return -1;

  for(Object *chec_recr = dest; chec_recr; chec_recr = chec_recr->parent) {
    if(chec_recr == this) return -1;		// Check for Recursive Loops
    }

  int cap = dest->Skill("Capacity");
  if(cap > 0) {
    cap -= dest->ContainedVolume();
    if(Volume() > cap) return -2;
    }

  int con = dest->Skill("Container");
  if(con > 0) {
    con -= dest->ContainedWeight();
    if(Weight() > con) return -3;
    }

  Object *oldp = parent;
  parent->RemoveLink(this);
  parent = dest;
  parent->AddLink(this);
  oldp->NotifyGone(this, dest);

  if(try_combine) {
    typeof(parent->contents.begin()) ind;
    for(ind = parent->contents.begin(); ind != parent->contents.end(); ++ind) {
      if((*ind) == this) continue;	//Skip self

      // Never combine with an actee.
      map<act_t, Object *>::iterator a = parent->act.begin();
      for(;a != parent->act.end(); ++a) {
	if(a->second == (*ind)) break;
	}
      if(a != parent->act.end()) continue;

      if((*this) == (*(*ind))) {
	//fprintf(stderr, "Combining '%s'\n", Name());
	int q = 1;
	if(Skill("Quantity")) q = Skill("Quantity");
	if((*ind)->Skill("Quantity")) q += (*ind)->Skill("Quantity");
	else q += 1;
	SetSkill("Quantity", q);
	delete(*ind);
	break;
	}
      }
    }

  StopAct(ACT_POINT);
  StopAct(ACT_FOLLOW);

  if(parent->Skill("DynamicInit") > 0) {  //Room is dynamic, but uninitialized
    parent->DynamicInit();
    }

  if(parent->Skill("Secret")) {
    set<Mind*>::iterator m;
    for(m = minds.begin(); m != minds.end(); ++m) {
      if((*m)->Owner()) {
	if((*m)->Owner()->Accomplish(parent->Skill("Secret"))) {
	  (*m)->Send("%sYou gain a player experience point for finding a secret!\n%s",
		CYEL, CNRM);
	  }
	}
      }
    }

  return 0;
  }

Object::~Object() {
  Deactivate();
  if(default_initial == this) default_initial = universe;

  //fprintf(stderr, "Deleting: %s\n", Name(0));

  set<Object*> movers;
  set<Object*> killers;
  typeof(contents.begin()) ind;
  for(ind = contents.begin(); ind != contents.end(); ++ind) {
    if(is_pc(*ind)) movers.insert(*ind);
    else killers.insert(*ind);
    }

  typeof(killers.begin()) indk;
  for(indk = killers.begin(); indk != killers.end(); ++indk) {
    if(find(contents.begin(), contents.end(), *indk) != contents.end()) {
      delete(*indk);
      }
    }
  killers.clear();

  contents.clear();

  typeof(movers.begin()) indm;
  for(indm = movers.begin(); indm != movers.end(); ++indm) {
    (*indm)->StopAll();
    typeof((*indm)->contents.begin()) ind2 = (*indm)->contents.begin();
    for(; ind2 != (*indm)->contents.end(); ++ind2) {
      (*ind2)->SetParent(NULL);
      killers.insert(*ind2);
      }
    (*indm)->contents.clear();
    (*indm)->Travel(default_initial);
    }

  for(indk = killers.begin(); indk != killers.end(); ++indk) {
    if(find(contents.begin(), contents.end(), *indk) != contents.end()) {
      delete(*indk);
      }
    }
  killers.clear();

  player_rooms_erase(this);

  set<Mind*>::iterator mind;
  for(mind = minds.begin(); mind != minds.end(); ++mind) {
    Unattach(*mind);
    }
  minds.clear();

  if(parent) {
    parent->RemoveLink(this);
    parent->NotifyGone(this);
    }

  //Actions over long distances must be notified!
  set<Object*> tonotify;
  if(ActTarg(ACT_SPECIAL_MASTER))
    tonotify.insert(ActTarg(ACT_SPECIAL_MASTER));
  if(ActTarg(ACT_SPECIAL_MONITOR))
    tonotify.insert(ActTarg(ACT_SPECIAL_MONITOR));
  if(ActTarg(ACT_SPECIAL_OLDMONITOR))
    tonotify.insert(ActTarg(ACT_SPECIAL_OLDMONITOR));
  if(ActTarg(ACT_SPECIAL_LINKED))
    tonotify.insert(ActTarg(ACT_SPECIAL_LINKED));

  StopAct(ACT_SPECIAL_MASTER);
  StopAct(ACT_SPECIAL_MONITOR);
  StopAct(ACT_SPECIAL_OLDMONITOR);
  StopAct(ACT_SPECIAL_LINKED);

  set<Object*>::iterator noti;
  for(noti = tonotify.begin(); noti != tonotify.end(); ++noti) {
    int del = 0;
    if((*noti)->ActTarg(ACT_SPECIAL_MASTER) == this) del = 1;
    else if((*noti)->ActTarg(ACT_SPECIAL_LINKED) == this) del = 1;
    (*noti)->NotifyGone(this);
    if(del) delete (*noti);
    }

  busylist.erase(this);

  //fprintf(stderr, "Done deleting: %s\n", Name(0));
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

int Object::ContainedWeight() {
  int ret = 0;
  typeof(contents.begin()) ind;
  for(ind = contents.begin(); ind != contents.end(); ++ind) {
    ret += (*ind)->weight;
    }
  return ret;
  }

int Object::ContainedVolume() {
  int ret = 0;
  typeof(contents.begin()) ind;
  for(ind = contents.begin(); ind != contents.end(); ++ind) {
    ret += (*ind)->volume;
    }
  return ret;
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
    if(!isgraph(*suf)) ret = -atoi(text);
    }
  else if(!strncasecmp(text, "first ",  6)) ret = 1;
  else if(!strncasecmp(text, "second ", 7)) ret = 2;
  else if(!strncasecmp(text, "third ",  6)) ret = 3;
  else if(!strncasecmp(text, "fourth ", 7)) ret = 4;
  else if(!strncasecmp(text, "fifth ",  6)) ret = 5;
  else if(!strncasecmp(text, "sixth ",  6)) ret = 6;
  else if(!strncasecmp(text, "seventh ",8)) ret = 7;
  else if(!strncasecmp(text, "eighth ", 7)) ret = 8;
  else if(!strncasecmp(text, "ninth ",  6)) ret = 9;
  else if(!strncasecmp(text, "tenth ",  6)) ret = 10;
  else if(!strncasecmp(text, "all ",  4)) ret = ALL;
  else if(!strncasecmp(text, "all.",  4)) ret = ALL;
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

Object *Object::PickObject(const char *name, int loc, int *ordinal) {
  typeof(contents) ret = PickObjects(name, loc, ordinal);
  if(ret.size() != 1) {
    return NULL;
    }
  return (*(ret.begin()));
  }

static int tag(Object *obj, list<Object *> &ret, int *ordinal) {
  if(obj->IsAct(ACT_SPECIAL_NOTSHOWN)) return 0;	//Shouldn't be detected.
  if(obj->Skill("Hidden") > 0) return 0;		//Can't be seen/affected

  Object *nobj = NULL;

  int cqty = 1, rqty = 1; //Contains / Requires

  if(obj->Skill("Quantity")) cqty = obj->Skill("Quantity");

  if(*ordinal == -1) (*ordinal) = 1;	// Need one - make it the first one!

  if(*ordinal == 0) return 0;		// They don't want anything.

  else if(*ordinal > 0) {
    if(*ordinal > cqty) {		// Have not gotten to my targ yet.
      *ordinal -= cqty;
      return 0;
      }
    else if(cqty == 1) {		// Just this one.
      *ordinal = 0;
      ret.push_back(obj);
      return 1;
      }
    else {				// One of this set.
      *ordinal = 0;
      nobj = new Object(*obj);
      nobj->SetParent(obj->Parent());
      nobj->SetSkill("Quantity", 0);
      ret.push_back(nobj);
      --cqty;
      obj->SetSkill("Quantity", (cqty == 1) ? 0 : cqty);
      return 1;
      }
    }

  else if(*ordinal == ALL) rqty = cqty + 1;

  else if(*ordinal < -1) rqty = -(*ordinal);

  if(rqty == cqty) {			// Exactly this entire thing.
    ret.push_back(obj);
    *ordinal = 0;
    return 1;
    }
  else if(rqty > cqty) {		// This entire thing and more.
    ret.push_back(obj);
    if(*ordinal != ALL) *ordinal += cqty;
    return 0;
    }
  else {				// Some of this set.
    nobj = new Object(*obj);
    nobj->SetParent(obj->Parent());
    nobj->SetSkill("Quantity", rqty);
    ret.push_back(nobj);

    cqty -= rqty;
    obj->SetSkill("Quantity", (cqty == 1) ? 0 : cqty);
    return 1;
    }
  return 0;
  }

list<Object*> Object::PickObjects(const char *nm, int loc, int *ordinal) {
  typeof(contents) ret;

  char *name = (char*)nm;
  while((!isgraph(*name)) && (*name)) ++name;

  int ordcontainer;
  if(ordinal) strip_ordinal(&name);
  else { ordinal = &ordcontainer; (*ordinal) = strip_ordinal(&name); }
  if(!strcasecmp(name, "all")) (*ordinal) = ALL;
  if(!strcasecmp(name, "everyone")) (*ordinal) = ALL;
  if(!strcasecmp(name, "everything")) (*ordinal) = ALL;
  if(!(*ordinal)) (*ordinal) = 1;

  char *keyword = NULL;
  char *keyword2 = NULL;
  if((keyword = strstr(name, "'s ")) || (keyword2 = strstr(name, "'S "))) {
    if(keyword && keyword2) keyword = keyword <? keyword2;
    else if(!keyword) keyword = keyword2;
    keyword2 = strdup(name);
    keyword2[keyword-name] = 0;

    typeof(contents) masters = PickObjects(keyword2, loc, ordinal);
    if(!masters.size()) { free(keyword2); return ret; }

    typeof(masters.begin()) master;
    for(master = masters.begin(); master != masters.end(); ++master) {
      typeof(contents) add = 
	(*master)->PickObjects(keyword2 + (keyword-name)+3, LOC_INTERNAL);
      ret.insert(ret.end(), add.begin(), add.end());
      }
    free(keyword2);
    return ret;
    }

  int len = strlen(name);
  while(!isgraph(name[len-1])) --len;

  if(loc & LOC_SELF) {
    if((!strcasecmp(name, "self")) || (!strcasecmp(name, "myself"))) {
      if((*ordinal) != 1) return ret;
      ret.push_back(this);
      return ret;
      }
    }

  if(loc & LOC_HERE) {
    if(!strcasecmp(name, "here")) {
      if((*ordinal) == 1 && parent) ret.push_back(parent);
      return ret;
      }
    }

  if(loc & LOC_INTERNAL) {
    if(!strncasecmp(name, "my ", 3)) {
      name += 3;
      return PickObjects(name, loc & (LOC_INTERNAL|LOC_SELF));
      }
    }

  if(loc & LOC_NEARBY) {
    typeof(parent->Contents()) cont = parent->Contents();

    typeof(cont.begin()) ind;
    for(ind = cont.begin(); ind != cont.end(); ++ind) if(!(*ind)->no_seek) {
      if((*ind) == this) continue;  // Must use "self" to pick self!
      if((*ind)->Matches(name)) {
	if(tag(*ind, ret, ordinal)) return ret;
	}
      if((*ind)->Skill("Open")) {
	typeof(contents) add = (*ind)->PickObjects(name, LOC_INTERNAL, ordinal);
	ret.insert(ret.end(), add.begin(), add.end());

	if((*ordinal) == 0) return ret;
	}
      }
    if(parent->Skill("Open")) {
      if(parent->parent) {
	parent->no_seek = 1;

	typeof(contents) add = parent->PickObjects(name, LOC_NEARBY, ordinal);
	ret.insert(ret.end(), add.begin(), add.end());

	parent->no_seek = 0;
	if((*ordinal) == 0) return ret;
	}
      }
    }

  if(loc & LOC_INTERNAL) {
    typeof(Contents()) cont = Contents();

    map<act_t,Object*>::iterator action;
    for(action = act.begin(); action != act.end(); ++action) {
      typeof(cont.begin()) ind = find(cont.begin(), cont.end(), action->second);
      if(ind != cont.end()) {		// IE: Is action->second within cont
	cont.erase(ind);
	if(action->second->Matches(name)) {
	  if(tag(action->second, ret, ordinal)) return ret;
	  }
	if(action->second->Skill("Container")) {
	  typeof(contents) add
		= action->second->PickObjects(name, LOC_INTERNAL, ordinal);
	  ret.insert(ret.end(), add.begin(), add.end());

	  if((*ordinal) == 0) return ret;
	  }
	}
      }

    typeof(cont.begin()) ind;
    for(ind = cont.begin(); ind != cont.end(); ++ind) {
      if((*ind) == this) continue;  // Must use "self" to pick self!
      if((*ind)->Matches(name)) {
	if(tag(*ind, ret, ordinal)) return ret;
	}
      if((*ind)->Skill("Container")) {
	typeof(contents) add = (*ind)->PickObjects(name, LOC_INTERNAL, ordinal);
	ret.insert(ret.end(), add.begin(), add.end());

	if((*ordinal) == 0) return ret;
	}
      }
    }
  return ret;
  }

int Object::IsWithin(Object *obj) {
  if(no_seek) return 0;
  typeof(contents.begin()) ind;
  for(ind = contents.begin(); ind != contents.end(); ++ind) {
    if((*ind) == obj) return 1;
    if((*ind)->Skill("Open")) {
      int ret = (*ind)->IsWithin(obj);
      if(ret) return ret;
      }
    }
  return 0;
  }

int Object::IsNearBy(Object *obj) {
  if(no_seek || (!parent)) return 0;
  typeof(contents.begin()) ind;
  for(ind = parent->contents.begin(); ind != parent->contents.end(); ++ind) {
    if((*ind) == obj) return 1;
    if((*ind) == this) continue;  // Not Nearby Self
    if((*ind)->Skill("Open")) {
      int ret = (*ind)->IsWithin(obj);
      if(ret) return ret;
      }
    }
  if(parent->parent && parent->Skill("Open")) {
    parent->no_seek = 1;
    int ret = parent->IsNearBy(obj);
    parent->no_seek = 0;
    if(ret) return ret;
    }
  return 0;
  }

void Object::NotifyGone(Object *obj, Object *newloc, int up) {
  if(up == 1 && parent && Skill("Open")) { //Climb to top first!
    parent->NotifyGone(obj, newloc, 1);
    return;
    }

  for(act_t act=ACT_NONE; act < ACT_MAX; ++((int&)(act))) {
    if(ActTarg(act) == obj) {
      if(act != ACT_FOLLOW || (!newloc)) { StopAct(act); }
      else if(parent == newloc) { } // Do nothing - didn't leave!
      else {
	parent->SendOut(ALL, -1, ";s follows ;s.\n", "You follow ;s.\n", this, obj);
	Travel(newloc);
	parent->SendOut(ALL, -1, ";s follows ;s.\n", "", this, obj);
	AddAct(ACT_FOLLOW, obj);
	}
      }
    }
  for(act_t act=ACT_MAX; act < ACT_SPECIAL_MAX; ++((int&)(act))) {
    if((!newloc) && ActTarg(act) == obj) {
      StopAct(act);
      for(act_t act2=ACT_MAX; act2 < ACT_SPECIAL_MAX; ++((int&)(act2)))
	if(obj->ActTarg(act2) == this) obj->StopAct(act2);
      }
    }

  map<Object *, int> tonotify;

  typeof(contents.begin()) ind;
  for(ind = contents.begin(); ind != contents.end(); ++ind) {
    if((*ind)->Skill("Open")) {
      tonotify[*ind] = 0;
      }
    else if(up >= 0) {
      tonotify[*ind] = -1;
      }
    }

  map<Object *, int>::iterator noti;
  for(noti = tonotify.begin(); noti != tonotify.end(); ++noti) {
    noti->first->NotifyGone(obj, newloc, noti->second);
    }
  }

Object *Object::ActTarg(act_t a) const {
  if(act.count(a)) return (act.find(a))->second;
  return NULL;
  };

void Object::StopAll() {
  act.clear();
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
  if(parent) {
    if(pos != POS_LIE) {
      parent->SendOut(ALL, -1, ";s collapses!\n", "You collapse!\n", this, NULL);
      pos = POS_LIE;
      }
    if(IsAct(ACT_HOLD) && ActTarg(ACT_HOLD) != ActTarg(ACT_WEAR_SHIELD)) {
      parent->SendOut(ALL, -1, ";s drops %s!\n", "You drop %s!\n",
		this, NULL, ActTarg(ACT_HOLD)->ShortDesc());
      ActTarg(ACT_HOLD)->Travel(parent);
      }
    else if(IsAct(ACT_HOLD)) {
      parent->SendOut(ALL, -1, ";s stops holding %s.\n", "You stop holding %s!\n",
		this, NULL, ActTarg(ACT_HOLD)->ShortDesc());
      StopAct(ACT_HOLD);
      }
    if(IsAct(ACT_WIELD)) {
      parent->SendOut(ALL, -1, ";s drops %s!\n", "You drop %s!\n",
		this, NULL, ActTarg(ACT_WIELD)->ShortDesc());
      ActTarg(ACT_WIELD)->Travel(parent);
      }
    }
  }

void Object::UpdateDamage() {
  if(stun > 10) {
    phys += stun-10;
    stun = 10;
    }
  if(phys > 10+Attribute(0)) {
    phys = 10+Attribute(0)+1;

    if(IsAct(ACT_DEAD) == 0) {
      parent->SendOut(ALL, -1, 
	";s expires from its wounds.\n", "You expire, sorry.\n", this, NULL);
      stun = 10;
      Collapse();
      AddAct(ACT_DEAD);
      set<Mind*> removals;
      set<Mind*>::iterator mind;
      for(mind = minds.begin(); mind != minds.end(); ++mind) {
	if((*mind)->Type() == MIND_REMOTE) removals.insert(*mind);
	}
      for(mind = removals.begin(); mind != removals.end(); ++mind) {
	Unattach(*mind);
	}
      }
    SetPos(POS_LIE);
    }
  else if(phys >= 10) {
    if(IsAct(ACT_DYING)+IsAct(ACT_DEAD) == 0) {
      parent->SendOut(ALL, -1, 
	";s collapses, bleeding and dying!\n",
	"You collapse, bleeding and dying!\n",
	this, NULL);
      stun = 10;
      Collapse();
      AddAct(ACT_DYING);
      }
    else if(IsAct(ACT_DEAD) == 0) {
      parent->SendOut(ALL, -1, 
	";s isn't quite dead yet!\n", "You aren't quite dead yet!\n",
	this, NULL);
      StopAct(ACT_DEAD);
      AddAct(ACT_DYING);
      }
    SetPos(POS_LIE);
    }
  else if(stun >= 10) {
    if(IsAct(ACT_UNCONSCIOUS)+IsAct(ACT_DYING)+IsAct(ACT_DEAD)==0) {
      parent->SendOut(ALL, -1, 
	";s falls unconscious!\n", "You fall unconscious!\n", this, NULL);
      Collapse();
      AddAct(ACT_UNCONSCIOUS);
      }
    else if(IsAct(ACT_DEAD)+IsAct(ACT_DYING) != 0) {
      parent->SendOut(ALL, -1, 
	";s isn't dead, just out cold.\n", "You aren't dead, just unconscious.",
	this, NULL);
      StopAct(ACT_DEAD);
      StopAct(ACT_DYING);
      AddAct(ACT_UNCONSCIOUS);
      }
    SetPos(POS_LIE);
    }
  else if(stun > 0) {
    if(IsAct(ACT_DEAD)+IsAct(ACT_DYING)+IsAct(ACT_UNCONSCIOUS) != 0) {
      parent->SendOut(ALL, -1, 
	";s wakes up, a little groggy.\n", "You wake up, a little groggy.",
	this, NULL);
      StopAct(ACT_DEAD);
      StopAct(ACT_DYING);
      StopAct(ACT_UNCONSCIOUS);
      }
    }
  else {
    if(IsAct(ACT_DEAD)+IsAct(ACT_DYING)+IsAct(ACT_UNCONSCIOUS) != 0) {
      parent->SendOut(ALL, -1, 
	";s wakes up, feeling fine!\n", "You wake up, feeling fine!\n",
	this, NULL);
      StopAct(ACT_DEAD);
      StopAct(ACT_DYING);
      StopAct(ACT_UNCONSCIOUS);
      }
    }
  }

int Object::HealStun(int boxes) {
  if(Attribute(1) <= 0) return 0;
  if(phys >= 10) return 0;
  if(boxes > stun) boxes = stun;
  stun -= boxes;
  UpdateDamage();
  return boxes;
  }

int Object::HealPhys(int boxes) {
  if(Attribute(1) <= 0) return 0;
  if(boxes > phys) boxes = phys;
  phys -= boxes;
  UpdateDamage();
  return boxes;
  }

int Object::HealStru(int boxes) {
  if(boxes > stru) boxes = stru;
  stru -= boxes;
  UpdateDamage();
  return boxes;
  }

int Object::HitMent(int force, int sev, int succ) {
  if(Attribute(1) <= 0) return 0;
  succ -= roll(Attribute(0), force);
  sev *= 2;  sev += succ;
  for(int ctr=0; ctr<=(sev/2) && ctr<=4; ++ctr) stun += ctr;
  if(sev > 8) stun += (sev-8);
  if(stun > 10) stun = 10;  // No Overflow to Phys from "Ment"
  UpdateDamage();
  return sev;
  }

int Object::HitStun(int force, int sev, int succ) {
  if(Attribute(1) <= 0) return 0;
  succ -= roll(Attribute(0), force);
  sev *= 2;  sev += succ;
  for(int ctr=0; ctr<=(sev/2) && ctr<=4; ++ctr) stun += ctr;
  if(sev > 8) stun += (sev-8);
  UpdateDamage();
  return sev;
  }

int Object::HitPhys(int force, int sev, int succ) {
  if(Attribute(1) <= 0) return 0;
  succ -= roll(Attribute(0), force);
  sev *= 2;  sev += succ;
  for(int ctr=0; ctr<=(sev/2) && ctr<=4; ++ctr) phys += ctr;
  if(sev > 8) phys += (sev-8);
  UpdateDamage();
  return sev;
  }

int Object::HitStru(int force, int sev, int succ) {
  succ -= roll(Attribute(0), force);
  sev *= 2;  sev += succ;
  for(int ctr=0; ctr<=(sev/2) && ctr<=4; ++ctr) stru += ctr;
  if(sev > 8) stru += (sev-8);
  UpdateDamage();
  return sev;
  }

void Object::Send(int tnum, int rsucc, const char *mes, ...) {
  static char buf[65536];

  if(mes[0] == 0) return;

  if(tnum != ALL && rsucc >= 0 && Roll("Perception", tnum) <= rsucc) {
    return;
    }

  memset(buf, 0, 65536);
  va_list stuff;  
  va_start(stuff, mes);
  vsprintf(buf, mes, stuff);
  va_end(stuff);

  char *tosend = strdup(buf);
  tosend[0] = toupper(tosend[0]);

  set<Mind*>::iterator mind;
  for(mind = minds.begin(); mind != minds.end(); ++mind) {
    Object *body = (*mind)->Body();
    (*mind)->Attach(this);
    (*mind)->Send(tosend);
    (*mind)->Attach(body);
    }
  free(tosend);
  }

void Object::SendIn(int tnum, int rsucc, const char *mes, const char *youmes,
	Object *actor, Object *targ, ...) {
  if(no_seek) return;

  static char buf[65536];
  static char youbuf[65536];

  string tstr = "";  if(targ) tstr = (char*)targ->Name(0, this, actor);
  string astr = "";  if(actor) astr = (char*)actor->Name(0, this);

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

  if(youmes && youbuf[0] == '*' && this == actor) {
    Send(ALL, -1, CYEL);
    Send(ALL, -1, youbuf+1, tstr.c_str());
    Send(ALL, -1, CNRM);
    }
  else if(buf[0] == '*' && targ == this) {
    Send(ALL, -1, CRED);
    Send(tnum, rsucc, buf+1, astr.c_str(), tstr.c_str());
    Send(ALL, -1, CNRM);
    }
  else if(buf[0] == '*') {
    Send(ALL, -1, CMAG);
    Send(tnum, rsucc, buf+1, astr.c_str(), tstr.c_str());
    Send(ALL, -1, CNRM);
    }
  else if(youmes && this == actor) {
    Send(ALL, -1, youbuf, tstr.c_str());
    }
  else {
    Send(tnum, rsucc, buf, astr.c_str(), tstr.c_str());
    }

  typeof(contents.begin()) ind;
  for(ind = contents.begin(); ind != contents.end(); ++ind) {
    if((*ind)->Skill("Open"))
      (*ind)->SendIn(tnum, rsucc, str, youstr, actor, targ);
    else if((*ind)->Pos() != POS_NONE)	//FIXME - Understand Transparency
      (*ind)->SendIn(tnum, rsucc, str, youstr, actor, targ);
    }

  free(str);
  free(youstr);
  }

void Object::SendOut(int tnum, int rsucc, const char *mes, const char *youmes,
	Object *actor, Object *targ, ...) {
  if(no_seek) return;

  static char buf[65536];
  static char youbuf[65536];

  string tstr = "";  if(targ) tstr = (char*)targ->Name(0, this, actor);
  string astr = "";  if(actor) astr = (char*)actor->Name(0, this);

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

  if(youmes && youbuf[0] == '*' && this == actor) {
    Send(ALL, -1, CGRN);
    Send(ALL, -1, youbuf+1, tstr.c_str());
    Send(ALL, -1, CNRM);
    }
  else if(buf[0] == '*') {
    Send(ALL, -1, CRED);
    Send(tnum, rsucc, buf+1, astr.c_str(), tstr.c_str());
    Send(ALL, -1, CNRM);
    }
  else if(youmes && this == actor) {
    Send(ALL, -1, youbuf, tstr.c_str());
    }
  else {
    Send(tnum, rsucc, buf, astr.c_str(), tstr.c_str());
    }

  typeof(contents.begin()) ind;
  for(ind = contents.begin(); ind != contents.end(); ++ind) {
    if((*ind)->Skill("Open"))
      (*ind)->SendIn(tnum, rsucc, str, youstr, actor, targ);
    else if((*ind)->Pos() != POS_NONE)	//FIXME - Understand Transparency
      (*ind)->SendIn(tnum, rsucc, str, youstr, actor, targ);
    }

  if(parent && Skill("Open")) {
    no_seek = 1;
    parent->SendOut(tnum, rsucc, str, youstr, actor, targ);
    no_seek = 0;
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

    Mind *automind = new Mind();
    automind->SetPlayer("AutoNinja");
    automind->SetSystem();
    automind->Attach(autoninja);

    int len = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    char *buf = new char[len+1];
    read(fd, buf, len);
    buf[len] = 0;


    handle_command(autoninja, buf, automind);
    close(fd);

    delete automind;
    delete anp;
    delete autoninja;

    delete buf;
    }


  }

void save_world(int with_net) {
  string fn = "acid/current";
  string wfn = fn + ".wld.tmp";
  if(!universe->Save(wfn.c_str())) {
    string pfn = fn + ".plr.tmp";
    if(!save_players(pfn.c_str())) {
      string nfn = fn + ".nst";
      if((!with_net) || (!save_net(nfn.c_str()))) {
	string nfn = fn + ".wld";
	unlink(nfn.c_str());
	rename(wfn.c_str(), nfn.c_str());

	nfn = fn + ".plr";
	unlink(nfn.c_str());
	rename(pfn.c_str(), nfn.c_str());
	}
      else {
	fprintf(stderr, "Unable to save network status!\n");
	perror("save_world");
	}
      }
    else {
      fprintf(stderr, "Unable to save players!\n");
      perror("save_world");
      }
    }
  else {
    fprintf(stderr, "Unable to save world!\n");
    perror("save_world");
    }
  }

int Object::WriteContentsTo(FILE *fl) {
  typeof(contents.begin()) cind;
  for(cind = contents.begin(); cind != contents.end(); ++cind) {
    fprintf(fl, ":%d", getnum(*cind));
    }
  return 0;
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
    handle_command(this, comm.c_str(), *(minds.begin()));
    if(!StillBusy()) handle_command(this, def.c_str(), *(minds.begin()));
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
      initlist[*busy] = (*busy)->RollInitiative();
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

int Object::operator != (const Object &in) const {
  return !((*this) == in);
  }

int Object::operator == (const Object &in) const {
  if(short_desc != in.short_desc) return 0;
  if(desc != in.desc) return 0;
  if(long_desc != in.long_desc) return 0;
  if(weight != in.weight) return 0;
  if(volume != in.volume) return 0;
  if(size != in.size) return 0;
  if(value != in.value) return 0;
  if(gender != in.gender) return 0;

  if(att[0] != in.att[0]) return 0;
  if(att[1] != in.att[1]) return 0;
  if(att[2] != in.att[2]) return 0;
  if(att[3] != in.att[3]) return 0;
  if(att[4] != in.att[4]) return 0;
  if(att[5] != in.att[5]) return 0;
  if(att[6] != in.att[6]) return 0;
  if(att[7] != in.att[7]) return 0;

  if(pos != in.pos) return 0;

  if(act.size() != 0 || in.act.size() != 0) return 0;

  if(contents.size() != 0 || in.contents.size() != 0) return 0;

  return 1;
  }

void Object::operator = (const Object &in) {
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
  att[6] = in.att[6];
  att[7] = in.att[7];

  pos = in.pos;

  if(in.IsActive()) Activate();
  else Deactivate();

//  contents = in.contents;	// NOT doing this for deep/shallow reasons.
//  act = in.act;
  }

list<Object *> Object::Contents() {
  return contents;
  }

int Object::Contains(Object *obj) {
  return (find(contents.begin(), contents.end(), obj) != contents.end());
  }

Object *Object::Stash(Object *obj) {
  list<Object*> containers, my_cont;
  my_cont = PickObjects("all", LOC_INTERNAL);
  typeof(my_cont.begin()) ind;
  for(ind = my_cont.begin(); ind != my_cont.end(); ++ind) {
    if((*ind)->Skill("Container") && (
	(!(*ind)->Skill("Locked")) || (*ind)->Skill("Open")
	)) {
      containers.push_back(*ind);
      }
    }

  Object *dest = NULL;
  list<Object*>::iterator con;
  for(con = containers.begin(); con != containers.end(); ++con) {
    if((*con)->Skill("Capacity") - (*con)->ContainedVolume() < obj->Volume())
      continue;
    if((*con)->Skill("Container") - (*con)->ContainedWeight() < obj->Weight())
      continue;
    if(!dest) dest = (*con);  //It CAN go here....
    for(ind = (*con)->contents.begin(); ind != (*con)->contents.end(); ++ind) {
      if((*obj) == (*(*ind))) { dest = (*con); break; }
      }
    }

  if(dest && (obj->Travel(dest))) dest = NULL;	//See if it actually makes it!
  return dest;
  }

int Object::IsActive() const {
  return has_tick(this);
  }

void Object::Activate() {
  add_tick(this);
  }

void Object::Deactivate() {
  remove_tick(this);
  }

void Object::EarnExp(int e) {
  exp += e;
  }

void Object::SpendExp(int e) {
  sexp += e;
  }

int Object::Accomplish(unsigned long acc) {
  if(completed.count(acc)) return 0;
  completed.insert(acc);
  ++exp;
  return 1;
  }

int Object::Exp(Player *p) {
  int ret = (exp-sexp);
  if(p) ret += p->Exp();
  return ret;
  }
