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

#include "test_main.hpp"

#include "../enum_utils.hpp"
#include "../object.hpp"

TEST_CASE("Test enum Names", "[enums]") {
  SECTION("Test pos_t") {
    REQUIRE(enum_save(pos_t::NONE) == u8"NONE");
    REQUIRE(enum_save(pos_t::LIE) == u8"LIE");
    REQUIRE(enum_save(pos_t::SIT) == u8"SIT");
    REQUIRE(enum_save(pos_t::PROP) == u8"PROP");
    REQUIRE(enum_save(pos_t::STAND) == u8"STAND");
    REQUIRE(enum_save(pos_t::USE) == u8"USE");
  }

  SECTION("Test act_t") {
    REQUIRE(enum_save(act_t::NONE) == u8"NONE");
    REQUIRE(enum_save(act_t::DEAD) == u8"DEAD");
    REQUIRE(enum_save(act_t::DYING) == u8"DYING");
    REQUIRE(enum_save(act_t::UNCONSCIOUS) == u8"UNCONSCIOUS");
    REQUIRE(enum_save(act_t::SLEEP) == u8"SLEEP");
    REQUIRE(enum_save(act_t::REST) == u8"REST");
    REQUIRE(enum_save(act_t::WORK) == u8"WORK");
    REQUIRE(enum_save(act_t::HEAL) == u8"HEAL");
    REQUIRE(enum_save(act_t::POINT) == u8"POINT");
    REQUIRE(enum_save(act_t::FOLLOW) == u8"FOLLOW");
    REQUIRE(enum_save(act_t::FIGHT) == u8"FIGHT");
    REQUIRE(enum_save(act_t::OFFER) == u8"OFFER");
    REQUIRE(enum_save(act_t::HOLD) == u8"HOLD");
    REQUIRE(enum_save(act_t::WEAR_BACK) == u8"WEAR_BACK");
    REQUIRE(enum_save(act_t::WEAR_CHEST) == u8"WEAR_CHEST");
    REQUIRE(enum_save(act_t::WEAR_HEAD) == u8"WEAR_HEAD");
    REQUIRE(enum_save(act_t::WEAR_NECK) == u8"WEAR_NECK");
    REQUIRE(enum_save(act_t::WEAR_COLLAR) == u8"WEAR_COLLAR");
    REQUIRE(enum_save(act_t::WEAR_WAIST) == u8"WEAR_WAIST");
    REQUIRE(enum_save(act_t::WEAR_SHIELD) == u8"WEAR_SHIELD");
    REQUIRE(enum_save(act_t::WEAR_LARM) == u8"WEAR_LARM");
    REQUIRE(enum_save(act_t::WEAR_RARM) == u8"WEAR_RARM");
    REQUIRE(enum_save(act_t::WEAR_LFINGER) == u8"WEAR_LFINGER");
    REQUIRE(enum_save(act_t::WEAR_RFINGER) == u8"WEAR_RFINGER");
    REQUIRE(enum_save(act_t::WEAR_LFOOT) == u8"WEAR_LFOOT");
    REQUIRE(enum_save(act_t::WEAR_RFOOT) == u8"WEAR_RFOOT");
    REQUIRE(enum_save(act_t::WEAR_LHAND) == u8"WEAR_LHAND");
    REQUIRE(enum_save(act_t::WEAR_RHAND) == u8"WEAR_RHAND");
    REQUIRE(enum_save(act_t::WEAR_LLEG) == u8"WEAR_LLEG");
    REQUIRE(enum_save(act_t::WEAR_RLEG) == u8"WEAR_RLEG");
    REQUIRE(enum_save(act_t::WEAR_LWRIST) == u8"WEAR_LWRIST");
    REQUIRE(enum_save(act_t::WEAR_RWRIST) == u8"WEAR_RWRIST");
    REQUIRE(enum_save(act_t::WEAR_LSHOULDER) == u8"WEAR_LSHOULDER");
    REQUIRE(enum_save(act_t::WEAR_RSHOULDER) == u8"WEAR_RSHOULDER");
    REQUIRE(enum_save(act_t::WEAR_LHIP) == u8"WEAR_LHIP");
    REQUIRE(enum_save(act_t::WEAR_RHIP) == u8"WEAR_RHIP");
    REQUIRE(enum_save(act_t::WEAR_FACE) == u8"WEAR_FACE");
    REQUIRE(enum_save(act_t::SPECIAL_LINKED) == u8"SPECIAL_LINKED");
    REQUIRE(enum_save(act_t::SPECIAL_HOME) == u8"SPECIAL_HOME");
    REQUIRE(enum_save(act_t::SPECIAL_WORK) == u8"SPECIAL_WORK");
    REQUIRE(enum_save(act_t::SPECIAL_OWNER) == u8"SPECIAL_OWNER");
    REQUIRE(enum_save(act_t::SPECIAL_ACTEE) == u8"SPECIAL_ACTEE");
  }
}
