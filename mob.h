#include <string>
#include <map>

class WeaponType {
public:
  WeaponType(const char *nm, const char *ds, const char *lds,
	const char *, int, int,int, int,int, int, int, int);
  string name, desc, long_desc;
  int type, reach;
  int force, forcem;
  int sev, sevm;
  int weight, volume, value;
  };

class MOBType {
public:
  MOBType(const char *nm, const char *ds, const char *lds,
	int,int, int,int, int,int, int,int, int,int, int,int, int,int);
  void Skill(const char *name, int, int);
  void Arm(WeaponType *);

  string name, desc, long_desc;
  int b, bm;
  int q, qm;
  int s, sm;
  int c, cm;
  int i, im;
  int w, wm;
  int g, gm;
  map<string, pair<int,int> > skills;
  WeaponType *armed;
  };
