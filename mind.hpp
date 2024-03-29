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

class Mind;
class Object;

#ifndef MIND_HPP
#define MIND_HPP

#include "player.hpp"

// Replace with C++20 std::format, when widely available
#define FMT_HEADER_ONLY
#include <fmt/format.h>

#include <map>
#include <memory>
#include <vector>

enum class mind_t : uint8_t {
  NONE = 0,
  REMOTE,
  NPC,
  TBAMOB,
  TBATRIG,
  MOB,
  SYSTEM,
  TEST,
};

class Mind : public std::enable_shared_from_this<Mind> {
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
      const std::u8string_view& text = u8"");
  void SetNPC();
  void SetSystem();
  Object* Body() const {
    return body;
  };
  Player* Owner() const {
    return player;
  };
  std::u8string PName() const {
    return pname;
  };
  void SetPName(const std::u8string_view&);
  void SetPPass(const std::u8string_view&);
  void SetPlayer(const std::u8string_view&);
  mind_t Type() const {
    return type;
  };
  int LogFD() const {
    return log;
  };

  bool Send(const std::u8string_view&); // Returns false when mind needs to be deleted
  template <typename S, typename... Args>
  bool Send(const S& mes, Args&&... args)
    requires(sizeof...(args) >= 1)
  {
    return Send(fmt::vformat(mes, fmt::make_args_checked<Args...>(mes, args...)));
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

  void StartNewProject();
  void ContinueWorkOn(Object* project);

  // Returns how much NPC/MOB would pay for item, or 0.
  size_t WouldBuyFor(const Object* item);

  void SetSVar(const std::u8string& var, const std::u8string& val);
  void ClearSVar(const std::u8string& var);
  const std::u8string& SVar(const std::u8string& var) const;
  int IsSVar(const std::u8string& var) const;
  void SetSVars(const std::map<std::u8string, std::u8string>& sv);
  const std::map<std::u8string, std::u8string> SVars() const;

 private:
  // Returns how much NPC/MOB would pay for item, or 0.
  size_t NPCWouldBuyFor(const Object* item);
  size_t TBAMOBWouldBuyFor(const Object* item);
  std::u8string TBAMOBTactics(int phase) const;
  bool TBAMOBSend(const std::u8string_view&); // Returns false when mind needs to be deleted
  bool TBAMOBThink(int istick); // Returns false when mind needs to be deleted
  bool TBATriggerThink(int istick); // Returns false when mind needs to be deleted

  int TBACanWanderTo(Object* dest) const;

  std::u8string TBAComp(const std::u8string_view& expr) const;
  int TBAEval(const std::u8string_view& expr) const;
  bool TBAVarSub(std::u8string& line) const; // Returns false when mind needs to be deleted

  int TBARunLine(std::u8string line);

  std::u8string pname;
  std::u8string prompt;
  Player* player = nullptr;
  Object* body = nullptr;
  friend class Object;

  static std::map<std::u8string, std::u8string> cvars;
  std::map<std::u8string, std::u8string> svars;
  std::map<std::u8string, Object*> ovars;

  mind_t type = mind_t::NONE;
  int status = 0; // AI Failures
  int pers = 0; // File Descriptor
  int log = -1; // File Descriptor

  std::vector<size_t> spos_s;

  static std::vector<std::pair<int64_t, std::shared_ptr<Mind>>> waiting;
};

std::shared_ptr<Mind> get_mob_mind();

std::shared_ptr<Mind> get_tbamob_mind();

std::shared_ptr<Mind> new_mind(
    mind_t tp,
    Object* obj = nullptr,
    Object* obj2 = nullptr,
    Object* obj3 = nullptr,
    const std::u8string_view& text = u8"");
int new_trigger(
    int msec,
    Object* obj,
    Object* tripper = nullptr,
    const std::u8string_view& text = u8"");
int new_trigger(
    int msec,
    Object* obj,
    Object* tripper,
    Object* targ,
    const std::u8string_view& text = u8"");

#endif // MIND_HPP
