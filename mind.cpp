#include <map>
#include <string>

using namespace std;

#include <arpa/telnet.h>

#include <cstdio>
#include <fcntl.h>
#include <cstdlib>
#include <cstdarg>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "net.h"
#include "mind.h"
#include "object.h"
#include "player.h"

Mind::Mind() {
  body = NULL;
  player = NULL;
  type = MIND_MORON;
  log = -1;
  pers = 0;
  }

Mind::Mind(int fd) {
  body = NULL;
  player = NULL;
  log = -1;
  SetRemote(fd);
  }

Mind::Mind(int fd, int l) {
  body = NULL;
  player = NULL;
  log = l;
  SetRemote(fd);
  }

Mind::~Mind() {
  if(type == MIND_REMOTE) close_socket(pers);
  type = MIND_MORON;
  Unattach();
  if(log >= 0) close(log);
  }

void Mind::SetRemote(int fd) {
  type = MIND_REMOTE;
  pers = fd;
  char buf[256];

  if(log >= 0) return;

  static unsigned long lognum = 0;
  struct stat st;
  sprintf(buf, "logs/%.8lX.log%c", lognum, 0);
  while(!stat(buf, &st)) {
    ++lognum;
    sprintf(buf, "logs/%.8lX.log%c", lognum, 0);
    }
  log = open(buf, O_WRONLY|O_CREAT|O_TRUNC|O_SYNC, S_IRUSR|S_IWUSR);
  }

void Mind::SetMob() {
  type = MIND_MOB;
  pers = fileno(stderr);
  }

void Mind::SetSlave(int master) {
  type = MIND_SLAVE;
  pers = master;
  }

void Mind::SetSystem() {
  type = MIND_SYSTEM;
  pers = fileno(stderr);
  }


static const char *sevs_p[]
	= {"-", "L", "L", "M", "M", "M", "S", "S", "S", "S", "D"};
static const char *sevs_s[]
	= {"-", "l", "l", "m", "m", "m", "s", "s", "s", "s", "u"};
void Mind::UpdatePrompt() {
  if(!Owner()) {
    SetPrompt(pers, "Player Name: ");
    if(pname.length() >= 1) SetPrompt(pers, "Password: ");
    }
  else if(Body()) {
    static char buf[65536];  //null-termed by sprintf below.
    sprintf(buf, "[%s][%s] %s> %c",
	sevs_p[10<?Body()->Phys()], sevs_s[10<?Body()->Stun()],
	Body()->ShortDesc(), 0);
    SetPrompt(pers, buf);
    }
  else SetPrompt(pers, "No Character> ");
  }

void Mind::Attach(Object *bod) {
  body = bod;
  if(body) body->Attach(this);
  }

void Mind::Unattach() {
  Object *bod = body;
  body = NULL;
  if(bod) bod->Unattach(this);
  }

static char buf[65536];
void Mind::Send(const char *mes, ...) {
  memset(buf, 0, 65536);
  va_list stuff;
  va_start(stuff, mes);
  vsprintf(buf, mes, stuff);
  va_end(stuff);

  if(type == MIND_REMOTE) {
    SendOut(pers, buf);
    }
  else if(type == MIND_MOB) {
//    string newmes = "";
//    if(body) newmes += body->ShortDesc();
//    newmes += ": ";
//    newmes += buf;
//
//    string::iterator chr = newmes.begin();
//    for(; chr != newmes.end(); ++chr) {
//      if((*chr) == '\n' || (*chr) == '\r') (*chr) = ' ';
//      }
//    newmes += "\n";
//
//    write(pers, newmes.c_str(), newmes.length());

    Think(); //Reactionary actions.
    }
  else if(type == MIND_SYSTEM) {
    string newmes = "";
    if(body) newmes += body->ShortDesc();
    newmes += ": ";
    newmes += buf;

    string::iterator chr = newmes.begin();
    for(; chr != newmes.end(); ++chr) {
      if((*chr) == '\n' || (*chr) == '\r') (*chr) = ' ';
      }
    newmes += "\n";

    write(pers, newmes.c_str(), newmes.length());
    }
  }

void Mind::SendRaw(const char *mes, ...) {
  memset(buf, 0, 65536);
  va_list stuff;
  va_start(stuff, mes);
  vsprintf(buf, mes, stuff);
  va_end(stuff);

  SendOut(pers, buf);
  }

void Mind::SetPName(string pn) {
  pname = pn;
  if(player_exists(pname))
    Send("%c%c%cReturning player - welcome back!\n", IAC, WILL, TELOPT_ECHO);
  else
    Send("%c%c%cNew player (%s) - enter SAME new password twice.\n",
	IAC, WILL, TELOPT_ECHO, pname.c_str());
  }

void Mind::SetPPass(string ppass) {
  if(player_exists(pname)) {
    player = player_login(pname, ppass);
    if(player == NULL) {
      if(player_exists(pname))
	Send("%c%c%cName and/or password is incorrect.\n",
		IAC, WONT, TELOPT_ECHO);
      else
	Send("%c%c%cPasswords do not match - try again.\n",
		IAC, WONT, TELOPT_ECHO);
      pname = "";
      return;
      }
    }
  else if(!player) {
    new Player(pname, ppass);
    Send("%c%c%cEnter password again for verification.\n", IAC, WILL, TELOPT_ECHO);
    return;
    }

  SendRaw("%c%c%c", IAC, WONT, TELOPT_ECHO);
  player->Room()->SendDesc(this);
  player->Room()->SendContents(this);
  }

void Mind::SetPlayer(string pn) {
  if(player_exists(pn)) {
    pname = pn;
    player = get_player(pname);
    }
  }

void Mind::Think(int istick) {
  if(type == MIND_MOB) {
    //AGGRESSIVE and WIMPY Circle Mobs
    if(body && body->Parent() && (body->Skill("CircleAction") & 160) == 160
	&& (!body->IsAct(ACT_FIGHT))) {
      typeof(body->Parent()->Contents()) others = body->Parent()->Contents();
      typeof(others.begin()) other;
      for(other = others.begin(); other != others.end(); ++other) {
	if((!(*other)->Skill("CircleAction")) //FIXME: Other mobs?
		&& (!body->StillBusy())			//I'm not busy
		&& body->Stun() < 6			//I'm not stunned
		&& body->Phys() < 6			//I'm not injured
		&& (!body->IsAct(ACT_SLEEP))		//I'm not asleep
		&& (!body->IsAct(ACT_REST))		//I'm not resting
		&& (*other)->IsAct(ACT_SLEEP)		//It's not awake (wuss!)
		&& (*other)->Attribute(1)		//It's not a rock
		&& (!(*other)->IsAct(ACT_UNCONSCIOUS))	//It's not already KOed
		&& (!(*other)->IsAct(ACT_DYING))	//It's not already dying
		&& (!(*other)->IsAct(ACT_DEAD))		//It's not already dead
	        ) {
	  string command = string("attack ") + (*other)->ShortDesc();
	  body->BusyFor(500, command.c_str());
	  //fprintf(stderr, "%s: Tried '%s'\n", body->ShortDesc(), command.c_str());
	  return;
	  }
	}
      }
    //AGGRESSIVE and (!WIMPY) Circle Mobs
    else if(body && body->Parent() && (body->Skill("CircleAction") & 160) == 32
	&& (!body->IsAct(ACT_FIGHT))) {
      typeof(body->Parent()->Contents()) others = body->Parent()->Contents();
      typeof(others.begin()) other;
      for(other = others.begin(); other != others.end(); ++other) {
	if((!(*other)->Skill("CircleAction")) //FIXME: Other mobs?
		&& (!body->StillBusy())			//I'm not busy
		&& body->Stun() < 6			//I'm not stunned
		&& body->Phys() < 6			//I'm not injured
		&& (!body->IsAct(ACT_SLEEP))		//I'm not asleep
		&& (!body->IsAct(ACT_REST))		//I'm not resting
		&& (*other)->Attribute(1)		//It's not a rock
		&& (!(*other)->IsAct(ACT_UNCONSCIOUS))	//It's not already KOed
		&& (!(*other)->IsAct(ACT_DYING))	//It's not already dying
		&& (!(*other)->IsAct(ACT_DEAD))		//It's not already dead
	        ) {
	  string command = string("attack ") + (*other)->ShortDesc();
	  body->BusyFor(500, command.c_str());
	  //fprintf(stderr, "%s: Tried '%s'\n", body->ShortDesc(), command.c_str());
	  return;
	  }
	}
      }
    //HELPER Circle Mobs
    if(body && body->Parent() && (body->Skill("CircleAction") & 4096)
	&& (!body->IsAct(ACT_FIGHT))) {
      typeof(body->Parent()->Contents()) others = body->Parent()->Contents();
      typeof(others.begin()) other;
      for(other = others.begin(); other != others.end(); ++other) {
	if((!(*other)->Skill("CircleAction")) //FIXME: Other mobs?
		&& (!body->StillBusy())			//I'm not busy
		&& body->Stun() < 6			//I'm not stunned
		&& body->Phys() < 6			//I'm not injured
		&& (!body->IsAct(ACT_SLEEP))		//I'm not asleep
		&& (!body->IsAct(ACT_REST))		//I'm not resting
		&& (*other)->Attribute(1)		//It's not a rock
		&& (!(*other)->IsAct(ACT_DEAD))		//It's not already dead
		&& (*other)->IsAct(ACT_FIGHT)		//It's figting someone
		&& (*other)->ActTarg(ACT_FIGHT)->Skill("CircleAction")
							//...against another MOB
	        ) {
	  string command = string("attack ") + (*other)->ShortDesc();
	  body->BusyFor(500, command.c_str());
	  //fprintf(stderr, "%s: Tried '%s'\n", body->ShortDesc(), command.c_str());
	  return;
	  }
	}
      }
    //NON-SENTINEL Circle Mobs
    if(body && body->Parent() && (body->Skill("CircleAction") & 2) == 0
	&& (!body->IsAct(ACT_FIGHT)) && istick && (!body->StillBusy())
	&& (!body->IsAct(ACT_REST)) && (!body->IsAct(ACT_SLEEP))
	&& body->Stun() < 6 && body->Phys() < 6) {
      map<string,Object*> cons = body->Parent()->Connections();

      map<string,Object*> cons2 = cons;
      map<string,Object*>::iterator dir = cons2.begin();
      for(; dir != cons2.end(); ++dir) {
	if(dir->second->Skill("CircleZone") == 999999) { //NO_MOBS Circle Zone
	  cons.erase(dir->first); //Don't Enter NO_MOBS Zone!
	  }
	else if(body->Skill("CircleAction") & 64) {	//STAY_ZONE Circle MOBs
	  if(dir->second->Skill("CircleZone")
			!= body->Parent()->Skill("CircleZone")) {
	    cons.erase(dir->first);			//Don't Leave Zone!
	    }
	  }
	else if(body->Skill("Swimming") == 0) {		//Can't Swim?
	  if(dir->second->Skill("WaterDepth") == 1) {
	    cons.erase(dir->first);			//Can't swim!
	    }
	  }
	else if(!(body->Skill("CircleAffection")&64)) {	//Can't Waterwalk?
	  if(dir->second->Skill("WaterDepth") >= 1) {	//Need boat!
	    cons.erase(dir->first);			//FIXME: Have boat?
	    }
	  }
	}

      if(cons.size() && body->Roll("Willpower", 9)) {
	int res = rand() % cons.size();
	map<string,Object*>::iterator dir = cons.begin();
	while(res > 0) { ++dir; --res; }
	body->BusyFor(500, dir->first.c_str());
	return;
	}
      }
    }
  }
