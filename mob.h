#include <string>
#include <map>

class MOBType {
public:
  MOBType(const char *nm, const char *ds, const char *lds,
	int,int, int,int, int,int, int,int, int,int, int,int, int,int);
  void Skill(const char *name, int, int);

  string name, desc, long_desc;
  int b, bm;
  int q, qm;
  int s, sm;
  int c, cm;
  int i, im;
  int w, wm;
  int g, gm;
  map<string, pair<int,int> > skills;
  };
