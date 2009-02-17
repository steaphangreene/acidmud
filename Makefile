ACIDHOST:=acidmud.dyndns.org

TSTR:=  $(shell date -u +"%Y%m%d%H%M")
OBJS:=	main.o version.o net.o commands.o mind.o player.o mob.o atomstring.o \
	object.o object_circle.o object_acid.o object_stats.o object_dynamic.o \
	command_ccreate.o

#Production Settings (dynamic)
#CCC:=	gcc$(ACIDMUD_CTAIL) -s -Wall -O3
#LIBS:=	-static -lstdc++ -lcrypt

#Production Settings (dynamic, except libstdc++)
#CCC:=	gcc$(ACIDMUD_CTAIL) -s -Wall -O3
#LIBS:=	-static `gcc$(ACIDMUD_CTAIL) -print-file-name=libstdc++.a` -lcrypt

#Production Settings (static)
CCC:=	gcc$(ACIDMUD_CTAIL) -s -Wall -O3
LIBS:=	-static -lstdc++ -lcrypt

#Debugging settings
#CCC:=	gcc$(ACIDMUD_CTAIL) -g -Wall
#LIBS:=	-lstdc++ -lcrypt -lefence

#Profiling settings
#CCC:=	gcc$(ACIDMUD_CTAIL) -g -pg -fprofile-arcs -Wall
#LIBS:=	-lstdc++ -lcrypt -lefence

all:	acidmud

clean:
	rm -f gmon.out deps.mk *.o *.da acidmud ChangeLog ChangeLog.bak

backup:
	cd ..;tar chvf ~/c/archive/acidmud.$(TSTR).tar \
		acidmud/*.[hc] acidmud/*.cpp acidmud/TODO\
		acidmud/Makefile acidmud/startup.conf
	gzip -9 ~/c/archive/acidmud.$(TSTR).tar

upload:
	./scripts/upload_binary.csh
	scp TODO acidmud@$(ACIDHOST):~acidmud/public_html/todo.txt
	./scripts/svn2cl.sh > ChangeLog
	scp ChangeLog acidmud@$(ACIDHOST):~acidmud/public_html/changes.txt

acidmud: $(OBJS)
	$(CCC) -Wall -o acidmud $(OBJS) $(LIBS)

%.o: %.cpp
	$(CCC) -c $<

version.cpp:	version.cpp.template *.h [a-uw-z]*.cpp
	cat version.cpp.template | sed s-DATE_STAMP-$(TSTR)-g > version.cpp

include deps.mk

deps.mk:	*.cpp *.h
	$(CCC) -MM *.cpp > deps.mk
