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

tmpdir=$(mktemp -d /tmp/skills-XXXXXX)
trap "rm -fr $tmpdir" EXIT

wget 'https://biaszero.com/~stea/gaming/wiki/index.php/SkillsData' -o /dev/null -O - \
	| csplit -q -n 1 -f ${tmpdir}/skills - %START% /END/
tail -n +2 ${tmpdir}/skills0 | cut -f3- -d'>' | grep '^S;' > ${tmpdir}/skills
rm -f ${tmpdir}/skills[01]

cat <<- 'EOF'
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

	// @generated code file: Do not edit.  Edit update_skills.sh instead.

	// clang-format off

	#include <map>
	#include <string>
	#include <vector>

	#include "properties.hpp"

	std::map<uint32_t, int32_t> defaults = {
EOF

for sk in $(cat ${tmpdir}/skills | cut -f3-4 -d";" | sed "s- -_-g" | sort -uk1.3,2); do
  skname="$(echo "$sk" | cut -f2 -d";" | sed "s-_- -g")"

  if [[ "$(echo "$sk" | cut -f1 -d";")" == "B" ]]; then
    echo "    {prhash(u8\"${skname}\"), 0},"
  elif [[ "$(echo "$sk" | cut -f1 -d";")" == "Q" ]]; then
    echo "    {prhash(u8\"${skname}\"), 1},"
  elif [[ "$(echo "$sk" | cut -f1 -d";")" == "S" ]]; then
    echo "    {prhash(u8\"${skname}\"), 2},"
  elif [[ "$(echo "$sk" | cut -f1 -d";")" == "C" ]]; then
    echo "    {prhash(u8\"${skname}\"), 3},"
  elif [[ "$(echo "$sk" | cut -f1 -d";")" == "I" ]]; then
    echo "    {prhash(u8\"${skname}\"), 4},"
  elif [[ "$(echo "$sk" | cut -f1 -d";")" == "W" ]]; then
    echo "    {prhash(u8\"${skname}\"), 5},"
  elif [[ "$(echo "$sk" | cut -f1 -d";")" == "R" ]]; then
    echo "    {prhash(u8\"${skname}\"), 6},"
  elif [[ "$(echo "$sk" | cut -f1 -d";")" == "M" ]]; then
    echo "    {prhash(u8\"${skname}\"), 7},"
  elif [[ "$(echo "$sk" | cut -f1 -d";")" == "F" ]]; then
    echo "    {prhash(u8\"${skname}\"), 7},"
  elif [[ "$(echo "$sk" | cut -f1 -d";")" == "J" ]]; then
    echo "    {prhash(u8\"${skname}\"), 7},"
  elif [[ "$(echo "$sk" | cut -f1 -d";")" == "T" ]]; then
    echo "    {prhash(u8\"${skname}\"), 7},"
  fi
done
echo '};'

wsnum=1
for sk in $(cat ${tmpdir}/skills | cut -f3-4 -d";" | sed "s- -_-g" | sort -uk1.3,2); do
  weapon=
  skname="$(echo "$sk" | cut -f2 -d";" | sed "s-_- -g")"
  sklink="$(echo $skname | sed "s| |_|g")"
  for skcat in $(wget https://biaszero.com/~stea/gaming/wiki/index.php/Skill:${sklink} -q -O - \
        | sed 's/title=Category:\([A-Za-z0-9_/\-]*\)&/@\1@/g' \
	| tr '\n' '?' | tr @ '\n' | grep -v '\?'); do
    catname="$(echo "$skcat" | sed "s-_- -g" | sed "s-/-_-g")"
    echo "         prhash(u8\"${skname}\")," >> "${tmpdir}/skcat#$catname"
    if [ -n "$(echo "$catname" | grep -E "(Combat|Pistol|Rifle|Weapon) Skills")" ]; then
      weapon=1
    fi
  done
  if [ -n "$weapon" ]; then
    echo "    {$wsnum, prhash(u8\"${skname}\")}," >> "${tmpdir}/weapons1"
    echo "    {prhash(u8\"${skname}\"), $wsnum}," >> "${tmpdir}/weapons2"
    wsnum=$(($wsnum + 1))
  fi
done

echo ""
echo "std::map<std::u8string, std::vector<uint32_t>> skcat = {"
for cat in ${tmpdir}/skcat#*; do
    catname="$(echo "$cat" | cut -f2 -d"#" | sed "s-_-/-g")"
    echo "    {u8\"${catname}\","
    echo "     {"
    cat "$cat"
    echo "     }},"
done
cat <<- EOF
	};

	std::map<int32_t, uint32_t> weaponskills = {
EOF

cat "${tmpdir}/weapons1"

cat <<- EOF
	};

	std::map<uint32_t, int32_t> weapontypes = {
EOF

cat "${tmpdir}/weapons2"
echo '};'
echo "// clang-format on"
