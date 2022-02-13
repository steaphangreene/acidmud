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

#ifndef STATS_HPP
#define STATS_HPP

#include <string>

std::string SkillName(uint32_t);

void init_skill_list();
void save_skill_names_to(FILE* fl);
void load_skill_names_from(FILE* fl);
void confirm_skill_hash(uint32_t);
void insert_skill_hash(uint32_t, const std::string&);

#endif // STATS_HPP
