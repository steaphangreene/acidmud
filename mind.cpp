#include <map>
#include <string>

using namespace std;

#include <arpa/telnet.h>

#include <cstdio>
#include <fcntl.h>
#include <cstdlib>
#include <cstdarg>
#include <cstring>
#include <algorithm>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "utils.h"
#include "color.h"
#include "net.h"
#include "mind.h"
#include "object.h"
#include "player.h"


//FIXME: This is not remotely done!
static int tba_eval(string expr) {
  size_t end = expr.find_first_of("\n\r");
  if(end != string::npos) expr = expr.substr(0, end);
  trim_string(expr);

  if(expr[0] == '(') return tba_eval(expr.substr(1));	//FIXME: Really Do This!

  size_t op = expr.find_first_of("|&=!<>/-+*");
  if(op == string::npos) return atoi(expr.c_str());	//No ops, just val

  int comp = 0;		//Positive for 2-char ops, negative for 1-char
  if(!strncmp(expr.c_str()+op, "==", 2)) comp = 1;
  if(!strncmp(expr.c_str()+op, "!=", 2)) comp = 2;

  if(comp != 0) {	//For == and !=
    string arg1 = expr.substr(0, op);
    trim_string(arg1);
    expr = expr.substr(op+2);
    string arg2 = expr;
    op = expr.find_first_of("|&=!<>/-+*)\n\r");
    if(op != string::npos) {
      arg2 = expr.substr(0, op);
      expr = expr.substr(op);
      }
    else {
      expr = "";
      }
    trim_string(arg2);

    if(expr != "") {
      expr = "0 " + expr;
      if(comp == 1 && arg1 == arg2) expr[0] = 1;
      else if(comp == 2 && arg1 != arg2) expr[0] = 1;
      return tba_eval(expr);
      }
    else {
      if(comp == 1) return (arg1 == arg2);
      if(comp == 2) return (arg1 != arg2);
      }
    }

  return 0;
  }

#define QUOTAERROR1	"Error: script quota exceeded in #%d\n"
#define QUOTAERROR2	body->Skill("TBAScript")
#define PING_QUOTA() { --quota; \
	if(quota < 1) {	\
	fprintf(stderr, QUOTAERROR1, QUOTAERROR2); \
	return; \
	}}


Mind::Mind() {
  body = NULL;
  player = NULL;
  type = MIND_MORON;
  pers = 0;
  log = -1;
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

void Mind::SetTBATrigger(Object *tr, Object *tripper) {
  if((!tr) || (!(tr->Parent()))) return;

  type = MIND_TBATRIG;
  pers = fileno(stderr);
  Attach(tr);
  spos = 0;
  script = body->LongDesc();
  script += "\n";
  actor = tripper;

  Object *targ = tr->Parent();

  //Variable Sub (For constants on this trigger instance)
  if(tr->Skill("TBAScriptType") >= 128) {	//Room Triggers
    replace_all(script, "%self.vnum%", targ->Skill("TBARoom")-1000000);
    }
  else if(tr->Skill("TBAScriptType") >= 64) {	//Object Triggers
    replace_all(script, "%self.vnum%", targ->Skill("TBAObject")-1000000);
    }
  else {					//MOB Triggers
    replace_all(script, "%self.vnum%", targ->Skill("TBAMOB")-1000000);
    }
  if(tripper) {
    int vnum = tripper->Skill("TBAMOB");
    if(vnum < 1) vnum = tripper->Skill("TBAObject");
    if(vnum < 1) vnum = tripper->Skill("TBARoom");
    if(vnum) {
      replace_all(script, "%actor.vnum%", vnum-1000000);
      }
    }
  }

void Mind::SetTBAMob() {
  type = MIND_TBAMOB;
  pers = fileno(stderr);
  }

void Mind::SetCircleMob() {
  type = MIND_CIRCLEMOB;
  pers = fileno(stderr);
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
  static char buf[65536];
  if(!Owner()) {
    SetPrompt(pers, "Player Name: ");
    if(pname.length() >= 1) SetPrompt(pers, "Password: ");
    }
  else if(prompt.length() > 0) {
    sprintf(buf, "%s> %c", prompt.c_str(), 0);
    SetPrompt(pers, buf);
    }
  else if(Body()) {
    sprintf(buf, "[%s][%s] %s> %c",
	sevs_p[MIN(10, Body()->Phys())], sevs_s[MIN(10, Body()->Stun())],
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

void Mind::Send(const char *mes) {
  if(type == MIND_REMOTE) {
    SendOut(pers, mes);
    }
  else if(type == MIND_MOB) {
    //Think(); //Reactionary actions (NO!).
    }
  else if(type == MIND_TBAMOB) {
    //HELPER TBA Mobs
    if(body && body->Parent()
	&& (body->Skill("TBAAction") & 4096)		//Helpers
	&& ((body->Skill("TBAAction") & 2) == 0)	//NON-SENTINEL
	&& body->Stun() < 6			//I'm not stunned
	&& body->Phys() < 6			//I'm not injured
	&& (!body->IsAct(ACT_SLEEP))		//I'm not asleep
	&& (!body->IsAct(ACT_REST))		//I'm not resting
	&& (!body->IsAct(ACT_FIGHT))		//I'm not already fighting
	) {
      if((!strncasecmp(mes, "From ", 5))
		&& (strcasestr(mes, " you hear someone shout '") != NULL)
		&& ((strstr(mes, "HELP")) || (strstr(mes, "ALARM")))
		) {
	char buf[256] = "                                               ";
	sscanf(mes+4, "%128s", buf);

	Object *door = body->PickObject(buf, LOC_NEARBY);

	if(door && door->ActTarg(ACT_SPECIAL_LINKED)
                && door->ActTarg(ACT_SPECIAL_LINKED)->Parent()
		&& TBACanWanderTo(
			door->ActTarg(ACT_SPECIAL_LINKED)->Parent()
			)
		) {
	  char buf[256] = "enter                                          ";
	  sscanf(mes+4, "%128s", buf+6);
	  body->BusyFor(500, buf);
	  }
	return;
	}
      }
    Think(); //Reactionary actions (HACK!).
    }
  else if(type == MIND_CIRCLEMOB) {
    //HELPER Circle Mobs
    if(body && body->Parent()
	&& (body->Skill("CircleAction") & 4096)		//Helpers
	&& ((body->Skill("CircleAction") & 2) == 0)	//NON-SENTINEL
	&& body->Stun() < 6			//I'm not stunned
	&& body->Phys() < 6			//I'm not injured
	&& (!body->IsAct(ACT_SLEEP))		//I'm not asleep
	&& (!body->IsAct(ACT_REST))		//I'm not resting
	&& (!body->IsAct(ACT_FIGHT))		//I'm not already fighting
	) {
      if((!strncasecmp(mes, "From ", 5))
		&& (strcasestr(mes, " you hear someone shout '") != NULL)
		&& ((strstr(mes, "HELP")) || (strstr(mes, "ALARM")))
		) {
	char buf[256] = "                                               ";
	sscanf(mes+4, "%128s", buf);

	Object *door = body->PickObject(buf, LOC_NEARBY);

	if(door && door->ActTarg(ACT_SPECIAL_LINKED)
                && door->ActTarg(ACT_SPECIAL_LINKED)->Parent()
		&& CircleCanWanderTo(
			door->ActTarg(ACT_SPECIAL_LINKED)->Parent()
			)
		) {
	  char buf[256] = "enter                                          ";
	  sscanf(mes+4, "%128s", buf+6);
	  body->BusyFor(500, buf);
	  }
	return;
	}
      }
    Think(); //Reactionary actions (HACK!).
    }
  else if(type == MIND_SYSTEM) {
    string newmes = "";
    if(body) newmes += body->ShortDesc();
    newmes += ": ";
    newmes += mes;

    string::iterator chr = newmes.begin();
    for(; chr != newmes.end(); ++chr) {
      if((*chr) == '\n' || (*chr) == '\r') (*chr) = ' ';
      }
    newmes += "\n";

    write(pers, newmes.c_str(), newmes.length());
    }
  }

void Mind::SendRaw(const char *mes) {
  SendOut(pers, mes);
  }

static char buf[65536];
void Mind::SendF(const char *mes, ...) {
  memset(buf, 0, 65536);
  va_list stuff;
  va_start(stuff, mes);
  vsprintf(buf, mes, stuff);
  va_end(stuff);

  Send(buf);
  }

void Mind::SendRawF(const char *mes, ...) {
  memset(buf, 0, 65536);
  va_list stuff;
  va_start(stuff, mes);
  vsprintf(buf, mes, stuff);
  va_end(stuff);

  SendRaw(buf);
  }

void Mind::SetPName(string pn) {
  pname = pn;
  if(player_exists(pname))
    SendF("%c%c%cReturning player - welcome back!\n", IAC, WILL, TELOPT_ECHO);
  else
    SendF("%c%c%cNew player (%s) - enter SAME new password twice.\n",
	IAC, WILL, TELOPT_ECHO, pname.c_str());
  }

void Mind::SetPPass(string ppass) {
  if(player_exists(pname)) {
    player = player_login(pname, ppass);
    if(player == NULL) {
      if(player_exists(pname))
	SendF("%c%c%cName and/or password is incorrect.\n",
		IAC, WONT, TELOPT_ECHO);
      else
	SendF("%c%c%cPasswords do not match - try again.\n",
		IAC, WONT, TELOPT_ECHO);
      pname = "";
      return;
      }
    }
  else if(!player) {
    new Player(pname, ppass);
    SendF("%c%c%cEnter password again for verification.\n",
		IAC, WILL, TELOPT_ECHO);
    return;
    }

  SendRawF("%c%c%c", IAC, WONT, TELOPT_ECHO);
  player->Room()->SendDesc(this);
  player->Room()->SendContents(this);
  }

void Mind::SetPlayer(string pn) {
  if(player_exists(pn)) {
    pname = pn;
    player = get_player(pname);
    }
  }

string Mind::Tactics(int phase) {
  if(type == MIND_TBAMOB) {
    //NON-HELPER and NON-AGGRESSIVE TBA Mobs (Innocent MOBs)
    if(body && (body->Skill("TBAAction") & 4128) == 0) {
      if(phase == -1) {
	return "call HELP; attack";
	}
      }
    }
  if(type == MIND_CIRCLEMOB) {
    //NON-HELPER and NON-AGGRESSIVE Circle Mobs (Innocent MOBs)
    if(body && (body->Skill("CircleAction") & 4128) == 0) {
      if(phase == -1) {
	return "call HELP; attack";
	}
      }
    }
  return "attack";
  }

static const char *dirnames[4] = { "north", "south", "east", "west" };
static const char *items[8] = {
  "Food", "Hungry",	//Order is Most to Least Important
  "Rest", "Tired",
  "Fun", "Bored",
  "Stuff", "Needy"
  };
void Mind::Think(int istick) {
  if(type == MIND_MOB) {
    if(body->Skill("Personality") & 1) {		// Group Mind
//      body->TryCombine();	// I AM a group, after all.
      int qty = body->Skill("Quantity"), req = -1;
      if(qty < 1) qty = 1;
      for(int item=0; item < 8; item += 2) {
	if(body->Parent()->Skill(items[item])) {
	  int val = body->Skill(items[item+1]);
	  val -= body->Parent()->Skill(items[item]) * qty * 100;
	  if(val < 0) val = 0;
	  body->SetSkill(items[item+1], val);
	  }
	else {
	  if((item & 2) == 0) {		// Food & Fun grow faster
	    body->SetSkill(items[item+1], body->Skill(items[item+1]) + qty*2);
	    }
	  else {
	    body->SetSkill(items[item+1], body->Skill(items[item+1]) + qty);
	    }
	  }
	if(req < 0 && body->Skill(items[item+1]) >= 10000) {
	  req = item;
	  }
	}
      if(req >= 0) {	// Am I already getting what I most need?
	if(body->Parent()->Skill(items[req]) > 0) req = -1;
	}
      if(req >= 0) {
	vector<const char*> dirs;
	for(int i = 0; i < 4; ++i) {
	  Object *dir = body->PickObject(dirnames[i], LOC_NEARBY);
	  if(dir && dir->Skill("Open") > 0) {
	    dirs.push_back(dirnames[i]);
	    }
	  }
	random_shuffle(dirs.begin(), dirs.end());

	int orig = body->Skill(items[req+1]);
	int leave = orig/10000;
	if(dirs.size() > 0 && leave >= qty) {
	  body->BusyFor(0, dirs.front());
	  }
	else {
	  if(dirs.size() > 0) {
	    Object *trav = body->Split(leave);
	    trav->SetSkill(items[req+1], leave*10000);
	    body->SetSkill(items[req+1], orig - leave*10000);
	    trav->BusyFor(0, dirs.front());
	    }
	  body->BusyFor(8000 + (rand() & 0xFFF));
	  }
	}
      else {
	body->BusyFor(8000 + (rand() & 0xFFF));
	}

//      if(body->Skill("Personality") & 2) {		// Punk
//	//body->BusyFor(500, "say Yo yo!");
//	}
//      else if(body->Skill("Personality") & 4) {		// Normal
//	//body->BusyFor(500, "say How do you do?");
//	}
//      else if(body->Skill("Personality") & 8) {		// Rich
//	//body->BusyFor(500, "say Hi.");
//	}
      }
    }
  else if(type == MIND_TBATRIG) {
    if(body && body->Parent()) {
      body->Parent()->SendOut(0, 0,
	(string(CMAG "TRIGGERED:\n") + script + CNRM).c_str(),
	(string(CMAG "TRIGGERED:\n") + script + CNRM).c_str(),
	NULL, NULL
	);
      if(!script[spos]) return;	//Empty
      int quota = 1000;
      while(spos != string::npos) {
	PING_QUOTA();
	if(!strncasecmp(script.c_str()+spos, "wait ", 5)) {
	  int time = 0;
	  sscanf(script.c_str()+spos+5, "%d", &time);
	  if(time > 0) {
	    spos = skip_line(script, spos);
	    Suspend(time*1000);
	    }
	  else {
	    fprintf(stderr, "Error: Told 'wait %s' in #%d\n",
		script.c_str()+spos+5, body->Skill("TBAScript")
		);
	    }
	  return;
	  }

	else if(!strncasecmp(script.c_str()+spos, "if ", 3)) {
	  if(tba_eval(script.c_str()+spos+3)) {
	    spos = skip_line(script, spos);	//Is "if" and is true
	    }
	  else {				//Was elseif or false
	    int depth = 0;
	    spos = skip_line(script, spos);
	    while(spos != string::npos) {	//Skip to end/elseif
	      PING_QUOTA();
	      if((!depth) && (!strncasecmp(script.c_str()+spos, "elseif ", 7))) {
		spos += 4;	//Make it into an "if" and go
		break;
		}
	      else if(!strncasecmp(script.c_str()+spos, "end", 3)) {
		if(!depth) {	//Only done if all the way back
		  spos = skip_line(script, spos);
		  break;
		  }
		--depth;	//Otherwise am just 1 nesting level less deep
		}
	      else if(!strncasecmp(script.c_str()+spos, "if ", 3)) {
		++depth;	//Am now 1 nesting level deeper!
		}
	      spos = skip_line(script, spos);
	      }
	    }
	  }

	else if(!strncasecmp(script.c_str()+spos, "elseif ", 7)) {
	  int depth = 0;
	  spos = skip_line(script, spos);
	  while(spos != string::npos) {	//Skip to end (considering nesting)
	    PING_QUOTA();
	    if(!strncasecmp(script.c_str()+spos, "end", 3)) {
	      if(depth == 0) {	//Only done if all the way back
		spos = skip_line(script, spos);
		break;
		}
	      --depth;	//Otherwise am just 1 nesting level less deep
	      }
	    else if(!strncasecmp(script.c_str()+spos, "if ", 3)) {
	      ++depth;	//Am now 1 nesting level deeper!
	      }
	    spos = skip_line(script, spos);
	    }
	  }

	else if(!strncasecmp(script.c_str()+spos, "%echo% ", 7)) {
	  string mes = script.c_str() + spos + 6;
	  size_t end = mes.find_first_of("\n\r");
	  if(end != string::npos) mes = mes.substr(0, end);
	  trim_string(mes);
	  mes += "\n";
	  body->Parent()->SendOut(0, 0, mes.c_str(), mes.c_str(), NULL, NULL);
	  spos = skip_line(script, spos);
	  }

	else if(!strncasecmp(script.c_str()+spos, "end", 3)) {
	  //Ignore these, as we only hit them when we're running inside if
	  spos = skip_line(script, spos);
	  }
	else {		//Silently ignore the rest for now!  FIXME: Error mes.
	  spos = skip_line(script, spos);
	  }
	}
      }
    }
  else if(type == MIND_TBAMOB) {
    if((!body) || (istick >= 0 && body->StillBusy())) return;

    //Temporary
    if(body && body->ActTarg(ACT_WEAR_SHIELD) && (!body->IsAct(ACT_HOLD))) {
      string command = string("hold ") + body->ActTarg(ACT_WEAR_SHIELD)->ShortDesc();
      body->BusyFor(500, command.c_str());
      return;
      }
    else if(body && body->ActTarg(ACT_WEAR_SHIELD) && body->ActTarg(ACT_HOLD)
	&& body->ActTarg(ACT_WEAR_SHIELD) != body->ActTarg(ACT_HOLD)) {
      Object *targ = body->ActTarg(ACT_HOLD);
      if(body->Stash(targ)) {
	if(body->Parent())
	  body->Parent()->SendOut(ALL, 0, ";s stashes ;s.\n", "", body, targ);
	string command = string("hold ") + body->ActTarg(ACT_WEAR_SHIELD)->ShortDesc();
	body->BusyFor(500, command.c_str());
	}
      else {
	//fprintf(stderr, "Warning: %s can't use his shield!\n", body->Name());
	}
      return;
      }

    //AGGRESSIVE and WIMPY TBA Mobs
    if(body && body->Parent() && (body->Skill("TBAAction") & 160) == 160
	&& (!body->IsAct(ACT_FIGHT))) {
      typeof(body->Parent()->Contents()) others
	= body->PickObjects("everyone", LOC_NEARBY);
      typeof(others.begin()) other;
      for(other = others.begin(); other != others.end(); ++other) {
	if((!(*other)->Skill("TBAAction")) //FIXME: Other mobs?
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
      if(istick == 1 && body->IsUsing("Perception")) {
	body->BusyFor(500, "stop");
	}
      else if(istick == 0				//Triggered Only
		&& (!body->IsUsing("Perception"))	//Not already searching
		&& (!body->StillBusy())			//Not already responding
		&& body->Stun() < 6			//I'm not stunned
		&& body->Phys() < 6			//I'm not injured
		&& (!body->IsAct(ACT_SLEEP))		//I'm not asleep
		&& (!body->IsAct(ACT_REST))		//I'm not resting
		) {
	body->BusyFor(500, "search");
	}
      }
    //AGGRESSIVE and (!WIMPY) TBA Mobs
    else if(body && body->Parent() && (body->Skill("TBAAction") & 160) == 32
	&& (!body->IsAct(ACT_FIGHT))) {
      typeof(body->Parent()->Contents()) others
	= body->PickObjects("everyone", LOC_NEARBY);
      typeof(others.begin()) other;
      for(other = others.begin(); other != others.end(); ++other) {
	if((!(*other)->Skill("TBAAction")) //FIXME: Other mobs?
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
      if(istick == 1 && body->IsUsing("Perception")) {
	body->BusyFor(500, "stop");
	}
      else if(istick == 0				//Triggered Only
		&& (!body->IsUsing("Perception"))	//Not already searching
		&& (!body->StillBusy())			//Not already responding
		&& body->Stun() < 6			//I'm not stunned
		&& body->Phys() < 6			//I'm not injured
		&& (!body->IsAct(ACT_SLEEP))		//I'm not asleep
		&& (!body->IsAct(ACT_REST))		//I'm not resting
		) {
	body->BusyFor(500, "search");
	}
      }
    //HELPER TBA Mobs
    if(body && body->Parent() && (body->Skill("TBAAction") & 4096)
	&& (!body->IsAct(ACT_FIGHT))) {
      typeof(body->Parent()->Contents()) others
	= body->PickObjects("everyone", LOC_NEARBY);
      typeof(others.begin()) other;
      for(other = others.begin(); other != others.end(); ++other) {
	if((!(*other)->Skill("TBAAction")) //FIXME: Other mobs?
		&& body->Stun() < 6			//I'm not stunned
		&& body->Phys() < 6			//I'm not injured
		&& (!body->IsAct(ACT_SLEEP))		//I'm not asleep
		&& (!body->IsAct(ACT_REST))		//I'm not resting
		&& (*other)->Attribute(1)		//It's not a rock
		&& (!(*other)->IsAct(ACT_DEAD))		//It's not already dead
		&& (*other)->IsAct(ACT_FIGHT)		//It's figting someone
		&& (*other)->ActTarg(ACT_FIGHT)->HasSkill("TBAAction")
							//...against another MOB
	        ) {
	  string command = string("call ALARM; attack ") + (*other)->ShortDesc();
	  body->BusyFor(500, command.c_str());
	  //fprintf(stderr, "%s: Tried '%s'\n", body->ShortDesc(), command.c_str());
	  return;
	  }
	}
      if(!body->IsUsing("Perception")) {		//Don't let guard down!
	body->BusyFor(500, "search");
	}
      else if(istick == 1				//Perioidic searching
		&& (!body->StillBusy())			//Not already responding
		&& body->Stun() < 6			//I'm not stunned
		&& body->Phys() < 6			//I'm not injured
		&& (!body->IsAct(ACT_SLEEP))		//I'm not asleep
		&& (!body->IsAct(ACT_REST))		//I'm not resting
		) {
	body->BusyFor(500, "search");
	}
      }
    //NON-SENTINEL TBA Mobs
    if(body && body->Parent() && ((body->Skill("TBAAction") & 2) == 0)
	&& (!body->IsAct(ACT_FIGHT)) && (istick == 1)
	&& (!body->IsAct(ACT_REST)) && (!body->IsAct(ACT_SLEEP))
	&& body->Stun() < 6 && body->Phys() < 6
	&& body->Roll("Willpower", 9)) {


      map<Object*, const char*> cons;
      cons[body->PickObject("north", LOC_NEARBY)] = "north";
      cons[body->PickObject("south", LOC_NEARBY)] = "south";
      cons[body->PickObject("east",  LOC_NEARBY)] = "east";
      cons[body->PickObject("west",  LOC_NEARBY)] = "west";
      cons[body->PickObject("up",    LOC_NEARBY)] = "up";
      cons[body->PickObject("down",  LOC_NEARBY)] = "down";
      cons.erase(NULL);

      map<Object*, const char*> cons2 = cons;
      map<Object*, const char*>::iterator dir = cons2.begin();
      for(; dir != cons2.end(); ++dir) {
	if((!dir->first->ActTarg(ACT_SPECIAL_LINKED))
		|| (!dir->first->ActTarg(ACT_SPECIAL_LINKED)->Parent())) {
	  cons.erase(dir->first);
	  continue;
	  }

	Object *dest = dir->first->ActTarg(ACT_SPECIAL_LINKED)->Parent();
	if(!TBACanWanderTo(dest)) {
	  cons.erase(dir->first);
	  }
	}

      if(cons.size()) {
	int res = rand() % cons.size();
	map<Object*, const char*>::iterator dir = cons.begin();
	while(res > 0) { ++dir; --res; }
	if(body->StillBusy()) {		//Already doing something (from above)
	  body->DoWhenFree(dir->second);
	  }
	else {
	  body->BusyFor(500, dir->second);
	  }
	return;
	}
      }
    }
  else if(type == MIND_CIRCLEMOB) {
    if((!body) || (istick >= 0 && body->StillBusy())) return;

    //Temporary
    if(body && body->ActTarg(ACT_WEAR_SHIELD) && (!body->IsAct(ACT_HOLD))) {
      string command = string("hold ") + body->ActTarg(ACT_WEAR_SHIELD)->ShortDesc();
      body->BusyFor(500, command.c_str());
      return;
      }
    else if(body && body->ActTarg(ACT_WEAR_SHIELD) && body->ActTarg(ACT_HOLD)
	&& body->ActTarg(ACT_WEAR_SHIELD) != body->ActTarg(ACT_HOLD)) {
      Object *targ = body->ActTarg(ACT_HOLD);
      if(body->Stash(targ)) {
	if(body->Parent())
	  body->Parent()->SendOut(ALL, 0, ";s stashes ;s.\n", "", body, targ);
	string command = string("hold ") + body->ActTarg(ACT_WEAR_SHIELD)->ShortDesc();
	body->BusyFor(500, command.c_str());
	}
      else {
	//fprintf(stderr, "Warning: %s can't use his shield!\n", body->Name());
	}
      return;
      }

    //AGGRESSIVE and WIMPY Circle Mobs
    if(body && body->Parent() && (body->Skill("CircleAction") & 160) == 160
	&& (!body->IsAct(ACT_FIGHT))) {
      typeof(body->Parent()->Contents()) others
	= body->PickObjects("everyone", LOC_NEARBY);
      typeof(others.begin()) other;
      for(other = others.begin(); other != others.end(); ++other) {
	if((!(*other)->Skill("CircleAction")) //FIXME: Other mobs?
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
      if(istick == 1 && body->IsUsing("Perception")) {
	body->BusyFor(500, "stop");
	}
      else if(istick == 0				//Triggered Only
		&& (!body->IsUsing("Perception"))	//Not already searching
		&& (!body->StillBusy())			//Not already responding
		&& body->Stun() < 6			//I'm not stunned
		&& body->Phys() < 6			//I'm not injured
		&& (!body->IsAct(ACT_SLEEP))		//I'm not asleep
		&& (!body->IsAct(ACT_REST))		//I'm not resting
		) {
	body->BusyFor(500, "search");
	}
      }
    //AGGRESSIVE and (!WIMPY) Circle Mobs
    else if(body && body->Parent() && (body->Skill("CircleAction") & 160) == 32
	&& (!body->IsAct(ACT_FIGHT))) {
      typeof(body->Parent()->Contents()) others
	= body->PickObjects("everyone", LOC_NEARBY);
      typeof(others.begin()) other;
      for(other = others.begin(); other != others.end(); ++other) {
	if((!(*other)->Skill("CircleAction")) //FIXME: Other mobs?
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
      if(istick == 1 && body->IsUsing("Perception")) {
	body->BusyFor(500, "stop");
	}
      else if(istick == 0				//Triggered Only
		&& (!body->IsUsing("Perception"))	//Not already searching
		&& (!body->StillBusy())			//Not already responding
		&& body->Stun() < 6			//I'm not stunned
		&& body->Phys() < 6			//I'm not injured
		&& (!body->IsAct(ACT_SLEEP))		//I'm not asleep
		&& (!body->IsAct(ACT_REST))		//I'm not resting
		) {
	body->BusyFor(500, "search");
	}
      }
    //HELPER Circle Mobs
    if(body && body->Parent() && (body->Skill("CircleAction") & 4096)
	&& (!body->IsAct(ACT_FIGHT))) {
      typeof(body->Parent()->Contents()) others
	= body->PickObjects("everyone", LOC_NEARBY);
      typeof(others.begin()) other;
      for(other = others.begin(); other != others.end(); ++other) {
	if((!(*other)->Skill("CircleAction")) //FIXME: Other mobs?
		&& body->Stun() < 6			//I'm not stunned
		&& body->Phys() < 6			//I'm not injured
		&& (!body->IsAct(ACT_SLEEP))		//I'm not asleep
		&& (!body->IsAct(ACT_REST))		//I'm not resting
		&& (*other)->Attribute(1)		//It's not a rock
		&& (!(*other)->IsAct(ACT_DEAD))		//It's not already dead
		&& (*other)->IsAct(ACT_FIGHT)		//It's figting someone
		&& (*other)->ActTarg(ACT_FIGHT)->HasSkill("CircleAction")
							//...against another MOB
	        ) {
	  string command = string("call ALARM; attack ") + (*other)->ShortDesc();
	  body->BusyFor(500, command.c_str());
	  //fprintf(stderr, "%s: Tried '%s'\n", body->ShortDesc(), command.c_str());
	  return;
	  }
	}
      if(!body->IsUsing("Perception")) {		//Don't let guard down!
	body->BusyFor(500, "search");
	}
      else if(istick == 1				//Perioidic searching
		&& (!body->StillBusy())			//Not already responding
		&& body->Stun() < 6			//I'm not stunned
		&& body->Phys() < 6			//I'm not injured
		&& (!body->IsAct(ACT_SLEEP))		//I'm not asleep
		&& (!body->IsAct(ACT_REST))		//I'm not resting
		) {
	body->BusyFor(500, "search");
	}
      }
    //NON-SENTINEL Circle Mobs
    if(body && body->Parent() && ((body->Skill("CircleAction") & 2) == 0)
	&& (!body->IsAct(ACT_FIGHT)) && (istick == 1)
	&& (!body->IsAct(ACT_REST)) && (!body->IsAct(ACT_SLEEP))
	&& body->Stun() < 6 && body->Phys() < 6
	&& body->Roll("Willpower", 9)) {


      map<Object*, const char*> cons;
      cons[body->PickObject("north", LOC_NEARBY)] = "north";
      cons[body->PickObject("south", LOC_NEARBY)] = "south";
      cons[body->PickObject("east",  LOC_NEARBY)] = "east";
      cons[body->PickObject("west",  LOC_NEARBY)] = "west";
      cons[body->PickObject("up",    LOC_NEARBY)] = "up";
      cons[body->PickObject("down",  LOC_NEARBY)] = "down";
      cons.erase(NULL);

      map<Object*, const char*> cons2 = cons;
      map<Object*, const char*>::iterator dir = cons2.begin();
      for(; dir != cons2.end(); ++dir) {
	if((!dir->first->ActTarg(ACT_SPECIAL_LINKED))
		|| (!dir->first->ActTarg(ACT_SPECIAL_LINKED)->Parent())) {
	  cons.erase(dir->first);
	  continue;
	  }

	Object *dest = dir->first->ActTarg(ACT_SPECIAL_LINKED)->Parent();
	if(!CircleCanWanderTo(dest)) {
	  cons.erase(dir->first);
	  }
	}

      if(cons.size()) {
	int res = rand() % cons.size();
	map<Object*, const char*>::iterator dir = cons.begin();
	while(res > 0) { ++dir; --res; }
	if(body->StillBusy()) {		//Already doing something (from above)
	  body->DoWhenFree(dir->second);
	  }
	else {
	  body->BusyFor(500, dir->second);
	  }
	return;
	}
      }
    }
  }


int Mind::TBACanWanderTo(Object *dest) {
  if(dest->Skill("TBAZone") == 999999) { //NO_MOBS TBA Zone
    return 0;				//Don't Enter NO_MOBS Zone!
    }
  else if(body->Skill("TBAAction") & 64) {	//STAY_ZONE TBA MOBs
    if(dest->Skill("TBAZone") != body->Parent()->Skill("TBAZone")) {
      return 0;				//Don't Leave Zone!
      }
    }
  else if(body->Skill("Swimming") == 0) {		//Can't Swim?
    if(dest->Skill("WaterDepth") == 1) {
      return 0;				//Can't swim!
      }
    }
  else if(!(body->Skill("TBAAffection")&64)) {	//Can't Waterwalk?
    if(dest->Skill("WaterDepth") >= 1) {	//Need boat!
      return 0;				//FIXME: Have boat?
      }
    }
  return 1;
  }

int Mind::CircleCanWanderTo(Object *dest) {
  if(dest->Skill("CircleZone") == 999999) { //NO_MOBS Circle Zone
    return 0;				//Don't Enter NO_MOBS Zone!
    }
  else if(body->Skill("CircleAction") & 64) {	//STAY_ZONE Circle MOBs
    if(dest->Skill("CircleZone") != body->Parent()->Skill("CircleZone")) {
      return 0;				//Don't Leave Zone!
      }
    }
  else if(body->Skill("Swimming") == 0) {		//Can't Swim?
    if(dest->Skill("WaterDepth") == 1) {
      return 0;				//Can't swim!
      }
    }
  else if(!(body->Skill("CircleAffection")&64)) {	//Can't Waterwalk?
    if(dest->Skill("WaterDepth") >= 1) {	//Need boat!
      return 0;				//FIXME: Have boat?
      }
    }
  return 1;
  }

void Mind::SetSpecialPrompt(const char *newp) {
  prompt = string(newp);
  UpdatePrompt();
  }

const char *Mind::SpecialPrompt() {
  return prompt.c_str();
  }

Mind *new_mind(int tp, Object *obj, Object *obj2) {
  Mind *m = new Mind();
  if(tp == MIND_TBATRIG && obj) {
    m->SetTBATrigger(obj, obj2);
    }
  else if(obj) {
    m->Attach(obj);
    }
  return m;
  }

int new_trigger(Object *obj, Object *tripper) {
  if((!obj) || (!(obj->Parent()))) return 0;

  Mind *m = new_mind(MIND_TBATRIG, obj, tripper);
//  fprintf(stderr, "Attached to '%s' on '%s'\n", 
//	obj->Name(), obj->Parent()->Name()
//	);
  m->Think(1);
//  delete(m);	//Needs to take care of itself!	FIXME!
  return 0;
  }

list<pair<int, Mind*> > Mind::waiting;
void Mind::Suspend(int msec) {
  waiting.push_back(make_pair(msec, this));
  }

void Mind::Resume(int passed) {
  list<pair<int, Mind*> >::iterator cur = waiting.begin();
  while(cur != waiting.end()) {
    list<pair<int, Mind*> >::iterator tmp = cur;
    ++cur;	//Inc cur first, in case I erase tmp.
    if(tmp->first <= passed) {
      waiting.erase(tmp);
      tmp->second->Think(0);
      }
    else {
      tmp->first -= passed;
      }
    }
  }
