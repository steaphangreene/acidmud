#include <cstdlib>

#include "stats.h"

const int SAVEFILE_VERSION=102;

static map<string,int> defaults;
static map<int,string> weaponskills;
static map<string,int> weapontypes;

static int last_wtype;
static void add_wts(const string &sk) {
  ++last_wtype;
  weaponskills[last_wtype] = sk;
  weapontypes[sk] = last_wtype;
  defaults[sk] = 1;  //Default of Quickness
  }

static int defaults_init = 0;
static void init_defaults() {
  if(defaults_init) return;
  defaults_init = 1;
  last_wtype = 0;

  add_wts("Grappling");
  add_wts("Kicking");
  add_wts("Punching");
  add_wts("Short Blades");
  add_wts("Short Cleaves");
  add_wts("Short Crushing");
  add_wts("Short Piercing");
  add_wts("Flails");
  add_wts("Staves");
  add_wts("Long Blades");
  add_wts("Long Cleaves");
  add_wts("Long Crushing");
  add_wts("Long Piercing");
  add_wts("Two-Handed Blades");
  add_wts("Two-Handed Cleaves");
  add_wts("Two-Handed Crushing");
  add_wts("Two-Handed Flails");
  add_wts("Two-Handed Piercing");
  add_wts("Two-Handed Staves");

  add_wts("Offhand Blades");
  add_wts("Offhand Cleaves");
  add_wts("Offhand Crushing");
  add_wts("Offhand Flails");
  add_wts("Offhand Piercing");
  add_wts("Offhand Staves");

  add_wts("Mounted Blades");
  add_wts("Mounted Cleaves");
  add_wts("Mounted Crushing");
  add_wts("Mounted Flails");
  add_wts("Mounted Piercing");
  add_wts("Mounted Staves");

  add_wts("Archery");
  add_wts("Blowgun");
  add_wts("Crossbow");
  add_wts("Hurling");
  add_wts("Nets");
  add_wts("Slings");
  add_wts("Staff Slings");
  add_wts("Throwing (Aero)");
  add_wts("Throwing (Non-Aero)");
  add_wts("Whips");

  add_wts("Mounted Archery");
  add_wts("Mounted Blowgun");
  add_wts("Mounted Crossbow");
  add_wts("Mounted Nets");
  add_wts("Mounted Slings");
  add_wts("Mounted Throwing (Aero)");
  add_wts("Mounted Throwing (Non-Aero)");
  add_wts("Mounted Whips");

  add_wts("Shields");

  //Default to Quickness if not listed here!
  defaults["Grappling"] = 2;
  defaults["Shields"] = 2;
  defaults["Two-Handed Blades"] = 2;
  defaults["Two-Handed Cleaves"] = 2;
  defaults["Two-Handed Crushing"] = 2;
  defaults["Two-Handed Flails"] = 2;
  defaults["Two-Handed Piercing"] = 2;
  defaults["Hurling"] = 2;
  }

string get_weapon_skill(int wtype) {
  if(!defaults_init) init_defaults();
  if(!weaponskills.count(wtype)) {
    fprintf(stderr, "Warning: No Skill Type %d!\n", wtype);
    return "None";
    }
  return weaponskills[wtype];
  }

int get_weapon_type(string wskill) {
  if(!defaults_init) init_defaults();
  if(!weapontypes.count(wskill)) {
    fprintf(stderr, "Warning: No Skill Named '%s'!\n", wskill.c_str());
    return 0;
    }
  return weapontypes[wskill];
  }

map<string,int> get_skills() {
  if(!defaults_init) init_defaults();
  return defaults;
  }

int roll(int ndice, int targ) {
  int ret = 0, ctr;
  for(ctr=0; ctr<ndice; ++ctr) {
    int val = 1+(rand()%6);
    while((val%6) == 0) val += (1+rand()%6);
    while(val > 1 && val >= targ) { ++ret; val -= 12; }
    }
  return ret;
  }

int stats_t::GetAttribute(int a) const {
  return att[a];
  }

void stats_t::SetAttribute(int a, int v) {
  att[a] = v;
  }

void stats_t::SetSkill(const string &s, int v) {
  skills[s] = v;
  if(v <= 0) skills.erase(s);
  }

int stats_t::GetSkill(const string &s, int *tnum) const {
  if(!defaults_init) init_defaults();
  if(skills.count(s)) return (skills.find(s))->second;  //const for 'skills[s]'
  if(tnum) {
    (*tnum) += 4;
    return att[defaults[s]];
    }
  return 0;
  }

int stats_t::SaveTo(FILE *fl) const {
  fprintf(fl, "%d:", SAVEFILE_VERSION);
  fprintf(fl, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
	att[0], att[1], att[2], att[3], att[4], att[5], att[6], att[7],
	phys, stun, stru);
  map<string,int>::const_iterator sk = skills.begin();
  for(; sk != skills.end(); ++sk)
    fprintf(fl, ":%s,%d", (*sk).first.c_str(), (*sk).second);
  fprintf(fl, "\n");
  return 0;
  }

int stats_t::LoadFrom(FILE *fl) {
  int ver;
  fscanf(fl, "%d:", &ver);
  fscanf(fl, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
	&att[0], &att[1], &att[2], &att[3], &att[4], &att[5], &att[6], &att[7],
	&phys, &stun, &stru);
  static char buf[65536];
  memset(buf, 0, 65536);
  int val;
  while(fscanf(fl, ":%[^\n,:],%d", buf, &val)) {
    //fprintf(stderr, "Loaded %s: %d\n", buf, val);
    skills[buf] = val;
    }
  return 0;
  }

int stats_t::Roll(const string &s1, const stats_t *p2, const string &s2, int bias, string *res) const {
  int succ = 0;

  int t1 = p2->GetSkill(s2) - bias;
  int t2 = GetSkill(s1) + bias;

  if(res) (*res) += "(";
  succ = Roll(s1, t1, res);
  if(res) (*res) += " - ";
  succ -= p2->Roll(s2, t2, res);
  if(res) (*res) += ")";
  return succ;
  }

int stats_t::Roll(const string &s1, int targ, string *res) const {
  if(phys>=10 || stun>=10 || (!(att[0]*att[1]*att[2]*att[3]*att[4]*att[5]))) {
    if(res) (*res) += "N/A";
    return 0;
    }
  targ += WoundPenalty();
  return RollNoWounds(s1, targ, res);
  }

int stats_t::RollNoWounds(const string &s1, int targ, string *res) const {
  int succ = 0;
  int d1 = GetSkill(s1, &targ);
  succ = roll(abs(d1), targ);

  if(res) {
    char buffer[256];
    sprintf(buffer, "%d%c", abs(d1), 0);
    (*res) += buffer;
    (*res) += "(";
    sprintf(buffer, "%d%c", targ, 0);
    (*res) += buffer;
    (*res) += "):";
    sprintf(buffer, "%d%c", succ, 0);
    (*res) += buffer;
    }

  return succ;
  }

int stats_t::WoundPenalty() const {
  int ret = 0;
  if(stun >= 10) ret += 4;
  else if(stun >= 6) ret += 3;
  else if(stun >= 3) ret += 2;
  else if(stun >= 1) ret += 1;
  if(phys >= 10) ret += 4;
  else if(phys >= 6) ret += 3;
  else if(phys >= 3) ret += 2;
  else if(phys >= 1) ret += 1;

  return ret;
  }

int stats_t::RollInitiative() const {
  int ret = 1+(rand()%6);
  ret += 1+(rand()%6);
  ret += att[1];
  ret += att[4];
  ret -= WoundPenalty();

  return ret;
  }

stats_t zero_stats;
