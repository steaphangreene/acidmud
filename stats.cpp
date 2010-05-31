#include <cstring>
#include <cstdlib>

#include "object.h"
#include "stats.h"

static map<AtomString,int> defaults;
static map<int,AtomString> weaponskills;
static map<AtomString,int> weapontypes;
static map<AtomString,list<AtomString> > skcat;

static int last_wtype = 0;
static void add_wts(const string &sk) {
  if(defaults.count(sk) == 0) {
    fprintf(stderr,
	"Warning: Tried to link weapon type %d to '%s' which isn't a skill.\n",
	last_wtype+1, sk.c_str());
    return;
    }
  ++last_wtype;
  weaponskills[last_wtype] = sk;
  weapontypes[sk] = last_wtype;
  }

static int defaults_init = 0;
static void init_defaults() {
  if(defaults_init) return;
  defaults_init = 1;

  skill_entry *cur = skill_data;
  while(cur->linked_attr >= 0) {
    defaults[cur->skname] = cur->linked_attr;
    skcat[cur->skcatname].push_back(cur->skname);
    ++cur;
    }

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

  add_wts("Archery");
  add_wts("Blowgun");
  add_wts("Crossbow");
  add_wts("Hurling");
  add_wts("Nets");
  add_wts("Slings");
  add_wts("Staff Slings");
  add_wts("Throwing, Aero");
  add_wts("Throwing, Non-Aero");
  add_wts("Whips");

  add_wts("Mounted Archery");
  add_wts("Mounted Blowgun");
  add_wts("Mounted Crossbow");
  add_wts("Mounted Nets");
  add_wts("Mounted Slings");
  add_wts("Mounted Throwing");
  add_wts("Mounted Whips");

  add_wts("Shields");
  }

int is_skill(string sk) {
  if(!defaults_init) init_defaults();
  return (defaults.count(sk) != 0);
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

string get_skill(string sk) {
  if(defaults.count(sk)) return sk;
  return "";
  }

string get_skill_cat(string cat) {
  if(skcat.count(cat)) return cat;
  return "";
  }

int get_linked(string sk) {
  if(defaults.count(sk)) return defaults[sk];
  return 4; // Default to Int for knowledges
  }

list<string> get_skills(string cat) {
  if(!defaults_init) init_defaults();
  list<string> ret;

  if(cat == "Categories") {
    typeof(skcat.begin()) ind;
    for(ind = skcat.begin(); ind != skcat.end(); ++ind) {
      ret.push_back(ind->first);
      }
    }
  else if(cat == "all") {
    typeof(defaults.begin()) ind;
    for(ind = defaults.begin(); ind != defaults.end(); ++ind) {
      ret.push_back(ind->first);
      }
    }
  else if(skcat.count(cat)) {
    typeof(skcat[cat].begin()) ind;
    for(ind = skcat[cat].begin(); ind != skcat[cat].end(); ++ind) {
      ret.push_back(*ind);
      }
    }

  return ret;
  }

int roll(int ndice, int targ, list<int> *wraps) {
  int ret = 0, ctr;
  for(ctr=0; ctr<ndice; ++ctr) {
    int val = 1+(rand()%6);
    while((val%6) == 0) val += (1+rand()%6);
    if(val > 1 && val >= targ) {
      unsigned int numwrap = (unsigned int)((val - targ) / 12);
      ret += (1 + int(numwrap));
      if(wraps && numwrap > 0) {
	if(wraps->size() < numwrap) wraps->resize(numwrap, 0);
	list<int>::iterator wit = wraps->begin();
	for(unsigned int i=0; i < numwrap; ++i, ++wit) {
	  (*wit) += numwrap - i;
	  }
	}
      }
    }
  return ret;
  }

int Object::Attribute(int a) const {
  return att[a];
  }

void Object::SetAttribute(int a, int v) {
  if(v > 1000000000) v = 1000000000;
  else if(v < -1000000000) v = -1000000000;
  att[a] = v;
  }

void Object::SetSkill(const string &s, int v) {
  if(v > 1000000000) v = 1000000000;
  else if(v < -1000000000) v = -1000000000;
  if(v <= 0) skills.erase(s);
  else skills[s] = v;
  }

int Object::Skill(const string &s, int *tnum) const {
  if(strlen(s.c_str()) == 0) return 0;
  if(!strncasecmp(s.c_str(), "Body", s.length())) return att[0];
  if(!strncasecmp(s.c_str(), "Quickness", s.length())) return att[1];
  if(!strncasecmp(s.c_str(), "Strength", s.length())) return att[2];
  if(!strncasecmp(s.c_str(), "Charisma", s.length())) return att[3];
  if(!strncasecmp(s.c_str(), "Intelligence", s.length())) return att[4];
  if(!strncasecmp(s.c_str(), "Willpower", s.length())) return att[5];
  if(!strncasecmp(s.c_str(), "Reaction", s.length())) return (att[1]+att[4])/2;
  if(!defaults_init) init_defaults();
  if(skills.count(s)) return (skills.find(s))->second;  //const for 'skills[s]'
  if(tnum) {
    (*tnum) += 4;
    return att[defaults[s]];
    }
  return 0;
  }

int Object::Roll(const string &s1, const Object *p2, const string &s2, int bias, string *res) const {
  return Roll(s1, p2, s2, bias, NULL, res);
  }

int Object::Roll(const string &s1, const Object *p2, const string &s2, int bias, list<int> *wraps, string *res) const {
  int succ = 0;

  int t1 = p2->Skill(s2) - bias;
  int t2 = Skill(s1) + bias;

  if(res) (*res) += "(";
  succ = Roll(s1, t1, wraps, res);
  if(s2 != "") {
    if(res) (*res) += " - ";
    succ -= p2->Roll(s2, t2, res);	//FIXME: Contested Wraps!
    }
  if(res) (*res) += ")";
  return succ;
  }

int Object::Roll(const string &s1, int targ, string *res) const {
  return Roll(s1, targ, NULL, res);
  }

int Object::Roll(const string &s1, int targ, list<int> *wraps, string *res) const {
  if(phys>=10 || stun>=10 || (!(att[0]*att[1]*att[2]*att[3]*att[4]*att[5]))) {
    if(res) (*res) += "N/A";
    return 0;
    }
  targ += WoundPenalty();
  return RollNoWounds(s1, targ, wraps, res);
  }

int Object::RollNoWounds(const string &s1, int targ, string *res) const {
  return RollNoWounds(s1, targ, NULL, res);
  }

int Object::RollNoWounds(const string &s1, int targ, list<int> *wraps, string *res) const {
  int succ = 0;
  int d1 = Skill(s1, &targ);
  succ = roll(abs(d1), targ, wraps);

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

int Object::WoundPenalty() const {
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

list<int> Object::RollInitiative() const {
  list<int> ret;
  int start = Roll("Reaction", 6 - att[5], &ret);
  ret.push_front(start);

  /* Begin Debug Output */
//  if(IsAct(ACT_FIGHT)) {
//    fprintf(stderr, "Initiative: [");
//    for(list<int>::iterator it = ret.begin(); it != ret.end();) {
//      fprintf(stderr, "%d", *it);
//      ++it;
//      if(it != ret.end()) fprintf(stderr, ", ");
//      }
//    fprintf(stderr, "] (%s)\n", Name());
//    }
  /* End Debug Output */

  return ret;
  }
