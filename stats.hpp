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

#include "outfile.hpp"

std::u8string SkillName(uint32_t);

void init_skill_list();
void save_prop_names_to(const outfile& fl);
void load_prop_names_from(FILE* fl);
void purge_invalid_prop_names();
void confirm_skill_hash(uint32_t);
void insert_skill_hash(uint32_t, const std::u8string_view&);

#endif // STATS_HPP
