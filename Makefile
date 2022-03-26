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
OBJS:=	global.o version.o stats.o net.o commands.o mind.o player.o npc.o object.o \
        object_acid.o object_dynamic.o command_ccreate.o command_wload.o utils.o \
	object_tba.o skills.o properties.o infile.o outfile.o log.o tags.o
LIBS:=
COPT:=	-std=c++2b -mbranches-within-32B-boundaries -ferror-limit=2 -stdlib=libc++
GOPT:=	-std=c++2b
ARCH:=	-mavx2 -mfma -mbmi2 -falign-functions=32
COMP:=	-Wall -Wshadow -Werror -Wno-format-security -fno-rtti -fno-exceptions -isystem ../range-v3/include

all:	acidmud test

#Production Settings (dynamic)
CXX=clang++-13
CXXFLAGS=-g3 -O3 $(COMP) $(ARCH) $(COPT) -flto

#Use debugging settings
debug:
	+make CXXFLAGS='-O0 -fno-omit-frame-pointer -fno-optimize-sibling-calls -g3 -fsanitize=address -fsanitize-address-use-after-scope -fsanitize=undefined -fno-sanitize-recover=undefined $(COMP) $(ARCH) $(COPT)'

#Use profiling settings
profile:
	+make CXXFLAGS='-O3 -fno-omit-frame-pointer -fno-optimize-sibling-calls -g3 $(COMP) $(ARCH) $(COPT)'

gcc:
	+make CXX='g++-11' CXXFLAGS='-Og -fno-omit-frame-pointer -fno-optimize-sibling-calls -g3 -fsanitize=address -fsanitize-address-use-after-scope -fsanitize=undefined -fno-sanitize-recover=undefined $(COMP) $(ARCH) $(GOPT)'

clean:
	rm -f gmon.out deps.mk tests/*.o tests/*.da *.o *.da version.cpp acidmud changes.txt

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

acidmud: main.o $(OBJS)
	rm -f acidmud
	$(CXX) $(CXXFLAGS) -o $@ $< $(OBJS) $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

test:	tests/test_object
	tests/test_object

tests/test_object: tests/test_object.o $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $< $(OBJS) $(LIBS)

tests/%.o: tests/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

version.cpp:	version.cpp.template *.hpp [a-uw-z]*.cpp
	cat version.cpp.template \
		| sed 's|DATE_STAMP|$(TSTR)|g' \
		| sed 's|GIT_REVS|$(RSTR)|g' \
		| sed 's|GIT_HASH|$(HSTR)|g' \
		> version.cpp

include deps.mk

deps.mk:	*.cpp *.hpp tests/*.cpp
	$(CXX) $(CXXFLAGS) -MM *.cpp tests/*.cpp > deps.mk
