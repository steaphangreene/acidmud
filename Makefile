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

ACIDHOST:=acidmud

TSTR:=  $(shell date -u +"%Y%m%d%H%M")
RSTR:=  $(shell git log --oneline | wc -l)
HSTR:=  $(shell git log -1 --format=%h)
OBJS:=	main.o version.o stats.o net.o commands.o mind.o player.o mob.o object.o \
        object_acid.o object_dynamic.o command_ccreate.o command_wcreate.o utils.o \
	object_tba.o skills.o properties.o
LIBS:=	-lcrypt

all:	acidmud

#Production Settings (dynamic)
CXX=clang++-12
CXXFLAGS=-std=c++20 -O3 -g -Wall -Wshadow -Werror -ferror-limit=2

#Use debugging settings
debug:
	+make CXXFLAGS='-std=c++20 -Og -fno-omit-frame-pointer -g -fsanitize=address -fsanitize-address-use-after-scope -fsanitize=undefined -fno-sanitize-recover=undefined -Wall -Wshadow -Werror -ferror-limit=2'

#Use profiling settings
profile:
	+make CXXFLAGS='-std=c++20 -O3 -fno-omit-frame-pointer -g -Wall -Wshadow -Werror -ferror-limit=2'

gcc:
	+make CXX='g++' CXXFLAGS='-std=c++20 -Og -fno-omit-frame-pointer -g -Wall -Wshadow -Werror -fmax-errors=2'

clean:
	rm -f gmon.out deps.mk *.o *.da version.cpp acidmud changes.txt

backup:
	cd ..;tar chvf ~/c/archive/acidmud.$(TSTR).tar \
		acidmud/*.[hc] acidmud/*.cpp acidmud/TODO\
		acidmud/Makefile acidmud/acid/startup.conf
	gzip -9 ~/c/archive/acidmud.$(TSTR).tar

upload:
	./scripts/upload_binary.sh
	scp TODO acidmud@$(ACIDHOST):~acidmud/public_html/todo.txt
	git log --no-decorate > changes.txt
	scp changes.txt acidmud@$(ACIDHOST):~acidmud/public_html/
	rm -f changes.txt

acidmud: $(OBJS)
	rm -f acidmud
	$(CXX) $(CXXFLAGS) -o acidmud $(OBJS) $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

version.cpp:	version.cpp.template *.hpp [a-uw-z]*.cpp
	cat version.cpp.template \
		| sed 's|DATE_STAMP|$(TSTR)|g' \
		| sed 's|GIT_REVS|$(RSTR)|g' \
		| sed 's|GIT_HASH|$(HSTR)|g' \
		> version.cpp

include deps.mk

deps.mk:	*.cpp *.hpp
	$(CXX) $(CXXFLAGS) -MM *.cpp > deps.mk
