#include <map>
#include <string>
#include <vector>

#include "object.hpp"

class ArmorType {
 public:
  ArmorType(
      const std::string& nm,
      const std::string& ds,
      const std::string& lds,
      int b,
      int bm,
      int i,
      int im,
      int t,
      int tm,
      int p,
      int pm,
      int w,
      int vol,
      int val,
      act_t l1 = ACT_NONE,
      act_t l2 = ACT_NONE,
      act_t l3 = ACT_NONE,
      act_t l4 = ACT_NONE,
      act_t l5 = ACT_NONE,
      act_t l6 = ACT_NONE);
  std::string name, desc, long_desc;
  std::vector<act_t> loc;
  int bulk, bulkm; // Body-Type (adds mass, reduces shock)
  int impact, impactm; // Cushion (absorbs shock, softens impact)
  int thread, threadm; // Tensile (won't rip/cut, prevents slashing)
  int planar, planarm; // Plate (won't bend, spreads impact)
  int weight, volume, value;
};

class WeaponType {
 public:
  WeaponType(
      const std::string& nm,
      const std::string& ds,
      const std::string& lds,
      const std::string& t,
      int,
      int,
      int,
      int,
      int,
      int w,
      int vol,
      int val);
  std::string name, desc, long_desc;
  int type, reach;
  int force, forcem;
  int sev, sevm;
  int weight, volume, value;
};

class MOBType {
 public:
  MOBType(
      const std::string& nm,
      const std::string& ds,
      const std::string& lds,
      const std::string& gens,
      int,
      int,
      int,
      int,
      int,
      int,
      int,
      int,
      int,
      int,
      int,
      int,
      int,
      int);
  void Skill(uint32_t, int, int);
  void Skill(uint32_t, int);
  void Arm(WeaponType*);
  void Armor(ArmorType*);

  std::string name, desc, long_desc, genders;
  int b, bm;
  int q, qm;
  int s, sm;
  int c, cm;
  int i, im;
  int w, wm;
  int g, gm;
  std::map<uint32_t, std::pair<int, int>> skills;
  WeaponType* armed;
  std::vector<ArmorType*> armor;
};

std::string gender_proc(const std::string& in, char gender);
