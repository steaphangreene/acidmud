#include <map>
#include <set>
#include <string>
#include <vector>

#include <fcntl.h>
#include <sys/time.h>
#include <cstdio>
#include <ctime>

#include "stats.hpp"

class Object;
class Player;
class Mind;
class MOBType;

#ifndef OBJECT_HPP
#define OBJECT_HPP

#define LOC_RESERVED 0x0001 // Reserved for individual funcion flags
#define LOC_SELF 0x0002
#define LOC_HERE 0x0004
#define LOC_INTERNAL 0x0008
#define LOC_NEARBY 0x0010
#define LOC_ADJACENT 0x0020
#define LOC_LOCATIONS 0xFFFF

// Filters
#define LOC_NOTWORN 0x00010000 // Allows held
#define LOC_NOTUNWORN 0x00020000 // ALSO allows held
#define LOC_FIXED 0x00040000
#define LOC_NOTFIXED 0x00080000
#define LOC_ALIVE 0x00100000
#define LOC_CONSCIOUS 0x00200000
#define LOC_FILTERS 0x00FF0000

// Special
#define LOC_TOUCH 0x01000000
#define LOC_NIGHT 0x02000000
#define LOC_DARK 0x04000000
#define LOC_HEAT 0x08000000
#define LOC_NINJA 0x10000000
#define LOC_SPECIAL 0xFF000000

enum pos_t { POS_NONE = 0, POS_LIE, POS_SIT, POS_STAND, POS_USE, POS_MAX };

// Changing these requires change to:
// * act_str: in object.cpp
// * act_save: in object_acid.cpp
enum act_t {
  ACT_NONE = 0,
  ACT_DEAD,
  ACT_DYING,
  ACT_UNCONSCIOUS,
  ACT_SLEEP,
  ACT_REST,
  ACT_HEAL,
  ACT_POINT,
  ACT_FOLLOW,
  ACT_FIGHT,
  ACT_OFFER,
  ACT_HOLD,
  ACT_WIELD,
  ACT_WEAR_BACK,
  ACT_WEAR_CHEST,
  ACT_WEAR_HEAD,
  ACT_WEAR_NECK,
  ACT_WEAR_COLLAR,
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
  ACT_WEAR_FACE,
  ACT_MAX,
  ACT_SPECIAL_MONITOR = ACT_MAX,
  ACT_SPECIAL_MASTER,
  ACT_SPECIAL_LINKED,
  ACT_SPECIAL_HOME,
  ACT_SPECIAL_MAX
};

#define ALL (-0x7FFFFFFF)
#define SOME (-0x7FFFFFFE)

class Object {
 public:
  Object();
  Object(Object*);
  Object(const Object&);
  ~Object();

  void Recycle(int inbin = 1);

  int IsActive() const;
  void Activate();
  void Deactivate();

  static Object* Universe();
  static Object* TrashBin();
  Object* World();

  Object* Next(std::string&);
  Object* Split(int nqty);
  const char* Name(int definite = 0, Object* rel = nullptr, Object* sub = nullptr) const;
  const char* Pron() const;
  const char* Poss() const;
  const char* Obje() const;
  const char* ShortDesc() const;
  const char* Desc() const;
  const char* LongDesc() const;
  void SetShortDesc(const std::string&);
  void SetDesc(const std::string&);
  void SetLongDesc(const std::string&);
  void SetParent(Object*);
  Object* Parent() const {
    return parent;
  };
  std::set<Object*> Touching() const {
    return touching_me;
  };

  void SendActions(Mind* m);
  void SendExtendedActions(Mind* m, int vmode = 0);
  void SendContents(Mind* m, Object* o = nullptr, int vmode = 0, std::string b = "");
  void SendContents(Object* m, Object* o = nullptr, int vmode = 0, std::string b = "");

  void SendFullSituation(Mind* m, Object* o = nullptr);
  void SendShortDesc(Mind* m, Object* o = nullptr);
  void SendShortDesc(Object* m, Object* o = nullptr);
  void SendDesc(Mind* m, Object* o = nullptr);
  void SendDescSurround(Mind* m, Object* o = nullptr, int vmode = 0);
  void SendDesc(Object* m, Object* o = nullptr);
  void SendDescSurround(Object* m, Object* o = nullptr, int vmode = 0);
  void SendLongDesc(Mind* m, Object* o = nullptr);
  void SendLongDesc(Object* m, Object* o = nullptr);
  void SendScore(Mind* m, Object* o = nullptr);
  std::vector<std::string> FormatStats(
      const std::vector<std::pair<uint32_t, int32_t>>& skls); // Modifies skls
  std::vector<std::string> FormatSkills(
      const std::vector<std::pair<uint32_t, int32_t>>& skls); // Modifies skls

  void Link(
      Object* other,
      const std::string& name,
      const std::string& desc,
      const std::string& oname,
      const std::string& odesc);
  void LinkClosed(
      Object* other,
      const std::string& name,
      const std::string& desc,
      const std::string& oname,
      const std::string& odesc);
  int Travel(Object*, int try_combine = 1);
  void AddLink(Object*);
  void RemoveLink(Object*);
  void Attach(Mind* mind);
  void Unattach(Mind* mind);

  void TryCombine();

  Object* Stash(Object*, int message = 1, int force = 0, int try_combine = 1);
  int Drop(Object* item, int message = 1, int force = 0, int try_combine = 1);
  int DropOrStash(Object* item, int message = 1, int force = 0, int try_combine = 1);
  int StashOrDrop(Object* item, int message = 1, int force = 0, int try_combine = 1);
  int Wear(Object* targ, int unsigned long = (~(0UL)), int mes = 1);

  Object* Owner() const;
  int NumMinds() const {
    return minds.size();
  }

  void NotifyGone(Object* obj, Object* newloc = nullptr, int up = 1);

  Object* PickObject(const char*, int loc, int* ordinal = nullptr) const;
  std::vector<Object*> PickObjects(const char*, int loc, int* ordinal = nullptr) const;
  int IsNearBy(const Object* obj);
  int SeeWithin(const Object* obj); // Recursive & Visible
  int HasWithin(const Object* obj); // Recursive (All)
  int Contains(const Object* obj); // Only Immediately (No Recursion)
  std::vector<Object*> Contents(int vmode);
  std::vector<Object*> Contents();

  int ContainedWeight();
  int ContainedVolume();

  int Weight() const {
    return weight;
  };
  int Volume() const {
    return volume;
  };
  int Size() const {
    return size;
  };
  int Value() const {
    return value;
  };
  char Gender() const {
    return gender;
  };

  void SetWeight(int w) {
    weight = w;
  };
  void SetVolume(int v) {
    volume = v;
  };
  void SetSize(int s) {
    size = s;
  };
  void SetValue(int v) {
    value = v;
  };
  void SetGender(char g) {
    gender = g;
  };

  int LightLevel(int updown = 0);

  void EarnExp(int);
  void SpendExp(int);
  int Accomplish(unsigned long, const std::string&);
  int TotalExp() const {
    return exp - sexp;
  };
  int Exp() const {
    return exp;
  };

  int Stun() const {
    return stun;
  };
  int Phys() const {
    return phys;
  };
  int Stru() const {
    return stru;
  };

  void SetStun(int s) {
    stun = s;
  }; // Don't use these for normal healing!
  void SetPhys(int p) {
    phys = p;
  };
  void SetStru(int s) {
    stru = s;
  };
  void UpdateDamage();

  void Consume(const Object* item); // Eat/Drink/Potions/Poison/etc....

  bool IsAnimate() const {
    return (att[1].cur != 0);
  };

  int BaseAttribute(int) const;
  int NormAttribute(int) const;
  int ModAttribute(int) const;

  int Modifier(int a) const;
  int Modifier(const std::string& m) const;
  int Power(const std::string& m) const;
  int Skill(uint32_t, int* tnum = nullptr) const;
  int HasSkill(uint32_t) const;
  int SubHasSkill(uint32_t) const;
  int SubMaxSkill(uint32_t) const;
  Object* NextHasSkill(uint32_t, const Object* last = nullptr);
  std::vector<std::pair<uint32_t, int32_t>> GetSkills() const;
  void SetAttribute(int, int);
  void SetModifier(int, int);
  void SetSkill(uint32_t, int);
  void SetSkill(const std::string&, int);

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

  int RollInitiative() const;
  int Roll(uint32_t, const Object*, uint32_t, int bias = 0, std::string* res = nullptr) const;
  int Roll(uint32_t, int, std::string* res = nullptr) const;
  int RollNoWounds(uint32_t, int, std::string* res = nullptr) const;

  int WoundPenalty() const;

  pos_t Pos();
  void SetPos(pos_t p);
  std::string PosString();
  void StartUsing(uint32_t skill);
  void StopUsing();
  uint32_t Using();
  int IsUsing(uint32_t);
  std::string UsingString();

  int Wearing(const Object* obj) const;
  int WearMask() const;
  std::set<act_t> WearSlots(int m = -1) const;
  std::string WearNames(const std::set<act_t>& locs) const;
  std::string WearNames(int m = -1) const;

  int Quantity() const;

  void Collapse();

  void StopAll();
  void AddAct(act_t a, Object* o = nullptr);
  void StopAct(act_t a);
  int IsAct(act_t a) const {
    return act.count(a);
  };
  Object* ActTarg(act_t a) const;

  int HitMent(int force, int sev, int succ);
  int HitStun(int force, int sev, int succ);
  int HitPhys(int force, int sev, int succ);
  int HitStru(int force, int sev, int succ);
  int HealStun(int succ);
  int HealPhys(int succ);
  int HealStru(int succ);

  void Deafen(int deaf = 1);

  // Unformatted (raw print, but with ;s/;s for actor/targ)
  void Send(int targ, int rsucc, const char* mes);
  void
  SendOut(int tnum, int rsucc, const char* mes, const char* youmes, Object* actor, Object* targ);
  void
  SendIn(int tnum, int rsucc, const char* mes, const char* youmes, Object* actor, Object* targ);
  void Loud(int str, const char* mes);

  // Formatted (printf style, plus with ;s/;s for actor/targ)
  void SendF(int targ, int rsucc, const char* mes, ...) __attribute__((format(printf, 4, 5)));
  void SendOutF(
      int tnum,
      int rsucc,
      const char* mes,
      const char* youmes,
      Object* actor,
      Object* targ,
      ...) __attribute__((format(printf, 4, 8)));
  void SendInF(
      int tnum,
      int rsucc,
      const char* mes,
      const char* youmes,
      Object* actor,
      Object* targ,
      ...) __attribute__((format(printf, 4, 8)));
  void LoudF(int str, const char* mes, ...) __attribute__((format(printf, 3, 4)));

  void TBALoadAll();
  void TBALoadWLD(const char*);
  void TBALoadOBJ(const char*);
  void TBALoadMOB(const char*);
  void TBAFinishMOB(Object*);
  void TBALoadZON(const char*);
  void TBALoadSHP(const char*);
  void TBALoadTRG(const char*);
  static void TBAFinalizeTriggers();
  static void TBACleanup();

  int Load(const char*);
  int LoadFrom(FILE*);
  int Save(const char*);
  int SaveTo(FILE*);
  int WriteContentsTo(FILE*);

  int Tick();

  int BusyAct();
  int StillBusy();
  void BusyFor(long msec, const char* default_next = "");
  void BusyWith(Object* other, const char* default_next = "");
  void DoWhenFree(const char*);

  std::string Tactics(int phase = -1);

  void operator=(const Object& in);
  int operator==(const Object& in) const;
  int operator!=(const Object& in) const;

  int Matches(std::string seek);
  int LooksLike(Object* other, int vmode = 0);

  void AddMOB(const MOBType*);

  static void FreeActions();

 private:
  void NotifyLeft(Object* obj, Object* newloc = nullptr);

  void Loud(std::set<Object*>& visited, int str, const char* mes);

  int Filter(int loc);

  int tickstep;

  std::string short_desc;
  std::string desc;
  std::string long_desc;
  std::vector<Object*> contents;
  Object* parent;
  std::set<Mind*> minds;
  pos_t pos;
  uint32_t cur_skill;

  std::set<unsigned long> completed;
  int exp, sexp;

  int weight, volume, size;
  int value;
  int8_t gender;

  int8_t phys, stun, stru;
  struct {
    int8_t base = 0;
    int8_t cur = 0;
    int16_t mod = 0;
  } att[6];
  std::vector<std::pair<uint32_t, int32_t>> skills;

  int no_seek; // Recursion protection
  int no_hear; // For Send() protection

  std::map<act_t, Object*> act;
  std::set<Object*> touching_me;

  int64_t busy_until;
  std::string dowhenfree, defact;
  friend void player_rooms_erase(Object*);
};

int roll(int ndice, int targ);

void init_world();
void save_world(int with_net = 0);
Object* new_body();
Object* getbynum(int);
int getnum(Object*);
int matches(const std::string& name, const std::string& seek);
Mind* get_mob_mind();
Mind* get_tba_mob_mind();

uint32_t get_skill(std::string sk);
std::string get_skill_cat(std::string cat);
int get_linked(std::string sk);
int get_linked(uint32_t sk);
std::vector<uint32_t> get_skills(std::string cat = "Categories");
int is_skill(uint32_t stok);
uint32_t get_weapon_skill(int wtype);
int get_weapon_type(std::string wskill);
int two_handed(int wtype);

void tick_world();

Object* new_obj();
Object* new_obj(Object*);
Object* new_obj(const Object&);

#endif // OBJECT_HPP
