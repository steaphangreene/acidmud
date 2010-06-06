ACIDHOST:=acidmud.dyndns.org

TSTR:=  $(shell date -u +"%Y%m%d%H%M")
OBJS:=	main.o version.o net.o commands.o mind.o player.o mob.o atomstring.o \
	object.o object_circle.o object_acid.o stats.o object_dynamic.o \
	command_ccreate.o

#Production Settings (dynamic)
CXX=	gcc$(ACIDMUD_CTAIL) -s -Wall -O3
LIBS:=	-static -lstdc++ -lcrypt

#Production Settings (dynamic, except libstdc++)
#CXX=	gcc$(ACIDMUD_CTAIL) -s -Wall -O3
#LIBS:=	-static `gcc$(ACIDMUD_CTAIL) -print-file-name=libstdc++.a` -lcrypt

#Production Settings (static)
#CXX=	gcc$(ACIDMUD_CTAIL) -s -Wall -O3
#LIBS:=	-static -lstdc++ -lcrypt

#Debugging settings
#CXX=	gcc$(ACIDMUD_CTAIL) -g -Wall
#LIBS:=	-lstdc++ -lcrypt

#Profiling settings
#CXX=	gcc$(ACIDMUD_CTAIL) -g -pg -fprofile-arcs -Wall
#LIBS:=	-lstdc++ -lcrypt

all:	acidmud

clean:
	rm -f gmon.out deps.mk *.o *.da acidmud ChangeLog ChangeLog.bak

backup:
	cd ..;tar chvf ~/c/archive/acidmud.$(TSTR).tar \
		acidmud/*.[hc] acidmud/*.cpp acidmud/TODO\
		acidmud/Makefile acidmud/startup.conf
	gzip -9 ~/c/archive/acidmud.$(TSTR).tar

upload:
#	./scripts/upload_binary.csh
#	scp TODO acidmud@$(ACIDHOST):~acidmud/public_html/todo.txt
	./scripts/svn2cl.sh > ChangeLog
	scp ChangeLog acidmud@$(ACIDHOST):~acidmud/public_html/changes.txt
	svn log -rHEAD:1 --xml > /tmp/svn.log.xml
	./scripts/svnlogatom.py file:///tmp/svn.log.xml "AcidMUD Feed" \
		> /tmp/rss.xml
	scp /tmp/rss.xml acidmud@$(ACIDHOST):~acidmud/public_html/rss.xml
	rm -f /tmp/rss.xml

acidmud: $(OBJS)
	rm -f acidmud
	$(CXX) -Wall -o acidmud $(OBJS) $(LIBS)

%.o: %.cpp
	$(CXX) -c $<

version.cpp:	version.cpp.template *.h [a-uw-z]*.cpp
	cat version.cpp.template | sed s-DATE_STAMP-$(TSTR)-g > version.cpp

include deps.mk

deps.mk:	*.cpp *.h
	$(CXX) -MM *.cpp > deps.mk
