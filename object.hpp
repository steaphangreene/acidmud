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

#include <forward_list>
#include <memory>
#include <random>
#include <set>
#include <string>
#include <vector>

// Replace with C++20 std::format, when widely available
#define FMT_HEADER_ONLY
#include <fmt/format.h>

#include <fcntl.h>
#include <sys/time.h>
#include <ctime>

#include "darr32.hpp"
#include "darr64.hpp"
#include "stats.hpp"

class Object;
class Player;
class Mind;
class ObjectTag;

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

// Changing these requires change to:
// * pos_str: in object.cpp
// * pos_save: in object_acid.cpp
enum class pos_t : uint8_t { NONE = 0, LIE, SIT, PROP, STAND, USE, MAX };
inline void operator++(pos_t& p) {
  p = static_cast<pos_t>(std::to_underlying(p) + 1);
};

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
  WORK,
  HEAL,
  POINT,
  FOLLOW,
  FIGHT,
  OFFER,
  HOLD,
  WIELD,
  SIMPLE_MAX = WIELD,
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
  SPECIAL_MONITOR,
  WEAR_MAX = SPECIAL_MONITOR,
  NORMAL_MAX = SPECIAL_MONITOR,
  SPECIAL_LINKED,
  SPECIAL_HOME,
  SPECIAL_WORK,
  SPECIAL_OWNER,
  SPECIAL_ACTEE,
  MAX
};
inline void operator++(act_t& a) {
  a = static_cast<act_t>(std::to_underlying(a) + 1);
};
static_assert(std::to_underlying(act_t::MAX) < 256);
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

enum class gender_t : int8_t {
  NONE = 0,
  FEMALE,
  MALE,
  NEITHER,
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
  const Object* Room() const;
  Object* Room();
  const Object* World() const;
  Object* World();
  const Object* Zone() const;
  Object* Zone();

  Object* Next(std::u8string&);
  Object* Split(int nqty);
  std::u8string Noun(
      bool definite = false,
      bool verbose = true,
      const Object* rel = nullptr,
      const Object* sub = nullptr) const;
  std::u8string Pron() const;
  std::u8string Poss() const;
  std::u8string Obje() const;

  bool HasName() const {
    return (dlens.n != 0);
  };

  bool HasDesc() const {
    return (dlens.d != 0);
  };

  bool HasLongDesc() const {
    return (dlens.ld != 0);
  };

  std::u8string_view ShortDesc() const {
    return std::u8string_view(descriptions, dlens.sd);
  };

  std::u8string_view Name() const {
    return std::u8string_view(descriptions + dlens.sd + 1, dlens.n);
  };

  std::u8string_view Desc() const {
    return std::u8string_view(descriptions + dlens.sd + dlens.n + 2, dlens.d);
  };

  std::u8string_view LongDesc() const {
    return std::u8string_view(descriptions + dlens.sd + dlens.n + dlens.d + 3, dlens.ld);
  };

  void SetDescs(std::u8string_view, std::u8string_view, std::u8string_view, std::u8string_view);
  void SetShortDesc(const std::u8string_view&);
  void SetName(const std::u8string_view&);
  void SetDesc(const std::u8string_view&);
  void SetLongDesc(const std::u8string_view&);
  void SetParent(Object*);
  Object* Parent() const {
    return parent;
  };
  DArr64<Object*> Touching() const {
    DArr64<Object*> ret;
    for (auto o : actions) {
      if (o.act() == act_t::SPECIAL_ACTEE) {
        ret.push_back(o.obj());
      }
    }
    return ret;
  };
  bool IsTouching(Object* other) const {
    auto itr = actions.begin();
    for (; itr != actions.end() && (itr->act() != act_t::SPECIAL_ACTEE || itr->obj() != other);
         ++itr) {
    }
    return (itr != actions.end());
  };
  void NowTouching(Object* other) {
    if (!IsTouching(other)) {
      actions.push_back(act_pair(act_t::SPECIAL_ACTEE, other));
    }
  };
  void NotTouching(Object* other) {
    auto itr = actions.begin();
    for (; itr != actions.end() && (itr->act() != act_t::SPECIAL_ACTEE || itr->obj() != other);
         ++itr) {
    }
    if (itr != actions.end()) {
      actions.erase(itr);
    }
  };

  void SendActions(std::shared_ptr<Mind> m);
  void SendExtendedActions(std::shared_ptr<Mind> m, int vmode = 0);
  void SendFullSituation(std::shared_ptr<Mind> m, Object* o = nullptr);
  void
  SendContents(std::shared_ptr<Mind> m, Object* o = nullptr, int vmode = 0, std::u8string b = u8"");

  void SendShortDesc(std::shared_ptr<Mind> m, Object* o = nullptr);
  void SendDesc(std::shared_ptr<Mind> m, Object* o = nullptr);
  void SendDescSurround(std::shared_ptr<Mind> m, Object* o = nullptr, int vmode = 0);
  void SendLongDesc(std::shared_ptr<Mind> m, Object* o = nullptr);
  void SendScore(std::shared_ptr<Mind> m, Object* o = nullptr);

  void SendShortDesc(Object* m, Object* o = nullptr);
  void SendDesc(Object* m, Object* o = nullptr);
  void SendDescSurround(Object* m, Object* o = nullptr, int vmode = 0);
  void SendLongDesc(Object* m, Object* o = nullptr);
  void SendContents(Object* m, Object* o = nullptr, int vmode = 0, std::u8string b = u8"");

  // All of these modify the "skls" parameter.
  std::vector<std::u8string> FormatStats(const DArr64<skill_pair, 7>& skls);
  std::vector<std::u8string> FormatSkills(const DArr64<skill_pair, 7>& skls);
  std::vector<std::u8string> FormatNonweaponSkills(const DArr64<skill_pair, 7>& skls);

  void Link(
      Object* other,
      const std::u8string& name,
      const std::u8string& desc,
      const std::u8string& oname,
      const std::u8string& odesc);
  void LinkClosed(
      Object* other,
      const std::u8string& name,
      const std::u8string& desc,
      const std::u8string& oname,
      const std::u8string& odesc);
  int Travel(Object*);
  void AddLink(Object*);
  void RemoveLink(Object*);
  void Attach(std::shared_ptr<Mind> mind);
  void Detach(std::shared_ptr<Mind> mind);

  void TryCombine();

  Object* Stash(Object*, bool message = true, bool force = false);
  int Drop(Object* item, bool message = true, bool force = false);
  int DropOrStash(Object* item, bool message = true, bool force = false);
  int StashOrDrop(Object* item, bool message = true, bool force = false);
  int Wear(Object* targ, int unsigned long = (~(0UL)), bool message = true);

  Object* Owner() const;
  bool HasMultipleMinds() const {
    auto mitr = minds.begin();
    if (mitr == minds.end()) {
      return false;
    }
    ++mitr;
    if (mitr == minds.end()) {
      return false;
    }
    return true;
  }
  bool HasMind() const {
    auto mitr = minds.begin();
    if (mitr == minds.end()) {
      return false;
    }
    return true;
  }

  void NotifyGone(Object* obj, Object* newloc = nullptr, int up = 1);

  Object* PickObject(const std::u8string_view& name, int loc, int* ordinal = nullptr) const;
  DArr64<Object*> PickObjects(const std::u8string_view& name, int loc, int* ordinal = nullptr)
      const;
  int IsNearBy(const Object* obj) const;
  int SeeWithin(const Object* obj) const; // Recursive & Visible
  int HasWithin(const Object* obj) const; // Recursive (All)
  int Contains(const Object* obj) const; // Only Immediately (No Recursion)
  DArr64<Object*, 3> Contents(int vmode) const;
  DArr64<Object*, 3> Contents() const;
  DArr64<Object*, 7> Connections(const Object* traveller) const; // Includes nulls for no-go dirs
  DArr64<Object*, 7> ConnectionExits() const; // Includes nulls for unconnected dirs
  std::u8string DirectionsTo( // From "nsewud", or empty if there/unreachable
      const Object* dest,
      const Object* traveller) const;
  size_t ManhattanDistance(const Object* dest) const;

  // Returns| =amount:Yes | <amount:Can't Afford | >amount:Can't Make Change
  size_t CanPayFor(std::size_t amount) const;
  DArr64<Object*, 3> PayFor(std::size_t amount);

  // Returns how much NPC/MOB would pay for item, or 0.
  size_t WouldBuyFor(const Object* item);

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
  gender_t Gender() const {
    return gender;
  };

  void SetCoords(int x, int y, int z = 0) {
    weight = x;
    volume = y;
    size = z;
  };
  int X() const {
    return weight;
  };
  int Y() const {
    return volume;
  };
  int Z() const {
    return size;
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
  void SetGender(gender_t g) {
    gender = g;
  };

  int LightLevel(int updown = 0);

  void SetTags(const std::u8string_view& tags);
  void AddTag(uint64_t tag);
  bool HasTag(uint64_t tag) const;

  void SpendExp(int);
  bool Accomplish(uint64_t, const std::u8string_view&);
  bool HasAccomplished(uint64_t) const;
  int TotalExp() const {
    return (completed.size() - sexp) + (known.size() / 100);
  };
  int Exp() const {
    return completed.size();
  };

  bool Learn(uint64_t, const std::u8string_view&);
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
  int Modifier(const std::u8string& m) const;
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
  DArr64<skill_pair, 7> GetSkills() const;
  void SetAttribute(int, int);
  void SetModifier(int, int);
  void SetSkill(uint32_t, int);
  void SetSkill(const std::u8string_view&, int);
  void ClearSkill(uint32_t);
  void ClearSkill(const std::u8string_view&);

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
  int Roll(uint32_t, const Object*, uint32_t, int bias = 0, std::u8string* res = nullptr) const;
  int Roll(uint32_t, int, std::u8string* res = nullptr) const;
  int RollNoWounds(uint32_t, int, int, std::u8string* res = nullptr) const;

  int WoundPenalty() const;

  pos_t Position() const;
  void SetPosition(pos_t p);
  std::u8string PositionString() const;
  void StartUsing(uint32_t skill);
  void StopUsing();
  uint32_t Using() const;
  int IsUsing(uint32_t) const;
  std::u8string UsingString() const;

  int Wearing(const Object* obj) const;
  int WearMask() const;
  std::set<act_t> WearSlots(int m = -1) const;
  std::u8string WearNames(const std::set<act_t>& locs) const;
  std::u8string WearNames(int m = -1) const;

  bool HasKeyFor(const Object* lock, int vmode = 0) const;

  uint32_t Quantity() const;
  void SetQuantity(uint32_t);

  void Collapse();

  void StopAll();
  void AddAct(act_t a, Object* o = nullptr);
  void StopAct(act_t a);
  bool IsAct(act_t a) const {
    for (const auto& ac : actions) {
      if (ac.act() == a) {
        return true;
      }
    }
    return false;
  };
  Object* ActTarg(act_t a) const {
    for (const auto& ac : actions) {
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
  void Send(channel_t channel, const std::u8string_view& mes);
  template <typename S, typename... Args>
  void Send(channel_t channel, const S& mes, Args&&... args)
    requires(sizeof...(args) >= 1)
  {
    Send(channel, fmt::vformat(mes, fmt::make_args_checked<Args...>(mes, args...)));
  };
  void Send(int targ, int rsucc, const std::u8string_view& mes);
  template <typename S, typename... Args>
  void Send(int targ, int rsucc, const S& mes, Args&&... args)
    requires(sizeof...(args) >= 1)
  {
    Send(targ, rsucc, fmt::vformat(mes, fmt::make_args_checked<Args...>(mes, args...)));
  };
  void SendOut(
      int tnum,
      int rsucc,
      const std::u8string& mes,
      const std::u8string& youmes,
      Object* actor,
      Object* targ,
      bool expanding = true);
  template <typename Sm, typename Sy, typename... Args>
  void SendOut(
      int tnum,
      int rsucc,
      const Sm& mes,
      const Sy& youmes,
      Object* actor,
      Object* targ,
      Args&&... args)
    requires(sizeof...(args) >= 1)
  {
    SendOut(
        tnum,
        rsucc,
        fmt::vformat(mes, fmt::make_args_checked<Args...>(mes, args...)),
        fmt::vformat(youmes, fmt::make_args_checked<Args...>(youmes, args...)),
        actor,
        targ);
  };
  void Loud(int str, const std::u8string& mes);
  template <typename S, typename... Args>
  void Loud(int str, const S& mes, Args&&... args)
    requires(sizeof...(args) >= 1)
  {
    Loud(str, fmt::vformat(mes, fmt::make_args_checked<Args...>(mes, args...)));
  };

  void TBALoadAll();
  void TBALoadWLD(const std::u8string&);
  void TBALoadOBJ(const std::u8string&);
  void TBALoadMOB(const std::u8string&);
  void TBAFinishMOB(Object*);
  void TBALoadZON(const std::u8string&);
  void TBALoadSHP(const std::u8string&);
  void TBALoadTRG(const std::u8string&);
  static void TBAFinalizeTriggers();
  static void TBACleanup();

  int Load(const std::u8string&);
  int LoadFrom(std::u8string_view&);
  int Save(const std::u8string&);
  int SaveTo(outfile&);
  std::u8string WriteContents();

  int Tick();

  bool BusyAct();
  bool StillBusy() const;
  void BusyFor(long msec, const std::u8string& default_next = u8"");
  void BusyWith(Object* other, const std::u8string& default_next = u8"");
  void DoWhenFree(const std::u8string&);

  std::u8string Tactics(int phase = -1);

  void operator=(const Object& in);

  bool IsSameAs(const Object& in) const;

  int Matches(const std::u8string_view& seek, bool knows = false) const;
  bool LooksLike(const Object* other, int vmode = 0, const Object* viewer = nullptr) const;

  ObjectTag BuildNPC(const std::u8string_view& tags);
  Object* MakeNPC(std::mt19937&, const ObjectTag& npcdef);
  Object* AddNPC(std::mt19937&, const std::u8string_view& tags);

  ObjectTag BuildRoom(const std::u8string_view& tags);
  Object* MakeRoom(std::mt19937&, const ObjectTag& roomdef);
  Object* AddRoom(std::mt19937&, const std::u8string_view& tags);

  bool LoadTagsFrom(const std::u8string_view& tagdefs, bool save = true);
  bool LoadTags();

  static void FreeActions();

 private:
  void GenerateNPC(const ObjectTag&, std::mt19937&);
  void GenerateRoom(const ObjectTag&, std::mt19937&);

  void NotifyLeft(Object* obj, Object* newloc = nullptr);

  void Loud(std::set<Object*>& visited, int str, const std::u8string& mes);

  bool Filter(int loc) const;

  constexpr static const char8_t* const default_descriptions = u8"a new object\0\0\0\0";
  const char8_t* descriptions = default_descriptions;
  struct {
    uint8_t sd, n;
    uint16_t d, ld;
  } dlens = {10, 0, 0, 0};

  bool no_seek; // Recursion protection
  bool no_hear; // For Send() protection

  int weight; // Also X Coordinate for Zone Locations
  int volume; // Also Y Coordinate for Zone Locations
  int size; // Also Z Coordinate for Zone Locations
  int value;
  gender_t gender;

  int8_t phys, stun, stru;
  int8_t attr[6] = {0, 0, 0, 0, 0, 0};

  int8_t tickstep;

  pos_t position;
  uint32_t quantity = 1;

  DArr32<uint32_t> completed;

  DArr64<Object*, 3> contents;
  Object* parent;
  uint32_t cur_skill;

  uint32_t busy_until = 0; // Encoded
  const char8_t* dowhenfree = u8"";
  const char8_t* defact = u8"";

  int sexp;

  std::forward_list<std::shared_ptr<Mind>> minds;

  DArr32<uint32_t> known;

  DArr64<act_pair, 3> actions;

  DArr64<skill_pair, 7> skills;

  friend void player_rooms_erase(Object*);
};
static_assert(sizeof(Object) == 256); // Prevent Bloat

int roll(int ndice, int targ, std::u8string* res = nullptr);

void init_universe();
void start_universe();
void save_universe(int with_net = 0);
void destroy_universe();
Object* new_body(Object* world);
Object* getbynum(int);
int getonum(Object*);
int matches(const std::u8string_view& name, const std::u8string_view& seek);

uint32_t get_skill(const std::u8string_view& sk);
std::u8string get_skill_cat(const std::u8string_view& cat);
int get_linked(const std::u8string_view& sk);
int get_linked(uint32_t sk);
std::vector<uint32_t> get_skills(const std::u8string_view& cat = u8"Categories");
int is_skill(uint32_t stok);
uint32_t get_weapon_skill(int wtype);
int get_weapon_type(const std::u8string_view& wskill);
int two_handed(int wtype);
std::u8string get_tags_string(Object* world, const DArr32<uint32_t>& tags);

void tick_universe();

Object* new_obj();
Object* new_obj(Object*);
Object* new_obj(const Object&);

inline std::u8string coin_str(size_t amount) {
  std::u8string ret;
  if (amount == 0) {
    ret = u8"0 coins";
  } else if (amount % 10UL != 0) {
    ret = fmt::format(u8"{} chits", amount);
  } else if (amount % 100UL != 0) {
    ret = fmt::format(u8"{} copper pieces", amount / 10);
  } else if (amount % 1000UL != 0) {
    ret = fmt::format(u8"{} silver pieces", amount / 100);
  } else if (amount % 10000UL != 0) {
    ret = fmt::format(u8"{} gold pieces", amount / 1000);
  } else {
    ret = fmt::format(u8"{} platinum pieces", amount / 10000);
  }
  return ret;
}

inline std::u8string coins(size_t amount) {
  std::u8string ret;
  if (amount == 0) {
    ret = u8"0";
  } else if (amount % 10UL != 0) {
    ret = fmt::format(u8"{}ch", amount);
  } else if (amount % 100UL != 0) {
    ret = fmt::format(u8"{}cp", amount / 10);
  } else if (amount % 1000UL != 0) {
    ret = fmt::format(u8"{}sp", amount / 100);
  } else if (amount % 10000UL != 0) {
    ret = fmt::format(u8"{}gp", amount / 1000);
  } else {
    ret = fmt::format(u8"{}pp", amount / 10000);
  }
  return ret;
}

#endif // OBJECT_HPP
