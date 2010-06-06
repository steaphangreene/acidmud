#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <cctype>
#include <cmath>
#include <fcntl.h>

using namespace std;

#include "utils.h"
#include "commands.h"
#include "object.h"
#include "color.h"
#include "mind.h"

const char *dirname[6] = { "north", "east", "south", "west", "up", "down" };

static char buf[65536];
void Object::CircleLoadAll() {
  FILE *mud = fopen("circle/wld/index", "r");
  if(mud) {
    sprintf(buf, "circle/wld/%c", 0);
    memset(buf+strlen(buf), 0, 256);
    fscanf(mud, "%[^\n]\n", buf+strlen(buf));
    while(strlen(buf) > 13) {
      CircleLoad(buf);
      sprintf(buf, "circle/wld/%c", 0);
      memset(buf+strlen(buf), 0, 256);
      fscanf(mud, "%[^\n]\n", buf+strlen(buf));
      }
    fclose(mud);
    }
  FILE *mudo = fopen("circle/obj/index", "r");
  if(mudo) {
    sprintf(buf, "circle/obj/%c", 0);
    memset(buf+strlen(buf), 0, 256);
    fscanf(mudo, "%[^\n]\n", buf+strlen(buf));
    while(strlen(buf) > 13) {
      CircleLoadObj(buf);
      sprintf(buf, "circle/obj/%c", 0);
      memset(buf+strlen(buf), 0, 256);
      fscanf(mudo, "%[^\n]\n", buf+strlen(buf));
      }
    fclose(mudo);
    }
  FILE *mudm = fopen("circle/mob/index", "r");
  if(mudm) {
    sprintf(buf, "circle/mob/%c", 0);
    memset(buf+strlen(buf), 0, 256);
    fscanf(mudm, "%[^\n]\n", buf+strlen(buf));
    while(strlen(buf) > 13) {
      CircleLoadMob(buf);
      sprintf(buf, "circle/mob/%c", 0);
      memset(buf+strlen(buf), 0, 256);
      fscanf(mudm, "%[^\n]\n", buf+strlen(buf));
      }
    fclose(mudm);
    }
  FILE *mudz = fopen("circle/zon/index", "r");
  if(mudz) {
    sprintf(buf, "circle/zon/%c", 0);
    memset(buf+strlen(buf), 0, 256);
    fscanf(mudz, "%[^\n]\n", buf+strlen(buf));
    while(strlen(buf) > 13) {
      CircleLoadZon(buf);
      sprintf(buf, "circle/zon/%c", 0);
      memset(buf+strlen(buf), 0, 256);
      fscanf(mudz, "%[^\n]\n", buf+strlen(buf));
      }
    fclose(mudz);
    }
  FILE *muds = fopen("circle/shp/index", "r");
  if(muds) {
    sprintf(buf, "circle/shp/%c", 0);
    memset(buf+strlen(buf), 0, 256);
    fscanf(muds, "%[^\n]\n", buf+strlen(buf));
    while(strlen(buf) > 13) {
      CircleLoadShp(buf);
      sprintf(buf, "circle/shp/%c", 0);
      memset(buf+strlen(buf), 0, 256);
      fscanf(muds, "%[^\n]\n", buf+strlen(buf));
      }
    fclose(muds);
    }
  CircleCleanup();
  }

static map<int,Object*> bynum;
static map<int,Object*> bynumobj;
static map<int,Object*> bynummob;
static map<int,Object*> bynummobinst;
static map<Object*,int> tonum[6];
static map<Object*,int> tynum[6];
static map<Object*,int> knum[6];
static map<Object*,string> nmnum[6];
static vector<Object*> olist;

void Object::CircleCleanup() {
  map<int,Object*>::iterator ind;
  for(ind = bynumobj.begin(); ind != bynumobj.end(); ++ind) delete((*ind).second);
  for(ind = bynummob.begin(); ind != bynummob.end(); ++ind) delete((*ind).second);

  bynum.clear();
  bynumobj.clear();
  bynummob.clear();
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
  }

static Mind *circle_mob_mind = NULL;

static Object *gold = NULL;
static void init_gold() {
  gold = new Object();
  gold->SetShortDesc("a gold piece");
  gold->SetDesc("A standard one-ounce gold piece.");
  gold->SetWeight(454 / 16);
  gold->SetVolume(0);
  gold->SetValue(1);
  gold->SetSize(0);
  gold->SetPos(POS_LIE);
  }

Mind *get_circle_mob_mind() {
  if(!circle_mob_mind) {
    circle_mob_mind = new Mind();
    circle_mob_mind->SetCircleMob();
    }
  return circle_mob_mind;
  }

Object *dup_circle_obj(Object *obj) {
  Object *obj2 = NULL;
  if(obj->Skill("Wearable on Left Hand")
    	!= obj->Skill("Wearable on Right Hand")) {
    obj2 = new Object(*obj);
    obj2->SetSkill("Wearable on Left Hand", 0);
    obj2->SetSkill("Wearable on Right Hand", 1);
    obj->SetShortDesc((string(obj->ShortDesc()) + " (left)").c_str());
    obj2->SetShortDesc((string(obj2->ShortDesc()) + " (right)").c_str());
//    fprintf(stderr, "Duped: '%s'\n", obj2->ShortDesc());
    }
  else if(obj->Skill("Wearable on Left Foot")
    	!= obj->Skill("Wearable on Right Foot")) {
    obj2 = new Object(*obj);
    obj2->SetSkill("Wearable on Left Foot", 0);
    obj2->SetSkill("Wearable on Right Foot", 1);
    obj->SetShortDesc((string(obj->ShortDesc()) + " (left)").c_str());
    obj2->SetShortDesc((string(obj2->ShortDesc()) + " (right)").c_str());
//    fprintf(stderr, "Duped: '%s'\n", obj2->ShortDesc());
    }
  else if(obj->Skill("Wearable on Left Leg")
    	!= obj->Skill("Wearable on Right Leg")) {
    obj2 = new Object(*obj);
//    fprintf(stderr, "Duped: '%s'\n", obj2->ShortDesc());
    }
  else if(obj->Skill("Wearable on Left Arm")
    	!= obj->Skill("Wearable on Right Arm")) {
    obj2 = new Object(*obj);
//    fprintf(stderr, "Duped: '%s'\n", obj2->ShortDesc());
    }
  return obj2;
  }

void Object::CircleFinishMob(Object *mob) {
  if(mob->Skill("CircleGold")) {
    Object *bag = new Object;

    bag->SetParent(mob);
    bag->SetShortDesc("a CircleMUD purse");
    bag->SetDesc("A mysterious purse that didn't seem to need to exist before.");

    bag->SetSkill("Wearable on Left Hip", 1);
    bag->SetSkill("Wearable on Right Hip", 2);
    bag->SetSkill("Container", 5 * 454);
    bag->SetSkill("Capacity", 5);
    bag->SetSkill("Closeable", 1);

    bag->SetWeight(1 * 454);
    bag->SetSize(2);
    bag->SetVolume(1);
    bag->SetValue(100);

    bag->SetPos(POS_LIE);
    mob->AddAct(ACT_WEAR_RHIP, bag);

    if(!gold) init_gold();
    Object *g = new Object(*gold);
    g->SetParent(bag);
    g->SetSkill("Quantity", mob->Skill("CircleGold"));
    mob->SetSkill("CircleGold", 0);
    }

  if(mob->ActTarg(ACT_WEAR_LSHOULDER)) { //CircleMUD Bags Only
    mob->ActTarg(ACT_WEAR_LSHOULDER)->SetSkill("Container",
	mob->ActTarg(ACT_WEAR_LSHOULDER)->ContainedWeight());
    mob->ActTarg(ACT_WEAR_LSHOULDER)->SetSkill("Capacity",
	mob->ActTarg(ACT_WEAR_LSHOULDER)->ContainedVolume());
    }

  if(mob->Skill("CircleAttack")) {
    if(mob->IsAct(ACT_WIELD)) {
      mob->SetSkill(
	get_weapon_skill(mob->ActTarg(ACT_WIELD)->Skill("WeaponType")),
	mob->Skill("CircleAttack")
	);
      }
    else {
      mob->SetSkill("Punching", mob->Skill("CircleAttack"));
      }
    mob->SetSkill("CircleAttack", 0);
    }
  if(mob->Skill("CircleDefense")) {
    if(mob->IsAct(ACT_WEAR_SHIELD)) {
      mob->SetSkill("Shields", mob->Skill("CircleDefense"));
      }
    else if(mob->Skill("Punching")) {
      mob->SetSkill("Kicking", mob->Skill("CircleDefense"));
      }
    else {
      mob->SetSkill("Kicking", mob->Skill("CircleDefense") / 2);
      mob->SetSkill("Punching", 
	mob->Skill("CircleDefense") - mob->Skill("Kicking"));
      }
    mob->SetSkill("CircleDefense", 0);
    }
  }

static Object *lastmob = NULL, *lastbag = NULL;
static map<int, Object*>lastobj;
void Object::CircleLoadZon(const char *fn) {
  FILE *mudz = fopen(fn, "r");
  if(mudz) {
    //fprintf(stderr, "Loading Circle Zone from \"%s\"\n", fn);
    for(int ctr=0; ctr<3; ++ctr) {
      fscanf(mudz, "%*[^\n]\n");
      }
    int done = 0;
    while(!done) {
      char type;
      fscanf(mudz, " %c", &type);
      //fprintf(stderr, "Processing %c zone directive.\n", type);
      switch(type) {
	case('S'): {
	  done = 1;
	  } break;
	case('D'): {	// Door state
	  int dnum, room, state;
	  fscanf(mudz, " %*d %d %d %d\n", &room, &dnum, &state);
	  Object *door = NULL;
	  if(bynum[room])
	    door = bynum[room]->PickObject(dirname[dnum], LOC_INTERNAL);
	  if(door && state == 0) {
	    door->SetSkill("Open", 1);
	    door->SetSkill("Locked", 0);
	    }
	  else if(door && state == 1) {
	    door->SetSkill("Open", 0);
	    door->SetSkill("Locked", 0);
	    }
	  else if(door && state == 2) {
	    door->SetSkill("Open", 0);
	    door->SetSkill("Locked", 1);
	    }
	  } break;
	case('M'): {
	  int num, room;
	  fscanf(mudz, " %*d %d %*d %d %*[^\n]\n", &num, &room);
	  if(bynum.count(room) && bynummob.count(num)) {
	    Object *obj = new Object;
	    obj->SetParent(bynum[room]);
	    obj->SetShortDesc("a CircleMUD MOB Popper");
	    obj->SetDesc("This thing just pops out MOBs.");

	    //fprintf(stderr, "Put Mob \"%s\" in Room \"%s\"\n", obj->ShortDesc(), bynum[room]->ShortDesc());

	    if(lastmob) CircleFinishMob(lastmob);
	    lastmob = new Object(*(bynummob[num]));
	    bynummobinst[num] = lastmob;
	    lastmob->SetParent(obj);
	    lastmob->AddAct(ACT_SPECIAL_MASTER, obj);
	    obj->SetSkill("CirclePopper", 1);
	    obj->AddAct(ACT_SPECIAL_PREPARE, lastmob);
	    obj->AddAct(ACT_SPECIAL_NOTSHOWN);
	    obj->Activate();
	    lastbag = NULL;
	    }
	  } break;
	case('O'): {
	  int num, room;
	  fscanf(mudz, " %*d %d %*d %d %*[^\n]\n", &num, &room);
	  if(bynum.count(room) && bynumobj.count(num)) {
	    Object *obj = new Object(*(bynumobj[num]));
	    obj->SetParent(bynum[room]);
	    //fprintf(stderr, "Put Obj \"%s\" in Room \"%s\"\n", obj->ShortDesc(), bynum[room]->ShortDesc());
	    lastobj[num] = obj;
	    }
	  } break;
	case('G'):
	case('E'): {
	  int num, pos = -1;
	  if(type == 'E') fscanf(mudz, " %*d %d %*d %d%*[^\n]\n", &num, &pos);
	  if(type == 'G') fscanf(mudz, " %*d %d %*d%*[^\n]\n", &num);
	  if(lastmob && bynumobj.count(num)) {
	    Object *obj = new Object(*(bynumobj[num]));
	    Object *obj2 = dup_circle_obj(obj);
	    obj->SetParent(lastmob);
	    if(obj2) obj2->SetParent(lastmob);
	    lastobj[num] = obj;

	    int bagit = 0;
	    switch(pos) {
	      case(1): { // Worn
		lastmob->AddAct(ACT_WEAR_RFINGER, obj); break;
		}
	      case(2): { // Worn
		lastmob->AddAct(ACT_WEAR_LFINGER, obj); break;
		}
	      case(3):		//Circle MOBs have two necks
	      case(4): { // Worn
		if(lastmob->IsAct(ACT_WEAR_NECK)) bagit = 1;
		else lastmob->AddAct(ACT_WEAR_NECK, obj);
		break;
		}
	      case(5): { // Worn
		lastmob->AddAct(ACT_WEAR_CHEST, obj); break;
		lastmob->AddAct(ACT_WEAR_BACK, obj); break;
		}
	      case(6): { // Worn
		lastmob->AddAct(ACT_WEAR_HEAD, obj); break;
		}
	      case(7): { // Worn
		lastmob->AddAct(ACT_WEAR_LLEG, obj);
		if(obj2) lastmob->AddAct(ACT_WEAR_RLEG, obj2);
		else lastmob->AddAct(ACT_WEAR_RLEG, obj);
		break;
		}
	      case(8): { // Worn
		lastmob->AddAct(ACT_WEAR_LFOOT, obj);
		if(obj2) lastmob->AddAct(ACT_WEAR_RFOOT, obj2);
		else lastmob->AddAct(ACT_WEAR_RFOOT, obj);
		break;
		}
	      case(9): { // Worn
		lastmob->AddAct(ACT_WEAR_LHAND, obj);
		if(obj2) lastmob->AddAct(ACT_WEAR_RHAND, obj2);
		else lastmob->AddAct(ACT_WEAR_RHAND, obj);
		break;
		}
	      case(10): { // Worn
		lastmob->AddAct(ACT_WEAR_LARM, obj);
		if(obj2) lastmob->AddAct(ACT_WEAR_RARM, obj2);
		else lastmob->AddAct(ACT_WEAR_RARM, obj);
		break;
		}
	      case(11): { // Worn
		lastmob->AddAct(ACT_WEAR_SHIELD, obj); break;
		}
	      case(12): { // Worn
		lastmob->AddAct(ACT_WEAR_BACK, obj); break;
		}
	      case(13): { // Worn
		lastmob->AddAct(ACT_WEAR_WAIST, obj); break;
		}
	      case(14): { // Worn
		lastmob->AddAct(ACT_WEAR_RWRIST, obj); break;
		}
	      case(15): { // Worn
		lastmob->AddAct(ACT_WEAR_LWRIST, obj); break;
		}
	      case(16): { // Wielded
		lastmob->AddAct(ACT_WIELD, obj); break;
		}
	      case(17): { // Held
		lastmob->AddAct(ACT_HOLD, obj); break;
		}
	      default: {
		bagit = 1;
		}break;
	      }
	    if(bagit) {
	      if(!lastbag) {
		lastbag = new Object;
		lastbag->SetParent(lastmob);
		lastbag->SetShortDesc("a CircleMUD pack");
		lastbag->SetDesc("A mysterious pack that didn't seem to need to exist before.");

		lastbag->SetSkill("Wearable on Left Shoulder", 1);
		lastbag->SetSkill("Wearable on Right Shoulder", 2);
		lastbag->SetSkill("Container", 1000 * 454);
		lastbag->SetSkill("Capacity", 1000);
		lastbag->SetSkill("Closeable", 1);

		lastbag->SetWeight(5 * 454);
		lastbag->SetSize(1000);
		lastbag->SetVolume(5);
		lastbag->SetValue(200);

		lastbag->SetPos(POS_LIE);
		lastmob->AddAct(ACT_WEAR_LSHOULDER, lastbag);
		}
	      obj->Travel(lastbag);
	      if(obj2) obj2->Travel(lastbag);
	      }
	    }
	  } break;
	case('P'): {
	  int num, innum;
	  fscanf(mudz, " %*d %d %*d %d %*[^\n]\n", &num, &innum);
	  if(lastobj.count(innum) && bynumobj.count(num)) {
	    Object *obj = new Object(*(bynumobj[num]));
	    Object *obj2 = dup_circle_obj(obj);
	    obj->SetParent(lastobj[innum]);
	    if(obj2) obj2->SetParent(lastobj[innum]);
	    //fprintf(stderr, "Put Obj \"%s\" in Obj \"%s\"\n", obj->ShortDesc(), lastobj[innum]->ShortDesc());
	    lastobj[num] = obj;
	    }
	  } break;
	default: {
	  fscanf(mudz, "%*[^\n]\n");
	  } break;
	}
      }
    if(lastmob) CircleFinishMob(lastmob);
    fclose(mudz);
    }
  }


void Object::CircleLoadMob(const char *fn) {
  FILE *mudm = fopen(fn, "r");
  if(mudm) {
    //fprintf(stderr, "Loading Circle Mobiles from \"%s\"\n", fn);
    while(1) {
      int onum;
      if(fscanf(mudm, " #%d\n", &onum) < 1) break;

      Object *obj = new Object(this);
      bynummob[onum] = obj;

      //FIXME: Nicknames should not be ignored!!!
      memset(buf, 0, 65536);
      fscanf(mudm, "%[^~\n]~\n", buf);

      memset(buf, 0, 65536);
      fscanf(mudm, "%[^~\n]~\n", buf);
      for(char *ch=buf; (*ch); ++ch) if((*ch) == ';') (*ch) = ',';
      obj->SetShortDesc(buf);
      //fprintf(stderr, "Loaded Circle Mobile with Name = %s\n", buf);

      memset(buf, 0, 65536);
      if(!fscanf(mudm, "%[^~]~\n", buf)) fscanf(mudm, "%*[^\n]\n");
      else {
        for(char *ch=buf; (*ch); ++ch) if((*ch) == ';') (*ch) = ',';
	obj->SetDesc(buf);
	}
      //fprintf(stderr, "Loaded Circle Mobile with Desc = %s\n", buf);

      memset(buf, 0, 65536);
      if(!fscanf(mudm, "%[^~]~\n", buf)) fscanf(mudm, "%*[^\n]\n");
      else {
        for(char *ch=buf; (*ch); ++ch) if((*ch) == ';') (*ch) = ',';
	obj->SetLongDesc(buf);
	}
      //fprintf(stderr, "Loaded Circle Mobile with LongDesc = %s\n", buf);

      obj->SetPos(POS_STAND);
      obj->SetAttribute(0, 3);
      obj->SetAttribute(1, 3);
      obj->SetAttribute(2, 3);
      obj->SetAttribute(3, 3);
      obj->SetAttribute(4, 3);
      obj->SetAttribute(5, 3);

      int aware = 0, hidden = 0, sneak = 0;
      int val, val2, val3;  char tp;
      memset(buf, 0, 65536);
      fscanf(mudm, "%[^ \t\n]", buf); //Rest of line read below...

      obj->SetSkill("CircleAction", 8); //IS_NPC - I'll use it to see if(MOB)
      if(string(buf).find('b') < strlen(buf) || (atoi(buf) & 2)) { //SENTINEL
	obj->SetSkill("CircleAction", obj->Skill("CircleAction") | 2);
	}
      if(string(buf).find('c') < strlen(buf) || (atoi(buf) & 4)) { //SCAVENGER
	obj->SetSkill("CircleAction", obj->Skill("CircleAction") | 4);
	}
      if(string(buf).find('e') < strlen(buf) || (atoi(buf) & 16)) { //AWARE
	aware = 1;
	}
      if(string(buf).find('f') < strlen(buf) || (atoi(buf) & 32)) { //AGGRESSIVE
	obj->SetSkill("CircleAction", obj->Skill("CircleAction") | 32);
	}
      if(string(buf).find('g') < strlen(buf) || (atoi(buf) & 64)) { //STAY_ZONE
	obj->SetSkill("CircleAction", obj->Skill("CircleAction") | 64);
	}
      if(string(buf).find('h') < strlen(buf) || (atoi(buf) & 128)) { //WIMPY
	obj->SetSkill("CircleAction", obj->Skill("CircleAction") | 128);
	}
      if(string(buf).find('l') < strlen(buf) || (atoi(buf) & 2048)) { //MEMORY
	obj->SetSkill("CircleAction", obj->Skill("CircleAction") | 2048);
	}
      if(string(buf).find('m') < strlen(buf) || (atoi(buf) & 4096)) { //HELPER
	obj->SetSkill("CircleAction", obj->Skill("CircleAction") | 4096);
	}
      //FIXME: Add others here.

      memset(buf, 0, 65536);
      fscanf(mudm, " %[^ \t\n]", buf); //Rest of line read below...
      if(string(buf).find('g') < strlen(buf) || (atoi(buf) & 64)) { //WATERWALK
	obj->SetSkill("CircleAffection", obj->Skill("CircleAffection") | 64);
	}
      if(string(buf).find('s') < strlen(buf) || (atoi(buf) & 262144)) { //SNEAK
	sneak = 1;
	}
      if(string(buf).find('t') < strlen(buf) || (atoi(buf) & 524288)) { //HIDE
	hidden = 1;
	}

      memset(buf, 0, 65536);
      fscanf(mudm, " %d %c\n", &val, &tp);
      //FIXME: Implement special powers of MOBs here.

      obj->SetSkill("Accomplishment", 120000+onum);

      if(tp == 'E' || tp == 'S') {
	fscanf(mudm, "%d %d %d", &val, &val2, &val3);
	for(int ctr=0; ctr<val; ++ctr)
	  obj->SetAttribute(ctr%6, obj->Attribute(ctr%6)+1);	// val1 = Level
	obj->SetSkill("CircleAttack", (20 - val2) / 3);		// val2 = THAC0
	obj->SetSkill("CircleDefense", (10 - val3) / 3);	// val2 = AC

	fscanf(mudm, " %dd%d+%d", &val, &val2, &val3);		// Hit Points
	val = (val*(val2+1) + 1) / 2 + val3;
	obj->SetAttribute(0, (val+49)/50);			// Becomes Body

	fscanf(mudm, " %dd%d+%d\n", &val, &val2, &val3);// Barehand Damage
	val = (val*(val2+1) + 1) / 2 + val3;
	obj->SetAttribute(2, (val+2)/3);		// Becomes Strength

	if(aware) //Perception = Int
	  obj->SetSkill("Perception", obj->Attribute(4));

	if(sneak && hidden)		//Stealth = 3Q/2
	  obj->SetSkill("Stealth", (3*obj->Attribute(1)+1)/2);
	else if(hidden)			//Stealth = Q
	  obj->SetSkill("Stealth", obj->Attribute(1));
	else if(sneak)			//Stealth = Q/2
	  obj->SetSkill("Stealth", (obj->Attribute(1)+1)/2);

	fscanf(mudm, "%d", &val);  // Gold
	obj->SetSkill("CircleGold", val);

	fscanf(mudm, "%*[^\n]\n"); // XP //FIXME: Worth Karma?

	fscanf(mudm, "%d %d %d\n", &val, &val2, &val3);

	if(val == 4) { // Mob Starts off Sleeping
	  obj->SetPos(POS_LIE);
	  obj->AddAct(ACT_SLEEP);
	  }
	else if(val == 5) { // Mob Starts off Resting
	  obj->SetPos(POS_SIT);
	  obj->AddAct(ACT_REST);
	  }
	else if(val == 6) { // Mob Starts off Sitting
	  obj->SetPos(POS_SIT);
	  }

	static char genderlist[] = { 'N', 'M', 'F' };
	obj->gender = genderlist[val3];
	}

      memset(buf, 0, 65536);
      while(tp == 'E') {  // Basically an if with an infinite loop ;)
	if(fscanf(mudm, "Con: %d\n", &val))
	  obj->SetAttribute(0, MAX(obj->Attribute(0), ((val+2)/3)));

	else if(fscanf(mudm, "Dex: %d\n", &val))
	  obj->SetAttribute(1, MAX(obj->Attribute(1), ((val+2)/3)));

	else if(fscanf(mudm, "Str: %d\n", &val))
	  obj->SetAttribute(2, MAX(obj->Attribute(2), ((val+2)/3)));

	else if(fscanf(mudm, "ha: %d\n", &val)) //'Cha' minus 'Con' Conflict!
	  obj->SetAttribute(3, MAX(obj->Attribute(3), ((val+2)/3)));

	else if(fscanf(mudm, "Int: %d\n", &val))
	  obj->SetAttribute(4, MAX(obj->Attribute(4), ((val+2)/3)));

	else if(fscanf(mudm, "Wis: %d\n", &val))
	  obj->SetAttribute(5, MAX(obj->Attribute(5), ((val+2)/3)));

	else if(fscanf(mudm, "Add: %d\n", &val)); //'StrAdd' - Do Nothing

	else if(fscanf(mudm, "BareHandAttack: %d\n", &val)) {
	  //FIXME: Add Teeth/claws/trample/etc...
	  }
	else break;
	}

      obj->SetWeight(obj->Attribute(0) * 20000);
      obj->SetSize(1000 + obj->Attribute(0) * 200);
      obj->SetVolume(100);
      obj->SetValue(-1);

      fscanf(mudm, " %*[^#$]");
      }
    fclose(mudm);
    }
  }

void Object::CircleLoadObj(const char *fn) {
  FILE *mudo = fopen(fn, "r");
  if(mudo) {
    //fprintf(stderr, "Loading Circle Objects from \"%s\"\n", fn);
    while(1) {
      int onum;
      int valmod = 1000;
      if(fscanf(mudo, " #%d\n", &onum) < 1) break;

      Object *obj = new Object(this);
      bynumobj[onum] = obj;

      memset(buf, 0, 65536);
      fscanf(mudo, "%[^~\n]~\n", buf);

      memset(buf, 0, 65536);
      fscanf(mudo, "%[^~\n]~\n", buf);
      for(char *ch=buf; (*ch); ++ch) if((*ch) == ';') (*ch) = ',';
      obj->SetShortDesc(buf);
      //fprintf(stderr, "Loaded Circle Object with Name = %s\n", buf);

      memset(buf, 0, 65536);
      if(!fscanf(mudo, "%[^~]~\n", buf)) fscanf(mudo, "%*[^\n]\n");
      else {
	for(char *ch=buf; (*ch); ++ch) if((*ch) == ';') (*ch) = ',';
	obj->SetDesc(buf);
	}
      //fprintf(stderr, "Loaded Circle Object with Desc = %s\n", buf);

      fscanf(mudo, "%*[^\n]\n");

      int tp=0, val[4];
      memset(buf, 0, 65536);
      fscanf(mudo, "%d %[^ \n\t] %[^ \n\t]%*[^\n]\n", &tp, buf+256, buf);
      if(string(buf).find('a') < strlen(buf) || (atoi(buf) & 1)) {
	obj->SetPos(POS_LIE);
	}

      if(tp == 1) { //LIGHTS - Their circle stats are all fuxxed up.
	buf[0] = '0';  buf[1] = 0;
        }

      int sf = 0;
      if(!strncasecmp(obj->ShortDesc(), "a pair of ", 10)) sf = 9;
      else if(!strncasecmp(obj->ShortDesc(), "some ", 5)) sf = 4;
      else if(!strncasecmp(obj->ShortDesc(), "a set of ", 9)) sf = 8;

      string name = obj->ShortDesc();
      if(string(buf).find('b') < strlen(buf) || (atoi(buf) & 2)) {
	obj->SetSkill("Wearable on Left Finger", 1);	//Two Alternatives
	obj->SetSkill("Wearable on Right Finger", 2);
	}
      if(string(buf).find('c') < strlen(buf) || (atoi(buf) & 4)) {
	obj->SetSkill("Wearable on Neck", 1);
	}
      if(string(buf).find('d') < strlen(buf) || (atoi(buf) & 8)) {
	obj->SetSkill("Wearable on Chest", 1);
	obj->SetSkill("Wearable on Back", 1);
	if(matches(name.c_str(), "suit of")) {
	  obj->SetSkill("Wearable on Right Leg", 1);
	  obj->SetSkill("Wearable on Left Leg", 1);
	  obj->SetSkill("Wearable on Right Arm", 1);
	  obj->SetSkill("Wearable on Left Arm", 1);
	  valmod *= 5;
	  }
	}
      if(string(buf).find('e') < strlen(buf) || (atoi(buf) & 16)) {
	obj->SetSkill("Wearable on Head", 1);
	}
      if(string(buf).find('f') < strlen(buf) || (atoi(buf) & 32)) {
	obj->SetSkill("Wearable on Left Leg", 1);
	if(sf) {
	  if(!strcasecmp(name.c_str()+(name.length()-9), " leggings"))
	    name = string("a") + name.substr(sf, name.length()-(sf+1));
	  else if(!strcasecmp(name.c_str()+(name.length()-7), " plates"))
	    name = string("a") + name.substr(sf, name.length()-(sf+1));
	  else obj->SetSkill("Wearable on Right Leg", 1);
	  }
	else obj->SetSkill("Wearable on Right Leg", 1);
	if(!obj->Skill("Wearable on Right Leg")) {	//Reversable
	  obj->SetSkill("Wearable on Right Leg", 2);
	  valmod /= 2;
	  }
	}
      if(string(buf).find('g') < strlen(buf) || (atoi(buf) & 64)) {
	obj->SetSkill("Wearable on Left Foot", 1);
	if(sf) {
	  if(!strcasecmp(name.c_str()+(name.length()-8), " sandals"))
	    name = string("a") + name.substr(sf, name.length()-(sf+1));
	  else if(!strcasecmp(name.c_str()+(name.length()-6), " boots"))
	    name = string("a") + name.substr(sf, name.length()-(sf+1));
	  else if(!strcasecmp(name.c_str()+(name.length()-6), " shoes"))
	    name = string("a") + name.substr(sf, name.length()-(sf+1));
	  else obj->SetSkill("Wearable on Right Foot", 1);
	  }
	else obj->SetSkill("Wearable on Right Foot", 1);
	if(!obj->Skill("Wearable on Right Foot")) {
	  valmod /= 2;
	  }
	}
      if(string(buf).find('h') < strlen(buf) || (atoi(buf) & 128)) {
	obj->SetSkill("Wearable on Left Hand", 1);
	if(sf) {
	  if(!strcasecmp(name.c_str()+(name.length()-10), " gauntlets"))
	    name = string("a") + name.substr(sf, name.length()-(sf+1));
	  else if(!strcasecmp(name.c_str()+(name.length()-7), " gloves"))
	    name = string("a") + name.substr(sf, name.length()-(sf+1));
	  else obj->SetSkill("Wearable on Right Hand", 1);
	  }
	else obj->SetSkill("Wearable on Right Hand", 1);
	if(!obj->Skill("Wearable on Right Hand")) {
	  valmod /= 2;
	  }
	}
      if(string(buf).find('i') < strlen(buf) || (atoi(buf) & 256)) {
	obj->SetSkill("Wearable on Left Arm", 1);
	if(sf) {
	  if(!strcasecmp(name.c_str()+(name.length()-8), " sleeves"))
	    name = string("a") + name.substr(sf, name.length()-(sf+1));
	  else if(!strcasecmp(name.c_str()+(name.length()-8), " bracers"))
	    name = string("a") + name.substr(sf, name.length()-(sf+1));
	  else if(!strcasecmp(name.c_str()+(name.length()-7), " plates"))
	    name = string("a") + name.substr(sf, name.length()-(sf+1));
	  else obj->SetSkill("Wearable on Right Arm", 1);
	  }
	else obj->SetSkill("Wearable on Right Arm", 1);
	if(!obj->Skill("Wearable on Right Arm")) {	//Reversable
	  obj->SetSkill("Wearable on Right Arm", 2);
	  valmod /= 2;
	  }
	}
      if(string(buf).find('j') < strlen(buf) || (atoi(buf) & 512)) {
	obj->SetSkill("Wearable on Shield", 1);	// FIXME: Wear Shield?
	}
      if(string(buf).find('k') < strlen(buf) || (atoi(buf) & 1024)) {
	obj->SetSkill("Wearable on Back", 1);		// "WEAR_ABOUT"
	obj->SetSkill("Wearable on Left Shoulder", 2);
	obj->SetSkill("Wearable on Right Shoulder", 2);
	}
      if(string(buf).find('l') < strlen(buf) || (atoi(buf) & 2048)) {
	obj->SetSkill("Wearable on Waist", 1);
	}
      if(string(buf).find('m') < strlen(buf) || (atoi(buf) & 4096)) {
	obj->SetSkill("Wearable on Left Wrist", 1);
	obj->SetSkill("Wearable on Right Wrist", 2);
	}
      obj->SetShortDesc(name.c_str());

      fscanf(mudo, "%d %d %d %d\n", val+0, val+1, val+2, val+3);

      if(tp == 9) { // ARMOR
	obj->SetAttribute(0, val[0]);
	}
      else if(tp == 20) { // MONEY
	if(!gold) init_gold();
	(*obj) = (*gold);
	obj->SetSkill("Quantity", val[0]);
	}
      else if(tp == 18) { // KEY
	obj->SetSkill("Key", 100000 + onum);		// Key's "code"
	}
      else if(tp == 15) { // CONTAINER
	obj->SetSkill("Container", val[0] * 454);
	obj->SetSkill("Capacity", val[0]);

	if(!(val[1] & 4)) obj->SetSkill("Open", 1);	//Start open?
	if(val[1] & 8) {
	  obj->SetSkill("Locked", 1);			//Start locked?
	  obj->SetSkill("Lockable", 1);			// Can it be locked?
	  }
	if(val[1] & 1) obj->SetSkill("Closeable", 1);	// Can it be closed?
	if(val[2] > 0) {
	  obj->SetSkill("Lock", 100000 + val[2]);	// Unlocking key's code
	  obj->SetSkill("Lockable", 1);			// Can it be locked?
	  }

	if(string(obj->ShortDesc()).find("bag") < strlen(obj->ShortDesc())) {
	  obj->SetSkill("Closeable", 1);		// Bags CAN be closed
	  obj->SetSkill("Wearable on Left Hip", 1);	// Bags CAN be belted
	  obj->SetSkill("Wearable on Right Hip", 2);
	  }

	if(string(obj->ShortDesc()).find("pouch") < strlen(obj->ShortDesc())) {
	  obj->SetSkill("Closeable", 1);		// Pouches CAN be closed
	  obj->SetSkill("Wearable on Left Hip", 1);	// Pouches CAN be belted
	  obj->SetSkill("Wearable on Right Hip", 2);
	  }

	}
      else if(tp == 10) { // POTION
	obj->SetSkill("Liquid Container", 1);
	obj->SetSkill("Capacity", 1);
	obj->SetSkill("Closeable", 1);

	Object *liq = new Object(obj);
	liq->SetSkill("Liquid", 1);
	liq->SetSkill("Ingestible", 1);
	liq->SetWeight(10);
	liq->SetVolume(1);
	liq->SetSkill("Quantity", 1);
	liq->SetSkill("Magical", val[0]);
	for(int idx=1; idx < 4; ++idx) {
	  switch(val[idx]) {
	    case(16): {		// CURE_LIGHT
	      liq->SetShortDesc("white");
	      liq->SetSkill("Heal Effect", 6);
	      } break;
	    case(15): {		// CURE_CRITIC
	      liq->SetShortDesc("blue");
	      liq->SetSkill("Heal Effect", 4);
	      } break;
	    case(28): {		// HEAL
	      liq->SetShortDesc("black");
	      liq->SetSkill("Heal Effect", 2);
	      } break;
	    case(33): {		// POISON
	      liq->SetShortDesc("red");
	      liq->SetSkill("Poisonous", val[0]);
	      } break;
	    case(43): {		// REMOVE_POISON
	      liq->SetShortDesc("silver");
	      liq->SetSkill("Cure Effect", 2);
	      } break;
	    }
	  }
	}
      else if(tp == 17 || tp == 23) { // DRINKCON/FOUNTAIN
	obj->SetSkill("Liquid Container", val[0]);
	obj->SetSkill("Capacity", val[0]);
	obj->SetSkill("Closeable", 1);
	if(val[1] > 0) {
	  Object *liq = new Object(obj);
	  liq->SetSkill("Liquid", 1);
	  liq->SetSkill("Ingestible", 1);
	  liq->SetWeight(20);
	  liq->SetVolume(2);
	  switch(val[2]) {
	    case(0): {		// WATER
	      liq->SetShortDesc("water");
	      liq->SetSkill("Drink", 10);
	      liq->SetSkill("Food", 1);
	      //liq->SetSkill("Alcohol", 0);
	      } break;
	    case(1): {		// BEER
	      liq->SetShortDesc("beer");
	      liq->SetSkill("Drink", 5);
	      liq->SetSkill("Food", 2);
	      liq->SetSkill("Alcohol", 3);
	      liq->SetSkill("Perishable", val[0] * 32);
	      } break;
	    case(2): {		// WINE
	      liq->SetShortDesc("wine");
	      liq->SetSkill("Drink", 5);
	      liq->SetSkill("Food", 2);
	      liq->SetSkill("Alcohol", 5);
	      } break;
	    case(3): {		// ALE
	      liq->SetShortDesc("ale");
	      liq->SetSkill("Drink", 5);
	      liq->SetSkill("Food", 2);
	      liq->SetSkill("Alcohol", 2);
	      liq->SetSkill("Perishable", val[0] * 16);
	      } break;
	    case(4): {		// DARKALE
	      liq->SetShortDesc("dark ale");
	      liq->SetSkill("Drink", 5);
	      liq->SetSkill("Food", 2);
	      liq->SetSkill("Alcohol", 1);
	      liq->SetSkill("Perishable", val[0] * 8);
	      } break;
	    case(5): {		// WHISKY
	      liq->SetShortDesc("whisky");
	      liq->SetSkill("Drink", 4);
	      liq->SetSkill("Food", 1);
	      liq->SetSkill("Alcohol", 6);
	      } break;
	    case(6): {		// LEMONADE
	      liq->SetShortDesc("lemonaid");
	      liq->SetSkill("Drink", 8);
	      liq->SetSkill("Food", 1);
	      //liq->SetSkill("Alcohol", 0);
	      liq->SetSkill("Perishable", val[0] * 4);
	      } break;
	    case(7): {		// FIREBRT
	      liq->SetShortDesc("firebreather");
	      //liq->SetSkill("Drink", 0);
	      //liq->SetSkill("Food", 0);
	      liq->SetSkill("Alcohol", 10);
	      } break;
	    case(8): {		// LOCALSPC
	      liq->SetShortDesc("local brew");
	      liq->SetSkill("Drink", 3);
	      liq->SetSkill("Food", 3);
	      liq->SetSkill("Alcohol", 3);
	      } break;
	    case(9): {		// SLIME
	      liq->SetShortDesc("slime");
	      liq->SetSkill("Drink", -8);
	      liq->SetSkill("Food", 4);
	      //liq->SetSkill("Alcohol", 0);
	      } break;
	    case(10): {		// MILK
	      liq->SetShortDesc("milk");
	      liq->SetSkill("Drink", 6);
	      liq->SetSkill("Food", 3);
	      //liq->SetSkill("Alcohol", 0);
	      liq->SetSkill("Perishable", val[0]);
	      } break;
	    case(11): {		// TEA
	      liq->SetShortDesc("tea");
	      liq->SetSkill("Drink", 6);
	      liq->SetSkill("Food", 1);
	      //liq->SetSkill("Alcohol", 0);
	      } break;
	    case(12): {		// COFFE
	      liq->SetShortDesc("coffee");
	      liq->SetSkill("Drink", 6);
	      liq->SetSkill("Food", 1);
	      //liq->SetSkill("Alcohol", 0);
	      } break;
	    case(13): {		// BLOOD
	      liq->SetShortDesc("blood");
	      liq->SetSkill("Drink", -1);
	      liq->SetSkill("Food", 2);
	      //liq->SetSkill("Alcohol", 0);
	      liq->SetSkill("Perishable", val[0] * 2);
	      } break;
	    case(14): {		// SALTWATER
	      liq->SetShortDesc("salt water");
	      liq->SetSkill("Drink", -2);
	      liq->SetSkill("Food", 1);
	      //liq->SetSkill("Alcohol", 0);
	      } break;
	    case(15): {		// CLEARWATER
	      liq->SetShortDesc("clear water");
	      liq->SetSkill("Drink", 13);
	      //liq->SetSkill("Food", 0);
	      //liq->SetSkill("Alcohol", 0);
	      } break;
	    }
	  if(val[3] != 0) {
	    liq->SetSkill("Poisionous", val[3]);
	    }
	  liq->SetSkill("Quantity", val[1]);
	  }
	}
      else if(tp == 19) { // FOOD
	obj->SetSkill("Ingestible", 1);
	obj->SetSkill("Perishable", val[0]);
	obj->SetSkill("Food", val[0]);
	if(val[3] != 0) {
	  obj->SetSkill("Poisionous", val[3]);
	  }
	}
      else if(tp == 22) { // BOAT
	obj->SetSkill("Enterable", 1);
	obj->SetSkill("Open", 1);
	obj->SetSkill("Vehicle", 4); //Unpowered (1=0), Calm Water (4=1).
	}
      else if(tp == 5) { // WEAPON
        int wreach = 0;						// default

        int skmatch = get_weapon_type("Long Crushing");		// default
	if(val[3] == 1)						// "stings"
		skmatch = get_weapon_type("Long Piercing");
	else if(val[3] == 2)					// "whips"
		skmatch = get_weapon_type("Whips");
	else if(val[3] == 3)					// "slashes"
		skmatch = get_weapon_type("Long Blades");
	else if(val[3] == 4)					// "bites"
		skmatch = get_weapon_type("Long Piercing");
	else if(val[3] == 8)					// "claws"
		skmatch = get_weapon_type("Long Blades");
	else if(val[3] == 9)					// "mauls"
		skmatch = get_weapon_type("Long Cleaves");
	else if(val[3] == 11)					// "pierces"
		skmatch = get_weapon_type("Long Piercing");
	else if(val[3] == 12)					// "blasts"
		skmatch = get_weapon_type("Shotguns");
	else if(val[3] == 13)					// "punches"
		skmatch = get_weapon_type("Punching");
	else if(val[3] == 14)					// "stabs"
		skmatch = get_weapon_type("Long Piercing");

	if(matches(obj->ShortDesc(), "lance")) {
	  skmatch = get_weapon_type("Two-Handed Piercing");
	  }
	else if(matches(obj->ShortDesc(), "spear")) {
	  skmatch = get_weapon_type("Two-Handed Piercing");
	  }
	else if(matches(obj->ShortDesc(), "bolg")) {
	  skmatch = get_weapon_type("Two-Handed Piercing");
	  }
	else if(matches(obj->ShortDesc(), "bulg")) {
	  skmatch = get_weapon_type("Two-Handed Piercing");
	  }
	else if(matches(obj->ShortDesc(), "pike")) {
	  skmatch = get_weapon_type("Two-Handed Piercing");
	  }
	else if(matches(obj->ShortDesc(), "trident")) {
	  skmatch = get_weapon_type("Two-Handed Piercing");
	  }
	else if(matches(obj->ShortDesc(), "sickle")) {
	  skmatch = get_weapon_type("Two-Handed Piercing");
	  }
	else if(matches(obj->ShortDesc(), "sap")) {
	  skmatch = get_weapon_type("Short Flails");
	  }
	else if(matches(obj->ShortDesc(), "flail")) {
	  skmatch = get_weapon_type("Long Flails");
	  }
	else if(matches(obj->ShortDesc(), "whip")) {
	  skmatch = get_weapon_type("Whips");
	  }
	else if(matches(obj->ShortDesc(), "crop")) {
	  skmatch = get_weapon_type("Whips");
	  }
	else if(matches(obj->ShortDesc(), "knife")) {
	  skmatch = get_weapon_type("Short Blades");
	  }
	else if(matches(obj->ShortDesc(), "shard")) {
	  skmatch = get_weapon_type("Short Blades");
	  }
	else if(matches(obj->ShortDesc(), "stake")) {
	  skmatch = get_weapon_type("Short Piercing");
	  }
	else if(matches(obj->ShortDesc(), "spike")) {
	  skmatch = get_weapon_type("Short Piercing");
	  }
	else if(matches(obj->ShortDesc(), "nail")) {
	  skmatch = get_weapon_type("Short Piercing");
	  }
	else if(matches(obj->ShortDesc(), "dagger")) {
	  skmatch = get_weapon_type("Short Piercing");
	  }
	else if(matches(obj->ShortDesc(), "kris")) {
	  skmatch = get_weapon_type("Short Piercing");
	  }
	else if(matches(obj->ShortDesc(), "chisel")) {
	  skmatch = get_weapon_type("Short Piercing");
	  }
	else if(matches(obj->ShortDesc(), "dirk")) {
	  skmatch = get_weapon_type("Short Piercing");
	  }
	else if(matches(obj->ShortDesc(), "rapier")) {
	  skmatch = get_weapon_type("Long Piercing");
	  }
	else if(matches(obj->ShortDesc(), "glaive")) {
	  skmatch = get_weapon_type("Two-Handed Blades");
	  }
	else if(matches(obj->ShortDesc(), "scimitar")) {
	  skmatch = get_weapon_type("Long Blades");
	  }
	else if(matches(obj->ShortDesc(), "katana")) {
	  skmatch = get_weapon_type("Long Blades");
	  }
	else if(matches(obj->ShortDesc(), "sword")) {
	  skmatch = get_weapon_type("Long Blades");
	  }
	else if(matches(obj->ShortDesc(), "cutlass")) {
	  skmatch = get_weapon_type("Long Blades");
	  }
	else if(matches(obj->ShortDesc(), "sabre")) {
	  skmatch = get_weapon_type("Long Blades");
	  }
	else if(matches(obj->ShortDesc(), "cleaver")) {
	  skmatch = get_weapon_type("Long Cleaves");
	  }
	else if(matches(obj->ShortDesc(), "pick")) {
	  skmatch = get_weapon_type("Long Cleaves");
	  }
	else if(matches(obj->ShortDesc(), "axe")) {
	  skmatch = get_weapon_type("Long Cleaves");
	  }
	else if(matches(obj->ShortDesc(), "club")) {
	  skmatch = get_weapon_type("Long Crushing");
	  }
	else if(matches(obj->ShortDesc(), "rod")) {
	  skmatch = get_weapon_type("Long Crushing");
	  }
	else if(matches(obj->ShortDesc(), "mace")) {
	  skmatch = get_weapon_type("Long Crushing");
	  }
	else if(matches(obj->ShortDesc(), "morning star")) {
	  skmatch = get_weapon_type("Long Crushing");
	  }
	else if(matches(obj->ShortDesc(), "cudgel")) {
	  skmatch = get_weapon_type("Long Crushing");
	  }
	else if(matches(obj->ShortDesc(), "bardiche")) {
	  skmatch = get_weapon_type("Two-Handed Cleaves");
	  }
	else if(matches(obj->ShortDesc(), "fauchard")) {
	  skmatch = get_weapon_type("Two-Handed Cleaves");
	  }
	else if(matches(obj->ShortDesc(), "bec de corbin")) {
	  skmatch = get_weapon_type("Two-Handed Cleaves");
	  }
	else if(matches(obj->ShortDesc(), "bill-gisarme")) {
	  skmatch = get_weapon_type("Two-Handed Cleaves");
	  }
	else if(matches(obj->ShortDesc(), "gisarme")) {
	  skmatch = get_weapon_type("Two-Handed Cleaves");
	  }
	else if(matches(obj->ShortDesc(), "halberd")) {
	  skmatch = get_weapon_type("Two-Handed Cleaves");
	  }
	else if(matches(obj->ShortDesc(), "hammer")) {
	  skmatch = get_weapon_type("Long Crushing");
	  }
	else if(matches(obj->ShortDesc(), "baton")) {
	  skmatch = get_weapon_type("Short Staves");
	  }
	else if(matches(obj->ShortDesc(), "quarterstaff")) {
	  skmatch = get_weapon_type("Long Staves");
	  }
	else if(matches(obj->ShortDesc(), "staff")) {
	  skmatch = get_weapon_type("Two-Handed Staves");
	  }
	else if(matches(obj->ShortDesc(), "bow")) {
	  skmatch = get_weapon_type("Archery");
	  }
	else if(matches(obj->ShortDesc(), "blade")) {
	  skmatch = get_weapon_type("Long Blades");
	  }
	else if(matches(obj->ShortDesc(), "bone")) {
	  skmatch = get_weapon_type("Long Crushing");
	  }
	else if(matches(obj->ShortDesc(), "branch")) {
	  skmatch = get_weapon_type("Long Crushing");
	  }
	else if(matches(obj->ShortDesc(), "pen")) {
	  skmatch = get_weapon_type("Short Piercing");
	  }
	else if(matches(obj->ShortDesc(), "shears")) {
	  skmatch = get_weapon_type("Short Piercing");
	  }
	else if(matches(obj->ShortDesc(), "stakes")) {
	  skmatch = get_weapon_type("Short Piercing");
	  }
	else {
	  fprintf(stderr, "Warning: Using Default of '%s' for '%s'!\n",
		get_weapon_skill(skmatch).c_str(), obj->ShortDesc());
	  }

	if(matches(obj->ShortDesc(), "two-handed")) {
	  if(skmatch == get_weapon_type("Short Blades"))
	    skmatch = get_weapon_type("Two-Handed Blades");

	  else if(skmatch == get_weapon_type("Long Blades"))
	    skmatch = get_weapon_type("Two-Handed Blades");

	  else if(skmatch == get_weapon_type("Short Piercing"))
	    skmatch = get_weapon_type("Two-Handed Piercing");

	  else if(skmatch == get_weapon_type("Long Piercing"))
	    skmatch = get_weapon_type("Two-Handed Piercing");

	  else if(skmatch == get_weapon_type("Short Flails"))
	    skmatch = get_weapon_type("Two-Handed Flails");

	  else if(skmatch == get_weapon_type("Long Flails"))
	    skmatch = get_weapon_type("Two-Handed Flails");

	  else if(skmatch == get_weapon_type("Short Staves"))
	    skmatch = get_weapon_type("Two-Handed Staves");

	  else if(skmatch == get_weapon_type("Long Staves"))
	    skmatch = get_weapon_type("Two-Handed Staves");

	  else if(skmatch == get_weapon_type("Long Crushing"))
	    skmatch = get_weapon_type("Two-Handed Crushing");

	  else if(skmatch == get_weapon_type("Short Crushing"))
	    skmatch = get_weapon_type("Two-Handed Crushing");

	  else if(skmatch == get_weapon_type("Short Cleaves"))
	    skmatch = get_weapon_type("Two-Handed Cleaves");

	  else if(skmatch == get_weapon_type("Long Cleaves"))
	    skmatch = get_weapon_type("Two-Handed Cleaves");
	  }

	if(wreach == 0) {
	  if(skmatch == get_weapon_type("Short Blades")) wreach = 0;
	  else if(skmatch == get_weapon_type("Short Piercing")) wreach = 0;
	  else if(skmatch == get_weapon_type("Long Blades")) wreach = 1;
	  else if(skmatch == get_weapon_type("Two-Handed Blades")) wreach = 2;
	  else if(skmatch == get_weapon_type("Long Piercing")) wreach = 1;
	  else if(skmatch == get_weapon_type("Two-Handed Piercing")) wreach = 2;
	  else if(skmatch == get_weapon_type("Short Crushing")) wreach = 0;
	  else if(skmatch == get_weapon_type("Long Crushing")) wreach = 1;
	  else if(skmatch == get_weapon_type("Two-Handed Crushing")) wreach = 2;
	  else if(skmatch == get_weapon_type("Short Cleaves")) wreach = 0;
	  else if(skmatch == get_weapon_type("Long Cleaves")) wreach = 1;
	  else if(skmatch == get_weapon_type("Two-Handed Cleaves")) wreach = 2;
	  else if(skmatch == get_weapon_type("Short Flails")) wreach = 0;
	  else if(skmatch == get_weapon_type("Long Flails")) wreach = 1;
	  else if(skmatch == get_weapon_type("Two-Handed Flails")) wreach = 2;
	  else if(skmatch == get_weapon_type("Short Staves")) wreach = 0;
	  else if(skmatch == get_weapon_type("Long Staves")) wreach = 1;
	  else if(skmatch == get_weapon_type("Two-Handed Staves")) wreach = 2;
	  else if(skmatch == get_weapon_type("Whips")) wreach = 3;
	  else if(skmatch == get_weapon_type("Archery")) wreach = 100;
	  else {
	    fprintf(stderr, "Warning: Using Default reach of zero for '%s'!\n",
		obj->ShortDesc());
	    }
	  }

	obj->SetSkill("WeaponType", skmatch);
//	obj->SetSkill("WeaponDamage", val[1]*val[2]);
	int sev = 0;
	int tot = (val[1] * (val[2]+1) + 1) / 2;  //Avg. Circle Dam. Rounded Up
	while(tot > sev) { ++sev; tot -= sev; }
	obj->SetSkill("WeaponForce", tot);
	obj->SetSkill("WeaponSeverity", sev);
	obj->SetSkill("WeaponReach", wreach);
	}

      int weight, value;
      fscanf(mudo, "%d %d %*d\n", &weight, &value);

      if(tp != 20) { // MONEY DOESN'T WORK THIS WAY
	obj->SetWeight(weight * 454);
	obj->SetVolume(weight); //FIXME: Better guess within units?
	obj->SetSize(1);
	obj->SetValue((value*valmod) / 1000);
	}

      fscanf(mudo, " %*[^#$]");
      }
    fclose(mudo);
    }
  }

void Object::CircleLoad(const char *fn) {
  FILE *mud = fopen(fn, "r");
  int zone = 0, offset = strlen(fn) - 5; //Chop off the .wld
  while(isdigit(fn[offset])) --offset;
  zone = atoi(fn+offset+1);
  if(mud) {
    //fprintf(stderr, "Loading Circle Realm from \"%s\"\n", fn);
    while(1) {
      int onum;
      if(fscanf(mud, " #%d\n", &onum) < 1) break;

      Object *obj = new Object(this);
      olist.push_back(obj);
      bynum[onum] = obj;

      obj->SetWeight(-1);
      obj->SetVolume(-1);
      obj->SetSize(-1);
      obj->SetValue(-1);

      memset(buf, 0, 65536);
      fscanf(mud, "%[^~\n]~\n", buf);
      for(char *ch=buf; (*ch); ++ch) if((*ch) == ';') (*ch) = ',';
      obj->SetShortDesc(buf);
      //fprintf(stderr, "Loaded Circle Room with Name = %s\n", buf);

      memset(buf, 0, 65536);
      fscanf(mud, "%[^~]~\n", buf);
      for(char *ch=buf; (*ch); ++ch) if((*ch) == ';') (*ch) = ',';
      obj->SetDesc(buf);
      //fprintf(stderr, "Loaded Circle Room with Desc = %s\n", buf);

      int val;
      fscanf(mud, "%*d %[^ \t\n] %d\n", buf, &val);
      if(val == 6) obj->SetSkill("WaterDepth", 1);	// WATER_SWIM
      else if(val == 7) obj->SetSkill("WaterDepth", 2);	// WATER_NOSWIM
      else if(val == 8) obj->SetSkill("WaterDepth", 3);	// UNDERWATER

      string name = obj->ShortDesc();
      if(name.find("Secret") >= 0 && name.find("Secret") < name.length()) {
        obj->SetSkill("Secret", 100000+onum);
	}

      if(string(buf).find('b') < strlen(buf) || (atoi(buf) & 2)) { //DEATH
        obj->SetSkill("Hazardous", 2);
	}
      if(string(buf).find('c') < strlen(buf) || (atoi(buf) & 4)) { //NOMOB
        obj->SetSkill("CircleZone", 999999);
	}
      else {
        obj->SetSkill("CircleZone", 100000 + zone);
	}

      while(1) {
	int dnum, tnum, tmp, tmp2;
	fscanf(mud, "%c", buf);
	if(buf[0] == 'D') {
	  fscanf(mud, "%d\n", &dnum);

	  memset(buf, 0, 65536);
	  fscanf(mud, "%[^\n]\n", buf);
	  while(strcmp(buf, "~")) {
	    memset(buf, 0, 65536);
	    fscanf(mud, "%[^\n]\n", buf);
	    }
	  memset(buf, 0, 65536);
	  if(!fscanf(mud, "%[^\n~]~\n", buf)) {
	    fscanf(mud, "~\n");
	    }
	  nmnum[dnum][obj] = buf;

	  fscanf(mud, "%d %d %d\n", &tmp, &tmp2, &tnum);

	  tonum[dnum][obj] = tnum;
	  tynum[dnum][obj] = tmp;
	  knum[dnum][obj] = tmp2;
	  }
	else if(buf[0] == 'E') {
	  fscanf(mud, "\n%[^\n]\n", buf);
	  memset(buf, 0, 65536);
	  fscanf(mud, "%[^\n]\n", buf);
	  while(strcmp(buf, "~")) {
	    memset(buf, 0, 65536);
	    fscanf(mud, "%[^\n]\n", buf);
	    }
	  }
	else break;
	}
      }
    vector<Object*>::iterator ob = olist.begin();
    for(; ob != olist.end(); ++ob) {
      for(int dir=0; dir<6; ++dir) {
	if(tonum[dir].count(*ob)) {
	  int tnum = tonum[dir][*ob];
	  if(bynum.count(tnum)) {
	    Object *nobj = NULL;
	    Object *nobj2 = NULL;
	    string des, nm = dirname[dir];

	    typeof((*ob)->Contents()) cont = (*ob)->Contents();
	    typeof(cont.begin()) cind;
	    for(cind = cont.begin(); cind != cont.end(); ++cind) {
	      if(string((*cind)->ShortDesc()) == "A passage exit.") {
		if((*cind)->ActTarg(ACT_SPECIAL_MASTER)->Parent()
			== bynum[tnum]) {
		  nobj = (*cind);
		  nobj2 = (*cind)->ActTarg(ACT_SPECIAL_MASTER);
		  }
		}
	      else if((*cind)->ActTarg(ACT_SPECIAL_LINKED)) {
		if((*cind)->ActTarg(ACT_SPECIAL_LINKED)->Parent()
			== bynum[tnum]) {
		  nobj = (*cind);
		  nobj2 = (*cind)->ActTarg(ACT_SPECIAL_LINKED);
		  nm = string(nobj->ShortDesc()) + " and " + dirname[dir];
		  }
		}
	      }
	    if(!nobj) {
	      nobj = new Object;
	      nobj2 = new Object;
	      nobj->SetParent(*ob);
	      nobj2->SetParent(bynum[tnum]);
	      nobj2->SetShortDesc("A passage exit.");
	      nobj2->SetDesc("A passage exit.");
	      nobj2->AddAct(ACT_SPECIAL_NOTSHOWN);
	      }
	    else {
	      nobj->StopAct(ACT_SPECIAL_NOTSHOWN);
	      }

	    if(nmnum[dir][*ob] != "") {
	      nm += " (";
	      nm += nmnum[dir][*ob];
	      nm += ")";
	      }
	    if(tynum[dir][*ob] != 0) {		//FIXME: Respond to "door"?
	      des = string("A door to the ") + dirname[dir] + " is here.";
	      nobj->SetSkill("Closeable", 1);
	      nobj->SetSkill("Lockable", 1);
	      if(tynum[dir][*ob] == 1) nobj->SetSkill("Pickable", 4);
	      if(tynum[dir][*ob] == 2) nobj->SetSkill("Pickable", 99);
	      if(knum[dir][*ob] > 0) {
		nobj->SetSkill("Lock", 100000 + knum[dir][*ob]);
		}
	      }
	    else {
	      des = string("A passage ") + dirname[dir] + " is here.";
	      }
	    nobj->SetShortDesc(nm.c_str());
	    nobj->SetDesc(des.c_str());
	    nobj->SetSkill("Open", 1);
	    nobj->SetSkill("Enterable", 1);
	    nobj->AddAct(ACT_SPECIAL_LINKED, nobj2);
	    nobj2->AddAct(ACT_SPECIAL_MASTER, nobj);

	    nmnum[dir].erase(*ob);
	    tonum[dir].erase(*ob);
	    tynum[dir].erase(*ob);
	    knum[dir].erase(*ob);
	    }
	  }
	}
      }
    fclose(mud);
    }
  else {
    fprintf(stderr, "Error: No Circle Realm \"%s\"\n", fn);
    }
  }

void Object::CircleLoadShp(const char *fn) {
  FILE *mud = fopen(fn, "r");
  if(mud) {
    Object *vortex = NULL;
    if(!fscanf(mud, "CircleMUD v3.0 Shop File~%[\n]", buf)) {
      fprintf(stderr, "Error: '%s' is not a CircleMUD v3.0 Shop File!\n", fn);
      }
    else {
      while(1) {
	int val;
	if(!fscanf(mud, "#%d~\n", &val)) break;  // Shop Number

	vortex = new Object;
	vortex->SetShortDesc("A shopkeeper vortex");
	vortex->SetDesc("An advanced wormhole that shopkeeper's use.");
	vortex->SetSkill("Vortex", 1); // Mark it as a shopkeeper Vortex.
	vortex->AddAct(ACT_SPECIAL_NOTSHOWN);
	vortex->SetSkill("Wearable on Right Shoulder", 1);
	vortex->SetSkill("Wearable on Left Shoulder", 2);

	fscanf(mud, "%d\n", &val);  // Item sold
	while(val >= 0) {
	  if(!bynumobj.count(val)) {
	    fprintf(stderr, "Error: Shop's item #%d does not exist!\n", val);
	    }
	  else {
	    Object *item = new Object(*(bynumobj[val]));
	    Object *item2 = dup_circle_obj(item);
	    item->SetParent(vortex);
	    item->SetSkill("Quantity", 1000);
	    if(item2) {
	      item2->SetParent(vortex);
	      item2->SetSkill("Quantity", 1000);
	      }
	    }
	  fscanf(mud, "%d\n", &val);  // Item sold
	  }

	double num, num2;
	fscanf(mud, "%lf\n", &num);  // Profit when Sell
	fscanf(mud, "%lf\n", &num2);  // Profit when Buy

	memset(buf, 0, 65536);
	fscanf(mud, "%[^\n]\n", buf);  // Item types bought
	val = atoi(buf);
	while(val >= 0) {
	  memset(buf, 0, 65536);
	  fscanf(mud, "%[^\n]\n", buf);  // Item types bought
	  val = atoi(buf);
	  }

	for(int ctr=0; ctr<8; ++ctr) {
	  fscanf(mud, "%[^\n]\n", buf+strlen(buf));
	  }

	memset(buf, 0, 65536);
	fscanf(mud, "%[^\n]\n", buf+strlen(buf));  // Shop Bitvectors

	fscanf(mud, "%d\n", &val);  // Shopkeeper!
	Object *keeper = NULL;
	if(bynummobinst.count(val)) keeper = bynummobinst[val];

	memset(buf, 0, 65536);
	fscanf(mud, "%[^\n]\n", buf+strlen(buf));  // With Bitvectors

	fscanf(mud, "%d\n", &val);  // Shop rooms
	while(val >= 0) {
	  fscanf(mud, "%d\n", &val);  // Shop rooms
	  }

	fscanf(mud, "%*d\n");  // Open time
	fscanf(mud, "%*d\n");  // Close time
	fscanf(mud, "%*d\n");  // Open time
	fscanf(mud, "%*d\n");  // Close time

	if(keeper) {
	  keeper->SetSkill("Sell Profit", (int)(num*1000.0+0.5));
	  keeper->SetSkill("Buy Profit", (int)(num2*1000.0+0.5));
	  vortex->SetParent(keeper);
	  keeper->AddAct(ACT_WEAR_RSHOULDER, vortex);
	  }
	}
      }
    fclose(mud);
    }
  else {
    fprintf(stderr, "Error: '%s' does not exist!\n", fn);
    }
  }
