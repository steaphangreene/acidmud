#include <cstdlib>
#include <cstring>

#include "object.h"
#include "mind.h"
#include "version.h"

static char buf[65536];
static vector<Object*> todo;
static map<int, Object*> num2obj;
static map<Object*, int> obj2num;

Object *getbynum(int num) {
  if(num2obj.count(num) < 1) return NULL;
  return num2obj[num];
  }

int getnum(Object *obj) {
  static int curnum = 0;
  if(!obj2num.count(obj)) obj2num[obj] = ++curnum;
  return obj2num[obj];
  }

int Object::Save(const char *filename) {
  num2obj[0] = NULL;
  obj2num[NULL] = 0;

  FILE *fl = fopen(filename, "w");
  if(!fl) return -1;

  fprintf(fl, "%.8X\n", CurrentVersion.savefile_version_object);

  if(SaveTo(fl)) { fclose(fl); return -1; }

  fprintf(fl, "%d\n", getnum(get_start_room()));

  fclose(fl);
  return 0;
  }

int Object::SaveTo(FILE *fl) {
  //fprintf(stderr, "Saving %s\n", Name());

  fprintf(fl, "%d\n", getnum(this));
  fprintf(fl, "%s;\n", short_desc.c_str());
  fprintf(fl, "%s;\n", desc.c_str());
  fprintf(fl, "%s;\n", long_desc.c_str());

  fprintf(fl, "%d %d %d %d %c;\n", weight, size, volume, value, gender);

  fprintf(fl, "%d", exp);
  typeof(completed.begin()) ind;
  for(ind=completed.begin(); ind != completed.end(); ++ind) {
    fprintf(fl, ";%ld", *ind);
    }

  fprintf(fl, " %d\n", sexp);

  fprintf(fl, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d;%d\n",
	att[0], att[1], att[2], att[3], att[4], att[5], att[6], att[7],
	phys, stun, stru, IsActive());

  map<AtomString,int>::const_iterator sk = skills.begin();
  for(; sk != skills.end(); ++sk)
    fprintf(fl, "|%s|%d", sk->first.c_str(), sk->second);
  if(cur_skill != AtomString("")) {	//Added current skill to end in v0x13
    fprintf(fl, "|%s", cur_skill.c_str());
    }
  fprintf(fl, ";\n");

  fprintf(fl, "%d\n", (int)(contents.size()));
  typeof(contents.begin()) cind;
  for(cind = contents.begin(); cind != contents.end(); ++cind) {
    fprintf(fl, "%d\n", getnum(*cind));
    }

  fprintf(fl, "%d\n", pos);

  fprintf(fl, "%d\n", (int)(act.size()));
  map<act_t,Object*>::iterator aind;
  for(aind = act.begin(); aind != act.end(); ++aind) {
    fprintf(fl, "%d;%d\n", aind->first, getnum(aind->second));
    }

  fprintf(fl, "\n");

  for(cind = contents.begin(); cind != contents.end(); ++cind) {
    (*cind)->SaveTo(fl);
    }

  //fprintf(stderr, "Saved %s\n", Name());
  return 0;
  }

static unsigned int ver;
int Object::Load(const char *fn) {
  num2obj[0] = NULL;
  obj2num[NULL] = 0;

  FILE *fl = fopen(fn, "r");
  if(!fl) return -1;

  fscanf(fl, "%X\n", &ver);

  todo.clear();
  if(LoadFrom(fl)) { fclose(fl); return -1; }

  vector<Object*>::iterator ind;
  for(ind = todo.begin(); ind != todo.end(); ++ind) {
    map<act_t,Object*>::iterator aind = (*ind)->act.begin();
    for(; aind != (*ind)->act.end(); ++aind) {
      /* Decode the Object Number from a pointer, Encoded in LoadFrom() */
      int num = int(aind->second - ((Object *)(NULL)));
      aind->second = num2obj[num];
      if(aind->first == ACT_FIGHT)
	(*ind)->BusyFor(500, (*ind)->Tactics().c_str());
      }
    if((*ind)->IsUsing("Lumberjack")) {		//FIXME: All long-term skills?
      (*ind)->BusyFor(500, "use Lumberjack");
      }
    }
  todo.clear();

  int num;
  fscanf(fl, "%d\n", &num);
  set_start_room(num2obj[num]);

  fclose(fl);
  return 0;
  }

int Object::LoadFrom(FILE *fl) {
  //static string debug_indent = "";

  int num, res;
  fscanf(fl, "%d ", &num);
  num2obj[num] = this;
  todo.push_back(this);

  memset(buf, 0, 65536);
  res = fscanf(fl, "%[^;]; ", buf);  short_desc = buf;
  if(res < 1) fscanf(fl, " ; ");

  //fprintf(stderr, "%sLoading %d:%s\n", debug_indent.c_str(), num, buf);

  memset(buf, 0, 65536);
  res = fscanf(fl, "%[^;];\n", buf);  desc = buf;
  if(res < 1) fscanf(fl, " ; ");

  memset(buf, 0, 65536);
  res = fscanf(fl, "%[^;]; ", buf);  long_desc = buf;
  if(res < 1) fscanf(fl, " ; ");

  fscanf(fl, "%d %d %d %d %c;\n", &weight, &size, &volume, &value, &gender);

  fscanf(fl, "%d", &exp);
  unsigned long accom;
  while(fscanf(fl, ";%ld", &accom)) {
    completed.insert(accom);
    }

  fscanf(fl, " %d\n", &sexp);

  fscanf(fl, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
	&att[0], &att[1], &att[2], &att[3], &att[4], &att[5], &att[6], &att[7],
	&phys, &stun, &stru);

  int do_tick;
  if(fscanf(fl, ";%d", &do_tick)) {
    if(do_tick) Activate();
    }

  fscanf(fl, "\n");

  memset(buf, 0, 65536);
  int val;
  if(ver <= 0x0010) {
    while(fscanf(fl, ":%[^\n,:],%d", buf, &val)) {
      //fprintf(stderr, "Loaded %s: %d\n", buf, val);
      SetSkill(buf, val);
      }
    }
  else if(ver <= 0x0011) {
    while(fscanf(fl, ":%[^\n:|]|%d", buf, &val)) {
      //fprintf(stderr, "Loaded %s: %d\n", buf, val);
      SetSkill(buf, val);
      }
    }
  else {	//Backward compatible load between v0x12 and v0x13
    int ret;
    ret = fscanf(fl, "|%[^\n|;]|%d", buf, &val);
    while(ret > 1) {
      //fprintf(stderr, "Loaded %s: %d\n", buf, val);
      SetSkill(buf, val);
      ret = fscanf(fl, "|%[^\n|;]|%d", buf, &val);
      }
    if(ret > 0) {	//Added the currently used skill to the end in v0x13
      StartUsing(buf);
      }
    }
  fscanf(fl, ";\n");

  vector<Object*> toload;
  fscanf(fl, "%d ", &num);
  //contents.reserve(num); // This was for vectors.
  for(int ctr=0; ctr<num; ++ctr) {
    int num2;
    fscanf(fl, "%d ", &num2);
    Object *obj = new Object(this);
    toload.push_back(obj);
    AddLink(obj);
    }

  fscanf(fl, "%d\n", &num); pos = (pos_t)num;

  fscanf(fl, "%d ", &num);
  for(int ctr=0; ctr<num; ++ctr) {
    int anum, num2;
    fscanf(fl, "%d;%d ", &anum, &num2);

    /* Encode the Object Number as a pointer, Decoded in Load() */
    act[(act_t)anum] = (Object *)((Object *)(NULL) + num2);
    }

  if(Skill("Personality")) get_mob_mind()->Attach(this);
  else if(Skill("CircleAction")) get_circle_mob_mind()->Attach(this);

  vector<Object*>::iterator cind;
  for(cind = toload.begin(); cind != toload.end(); ++cind) {
    //fprintf(stderr, "%sCalling loader from %s\n", debug_indent.c_str(), short_desc.c_str());
    //string tmp = debug_indent;
    //debug_indent += "  ";
    (*cind)->LoadFrom(fl);
    //debug_indent = tmp;
    //fprintf(stderr, "%sCalled loader from %s\n", debug_indent.c_str(), short_desc.c_str());
    }

  //fprintf(stderr, "%sLoaded %s\n", debug_indent.c_str(), short_desc.c_str());

//  if(HasSkill("Drink")) {
//    SetSkill("Drink", Skill("Drink") * 15);
//    SetSkill("Food", Skill("Food") * 15);
//    }
//  else if(HasSkill("Food")) {
//    SetSkill("Food", Skill("Food") * 60);
//    }

//  if(HasSkill("Heal Effect")) {
//    SetSkill("Heal Spell", Skill("Heal Effect"));
//    SetSkill("Heal Effect", 0);
//    }

//  if(HasSkill("Open")) {
//    SetSkill("Open", 1000);
//    }

//  if(HasSkill("Day Time")) {
//    Activate();
//    }

//  if(IsAct(ACT_SPECIAL_NOTSHOWN)) {
//    SetSkill("Invisible", 1000);
//    }

//  if(short_desc == "A passage exit.") {
//    short_desc = "a passage exit";
//    }

//  if(HasSkill("Evasion Penalty")) {
//    SetSkill("Evasion Bonus", Skill("Evasion Penalty"));
//    SetSkill("Evasion Penalty", 0);
//    }
//  else if(HasSkill("Evasion Bonus")) {
//    SetSkill("Evasion Penalty", Skill("Evasion Bonus"));
//    SetSkill("Evasion Bonus", 0);
//    }

//  if(IsAct(ACT_SPECIAL_PREPARE)) {
//    fprintf(stderr, "Found one!\n");
//    act.erase(ACT_SPECIAL_PREPARE);
//    }

  return 0;
  }
