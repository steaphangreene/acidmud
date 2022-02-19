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

#ifndef VERSION_HPP
#define VERSION_HPP

#include <string>

struct VersionInformation {
  unsigned int savefile_version_net;
  unsigned int savefile_version_player;
  unsigned int savefile_version_object;
  unsigned int acidmud_version[3];
  unsigned int acidmud_git_revs;
  std::string acidmud_git_hash;
  std::string acidmud_datestamp;
};

#ifndef VERSION_DEFINED
#define VERSION_DEFINED

extern VersionInformation CurrentVersion;

#endif // VERSION_DEFINED

#endif // VERSION_HPP
