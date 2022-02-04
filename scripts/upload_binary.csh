#!/bin/csh -f

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

set acidhost="acidmud.dyndns.org"

if("`file acidmud | grep 'ELF 32-bit LSB executable, Intel 80386'`" == "") then
  echo ERROR: acidmud is the wrong kind of executable for host.
  exit
endif

ssh acidmud@${acidhost} rm -f ~acidmud/acidmud
scp acidmud acidmud@${acidhost}:~acidmud/acidmud
