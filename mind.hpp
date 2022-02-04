// *************************************************************************
//  This file is part of AcidMUD by Steaphan Greene
//
//  Copyright 1999-2022 Steaphan Greene <steaphan@gmail.com>
//
//  AcidMUD is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 3 of the License, or
//  (at your option) any later version.
//
//  AcidMUD is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with AcidMUD (see the file named "COPYING");
//  If not, see <http://www.gnu.org/licenses/>.
//
// *************************************************************************

#include "player.hpp"

#include <map>
#include <vector>

#define MIND_MORON 0
#define MIND_REMOTE 1
#define MIND_NPC 2
#define MIND_TBAMOB 3
#define MIND_TBATRIG 4
#define MIND_MOB 5
#define MIND_SYSTEM 6

class Mind;
class Object;

#ifndef MIND_HPP
#define MIND_HPP

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
  void SetTBATrigger(
      Object* tr,
      Object* tripper = nullptr,
      Object* targ = nullptr,
      std::string text = "");
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
  void Send(const std::string&);
  void SendRaw(const std::string&);
  void SendF(const char*, ...) __attribute__((format(printf, 2, 3)));
  void SendRawF(const char*, ...) __attribute__((format(printf, 2, 3)));
  void UpdatePrompt();

  void Think(int istick = 0);
  std::string Tactics(int phase = -1);

  static void Resume();
  void Suspend(int msec);
  void Disable();

  void SetSpecialPrompt(const std::string& newp);
  std::string SpecialPrompt();

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
  std::vector<size_t> spos_s;

  static std::vector<std::pair<int64_t, Mind*>> waiting;
};

Mind* new_mind(
    int tp,
    Object* obj = nullptr,
    Object* obj2 = nullptr,
    Object* obj3 = nullptr,
    std::string text = "");
int new_trigger(int msec, Object* obj, Object* tripper = nullptr, std::string text = "");
int new_trigger(int msec, Object* obj, Object* tripper, Object* targ, std::string text = "");

#endif // MIND_HPP
