#include <set>

using namespace std;

#include <crypt.h>

#include "player.h"

static map<string, Player *> player_list;
static set<Player *> non_init;

static const char *salt_char = 
	"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./";

Player::Player(string nm, string ps) {
  flags = 0;

  room = new Object();
  creator = NULL;

  SetName(nm);
  if(ps[0] == '$' && ps[1] == '1' && ps[2] == '$') {
    pass = ps;
    }
  else {
    string salt = "$1$", app = " ";
    for(int ctr=0; ctr<8; ++ctr) {
      app[0] = salt_char[rand()&63];
      salt += app;
      }
    pass = crypt(ps.c_str(), salt.c_str());
    }
  player_list[name] = this;
  non_init.insert(this);
  }

Player::~Player() {
  player_list.erase(name);
  non_init.erase(this);
  }

void Player::SetName(string nm) {
  player_list.erase(name);
  name = nm;
  player_list[name] = this;
  string desc = nm + "'s characters";
  room->SetShortDesc(desc.c_str());
  desc = (string)"Here are all of " + nm + "'s characters:";
  room->SetDesc(desc.c_str());
  }

void Player::Link(Object *obj) {
  }

void Player::AddChar(Object *ch) {
  room->AddLink(ch);
  creator = ch;
  fprintf(stderr, "Added %p\n", ch);
  fprintf(stderr, "Added %s\n", ch->Name());
  }

int player_exists(string name) {
  return player_list.count(name);
  }

Player *get_player(string name) {
  if(!player_list.count(name)) return NULL;

  Player *pl = player_list[name];
  non_init.erase(pl);
  
  return pl;
  }

Player *player_login(string name, string pass) {
  if(!player_list.count(name)) return NULL;

  Player *pl = player_list[name];
  string enpass = crypt(pass.c_str(), pl->pass.c_str());

//  fprintf(stderr, "Trying %s:%s\n", name.c_str(), enpass.c_str());

  if(enpass != pl->pass) {
    if(non_init.count(pl)) {
      player_list.erase(name);
      non_init.erase(pl);
      }
    return NULL;
    }

  non_init.erase(pl);
  if((player_list.size() - non_init.size()) == 1) {
    pl->Set(PLAYER_NINJA);
    pl->Set(PLAYER_SUPERNINJA);
    }
  return pl;
  }

static char buf[65536];
const int SAVEFILE_VERSION=102;
int Player::LoadFrom(FILE *fl) {
  memset(buf, 0, 65536);
  fscanf(fl, "%s\n", buf);
//  name = buf;
  SetName(buf);
  memset(buf, 0, 65536);
  fscanf(fl, "%s\n", buf);
  pass = buf;
  fscanf(fl, "%lX\n", &flags);

  int num;
  while(fscanf(fl, ":%d\n", &num) > 0) {
    AddChar(getbynum(num));
    fprintf(stderr, "Added %d to %s\n", num, name.c_str());
    }

  return 0;
  }

int load_players(const char *fn) {
  fprintf(stderr, "Loading Players.\n");

  FILE *fl = fopen(fn, "r");
  if(!fl) return -1;

  int ver, num;
  fscanf(fl, "%d\n%d\n", &ver, &num);

  fprintf(stderr, "Loading Players: %d,%d\n", ver, num);

  for(int ctr=0; ctr<num; ++ctr) {
    Player *pl = new Player(";;TEMP;;", ";;TEMP;;");
    non_init.erase(pl);
    pl->LoadFrom(fl);
    //fprintf(stderr, "Loaded Player: %s\n", pl->Name());
    }

  fclose(fl);
  return 0;
  }

int Player::SaveTo(FILE *fl) {
  fprintf(fl, "%s\n%s\n%lX", name.c_str(), pass.c_str(), flags);
  room->WriteContentsTo(fl);
  fprintf(fl, "\n");
  return 0;
  }

int save_players(const char *fn) {
  FILE *fl = fopen(fn, "w");
  if(!fl) return -1;

  fprintf(fl, "%d\n", SAVEFILE_VERSION);

  fprintf(fl, "%d\n", player_list.size() - non_init.size());

  map<string, Player *>::iterator pl = player_list.begin();
  for(; pl != player_list.end(); ++pl) {
    if(non_init.count((*pl).second) == 0) {
      if((*pl).second->SaveTo(fl)) return -1;
      }
    }

  fclose(fl);
  return 0;
  }

void player_rooms_erase(Object *obj) {
  map<string, Player *>::iterator pl = player_list.begin();
  for(; pl != player_list.end(); ++pl) {
    if((*pl).second->Creator() == obj)
      (*pl).second->SetCreator(NULL);
    (*pl).second->Room()->contents.erase(obj);
    }
  }

int is_pc(Object *obj) {
  map<string, Player *>::iterator pl = player_list.begin();
  for(; pl != player_list.end(); ++pl) {
    if(pl->second->Room() && pl->second->Room()->Contains(obj)) {
      return 1;
      }
    }
  return 0;
  }

vector<Player *> get_current_players() {
  fprintf(stderr, "Warning: %s called, but not implemented yet!\n",
	__PRETTY_FUNCTION__);
  vector<Player *> ret;
  return ret;
  }

vector<Player *> get_all_players() {
  vector<Player *> ret;
  map<string, Player *>::iterator pl = player_list.begin();
  for(; pl != player_list.end(); ++pl) {
    if((*pl).second->Room()) {
      ret.push_back((*pl).second);
      }
    }
  return ret;
  }
