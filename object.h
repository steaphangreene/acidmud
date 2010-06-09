#include <map>
#include <set>
#include <list>
#include <string>

#include <ctime>
#include <cstdio>
#include <list>
#include <fcntl.h>
#include <sys/time.h>

using namespace std;

#include "atomstring.h"

class Object;
class Player;
class Mind;
class MOBType;

#ifndef OBJECT_H
#define OBJECT_H

#define	LOC_SELF	0x0001
#define	LOC_HERE	0x0002
#define	LOC_INTERNAL	0x0004
#define	LOC_NEARBY	0x0008
#define	LOC_ADJACENT	0x0010
#define LOC_LOCATIONS	0xFFFF

//Filters
#define	LOC_NOTWORN	0x00010000
#define	LOC_ALIVE	0x00020000
#define	LOC_CONSCIOUS	0x00040000
#define LOC_FILTERS	0xFFFF0000

enum pos_t {
	POS_NONE=0,
	POS_LIE,
	POS_SIT,
	POS_STAND,
	POS_USE,
	POS_MAX
	};

enum act_t {
	ACT_NONE=0,
	ACT_DEAD,
	ACT_DYING,
	ACT_UNCONSCIOUS,
	ACT_SLEEP,
	ACT_REST,
	ACT_POINT,
	ACT_FOLLOW,
	ACT_FIGHT,
	ACT_HOLD,
	ACT_WIELD,
	ACT_WEAR_BACK,
	ACT_WEAR_CHEST,
	ACT_WEAR_HEAD,
	ACT_WEAR_NECK,
	ACT_WEAR_WAIST,
	ACT_WEAR_SHIELD,
	ACT_WEAR_LARM,
	ACT_WEAR_RARM,
	ACT_WEAR_LFINGER,
	ACT_WEAR_RFINGER,
	ACT_WEAR_LFOOT,
	ACT_WEAR_RFOOT,
	ACT_WEAR_LHAND,
	ACT_WEAR_RHAND,
	ACT_WEAR_LLEG,
	ACT_WEAR_RLEG,
	ACT_WEAR_LWRIST,
	ACT_WEAR_RWRIST,
	ACT_WEAR_LSHOULDER,
	ACT_WEAR_RSHOULDER,
	ACT_WEAR_LHIP,
	ACT_WEAR_RHIP,
	ACT_MAX,
	ACT_SPECIAL_MONITOR,
	ACT_SPECIAL_OLDMONITOR,
	ACT_SPECIAL_PREPARE,
	ACT_SPECIAL_NOTSHOWN,
	ACT_SPECIAL_MASTER,
	ACT_SPECIAL_LINKED,
	ACT_SPECIAL_MAX
	};

#define ALL	(-0x7FFFFFFF)
#define SOME	(-0x7FFFFFFE)

extern const char *pos_str[POS_MAX];
extern const char *act_str[ACT_SPECIAL_MAX];

class Object {
public:
  Object();
  Object(Object *);
  Object(const Object &);
  ~Object();

  int IsActive() const;
  void Activate();
  void Deactivate();

  Object *Next(string &);
  Object *Split(int nqty);
  const char *Name(int definite = 0, Object *rel = NULL, Object *sub = NULL) const;
  const char *ShortDesc() const;
  const char *Desc() const;
  const char *LongDesc() const;
  void SetShortDesc(const char *);
  void SetDesc(const char *);
  void SetLongDesc(const char *);
  void SetParent(Object *);
  Object *Parent() { return parent; };

  void SendActions(Mind *m);
  void SendExtendedActions(Mind *m, int seeinside=0);
  void SendContents(Mind *m, Object *o = NULL, int seeinside=0, string b="");
  void SendContents(Object *m, Object *o = NULL, int seeinside=0, string b="");

  void SendFullSituation(Mind *m, Object *o = NULL);
  void SendShortDesc(Mind *m, Object *o = NULL);
  void SendShortDesc(Object *m, Object *o = NULL);
  void SendDesc(Mind *m, Object *o = NULL);
  void SendDescSurround(Mind *m, Object *o = NULL);
  void SendDesc(Object *m, Object *o = NULL);
  void SendDescSurround(Object *m, Object *o = NULL);
  void SendLongDesc(Mind *m, Object *o = NULL);
  void SendLongDesc(Object *m, Object *o = NULL);
  void SendScore(Mind *m, Object *o = NULL);
  void SendStats(Mind *m, Object *o = NULL);

  void Link(Object *other, const string &name, const string &desc,
        const string &oname, const string &odesc);
  void LinkClosed(Object *other, const string &name, const string &desc,
        const string &oname, const string &odesc);
  int Travel(Object *, int try_combine = 1);
  void AddLink(Object *);
  void RemoveLink(Object *);
  void Attach(Mind *mind);
  void Unattach(Mind *mind);

  void TryCombine();

  Object *Stash(Object *, int try_combine = 1);

  Object *PickObject(const char *, int loc, int *ordinal=NULL);
  list<Object *> PickObjects(const char *, int loc, int *ordinal=NULL);
  void NotifyGone(Object *obj, Object *newloc = NULL, int up = 1);
  int IsNearBy(Object *obj);
  int IsWithin(Object *obj);
  int Contains(Object *obj);
  list<Object *> Contents();

  int ContainedWeight();
  int ContainedVolume();

  int Weight()	{ return weight; };
  int Volume()	{ return volume; };
  int Size()	{ return size; };
  int Value()	{ return value; };
  char Gender()	{ return gender; };

  void SetWeight(int w)	{ weight = w; };
  void SetVolume(int v)	{ volume = v; };
  void SetSize(int s)	{ size = s; };
  void SetValue(int v)	{ value = v; };
  void SetGender(char g){ gender = g; };

  int LightLevel(int updown=0);

  void EarnExp(int);
  void SpendExp(int);
  int Accomplish(unsigned long);
  int Exp(Player *p = NULL);

  int Stun()	{ return stun; };
  int Phys()	{ return phys; };
  int Stru()	{ return stru; };

  void SetStun(int s)	{ stun = s; };  //Don't use these for normal healing!
  void SetPhys(int p)	{ phys = p; };
  void SetStru(int s)	{ stru = s; };
  void UpdateDamage();

  void Consume(const Object *item);	//Eat/Drink/Potions/Poison/etc....

  int Attribute(int) const;
  int Skill(const string &, int *tnum = NULL) const;
  int HasSkill(const string &) const;
  const map<AtomString,int> &GetSkills() const { return skills; }

  void SetAttribute(int, int);
  void SetSkill(const string &, int);

  void DynamicInit();
  void DynamicInit1();
  void DynamicInit2();
  void DynamicInit3();
  void DynamicInit4();
  void DynamicInit5();
  void DynamicInit6();
  void DynamicInit7();
  void DynamicInit8();
  void DynamicInit9();

  list<int> RollInitiative() const;
  int Roll(const string &, const Object *, const string &, int bias = 0,
	string *res=NULL) const;
  int Roll(const string &, const Object *, const string &, int bias = 0,
	list<int> *wraps = NULL, string *res = NULL) const;
  int Roll(const string &, int, list<int> *wraps, string *res=NULL) const;
  int Roll(const string &, int, string *res=NULL) const;
  int RollNoWounds(const string &, int, list<int> *wraps,
	string *res = NULL) const;
  int RollNoWounds(const string &, int, string *res = NULL) const;

  int WoundPenalty() const;

  pos_t Pos();
  void SetPos(pos_t p);
  const char *PosString();
  void StartUsing(const string &skill);
  void StopUsing();
  const char *Using();
  int IsUsing(const string &skill);
  const char *UsingString();

  int Wearing(const Object *obj) const;

  void Collapse();

  void StopAll();
  void AddAct(act_t a, Object *o=NULL) { act[a] = o; };
  void StopAct(act_t a);
  int IsAct(act_t a) const { return act.count(a); };
  Object *ActTarg(act_t a) const;

  int HitMent(int force, int sev, int succ);
  int HitStun(int force, int sev, int succ);
  int HitPhys(int force, int sev, int succ);
  int HitStru(int force, int sev, int succ);
  int HealStun(int succ);
  int HealPhys(int succ);
  int HealStru(int succ);

  void Send(int targ, int rsucc, const char *mes, ...)
	__attribute__ ((format (printf, 4, 5)));
  void SendOut(int tnum, int rsucc, const char *mes, const char *youmes,
	Object *actor, Object *targ, ...)
	__attribute__ ((format (printf, 4, 8)));
  void SendIn(int tnum, int rsucc, const char *mes, const char *youmes,
	Object *actor, Object *targ, ...)
	__attribute__ ((format (printf, 4, 8)));
  void Loud(int str, const char *mes, ...)
	__attribute__ ((format (printf, 3, 4)));

  void CircleLoad(const char *);
  void CircleLoadObj(const char *);
  void CircleLoadMob(const char *);
  void CircleFinishMob(Object *);
  void CircleLoadZon(const char *);
  void CircleLoadShp(const char *);
  void CircleCleanup();
  void CircleLoadAll();

  int Load(const char *);
  int LoadFrom(FILE *);
  int Save(const char *);
  int SaveTo(FILE *);
  int WriteContentsTo(FILE *);

  int Tick();

  int BusyAct();
  int StillBusy();
  void BusyFor(long msec, const char *default_next = "");
  void BusyWith(Object *other, const char *default_next = "");
  void DoWhenFree(const char *);

  string Tactics(int phase = -1);

  void operator = (const Object &in);
  int operator == (const Object &in) const;
  int operator != (const Object &in) const;

  int Matches(const char *seek);
  int LooksLike(Object *other);

  void AddMOB(const MOBType *);

  static void FreeActions();

private:
  void Loud(set<Object*> &visited, int str, const char *mes);

  int Filter(int loc);

  AtomString short_desc;
  AtomString desc;
  AtomString long_desc;
  list<Object*> contents;
  Object *parent;
  set<Mind*> minds;
  pos_t pos;
  AtomString cur_skill;

  int weight, volume, size;
  int value;
  char gender;

  set<unsigned long> completed;
  int exp, sexp;

  int phys, stun, stru;
  int att[8];
  map<AtomString,int> skills;

  int no_seek; //Recursion protection

  map<act_t,Object*> act;
  timeval busytill;
  string dowhenfree, defact;
  friend void player_rooms_erase(Object *);
  };

int roll(int ndice, int targ, list<int> *wraps = NULL);

void init_world();
void save_world(int with_net=0);
Object *new_body();
void set_start_room(Object *);
Object *get_start_room();
Object *getbynum(int);
int getnum(Object *);
int matches(const char *name, const char *seek);
Mind *get_mob_mind();
Mind *get_circle_mob_mind();

string get_skill(string sk);
string get_skill_cat(string cat);
int get_linked(string sk);
list<string> get_skills(string cat = "Categories");
int is_skill(string sk);
string get_weapon_skill(int wtype);
int get_weapon_type(string wskill);
int two_handed(int wtype);

void tick_world();

#endif
