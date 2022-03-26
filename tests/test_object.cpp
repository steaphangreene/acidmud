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

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include "../object.hpp"

TEST_CASE("Object", "Basic Sanity") {
  auto universe = new Object();
  REQUIRE(universe != nullptr);

  new Object(universe);
  new Object(universe);
  new Object(universe);
  auto inobj = new Object(universe);
  REQUIRE(universe->HasWithin(inobj));

  REQUIRE(universe->Contents().size() == 4);

  delete inobj;
  REQUIRE(universe->Contents().size() == 3);

  delete universe;
}
