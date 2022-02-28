#!/bin/bash

# *************************************************************************
#  This file is part of AcidMUD by Steaphan Greene
#
#  Copyright 1999-2022 Steaphan Greene <steaphan@gmail.com>
#
#  AcidMUD is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 3 of the License, or
#  (at your option) any later version.
#
#  AcidMUD is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with AcidMUD (see the file named "COPYING");
#  If not, see <http://www.gnu.org/licenses/>.
#
# *************************************************************************

# The bulk of the conversions
sed -i 's|{"|{u8"|g' *[a-z].[ch]pp*
sed -i 's|("|(u8"|g' *[a-z].[ch]pp*
sed -i 's|\["|[u8"|g' *[a-z].[ch]pp*
sed -i 's| "| u8"|g' *[a-z].[ch]pp*
sed -i 's|char const\*|const char8_t*|g' *[a-z].[ch]pp*
sed -i 's|const char\*|const char8_t*|g' *[a-z].[ch]pp*
sed -i 's| char(|static_cast<char8_t>(|g' *[a-z].[ch]pp*
sed -i 's|std::string|std::u8string|g' *[a-z].[ch]pp*
sed -i 's| char | char8_t |g' *[a-z].[ch]pp*
sed -i 's| __attribute__((format(printf, [0-9], [0-9])));|;|g' *[a-z].[ch]pp*
sed -i 's| new char\[| new char8_t[|g' *[a-z].[ch]pp*
sed -i 's| char\*| char8_t*|g' *[a-z].[ch]pp*
sed -i 's|\.path()|.path().u8string()|g' *[a-z].[ch]pp*

# Undo missed with u8 inside the string
sed -i 's|u8"\([^"]*\)u8"|u8"\1"|g' *[a-z].[ch]pp* *.cpp.template

# Undo changes around C main() argv
sed -i 's|char8_t\*\* argv|char** argv|g' *[a-z].[ch]pp*
sed -i 's|char8_t\*\* new_argv|char** new_argv|g' *[a-z].[ch]pp*
sed -i 's|new_argv = new char8_t|new_argv = new char|g' *[a-z].[ch]pp*
sed -i 's|strdup(u8"|strdup("|g' *[a-z].[ch]pp*
sed -i 's|strdup(buf)|strdup(reinterpret_cast<const char *>(buf))|g' *[a-z].[ch]pp*

# Undo changes around C getopt_long
sed -i 's| getopt_long(argc, argv, u8"| getopt_long(argc, argv, "|g' *[a-z].[ch]pp*
sed -i 's| = optarg| = reinterpret_cast<const char8_t*>(optarg)|g' *[a-z].[ch]pp*
sed -i 's|{u8"port"|{"port"|g' *[a-z].[ch]pp*
sed -i 's|{u8"host"|{"host"|g' *[a-z].[ch]pp*
sed -i 's|{u8"network-|{"network-|g' *[a-z].[ch]pp*

# Undo the non-code changes
sed -i 's|^#include u8"|#include "|g' *[a-z].[ch]pp*
sed -i 's|(see the file named u8"COPYING")|(see the file named "COPYING")|g' *[a-z].[ch]pp*

# Replace old C char header includes with cchar8.hpp.
sed -i 's|^#include <cstdio>|#include "cchar8.hpp"|g' *[a-z].[ch]pp*
sed -i 's|^#include <cstring>|#include "cchar8.hpp"|g' *[a-z].[ch]pp*
sed -i 's|^#include <cstdlib>|#include "cchar8.hpp"|g' *[a-z].[ch]pp*
