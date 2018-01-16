#include <map>
#include <vector>

using namespace std;

#include "object.hpp"

class Player;

#ifndef PLAYER_H
#define PLAYER_H

#define PLAYER_NINJAMODE (1 << 29)
#define PLAYER_NINJA (1 << 30)
#define PLAYER_SUPERNINJA (1 << 31)

class Player {
 public:
  Player(string nm, string ps);
  ~Player();
  void SetName(string);
  void Link(Object*);
  Object* Room() {
    return room;
  };
  Object* Creator() {
    return creator;
  };
  void SetCreator(Object* o) {
    creator = o;
  };
  int Is(unsigned long f) {
    return (flags & f);
  };
  void Set(unsigned long f) {
    flags |= f;
  };
  void UnSet(unsigned long f) {
    flags &= (~f);
  };
  int SaveTo(FILE*);
  int LoadFrom(FILE*);
  const char* Name() {
    return name.c_str();
  }
  void AddChar(Object*);
  int Accomplish(unsigned long);
  int Exp() const {
    return exp;
  }
  const map<string, string> Vars() {
    return vars;
  };
  void SetVars(const map<string, string> v) {
    vars = v;
  };

 private:
  map<string, Object*> body;
  string name, pass;
  Object *room, *creator;
  unsigned long flags;
  set<unsigned long> completed;
  int exp;
  map<string, string> vars;

  friend Player* player_login(string name, string pass);
  friend Player* get_player(string name);
};

Player* player_login(string name, string pass);
Player* get_player(string name);
int player_exists(string name);
int save_players(const char* fn);
int load_players(const char* fn);
void player_rooms_erase(Object*);
int is_pc(const Object*);

vector<Player*> get_current_players();
vector<Player*> get_all_players();

#endif
