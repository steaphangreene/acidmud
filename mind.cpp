#include <map>
#include <string>

using namespace std;

#include <arpa/telnet.h>

#include <cstdio>
#include <fcntl.h>
#include <cstdlib>
#include <cstdarg>
#include <unistd.h>

#include "net.h"
#include "mind.h"
#include "object.h"
#include "player.h"

Mind::Mind() {
  body = NULL;
  player = NULL;
  type = MIND_MORON;
  pers = 0;
  }

Mind::Mind(int fd) {
  body = NULL;
  player = NULL;
  SetRemote(fd);
  }

Mind::~Mind() {
  if(type == MIND_REMOTE) close_socket(pers);
  type = MIND_MORON;
  Unattach();
  }

void Mind::SetRemote(int fd) {
  type = MIND_REMOTE;
  pers = fd;
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


static const char *sevs[]
	= {"-", "L", "L", "M", "M", "M", "S", "S", "S", "S", "D"};
void Mind::UpdatePrompt() {
  if(!Owner()) {
    SetPrompt(pers, "Player Name: ");
    if(pname.length() >= 1) SetPrompt(pers, "Password: ");
    }
  else if(Body()) {
    static char buf[65536];  //null-termed by sprintf below.
    sprintf(buf, "[%s][%s] %s> %c", sevs[10<?Body()->Stats()->phys],
	sevs[10<?Body()->Stats()->stun], Body()->ShortDesc(), 0);
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

    //AGGRESSIVE Circle Mobs
    if(body && body->Parent() && (body->Stats()->GetSkill("CircleAction") & 32)
	&& (!body->IsAct(ACT_FIGHT))) {
      set<Object*> others = body->Parent()->Contents();
      set<Object*>::iterator other;
      for(other = others.begin(); other != others.end(); ++other) {
	if((!(*other)->Stats()->GetSkill("CircleAction")) //FIXME: Other mobs?
		&& (!body->StillBusy())                   //I'm not busy.
		&& body->Stats()->stun < 6                //I'm not stunned.
		&& body->Stats()->phys < 6                //I'm not injured.
		&& (!body->IsAct(ACT_ASLEEP))             //I'm not asleep.
		&& (!body->IsAct(ACT_REST))               //I'm not resting.
		&& (*other)->Stats()->GetAttribute(1)     //Not a rock
		&& (!(*other)->IsAct(ACT_DEAD))           //Not a dead
	        ) {
	  string command = string("attack ") + (*other)->ShortDesc();
	  body->BusyFor(500, command.c_str());
	  fprintf(stderr, "%s: Tried '%s'\n", body->ShortDesc(), command.c_str());
	  }
	}
      }

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
  }

void Mind::SetPlayer(string pn) {
  if(player_exists(pn)) {
    pname = pn;
    player = get_player(pname);
    }
  }
