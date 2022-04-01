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
OBJS:=	global.o version.o stats.o net.o player.o npc.o tags.o \
	mind.o mind_craft.o mind_tba.o \
	object.o object_acid.o object_dynamic.o object_tba.o \
	commands.o command_shops.o command_ccreate.o command_wload.o \
	skills.o properties.o infile.o outfile.o log.o utils.o
TOBJS:=	tests/test_darr.o tests/test_object.o tests/test_utils.o
LIBS:=
COPT:=	-std=c++2b -mbranches-within-32B-boundaries -ferror-limit=2 -stdlib=libc++
GOPT:=	-std=c++2b
ARCH:=	-mavx2 -mfma -mbmi2 -falign-functions=32
COMP:=	-Wall -Wshadow -Werror -Wno-format-security -fno-rtti -fno-exceptions -isystem include

all:	acidmud test

#Production Settings (dynamic)
CXX=clang++-13
CXXFLAGS=-g3 -O3 $(COMP) $(ARCH) $(COPT) -flto

#Use debugging settings
debug: CXXFLAGS=-O0 -fno-omit-frame-pointer -fno-optimize-sibling-calls -g3 -fsanitize=address -fsanitize-address-use-after-scope -fsanitize=undefined -fno-sanitize-recover=undefined $(COMP) $(ARCH) $(COPT)
debug: all

#Use profiling settings
profile: CXXFLAGS=-O3 -fno-omit-frame-pointer -fno-optimize-sibling-calls -g3 $(COMP) $(ARCH) $(COPT)
profile: all

#Use code coverage test-run settings
coverage: CXXFLAGS=-O3 -fprofile-instr-generate -fcoverage-mapping -g3 $(COMP) $(ARCH) $(COPT)
coverage: tests/tests
	LLVM_PROFILE_FILE=acidmud.profraw tests/tests
	llvm-profdata-13 merge -sparse acidmud.profraw -o acidmud.profdata

gcc: CXX=g++-11
gcc: CXXFLAGS=-Og -fno-omit-frame-pointer -fno-optimize-sibling-calls -g3 -fsanitize=address -fsanitize-address-use-after-scope -fsanitize=undefined -fno-sanitize-recover=undefined $(COMP) $(ARCH) $(GOPT)
gcc: all

clean:
	rm -f gmon.out *.profraw *.profdata deps.mk tests/*.o tests/*.da *.o *.da version.cpp acidmud tests/tests changes.txt

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

test:	tests/tests
	tests/tests

tests/tests: tests/test_main.o $(TOBJS) $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $< $(TOBJS) $(OBJS) $(LIBS)

tests/%.o: tests/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

version.cpp:	version.cpp.template *.hpp [a-uw-z]*.cpp .git/logs/HEAD
	cat version.cpp.template \
		| sed 's|DATE_STAMP|$(TSTR)|g' \
		| sed 's|GIT_REVS|$(RSTR)|g' \
		| sed 's|GIT_HASH|$(HSTR)|g' \
		> version.cpp

include deps.mk

deps.mk:	*.cpp *.hpp tests/*.cpp
	$(CXX) $(CXXFLAGS) -MM *.cpp tests/*.cpp > deps.mk
	sed -i 's|^test_|tests/test_|' deps.mk
