#include "player.h"

#define MIND_MORON 0
#define MIND_REMOTE 1
#define MIND_CIRCLEMOB 2
#define MIND_TBAMOB 3
#define MIND_TBATRIG 4
#define MIND_MOB 5
#define MIND_SYSTEM 6

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
  void SetTBAMob();
  void SetTBATrigger(Object *tr);
  void SetCircleMob();
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
  int LogFD() { return log; };
  void Send(const char *, ...) __attribute__ ((format (printf, 2, 3)));
  void SendRaw(const char *, ...) __attribute__ ((format (printf, 2, 3)));
  void UpdatePrompt();
  void Think(int istick = 0);
  string Tactics(int phase = -1);

  void SetSpecialPrompt(const char *newp = "");
  const char *SpecialPrompt();

private:
  int CircleCanWanderTo(Object *dest);
  int TBACanWanderTo(Object *dest);

  int type;
  int pers;
  int line;
  Object *body;
  Object *script;
  Player *player;
  string pname;
  string prompt;
  int log;
  };

Mind *new_mind(int tp, Object *obj = NULL);
int new_trigger(Object *obj);

#endif
