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

rm -f skills.txt
wget http://www.cs.binghamton.edu/~stea/gaming/data/skills.txt

echo 'struct skill_entry {'
echo '  const char *skcatname;'
echo '  const char *skname;'
echo '  int linked_attr;'
echo '  };'

echo ''

echo '#define CUR_CAT "None"'
echo 'static skill_entry skill_data[] = {'
cat skills.txt | tr '\t' '@' | uniq \
	| sed 's-^S@[A-Za-z0-9]*@B@\(.*\)-  { CUR_CAT, "\1", 0 },-g' \
	| sed 's-^S@[A-Za-z0-9]*@Q@\(.*\)-  { CUR_CAT, "\1", 1 },-g' \
	| sed 's-^S@[A-Za-z0-9]*@S@\(.*\)-  { CUR_CAT, "\1", 2 },-g' \
	| sed 's-^S@[A-Za-z0-9]*@C@\(.*\)-  { CUR_CAT, "\1", 3 },-g' \
	| sed 's-^S@[A-Za-z0-9]*@I@\(.*\)-  { CUR_CAT, "\1", 4 },-g' \
	| sed 's-^S@[A-Za-z0-9]*@W@\(.*\)-  { CUR_CAT, "\1", 5 },-g' \
	| sed 's-^C.*@\(.*\)$-  #undef CUR_CAT@  #define CUR_CAT "\1"-' \
	| grep '^  ' | tr '@' '\n'
echo '  { NULL, NULL, -1 }'
echo '  };'

rm -f skills.txt
