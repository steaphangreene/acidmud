#include "player.h"

#define MIND_MORON 0
#define MIND_REMOTE 1
#define MIND_SLAVE 2
#define MIND_MOB 3
#define MIND_SYSTEM 4

class Mind;
class Object;

#ifndef MIND_H
#define MIND_H

class Mind {
public:
  Mind();
  Mind(int fd);
  ~Mind();
  void SetRemote(int fd);
  void SetMob(int fd);
  void SetSlave(int master);
  void SetSystem();
  void Attach(Object *bod);
  void Unattach();
  Object *Body() { return body; };
  Player *Owner() { return player; };
  string PName() { return pname; };
  void SetPName(string);
  void SetPPass(string);
  void SetPlayer(string);
  int Type() { return type; };
  int ID() { return pers; };
  void Send(const char *, ...) __attribute__ ((format (printf, 2, 3)));
  void SendRaw(const char *, ...) __attribute__ ((format (printf, 2, 3)));
  void UpdatePrompt();

private:
  int type;
  int pers;
  Object *body;
  Player *player;
  string pname;
  };

Mind *new_mind(int);

#endif
