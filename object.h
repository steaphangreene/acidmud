#include <map>
#include <set>
#include <vector>
#include <string>

#include <ctime>
#include <cstdio>
#include <fcntl.h>
#include <sys/time.h>

using namespace std;

class Object;

#ifndef OBJECT_H
#define OBJECT_H

#define	LOC_SELF	1
#define	LOC_INTERNAL	2
#define	LOC_NEARBY	4
#define	LOC_ADJACENT	8

enum pos_t {
	POS_NONE=0,
	POS_LIE,
	POS_SIT,
	POS_STAND,
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
	ACT_MAX
	};

#define ALL (-2147483647-1)

extern const char *pos_str[POS_MAX];
extern const char *act_str[ACT_MAX];

class Mind;

class Object {
public:
  Object();
  Object(Object *);
  Object(const Object &);
  ~Object();

  Object *Next(string &);
  const char *Name(int definite = 0, Object *rel = NULL, Object *sub = NULL);
  const char *ShortDesc();
  const char *Desc();
  const char *LongDesc();
  void SetShortDesc(const char *);
  void SetDesc(const char *);
  void SetLongDesc(const char *);
  void LinkTo(const char *, Object *);
  void LinkToNew(const char *);
  void SetParent(Object *);
  Object *Parent() { return parent; };

  void SendActions(Mind *m);
  void SendExtendedActions(Mind *m, int seeinside=0);
  void SendContents(Mind *m, Object *o = NULL, int seeinside=0);
  void SendContents(Object *m, Object *o = NULL, int seeinside=0);

  void SendFullSituation(Mind *m, Object *o = NULL);
  void SendShortDesc(Mind *m, Object *o = NULL);
  void SendShortDesc(Object *m, Object *o = NULL);
  void SendDesc(Mind *m, Object *o = NULL);
  void SendDescSurround(Mind *m, Object *o = NULL);
  void SendDesc(Object *m, Object *o = NULL);
  void SendDescSurround(Object *m, Object *o = NULL);
  void SendLongDesc(Mind *m, Object *o = NULL);
  void SendLongDesc(Object *m, Object *o = NULL);
  void SendStats(Mind *m, Object *o = NULL);
  void SendStats(Object *m, Object *o = NULL);

  int Travel(Object *);
  void AddLink(Object *);
  void RemoveLink(Object *);
  void Attach(Mind *mind);
  void Unattach(Mind *mind);

  Object *Stash(Object *);

  Object *PickObject(char *, int loc, int *ordinal=NULL);
  vector<Object *> PickObjects(char *, int loc, int *ordinal=NULL);
  int IsNearBy(Object *obj);
  int IsWithin(Object *obj);
  int Contains(Object *obj);
  vector<Object *> Contents();
  map<string,Object*> Connections() { return connections; }

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

  int Stun()	{ return stun; };
  int Phys()	{ return phys; };
  int Stru()	{ return stru; };

  void SetStun(int s)	{ stun = s; };  //Don't use these for normal healing!
  void SetPhys(int p)	{ phys = p; };
  void SetStru(int s)	{ stru = s; };
  void UpdateDamage();

  int Attribute(int) const;
  int Skill(const string &, int *tnum = NULL) const;
  map<string,int> GetSkills() const { return skills; }

  void SetAttribute(int, int);
  void SetSkill(const string &, int);

  int RollInitiative() const;
  int Roll(const string &, const Object *, const string &, int bias = 0, string *res=NULL) const;
  int Roll(const string &, int, string *res=NULL) const;
  int RollNoWounds(const string &, int, string *res=NULL) const;

  int WoundPenalty() const;

  pos_t Pos() { return pos; };
  void SetPos(pos_t p) { pos = p; };
  const char *PosString() { return pos_str[pos]; };

  void Collapse();

  void StopAll();
  void AddAct(act_t a, Object *o=NULL) { act[a] = o; };
  void StopAct(act_t a) { act.erase(a); };
  int IsAct(act_t a) const { return act.count(a); };
  Object *ActTarg(act_t a) const;

  int HitMent(int force, int sev, int succ);
  int HitStun(int force, int sev, int succ);
  int HitPhys(int force, int sev, int succ);
  int HitStru(int force, int sev, int succ);
  int HealStun(int succ);
  int HealPhys(int succ);
  int HealStru(int succ);

  void SendAll(const set<Object *> &excl, const char *mes, ...)
	__attribute__ ((format (printf, 3, 4)));
  void Send(const char *mes, ...)
	__attribute__ ((format (printf, 2, 3)));
  void SendOut(const char *mes, const char *youmes,
	Object *actor, Object *targ, ...)
	__attribute__ ((format (printf, 2, 6)));
  void SendIn(const char *mes, const char *youmes,
	Object *actor, Object *targ, ...)
	__attribute__ ((format (printf, 2, 6)));

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

  void Tick();

  void BusyAct();
  int StillBusy();
  void BusyFor(long msec, const char *default_next = "");
  void BusyWith(Object *other, const char *default_next = "");
  void DoWhenFree(const char *);

  void operator = (const Object &in);
  int operator == (const Object &in) const;

private:
  string short_desc;
  string desc;
  string long_desc;
  map<string,Object*> connections;
  vector<Object*> contents;
  Object *parent;
  set<Mind*> minds;
  pos_t pos;

  int weight, volume, size;
  int value;
  char gender;

  int phys, stun, stru;
  int att[8];
  map<string,int> skills;

  int no_seek; //Recursion protection

  map<act_t,Object*> act;
  timeval busytill;
  string dowhenfree, defact;
  friend void player_rooms_erase(Object *);
  };

int roll(int ndice, int targ);

void init_world();
void save_world(int with_net=0);
Object *new_body();
void set_start_room(Object *);
Object *get_start_room();
Object *getbynum(int);
int getnum(Object *);
int matches(const char *name, const char *seek);
Mind *get_mob_mind();

void FreeActions();

map<string,int> get_skills();
string get_weapon_skill(int wtype);
int get_weapon_type(string wskill);

void tick_world();

#endif
