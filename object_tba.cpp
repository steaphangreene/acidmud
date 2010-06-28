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

static int tba_bitvec(const string& val) {
  int ret = atoi(val.c_str());
  if(ret == 0) {	//Works fine for "0" too
    for(size_t idx = 0; idx < val.length(); ++idx) {
      ret |= 1 << ((val[idx] & 31) - 1);
      }
    }
  return ret;
  }

static const char *dirname[6] = {
	"north", "east", "south", "west", "up", "down"
	};

static int unhandled_trig = 0;
static char buf[65536];
void Object::TBALoadAll() {
  FILE *mudt = fopen("tba/trg/index", "r");
  if(mudt) {
    sprintf(buf, "tba/trg/%c", 0);
    memset(buf+strlen(buf), 0, 256);
    fscanf(mudt, "%255[^\n\r]\n", buf+strlen(buf));
    while(strlen(buf) > 10) {
      TBALoadTRG(buf);
      sprintf(buf, "tba/trg/%c", 0);
      memset(buf+strlen(buf), 0, 256);
      fscanf(mudt, "%255[^\n\r]\n", buf+strlen(buf));
      }
    fclose(mudt);
    }
  FILE *mudw = fopen("tba/wld/index", "r");
  if(mudw) {
    sprintf(buf, "tba/wld/%c", 0);
    memset(buf+strlen(buf), 0, 256);
    fscanf(mudw, "%255[^\n\r]\n", buf+strlen(buf));
    while(strlen(buf) > 10) {
      TBALoadWLD(buf);
      sprintf(buf, "tba/wld/%c", 0);
      memset(buf+strlen(buf), 0, 256);
      fscanf(mudw, "%255[^\n\r]\n", buf+strlen(buf));
      }
    fclose(mudw);
    }
  FILE *mudo = fopen("tba/obj/index", "r");
  if(mudo) {
    sprintf(buf, "tba/obj/%c", 0);
    memset(buf+strlen(buf), 0, 256);
    fscanf(mudo, "%255[^\n\r]\n", buf+strlen(buf));
    while(strlen(buf) > 10) {
      TBALoadOBJ(buf);
      sprintf(buf, "tba/obj/%c", 0);
      memset(buf+strlen(buf), 0, 256);
      fscanf(mudo, "%255[^\n\r]\n", buf+strlen(buf));
      }
    fclose(mudo);
    }
  FILE *mudm = fopen("tba/mob/index", "r");
  if(mudm) {
    sprintf(buf, "tba/mob/%c", 0);
    memset(buf+strlen(buf), 0, 256);
    fscanf(mudm, "%255[^\n\r]\n", buf+strlen(buf));
    while(strlen(buf) > 10) {
      TBALoadMOB(buf);
      sprintf(buf, "tba/mob/%c", 0);
      memset(buf+strlen(buf), 0, 256);
      fscanf(mudm, "%255[^\n\r]\n", buf+strlen(buf));
      }
    fclose(mudm);
    }
  FILE *mudz = fopen("tba/zon/index", "r");
  if(mudz) {
    sprintf(buf, "tba/zon/%c", 0);
    memset(buf+strlen(buf), 0, 256);
    fscanf(mudz, "%255[^\n\r]\n", buf+strlen(buf));
    while(strlen(buf) > 10) {
      TBALoadZON(buf);
      sprintf(buf, "tba/zon/%c", 0);
      memset(buf+strlen(buf), 0, 256);
      fscanf(mudz, "%255[^\n\r]\n", buf+strlen(buf));
      }
    fclose(mudz);
    }
  TBAFinalizeTriggers();
  FILE *muds = fopen("tba/shp/index", "r");
  if(muds) {
    sprintf(buf, "tba/shp/%c", 0);
    memset(buf+strlen(buf), 0, 256);
    fscanf(muds, "%255[^\n\r]\n", buf+strlen(buf));
    while(strlen(buf) > 10) {
      TBALoadSHP(buf);
      sprintf(buf, "tba/shp/%c", 0);
      memset(buf+strlen(buf), 0, 256);
      fscanf(muds, "%255[^\n\r]\n", buf+strlen(buf));
      }
    fclose(muds);
    }
  TBACleanup();
  fprintf(stderr, "Warning: %d unhandled triggers!\n", unhandled_trig);
  }

static list<Object*> todotrg;
static map<int,Object*> bynumtrg;
static map<int,Object*> bynumwld;
static map<int,Object*> bynumobj;
static map<int,Object*> bynummob;
static map<int,Object*> bynummobinst;
static map<Object*,int> tonum[6];
static map<Object*,int> tynum[6];
static map<Object*,int> knum[6];
static map<Object*,string> nmnum[6];
static vector<Object*> olist;
static Object *objroom = NULL;
static Object *mobroom = NULL;

void Object::TBACleanup() {
  map<int,Object*>::iterator ind;

//  for(ind = bynumobj.begin(); ind != bynumobj.end(); ++ind) {
//    (*ind).second->Recycle();
//    }
//  for(ind = bynummob.begin(); ind != bynummob.end(); ++ind) {
//    (*ind).second->Recycle();
//    }
  for(ind = bynumtrg.begin(); ind != bynumtrg.end(); ++ind) {
    (*ind).second->Recycle();
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
  list<Object*>::iterator trg = todotrg.begin();
  for(; trg != todotrg.end(); ++trg) {
    string newtext = "Powers List:\n";
    const char *cur = (*trg)->long_desc.c_str();
    while((cur = strstr(cur, "teleport [")) != NULL) {
      int rnum;
      (*trg)->Parent()->SetSkill("Teleport", 10);
      (*trg)->Parent()->SetSkill("Restricted Item", 1);
      sscanf(cur, "teleport [%d]\n", &rnum);
      if(bynumwld.count(rnum) > 0) {
	newtext += string("teleport ") + bynumwld[rnum]->Name() + "\n";
	}
      else {
	fprintf(stderr, "Error: Can't find teleport dest: %d\n", rnum);
	}
      ++cur;
      }
    if(newtext != "Powers List:\n") {
      (*trg)->Parent()->SetLongDesc(newtext.c_str());
      (*trg)->Recycle();
      //fprintf(stderr, "%s", newtext.c_str());
      }
    }
  todotrg.clear();
  }

static Mind *tba_mob_mind = NULL;

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
  gold->SetSkill("Money", 1);
  }

Mind *get_tba_mob_mind() {
  if(!tba_mob_mind) {
    tba_mob_mind = new Mind();
    tba_mob_mind->SetTBAMob();
    }
  return tba_mob_mind;
  }

Object *dup_tba_obj(Object *obj) {
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

void Object::TBAFinishMOB(Object *mob) {
  if(mob->Skill("TBAGold")) {
    Object *bag = new Object(mob);
    bag->SetShortDesc("a TBAMUD purse");
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

    bag->SetSkill("Perishable", 1);
    mob->AddAct(ACT_WEAR_RHIP, bag);

    if(!gold) init_gold();
    Object *g = new Object(*gold);
    g->SetParent(bag);
    g->SetSkill("Quantity", mob->Skill("TBAGold"));
    mob->SetSkill("TBAGold", 0);
    }

  if(mob->ActTarg(ACT_WEAR_LSHOULDER)) { //TBAMUD Bags Only
    mob->ActTarg(ACT_WEAR_LSHOULDER)->SetSkill("Container",
	mob->ActTarg(ACT_WEAR_LSHOULDER)->ContainedWeight());
    mob->ActTarg(ACT_WEAR_LSHOULDER)->SetSkill("Capacity",
	mob->ActTarg(ACT_WEAR_LSHOULDER)->ContainedVolume());
    }

  if(mob->Skill("TBAAttack")) {
    if(mob->IsAct(ACT_WIELD)) {
      //fprintf(stderr, "Weapon def: %s\n", mob->ActTarg(ACT_WIELD)->Name());
      if(mob->ActTarg(ACT_WIELD)->Skill("WeaponType") == 0) {
	if(!mob->ActTarg(ACT_HOLD)) {	//Don't wield non-weapons, hold them
	  fprintf(stderr, "Warning: Wielded non-weapon: %s\n",
		mob->ActTarg(ACT_WIELD)->Name()
		);
	  mob->AddAct(ACT_HOLD, mob->ActTarg(ACT_WIELD));
	  mob->StopAct(ACT_WIELD);
	  }
	else {
	  fprintf(stderr, "Error: Wielded non-weapon with a held item: %s\n",
		mob->ActTarg(ACT_WIELD)->Name()
		);
	  }
	}
      else {
	mob->SetSkill(
		get_weapon_skill(mob->ActTarg(ACT_WIELD)->Skill("WeaponType")),
		mob->Skill("TBAAttack")
		);
	}
      if(mob->Skill("NaturalWeapon") == 13) {	//Default (hit), but is armed!
	mob->SetSkill("NaturalWeapon", 0);	//So remove it
	}
      }
    else {
      mob->SetSkill("Punching", mob->Skill("TBAAttack"));
      }
    mob->SetSkill("TBAAttack", 0);
    }
  if(mob->Skill("TBADefense")) {
    if(mob->IsAct(ACT_WEAR_SHIELD)) {
      mob->SetSkill("Shields", mob->Skill("TBADefense"));
      }
    else if(mob->Skill("Punching")) {
      mob->SetSkill("Kicking", mob->Skill("TBADefense"));
      }
    else {
      mob->SetSkill("Kicking", mob->Skill("TBADefense") / 2);
      mob->SetSkill("Punching",
	mob->Skill("TBADefense") - mob->Skill("Kicking"));
      }
    mob->SetSkill("TBADefense", 0);
    }

  if(Matches("snake") || Matches("spider") || Matches("poisonous")) {
    SetSkill("Poisonous", att[2]);
    }
  }

static Object *lastmob = NULL, *lastbag = NULL;
static map<int, Object*>lastobj;
void Object::TBALoadZON(const char *fn) {
  FILE *mudz = fopen(fn, "r");
  if(mudz) {
    //fprintf(stderr, "Loading TBA Zone from \"%s\"\n", fn);
    for(int ctr=0; ctr<3; ++ctr) {
      fscanf(mudz, "%*[^\n\r]\n");
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
	  if(bynumwld.count(room) > 0)
	    door = bynumwld[room]->PickObject(dirname[dnum], LOC_INTERNAL);
	  if(door && state == 0) {
	    door->SetSkill("Open", 1000);
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
	  fscanf(mudz, " %*d %d %*d %d %*[^\n\r]\n", &num, &room);
	  if(bynumwld.count(room) && bynummob.count(num)) {
	    Object *obj = new Object(bynumwld[room]);
	    obj->SetShortDesc("a TBAMUD MOB Popper");
	    obj->SetDesc("This thing just pops out MOBs.");

	    //fprintf(stderr, "Put Mob \"%s\" in Room \"%s\"\n", obj->ShortDesc(), bynumwld[room]->ShortDesc());

	    if(lastmob) TBAFinishMOB(lastmob);
	    lastmob = new Object(*(bynummob[num]));
	    bynummobinst[num] = lastmob;
	    lastmob->SetParent(obj);
	    lastmob->AddAct(ACT_SPECIAL_MASTER, obj);
	    obj->SetSkill("TBAPopper", 1);
	    obj->SetSkill("Invisible", 1000);
	    obj->Activate();
	    lastbag = NULL;
	    }
	  } break;
	case('O'): {
	  int num, room;
	  fscanf(mudz, " %*d %d %*d %d %*[^\n\r]\n", &num, &room);
	  if(bynumwld.count(room) && bynumobj.count(num)) {
	    Object *obj = new Object(*(bynumobj[num]));
	    obj->SetParent(bynumwld[room]);
	    //fprintf(stderr, "Put Obj \"%s\" in Room \"%s\"\n", obj->ShortDesc(), bynumwld[room]->ShortDesc());
	    lastobj[num] = obj;
	    }
	  } break;
	case('G'):
	case('E'): {
	  int num, pos = -1;
	  if(type == 'E') fscanf(mudz, " %*d %d %*d %d%*[^\n\r]\n", &num, &pos);
	  if(type == 'G') fscanf(mudz, " %*d %d %*d%*[^\n\r]\n", &num);
	  if(lastmob && bynumobj.count(num)) {
	    Object *obj = new Object(*(bynumobj[num]));
	    Object *obj2 = dup_tba_obj(obj);
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
	      case(3):		//TBA MOBs have two necks
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
		lastbag = new Object(lastmob);
		lastbag->SetShortDesc("a TBAMUD pack");
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

		lastbag->SetSkill("Perishable", 1);
		lastmob->AddAct(ACT_WEAR_LSHOULDER, lastbag);
		}
	      obj->Travel(lastbag);
	      if(obj2) obj2->Travel(lastbag);
	      }
	    }
	  } break;
	case('P'): {
	  int num, innum;
	  fscanf(mudz, " %*d %d %*d %d %*[^\n\r]\n", &num, &innum);
	  if(lastobj.count(innum) && bynumobj.count(num)) {
	    Object *obj = new Object(*(bynumobj[num]));
	    Object *obj2 = dup_tba_obj(obj);
	    obj->SetParent(lastobj[innum]);
	    if(obj2) obj2->SetParent(lastobj[innum]);
	    //fprintf(stderr, "Put Obj \"%s\" in Obj \"%s\"\n", obj->ShortDesc(), lastobj[innum]->ShortDesc());
	    lastobj[num] = obj;
	    }
	  } break;
	default: {
	  fscanf(mudz, "%*[^\n\r]\n");
	  } break;
	}
      }
    if(lastmob) TBAFinishMOB(lastmob);
    fclose(mudz);
    }
  }


void Object::TBALoadMOB(const char *fn) {
  if(mobroom == NULL) {
    mobroom = new Object(this);
    mobroom->SetSkill("Invisible", 1000);
    mobroom->SetShortDesc("The TBAMUD MOB Room");
    }
  FILE *mudm = fopen(fn, "r");
  if(mudm) {
    //fprintf(stderr, "Loading TBA Mobiles from \"%s\"\n", fn);
    while(1) {
      int onum;
      if(fscanf(mudm, " #%d\n", &onum) < 1) break;
      //fprintf(stderr, "Loaded MOB #%d\n", onum);

      Object *obj = new Object(mobroom);
      obj->SetSkill("TBAMOB", 1000000+onum);
      bynummob[onum] = obj;

      //FIXME: Nicknames should not be ignored!!!
      memset(buf, 0, 65536);
      fscanf(mudm, "%65535[^~\n]~\n", buf);

      memset(buf, 0, 65536);
      fscanf(mudm, "%65535[^~\n]~\n", buf);
      for(char *ch=buf; (*ch); ++ch) if((*ch) == ';') (*ch) = ',';
      obj->SetShortDesc(buf);
      //fprintf(stderr, "Loaded TBA Mobile with Name = %s\n", buf);

      memset(buf, 0, 65536);
      if(!fscanf(mudm, "%65535[^~]~\n", buf)) fscanf(mudm, "%*[^\n\r]\n");
      else {
        for(char *ch=buf; (*ch); ++ch) if((*ch) == ';') (*ch) = ',';
	obj->SetDesc(buf);
	}
      //fprintf(stderr, "Loaded TBA Mobile with Desc = %s\n", buf);

      memset(buf, 0, 65536);
      if(!fscanf(mudm, "%65535[^~]~\n", buf)) fscanf(mudm, "%*[^\n\r]\n");
      else {
        for(char *ch=buf; (*ch); ++ch) if((*ch) == ';') (*ch) = ',';
	obj->SetLongDesc(buf);
	}
      //fprintf(stderr, "Loaded TBA Mobile with LongDesc = %s\n", buf);

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
      fscanf(mudm, "%65535[^ \t\n]", buf); //Rest of line read below...

      obj->SetSkill("TBAAction", 8); //IS_NPC - I'll use it to see if(MOB)
      if(strcasestr(buf, "b") || (atoi(buf) & 2)) { //SENTINEL
	obj->SetSkill("TBAAction", obj->Skill("TBAAction") | 2);
	}
      if(strcasestr(buf, "c") || (atoi(buf) & 4)) { //SCAVENGER
	obj->SetSkill("TBAAction", obj->Skill("TBAAction") | 4);
	}
      if(strcasestr(buf, "e") || (atoi(buf) & 16)) { //AWARE
	aware = 1;
	}
      if(strcasestr(buf, "f") || (atoi(buf) & 32)) { //AGGRESSIVE
	obj->SetSkill("TBAAction", obj->Skill("TBAAction") | 32);
	}
      if(strcasestr(buf, "g") || (atoi(buf) & 64)) { //STAY_ZONE
	obj->SetSkill("TBAAction", obj->Skill("TBAAction") | 64);
	}
      if(strcasestr(buf, "h") || (atoi(buf) & 128)) { //WIMPY
	obj->SetSkill("TBAAction", obj->Skill("TBAAction") | 128);
	}
      if(strcasestr(buf, "l") || (atoi(buf) & 2048)) { //MEMORY
	obj->SetSkill("TBAAction", obj->Skill("TBAAction") | 2048);
	}
      if(strcasestr(buf, "m") || (atoi(buf) & 4096)) { //HELPER
	obj->SetSkill("TBAAction", obj->Skill("TBAAction") | 4096);
	}
      //FIXME: Add others here.

      memset(buf, 0, 65536);
      fscanf(mudm, " %*s %*s %*s %65535[^ \t\n]", buf); //Rest of line read below...
      if(strcasestr(buf, "g") || (atoi(buf) & 64)) { //WATERWALK
	obj->SetSkill("TBAAffection", obj->Skill("TBAAffection") | 64);
	}
      if(strcasestr(buf, "s") || (atoi(buf) & 262144)) { //SNEAK
	sneak = 1;
	}
      if(strcasestr(buf, "t") || (atoi(buf) & 524288)) { //HIDE
	hidden = 1;
	}
      //FIXME: Implement special powers of MOBs here.

      memset(buf, 0, 65536);
      fscanf(mudm, " %*s %*s %*s %d %c\n", &val, &tp);
      if(val > 0) obj->SetSkill("Honor", val);
      else obj->SetSkill("Dishonor", -val);

      obj->SetSkill("Accomplishment", 120000+onum);

      if(tp == 'E' || tp == 'S') {
	fscanf(mudm, "%d %d %d", &val, &val2, &val3);
	for(int ctr=0; ctr<val; ++ctr)
	  obj->SetAttribute(ctr%6, obj->Attribute(ctr%6)+1);	// val1 = Level
	obj->SetSkill("TBAAttack", ((20-val2)/3) + 3);		// val2 = THAC0
	obj->SetSkill("TBADefense", ((10-val3)/3) + 3)	;	// val2 = AC

	fscanf(mudm, " %dd%d+%d", &val, &val2, &val3);		// Hit Points
	val = (val*(val2+1) + 1) / 2 + val3;
	obj->SetAttribute(0, (val+49)/50);			// Becomes Body

	fscanf(mudm, " %dd%d+%d\n", &val, &val2, &val3);// Barehand Damage
	val = (val*(val2+1) + 1) / 2 + val3;
	obj->SetAttribute(2, (val/3)+3);		// Becomes Strength

	fscanf(mudm, "%d", &val);  // Gold
	obj->SetSkill("TBAGold", val);

	fscanf(mudm, "%*[^\n\r]\n"); // XP //FIXME: Worth Karma?

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

      obj->SetSkill("NaturalWeapon", 13);	//"Hits" (is default in TBA)
      memset(buf, 0, 65536);
      while(tp == 'E') {  // Basically an if with an infinite loop ;)
	if(fscanf(mudm, "Con: %d\n", &val))
	  obj->SetAttribute(0, MAX(obj->Attribute(0), (val/3)+3));

	else if(fscanf(mudm, "Dex: %d\n", &val))
	  obj->SetAttribute(1, MAX(obj->Attribute(1), (val/3)+3));

	else if(fscanf(mudm, "Str: %d\n", &val))
	  obj->SetAttribute(2, MAX(obj->Attribute(2), (val/3)+3));

	else if(fscanf(mudm, "ha: %d\n", &val)) //'Cha' minus 'Con' Conflict!
	  obj->SetAttribute(3, MAX(obj->Attribute(3), (val/3)+3));

	else if(fscanf(mudm, "Int: %d\n", &val))
	  obj->SetAttribute(4, MAX(obj->Attribute(4), (val/3)+3));

	else if(fscanf(mudm, "Wis: %d\n", &val))
	  obj->SetAttribute(5, MAX(obj->Attribute(5), (val/3)+3));

	else if(fscanf(mudm, "Add: %d\n", &val)); //'StrAdd' - Do Nothing

	else if(fscanf(mudm, "BareHandAttack: %d\n", &val)) {
	  if(val == 13) val = 0;	//Punches (is the Default in Acid)
	  obj->SetSkill("NaturalWeapon", val);
	  }

	else break;
	}
      fscanf(mudm, " E");	//Nuke the terminating "E", if present.

      obj->SetWeight(obj->Attribute(0) * 20000);
      obj->SetSize(1000 + obj->Attribute(0) * 200);
      obj->SetVolume(100);
      obj->SetValue(-1);

      if(aware) {	//Perception = Int
	obj->SetSkill("Perception", obj->Attribute(4));
	}

      if(sneak && hidden) {		//Stealth = 3Q/2
	obj->SetSkill("Stealth", (3*obj->Attribute(1)+1)/2);
	}
      else if(hidden) {			//Stealth = Q
	obj->SetSkill("Stealth", obj->Attribute(1));
	}
      else if(sneak) {			//Stealth = Q/2
	obj->SetSkill("Stealth", (obj->Attribute(1)+1)/2);
	}

      if(hidden) {
	obj->SetSkill("Hidden", obj->Skill("Stealth") * 2);
	}
      if(sneak) {
	obj->StartUsing("Stealth");
	}

      int tnum;
      while(fscanf(mudm, " T %d\n", &tnum) > 0) {
	if(tnum > 0 && bynumtrg.count(tnum) > 0) {
	  Object *trg = new Object(*(bynumtrg[tnum]));
	  trg->SetParent(obj);
	  todotrg.push_back(trg);
	//  fprintf(stderr, "Put Trg \"%s\" on MOB \"%s\"\n",
	//	trg->Desc(), obj->ShortDesc()
	//	);
	  }
	}

      fscanf(mudm, " %*[^#$]");
      }
    fclose(mudm);
    }
  }


static void add_tba_spell(Object *obj, int spell, int power) {
  switch(spell) {
    case(-1): {		// No Effect
      } break;
    case(1): {		// ARMOR
      obj->SetSkill("Resilience Spell", power);
      } break;
    case(2): {		// TELEPORT
      obj->SetSkill("Teleport Spell", power);
      } break;
    case(3): {		// BLESS
      obj->SetSkill("Luck Spell", power);
      } break;
    case(4): {		// BLINDNESS
      obj->SetSkill("Blind Spell", power);
      } break;
    case(5): {		// BURNING HANDS
      obj->SetSkill("Fire Dart Spell", power);
      } break;
    case(6): {		// CALL LIGHTNING
      obj->SetSkill("Lightning Bolt Spell", power);
      } break;
    case(7): {		// CHARM
      obj->SetSkill("Influence Spell", power);
      } break;
    case(8): {		// CHILL TOUCH
      obj->SetSkill("Injure Spell", power);
      } break;
    case(9): {		// CLONE
      obj->SetSkill("Copy Book Spell", power);
      } break;
    case(10): {		// COLOR SPRAY
      obj->SetSkill("Distract Spell", power);
      } break;
    case(11): {		// CONTROL WEATHER
      obj->SetSkill("Clear Weather Spell", power);
      } break;
    case(12): {		// CREATE FOOD
      obj->SetSkill("Create Food Spell", power);
      } break;
    case(13): {		// CREATE WATER
      obj->SetSkill("Create Water Spell", power);
      } break;
    case(14): {		// CURE BLIND
      obj->SetSkill("Cure Blindness Spell", power);
      } break;
    case(15): {		// CURE_CRITIC
      obj->SetSkill("Heal Spell", power);
      } break;
    case(16): {		// CURE_LIGHT
      obj->SetSkill("Energize Spell", power);
      } break;
    case(17): {		// CURSE
      obj->SetSkill("Misfortune Spell", power);
      } break;
    case(18): {		// DETECT ALIGN
      obj->SetSkill("Identify Character Spell", power);
      } break;
    case(19): {		// DETECT INVIS
      obj->SetSkill("Heat Vision Spell", power);
      } break;
    case(20): {		// DETECT MAGIC
      obj->SetSkill("Detect Cursed Items Spell", power);
      } break;
    case(21): {		// DETECT POISON
      obj->SetSkill("Detect Poison Spell", power);
      } break;
    case(22): {		// DETECT EVIL
      obj->SetSkill("Identify Person Spell", power);
      } break;
    case(23): {		// EARTHQUAKE
      obj->SetSkill("Earthquake Spell", power);
      } break;
    case(24): {		// ENCHANT WEAPON
      obj->SetSkill("Force Sword Spell", power);
      } break;
    case(25): {		// ENERGY DRAIN
      obj->SetSkill("Weaken Subject Spell", power);
      } break;
    case(26): {		// FIREBALL
      obj->SetSkill("Fireball Spell", power);
      } break;
    case(27): {		// HARM
      obj->SetSkill("Harm Spell", power);
      } break;
    case(28): {		// HEAL
      obj->SetSkill("Heal Spell", power);
      obj->SetSkill("Energize Spell", power);
      } break;
    case(29): {		// INVISIBLE
      obj->SetSkill("Invisibility Spell", power);
      } break;
    case(30): {		// LIGHTNING BOLT
      obj->SetSkill("Fire Burst Spell", power);
      } break;
    case(31): {		// LOCATE OBJECT
      obj->SetSkill("Locate Object Spell", power);
      } break;
    case(32): {		// MAGIC MISSILE
      obj->SetSkill("Force Arrow Spell", power);
      } break;
    case(33): {		// POISON
      obj->SetSkill("Poisonous", power);
      } break;
    case(34): {		// PROT FROM EVIL
      obj->SetSkill("Personal Shield Spell", power);
      } break;
    case(35): {		// REMOVE CURSE
      obj->SetSkill("Remove Curse Spell", power);
      } break;
    case(36): {		// SANCTUARY
      obj->SetSkill("Treatment Spell", power);
      } break;
    case(37): {		// SHOCKING GRASP
      obj->SetSkill("Spark Spell", power);
      } break;
    case(38): {		// SLEEP
      obj->SetSkill("Sleep Other Spell", power);
      } break;
    case(39): {		// STRENGTH
      obj->SetSkill("Strength Spell", power);
      } break;
    case(40): {		// SUMMON
      obj->SetSkill("Summon Creature Spell", power);
      } break;
    case(41): {		// VENTRILOQUATE
      obj->SetSkill("Translate Spell", power);
      } break;
    case(42): {		// WORD OF RECALL
      obj->SetSkill("Recall Spell", power);
      } break;
    case(43): {		// REMOVE_POISON
      obj->SetSkill("Cure Poison Spell", power);
      } break;
    case(44): {		// SENSE LIFE
      obj->SetSkill("Light Spell", power);
      } break;
    case(45): {		// ANIMATE DEAD
      obj->SetSkill("Create Zombie Spell", power);
      } break;
    case(46): {		// DISPEL GOOD??
      obj->SetSkill("Protection Spell", power);
      } break;
    case(47): {		// GROUP ARMOR
      obj->SetSkill("Group Resilience Spell", power);
      } break;
    case(48): {		// GROUP HEAL
      obj->SetSkill("Heal Group Spell", power);
      } break;
    case(49): {		// GROUP RECALL
      obj->SetSkill("Recall Group Spell", power);
      } break;
    case(50): {		// INFRAVISION
      obj->SetSkill("Dark Vision Spell", power);
      } break;
    case(51): {		// WATERWALK
      obj->SetSkill("Float Spell", power);
      } break;
    case(52):
    case(201): {	// IDENTIFY
      obj->SetSkill("Identify Spell", power);
      } break;
    default: {
      fprintf(stderr, "Warning: Unhandled CicleMUD Spell: %d\n", spell);
      }
    }
  }

void Object::TBALoadOBJ(const char *fn) {
  if(objroom == NULL) {
    objroom = new Object(this);
    objroom->SetSkill("Invisible", 1000);
    objroom->SetShortDesc("The TBAMUD Object Room");
    }
  FILE *mudo = fopen(fn, "r");
  if(mudo) {
    //fprintf(stderr, "Loading TBA Objects from \"%s\"\n", fn);
    while(1) {
      int onum;
      int valmod = 1000, powmod = 1;
      if(fscanf(mudo, " #%d\n", &onum) < 1) break;
      //fprintf(stderr, "Loaded object #%d\n", onum);

      Object *obj = new Object(objroom);
      obj->SetSkill("TBAObject", 1000000+onum);
      bynumobj[onum] = obj;

      vector<string> aliases;
      memset(buf, 0, 65536);	//Alias List
      fscanf(mudo, "%65535[^~\n]~\n", buf);

      const char *str;
      const char *ind = buf;
      while(sscanf(ind, " %as", &str) > 0) {
	ind += strlen(str) + 1;
	aliases.push_back(string(str));
	free((void *)(str));
	}

      memset(buf, 0, 65536);	//Short Desc
      fscanf(mudo, "%65535[^~\n]~\n", buf);
      for(char *ch=buf; (*ch); ++ch) if((*ch) == ';') (*ch) = ',';
      obj->SetShortDesc(buf);
      //fprintf(stderr, "Loaded TBA Object with Name = %s\n", buf);

      string label = "";
      for(unsigned int actr = 0; actr < aliases.size(); ++actr) {
	if(!obj->Matches(aliases[actr].c_str())) {
	  label += " " + aliases[actr];
	  }
	}
      if(label.length() > 0) {
	label[0] = '(';
	label += ')';
	obj->SetShortDesc((string(obj->ShortDesc()) + " " + label).c_str());
	}

      memset(buf, 0, 65536);	//Long Desc
      if(!fscanf(mudo, "%65535[^~]~\n", buf)) fscanf(mudo, "%*[^\n\r]\n");
      else {
	for(char *ch=buf; (*ch); ++ch) if((*ch) == ';') (*ch) = ',';
	obj->SetDesc(buf);
	}
      //fprintf(stderr, "Loaded TBA Object with Desc = %s\n", buf);

      fscanf(mudo, "%*[^\n\r]\n");

      int tp=0, val[4];
      memset(buf, 0, 65536);
      fscanf(mudo, "%d %65535[^ \n\t]", &tp, buf);	//Effects Bitvector
      if(strcasestr(buf, "a") || (atoi(buf) & 1)) { //GLOW
	obj->SetSkill("Light Source", 10);
	}
      if(strcasestr(buf, "b") || (atoi(buf) & 2)) { //HUM
	obj->SetSkill("Noise Source", 10);
	}
//      if(strcasestr(buf, "c") || (atoi(buf) & 4)) { //NORENT
//	}
//      if(strcasestr(buf, "d") || (atoi(buf) & 8)) { //NODONATE
//	}
      if(strcasestr(buf, "e") || (atoi(buf) & 16)) { //NOINVIS
	obj->SetSkill("Obvious", 1000);
	}
      if(strcasestr(buf, "f") || (atoi(buf) & 32)) { //INVISIBLE
	obj->SetSkill("Invisible", 10);
	}
      if(strcasestr(buf, "g") || (atoi(buf) & 64)) { //MAGIC
	obj->SetSkill("Magical", 10);
	}
      if(strcasestr(buf, "h") || (atoi(buf) & 128)) { //NODROP
	obj->SetSkill("Cursed", 10);
	}
      if(strcasestr(buf, "i") || (atoi(buf) & 256)) { //BLESS
	obj->SetSkill("Blessed", 10);
	}
//      if(strcasestr(buf, "j") || (atoi(buf) & 512)) { //ANTI_GOOD
//	}
//      if(strcasestr(buf, "k") || (atoi(buf) & 1024)) { //ANTI_EVIL
//	}
//      if(strcasestr(buf, "l") || (atoi(buf) & 2048)) { //ANTI_NEUTRAL
//	}
//      if(strcasestr(buf, "m") || (atoi(buf) & 4096)) { //ANTI_MAGIC_USER
//	}
//      if(strcasestr(buf, "n") || (atoi(buf) & 8192)) { //ANTI_CLERIC
//	}
//      if(strcasestr(buf, "o") || (atoi(buf) & 16384)) { //ANTI_THIEF
//	}
//      if(strcasestr(buf, "p") || (atoi(buf) & 32768)) { //ANTI_WARRIOR
//	}
      if(strcasestr(buf, "q") || (atoi(buf) & 65536)) { //NOSELL
	obj->SetSkill("Priceless", 1);
	}

	//Wear Bitvector
      memset(buf, 0, 65536);
      fscanf(mudo, "%*s %*s %*s %65535[^ \n\t]%*[^\n\r]\n", buf);
      if(strcasestr(buf, "a") || (atoi(buf) & 1)) { //TAKE
	obj->SetPos(POS_LIE);
	}

      int sf = 0;
      if(!strncasecmp(obj->ShortDesc(), "a pair of ", 10)) sf = 9;
      else if(!strncasecmp(obj->ShortDesc(), "some ", 5)) sf = 4;
      else if(!strncasecmp(obj->ShortDesc(), "a set of ", 9)) sf = 8;

      string name = obj->ShortDesc();
      if(strcasestr(buf, "b") || (atoi(buf) & 2)) {
	obj->SetSkill("Wearable on Left Finger", 1);	//Two Alternatives
	obj->SetSkill("Wearable on Right Finger", 2);
	}
      if(strcasestr(buf, "c") || (atoi(buf) & 4)) {
	obj->SetSkill("Wearable on Neck", 1);
	}
      if(strcasestr(buf, "d") || (atoi(buf) & 8)) {
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
      if(strcasestr(buf, "e") || (atoi(buf) & 16)) {
	obj->SetSkill("Wearable on Head", 1);
	}
      if(strcasestr(buf, "f") || (atoi(buf) & 32)) {
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
	  powmod = 2;
	  }
	}
      if(strcasestr(buf, "g") || (atoi(buf) & 64)) {
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
	  powmod = 2;
	  }
	}
      if(strcasestr(buf, "h") || (atoi(buf) & 128)) {
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
	  powmod = 2;
	  }
	}
      if(strcasestr(buf, "i") || (atoi(buf) & 256)) {
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
	  powmod = 2;
	  }
	}
      if(strcasestr(buf, "j") || (atoi(buf) & 512)) {
	obj->SetSkill("Wearable on Shield", 1);	// FIXME: Wear Shield?
	}
      if(strcasestr(buf, "k") || (atoi(buf) & 1024)) {
	obj->SetSkill("Wearable on Back", 1);		// "WEAR_ABOUT"
	obj->SetSkill("Wearable on Left Shoulder", 2);
	obj->SetSkill("Wearable on Right Shoulder", 2);
	}
      if(strcasestr(buf, "l") || (atoi(buf) & 2048)) {
	obj->SetSkill("Wearable on Waist", 1);
	}
      if(strcasestr(buf, "m") || (atoi(buf) & 4096)) {
	obj->SetSkill("Wearable on Left Wrist", 1);
	obj->SetSkill("Wearable on Right Wrist", 2);
	}
      obj->SetShortDesc(name.c_str());

      fscanf(mudo, "%d %d %d %d\n", val+0, val+1, val+2, val+3);

      if(tp == 1) { //LIGHTS
	if(val[2] > 1) {
	  obj->SetSkill("Lightable", val[2] * 60);	// Total Lit Minutes
	  obj->SetSkill("Brightness", 100);		// All TBAMUD Lights
	  }
	else {
	  obj->SetSkill("Light Source", 100);		// All TBAMUD Lights
	  }
        }
      else if(tp == 9) { // ARMOR
	obj->SetAttribute(0, val[0]);
	}
      else if(tp == 20) { // MONEY
	if(!gold) init_gold();
	(*obj) = (*gold);
	obj->SetSkill("Quantity", val[0]);
	}
      else if(tp == 18) { // KEY
	obj->SetSkill("Key", 1000000 + onum);		// Key's "code"
	}
      else if(tp == 15) { // CONTAINER
	obj->SetSkill("Container", val[0] * 454);
	obj->SetSkill("Capacity", val[0]);

	if(!(val[1] & 4)) obj->SetSkill("Open", 1000);	//Start open?
	if(val[1] & 8) {
	  obj->SetSkill("Locked", 1);			//Start locked?
	  obj->SetSkill("Lockable", 1);			// Can it be locked?
	  }
	if(val[1] & 1) obj->SetSkill("Closeable", 1);	// Can it be closed?
	if(val[2] > 0) {
	  obj->SetSkill("Lock", 1000000 + val[2]);	// Unlocking key's code
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
      else if(tp == 2) { // SCROLL
	obj->SetSkill("Magical", val[0]);
	obj->SetSkill("Magical Scroll", val[0]);
	for(int idx=1; idx < 4; ++idx) {
	  add_tba_spell(obj, val[idx], val[0]);
	  }
	}
      else if(tp == 3) { // WAND
	obj->SetSkill("Magical", val[0]);
	obj->SetSkill("Magical Wand", val[1]);
	obj->SetSkill("Magical Charges", val[2]);
	add_tba_spell(obj, val[3], val[0]);
	}
      else if(tp == 4) { // STAFF
	obj->SetSkill("Magical", val[0]);
	obj->SetSkill("Magical Staff", val[1]);
	obj->SetSkill("Magical Charges", val[2]);
	add_tba_spell(obj, val[3], val[0]);
	}
      else if(tp == 10) { // POTION
	obj->SetSkill("Liquid Container", 1);
	obj->SetSkill("Capacity", 1);
	obj->SetSkill("Closeable", 1);
	obj->SetSkill("Perishable", 1);

	Object *liq = new Object(obj);
	liq->SetSkill("Liquid", 1);
	liq->SetSkill("Ingestible", 1);
	liq->SetWeight(10);
	liq->SetVolume(1);
	liq->SetSkill("Quantity", 1);
	liq->SetSkill("Magical", val[0]);
	liq->SetShortDesc("some liquid");
	for(int idx=1; idx < 4; ++idx) {
	  add_tba_spell(liq, val[idx], val[0]);
	  }
	}
      else if(tp == 17 || tp == 23) { // DRINKCON/FOUNTAIN
	obj->SetSkill("Liquid Container", val[0]);
	obj->SetSkill("Capacity", val[0]);
	if(tp == 23) {	//FOUNTAIN only
	  obj->SetSkill("Open", 1000);
	  obj->SetSkill("Liquid Source", 1);
	  obj->Activate();
	  }
	else {		//DRINKCON only
	  obj->SetSkill("Closeable", 1);
	  }
	if(val[1] > 0 || tp == 23) {
	  Object *liq = new Object(obj);
	  liq->SetSkill("Liquid", 1);
	  liq->SetSkill("Ingestible", 1);
	  liq->SetWeight(20);
	  liq->SetVolume(2);
	  switch(val[2]) {	// Note: *15 is correct for TBAMUD's hours/4
	    case(0): {		// WATER
	      liq->SetShortDesc("water");
	      liq->SetSkill("Drink", 10 * 15);
	      liq->SetSkill("Food", 1 * 15);
	      //liq->SetSkill("Alcohol", 0 * 15);
	      } break;
	    case(1): {		// BEER
	      liq->SetShortDesc("beer");
	      liq->SetSkill("Drink", 5 * 15);
	      liq->SetSkill("Food", 2 * 15);
	      liq->SetSkill("Alcohol", 3 * 15);
	      liq->SetSkill("Perishable", val[0] * 32);
	      } break;
	    case(2): {		// WINE
	      liq->SetShortDesc("wine");
	      liq->SetSkill("Drink", 5 * 15);
	      liq->SetSkill("Food", 2 * 15);
	      liq->SetSkill("Alcohol", 5 * 15);
	      } break;
	    case(3): {		// ALE
	      liq->SetShortDesc("ale");
	      liq->SetSkill("Drink", 5 * 15);
	      liq->SetSkill("Food", 2 * 15);
	      liq->SetSkill("Alcohol", 2 * 15);
	      liq->SetSkill("Perishable", val[0] * 16);
	      } break;
	    case(4): {		// DARKALE
	      liq->SetShortDesc("dark ale");
	      liq->SetSkill("Drink", 5 * 15);
	      liq->SetSkill("Food", 2 * 15);
	      liq->SetSkill("Alcohol", 1 * 15);
	      liq->SetSkill("Perishable", val[0] * 8);
	      } break;
	    case(5): {		// WHISKY
	      liq->SetShortDesc("whisky");
	      liq->SetSkill("Drink", 4 * 15);
	      liq->SetSkill("Food", 1 * 15);
	      liq->SetSkill("Alcohol", 6 * 15);
	      } break;
	    case(6): {		// LEMONADE
	      liq->SetShortDesc("lemonaid");
	      liq->SetSkill("Drink", 8 * 15);
	      liq->SetSkill("Food", 1 * 15);
	      //liq->SetSkill("Alcohol", 0 * 15);
	      liq->SetSkill("Perishable", val[0] * 4);
	      } break;
	    case(7): {		// FIREBRT
	      liq->SetShortDesc("firebreather");
	      //liq->SetSkill("Drink", 0 * 15);
	      //liq->SetSkill("Food", 0 * 15);
	      liq->SetSkill("Alcohol", 10 * 15);
	      } break;
	    case(8): {		// LOCALSPC
	      liq->SetShortDesc("local brew");
	      liq->SetSkill("Drink", 3 * 15);
	      liq->SetSkill("Food", 3 * 15);
	      liq->SetSkill("Alcohol", 3 * 15);
	      } break;
	    case(9): {		// SLIME
	      liq->SetShortDesc("slime");
	      liq->SetSkill("Dehydrate Effect", 8 * 15);
	      liq->SetSkill("Food", 4 * 15);
	      //liq->SetSkill("Alcohol", 0 * 15);
	      } break;
	    case(10): {		// MILK
	      liq->SetShortDesc("milk");
	      liq->SetSkill("Drink", 6 * 15);
	      liq->SetSkill("Food", 3 * 15);
	      //liq->SetSkill("Alcohol", 0 * 15);
	      liq->SetSkill("Perishable", val[0]);
	      } break;
	    case(11): {		// TEA
	      liq->SetShortDesc("tea");
	      liq->SetSkill("Drink", 6 * 15);
	      liq->SetSkill("Food", 1 * 15);
	      //liq->SetSkill("Alcohol", 0 * 15);
	      } break;
	    case(12): {		// COFFE
	      liq->SetShortDesc("coffee");
	      liq->SetSkill("Drink", 6 * 15);
	      liq->SetSkill("Food", 1 * 15);
	      //liq->SetSkill("Alcohol", 0 * 15);
	      } break;
	    case(13): {		// BLOOD
	      liq->SetShortDesc("blood");
	      liq->SetSkill("Dehydrate Effect", 1 * 15);
	      liq->SetSkill("Food", 2 * 15);
	      //liq->SetSkill("Alcohol", 0 * 15);
	      liq->SetSkill("Perishable", val[0] * 2);
	      } break;
	    case(14): {		// SALTWATER
	      liq->SetShortDesc("salt water");
	      liq->SetSkill("Dehydrate Effect", 2 * 15);
	      liq->SetSkill("Food", 1 * 15);
	      //liq->SetSkill("Alcohol", 0 * 15);
	      } break;
	    case(15): {		// CLEARWATER
	      liq->SetShortDesc("clear water");
	      liq->SetSkill("Drink", 13 * 15);
	      //liq->SetSkill("Food", 0 * 15);
	      //liq->SetSkill("Alcohol", 0 * 15);
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
	obj->SetSkill("Food", val[0] * 60);
	if(val[3] != 0) {
	  obj->SetSkill("Poisionous", val[3]);
	  }
	}
      else if(tp == 22) { // BOAT
	obj->SetSkill("Enterable", 1);
	obj->SetSkill("Open", 1000);
	obj->SetSkill("Vehicle", 4); //Unpowered (1=0), Calm Water (4=1).
	}
      else if(tp == 5) { // WEAPON
        int wreach = 0;						// default

        int skmatch = get_weapon_type("Short Crushing");	// default
	if(val[3] == 1)						// "stings"
		skmatch = get_weapon_type("Short Piercing");
	else if(val[3] == 2)					// "whips"
		skmatch = get_weapon_type("Whips");
	else if(val[3] == 3)					// "slashes"
		skmatch = get_weapon_type("Short Blades");
	else if(val[3] == 4)					// "bites"
		skmatch = get_weapon_type("Short Piercing");
	else if(val[3] == 8)					// "claws"
		skmatch = get_weapon_type("Short Blades");
	else if(val[3] == 9)					// "mauls"
		skmatch = get_weapon_type("Short Cleaves");
	else if(val[3] == 11)					// "pierces"
		skmatch = get_weapon_type("Short Piercing");
	else if(val[3] == 12)					// "blasts"
		skmatch = get_weapon_type("Shotguns");
	else if(val[3] == 13)					// "punches"
		skmatch = get_weapon_type("Punching");
	else if(val[3] == 14)					// "stabs"
		skmatch = get_weapon_type("Short Piercing");

	if(matches(obj->ShortDesc(), "lance")) {
	  skmatch = get_weapon_type("Two-Handed Piercing");
	  }
	else if(matches(obj->ShortDesc(), "spear")) {
	  skmatch = get_weapon_type("Two-Handed Piercing");
	  }
	else if(matches(obj->ShortDesc(), "pitchfork")) {
	  skmatch = get_weapon_type("Two-Handed Piercing");
	  }
	else if(matches(obj->ShortDesc(), "pitch fork")) {
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
	else if(matches(obj->ShortDesc(), "ball and chain")) {
	  skmatch = get_weapon_type("Two-Handed Flails");
	  }
	else if(matches(obj->ShortDesc(), "whip")) {
	  skmatch = get_weapon_type("Whips");
	  }
	else if(matches(obj->ShortDesc(), "bullwhip")) {
	  skmatch = get_weapon_type("Whips");
	  }
	else if(matches(obj->ShortDesc(), "scourge")) {
	  skmatch = get_weapon_type("Whips");
	  }
	else if(matches(obj->ShortDesc(), "crop")) {
	  skmatch = get_weapon_type("Whips");
	  }
	else if(matches(obj->ShortDesc(), "chain")) {
	  skmatch = get_weapon_type("Whips");
	  }
	else if(matches(obj->ShortDesc(), "batleth")) {
	  skmatch = get_weapon_type("Two-Handed Blades");
	  }
	else if(matches(obj->ShortDesc(), "lochaber")) {
	  skmatch = get_weapon_type("Two-Handed Cleaves");
	  }
	else if(matches(obj->ShortDesc(), "claymore")) {
	  skmatch = get_weapon_type("Two-Handed Blades");
	  }
	else if(matches(obj->ShortDesc(), "knife")) {
	  skmatch = get_weapon_type("Short Blades");
	  }
	else if(matches(obj->ShortDesc(), "shard")) {
	  skmatch = get_weapon_type("Short Blades");
	  }
	else if(matches(obj->ShortDesc(), "hook")) {
	  skmatch = get_weapon_type("Short Piercing");
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
	else if(matches(obj->ShortDesc(), "needles")) {
	  skmatch = get_weapon_type("Short Piercing");
	  }
	else if(matches(obj->ShortDesc(), "awl")) {
	  skmatch = get_weapon_type("Short Piercing");
	  }
	else if(matches(obj->ShortDesc(), "screwdriver")) {
	  skmatch = get_weapon_type("Short Piercing");
	  }
	else if(matches(obj->ShortDesc(), "stiletto")) {
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
	else if(matches(obj->ShortDesc(), "epee")) {
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
	else if(matches(obj->ShortDesc(), "longsword")) {
	  skmatch = get_weapon_type("Long Blades");
	  }
	else if(matches(obj->ShortDesc(), "straightsword")) {
	  skmatch = get_weapon_type("Long Blades");
	  }
	else if(matches(obj->ShortDesc(), "saber")) {
	  skmatch = get_weapon_type("Long Blades");
	  }
	else if(matches(obj->ShortDesc(), "sabre")) {
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
	else if(matches(obj->ShortDesc(), "hatchet")) {
	  skmatch = get_weapon_type("Long Cleaves");
	  }
	else if(matches(obj->ShortDesc(), "pickaxe")) {
	  skmatch = get_weapon_type("Long Cleaves");
	  }
	else if(matches(obj->ShortDesc(), "hand axe")) {
	  skmatch = get_weapon_type("Short Cleaves");
	  }
	else if(matches(obj->ShortDesc(), "handaxe")) {
	  skmatch = get_weapon_type("Short Cleaves");
	  }
	else if(matches(obj->ShortDesc(), "club")) {
	  skmatch = get_weapon_type("Long Crushing");
	  }
	else if(matches(obj->ShortDesc(), "peg leg")) {
	  skmatch = get_weapon_type("Long Crushing");
	  }
	else if(matches(obj->ShortDesc(), "stick")) {
	  skmatch = get_weapon_type("Long Crushing");
	  }
	else if(matches(obj->ShortDesc(), "bat")) {
	  skmatch = get_weapon_type("Long Crushing");
	  }
	else if(matches(obj->ShortDesc(), "rod")) {
	  skmatch = get_weapon_type("Long Crushing");
	  }
	else if(matches(obj->ShortDesc(), "scepter")) {
	  skmatch = get_weapon_type("Long Crushing");
	  }
	else if(matches(obj->ShortDesc(), "sceptre")) {
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
	else if(matches(obj->ShortDesc(), "scythe")) {
	  skmatch = get_weapon_type("Two-Handed Blades");
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
	else if(matches(obj->ShortDesc(), "poleaxe")) {
	  skmatch = get_weapon_type("Two-Handed Cleaves");
	  }
	else if(matches(obj->ShortDesc(), "polearm")) {
	  skmatch = get_weapon_type("Two-Handed Cleaves");
	  }
	else if(matches(obj->ShortDesc(), "pole axe")) {
	  skmatch = get_weapon_type("Two-Handed Cleaves");
	  }
	else if(matches(obj->ShortDesc(), "pole arm")) {
	  skmatch = get_weapon_type("Two-Handed Cleaves");
	  }
	else if(matches(obj->ShortDesc(), "fishing pole")) {
	  skmatch = get_weapon_type("Whips");
	  }
	else if(matches(obj->ShortDesc(), "hammer")) {
	  skmatch = get_weapon_type("Long Crushing");
	  }
	else if(matches(obj->ShortDesc(), "maul")) {
	  skmatch = get_weapon_type("Long Crushing");
	  }
	else if(matches(obj->ShortDesc(), "mallet")) {
	  skmatch = get_weapon_type("Long Crushing");
	  }
	else if(matches(obj->ShortDesc(), "gavel")) {
	  skmatch = get_weapon_type("Short Crushing");
	  }
	else if(matches(obj->ShortDesc(), "baton")) {
	  skmatch = get_weapon_type("Short Staves");
	  }
	else if(matches(obj->ShortDesc(), "quarterstaff")) {
	  skmatch = get_weapon_type("Long Staves");
	  }
	else if(matches(obj->ShortDesc(), "shovel")) {
	  skmatch = get_weapon_type("Long Crushing");
	  }
	else if(matches(obj->ShortDesc(), "hoe")) {
	  skmatch = get_weapon_type("Long Crushing");
	  }
	else if(matches(obj->ShortDesc(), "broom")) {
	  skmatch = get_weapon_type("Two-Handed Staves");
	  }
	else if(matches(obj->ShortDesc(), "rake")) {
	  skmatch = get_weapon_type("Two-Handed Staves");
	  }
	else if(matches(obj->ShortDesc(), "mop")) {
	  skmatch = get_weapon_type("Two-Handed Staves");
	  }
	else if(matches(obj->ShortDesc(), "staff")) {
	  skmatch = get_weapon_type("Two-Handed Staves");
	  }
	else if(matches(obj->ShortDesc(), "bow")) {
	  skmatch = get_weapon_type("Archery");
	  }
	else if(matches(obj->ShortDesc(), "longbow")) {
	  skmatch = get_weapon_type("Archery");
	  }
	else if(matches(obj->ShortDesc(), "arrow")) {
	  skmatch = get_weapon_type("Archery");
	  }
	else if(matches(obj->ShortDesc(), "arrows")) {
	  skmatch = get_weapon_type("Archery");
	  }
	else if(matches(obj->ShortDesc(), "slingshot")) {
	  skmatch = get_weapon_type("Crossbow");
	  }
	else if(matches(obj->ShortDesc(), "sling shot")) {
	  skmatch = get_weapon_type("Crossbow");
	  }
	else if(matches(obj->ShortDesc(), "sling")) {
	  skmatch = get_weapon_type("Slings");
	  }
	else if(matches(obj->ShortDesc(), "dart")) {
	  skmatch = get_weapon_type("Throwing, Aero");
	  }
	else if(matches(obj->ShortDesc(), "darts")) {
	  skmatch = get_weapon_type("Throwing, Aero");
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
	else if(matches(obj->ShortDesc(), "scalpel")) {
	  skmatch = get_weapon_type("Short Blades");
	  }
	else if(matches(obj->ShortDesc(), "crook")) {
	  skmatch = get_weapon_type("Two-Handed Staves");
	  }
	else if(matches(obj->ShortDesc(), "knuckles")) {
	  skmatch = get_weapon_type("Punching");
	  }
	else if(matches(obj->ShortDesc(), "laser rifle")) {
	  skmatch = get_weapon_type("Laser Rifles");
	  }
	else if(matches(obj->ShortDesc(), "assault rifle")) {
	  skmatch = get_weapon_type("Assault Rifles");
	  }
	else if(matches(obj->ShortDesc(), "rifle")) {
	  skmatch = get_weapon_type("Rifles");
	  }
	else if(matches(obj->ShortDesc(), "phaser")) {
	  skmatch = get_weapon_type("Plasma Pistols");
	  }
	else if(matches(obj->ShortDesc(), "plasma gun")) {
	  skmatch = get_weapon_type("Plasma Pistols");
	  }
	else if(matches(obj->ShortDesc(), "laser pistol")) {
	  skmatch = get_weapon_type("Laser Pistols");
	  }
	else if(matches(obj->ShortDesc(), "pistol")) {
	  skmatch = get_weapon_type("Pistols");
	  }
	else if(matches(obj->ShortDesc(), "handgun")) {
	  skmatch = get_weapon_type("Pistols");
	  }
	else if(matches(obj->ShortDesc(), "hand gun")) {
	  skmatch = get_weapon_type("Pistols");
	  }
	else if(matches(obj->ShortDesc(), "gun")) {
	  skmatch = get_weapon_type("Shotguns");
	  }
	else if(matches(obj->ShortDesc(), "thunderbolt")) {
	  skmatch = get_weapon_type("Throwing, Aero");
	  }
	else if(matches(obj->ShortDesc(), "bola")) {
	  skmatch = get_weapon_type("Throwing, Aero");
	  }
	else if(matches(obj->ShortDesc(), "bolas")) {
	  skmatch = get_weapon_type("Throwing, Aero");
	  }
	else if(matches(obj->ShortDesc(), "pole")) {
	  skmatch = get_weapon_type("Hurling");
	  }
	else if(matches(obj->ShortDesc(), "boulder")) {
	  skmatch = get_weapon_type("Hurling");
	  }
	else if(matches(obj->ShortDesc(), "rock")) {
	  skmatch = get_weapon_type("Throwing, Non-Aero");
	  }
	else if(matches(obj->ShortDesc(), "stone")) {
	  skmatch = get_weapon_type("Throwing, Non-Aero");
	  }
	else if(matches(obj->ShortDesc(), "boomerang")) {
	  skmatch = get_weapon_type("Throwing, Aero");
	  }
	else if(matches(obj->ShortDesc(), "blowgun")) {
	  skmatch = get_weapon_type("Blowgun");
	  }
	else if(matches(obj->ShortDesc(), "crossbow")) {
	  skmatch = get_weapon_type("Crossbow");
	  }
	else if(matches(obj->ShortDesc(), "arbalest")) {
	  skmatch = get_weapon_type("Crossbow");
	  }
	else if(matches(obj->ShortDesc(), "net")) {
	  skmatch = get_weapon_type("Nets");
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
	  else if(skmatch == get_weapon_type("Whips")) wreach = 4;
	  else if(skmatch == get_weapon_type("Nets")) wreach = 4;
	  else if(skmatch == get_weapon_type("Archery")) wreach = 100;
	  else if(skmatch == get_weapon_type("Crossbow")) wreach = 100;
	  else if(skmatch == get_weapon_type("Slings")) wreach = 50;
	  else if(skmatch == get_weapon_type("Blowgun")) wreach = 20;
	  else if(skmatch == get_weapon_type("Throwing, Aero")) wreach = 20;
	  else if(skmatch == get_weapon_type("Throwing, Non-Aero")) wreach = 10;
	  else if(skmatch == get_weapon_type("Hurling")) wreach = 5;
	  else if(skmatch == get_weapon_type("Assault Rifles")) wreach = 1000;
	  else if(skmatch == get_weapon_type("Laser Rifles")) wreach = 1000;
	  else if(skmatch == get_weapon_type("Rifles")) wreach = 1000;
	  else if(skmatch == get_weapon_type("Shotguns")) wreach = 500;
	  else if(skmatch == get_weapon_type("Pistols")) wreach = 200;
	  else if(skmatch == get_weapon_type("Laser Pistols")) wreach = 200;
	  else if(skmatch == get_weapon_type("Plasma Pistols")) wreach = 200;
	  else if(skmatch == get_weapon_type("Punching")) wreach = 0;
	  else {
	    fprintf(stderr, "Warning: Using Default reach of zero for '%s'!\n",
		obj->ShortDesc());
	    }
	  }

	obj->SetSkill("WeaponType", skmatch);
//	obj->SetSkill("WeaponDamage", val[1]*val[2]);
	int sev = 0;
	int tot = (val[1] * (val[2]+1) + 1) / 2;  //Avg. TBA Dam. Rounded Up
	while(tot > sev) { ++sev; tot -= sev; }
	obj->SetSkill("WeaponForce", tot);
	obj->SetSkill("WeaponSeverity", sev);
	obj->SetSkill("WeaponReach", wreach);
	}

      int weight, value;
      fscanf(mudo, "%d %d %*[^\n\r]\n", &weight, &value);

      if(tp != 20) { // MONEY DOESN'T WORK THIS WAY
	obj->SetWeight(weight * 454);
	obj->SetVolume(weight); //FIXME: Better guess within units?
	obj->SetSize(1);
	obj->SetValue((value*valmod) / 1000);
	if(obj->short_desc == "a cashcard (card atm)") {	//Is Script Now
	  obj->SetSkill("Money", 10000);
	  }
	}

      int magresist = 0;
      while(fscanf(mudo, "%1[AET] ", buf) > 0) {
	if(buf[0] == 'A') {	//Extra Affects
	  int anum, aval;
	  fscanf(mudo, "%d %d\n", &anum, &aval);
	  switch(anum) {
	    case(1): {	// STR
	      if(aval > 0) obj->SetSkill("Strength Bonus", aval*400/powmod);
	      else if(aval < 0) obj->SetSkill("Strength Penalty", -aval*400/powmod);
	      } break;
	    case(2): {	// DEX
	      if(aval > 0) obj->SetSkill("Quickness Bonus", aval*400/powmod);
	      else if(aval < 0) obj->SetSkill("Quickness Penalty", -aval*400/powmod);
	      } break;
	    case(3): {	// INT
	      if(aval > 0) obj->SetSkill("Intelligence Bonus", aval*400/powmod);
	      else if(aval < 0) obj->SetSkill("Intelligence Penalty", -aval*400/powmod);
	      } break;
	    case(4): {	// WIS
	      if(aval > 0) obj->SetSkill("Willpower Bonus", aval*400/powmod);
	      else if(aval < 0) obj->SetSkill("Willpower Penalty", -aval*400/powmod);
	      } break;
	    case(5): {	// CON
	      if(aval > 0) obj->SetSkill("Body Bonus", aval*400/powmod);
	      else if(aval < 0) obj->SetSkill("Body Penalty", -aval*400/powmod);
	      } break;
	    case(6): {	// CHA
	      if(aval > 0) obj->SetSkill("Charisma Bonus", aval*400/powmod);
	      else if(aval < 0) obj->SetSkill("Charisma Penalty", -aval*400/powmod);
	      } break;
//	    case(7): {	// CLASS (Even TBAMUD Doesn't Use This!)
//	      } break;
//	    case(8): {	// LEVEL (Even TBAMUD Doesn't Use This!)
//	      } break;
	    case(9): {	// AGE
	      if(aval > 0) obj->SetSkill("Youth Penalty", aval/powmod);
	      else if(aval < 0) obj->SetSkill("Youth Bonus", -aval/powmod);
	      } break;
//	    case(10): {	// CHAR_WEIGHT (Unused by main TBAMUD data)
//	      } break;
//	    case(11): {	// CHAR_HEIGHT (Unused by main TBAMUD data)
//	      } break;
	    case(12): {	// MANA
	      if(aval > 0) obj->SetSkill("Magic Force Bonus", aval*100/powmod);
	      else if(aval < 0) obj->SetSkill("Magic Force Penalty", -aval*100/powmod);
	      } break;
	    case(13): {	// HIT
	      if(aval > 0) obj->SetSkill("Resilience Bonus", aval*100/powmod);
	      else if(aval < 0) obj->SetSkill("Resilience Penalty", -aval*100/powmod);
	      } break;
	    case(14): {	// MOVE
	      if(aval > 0) obj->SetSkill("Encumbrance Bonus", aval*20/powmod);
	      else if(aval < 0) obj->SetSkill("Encumbrance Penalty", -aval*20/powmod);
	      } break;
//	    case(15): {	// GOLD (Even TBAMUD Doesn't Use This!)
//	      } break;
//	    case(16): {	// EXP (Even TBAMUD Doesn't Use This!)
//	      } break;
	    case(17): {	// AC
	      if(aval > 0) obj->SetSkill("Evasion Penalty", aval*400/powmod);
	      else if(aval < 0) obj->SetSkill("Evasion Bonus", -aval*400/powmod);
	      } break;
	    case(18): {	// HITROLL
	      if(aval > 0) obj->SetSkill("Accuracy Bonus", aval*400/powmod);
	      else if(aval < 0) obj->SetSkill("Accuracy Penalty", -aval*400/powmod);
	      } break;
	    case(19): {	// DAMROLL
	      if(aval > 0) obj->SetSkill("Damage Bonus", aval*400/powmod);
	      else if(aval < 0) obj->SetSkill("Damage Penalty", -aval*400/powmod);
	      } break;
	    case(20): {	// SAVING_PARA
	      magresist += (aval*400/powmod);
	      } break;
	    case(21): {	// SAVING_ROD
	      magresist += (aval*400/powmod);
	      } break;
	    case(22): {	// SAVING_PETRI
	      magresist += (aval*400/powmod);
	      } break;
	    case(23): {	// SAVING_BREATH
	      magresist += (aval*400/powmod);
	      } break;
	    case(24): {	// SAVING_SPELL
	      magresist += (aval*400/powmod);
	      } break;
	    }
	  }
	else if(buf[0] == 'T') {	//Triggers
	  int tnum = 0;
	  fscanf(mudo, "%d\n", &tnum);
	  if(tnum > 0 && bynumtrg.count(tnum) > 0) {
	    Object *trg = new Object(*(bynumtrg[tnum]));
	    trg->SetParent(obj);
	    todotrg.push_back(trg);
	//    fprintf(stderr, "Put Trg \"%s\" on Obj \"%s\"\n",
	//	trg->Desc(), obj->ShortDesc()
	//	);
	    }
	  }
	else if(buf[0] == 'E') {	//Extra Affects
	  fscanf(mudo, "%*[^~]");	//FIXME: Load These!
	  fscanf(mudo, "~%*[\n\r]");
	  fscanf(mudo, "%*[^~]");	//FIXME: Load These!
	  fscanf(mudo, "~%*[\n\r]");
	  }
	else {			//Extra Descriptions FIXME: Handle!
	  fprintf(stderr, "ERROR: Unknown tag!\n");
	  }
	}
      if(magresist > 0) obj->SetSkill("Magic Resistance", magresist);
      else if(magresist < 0) obj->SetSkill("Magic Vulnerability", -magresist);

      fscanf(mudo, "%*[^#$]");
      }
    fclose(mudo);
    }
  }

void Object::TBALoadWLD(const char *fn) {
  FILE *mud = fopen(fn, "r");
  int zone = 0, offset = strlen(fn) - 5; //Chop off the .wld
  while(isdigit(fn[offset])) --offset;
  zone = atoi(fn+offset+1);
  if(mud) {
    //fprintf(stderr, "Loading TBA Realm from \"%s\"\n", fn);
    while(1) {
      int onum;
      if(fscanf(mud, " #%d\n", &onum) < 1) break;
      //fprintf(stderr, "Loading room #%d\n", onum);

      Object *obj = new Object(this);
      olist.push_back(obj);
      obj->SetSkill("TBARoom", 1000000+onum);
      bynumwld[onum] = obj;

      obj->SetWeight(-1);
      obj->SetVolume(-1);
      obj->SetSize(-1);
      obj->SetValue(-1);

      memset(buf, 0, 65536);
      fscanf(mud, "%65535[^~\n]~\n", buf);
      for(char *ch=buf; (*ch); ++ch) if((*ch) == ';') (*ch) = ',';
      obj->SetShortDesc(buf);
      //fprintf(stderr, "Loaded TBA Room with Name = %s\n", buf);

      memset(buf, 0, 65536);
      fscanf(mud, "%65535[^~]~\n", buf);
      for(char *ch=buf; (*ch); ++ch) if((*ch) == ';') (*ch) = ',';
      obj->SetDesc(buf);
      //fprintf(stderr, "Loaded TBA Room with Desc = %s\n", buf);

      int val;
      fscanf(mud, "%*d %65535[^ \t\n] %d %*[^\n\r]\n", buf, &val);
		//FIXME: TBA's extra 3 flags variables (ignored now)?
      if(val == 6) obj->SetSkill("WaterDepth", 1);	// WATER_SWIM
      else if(val == 7) obj->SetSkill("WaterDepth", 2);	// WATER_NOSWIM
      else if(val == 8) obj->SetSkill("WaterDepth", 3);	// UNDERWATER

      string name = obj->ShortDesc();
      if(name.find("Secret") >= 0 && name.find("Secret") < name.length()) {
        obj->SetSkill("Secret", 1000000+onum);
	}

      obj->SetSkill("Translucent", 1000);	// Full sky, by default
      if(strcasestr(buf, "d") || (atoi(buf) & 8)) { //INDOORS
	obj->SetSkill("Translucent", 200);	// Windows (unless DARK)
	obj->SetSkill("Light Source", 100);	// Torches (unless DARK)
	}
      if(strcasestr(buf, "a") || (atoi(buf) & 1)) { //DARK
	obj->SetSkill("Translucent", 0);	// No sky, no windows
	obj->SetSkill("Light Source", 0);	// No torches
	}
      if(strcasestr(buf, "b") || (atoi(buf) & 2)) { //DEATH
	obj->SetSkill("Secret", 1000000+onum);
//	obj->SetSkill("Hazardous", 2);		//FIXME: Actually Dangerous?
	}
      if(strcasestr(buf, "c") || (atoi(buf) & 4)) { //NOMOB
        obj->SetSkill("TBAZone", 999999);
	}
      else {
        obj->SetSkill("TBAZone", 1000000 + zone);
	}
      if(strcasestr(buf, "e") || (atoi(buf) & 16)) { //PEACEFUL
        obj->SetSkill("Peaceful", 1000);
	}
      if(strcasestr(buf, "f") || (atoi(buf) & 32)) { //SOUNDPROOF
        obj->SetSkill("Soundproof", 1000);
	}
//      if(strcasestr(buf, "g") || (atoi(buf) & 64)) { //NOTRACK
//	//FIXME: Implement
//	}
      if(strcasestr(buf, "h") || (atoi(buf) & 128)) { //NOMAGIC
        obj->SetSkill("Magic Dead", 1000);
	}
//      if(strcasestr(buf, "i") || (atoi(buf) & 256)) { //TUNNEL
//	//FIXME: Implement
//	}
//      if(strcasestr(buf, "j") || (atoi(buf) & 512)) { //PRIVATE
//	//FIXME: Implement
//	}
//      if(strcasestr(buf, "k") || (atoi(buf) & 1024)) { //GODROOM
//	//FIXME: Implement
//	}

      while(1) {
	int dnum, tnum, tmp, tmp2;
	fscanf(mud, "%c", buf);
	if(buf[0] == 'D') {
	  fscanf(mud, "%d\n", &dnum);

	  memset(buf, 0, 65536);
	  fscanf(mud, "%65535[^~]", buf);
	  fscanf(mud, "~%*[\n\r]");
	  memset(buf, 0, 65536);
	  fscanf(mud, "%65535[^~]", buf);
	  fscanf(mud, "~%*[\n\r]");
	  nmnum[dnum][obj] = buf;

	  fscanf(mud, "%d %d %d\n", &tmp, &tmp2, &tnum);

	  tonum[dnum][obj] = tnum;
	  tynum[dnum][obj] = tmp;
	  knum[dnum][obj] = tmp2;
	  }
	else if(buf[0] == 'E') {
	  fscanf(mud, "%*[^~]");	//FIXME: Load These!
	  fscanf(mud, "~%*[\n\r]");
	  fscanf(mud, "%*[^~]");	//FIXME: Load These!
	  fscanf(mud, "~%*[\n\r]");
	  }
	else if(buf[0] != 'S') {
	  fprintf(stderr, "Warning, didn't see an ending S!\n");
	  }
	else {
	  break;
	  }
	}
      { int tnum;
	while(fscanf(mud, " T %d\n", &tnum) > 0) {
	  if(tnum > 0 && bynumtrg.count(tnum) > 0) {
	    Object *trg = new Object(*(bynumtrg[tnum]));
	    trg->SetParent(obj);
	    todotrg.push_back(trg);
	//    fprintf(stderr, "Put Trg \"%s\" on Room \"%s\"\n",
	//	trg->Desc(), obj->ShortDesc()
	//	);
	    }
	  }
	}
      }

    vector<Object*>::iterator ob = olist.begin();
    for(; ob != olist.end(); ++ob) {
      for(int dir=0; dir<6; ++dir) {
	if(tonum[dir].count(*ob)) {
	  int tnum = tonum[dir][*ob];
	  if(bynumwld.count(tnum)) {
	    Object *nobj = NULL;
	    Object *nobj2 = NULL;
	    string des, nm = dirname[dir];

	    typeof((*ob)->Contents()) cont = (*ob)->Contents();
	    typeof(cont.begin()) cind;
	    for(cind = cont.begin(); cind != cont.end(); ++cind) {
	      if(string((*cind)->ShortDesc()) == "a passage exit") {
		if((*cind)->ActTarg(ACT_SPECIAL_MASTER)->Parent()
			== bynumwld[tnum]) {
		  nobj = (*cind);
		  nobj2 = (*cind)->ActTarg(ACT_SPECIAL_MASTER);
		  }
		}
	      else if((*cind)->ActTarg(ACT_SPECIAL_LINKED)) {
		if((*cind)->ActTarg(ACT_SPECIAL_LINKED)->Parent()
			== bynumwld[tnum]) {
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
	      nobj2->SetParent(bynumwld[tnum]);
	      nobj2->SetShortDesc("a passage exit");
	      nobj2->SetDesc("A passage exit.");
	      nobj2->SetSkill("Invisible", 1000);
	      }
	    else {
	      nobj->SetSkill("Invisible", 0);
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
		nobj->SetSkill("Lock", 1000000 + knum[dir][*ob]);
		}
	      }
	    else {
	      des = string("A passage ") + dirname[dir] + " is here.";
	      }
	    nobj->SetShortDesc(nm.c_str());
	    nobj->SetDesc(des.c_str());
	    nobj->SetSkill("Open", 1000);
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
    fprintf(stderr, "Error: No TBA Realm \"%s\"\n", fn);
    }
  }

const char *base = "'^&*abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ ";
static set<string> parse_tba_shop_rules(string rules) {
  set<string> ret;
  if(rules[0]) {
//    fprintf(stderr, "Initial: '%s'\n", rules.c_str());
    size_t done = rules.find_first_not_of(base);
    while(done != string::npos) {
      if(rules[done] == '|' || rules[done] == '+') {
	string first = rules.substr(0, done);
	trim_string(first);
	ret.insert(first);
//	fprintf(stderr, "  Done: '%s'\n", first.c_str());
	rules = rules.substr(done+1);
	trim_string(rules);
	done = rules.find_first_not_of(base);
	}
      else if(rules[done] == '(' || rules[done] == '[') {
	size_t end = rules.find_first_of(")]");
	if(end == string::npos) end = rules.length();
	set<string> tmp = parse_tba_shop_rules(rules.substr(done+1));
	set<string>::iterator next = tmp.begin();
	for(; next != tmp.end(); ++next) {
	  ret.insert((*next) + rules.substr(end+1));
//	  fprintf(stderr, "  Built: '%s'\n",
//		((*next) + rules.substr(end+1)).c_str()
//		);
	  }
	return ret;	//FIXME: Handled multiple ()()()....
	}
      else if(rules[done] == ')' || rules[done] == ']') {
	string first = rules.substr(0, done);
	trim_string(first);
//	fprintf(stderr, "  Done: '%s'\n", first.c_str());
	ret.insert(first);
	return ret;	//End of sub-call
	}
      else {
	fprintf(stderr, "Warning: Can't handle shop rule fragment: '%s'\n",
		rules.c_str()
		);
	done = string::npos;
	}
      }
//    fprintf(stderr, "  Done: '%s'\n", rules.c_str());
    ret.insert(rules);
    }
  return ret;
  }

void Object::TBALoadSHP(const char *fn) {
  FILE *mud = fopen(fn, "r");
  if(mud) {
    Object *vortex = NULL;
    if(fscanf(mud, "CircleMUD v3.0 Shop File~%65535[\n\r]", buf) > 0) {
      while(1) {
	int val, kpr;
	if(!fscanf(mud, "#%d~\n", &val)) break;  // Shop Number
	//fprintf(stderr, "Loaded shop #%d\n", val);

	vortex = new Object;
	vortex->SetShortDesc("a shopkeeper vortex");
	vortex->SetDesc("An advanced wormhole that shopkeeper's use.");
	vortex->SetSkill("Vortex", 1); // Mark it as a shopkeeper Vortex.
	vortex->SetSkill("Invisible", 1000);
	vortex->SetSkill("Perishable", 1);
	vortex->SetSkill("Wearable on Right Shoulder", 1);
	vortex->SetSkill("Wearable on Left Shoulder", 2);

	fscanf(mud, "%d\n", &val);  // Item sold
	while(val >= 0) {
	  if(val != 0 && bynumobj.count(val) == 0) {
	    fprintf(stderr, "Error: Shop's item #%d does not exist!\n", val);
	    }
	  else if(val != 0) {
	    Object *item = new Object(*(bynumobj[val]));
	    Object *item2 = dup_tba_obj(item);
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
	fscanf(mud, "%65535[^\n\r]\n", buf);  // Item types bought
	val = atoi(buf);
	list<string> types;
	while(val >= 0) {
	  types.push_back(string(buf));
	  memset(buf, 0, 65536);
	  fscanf(mud, "%65535[^\n\r]\n", buf);  // Item types bought
	  val = atoi(buf);
	  }

	memset(buf, 0, 65536);
	for(int ctr=0; ctr<8; ++ctr) {
	  fscanf(mud, "%255[^\n\r]\n", buf+strlen(buf));
	  }

	memset(buf, 0, 65536);
	fscanf(mud, "%65535[^\n\r]\n", buf);  // Shop Bitvectors

	fscanf(mud, "%d\n", &kpr);  // Shopkeeper!
	Object *keeper = NULL;
	if(bynummobinst.count(kpr)) keeper = bynummobinst[kpr];

	memset(buf, 0, 65536);
	fscanf(mud, "%65535[^\n\r]\n", buf);  // With Bitvectors

	fscanf(mud, "%d\n", &val);  // Shop rooms
	while(val >= 0) {
	  fscanf(mud, "%d\n", &val);  // Shop rooms
	  }

	fscanf(mud, "%*d\n");  // Open time
	fscanf(mud, "%*d\n");  // Close time
	fscanf(mud, "%*d\n");  // Open time
	fscanf(mud, "%*d\n");  // Close time

	if(keeper) {
	  string picky = "";
	  keeper->SetSkill("Sell Profit", (int)(num*1000.0+0.5));

	  list<string>::iterator type = types.begin();	//Buy Types
	  for(; type != types.end(); ++type) {
	    for(unsigned int ctr=1; isalpha((*type)[ctr]); ++ctr) {
	      (*type)[ctr] = tolower((*type)[ctr]);
	      }

	    string extra = *type;
	    int itnum = atoi(extra.c_str());
	    if(itnum > 0) {
	      while(isdigit(extra[0])) extra = extra.substr(1);
	      }
	    else {
	      while(isgraph(extra[0])) extra = extra.substr(1);
	      }
	    while(isspace(extra[0])) extra = extra.substr(1);

	    if(itnum == 1) (*type) = "Light";
	    else if(itnum == 2) (*type) = "Scroll";
	    else if(itnum == 3) (*type) = "Wand";
	    else if(itnum == 4) (*type) = "Staff";
	    else if(itnum == 5) (*type) = "Weapon";
	    else if(itnum == 6) (*type) = "Fire Weapon";
	    else if(itnum == 7) (*type) = "Missile";
	    else if(itnum == 8) (*type) = "Treasure";
	    else if(itnum == 9) (*type) = "Armor";
	    else if(itnum == 10) (*type) = "Potion";
	    else if(itnum == 11) (*type) = "Worn";
	    else if(itnum == 12) (*type) = "Other";
	    else if(itnum == 13) (*type) = "Trash";
	    else if(itnum == 14) (*type) = "Trap";
	    else if(itnum == 15) (*type) = "Container";
	    else if(itnum == 16) (*type) = "Note";
	    else if(itnum == 17) (*type) = "Liquid Container";
	    else if(itnum == 18) (*type) = "Key";
	    else if(itnum == 19) (*type) = "Food";
	    else if(itnum == 20) (*type) = "Money";
	    else if(itnum == 21) (*type) = "Pen";
	    else if(itnum == 22) (*type) = "Boat";
	    else if(itnum == 23) (*type) = "Fountain";
	    else if(itnum == 55) (*type) = "Cursed";	//According To: Rumble

	    if(extra[0]) {
	      //fprintf(stderr, "Rule: '%s'\n", extra.c_str());
	      set<string> extras = parse_tba_shop_rules(extra);
	      set<string>::iterator ex = extras.begin();
	      for(; ex != extras.end(); ++ex) {
		//fprintf(stderr, "Adding: 'Accept %s'\n", ex->c_str());
		//keeper->SetSkill("Accept " + (*ex), 1);
		picky += ((*type) + ": " + (*ex) + "\n");
		}
	      }
	    else {
	      picky += (*type) + ": all\n";
	      }

	    if((*type) != "0"
		&& (*type) != "Light"
		&& (*type) != "Scroll"
		&& (*type) != "Wand"
		&& (*type) != "Staff"
		&& (*type) != "Weapon"
		&& (*type) != "Fire Weapon"
		&& (*type) != "Missile"
		&& (*type) != "Treasure"
		&& (*type) != "Armor"
		&& (*type) != "Potion"
		&& (*type) != "Worn"
		&& (*type) != "Other"
		&& (*type) != "Trash"
		&& (*type) != "Trap"
		&& (*type) != "Container"
		&& (*type) != "Note"
		&& (*type) != "Liquid Container"
		&& (*type) != "Key"
		&& (*type) != "Food"
		&& (*type) != "Money"
		&& (*type) != "Pen"
		&& (*type) != "Boat"
		&& (*type) != "Fountain"
		) {
	      fprintf(stderr,
		"Warning: Can't handle %s's buy target: '%s'\n",
		keeper->Name(), type->c_str()
		);
	      }
	    else if((*type) != "0") {	//Apparently 0 used for "Ignore This"
	      keeper->SetSkill(
		string("Buy ") + (*type),
		(int)(num2*1000.0+0.5)
		);
	      }
	    }
	  vortex->long_desc = picky;
	  vortex->SetParent(keeper);
	  keeper->AddAct(ACT_WEAR_RSHOULDER, vortex);
	  }
	else {
	  vortex->Recycle();
	  fprintf(stderr, "Warning: Can't find shopkeeper #%d!\n", kpr);
	  }
	}
      }
    else if(fscanf(mud, "%1[$]", buf) < 1) {	//Not a Null Shop File!
      fprintf(stderr, "Error: '%s' is not a CircleMUD v3.0 Shop File!\n", fn);
      }
    fclose(mud);
    }
  else {
    fprintf(stderr, "Error: '%s' does not exist!\n", fn);
    }
  }

void Object::TBALoadTRG(const char *fn) {	//Triggers
  FILE *mud = fopen(fn, "r");
  if(mud) {
    int tnum = -1;
    while(fscanf(mud, " #%d", &tnum) > 0) {
      Object *script = NULL;
      script = new Object();
      bynumtrg[tnum] = script;
      script->SetSkill("Invisible", 1000);
      script->SetSkill("TBAScript", 1000000 + tnum);
      script->SetShortDesc("A tbaMUD trigger script.");
      //fprintf(stderr, "Loading #%d\n", tnum);
      fscanf(mud, " %65535[^~]", buf);		//Trigger Name - Discarded!
      //script->SetDesc(buf);
      fscanf(mud, "~");

      int atype = -1, ttype = -1, narg;
      fscanf(mud, " %d %65535s %d", &atype, buf, &narg);
      ttype = tba_bitvec(buf);					//Trigger Types
      atype = 1 << (atype + 24);				//Attach Type
      script->SetSkill("TBAScriptType", atype | ttype);		//Combined
      script->SetSkill("TBAScriptNArg", narg);			//Numeric Arg

      fscanf(mud, " %65536[^~]", buf);		//Text argument!
      script->SetDesc(buf);

      fscanf(mud, "~");
      fscanf(mud, "%*[\n\r]");		//Go to next Line, don't eat spaces.
      fscanf(mud, "%[^~]~", buf);	//Command List (Multi-Line)
      while(buf[strlen(buf)-1] != '\n' && (!feof(mud))) {	//~ in Middle
	buf[strlen(buf)+1] = 0;
	buf[strlen(buf)] = '~';
	fscanf(mud, "%[^~]~", buf+strlen(buf));
	}
      script->SetLongDesc(buf);
      if(strstr(buf,
		"* Check the direction the player must go to enter the guild."
		)) {
	char dir[16];
	char *dirp = strstr(buf, "if %direction% == ");
	if(dirp) sscanf(dirp + strlen("if %direction% == "), "%s", dir);

	char cls[16];
	char *clsp = strstr(buf, "if %actor.class% != ");
	if(clsp) sscanf(clsp + strlen("if %actor.class% != "), "%s", cls);

	if(dirp) {
	  if(clsp) {
//	    fprintf(stderr,
//		"%d appears to be a '%s' %s-guild guard trigger.\n",
//		tnum, dir, cls);
	    }
	  else {
//	    fprintf(stderr,
//		"%d appears to be a '%s' direction guard trigger.\n",
//		tnum, dir);
	    }
	  }
	++unhandled_trig;	//This is NOT really handled yet.
	}
      else if(strstr(buf, "if %direction% == ")) {
//	fprintf(stderr, "%d appears to be a direction trigger.\n", tnum);
	++unhandled_trig;	//This is NOT really handled yet.
	}
      else if(0) {
	}
      else {
	++unhandled_trig;	//FIXME: Handle some more!
	}
      }
    fclose(mud);
    }
  else {
    fprintf(stderr, "Error: '%s' does not exist!\n", fn);
    }
  }
