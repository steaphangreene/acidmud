#include "player.h"

#define MIND_MORON 0
#define MIND_REMOTE 1
#define MIND_SLAVE 2
#define MIND_CIRCLEMOB 3
#define MIND_MOB 4
#define MIND_SYSTEM 5

class Mind;
class Object;

#ifndef MIND_H
#define MIND_H

class Mind {
public:
  Mind();
  Mind(int fd);
  Mind(int fd, int l);
  ~Mind();
  void SetRemote(int fd);
  void SetMob();
  void SetCircleMob();
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
  int LogFD() { return log; };
  void Send(const char *, ...) __attribute__ ((format (printf, 2, 3)));
  void SendRaw(const char *, ...) __attribute__ ((format (printf, 2, 3)));
  void UpdatePrompt();
  void Think(int istick = 0);
  string Tactics(int phase = -1);

private:
  int type;
  int pers;
  Object *body;
  Player *player;
  string pname;
  int log;
  };

Mind *new_mind(int);

#endif
