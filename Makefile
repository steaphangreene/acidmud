TSTR:=  $(shell date -u +"%Y%m%d%H%M")
OBJS:=	main.o net.o commands.o mind.o player.o \
	object.o object_circle.o object_acid.o object_stats.o

#Production Settings
CCC:=	gcc -s -Wall -O3
LIBS:=	-lstdc++ -lcrypt

#Debugging settings
#CCC:=	gcc -g -Wall
#LIBS:=	-lstdc++ -lcrypt -lefence

#Profiling settings
#CCC:=	gcc -g -pg -fprofile-arcs -Wall
#LIBS:=	-lstdc++ -lcrypt -lefence

all: acidmud

clean:
	rm -f gmon.out deps.mk *.o *.da acidmud

backup:
	cd ..;tar chvf ~/c/archive/acidmud.$(TSTR).tar \
		acidmud/*.[hc] acidmud/*.cpp acidmud/TODO\
		acidmud/Makefile acidmud/startup.conf
	gzip -9 ~/c/archive/acidmud.$(TSTR).tar

upload:
	ssh acidmud@reactor rm -f ~acidmud/acidmud
	scp acidmud acidmud@reactor:~acidmud/acidmud
	scp TODO acidmud@reactor:~acidmud/public_html/todo.txt
	cvs2cl.pl --no-wrap --stdout | tr '\t' '@' | sed 's-^@\([A-Z]\)-@* \1-g' | tr '@' '\t' > ChangeLog
	scp ChangeLog acidmud@reactor:~acidmud/public_html/changes.txt

acidmud: $(OBJS)
	$(CCC) -Wall -o acidmud $(OBJS) $(LIBS)

%.o: %.cpp
	$(CCC) -c $<

include deps.mk

deps.mk:	*.cpp *.h
	$(CCC) -MM *.cpp > deps.mk
