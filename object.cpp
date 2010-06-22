#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <unistd.h>
#include <cstring>
#include <cctype>
#include <fcntl.h>
#include <algorithm>

using namespace std;

#include "utils.h"
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
        "is using a skill",
        };

const char *act_str[ACT_SPECIAL_MAX] = {
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
        "ACT_SPECIAL_MASTER",
        "ACT_SPECIAL_LINKED",
//	"ACT_SPECIAL_MAX"
        };

static Object *universe = NULL;
static Object *trash_bin = NULL;
static Object *default_initial;

static set<Object *> busylist;
extern timeval current_time; // From main.cpp

void set_start_room(Object *o) {
  default_initial = o;
  }

Object *Object::Universe() {
  return universe;
  }

Object *Object::TrashBin() {
  return trash_bin;
  }

int matches(const char *name, const char *seek) {
  int len = strlen(seek);
  if(len == 0) return 0;
  if(!strcasecmp(seek, "all")) return 1;

  const char *desc = name;
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
  else if((!strcasecmp(seek, "scepter")) && matches(name, "sceptre")) ret=1;
  else if((!strcasecmp(seek, "sceptre")) && matches(name, "scepter")) ret=1;
  else if((!strcasecmp(seek, "grey")) && matches(name, "gray")) ret=1;
  else if((!strcasecmp(seek, "gray")) && matches(name, "grey")) ret=1;
  else if((!strcasecmp(seek, "bread")) && matches(name, "waybread")) ret=1;

  dont_recur = 0;
  return ret;
  }

int Object::Matches(const char *seek) {
  string targ = seek;
  while(isspace(targ[targ.length()-1])) targ=targ.substr(0, targ.length()-1);

  //Keywords Only
  if(!strcasecmp(targ.c_str(), "everyone")) return (Attribute(1) > 0);
  if(!strcasecmp(targ.c_str(), "someone")) return (Attribute(1) > 0);
  if(!strcasecmp(targ.c_str(), "anyone")) return (Attribute(1) > 0);
  if(!strcasecmp(targ.c_str(), "everything")) return (Attribute(1) == 0);
  if(!strcasecmp(targ.c_str(), "something")) return (Attribute(1) == 0);
  if(!strcasecmp(targ.c_str(), "anything")) return (Attribute(1) == 0);

  //Keywords which can also be things
  if((!strcasecmp(targ.c_str(), "corpse")) && IsAct(ACT_DEAD)) return 1;
  if((!strcasecmp(targ.c_str(), "money")) && Skill("Money")) return 1;

  return matches(ShortDesc(), targ.c_str());
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

  body->SetSkill("Attributes", 59);
  body->SetSkill("Skills", 60);
  body->SetSkill("Resources", 90000);

  body->SetPos(POS_STAND);

  body->Activate();

  Object *bag = new Object(body);
  bag->SetSkill("Capacity", 40);
  bag->SetSkill("Closeable", 1);
  bag->SetSkill("Container", 20000);
  bag->SetSkill("Open", 1000);
  bag->SetSkill("Wearable on Left Hip", 1);
  bag->SetSkill("Wearable on Right Hip", 2);

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

#define TICKSPLIT 6000 //60 seconds
static set<Object*> ticklist[TICKSPLIT];
int Object::IsActive() const {
  return(tickstep >= 0);
  }

void Object::Activate() {
  if(tickstep >= 0) return;
  static int tickstage = 0;
  ticklist[tickstage].insert(this);
  tickstep = tickstage++;
  if(tickstage >= TICKSPLIT) tickstage = 0;
  }

void Object::Deactivate() {
  if(tickstep < 0) return;
  ticklist[tickstep].erase(this);
  tickstep = -1;
  }

void tick_world() {
  static int tickstage = 0;
  set<Object*> todel, todeact;
  set<Object*>::iterator ind = ticklist[tickstage].begin();
//  fprintf(stderr, "Ticking %d items\n", ticklist[tickstage].size());
  for(; ind != ticklist[tickstage].end(); ++ind) {
    int res = (*ind)->Tick();
    if(res != 0) {
      if(res == 1) { todel.insert(*ind); }
      else if(res == -1) { todeact.insert(*ind); }
      }
    }
  for(ind = todel.begin(); ind != todel.end(); ++ind) {
    todeact.erase(*ind);
    (*ind)->Recycle();
    }
  for(ind = todeact.begin(); ind != todeact.end(); ++ind) {
    (*ind)->Deactivate();
    }
  ++tickstage;
  if(tickstage >= TICKSPLIT) tickstage = 0;
  Mind::Resume(10);	//Tell suspended minds to resume if their time is up
  }

int Object::Tick() {
  set<Mind*>::iterator m;
  for(m = minds.begin(); m != minds.end(); ++m) {
    (*m)->Attach(this);
    (*m)->Think(1);
    }

  if(phys > (10+Attribute(2))) {
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
    phys = MAX(0, phys);
    UpdateDamage();
    }
  if(phys < 10 && stun >= 10) {
    int rec = 0;
    rec = RollNoWounds("Willpower", 12);
    stun -= rec;  stun = MAX(0, stun);
    UpdateDamage();
    }
  else if(phys < 6 && stun > 0) {
    int rec = 0;
    if(IsAct(ACT_SLEEP)) rec = Roll("Willpower", 2);
    else if(IsAct(ACT_REST)) rec = Roll("Willpower", 4);
    else if(!IsAct(ACT_FIGHT)) rec = Roll("Willpower", 6);
    stun -= rec;  stun = MAX(0, stun);
    UpdateDamage();
    }

  if(parent && Skill("TBAPopper") > 0 && contents.size() > 0) {
    if(!ActTarg(ACT_SPECIAL_MONITOR)) {
      Object *obj = new Object(*(contents.front()));
      obj->SetParent(this);
      obj->Travel(parent);
      AddAct(ACT_SPECIAL_MONITOR, obj);
      obj->Attach(get_tba_mob_mind());
      obj->Activate();
      parent->SendOut(ALL, -1, ";s arrives.\n", "", obj, NULL);
      }
    }

  if(parent && Skill("CirclePopper") > 0 && contents.size() > 0) {
    if(!ActTarg(ACT_SPECIAL_MONITOR)) {
      Object *obj = new Object(*(contents.front()));
      obj->SetParent(this);
      obj->Travel(parent);
      AddAct(ACT_SPECIAL_MONITOR, obj);
      obj->Attach(get_circle_mob_mind());
      obj->Activate();
      parent->SendOut(ALL, -1, ";s arrives.\n", "", obj, NULL);
      }
    }

  //Grow Trees (Silently)
  if(HasSkill("Mature Trees") && Skill("Mature Trees") < 100) {
    SetSkill("Mature Trees", Skill("Mature Trees") + 1);
    }

  if(IsAct(ACT_DEAD)) {		//Rotting corpses
    ++stru;
    if(stru == 1) {
      parent->SendOut(ALL, 0,
	";s's corpse starts to smell.\n", "",
	this, NULL
	);
      }
    else if(stru == 3) {
      parent->SendOut(ALL, 0,
	";s's corpse starts to rot.\n", "",
	this, NULL
	);
      }
    else if(stru == 6) {
      parent->SendOut(ALL, 0,
	";s's corpse starts to fall apart.\n", "",
	this, NULL
	);
      }
    else if(stru >= 10) {
      Object *corpse = new Object(parent);

      corpse->SetShortDesc("an unidentifiable corpse");
      corpse->SetDesc("A pile of rotting remains.");
      corpse->SetPos(POS_LIE);

      corpse->SetSkill("Perishable", 1);
      corpse->SetSkill("Rot", 1);
      corpse->Activate();

      corpse->SetWeight(Weight());
      corpse->SetSize(Size());
      corpse->SetVolume(Volume());

      set<Object*> todrop;
      list<Object*> todropfrom;

      typeof(contents.begin()) cur;
      for(cur = contents.begin(); cur != contents.end(); ++cur) {
	Object *item = (*cur);
	if(item->HasSkill("Vortex")) { //Vortex can't be dropped!
	  }
	else if(item->contents.size() > 0 && item->Pos() == POS_NONE) {
	  todropfrom.push_back(item);
	  }
	else {
	  todrop.insert(item);
	  }
	}

      list<Object*>::iterator tdf;
      for(tdf = todropfrom.begin(); tdf != todropfrom.end(); ++tdf) {
	Object *con = (*tdf);
	for(cur = con->contents.begin(); cur != con->contents.end(); ++cur) {
	  Object *item = (*cur);
	  if(item->HasSkill("Vortex")) { //Vortex can't be dropped!
	    }
	  else if(item->contents.size() > 0 && item->Pos() == POS_NONE) {
	    todropfrom.push_back(item);
	    }
	  else {
	    todrop.insert(item);
	    }
	  }
	}

      set<Object*>::iterator td;
      for(td = todrop.begin(); td != todrop.end(); ++td) {
	if(Parent()) Drop(*td, 0, 1);
	else (*td)->Recycle();
	}

      typeof(contents) cont = contents;
      typeof(cont.begin()) todel;
      for(todel = cont.begin(); todel != cont.end(); ++todel) {
	(*todel)->Recycle();
	}

      parent->SendOut(ALL, 0,
	";s's corpse completely falls apart.\n", "",
	this, NULL
	);

      if(is_pc(this)) {	//Hide me in the VOID!
	Travel(default_initial);
	SetSkill("Hidden", 65535);
	return -1;	//Deactivate Me!
	}
      else {
	return 1;	//Delete Me!
	}
      }
    }

  if(HasSkill("Perishable")) {		//Degrading Items
    SetSkill("Rot", Skill("Rot") - 1);
    if(Skill("Rot") < 1) {
      ++stru;
      if(stru < 10) {
	SetSkill("Rot", Skill("Perishable"));
	}
      else {
	return 1;	//Delete Me!
	}
      }
    }

  if(HasSkill("Temporary")) {		//Temporary Items
    SetSkill("Temporary", Skill("Temporary") - 1);
    if(Skill("Temporary") < 1) {
      if(Owner() && Owner()->Parent()) {
	Owner()->Parent()->SendOut(0, 0,
		";s vanishes in a flash of light.", "", this, NULL
		);
	}
      return 1;	//Delete Me!
      }
    }

  if(Attribute(2) > 0				//Needs Food & Water
	&& (!HasSkill("TBAAction"))		//MOBs don't
	&& (!HasSkill("CircleAction"))		//MOBs don't
	) {
    int level;

    //Get Hungrier
    level = Skill("Hungry");
    if(level < 1) level = att[2];
    else level += att[2];		//Base Strength Scales Food Req
    if(level > 29999) level = 29999;
    SetSkill("Hungry", level);

    if(level == 500) Send(ALL, -1, "You could use a snack.\n");
    else if(level == 1000) Send(ALL, -1, "You officially have the munchies.\n");
    else if(level == 1500) Send(ALL, -1, "You really could go for a snack.\n");
    else if(level == 2000) Send(ALL, -1, "You are getting hungry.\n");
    else if(level == 2500) Send(ALL, -1, "You are getting very hungry.\n");
    else if(level == 3000) Send(ALL, -1, "You are really quite hungry.\n");
    else if(level == 3500) Send(ALL, -1, "You are really dying for food.\n");
    else if(level == 4000) Send(ALL, -1, "You need to get some food soon!\n");
    else if(level == 4500) Send(ALL, -1, "You are starting to starve!\n");
    else if(level >= 5000) {
      if(level % 10 == 0) {
	Send(ALL, -1, "You are starving!\n");
	}
      UpdateDamage();
      }

    //Get Thurstier
    level = Skill("Thirsty");
    if(level < 1) level = att[0];
    else level += att[0];		//Body Scales Water Req
    if(level > 29999) level = 29999;
    SetSkill("Thirsty", level);

    if(level == 500) Send(ALL, -1, "You could use a drink.\n");
    else if(level == 1000) Send(ALL, -1, "Your mouth is getting dry.\n");
    else if(level == 1500) Send(ALL, -1, "You really could go for a drink.\n");
    else if(level == 2000) Send(ALL, -1, "You are getting thirsty.\n");
    else if(level == 2500) Send(ALL, -1, "You are getting very thirsty.\n");
    else if(level == 3000) Send(ALL, -1, "You are really quite thirsty.\n");
    else if(level == 3500) Send(ALL, -1, "You are really dying for water.\n");
    else if(level == 4000) Send(ALL, -1, "You need to get some water soon!\n");
    else if(level == 4500) Send(ALL, -1, "You are starting to dehydrate!\n");
    else if(level >= 5000) {
      if(level % 10 == 0) {
	Send(ALL, -1, "You are dehydrated!\n");
	}
      UpdateDamage();
      }

    }

  if(HasSkill("Liquid Source")) {		//Refills Itself
    if(contents.size() > 0) {
      int qty = 1;
      if(contents.front()->Skill("Quantity") > 1) {
	qty = contents.front()->Skill("Quantity");
	}
      if(qty < Skill("Capacity")) {
	contents.front()->SetSkill("Quantity", qty + Skill("Liquid Source"));
	if(contents.front()->Skill("Quantity") > Skill("Capacity")) {
	  contents.front()->SetSkill("Quantity", Skill("Capacity"));
	  }
	}
      }
    else {
      fprintf(stderr, "Warning: Fountain completely out of liquid!\n");
      }
    }

  //Lit Torches/Lanterns
  if(HasSkill("Brightness") && HasSkill("Light Source")) {
    SetSkill("Lightable", Skill("Lightable") - 1);
    if(Skill("Lightable") < 1) {
      SetSkill("Light Source", 0);
      return -1;		//Deactivate Me!
      }
    }

  //Skys
  if(Skill("Day Length") > 1) {	//Must be > 1 (divide by it/2 below!)
    SetSkill("Day Time", Skill("Day Time") + 1);
    if(Skill("Day Time") >= Skill("Day Length")) {
      SetSkill("Day Time", 0);
      }
    int light = Skill("Day Time") - (Skill("Day Length") / 2);
    if(light < 0) light = -light;
    light *= 900;
    light /= (Skill("Day Length") / 2);
    SetSkill("Light Source", 1000 - light);
    }

  //Poisoned
  if(Skill("Poisoned") > 0) {
    int succ = Roll("Strength", Skill("Poisoned"));
    SetSkill("Poisoned", Skill("Poisoned") - succ);
    Parent()->SendOut(0, 0,
	";s chokes and writhes in pain.\n",
	CRED "You choke and writhe in pain.  POISON!!!!\n" CNRM,
	this, NULL
	);
    if(succ < 2) phys += 6;
    else if(succ < 4) phys += 3;
    else if(succ < 6) phys += 1;
    UpdateDamage();
    }

  return 0;
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
  tickstep = -1;
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
  tickstep = -1;
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
    for(act_t act=ACT_WEAR_BACK; act < ACT_MAX; act = act_t(int(act)+1))
      if(o.ActTarg(act) == (*ind)) AddAct(act, nobj);
    }
  if(o.IsAct(ACT_DEAD)) AddAct(ACT_DEAD);
  if(o.IsAct(ACT_DYING)) AddAct(ACT_DYING);
  if(o.IsAct(ACT_UNCONSCIOUS)) AddAct(ACT_UNCONSCIOUS);
  if(o.IsAct(ACT_SLEEP)) AddAct(ACT_SLEEP);
  if(o.IsAct(ACT_REST)) AddAct(ACT_REST);

  parent = NULL;

  if(o.IsActive()) Activate();

  minds = o.minds;	// Transmit Attached Minds

  no_seek = 0;
  tickstep = -1;
  }

// Generate truly-formatted name
const char *Object::Name(int definite, Object *rel, Object *sub) const {
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
    Object *pos = Owner();
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

const char *Object::ShortDesc() const {
  return short_desc.c_str();
  }

const char *Object::Desc() const {
  if(desc.length() <= 0) return ShortDesc();
  return desc.c_str();
  }

const char *Object::LongDesc() const {
  if(long_desc.length() <= 0) return Desc();
  return long_desc.c_str();
  }

static void trim(string &s) {
  if(s.length() < 1) return;
  while(s[0] != 0 && (!isgraph(s[0]))) {
    s = s.substr(1);
    }
  while(s[s.length()-1] != 0 && (!isgraph(s[s.length()-1]))) {
    s = s.substr(0, s.length()-1);
    }

  size_t n00b = s.find('@');
  while(n00b != string::npos) {
    //fprintf(stderr, "Step: %s\n", s.c_str());
    if(s[n00b+1] == '@') {		//@@ -> @
      s = s.substr(0, n00b) + "@" + s.substr(n00b+2);
      n00b = s.find('@', n00b + 1);
      }
    else {	//FIXME: Actually use ANSI colors?
      s = s.substr(0, n00b) + s.substr(n00b+2);
      n00b = s.find('@', n00b);
      }
    //if(n00b == string::npos) fprintf(stderr, "Done: %s\n\n", s.c_str());
    }
  }

void Object::SetShortDesc(const char *d) {
  string s = d;
  trim(s);
  short_desc = s;
  }

void Object::SetDesc(const char *d) {
  string s = d;
  trim(s);
  desc = s;
  }

void Object::SetLongDesc(const char *d) {
  string s = d;
  trim(s);
  long_desc = s;
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

void Object::SendDescSurround(Object *targ, Object *o, int seeinside) {
  set<Mind*>::iterator m = targ->minds.begin();
  for(; m != targ->minds.end(); ++m) {
    SendDescSurround(*m, o, seeinside);
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
      const char *targ;
      const char *dirn = "";
      const char *dirp = "";

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
      m->SendF(act_str[cur->first], targ, dirn, dirp);
      }
    }
  if(HasSkill("Light Source")) {
    if(Skill("Light Source") < 20) m->Send(", glowing");
    else if(HasSkill("Brightness")) m->Send(", burning");
    else if(Skill("Light Source") < 200) m->Send(", lighting the area");
    else m->Send(", shining");
    }
  if(HasSkill("Noise Source")) {
    if(Skill("Noise Source") < 20) m->Send(", humming");
    else if(Skill("Noise Source") < 200) m->Send(", buzzing");
    else m->Send(", roaring");
    }
  m->Send(".\n");
  }

void Object::SendExtendedActions(Mind *m, int seeinside) {
  map<Object*,string> shown;
  map<act_t,Object*>::iterator cur;
  for(cur = act.begin(); cur != act.end(); ++cur) {
    if(cur->first == ACT_HOLD) m->SendF("%24s", "Held: ");
    else if(cur->first == ACT_WIELD) m->SendF("%24s", "Wielded: ");
    else if(cur->first == ACT_WEAR_BACK) m->SendF("%24s", "Worn on back: ");
    else if(cur->first == ACT_WEAR_CHEST) m->SendF("%24s", "Worn on chest: ");
    else if(cur->first == ACT_WEAR_HEAD) m->SendF("%24s", "Worn on head: ");
    else if(cur->first == ACT_WEAR_NECK) m->SendF("%24s", "Worn on neck: ");
    else if(cur->first == ACT_WEAR_WAIST) m->SendF("%24s", "Worn on waist: ");
    else if(cur->first == ACT_WEAR_SHIELD) m->SendF("%24s", "Worn as shield: ");
    else if(cur->first == ACT_WEAR_LARM) m->SendF("%24s", "Worn on left arm: ");
    else if(cur->first == ACT_WEAR_RARM) m->SendF("%24s", "Worn on right arm: ");
    else if(cur->first == ACT_WEAR_LFINGER) m->SendF("%24s", "Worn on left finger: ");
    else if(cur->first == ACT_WEAR_RFINGER) m->SendF("%24s", "Worn on right finger: ");
    else if(cur->first == ACT_WEAR_LFOOT) m->SendF("%24s", "Worn on left foot: ");
    else if(cur->first == ACT_WEAR_RFOOT) m->SendF("%24s", "Worn on right foot: ");
    else if(cur->first == ACT_WEAR_LHAND) m->SendF("%24s", "Worn on left hand: ");
    else if(cur->first == ACT_WEAR_RHAND) m->SendF("%24s", "Worn on right hand: ");
    else if(cur->first == ACT_WEAR_LLEG) m->SendF("%24s", "Worn on left leg: ");
    else if(cur->first == ACT_WEAR_RLEG) m->SendF("%24s", "Worn on right leg: ");
    else if(cur->first == ACT_WEAR_LWRIST) m->SendF("%24s", "Worn on left wrist: ");
    else if(cur->first == ACT_WEAR_RWRIST) m->SendF("%24s", "Worn on right wrist: ");
    else if(cur->first == ACT_WEAR_LSHOULDER) m->SendF("%24s", "Worn on left shoulder: ");
    else if(cur->first == ACT_WEAR_RSHOULDER) m->SendF("%24s", "Worn on right shoulder: ");
    else if(cur->first == ACT_WEAR_LHIP) m->SendF("%24s", "Worn on left hip: ");
    else if(cur->first == ACT_WEAR_RHIP) m->SendF("%24s", "Worn on right hip: ");
    else continue;

    const char *targ;
    if(!cur->second) targ = "";
    else targ = (char*) cur->second->Name(0, m->Body(), this);

    char qty[256] = { 0 };
    if(cur->second->Skill("Quantity") > 1)
      sprintf(qty, "(x%d) ", cur->second->Skill("Quantity"));

    if(shown.count(cur->second) > 0) {
      m->SendF("%s%s (%s).\n", qty, targ, shown[cur->second].c_str());
      }
    else {
      m->SendF(CGRN "%s%s.\n" CNRM, qty, targ);
      if(cur->second->Skill("Open") || cur->second->Skill("Transparent")) {
	sprintf(buf, "%16s  %c", " ", 0);
	base = buf;
	cur->second->SendContents(m, NULL, seeinside);
	base = "";
	m->Send(CNRM);
	}
      else if(cur->second->Skill("Container")) {
	if(seeinside == 1 && cur->second->Skill("Locked")) {
	  string mes = base + CNRM + "                "
		+ "  It is closed and locked.\n" + CGRN;
	  m->Send(mes.c_str());
	  }
	else if(seeinside) {
	  sprintf(buf, "%16s  %c", " ", 0);
	  base = buf;
	  cur->second->SendContents(m, NULL, seeinside);
	  base = "";
	  m->Send(CNRM);
	  }
	}
      }
    shown[cur->second] = "already listed";
    }
  }

void Object::SendContents(Mind *m, Object *o, int seeinside, string b) {
  typeof(contents) cont = contents;

  if(b.length() > 0) base += b;

  set<Object*> master;
  master.insert(cont.begin(), cont.end());

  for(act_t act = ACT_HOLD; act < ACT_MAX; act = act_t(int(act)+1)) {
    master.erase(ActTarg(act));  //Don't show worn/wielded stuff.
    }

  int tlines = 0, total = 0;
  typeof(cont.begin()) ind;
  for(ind = cont.begin(); ind != cont.end(); ++ind) if(master.count(*ind)) {
    if((*ind)->HasSkill("Invisible") && seeinside <= 1) continue;
    if((*ind)->Skill("Hidden") > 0 && Parent() != NULL) continue;

    if((*ind)->IsAct(ACT_SPECIAL_LINKED)) {
      if((*ind)->ActTarg(ACT_SPECIAL_LINKED)
		&& (*ind)->ActTarg(ACT_SPECIAL_LINKED)->Parent()) {
	if(base != "") m->SendF("%s%sInside: ", base.c_str(), CNRM);
	m->Send(CCYN);
	string send = (*ind)->ShortDesc();
	if(!((*ind)->Skill("Open") || (*ind)->Skill("Transparent"))) {
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

    m->Send(CGRN);
    master.erase(*ind);

    if((*ind) != o) {
      /* Comment out this block to disable 20-item limit in view */
      if(tlines >= 20) {
	int ignore = 0;
	if(o && o->Parent() == this) ignore = 1;
	m->Send(base.c_str());
	m->SendF("...and %d more things are here too.\n",
		((int)(cont.size())) - total - ignore);
	break;
	}

      if(base != "") m->SendF("%s%sInside:%s ", base.c_str(), CNRM, CGRN);

/*	Uncomment this and comment the block below to disable auto-pluralizing.
      int qty = MAX(1, (*ind)->Skill("Quantity"));
*/
      int qty = 1;	// Even animate objects can have higher quantities.
      typeof(cont.begin()) oth = ind;
      for(qty = 0; oth != cont.end(); ++oth) {
	if((*ind)->LooksLike(*oth)) {
	  master.erase(*oth);
	  qty += MAX(1, (*oth)->Skill("Quantity"));
	  }
	}

      if(qty > 1) m->SendF("(x%d) ", qty);
      total += qty;
      ++tlines;

      if((*ind)->parent && (*ind)->parent->Skill("Container"))
	sprintf(buf, "%s%c", (*ind)->ShortDesc(), 0);
      else
	sprintf(buf, "%s %s%c", (*ind)->ShortDesc(), (*ind)->PosString(), 0);
      buf[0] = toupper(buf[0]);
      m->Send(buf);

      (*ind)->SendActions(m);

      if((*ind)->Skill("Open") || (*ind)->Skill("Transparent")) {
	string tmp = base;
	base += "  ";
	(*ind)->SendContents(m, o, seeinside);
	base = tmp;
	}
      else if((*ind)->Skill("Container")) {
	if(seeinside == 1 && (*ind)->Skill("Locked")) {
	  string mes = base + CNRM
		+ "  It is closed and locked, you can't see inside.\n" + CGRN;
	  m->Send(mes.c_str());
	  }
	else if(seeinside) {
	  string tmp = base;
	  base += "  ";
	  (*ind)->SendContents(m, o, seeinside);
	  base = tmp;
	  }
	}
      }
    m->Send(CNRM);
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

  if(Skill("Quantity") > 1) {
    sprintf(buf, "(x%d) ", Skill("Quantity"));
    m->Send(buf);
    }

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
    m->Send(CCYN);
    SendFullSituation(m, o);
    SendActions(m);
    }
  else {
    m->Send(CCYN);
    sprintf(buf, "%s\n%c", ShortDesc(), 0);
    buf[0] = toupper(buf[0]);
    m->Send(buf);
    }

  m->SendF("%s   ", CNRM);
  sprintf(buf, "%s\n%c", Desc(), 0);
  buf[0] = toupper(buf[0]);
  m->Send(buf);
  m->Send(CNRM);
  }

void Object::SendDescSurround(Mind *m, Object *o, int seeinside) {
  if(no_seek) return;
  memset(buf, 0, 65536);

  if(pos != POS_NONE) {
    m->Send(CCYN);
    SendFullSituation(m, o);
    SendActions(m);
    }
  else {
    m->Send(CCYN);
    sprintf(buf, "%s\n%c", ShortDesc(), 0);
    buf[0] = toupper(buf[0]);
    m->Send(buf);
    }

  m->SendF("%s   ", CNRM);
  sprintf(buf, "%s\n%c", Desc(), 0);
  buf[0] = toupper(buf[0]);
  m->Send(buf);

  m->Send(CNRM);
  SendExtendedActions(m, seeinside);

  if((!parent) || Contains(o) || Skill("Open") || Skill("Transparent")) {
    SendContents(m, o, seeinside);
    }

  if(parent && (Skill("Open") || Skill("Transparent"))) {
    m->Send(CCYN);
    m->Send("Outside you see: ");
    no_seek = 1;
    parent->SendDescSurround(m, this, seeinside);
    no_seek = 0;
    }

  m->Send(CNRM);
  }

void Object::SendLongDesc(Mind *m, Object *o) {
  if(pos != POS_NONE) {
    m->Send(CCYN);
    SendFullSituation(m, o);
    SendActions(m);
    }
  else {
    m->Send(CCYN);
    sprintf(buf, "%s\n%c", ShortDesc(), 0);
    buf[0] = toupper(buf[0]);
    m->Send(buf);
    }

  m->SendF("%s   ", CNRM);
  sprintf(buf, "%s\n%c", LongDesc(), 0);
  buf[0] = toupper(buf[0]);
  m->Send(buf);
  m->Send(CNRM);
  }

const char * const atnames[] = {"Bod", "Qui", "Str", "Cha", "Int", "Wil"};
void Object::SendScore(Mind *m, Object *o) {
  if(!m) return;
  m->SendF("\n%s", CNRM);
  for(int ctr = 0; ctr < 6; ++ctr) {
    if(MIN(att[ctr], 99) == MIN(Attribute(ctr), 99)) {
      m->SendF("%s: %2d     ", atnames[ctr], MIN(Attribute(ctr), 99));
      }
    else if(Attribute(ctr) > 9) {	//2-Digits!
      m->SendF("%s: %2d (%d)", atnames[ctr],
	MIN(att[ctr], 99), MIN(Attribute(ctr), 99));
      }
    else {				//1 Digit!
      m->SendF("%s: %2d (%d) ", atnames[ctr],
	MIN(att[ctr], 99), MIN(Attribute(ctr), 99));
      }
    if(ctr == 0) {
      m->Send("         L     M        S           D");
      }
    else if(ctr == 1) {
      m->SendF("  Stun: [%c][%c][%c][%c][%c][%c][%c][%c][%c][%c]",
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
      }
    else if(ctr == 2) {
      m->SendF("  Phys: [%c][%c][%c][%c][%c][%c][%c][%c][%c][%c]",
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
	m->SendF(" Overflow: %d", phys-10);
	}
      }
    else if(ctr == 3) {
      m->SendF("  Stru: [%c][%c][%c][%c][%c][%c][%c][%c][%c][%c]",
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
      }
    else if(ctr == 5) {
      m->SendF("  Sex: %c, %d.%.3dkg, %d.%.3dm, %dv, %dY\n\n",
	gender, weight / 1000, weight % 1000,
	size / 1000, size % 1000, volume, value
	);
      }
    m->Send("\n");
    }

  map<string,int> skills = GetSkills();
  map<string,int> sks;
  map<string,int>::iterator skl;
  map<string,int> nsks;
  map<string,int>::iterator nskl;

  if(!HasSkill("WeaponType")) {
    for(skl = skills.begin(); skl != skills.end(); ++skl) {
      if(is_skill(skl->first)) sks[skl->first] = skl->second;
      else nsks[skl->first] = skl->second;
      }
    skl = sks.begin();
    nskl = nsks.begin();
    while(nskl != nsks.end() || skl != sks.end()) {
      if(skl != sks.end()) {
	m->SendF("%28s: %2d ", skl->first.c_str(), MIN(99, skl->second));
	++skl;
	}
      else {
	m->SendF("%28s     ", " ");
	}

      if(nskl != nsks.end()) {
	m->SendF("%28s: %7d", nskl->first.c_str(), nskl->second);
	++nskl;
	}

      m->Send("\n");
      }
    }

  if(Attribute(1) > 0) {
    m->Send(CYEL);
    m->SendF("\nEarned Exp: %4d  Player Exp: %4d  Unspent Exp: %4d\n", exp,
	(minds.count(m) && m->Owner()) ? m->Owner()->Exp() : -1,
	(minds.count(m) && m->Owner()) ? Exp(m->Owner()) : 0);
    m->Send(CNRM);
    }
  else {
    m->SendF(CYEL "\n  Light Level: %d (%d)",
	Skill("Light Source"), LightLevel()
	);
    }
  }

void Object::SendStats(Mind *m, Object *o) {
  if(!m) return;

  m->Send("\n");

  if(HasSkill("WeaponType")) {
    static char sevs[] = { '-', 'L', 'M', 'S', 'D' };
    m->SendF("    %s: (Str+%d)%c",
	get_weapon_skill(Skill("WeaponType")).c_str(),
	Skill("WeaponForce"), sevs[MIN(4, Skill("WeaponSeverity"))]);
    if(Skill("WeaponSeverity") > 4)
      m->SendF("%d", (Skill("WeaponSeverity")-4)*2);
    if(Skill("WeaponReach") > 4)
      m->SendF("  Range: %d", Skill("WeaponReach"));
    else if(Skill("WeaponReach") >= 0)
      m->SendF("  Reach: %d", Skill("WeaponReach"));
    m->Send("\n");
    }

  for(act_t act = ACT_MAX; act < ACT_SPECIAL_MAX; act = act_t(int(act)+1)) {
    if(ActTarg(act)) m->SendF("  %s -> %s\n", act_str[act], ActTarg(act)->Name());
    else if(IsAct(act)) m->SendF("  %s\n", act_str[act]);
    }

  if(IsActive()) m->Send("  ACTIVE\n");

  set<Mind*>::iterator mind;
  for(mind = minds.begin(); mind != minds.end(); ++mind) {
    if((*mind)->Owner()) {
      m->SendF("->Player Connected: %s (%d exp)\n",
	(*mind)->Owner()->Name(), (*mind)->Owner()->Exp());
      }
    else if((*mind) == get_mob_mind()) {
      m->Send("->MOB_MIND\n");
      }
    else if((*mind) == get_tba_mob_mind()) {
      m->Send("->TBA_MOB_MIND\n");
      }
    else if((*mind) == get_circle_mob_mind()) {
      m->Send("->CIRCLE_MOB_MIND\n");
      }
    else if((*mind)->Type() == MIND_TBATRIG) {
      m->Send("->CIRCLE_TBA_TRIGGER\n");
      }
    }
  m->Send(CNRM);
  }

void Object::AddLink(Object *ob) {
  typeof(contents.begin()) ind;
  ind = find(contents.begin(), contents.end(), ob);
  if(ind == contents.end()) {
    contents.push_back(ob);
//    typeof(contents.begin()) place = contents.end();
//    for(ind = contents.begin(); ind != contents.end(); ++ind) {
//      if((*ind) == ob) return;				//Already there!
//      if((*(*ind)) == (*ob)) { place = ind; ++place; }	//Likes by likes
//      }
//    contents.insert(place, ob);
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

void Object::Link(Object *other, const string &name, const string &desc,
	const string &oname, const string &odesc) {
  Object *door1 = new Object(this);
  Object *door2 = new Object(other);
  door1->SetShortDesc(name.c_str());
  door2->SetShortDesc(oname.c_str());
  door1->SetDesc(desc.c_str());
  door2->SetDesc(odesc.c_str());
  door1->AddAct(ACT_SPECIAL_LINKED, door2);
  door1->AddAct(ACT_SPECIAL_MASTER, door2);
  door1->SetSkill("Open", 1000);
  door1->SetSkill("Enterable", 1);
  door2->AddAct(ACT_SPECIAL_LINKED, door1);
  door2->AddAct(ACT_SPECIAL_MASTER, door1);
  door2->SetSkill("Open", 1000);
  door2->SetSkill("Enterable", 1);
  }

void Object::LinkClosed(Object *other, const string &name, const string &desc,
	const string &oname, const string &odesc) {
  Object *door1 = new Object(this);
  Object *door2 = new Object(other);
  door1->SetShortDesc(name.c_str());
  door2->SetShortDesc(oname.c_str());
  door1->SetDesc(desc.c_str());
  door2->SetDesc(odesc.c_str());
  door1->AddAct(ACT_SPECIAL_LINKED, door2);
  door1->AddAct(ACT_SPECIAL_MASTER, door2);
  door1->SetSkill("Closeable", 1);
  door1->SetSkill("Enterable", 1);
  door1->SetSkill("Transparent", 1000);
  door2->AddAct(ACT_SPECIAL_LINKED, door1);
  door2->AddAct(ACT_SPECIAL_MASTER, door1);
  door2->SetSkill("Closeable", 1);
  door2->SetSkill("Enterable", 1);
  door2->SetSkill("Transparent", 1000);
  }

void Object::TryCombine() {
  if(!parent) return;
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
      int val;

      val = MAX(1, Skill("Quantity")) + MAX(1, (*ind)->Skill("Quantity"));
      SetSkill("Quantity", val);

      val = Skill("Hungry") + (*ind)->Skill("Hungry");
      SetSkill("Hungry", val);

      val = Skill("Bored") + (*ind)->Skill("Bored");
      SetSkill("Bored", val);

      val = Skill("Needy") + (*ind)->Skill("Needy");
      SetSkill("Needy", val);

      val = Skill("Tired") + (*ind)->Skill("Tired");
      SetSkill("Tired", val);

      (*ind)->Recycle();
      break;
      }
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

  if(att[1] > 0) {	//Type 135 (ROOM-ENTER) Triggers
    typeof(contents.begin()) trig = dest->contents.begin();
    for(; trig != dest->contents.end(); ++trig) {
      if((*trig)->Skill("TBAScriptType") == 135) {
	if((rand() % 100) < (*trig)->Skill("TBAScriptNArg")) {	// % Chance
	  //fprintf(stderr, "Triggering: %s\n", (*trig)->Name());
	  new_trigger(0, *trig, this);
	  }
	}
      }
    }

  Object *oldp = parent;
  parent->RemoveLink(this);
  parent = dest;
  oldp->NotifyGone(this, dest);
  parent->AddLink(this);

  typeof(touching_me) touches = touching_me;
  typeof(touches.begin()) touch = touches.begin();
  for(; touch != touches.end(); ++touch) {
    (*touch)->NotifyLeft(this, dest);
    }

  if(try_combine) TryCombine();

  StopAct(ACT_POINT);
  StopAct(ACT_FOLLOW);
  if(IsAct(ACT_HOLD) && ActTarg(ACT_HOLD)->Parent() != this) {	//Dragging
    if(ActTarg(ACT_HOLD)->Parent() != Parent()) {	//Didn't come with me!
      StopAct(ACT_HOLD);
      }
    }
  SetSkill("Hidden", 0);

  if(parent->Skill("DynamicInit") > 0) {  //Room is dynamic, but uninitialized
    parent->DynamicInit();
    }

  if(parent->Skill("Secret")) {
    set<Mind*>::iterator m;
    for(m = minds.begin(); m != minds.end(); ++m) {
      if((*m)->Owner()) {
	if((*m)->Owner()->Accomplish(parent->Skill("Secret"))) {
	  (*m)->SendF("%sYou gain a player experience point for finding a secret!\n%s",
		CYEL, CNRM);
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
  if(default_initial == this) default_initial = universe;

  //fprintf(stderr, "Deleting: %s\n", Name(0));

  set<Object*> movers;
  set<Object*> killers;
  typeof(contents.begin()) ind;
  for(ind = contents.begin(); ind != contents.end(); ++ind) {
    if(is_pc(*ind)) {
      movers.insert(*ind);
      }
    else {
      killers.insert(*ind);
      }
    }

  typeof(killers.begin()) indk;
  for(indk = killers.begin(); indk != killers.end(); ++indk) {
    if(find(contents.begin(), contents.end(), *indk) != contents.end()) {
      (*indk)->SetParent(NULL);
      RemoveLink(*indk);
      (*indk)->Recycle();
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
    (*indk)->Recycle();
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
  if(ActTarg(ACT_SPECIAL_LINKED))
    tonotify.insert(ActTarg(ACT_SPECIAL_LINKED));

  typeof(touching_me) touches = touching_me;
  typeof(touches.begin()) touch = touches.begin();
  for(; touch != touches.end(); ++touch) {
    tonotify.insert(*touch);
    }

  StopAct(ACT_SPECIAL_MASTER);
  StopAct(ACT_SPECIAL_MONITOR);
  StopAct(ACT_SPECIAL_LINKED);

  set<Object*>::iterator noti;
  for(noti = tonotify.begin(); noti != tonotify.end(); ++noti) {
    int del = 0;
    if((*noti)->ActTarg(ACT_SPECIAL_MASTER) == this) del = 1;
    else if((*noti)->ActTarg(ACT_SPECIAL_LINKED) == this) del = 1;
    (*noti)->NotifyLeft(this);
    if(del) (*noti)->Recycle();
    }

  busylist.erase(this);

  if(inbin && trash_bin) {
    parent = trash_bin;
    parent->AddLink(this);
    }

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

int get_ordinal(const char *text) {
  int ret = 0, len = 0;
  while((!isgraph(*text)) && (*text)) ++text;
  while(isgraph(text[len])) ++len;
  if(isdigit(*text)) {
    const char *suf = text;
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
  else if(!strncasecmp(text, "some ",  5)) ret = SOME;
  else if(!strncasecmp(text, "some.",  5)) ret = SOME;
  return ret;
  }

int strip_ordinal(const char **text) {
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

static const char *splits[4] = {"Hungry", "Bored", "Tired", "Needy"};
Object *Object::Split(int nqty) {
  if(nqty < 1) nqty = 1;
  int qty = Skill("Quantity") - nqty;
  if(qty < 1) qty = 1;

  Object *nobj = new Object(*this);
  nobj->SetParent(Parent());
  nobj->SetSkill("Quantity", (nqty <= 1) ? 0 : nqty);

  SetSkill("Quantity", (qty <= 1) ? 0 : qty);

  for(int ctr = 0; ctr < 4; ++ctr) {
    int val = Skill(splits[ctr]);
    int nval = val / (qty + nqty) * nqty;
    val -= nval;
    SetSkill(splits[ctr], val);
    nobj->SetSkill(splits[ctr], nval);
    }

  return nobj;
  }

static int tag(Object *obj, list<Object *> &ret, int *ordinal, int show = 0) {
  //Only Ninjas in Ninja-Mode should detect these
  if(obj->HasSkill("Invisible") && (show <= 1)) return 0;

  //Can't be seen/affected (except in char rooms)
  if(obj->Skill("Hidden") > 0 && (!show)) return 0;
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
      nobj = obj->Split(1);
      ret.push_back(nobj);
      return 1;
      }
    }

  else if(*ordinal == ALL) rqty = cqty + 1;
  else if(*ordinal == SOME) rqty = cqty;

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
    nobj = obj->Split(rqty);
    ret.push_back(nobj);
    return 1;
    }
  return 0;
  }

list<Object*> Object::PickObjects(const char *name, int loc, int *ordinal) {
  typeof(contents) ret;

  while((!isgraph(*name)) && (*name)) ++name;

  int ordcontainer;
  if(ordinal) strip_ordinal(&name);
  else { ordinal = &ordcontainer; (*ordinal) = strip_ordinal(&name); }
  if(!strcasecmp(name, "all")) (*ordinal) = ALL;
  if(!strcasecmp(name, "everyone")) (*ordinal) = ALL;
  if(!strcasecmp(name, "everything")) (*ordinal) = ALL;
  if(!(*ordinal)) (*ordinal) = 1;

  const char *keyword = NULL;
  const char *keyword2 = NULL;
  if((keyword = strstr(name, "'s ")) || (keyword2 = strstr(name, "'S "))) {
    if(keyword && keyword2) keyword = MIN(keyword, keyword2);
    else if(!keyword) keyword = keyword2;
    char *keyword3 = strdup(name);
    keyword3[keyword-name] = 0;

    typeof(contents) masters = PickObjects(keyword3, loc, ordinal);
    if(!masters.size()) { free(keyword3); return ret; }

    typeof(masters.begin()) master;
    for(master = masters.begin(); master != masters.end(); ++master) {
      typeof(contents) add =
	(*master)->PickObjects(keyword3 + (keyword-name)+3,
		(loc & LOC_SPECIAL)|LOC_INTERNAL
		);
      ret.insert(ret.end(), add.begin(), add.end());
      }
    free(keyword3);
    return ret;
    }

  int len = strlen(name);
  while(!isgraph(name[len-1])) --len;

  if(loc & LOC_SELF) {
    if((!strcasecmp(name, "self"))
	|| (!strcasecmp(name, "myself"))
	|| (!strcasecmp(name, "me"))
	) {
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
      return PickObjects(name, loc & (LOC_SPECIAL|LOC_INTERNAL|LOC_SELF));
      }
    }

  if((loc & LOC_NEARBY) && (parent != NULL)) {
    typeof(parent->Contents()) cont = parent->Contents();

    typeof(cont.begin()) ind;
    for(ind = cont.begin(); ind != cont.end(); ++ind) if(!(*ind)->no_seek) {
      if((*ind) == this) continue;  // Must use "self" to pick self!
      if((*ind)->Filter(loc) && (*ind)->Matches(name)) {
	if(tag(*ind, ret, ordinal,
		(parent->Parent() == NULL) | (loc & LOC_SPECIAL)
		)) {
	  return ret;
	  }
	}
      if((*ind)->Skill("Open") || (*ind)->Skill("Transparent")) {
	typeof(contents) add = (*ind)->PickObjects(name, LOC_INTERNAL, ordinal);
	ret.insert(ret.end(), add.begin(), add.end());

	if((*ordinal) == 0) return ret;
	}
      }
    if(parent->Skill("Open") || parent->Skill("Transparent")) {
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
	if(action->second->Filter(loc) && action->second->Matches(name)
		&& ((loc & LOC_NOTWORN) == 0 || action->first <= ACT_HOLD)
		) {
	  if(tag(action->second, ret, ordinal,
		(Parent() == NULL) | (loc & LOC_SPECIAL)
		)) {
	    return ret;
	    }
	  }
	if(action->second->HasSkill("Container")) {
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
      if((*ind)->Filter(loc) && (*ind)->Matches(name)) {
	if(tag(*ind, ret, ordinal,
		(Parent() == NULL) | (loc & LOC_SPECIAL)
		)) {
	  return ret;
	  }
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

int Object::IsWithin(const Object *obj) {
  if(no_seek) return 0;
  typeof(contents.begin()) ind;
  for(ind = contents.begin(); ind != contents.end(); ++ind) {
    if((*ind) == obj) return 1;
    if((*ind)->Skill("Open") || (*ind)->Skill("Transparent")) {
      int ret = (*ind)->IsWithin(obj);
      if(ret) return ret;
      }
    }
  return 0;
  }

int Object::IsNearBy(const Object *obj) {
  if(no_seek || (!parent)) return 0;
  typeof(contents.begin()) ind;
  for(ind = parent->contents.begin(); ind != parent->contents.end(); ++ind) {
    if((*ind) == obj) return 1;
    if((*ind) == this) continue;  // Not Nearby Self
    if((*ind)->Skill("Open") || (*ind)->Skill("Transparent")) {
      int ret = (*ind)->IsWithin(obj);
      if(ret) return ret;
      }
    }
  if(parent->parent && (parent->Skill("Open") || parent->Skill("Transparent"))) {
    parent->no_seek = 1;
    int ret = parent->IsNearBy(obj);
    parent->no_seek = 0;
    if(ret) return ret;
    }
  return 0;
  }

void Object::NotifyLeft(Object *obj, Object *newloc) {
  set<act_t> stops, stops2;
  typeof(act.begin()) curact = act.begin();
  for(; curact != act.end(); ++curact) {
    if(curact->second && curact->first < ACT_MAX && (
		curact->second == obj || obj->IsWithin(curact->second)
	)) {
      if(curact->first != ACT_FOLLOW || (!newloc)) {
	stops.insert(curact->first);
	}
      else if(parent != newloc) {	// Do nothing if we didn't leave!
	int stealth_t = 0, stealth_s = 0;
	if(IsUsing("Stealth") && Skill("Stealth") > 0) {
	  stealth_t = Skill("Stealth");
	  stealth_s = Roll("Stealth", 2);
	  }
	parent->SendOut(stealth_t, stealth_s,
		";s follows ;s.\n", "You follow ;s.\n", this, obj);
	Travel(newloc);
	parent->SendOut(stealth_t, stealth_s,
		";s follows ;s.\n", "", this, obj);
	AddAct(ACT_FOLLOW, obj);
	if(stealth_t > 0) {
	  SetSkill("Hidden", Roll("Stealth", 2) * 2);
	  }
	}
      }
    if(curact->first >= ACT_MAX && (!newloc) && curact->second == obj) {
      typeof(act.begin()) curact2 = act.begin();
      for(; curact2 != act.end(); ++curact2) {
	if(curact2->first >= ACT_MAX) {
	  if(curact2->second == this) {
	    stops2.insert(curact2->first);
	    }
	  }
	}
      stops.insert(curact->first);
      }
    }

  set<act_t>::iterator stop = stops.begin();
  for(; stop != stops.end(); ++stop) {
    StopAct(*stop);
    }
  stop = stops2.begin();
  for(; stop != stops2.end(); ++stop) {
    obj->StopAct(*stop);
    }

  if(obj->ActTarg(ACT_HOLD) == this) {		//Dragging
    if(parent != newloc) {	// Actually went somewhere
      parent->SendOut(ALL, -1,
	";s drags ;s along.\n", "You drag ;s along with you.\n",
	obj, this);
      Travel(newloc, 0);
      parent->SendOut(ALL, -1, ";s drags ;s along.\n", "", obj, this);
      }
    }
  }

void Object::NotifyGone(Object *obj, Object *newloc, int up) {
  if(obj == this) return;	//Never notify self or sub-self objects.

  //Climb to top first!
  if(up == 1 && parent && (Skill("Open") || Skill("Transparent"))) {
    parent->NotifyGone(obj, newloc, 1);
    return;
    }

  NotifyLeft(obj, newloc);

  map<Object *, int> tonotify;

  typeof(contents.begin()) ind;
  for(ind = contents.begin(); ind != contents.end(); ++ind) {
    if(up >= 0) {
      tonotify[*ind] = -1;
      }
    else if((*ind)->Skill("Open") || (*ind)->Skill("Transparent")) {
      tonotify[*ind] = 0;
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

void Object::AddAct(act_t a, Object *o) {
  if(o) {
    o->touching_me.insert(this);
    }
  act[a] = o;
  }

void Object::StopAct(act_t a) {
  Object *obj = act[a];
  act.erase(a);
  if(obj && obj->HasSkill("Brightness") && obj->HasSkill("Light Source")) {
    obj->SetSkill("Light Source", 0);
    //obj->SendOut(0, 0, ";s goes out.\n", "", obj, NULL);
    obj->Deactivate();
    }
  if(obj) {
    typeof(act.begin()) opt = act.begin();
    for(; opt != act.end(); ++opt) {
      if(opt->second == obj) {
	return;				//Still touching it 
	}
      }
    obj->touching_me.erase(this);	//No longer touching it
    }
  }

void Object::StopAll() {
  typeof(act) oldact = act;
  typeof(oldact.begin()) opt = oldact.begin();
  for(; opt != oldact.end(); ++opt) {
    StopAct(opt->first);
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
  if(parent) {
    if(pos != POS_LIE) {
      parent->SendOut(ALL, -1, ";s collapses!\n", "You collapse!\n", this, NULL);
      pos = POS_LIE;
      }
    if(IsAct(ACT_WIELD)) {
      parent->SendOutF(ALL, -1, ";s drops %s!\n", "You drop %s!\n",
		this, NULL, ActTarg(ACT_WIELD)->ShortDesc());
      ActTarg(ACT_WIELD)->Travel(parent);
      }
    if(IsAct(ACT_HOLD) && ActTarg(ACT_HOLD) != ActTarg(ACT_WEAR_SHIELD)) {
      parent->SendOutF(ALL, -1, ";s drops %s!\n", "You drop %s!\n",
		this, NULL, ActTarg(ACT_HOLD)->ShortDesc());
      ActTarg(ACT_HOLD)->Travel(parent);
      }
    else if(IsAct(ACT_HOLD)) {
      parent->SendOutF(ALL, -1, ";s stops holding %s.\n", "You stop holding %s!\n",
		this, NULL, ActTarg(ACT_HOLD)->ShortDesc());
      StopAct(ACT_HOLD);
      }
    }
  }

void Object::UpdateDamage() {
  if(stun > 10) {	//Overflow
    phys += stun-10;
    stun = 10;
    }
  if(stun < Skill("Hungry") / 5000) {	//Hungry Stuns
    stun = Skill("Hungry") / 5000;
    }
  if(phys < Skill("Thirsty") / 5000) {	//Thirsty Wounds
    phys = Skill("Thirsty") / 5000;
    }
  if(phys > 10+Attribute(2)) {
    phys = 10+Attribute(2)+1;

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
  succ -= roll(Attribute(0) + Modifier("Resilience"), force);
  sev *= 2;  sev += succ;
  for(int ctr=0; ctr<=(sev/2) && ctr<=4; ++ctr) stun += ctr;
  if(sev > 8) stun += (sev-8);
  if(stun > 10) stun = 10;  // No Overflow to Phys from "Ment"
  UpdateDamage();
  return sev;
  }

int Object::HitStun(int force, int sev, int succ) {
  if(Attribute(1) <= 0) return 0;
  succ -= roll(Attribute(0) + Modifier("Resilience"), force);
  sev *= 2;  sev += succ;
  for(int ctr=0; ctr<=(sev/2) && ctr<=4; ++ctr) stun += ctr;
  if(sev > 8) stun += (sev-8);
  UpdateDamage();
  return sev;
  }

int Object::HitPhys(int force, int sev, int succ) {
  if(Attribute(1) <= 0) return 0;
  succ -= roll(Attribute(0) + Modifier("Resilience"), force);
  sev *= 2;  sev += succ;
  for(int ctr=0; ctr<=(sev/2) && ctr<=4; ++ctr) phys += ctr;
  if(sev > 8) phys += (sev-8);
  UpdateDamage();
  return sev;
  }

int Object::HitStru(int force, int sev, int succ) {
  succ -= roll(Attribute(0) + Modifier("Resilience"), force);
  sev *= 2;  sev += succ;
  for(int ctr=0; ctr<=(sev/2) && ctr<=4; ++ctr) stru += ctr;
  if(sev > 8) stru += (sev-8);
  UpdateDamage();
  return sev;
  }

void Object::Send(int tnum, int rsucc, const char *mes) {
  char *tosend = strdup(mes);
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

void Object::SendF(int tnum, int rsucc, const char *mes, ...) {
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

  Send(tnum, rsucc, buf);
  }

void Object::SendIn(int tnum, int rsucc, const char *mes, const char *youmes,
	Object *actor, Object *targ) {
  if(no_seek) return;

  string tstr = "";  if(targ) tstr = (char*)targ->Name(0, this, actor);
  string astr = "";  if(actor) astr = (char*)actor->Name(0, this);

  char *str = strdup(mes);
  char *youstr = strdup(youmes);

  for(char *ctr=str; *ctr; ++ctr) if((*ctr) == ';') (*ctr) = '%';
  for(char *ctr=youstr; *ctr; ++ctr) if((*ctr) == ';') (*ctr) = '%';

  if(youmes && youstr[0] == '*' && this == actor) {
    Send(ALL, -1, CYEL);
    if(targ) SendF(ALL, -1, youstr+1, tstr.c_str());
    else Send(ALL, -1, youstr+1);
    Send(ALL, -1, CNRM);
    }
  else if(str[0] == '*' && targ == this) {
    Send(ALL, -1, CRED);
    if(targ || actor) SendF(tnum, rsucc, str+1, astr.c_str(), tstr.c_str());
    else Send(tnum, rsucc, str+1);
    Send(ALL, -1, CNRM);
    }
  else if(str[0] == '*') {
    Send(ALL, -1, CMAG);
    if(targ || actor) SendF(tnum, rsucc, str+1, astr.c_str(), tstr.c_str());
    else Send(tnum, rsucc, str+1);
    Send(ALL, -1, CNRM);
    }
  else if(youmes && this == actor) {
    if(targ) SendF(ALL, -1, youstr, tstr.c_str());
    else Send(ALL, -1, youstr);
    }
  else {
    if(targ || actor) SendF(tnum, rsucc, str, astr.c_str(), tstr.c_str());
    else Send(tnum, rsucc, str);
    }
  free(str);
  free(youstr);

  typeof(contents.begin()) ind;
  for(ind = contents.begin(); ind != contents.end(); ++ind) {
    if((*ind)->Skill("Open") || (*ind)->Skill("Transparent"))
      (*ind)->SendIn(tnum, rsucc, mes, youmes, actor, targ);
    else if((*ind)->Pos() != POS_NONE)	//FIXME - Understand Transparency
      (*ind)->SendIn(tnum, rsucc, mes, youmes, actor, targ);
    }
  }

void Object::SendInF(int tnum, int rsucc, const char *mes, const char *youmes,
	Object *actor, Object *targ, ...) {
  if(no_seek) return;

  static char buf[65536];
  static char youbuf[65536];

  memset(buf, 0, 65536);
  memset(youbuf, 0, 65536);
  va_list stuff;
  va_start(stuff, targ);
  vsprintf(buf, mes, stuff);
  va_end(stuff);
  va_start(stuff, targ);
  vsprintf(youbuf, youmes, stuff);
  va_end(stuff);

  SendIn(tnum, rsucc, buf, youbuf, actor, targ);
  }

void Object::SendOut(int tnum, int rsucc, const char *mes, const char *youmes,
	Object *actor, Object *targ) {
  if(no_seek) return;

  if(!strncmp(mes, ";s says '", 9)) {	//Type 132 (ROOM-SPEECH) Triggers
    typeof(contents.begin()) trig = contents.begin();
    for(; trig != contents.end(); ++trig) {
      if((*trig)->Skill("TBAScriptType") == 132) {
        if((rand() % 100) < (*trig)->Skill("TBAScriptNArg")) {  // % Chance
	  string speech = mes+9;
	  size_t len = speech.find_last_of("'");
	  if(len != string::npos) speech = speech.substr(0, len);
          //fprintf(stderr, "Triggering: %s\n", (*trig)->Name());
          new_trigger((rand() % 400) + 300, *trig, actor, speech);
          }
        }
      }
    }

  string tstr = "";  if(targ) tstr = (char*)targ->Name(0, this, actor);
  string astr = "";  if(actor) astr = (char*)actor->Name(0, this);

  char *str = strdup(mes);
  char *youstr = strdup(youmes);

  for(char *ctr=str; *ctr; ++ctr) if((*ctr) == ';') (*ctr) = '%';
  for(char *ctr=youstr; *ctr; ++ctr) if((*ctr) == ';') (*ctr) = '%';

  if(youmes && youstr[0] == '*' && this == actor) {
    Send(ALL, -1, CGRN);
    if(targ) SendF(ALL, -1, youstr+1, tstr.c_str());
    else Send(ALL, -1, youstr+1);
    Send(ALL, -1, CNRM);
    }
  else if(str[0] == '*') {
    Send(ALL, -1, CRED);
    if(targ || actor) SendF(tnum, rsucc, str+1, astr.c_str(), tstr.c_str());
    else Send(tnum, rsucc, str+1);
    Send(ALL, -1, CNRM);
    }
  else if(youmes && this == actor) {
    if(targ) SendF(ALL, -1, youstr, tstr.c_str());
    else Send(ALL, -1, youstr);
    }
  else {
    if(targ || actor) SendF(tnum, rsucc, str, astr.c_str(), tstr.c_str());
    else Send(tnum, rsucc, str);
    }
  free(str);
  free(youstr);

  typeof(contents.begin()) ind;
  for(ind = contents.begin(); ind != contents.end(); ++ind) {
    if((*ind)->Skill("Open") || (*ind)->Skill("Transparent"))
      (*ind)->SendIn(tnum, rsucc, mes, youmes, actor, targ);
    else if((*ind)->Pos() != POS_NONE)	//FIXME - Understand Transparency
      (*ind)->SendIn(tnum, rsucc, mes, youmes, actor, targ);
    }

  if(parent && (Skill("Open") || Skill("Transparent"))) {
    no_seek = 1;
    parent->SendOut(tnum, rsucc, mes, youmes, actor, targ);
    no_seek = 0;
    }

  }

void Object::SendOutF(int tnum, int rsucc, const char *mes, const char *youmes,
	Object *actor, Object *targ, ...) {
  if(no_seek) return;

  static char buf[65536];
  static char youbuf[65536];

  memset(buf, 0, 65536);
  memset(youbuf, 0, 65536);
  va_list stuff;
  va_start(stuff, targ);
  vsprintf(buf, mes, stuff);
  va_end(stuff);
  va_start(stuff, targ);
  vsprintf(youbuf, youmes, stuff);
  va_end(stuff);

  SendOut(tnum, rsucc, buf, youbuf, actor, targ);
  }

void Object::Loud(int str, const char *mes) {
  set<Object*> visited;
  Loud(visited, str, mes);
  }

void Object::LoudF(int str, const char *mes, ...) {
  static char buf[65536];

  if(mes[0] == 0) return;

  memset(buf, 0, 65536);
  va_list stuff;
  va_start(stuff, mes);
  vsprintf(buf, mes, stuff);
  va_end(stuff);

  set<Object*> visited;
  Loud(visited, str, buf);
  }

void Object::Loud(set<Object*> &visited, int str, const char *mes) {
  visited.insert(this);
  typeof(Contents()) targs;
  typeof(targs.begin()) targ_it;
  targs = PickObjects("all", LOC_INTERNAL);
  for(targ_it = targs.begin(); targ_it != targs.end(); ++targ_it) {
    Object *dest = *targ_it;
    if(dest->HasSkill("Enterable")) {
      int ostr=str;
      --str;
      if(dest->Skill("Open") < 1) {
	--str;
	}
      if(str > 0) {
	if(dest->ActTarg(ACT_SPECIAL_LINKED)
		&& dest->ActTarg(ACT_SPECIAL_LINKED)->Parent()) {
	  dest = dest->ActTarg(ACT_SPECIAL_LINKED);
	  if(visited.count(dest->Parent()) < 1) {
	    dest->Parent()->SendOutF(ALL, 0,
		"From ;s you hear %s\n", "",
		dest, dest, mes);
	    dest->Parent()->Loud(visited, str, mes);
	    }
	  }
	}
      str=ostr;
      }
    }
  }

void init_world() {
  universe = new Object;
  universe->SetShortDesc("The Universe");
  universe->SetDesc("An Infinite Universe within which to play.");
  universe->SetLongDesc("A Really Big Infinite Universe within which to play.");
  default_initial = universe;
  trash_bin = new Object;
  trash_bin->SetShortDesc("The Trash Bin");
  trash_bin->SetDesc("The place objects come to die.");

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
    autoninja->Recycle();
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

int Object::BusyAct() {
//  fprintf(stderr, "Taking busyact %p!\n", this);
  busylist.erase(this);

  if(StillBusy()) {		//Should only be true on non-first round action
    if(IsAct(ACT_FIGHT)) {	//If in actual combat (real rounds)
      busytill.tv_sec -= 3;	//...remove the time used of last action
      }
    }

  string comm = dowhenfree;
  string def = defact;
  dowhenfree = "";
  defact = "";

//  fprintf(stderr, "Act is %s [%s]!\n", comm.c_str(), def.c_str());

  int ret;
  if(minds.size()) {
    ret = handle_command(this, comm.c_str(), *(minds.begin()));
    if(ret != 2 && (!StillBusy()))
	ret = handle_command(this, def.c_str(), *(minds.begin()));
    }
  else {
    ret = handle_command(this, comm.c_str());
    if(ret != 2 && (!StillBusy()))
	ret = handle_command(this, def.c_str());
    }
  return (ret == 2); //Return 1 if no more actions this round!
  }

void Object::FreeActions() {
  int maxinit = 0;
  map<Object *, list<int> > initlist;
  for(set<Object *>::iterator busy = busylist.begin();
		busy != busylist.end(); ++busy) {
    if(!(*busy)->StillBusy()) {
      initlist[*busy] = (*busy)->RollInitiative();
      if(maxinit < initlist[*busy].front()) {
	maxinit = initlist[*busy].front();
	}
      }
    }
  for(int phase = maxinit; phase > 0; --phase) {
    for(map<Object *,list<int> >::iterator init = initlist.begin();
		init != initlist.end(); ++init) {
				// Make sure it's still in busylist
				// (hasn't been deleted by another's BusyAct)!
      if(init->second.front() == phase && busylist.count(init->first)) {
//	if(init->first->IsAct(ACT_FIGHT))
//	  fprintf(stderr, "Going at %d (%s)\n", phase, init->first->Name());
	int ret = init->first->BusyAct();
	if(ret || init->second.size() <= 1) init->second.front() = 0;
	else init->second.pop_front();
	}
      }
    }
  for(map<Object *,list<int> >::iterator init = initlist.begin();
	init != initlist.end(); ++init) {

    // Still in combat!
    if(init->first->IsAct(ACT_FIGHT) && (!init->first->StillBusy())) {
      string ret = init->first->Tactics();
      init->first->BusyFor(3000, ret.c_str());
      }
    }
  }

string Object::Tactics(int phase) {
  if(minds.size() < 1) return "attack";
  Mind *mind = (*(minds.begin()));	//FIXME: Handle Multiple Minds
  Object *body = mind->Body();
  mind->Attach(this);
  string ret = mind->Tactics();
  mind->Attach(body);
  return ret;
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

  if(minds != in.minds) return 0;	//Only no-mind or same-group-minds

  if(act.size() != 0 || in.act.size() != 0) return 0;

  if(contents.size() != 0 || in.contents.size() != 0) return 0;

  map<string,int> sk1 = skills;
  map<string,int> sk2 = in.skills;
  sk1.erase("Quantity");
  sk2.erase("Quantity");
  sk1.erase("Hungry");
  sk2.erase("Hungry");
  sk1.erase("Bored");
  sk2.erase("Bored");
  sk1.erase("Needy");
  sk2.erase("Needy");
  sk1.erase("Tired");
  sk2.erase("Tired");
  if(sk1 != sk2) return 0;

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

  skills = in.skills;

  pos = in.pos;

  if(in.IsActive()) Activate();
  else Deactivate();

//  contents = in.contents;	// NOT doing this for deep/shallow reasons.
//  act = in.act;
  }

list<Object *> Object::Contents() {
  return contents;
  }

int Object::Contains(const Object *obj) {
  return (find(contents.begin(), contents.end(), obj) != contents.end());
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

int two_handed(int wtype) {
  static set<int> thsks;
  if(thsks.size() == 0) {
    thsks.insert(get_weapon_type("Two-Handed Blades"));
    thsks.insert(get_weapon_type("Two-Handed Cleaves"));
    thsks.insert(get_weapon_type("Two-Handed Crushing"));
    thsks.insert(get_weapon_type("Two-Handed Flails"));
    thsks.insert(get_weapon_type("Two-Handed Piercing"));
    thsks.insert(get_weapon_type("Two-Handed Staves"));
    }
  return int(thsks.count(wtype));
  }

const char *Object::PosString() {
  static char buf[128];
  if(pos == POS_USE) {
    sprintf(buf, "is %s here", UsingString());
    return buf;
    }
  return pos_str[pos];
  }

const char *Object::UsingString() {
  static char buf[128];
  if(pos == POS_USE) {
    if(cur_skill == "Stealth") {
      sprintf(buf, "sneaking around");
      }
    else if(cur_skill == "Lumberjack") {
      sprintf(buf, "chopping down trees");
      }
    else if(cur_skill == "Perception") {
      sprintf(buf, "keeping an eye out");
      }
    else if(cur_skill == "Healing") {
      sprintf(buf, "caring for others' wounds");
      }
    else if(cur_skill == "First Aid") {
      sprintf(buf, "giving first-aid");
      }
    else if(cur_skill == "Treatment") {
      sprintf(buf, "treating others' wounds");
      }
    else {
      sprintf(buf, "using the %s skill", Using());
      }
    return buf;
    }
  return "doing nothing";
  }

void Object::StartUsing(const string &skill) {
  cur_skill = skill;
  pos = POS_USE;
  }

void Object::StopUsing() {
  if(pos == POS_USE) pos = POS_STAND;
  cur_skill = "";
  }

const char *Object::Using() {
  return cur_skill.c_str();
  }

int Object::IsUsing(const string &skill) {
  return (string(skill) == string(cur_skill));
  }

pos_t Object::Pos() {
  return pos;
  }

void Object::SetPos(pos_t p) {
  if(pos == POS_USE && p != POS_USE) StopUsing();
  pos = p;
  }

int Object::Filter(int loc) {
  if(loc & (LOC_ALIVE|LOC_CONSCIOUS)) {
    if(Attribute(1) <= 0 || IsAct(ACT_DEAD)) return 0;
    }
  if(loc & LOC_CONSCIOUS) {
    if(IsAct(ACT_DYING) || IsAct(ACT_UNCONSCIOUS)) return 0;
    }
  return 1;
  }

int Object::LooksLike(Object *other) {
  if(string(Name()) != string(other->Name())) return 0;
  if(Pos() != other->Pos()) return 0;
  if(string(Using()) != string(other->Using())) return 0;
  for(act_t act = ACT_NONE; act < ACT_WIELD;) {		//Off-By-One!
    act = act_t(int(act)+1);				//Increments First!
    if(IsAct(act) != other->IsAct(act)) return 0;
    if(ActTarg(act) != other->ActTarg(act)) {
      string s1 = "";
      if(ActTarg(act)) s1 = ActTarg(act)->Name(0, this);
      string s2 = "";
      if(ActTarg(act)) s2 = other->ActTarg(act)->Name(0, other);
      if(s1 != s2) return 0;
      }
    }
  return 1;
  }

void Object::Consume(const Object *item) {
  //Standard food/drink effects
  SetSkill("Hungry", Skill("Hungry") - item->Skill("Food"));
  SetSkill("Thirsty", Skill("Thirsty") - item->Skill("Drink"));
  SetSkill("Thirsty", Skill("Thirsty") + item->Skill("Dehydrate Effect"));

  //Special effect: Poisonous
  if(item->Skill("Poisonous") > 0) {
    SetSkill("Poisoned", Skill("Poisoned") + item->Skill("Poisonous"));
    }

  //Special effect: Cure Poison
  if(item->Skill("Cure Poison Spell") > 0 && Skill("Poisoned") > 0) {
    if(item->Skill("Cure Poison Spell") >= Skill("Poisoned")) {
      SetSkill("Poisoned", 0);
      Send(ALL, 0, "You feel better.\n");
      }
    }

  //Special effect: Heal
  if(item->Skill("Heal Spell") > 0) {
    int succ = Roll("Strength", 12 - item->Skill("Heal Spell"));
    if(phys > 0 && succ > 0) Send(ALL, 0, "You feel a bit less pain.\n");
    phys -= succ;
    if(phys < 0) phys = 0;
    UpdateDamage();
    }

  //Special effect: Energize
  if(item->Skill("Energize Spell") > 0) {
    int succ = Roll("Strength", 12 - item->Skill("Energize Spell"));
    if(stun > 0 && succ > 0) Send(ALL, 0, "You feel a bit more rested.\n");
    stun -= succ;
    if(stun < 0) stun = 0;
    UpdateDamage();
    }

  //Special effect: Remove Curse - Note: Can't remove curse from cursed items
  if(item->Skill("Remove Curse Spell") > 0 && (!HasSkill("Cursed"))) {
    Object *cursed = NextHasSkill("Cursed");
    while(cursed) {
      if(cursed->Skill("Cursed") <= item->Skill("Remove Curse Spell")) {
	Drop(cursed, 1, 1);
        cursed = NextHasSkill("Cursed");
	}
      else {
	cursed = NextHasSkill("Cursed", cursed);
	}
      }
    }

  //Special effect: Recall
  if(item->Skill("Recall Spell") > 0) {
    if(parent) {
      parent->SendOut(0, 0,
	"BAMF! ;s teleports away.\n", "BAMF! You teleport home.\n", this, NULL
	);
      }
    Travel(get_start_room(), 0);
    if(parent) {
      parent->SendOut(0, 0, "BAMF! ;s teleports here.\n", "", this, NULL);
      parent->SendDescSurround(this, this);
      }
    }

  //Special effect: Teleport Spell (Grants Ability)
  if(item->Skill("Teleport Spell")) {
    SetSkill("Teleport", 1);		//Can use once!
    Send(ALL, 0, "You can now teleport once!\n");
    }

  //Special effect: Resurrect Spell (Grants Ability)
  if(item->Skill("Resurrect Spell")) {
    SetSkill("Resurrect", 1);		//Can use once!
    Send(ALL, 0, "You can now resurrect a character once!\n");
    }

  //Special effect: Sleep Other
  if(item->Skill("Sleep Other Spell") > 0) {
    int succ = Roll("Willpower", item->Skill("Sleep Other Spell"));
    if(succ > 0) {
      if(parent) {
	parent->SendOut(0, 0,
		";s looks groggy for a moment, but recovers.\n",
		"You feel groggy for a moment, but recover.\n",
		this, NULL
		);
	}
      }
    else {
      handle_command(this, "sleep");
      }
    }

  }

int Object::LightLevel(int updown) {
  int level = 0;

  if(updown != -1 && Parent()) {	//Go Up
    int fac = Skill("Open") + Skill("Transparent") + Skill("Translucent");
    if(fac > 1000) fac = 1000;
    if(fac > 0) {
      level += (fac * Parent()->LightLevel(1));
      }
    }
  if(updown != 1) {	//Go Down
    typeof(contents.begin()) item = contents.begin();
    for(; item != contents.end(); ++item) {
      if(!Wearing(*item)) {	//Containing it (internal)
	int fac = (*item)->Skill("Open")
		+ (*item)->Skill("Transparent")
		+ (*item)->Skill("Translucent");
	if(fac > 1000) fac = 1000;
	if(fac > 0) {
	  level += (fac * (*item)->LightLevel(-1));
	  }
	}

      typeof(contents.begin()) subitem = (*item)->contents.begin();
      for(; subitem != (*item)->contents.end(); ++subitem) {
	//Wearing it (external - so reaching one level further)
	if((*item)->Wearing(*subitem)) {
	  level += (1000 * (*subitem)->LightLevel(-1));
	  }
	}
      }
    }
  level /= 1000;
  level += Skill("Light Source");
  if(level > 1000) level = 1000;
  return level;
  }

const char * const attnames[] = {
	"Body",
	"Quickness",
	"Strength",
	"Charisma",
	"Intelligence",
	"Willpower"
	};
int Object::Attribute(int a) const {
  if(att[a] == 0) return 0;	//Can't Enhance Nothing
  return att[a] + Modifier(attnames[a]);
  }

int Object::Modifier(const string &m) const {
  int ret = 0;
  typeof(contents.begin()) item = contents.begin();
  for(; item != contents.end(); ++item) {
    if(Wearing(*item)) {
      ret += (*item)->Skill(m + " Bonus");
      ret -= (*item)->Skill(m + " Penalty");
      }
    //FIXME: Spell Effects!
    }
  ret += Skill(m + " Bonus");
  ret -= Skill(m + " Penalty");
  if(ret < 0) return (ret-999)/1000;
  return (ret/1000);
  }

int Object::Wearing(const Object *obj) const {
  for(act_t act = ACT_HOLD; act < ACT_MAX; act = act_t(act + 1)) {
    if(ActTarg(act) == obj) return 1;
    }
  return 0;
  }

int Object::WearMask() const {
  return(Skill("Wearable on Back")
	| Skill("Wearable on Chest")
	| Skill("Wearable on Head")
	| Skill("Wearable on Neck")
	| Skill("Wearable on Waist")
	| Skill("Wearable on Shield")
	| Skill("Wearable on Left Arm")
	| Skill("Wearable on Right Arm")
	| Skill("Wearable on Left Finger")
	| Skill("Wearable on Right Finger")
	| Skill("Wearable on Left Foot")
	| Skill("Wearable on Right Foot")
	| Skill("Wearable on Left Hand")
	| Skill("Wearable on Right Hand")
	| Skill("Wearable on Left Leg")
	| Skill("Wearable on Right Leg")
	| Skill("Wearable on Left Wrist")
	| Skill("Wearable on Right Wrist")
	| Skill("Wearable on Left Shoulder")
	| Skill("Wearable on Right Shoulder")
	| Skill("Wearable on Left Hip")
	| Skill("Wearable on Right Hip")
	);
  }

set<act_t> Object::WearSlots(int m) const {
  set<act_t> locs;
  if(Skill("Wearable on Back") & m) locs.insert(ACT_WEAR_BACK);
  if(Skill("Wearable on Chest") & m) locs.insert(ACT_WEAR_CHEST);
  if(Skill("Wearable on Head") & m) locs.insert(ACT_WEAR_HEAD);
  if(Skill("Wearable on Neck") & m) locs.insert(ACT_WEAR_NECK);
  if(Skill("Wearable on Waist") & m) locs.insert(ACT_WEAR_WAIST);
  if(Skill("Wearable on Shield") & m) locs.insert(ACT_WEAR_SHIELD);
  if(Skill("Wearable on Left Arm") & m) locs.insert(ACT_WEAR_LARM);
  if(Skill("Wearable on Right Arm") & m) locs.insert(ACT_WEAR_RARM);
  if(Skill("Wearable on Left Finger") & m) locs.insert(ACT_WEAR_LFINGER);
  if(Skill("Wearable on Right Finger") & m) locs.insert(ACT_WEAR_RFINGER);
  if(Skill("Wearable on Left Foot") & m) locs.insert(ACT_WEAR_LFOOT);
  if(Skill("Wearable on Right Foot") & m) locs.insert(ACT_WEAR_RFOOT);
  if(Skill("Wearable on Left Hand") & m) locs.insert(ACT_WEAR_LHAND);
  if(Skill("Wearable on Right Hand") & m) locs.insert(ACT_WEAR_RHAND);
  if(Skill("Wearable on Left Leg") & m) locs.insert(ACT_WEAR_LLEG);
  if(Skill("Wearable on Right Leg") & m) locs.insert(ACT_WEAR_RLEG);
  if(Skill("Wearable on Left Wrist") & m) locs.insert(ACT_WEAR_LWRIST);
  if(Skill("Wearable on Right Wrist") & m) locs.insert(ACT_WEAR_RWRIST);
  if(Skill("Wearable on Left Shoulder") & m) locs.insert(ACT_WEAR_LSHOULDER);
  if(Skill("Wearable on Right Shoulder") & m) locs.insert(ACT_WEAR_RSHOULDER);
  if(Skill("Wearable on Left Hip") & m) locs.insert(ACT_WEAR_LHIP);
  if(Skill("Wearable on Right Hip") & m) locs.insert(ACT_WEAR_RHIP);
  return locs;
  }

string Object::WearNames(const set<act_t> &locs) const {
  string ret = "";
  set<act_t>::const_iterator loc = locs.begin();
  for(; loc != locs.end(); ++loc) {
    if(loc != locs.begin()) {
      typeof(loc) tmp = loc;  ++tmp;
      if(tmp == locs.end()) ret += " and ";
      else ret += ", ";	//I put no comma before " and "
      }
    if(*loc == ACT_WEAR_BACK) ret += "back";
    else if(*loc == ACT_WEAR_CHEST) ret += "chest";
    else if(*loc == ACT_WEAR_HEAD) ret += "head";
    else if(*loc == ACT_WEAR_NECK) ret += "neck";
    else if(*loc == ACT_WEAR_WAIST) ret += "waist";
    else if(*loc == ACT_WEAR_SHIELD) ret += "shield";
    else if(*loc == ACT_WEAR_LARM) ret += "left arm";
    else if(*loc == ACT_WEAR_RARM) ret += "right arm";
    else if(*loc == ACT_WEAR_LFINGER) ret += "left finger";
    else if(*loc == ACT_WEAR_RFINGER) ret += "right finger";
    else if(*loc == ACT_WEAR_LFOOT) ret += "left foot";
    else if(*loc == ACT_WEAR_RFOOT) ret += "right foot";
    else if(*loc == ACT_WEAR_LHAND) ret += "left hand";
    else if(*loc == ACT_WEAR_RHAND) ret += "right hand";
    else if(*loc == ACT_WEAR_LLEG) ret += "left leg";
    else if(*loc == ACT_WEAR_RLEG) ret += "right leg";
    else if(*loc == ACT_WEAR_LWRIST) ret += "left wrist";
    else if(*loc == ACT_WEAR_RWRIST) ret += "right wrist";
    else if(*loc == ACT_WEAR_LSHOULDER) ret += "left shoulder";
    else if(*loc == ACT_WEAR_RSHOULDER) ret += "right shoulder";
    else if(*loc == ACT_WEAR_LHIP) ret += "left hip";
    else if(*loc == ACT_WEAR_RHIP) ret += "right hip";
    }
  return ret;
  }

string Object::WearNames(int m) const {
  return WearNames(WearSlots(m));
  }

Object *Object::Stash(Object *item, int message, int force, int try_combine) {
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
    if((*con)->Skill("Capacity") - (*con)->ContainedVolume() < item->Volume())
      continue;
    if((*con)->Skill("Container") - (*con)->ContainedWeight() < item->Weight())
      continue;
    if(!dest) dest = (*con);  //It CAN go here....
    for(ind = (*con)->contents.begin(); ind != (*con)->contents.end(); ++ind) {
      if((*item) == (*(*ind))) { dest = (*con); break; }
      }
    }

  //See if it actually makes it!
  if(dest && (item->Travel(dest, try_combine))) dest = NULL;


  if(message && dest) {
    int openclose = 0;
    if(dest->Skill("Open") < 1) openclose = 1;

    if(openclose)
      parent->SendOut(0, 0, ";s opens ;s.\n", "You open ;s.\n", this, dest);
    parent->SendOut(0, 0, ";s stashes ;s.\n", "You stash ;s.\n", this, item);
    if(openclose)
      parent->SendOut(0, 0, ";s closes ;s.\n", "You close ;s.\n", this, dest);
    }

  return dest;
  }

int Object::Drop(Object *item, int message, int force, int try_combine) {
  if(!item) return 1;
  if(!parent) return 1;

	//Can't drop cursed stuff (unless ninja or otherwise forcing)
  if((!force) && item->SubHasSkill("Cursed")) {
    return -4;
    }

  int ret = item->Travel(parent, try_combine);
  if(ret) return ret;

	//Activate perishable dropped stuff, so it will rot
  if(item->HasSkill("Perishable")) {
    item->Activate();
    }

  if(message) {
    parent->SendOut(0, 0, ";s drops ;s.\n", "You drop ;s.\n", this, item);
    }
  return 0;
  }

int Object::DropOrStash(Object *item, int message, int force, int try_combine) {
  int ret = Drop(item, message, force, try_combine);
  if(ret) {
    if(!Stash(item, message, force, try_combine)) {
      return ret;
      }
    }
  return 0;
  }

int Object::StashOrDrop(Object *item, int message, int force, int try_combine) {
  if(!Stash(item, message, force, try_combine)) {
    return Drop(item, message, force, try_combine);
    }
  return 0;
  }

int Object::SubHasSkill(const string &s) const {
  if(HasSkill(s)) return 1;
  typeof(contents.begin()) item = contents.begin();
  for(; item != contents.end(); ++item) {
    if((*item)->SubHasSkill(s)) return 1;
    }
  return 0;
  }

Object *Object::NextHasSkill(const string &s, const Object *last) {
  if(HasSkill(s) && (!last)) return this;
  if(last == this) last = NULL;				//I was last one
  typeof(contents.begin()) item = contents.begin();
  for(; item != contents.end(); ++item) {
    Object *found = (*item)->NextHasSkill(s, last);
    if(found) return found;
    if(last && (last == (*item) || (*item)->IsWithin(last))) {
      last = NULL;	//Was last item in sub-item
      }
    }
  return NULL;
  }

Object *Object::Owner() const {
  Object *owner = parent;
  while(owner && (!owner->Attribute(1))) owner = owner->Parent();
  return owner;
  }

int Object::Quantity() const {
  if(!HasSkill("Quantity")) return 1;
  return Skill("Quantity");
  }

Object *new_obj() {
  return new Object();
  }

Object *new_obj(Object *o) {
  return new Object(o);
  }

Object *new_obj(const Object &o) {
  return new Object(o);
  }

