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

#include <random>
#include <set>
#include <string>
#include <vector>

#include <fcntl.h>
#include <sys/time.h>
#include <cstdio>
#include <ctime>

#include "minvec.hpp"
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

// Optional Message Channels
enum channel_t {
  CHANNEL_OOC,
  CHANNEL_NEWBIE,
  CHANNEL_ROLLS,
};

enum class pos_t : uint8_t { NONE = 0, LIE, SIT, STAND, USE, MAX };

// Changing these requires change to:
// * act_str: in object.cpp
// * act_save: in object_acid.cpp
enum class act_t : uint8_t {
  NONE = 0,
  DEAD,
  DYING,
  UNCONSCIOUS,
  SLEEP,
  REST,
  HEAL,
  POINT,
  FOLLOW,
  FIGHT,
  OFFER,
  HOLD,
  WIELD,
  WEAR_BACK,
  WEAR_CHEST,
  WEAR_HEAD,
  WEAR_NECK,
  WEAR_COLLAR,
  WEAR_WAIST,
  WEAR_SHIELD,
  WEAR_LARM,
  WEAR_RARM,
  WEAR_LFINGER,
  WEAR_RFINGER,
  WEAR_LFOOT,
  WEAR_RFOOT,
  WEAR_LHAND,
  WEAR_RHAND,
  WEAR_LLEG,
  WEAR_RLEG,
  WEAR_LWRIST,
  WEAR_RWRIST,
  WEAR_LSHOULDER,
  WEAR_RSHOULDER,
  WEAR_LHIP,
  WEAR_RHIP,
  WEAR_FACE,
  MAX,
  SPECIAL_MONITOR = MAX,
  SPECIAL_MASTER,
  SPECIAL_LINKED,
  SPECIAL_HOME,
  SPECIAL_WORK,
  SPECIAL_ACTEE,
  SPECIAL_MAX
};
inline void operator++(act_t& a) {
  a = static_cast<act_t>(static_cast<uint8_t>(a) + 1);
};
static_assert(static_cast<uint8_t>(act_t::SPECIAL_MAX) < 256);
class act_pair {
 public:
  act_pair() = default;
  act_pair(act_t a, Object* o) {
    assert((reinterpret_cast<uintptr_t>(o) & act_mask) == 0);
    item_ = ((reinterpret_cast<uintptr_t>(o) & ~act_mask) | (static_cast<uintptr_t>(a) & act_mask));
  };
  act_t act() const {
    return static_cast<act_t>(item_ & act_mask);
  };
  Object* obj() const {
    return reinterpret_cast<Object*>(item_ & ~act_mask);
  };
  void set_obj(Object* o) {
    assert((reinterpret_cast<uintptr_t>(o) & act_mask) == 0);
    item_ = ((reinterpret_cast<uintptr_t>(o) & ~act_mask) | static_cast<uintptr_t>(act()));
  };

 private:
  static const uintptr_t act_mask = 0xFFUL;
  uintptr_t item_;
};
static_assert(sizeof(act_pair) == 8);

struct skill_pair {
  uint32_t first;
  int32_t second;
  auto operator<=>(const skill_pair&) const = default;
};

#define ALL (-0x7FFFFFFF)
#define SOME (-0x7FFFFFFE)

class alignas(256) Object {
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
  Object* Zone();

  Object* Next(std::string&);
  Object* Split(int nqty);
  std::string Noun(bool definite = false, const Object* rel = nullptr, const Object* sub = nullptr)
      const;
  std::string Pron() const;
  std::string Poss() const;
  std::string Obje() const;
  std::string ShortDesc() const;
  std::string Name() const;
  std::string Desc() const;
  std::string LongDesc() const;
  const char* ShortDescC() const;
  const char* NameC() const;
  const char* DescC() const;
  const char* LongDescC() const;
  bool HasName() const;
  bool HasDesc() const;
  bool HasLongDesc() const;
  void SetDescs(std::string, std::string, std::string, std::string);
  void SetShortDesc(const std::string&);
  void SetName(const std::string&);
  void SetDesc(const std::string&);
  void SetLongDesc(const std::string&);
  void SetParent(Object*);
  Object* Parent() const {
    return parent;
  };
  MinVec<1, Object*> Touching() const {
    MinVec<1, Object*> ret;
    for (auto o : act) {
      if (o.act() == act_t::SPECIAL_ACTEE) {
        ret.push_back(o.obj());
      }
    }
    return ret;
  };
  bool IsTouching(Object* other) const {
    auto itr = act.begin();
    for (; itr != act.end() && (itr->act() != act_t::SPECIAL_ACTEE || itr->obj() != other); ++itr) {
    }
    return (itr != act.end());
  };
  void NowTouching(Object* other) {
    if (!IsTouching(other)) {
      act.push_back(act_pair(act_t::SPECIAL_ACTEE, other));
    }
  };
  void NotTouching(Object* other) {
    auto itr = act.begin();
    for (; itr != act.end() && (itr->act() != act_t::SPECIAL_ACTEE || itr->obj() != other); ++itr) {
    }
    if (itr != act.end()) {
      act.erase(itr);
    }
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
  std::vector<std::string> FormatStats(const MinVec<7, skill_pair>& skls); // Modifies skls
  std::vector<std::string> FormatSkills(const MinVec<7, skill_pair>& skls); // Modifies skls

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

  Object* PickObject(const std::string& name, int loc, int* ordinal = nullptr) const;
  MinVec<1, Object*> PickObjects(std::string name, int loc, int* ordinal = nullptr) const;
  int IsNearBy(const Object* obj) const;
  int SeeWithin(const Object* obj) const; // Recursive & Visible
  int HasWithin(const Object* obj) const; // Recursive (All)
  int Contains(const Object* obj) const; // Only Immediately (No Recursion)
  MinVec<3, Object*> Contents(int vmode) const;
  MinVec<3, Object*> Contents() const;

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

  void SpendExp(int);
  bool Accomplish(uint64_t, const std::string&);
  bool HasAccomplished(uint64_t) const;
  int TotalExp() const {
    return completed.size() - sexp;
  };
  int Exp() const {
    return completed.size();
  };

  bool Learn(uint64_t, const std::string&);
  bool Knows(uint64_t) const;
  bool Knows(const Object*) const;

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
    return (attr[1] != 0);
  };

  int NormAttribute(int) const;
  int ModAttribute(int) const;

  int Modifier(int a) const;
  int Modifier(const std::string& m) const;
  int Power(uint32_t ptok) const;
  int Skill(uint32_t stok) const {
    for (const auto& sk : skills) {
      if (sk.first == stok) {
        return sk.second;
      }
    }
    return 0;
  };
  int SkillTarget(uint32_t) const;
  bool HasSkill(uint32_t stok) const {
    for (const auto& sk : skills) {
      if (sk.first == stok) {
        return true;
      }
    }
    return false;
  };
  int SubHasSkill(uint32_t) const;
  int SubMaxSkill(uint32_t) const;
  Object* NextHasSkill(uint32_t, const Object* last = nullptr);
  MinVec<7, skill_pair> GetSkills() const;
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
  int RollNoWounds(uint32_t, int, int, std::string* res = nullptr) const;

  int WoundPenalty() const;

  pos_t Pos() const;
  void SetPos(pos_t p);
  std::string PosString() const;
  void StartUsing(uint32_t skill);
  void StopUsing();
  uint32_t Using() const;
  int IsUsing(uint32_t) const;
  std::string UsingString() const;

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
  bool IsAct(act_t a) const {
    for (const auto& ac : act) {
      if (ac.act() == a) {
        return true;
      }
    }
    return false;
  };
  Object* ActTarg(act_t a) const {
    for (const auto& ac : act) {
      if (ac.act() == a) {
        return ac.obj();
      }
    }
    return nullptr;
  };

  int HitMent(int force, int sev, int succ);
  int HitStun(int force, int sev, int succ);
  int HitPhys(int force, int sev, int succ);
  int HitStru(int force, int sev, int succ);
  int HealStun(int succ);
  int HealPhys(int succ);
  int HealStru(int succ);

  void Deafen(bool deaf = true);

  // Unformatted (raw print, but with ;s/;s for actor/targ)
  void Send(channel_t channel, const std::string& mes);
  void Send(int targ, int rsucc, const std::string& mes);
  void SendOut(
      int tnum,
      int rsucc,
      const std::string& mes,
      const std::string& youmes,
      Object* actor,
      Object* targ);
  void SendIn(
      int tnum,
      int rsucc,
      const std::string& mes,
      const std::string& youmes,
      Object* actor,
      Object* targ);
  void Loud(int str, const std::string& mes);

  // Formatted (printf style, plus with ;s/;s for actor/targ)
  void SendF(channel_t channel, const char* mes, ...) __attribute__((format(printf, 3, 4)));
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
  void TBALoadWLD(const std::string&);
  void TBALoadOBJ(const std::string&);
  void TBALoadMOB(const std::string&);
  void TBAFinishMOB(Object*);
  void TBALoadZON(const std::string&);
  void TBALoadSHP(const std::string&);
  void TBALoadTRG(const std::string&);
  static void TBAFinalizeTriggers();
  static void TBACleanup();

  int Load(const std::string&);
  int LoadFrom(FILE*);
  int Save(const std::string&);
  int SaveTo(FILE*);
  int WriteContentsTo(FILE*);

  int Tick();

  bool BusyAct();
  bool StillBusy() const;
  void BusyFor(long msec, const std::string& default_next = "");
  void BusyWith(Object* other, const std::string& default_next = "");
  void DoWhenFree(const std::string&);

  std::string Tactics(int phase = -1);

  void operator=(const Object& in);

  bool IsSameAs(const Object& in) const;

  int Matches(std::string seek, bool knows = false) const;
  int LooksLike(Object* other, int vmode = 0, Object* viewer = nullptr) const;

  void AddMOB(std::mt19937&, const MOBType*);

  static void FreeActions();

  static void InitSkillsData();

 private:
  void NotifyLeft(Object* obj, Object* newloc = nullptr);

  void Loud(std::set<Object*>& visited, int str, const std::string& mes);

  bool Filter(int loc) const;

  constexpr static const char* const default_descriptions = "a new object\0\0\0\0";
  const char* descriptions = default_descriptions;
  struct {
    uint8_t sd, n;
    uint16_t d, ld;
  } dlens = {10, 0, 0, 0};

  bool no_seek; // Recursion protection
  bool no_hear; // For Send() protection

  int weight, volume, size;
  int value;
  int8_t gender;

  int8_t phys, stun, stru;
  int8_t attr[6] = {0, 0, 0, 0, 0, 0};

  int8_t tickstep;

  pos_t pos;

  int sexp;
  MinVec<1, uint64_t> completed;

  MinVec<3, Object*> contents;
  Object* parent;
  uint32_t cur_skill;

  uint32_t busy_until = 0; // Encoded
  char const* dowhenfree = "";
  char const* defact = "";

  MinVec<1, Mind*> minds;

  MinVec<1, uint64_t> known;

  MinVec<3, act_pair> act;

  MinVec<7, skill_pair> skills;

  friend void player_rooms_erase(Object*);
};
static_assert(sizeof(Object) == 256); // Prevent Bloat

int roll(int ndice, int targ, std::string* res = nullptr);

void init_world();
void save_world(int with_net = 0);
Object* new_body(Object* world);
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
