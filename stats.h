#include <map>
#include <string>
#include <cstdio>
#include <fcntl.h>

using namespace std;

class stats_t;

#ifndef STATS_H
#define STATS_H

int roll(int ndice, int targ);

class stats_t {
public:
  int SaveTo(FILE *) const;
  int LoadFrom(FILE *);

  int GetAttribute(int) const;
  int GetSkill(const string &, int *tnum = NULL) const;
  map<string,int> GetSkills() const { return skills; }

  void SetAttribute(int, int);
  void SetSkill(const string &, int);

  int RollInitiative() const;
  int Roll(const string &, const stats_t *, const string &, int bias = 0, string *res=NULL) const;
  int Roll(const string &, int, string *res=NULL) const;
  int RollNoWounds(const string &, int, string *res=NULL) const;

  int WoundPenalty() const;
  int phys, stun, stru;
  int weight, volume, size;
  int value;

private:
  int att[8];
  map<string,int> skills;
  };

map<string,int> get_skills();
string get_weapon_skill(int wtype);
int get_weapon_type(string wskill);

#endif
