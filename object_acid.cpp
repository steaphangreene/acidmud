#include <vector>
#include <set>
#include <map>

using namespace std;
const int SAVEFILE_VERSION = 102;

#include "object.h"
#include "mind.h"

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

  fprintf(fl, "%d\n", SAVEFILE_VERSION);

  if(SaveTo(fl)) { fclose(fl); return -1; }

  fprintf(fl, "%d\n", getnum(get_start_room()));

  fclose(fl);
  return 0;
  }

int Object::SaveTo(FILE *fl) {
  fprintf(fl, "%d\n", getnum(this));
  fprintf(fl, "%s;\n", short_desc.c_str());
  fprintf(fl, "%s;\n", desc.c_str());
  fprintf(fl, "%s;\n", long_desc.c_str());

  fprintf(fl, "%d\n", connections.size());
  map<string,Object*>::iterator dind;
  for(dind = connections.begin(); dind != connections.end(); ++dind) {
    fprintf(fl, "%s;%d\n", (*dind).first.c_str(), getnum((*dind).second));
    }

  fprintf(fl, "%d\n", contents.size());
  set<Object*>::iterator cind;
  for(cind = contents.begin(); cind != contents.end(); ++cind) {
    fprintf(fl, "%d\n", getnum(*cind));
    }

  fprintf(fl, "%d\n", pos);

  fprintf(fl, "%d\n", act.size());
  map<act_t,Object*>::iterator aind;
  for(aind = act.begin(); aind != act.end(); ++aind) {
    fprintf(fl, "%d;%d\n", (*aind).first, getnum((*aind).second));
    }

  stats.SaveTo(fl);

  fprintf(fl, "\n");

  for(cind = contents.begin(); cind != contents.end(); ++cind) {
    (*cind)->SaveTo(fl);
    }
  return 0;
  }

static int ver;
int Object::Load(const char *fn) {
  num2obj[0] = NULL;
  obj2num[NULL] = 0;

  FILE *fl = fopen(fn, "r");
  if(!fl) return -1;

  fscanf(fl, "%d\n", &ver);

  todo.clear();
  if(LoadFrom(fl)) { fclose(fl); return -1; }
  
  vector<Object*>::iterator ind;
  for(ind = todo.begin(); ind != todo.end(); ++ind) {
    map<string,Object*>::iterator dind = (*ind)->connections.begin();
    for(; dind != (*ind)->connections.end(); ++dind) {
      int num = int((*dind).second);
      (*dind).second = num2obj[num];
      }
    map<act_t,Object*>::iterator aind = (*ind)->act.begin();
    for(; aind != (*ind)->act.end(); ++aind) {
      int num = int((*aind).second);
      (*aind).second = num2obj[num];
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
  int num, res;
  fscanf(fl, "%d ", &num);
  num2obj[num] = this;
  todo.push_back(this);

  memset(buf, 0, 65536);
  res = fscanf(fl, "%[^;]; ", buf);  short_desc = buf;
  if(res < 1) fscanf(fl, " ; ");

  //printf("Loading %d:%s\n", num, buf);

  memset(buf, 0, 65536);
  res = fscanf(fl, "%[^;];\n", buf);  desc = buf;
  if(res < 1) fscanf(fl, " ; ");

  memset(buf, 0, 65536);
  res = fscanf(fl, "%[^;]; ", buf);  long_desc = buf;
  if(res < 1) fscanf(fl, " ; ");

  fscanf(fl, "%d ", &num);
  for(int ctr=0; ctr<num; ++ctr) {
    memset(buf, 0, 65536);  int num2;
    fscanf(fl, "%[^;];%d ", buf, &num2);
    connections[buf] = (Object*)num2;
    }

  vector<Object*> toload;
  fscanf(fl, "%d ", &num);
  for(int ctr=0; ctr<num; ++ctr) {
    int num2;
    fscanf(fl, "%d ", &num2);
    Object *obj = new Object(this);
    toload.push_back(obj);
    contents.insert(obj);
    }

  fscanf(fl, "%d\n", &num); pos = (pos_t)num;

  fscanf(fl, "%d ", &num);
  for(int ctr=0; ctr<num; ++ctr) {
    int anum, num2;
    fscanf(fl, "%d;%d ", &anum, &num2);
    act[(act_t)anum] = (Object*)num2;
    }

  stats.LoadFrom(fl);
  if(stats.GetSkill("CircleAction")) get_mob_mind()->Attach(this);

  vector<Object*>::iterator cind;
  for(cind = toload.begin(); cind != toload.end(); ++cind) {
    (*cind)->LoadFrom(fl);
    }
  return 0;
  }
