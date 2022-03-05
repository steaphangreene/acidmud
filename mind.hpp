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

// Replace with C++20 std::format, when widely available
#define FMT_HEADER_ONLY
#include <fmt/format.h>

#include <map>
#include <vector>

class Mind;
class Object;

#ifndef MIND_HPP
#define MIND_HPP

enum class mind_t : uint8_t {
  NONE = 0,
  REMOTE = 1,
  NPC = 2,
  TBAMOB = 3,
  TBATRIG = 4,
  MOB = 5,
  SYSTEM = 6,
};

class Mind {
 public:
  Mind() = default; // New mind of unknown type
  Mind(mind_t t) : type(t){}; // New mind of known type (MOBs)
  Mind(int fd) { // Player, active net connected
    SetRemote(fd);
  };
  Mind(int fd, int l) : log(l) { // Player, active net connected, with active log
    SetRemote(fd);
  };
  ~Mind();
  void SetRemote(int fd);
  void SetMob();
  void SetTBAMob();
  void SetTBATrigger(
      Object* tr,
      Object* tripper = nullptr,
      Object* targ = nullptr,
      std::u8string text = u8"");
  void SetNPC();
  void SetSystem();
  void Attach(Object* bod);
  void Unattach();
  Object* Body() const {
    return body;
  };
  Player* Owner() const {
    return player;
  };
  std::u8string PName() const {
    return pname;
  };
  void SetPName(std::u8string);
  void SetPPass(std::u8string);
  void SetPlayer(std::u8string);
  mind_t Type() const {
    return type;
  };
  int LogFD() const {
    return log;
  };

  bool Send(const std::u8string&); // Returns false when mind needs to be deleted
  template <typename... Args>
  bool Send(const std::u8string& mes, Args... args) {
    auto buf = fmt::format(mes, args...);
    return Send(buf);
  };

  void UpdatePrompt();

  bool Think(int istick = 0); // Returns false when mind needs to be deleted
  std::u8string Tactics(int phase = -1) const;

  static void Resume();
  void Suspend(int msec);

  void SetSpecialPrompt(const std::u8string& newp);
  std::u8string SpecialPrompt() const;

  int Status() const;
  void ClearStatus();

  void SetSVar(const std::u8string& var, const std::u8string& val);
  void ClearSVar(const std::u8string& var);
  const std::u8string& SVar(const std::u8string& var) const;
  int IsSVar(const std::u8string& var) const;
  void SetSVars(const std::map<std::u8string, std::u8string>& sv);
  const std::map<std::u8string, std::u8string> SVars() const;

 private:
  int TBACanWanderTo(Object* dest) const;

  std::u8string TBAComp(std::u8string expr) const;
  int TBAEval(std::u8string expr) const;
  bool TBAVarSub(std::u8string& line) const; // Returns false when mind needs to be deleted

  int TBARunLine(std::u8string line);

  std::u8string pname;
  std::u8string prompt;
  Player* player = nullptr;
  Object* body = nullptr;

  static std::map<std::u8string, std::u8string> cvars;
  std::map<std::u8string, std::u8string> svars;
  std::map<std::u8string, Object*> ovars;

  mind_t type = mind_t::NONE;
  int status = 0; // AI Failures
  int pers = 0; // File Descriptor
  int log = -1; // File Descriptor

  std::u8string script;
  std::vector<size_t> spos_s;

  static std::vector<std::pair<int64_t, Mind*>> waiting;
};

Mind* new_mind(
    mind_t tp,
    Object* obj = nullptr,
    Object* obj2 = nullptr,
    Object* obj3 = nullptr,
    std::u8string text = u8"");
int new_trigger(int msec, Object* obj, Object* tripper = nullptr, std::u8string text = u8"");
int new_trigger(int msec, Object* obj, Object* tripper, Object* targ, std::u8string text = u8"");

#endif // MIND_HPP
