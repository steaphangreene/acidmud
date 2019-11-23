#include <map>
#include <set>
#include <vector>

#include "object.hpp"

class Player;

#ifndef PLAYER_H
#define PLAYER_H

#define PLAYER_NINJAMODE (1 << 29)
#define PLAYER_NINJA (1 << 30)
#define PLAYER_SUPERNINJA (1 << 31)

class Player {
 public:
  Player(std::string nm, std::string ps);
  ~Player();
  void SetName(std::string);
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
  const std::map<std::string, std::string> Vars() {
    return vars;
  };
  void SetVars(const std::map<std::string, std::string> v) {
    vars = v;
  };

 private:
  std::map<std::string, Object*> body;
  std::string name, pass;
  Object *room, *creator;
  unsigned long flags;
  std::map<std::string, std::string> vars;

  friend Player* player_login(std::string name, std::string pass);
  friend Player* get_player(std::string name);
};

Player* player_login(std::string name, std::string pass);
Player* get_player(std::string name);
int player_exists(std::string name);
int save_players(const char* fn);
int load_players(const char* fn);
void player_rooms_erase(Object*);
int is_pc(const Object*);

std::vector<Player*> get_current_players();
std::vector<Player*> get_all_players();

#endif
