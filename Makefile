ACIDHOST:=acidmud

TSTR:=  $(shell date -u +"%Y%m%d%H%M")
SSTR:=  $(shell git log --oneline | wc -l)
OBJS:=	main.o version.o stats.o net.o commands.o mind.o player.o mob.o \
	object.o object_acid.o object_dynamic.o command_ccreate.o utils.o \
	object_tba.o
LIBS:=	-lstdc++ -lcrypt

all:	acidmud

#Production Settings (dynamic)
CXX=	g++ -std=c++14 -s -Wall -Werror -O3

#Use debugging settings
debug:
	+make CXX='g++ -std=c++14 -g -Wall -Werror'

#Use profiling settings
profile:
	+make CXX='g++ -std=c++14 -g -pg -fprofile-arcs -Wall -Werror'

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
	$(CXX) -Wall -Werror -o acidmud $(OBJS) $(LIBS)

%.o: %.cpp
	$(CXX) -c $<

version.cpp:	version.cpp.template *.h [a-uw-z]*.cpp
	cat version.cpp.template \
		| sed s-DATE_STAMP-$(TSTR)-g \
		| sed s-SVN_STAMP-$(SSTR)-g \
		> version.cpp

include deps.mk

deps.mk:	*.cpp *.h
	$(CXX) -MM *.cpp > deps.mk
