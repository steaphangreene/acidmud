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

target="$1"
if [ "$target" = "" ]; then
  echo "Usage: $0 <target_filename>"
  exit
fi

tmpdir=$(mktemp -d /tmp/socials-XXXXXX)
trap "rm -fr $tmpdir" EXIT
cat ${target}.template | csplit -q -n 1 -f ${tmpdir}/socials - /COM_SOCIAL,/

cat ${tmpdir}/socials0

for com in $(grep ^~ tba/socials.new | cut -c2- | cut -f1 -d' '); do
  enum=$(echo $com | tr a-z A-Z | tr '.-' '__')
  if [ "$target" = "commands.hpp" ]; then
    echo "  COM_SOCIAL_$enum,"
  else
    echo "    {u8\"$com\","
    echo "     u8\"Social Command.\","
    echo "     u8\"Social Command.\","
    echo "     (REQ_ALERT | REQ_UP | REQ_ACTION),"
    echo "     COM_SOCIAL_$enum},"
  fi
done

tail -n +2 ${tmpdir}/socials1
