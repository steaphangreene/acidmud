#include "player.hpp"

#include <list>
#include <map>

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
  void SetTBATrigger(Object* tr, Object* tripper = NULL, Object* targ = NULL, std::string text = "");
  void SetNPC();
  void SetSystem();
  void Attach(Object* bod);
  void Unattach();
  Object* Body() {
    return body;
  };
  Player* Owner() {
    return player;
  };
  std::string PName() {
    return pname;
  };
  void SetPName(std::string);
  void SetPPass(std::string);
  void SetPlayer(std::string);
  int Type() {
    return type;
  };
  int LogFD() {
    return log;
  };
  void Send(const char*);
  void SendRaw(const char*);
  void SendF(const char*, ...) __attribute__((format(printf, 2, 3)));
  void SendRawF(const char*, ...) __attribute__((format(printf, 2, 3)));
  void UpdatePrompt();

  void Think(int istick = 0);
  std::string Tactics(int phase = -1);

  static void Resume(int passed);
  void Suspend(int msec);
  void Disable();

  void SetSpecialPrompt(const char* newp = "");
  const char* SpecialPrompt();

  int Status() const;
  void ClearStatus();

  void SetSVar(const std::string& var, const std::string& val);
  void ClearSVar(const std::string& var);
  const std::string& SVar(const std::string& var) const;
  int IsSVar(const std::string& var) const;
  void SetSVars(const std::map<std::string, std::string>& sv);
  const std::map<std::string, std::string> SVars() const;

 private:
  int TBACanWanderTo(Object* dest);

  std::string TBAComp(std::string expr);
  int TBAEval(std::string expr);
  void TBAVarSub(std::string& line);

  int TBARunLine(std::string line);

  int type;
  int pers;
  Object* body;
  Player* player;
  std::string pname;
  std::string prompt;
  int log;

  static std::map<std::string, std::string> cvars;
  std::map<std::string, std::string> svars;
  std::map<std::string, Object*> ovars;
  int status;
  std::string script;
  std::list<size_t> spos_s;

  static std::list<std::pair<int, Mind*>> waiting;
};

Mind* new_mind(
    int tp,
    Object* obj = NULL,
    Object* obj2 = NULL,
    Object* obj3 = NULL,
    std::string text = "");
int new_trigger(int msec, Object* obj, Object* tripper = NULL, std::string text = "");
int new_trigger(int msec, Object* obj, Object* tripper, Object* targ, std::string text = "");

#endif
