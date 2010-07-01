#include "player.h"

#include <list>

#define MIND_MORON 0
#define MIND_REMOTE 1
#define MIND_NPC 2
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
  void Init();
  void SetRemote(int fd);
  void SetMob();
  void SetTBAMob();
  void SetTBATrigger(Object *tr,
	Object *tripper = NULL, Object *targ = NULL, string text = ""
	);
  void SetNPC();
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
  void Send(const char *);
  void SendRaw(const char *);
  void SendF(const char *, ...) __attribute__ ((format (printf, 2, 3)));
  void SendRawF(const char *, ...) __attribute__ ((format (printf, 2, 3)));
  void UpdatePrompt();

  void Think(int istick = 0);
  string Tactics(int phase = -1);

  static void Resume(int passed);
  void Suspend(int msec);
  void Disable();

  void SetSpecialPrompt(const char *newp = "");
  const char *SpecialPrompt();

  int Status() const;
  void ClearStatus();

private:
  int TBACanWanderTo(Object *dest);

  string TBAComp(string expr);
  int TBAEval(string expr);
  void TBAVarSub(string &line);

  int TBARunLine(string line);

  int type;
  int pers;
  Object *body;
  Player *player;
  string pname;
  string prompt;
  int log;

  static map<string, string> cvars;
  map<string, string> svars;
  map<string, Object *> ovars;
  int status;
  string script;
  list<size_t> spos_s;

  static list<pair<int, Mind*> > waiting;
  };

Mind *new_mind(int tp,
	Object *obj = NULL, Object *obj2 = NULL, Object *obj3 = NULL,
	string text = ""
	);
int new_trigger(int msec,
	Object *obj, Object *tripper = NULL, string text = ""
	);
int new_trigger(int msec,
	Object *obj, Object *tripper, Object *targ, string text = ""
	);

#endif
