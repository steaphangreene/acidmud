ACIDHOST:=acidmud

TSTR:=  $(shell date -u +"%Y%m%d%H%M")
SSTR:=  $(shell git log --oneline | wc -l)
OBJS:=	main.o version.o stats.o net.o commands.o mind.o player.o mob.o \
	object.o object_acid.o object_dynamic.o command_ccreate.o utils.o \
	object_tba.o skill_names.o
LIBS:=	-lstdc++ -lcrypt

all:	acidmud

#Production Settings (dynamic)
CXX=clang++-8
CXXFLAGS=-std=c++17 -O3 -g -Wall -Wshadow -Werror -ferror-limit=2

#Use debugging settings
debug:
	+make CXXFLAGS='-std=c++17 -Og -fno-omit-frame-pointer -g -fsanitize=address -fsanitize-address-use-after-scope -fsanitize=undefined -fno-sanitize-recover=undefined -Wall -Wshadow -Werror -ferror-limit=2'

#Use profiling settings
profile:
	+make CXXFLAGS='-std=c++17 -O3 -fno-omit-frame-pointer -g -Wall -Wshadow -Werror -ferror-limit=2'

gcc:
	+make CXX='g++' CXXFLAGS='-std=c++17 -Og -fno-omit-frame-pointer -g -Wall -Wshadow -Werror -fmax-errors=2'

clean:
	rm -f gmon.out deps.mk *.o *.da acidmud ChangeLog ChangeLog.bak

backup:
	cd ..;tar chvf ~/c/archive/acidmud.$(TSTR).tar \
		acidmud/*.[hc] acidmud/*.cpp acidmud/TODO\
		acidmud/Makefile acidmud/startup.conf
	gzip -9 ~/c/archive/acidmud.$(TSTR).tar

upload:
#	./scripts/upload_binary.csh
	scp TODO acidmud@$(ACIDHOST):~acidmud/public_html/todo.txt
	./scripts/svn2cl.sh > ChangeLog
	scp ChangeLog acidmud@$(ACIDHOST):~acidmud/public_html/changes.txt

acidmud: $(OBJS)
	rm -f acidmud
	$(CXX) $(CXXFLAGS) -o acidmud $(OBJS) $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

version.cpp:	version.cpp.template *.hpp [a-uw-z]*.cpp
	cat version.cpp.template \
		| sed s-DATE_STAMP-$(TSTR)-g \
		| sed s-SVN_STAMP-$(SSTR)-g \
		> version.cpp

include deps.mk

deps.mk:	*.cpp *.hpp
	$(CXX) $(CXXFLAGS) -MM *.cpp > deps.mk
