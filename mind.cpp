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
#include "commands.h"

list<Mind *>recycle_bin;

static const char *bstr[2] = {"0", "1"};

static int tba_eval(string expr);

static string tba_comp(string expr) {
  size_t end = expr.find_first_of("\n\r");
  if(end != string::npos) expr = expr.substr(0, end);
  trim_string(expr);
//  if(strcasestr(expr.c_str(), "tea")) {
//    fprintf(stderr, "Expr: '%s'\n", expr.c_str());
//    }

  if(expr[0] == '(') return tba_comp(expr.substr(1));	//FIXME: Really Do This!
  if(expr[0] == '!') {
    string base = tba_comp(expr.substr(1));
    if(base == "0" || base == "") return "1";
    return "0";
    }

  size_t op = expr.find_first_of("|&=!<>/-+*", 1);	//Skip Leading "-"
  if(op == string::npos) return expr;	//No ops, just val

  int oper = 0;		//Positive for 2-char ops, negative for 1-char
  int weak = 0;		//Reverse-Precedence!  Hack!!!
  if(!strncmp(expr.c_str()+op, "/=", 2)) { oper = 1; }
  else if(!strncmp(expr.c_str()+op, "==", 2)) { oper = 2; }
  else if(!strncmp(expr.c_str()+op, "!=", 2)) { oper = 3; }
  else if(!strncmp(expr.c_str()+op, "<=", 2)) { oper = 4; }
  else if(!strncmp(expr.c_str()+op, ">=", 2)) { oper = 5; }
  else if(!strncmp(expr.c_str()+op, "&&", 2)) { oper = 6; weak = 1; }
  else if(!strncmp(expr.c_str()+op, "||", 2)) { oper = 7; weak = 1; }
  else if(!strncmp(expr.c_str()+op, "<", 1)) { oper = -1; }
  else if(!strncmp(expr.c_str()+op, ">", 1)) { oper = -2; }
  else if(!strncmp(expr.c_str()+op, "+", 1)) { oper = -3; }
  else if(!strncmp(expr.c_str()+op, "-", 1)) { oper = -4; }
  else if(!strncmp(expr.c_str()+op, "*", 1)) { oper = -5; }
  else if(!strncmp(expr.c_str()+op, "/", 1)) { oper = -6; }

  if(oper != 0) {
    string arg1 = expr.substr(0, op);
    trim_string(arg1);
    if(oper > 0) expr = expr.substr(op+2);	//2-char
    else expr = expr.substr(op+1);		//1-char
    string arg2;
    if(weak) {
      arg2 = tba_comp(expr);
      expr = "";
      }
    else {
      arg2 = expr;
      op = expr.find_first_of("|&=!<>/-+*)\n\r");
      if(op != string::npos) {
	arg2 = expr.substr(0, op);
	expr = expr.substr(op);
	}
      else {
	expr = "";
	}
      }
    trim_string(arg2);

    int res = 0;
    string comp = "0";
    if(oper == 1 && strcasestr(arg1.c_str(), arg2.c_str())) comp = "1";
    else if(oper == 2 && (arg1 == arg2)) comp = "1";
    else if(oper == 3 && (arg1 != arg2)) comp = "1";
    else if(oper == 4 && (atoi(arg1.c_str()) <= atoi(arg2.c_str()))) comp = "1";
    else if(oper == 5 && (atoi(arg1.c_str()) >= atoi(arg2.c_str()))) comp = "1";
    else if(oper == -1 && (atoi(arg1.c_str()) < atoi(arg2.c_str()))) comp = "1";
    else if(oper == -2 && (atoi(arg1.c_str()) > atoi(arg2.c_str()))) comp = "1";
    else if(oper == 6 && (tba_eval(arg1) && tba_eval(arg2))) comp = "1";
    else if(oper == 7 && (tba_eval(arg1) || tba_eval(arg2))) comp = "1";
    else if(oper == -3) res = atoi(arg1.c_str()) + atoi(arg2.c_str());
    else if(oper == -4) res = atoi(arg1.c_str()) - atoi(arg2.c_str());
    else if(oper == -5) res = atoi(arg1.c_str()) * atoi(arg2.c_str());
    else if(oper == -6) {	//Protect from div by zero
      int val2 = atoi(arg2.c_str());
      res = atoi(arg1.c_str());
      if(val2 != 0) res /= val2;
      }

    if(oper <= -3) {	//Non-Boolean - actual numeric value
      char buf[256];
      sprintf(buf, "%d", res);
      comp = buf;
      //fprintf(stderr, "RES: %s\n", buf);
      }

    if(expr != "") {
      expr = comp + " " + expr;
      return tba_comp(expr);
      }
    return comp;
    }

  return "0";
  }

static int tba_eval(string expr) {
  string base = tba_comp(expr);
  trim_string(base);
  if(base.length() == 0) return 0;	//Null
  int ret = 0, len = 0;
  sscanf(base.c_str(), " %d %n", &ret, &len);
  if(len == int(base.length())) return ret;	//Numeric
  return 1;	//Non-Numberic, Non-NULL
  }

static void tba_varsub_str(string &code, const string &var, const string &val) {
  replace_all(code, "%%"+var+"%%", val);
  replace_all(code, "%"+var+"%", val);

  string arg1 = val, argr = "";
  size_t apos = val.find_first_of(" \t\n\r");
  if(apos != string::npos) {
    arg1 = val.substr(0, apos);
    apos = val.find_first_not_of(" \t\n\r", apos);
    if(apos != string::npos) argr = val.substr(apos);
    }
  replace_all(code, "%%"+var+".car%%", arg1);
  replace_all(code, "%"+var+".car%", arg1);
  replace_all(code, "%%"+var+".cdr%%", argr);
  replace_all(code, "%"+var+".cdr%", argr);
  string trim = val;
  trim_string(trim);
  replace_all(code, "%%"+var+".trim%%", trim);
  replace_all(code, "%"+var+".trim%", trim);

  if(val[0] == '%' && val[val.length()-1] == '%') {
    string tval = val.substr(1, val.length()-2);
    replace_all(code, "%"+var+".", "%"+tval+".");
    }
  }
static void tba_varsub_obj(string &code, const string &var, const Object *obj) {
  if(!obj) return;
  int vnum = obj->Skill("TBAMOB");
  if(vnum < 1) vnum = obj->Skill("TBAObject");
  if(vnum < 1) vnum = obj->Skill("TBARoom");
  if(vnum > 0) {
    vnum -= 1000000;	//Convert from Acid number
    char svnum[256];
    sprintf(svnum, "%d", vnum);
    replace_all(code, "%"+var+".vnum%", vnum);
    replace_all(code, "%"+var+".vnum("+svnum+")%", "1");
    size_t beg = code.find("%"+var+".vnum(");
    while(beg != string::npos) {
      size_t end = code.find('%');
      if(end != string::npos) {
	code.replace(beg, end+1, "0");
	}
      beg = code.find("%"+var+".vnum(");
      }
    }
  replace_all(code, "%"+var+".level%", obj->Exp()/10+1);
  replace_all(code, "%"+var+".name%", obj->Name());
  replace_all(code, "%"+var+".shortdesc%", obj->ShortDesc());
  replace_all(code, "%"+var+".heshe%", obj->Pron());
  replace_all(code, "%"+var+".hisher%", obj->Poss());
  replace_all(code, "%"+var+".himher%", obj->Obje());
  replace_all(code, "%"+var+".maxhitp%", 1000);	//Everybody has 1000 HP.
  replace_all(code, "%"+var+".hitp%", 1000 - 50 * (obj->Phys() + obj->Stun()));
  replace_all(code, "%"+var+".is_pc%", is_pc(obj));
  replace_all(code, "%"+var+".is_killer%", 0);	//FIXME: Real value?
  replace_all(code, "%"+var+".is_thief%", 0);	//FIXME: Real value?
  replace_all(code, "%"+var+".con%", (obj->Attribute(0) - 2) * 3);
  replace_all(code, "%"+var+".dex%", (obj->Attribute(1) - 2) * 3);
  replace_all(code, "%"+var+".str%", (obj->Attribute(2) - 2) * 3);
  replace_all(code, "%"+var+".cha%", (obj->Attribute(3) - 2) * 3);
  replace_all(code, "%"+var+".int%", (obj->Attribute(4) - 2) * 3);
  replace_all(code, "%"+var+".wis%", (obj->Attribute(5) - 2) * 3);
  replace_all(code, "%"+var+".fighting%", bstr[obj->IsAct(ACT_FIGHT)]);

  int gold;
  list<Object *> pay = obj->PickObjects("all a gold piece", LOC_INTERNAL);
  list<Object *>::const_iterator coin;
  for(coin = pay.begin(); coin != pay.end(); ++coin) {
    gold += (*coin)->Quantity();
    }
  replace_all(code, "%"+var+".gold%", gold);
  }

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

void Mind::SetTBATrigger(Object *tr, Object *tripper, string text) {
  if((!tr) || (!(tr->Parent()))) return;

  type = MIND_TBATRIG;
  pers = fileno(stderr);
  Attach(tr);
  spos = 0;
  script = body->LongDesc();
  script += "\n";
  if(tripper) ovars["actor"] = tripper;

  int stype = body->Skill("TBAScriptType");
  if(stype & 0x0000008) {				//-SPEECH Triggers
    svars["speech"] = text;
    }
  if((stype & 0x4000040) == 0x4000040			//ROOM-ENTER Triggers
		|| stype & 0x0010000) {			//-LEAVE Triggers
    svars["direction"] = text;
    }
  if(stype & 0x0000004) {				//-COMMAND Triggers
    size_t part = text.find_first_of(" \t\n\r");
    if(part == string::npos) svars["cmd"] = text;
    else {
      svars["cmd"] = text.substr(0, part);
      part = text.find_first_not_of(" \t\n\r", part);
      if(part != string::npos) svars["arg"] = text.substr(part);
      }
    }
  }

void Mind::SetTBAMob() {
  type = MIND_TBAMOB;
  pers = fileno(stderr);
  }

void Mind::SetNPC() {
  type = MIND_NPC;	//FIXME: Implement these!
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
  return "attack";
  }


#define QUOTAERROR1	CRED "#%d Error: script quota exceeded - killed.\n" CNRM
#define QUOTAERROR2	body->Skill("TBAScript")
#define PING_QUOTA() { \
	--quota; \
	if(quota < 1) {	\
		fprintf(stderr, QUOTAERROR1, QUOTAERROR2); \
		Disable(); \
		return; \
		} \
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
      ovars["self"] = body->Parent();
      Object *room = ovars["self"];
      while(room && room->Skill("TBARoom") == 0) room = room->Parent();
      if(!room) {
//	fprintf(stderr, CRED "#%d Error: Self not in a room - killed!\n" CNRM,
//		body->Skill("TBAScript")
//		);
	Disable();
	return;
	}
      Object *aroom = NULL;
      if(ovars.count("actor") && ovars["actor"]) {
	aroom = ovars["actor"];
	while(aroom && aroom->Skill("TBARoom") == 0) aroom = aroom->Parent();
	}

//      if(self->Matches("picard")
//		|| self->Matches("teleporter")
//		|| self->Matches("mindflayer")
//		|| self->Matches("in the mines")
//		) {
//	room->SendOut(0, 0,
//		(string(CMAG "TRIGGERED:\n") + script + CNRM).c_str(),
//		(string(CMAG "TRIGGERED:\n") + script + CNRM).c_str(),
//		NULL, NULL
//		);
//	fprintf(stderr, "%s\n",
//		(string(CMAG "TRIGGERED:\n") + script + CNRM).c_str()
//		);
//	}

      if(!script[spos]) return;	//Empty
      int quota = 1024;
      int stype = body->Skill("TBAScriptType");
      while(spos != string::npos) {
	if(script[spos] == '*') {		//Comments
	  spos = skip_line(script, spos);
	  continue;
	  }

	string line;
	size_t endl = script.find_first_of("\n\r", spos);
	if(endl == string::npos) line = script.substr(spos);
	else line = script.substr(spos, endl-spos);

	//Variable sub (Single line)
	string tmp = "";
	map<string, Object *> tmpvars;
	map<string, Object *> curvars = ovars;
	while(tmp != line || tmpvars != curvars) {
	  if(0
//		|| line.find("eval loc ") != string::npos
//		|| line.find("set first ") != string::npos
//		|| line.find("exclaim") != string::npos
//		|| line.find("speech") != string::npos
		) {
	    fprintf(stderr, CGRN "#%d Debug: '%s'\n" CNRM,
		body->Skill("TBAScript"), line.c_str());
	    }
	  PING_QUOTA();
	  tmp = line;
	  tmpvars = curvars;
	  map<string, string>::iterator svarent = svars.begin();
	  for(; svarent != svars.end(); ++svarent) {
	    tba_varsub_str(line, svarent->first, svarent->second);
	    }
	  map<string, Object *>::iterator ovarent = tmpvars.begin();
	  for(; ovarent != tmpvars.end(); ++ovarent) {
	    tba_varsub_obj(line, ovarent->first, ovarent->second);
	    //Sub-Object Vars
	    if(line.find("%"+ovarent->first+".room") != string::npos) {
	      Object *other = ovarent->second;
	      while(other && other->Skill("TBARoom") == 0)
		other = other->Parent();
	      if(other) curvars[ovarent->first+".room"] = other;
	      }
	    if(line.find("%"+ovarent->first+".people") != string::npos) {
	      Object *other =
		ovarent->second->PickObject("someone", LOC_INTERNAL);
	      if(other) curvars[ovarent->first+".people"] = other;
	      }
	    if(line.find("%"+ovarent->first+".contents") != string::npos) {
	      Object *other =
		ovarent->second->PickObject("something", LOC_INTERNAL);
	      if(other) curvars[ovarent->first+".contents"] = other;
	      }
	    if(line.find("%"+ovarent->first+".carried_by") != string::npos) {
	      Object *other = ovarent->second->Owner();
	      if(other) curvars[ovarent->first+".carried_by"] = other;
	      }
	    if(line.find("%"+ovarent->first+".follower") != string::npos) {
	      set<Object*> touch = ovarent->second->Touching();
	      set<Object*>::iterator tent = touch.begin();
	      for(; tent != touch.end(); ++tent) {
		if((*tent)->ActTarg(ACT_FOLLOW) == ovarent->second) {
		  curvars[ovarent->first+".follower"] = (*tent);
		  break;
		  }
		}
	      }
	    }

	  size_t var;

	  var = string::npos;
	  var = line.find("%random.char%");
	  while(var != string::npos) {
	    list<Object*> others;
	    if(curvars["self"] == room) {
	      others = curvars["self"]->PickObjects("everyone", LOC_INTERNAL);
	      }
	    else if(curvars["self"]->Owner()) {
	      others = curvars["self"]->Owner()->PickObjects("everyone", LOC_NEARBY);
	      }
	    else {
	      others = curvars["self"]->PickObjects("everyone", LOC_NEARBY);
	      }
	    if(others.size() > 0) {
	      curvars["temp_rand_char"] = others.back();
	      line.replace(var, var+13, "%temp_rand_char%");
	      if(0
//		|| script.find("%damage% %actor% -%actor.level%") != string::npos
		) {
		fprintf(stderr, CGRN "#%d Random: '%s'\n" CNRM,
			body->Skill("TBAScript"), curvars["temp_rand_char"]->Name());
		}
	      }
	    else {
	      curvars.erase("temp_rand_char");
	      line.replace(var, var+13, "");
	      }
	    var = line.find("%random.char%", var + 1);
	    }
	  var = string::npos;
	  var = line.find("%random.dir%");
	  while(var != string::npos) {
	    set<Object*> options;
	    options.insert(room->PickObject("north", LOC_INTERNAL));
	    options.insert(room->PickObject("south", LOC_INTERNAL));
	    options.insert(room->PickObject("east", LOC_INTERNAL));
	    options.insert(room->PickObject("west", LOC_INTERNAL));
	    options.insert(room->PickObject("up", LOC_INTERNAL));
	    options.insert(room->PickObject("down", LOC_INTERNAL));
	    options.erase(NULL);
	    if(options.size() > 0) {
	      line.replace(var, var+12, (*(options.begin()))->ShortDesc());
	      }
	    else {
	      line.replace(var, var+12, "");
	      }
	    var = line.find("%random.dir%", var + 1);
	    }
	  var = line.find("%random.");
	  while(var != string::npos) {
	    if(isdigit(line[var+8])) {
	      size_t vend = line.find_first_not_of("0123456789", var+8);
	      if(vend != string::npos && line[vend] == '%') {
		char nstr[64];
		sprintf(nstr, "%d", (rand()%atoi(line.c_str()+var+8))+1);
		line = line.replace(var, vend+1, nstr);
		var = 0;
		}
	      }
	    var = line.find("%random.", var + 1);
	    }
	  if(line.find("%time.hour%") != string::npos) {
	    Object *world = body;
	    while(world->Parent()->Parent()) world = world->Parent();
	    if(world->Skill("Day Time") && world->Skill("Day Length")) {
	      int hour = world->Skill("Day Time");
	      hour *= 24;
	      hour /= world->Skill("Day Length");
	      replace_all(line, "%time.hour%", hour);
	      }
	    }
	  }
	replace_all(line, "%damage% ", "wdamage ");
	replace_all(line, "%echo% ", "mecho ");
	replace_all(line, "%send% %actor% ", "send_actor ");
	replace_all(line, "%force% %actor% ", "force_actor ");
	replace_all(line, "%echoaround% ", "echoaround ");
	replace_all(line, "%teleport% ", "transport ");
	replace_all(line, "%zoneecho% ", "zoneecho ");
	replace_all(line, "%asound% ", "asound ");
	replace_all(line, "%force% ", "force ");
	replace_all(line, "%send% ", "send ");
	replace_all(line, "%door% ", "door ");
	replace_all(line, "%load% ", "load ");
	replace_all(line, "%at% ", "at ");

	com_t com = identify_command(line);	//ComNum for Pass-Through

	if((!strncasecmp(line.c_str(), "eval ", 5))
		|| (!strncasecmp(line.c_str(), "set ", 4))) {
	  size_t lpos = line.find_first_not_of(" \t", 4);
	  if(lpos != string::npos) {
	    line = line.substr(lpos);
	    size_t end1 = line.find_first_of(" \t\n\r");
	    if(end1 != string::npos) {
	      string var = line.substr(0, end1);
	      lpos = line.find_first_not_of(" \t", end1 + 1);
	      if(lpos != string::npos) {
		string val = line.substr(lpos);
		if(0
//			|| var.find("midgaard") != string::npos
//			|| var.find("exclaim") != string::npos
//			|| var.find("speech") != string::npos
			) {
		  fprintf(stderr,CGRN "#%d Debug: '%s' = '%s'\n" CNRM,
			body->Skill("TBAScript"), var.c_str(), val.c_str());
		  }
		if(tolower(script[spos]) == 'e') {
		  val = tba_comp(val);
		  }

		if(val[0] == '%' && val[val.length()-1] == '%'
			&& curvars.count(val.substr(1, val.length()-2))
			) {
		  ovars[var] = curvars[val.substr(1, val.length()-2)];
		  svars.erase(var);
		  }
		else {
		  svars[var] = val;
		  ovars.erase(var);
		  }
		}
	      }
	    }
	  spos = skip_line(script, spos);
	  continue;
	  }

	map<string, Object *>::iterator ovarent = curvars.begin();
	for(; ovarent != curvars.end(); ++ovarent) {
	  if(ovarent->second) {	//FIXME: Why does this happen?
	    char buf[128];
	    sprintf(buf, "OBJ:%p", ovarent->second);
	    replace_all(line, "%"+ovarent->first+"%", buf);
	    }
	  }
	if(line.find("%") != line.rfind("%")) {	//More than one '%'
	  fprintf(stderr, CYEL "#%d Warning: Didn't fully expand '%s'\n" CNRM,
		body->Skill("TBAScript"), line.c_str()
		);
	  }
	//Set undefined variables to ""
	while(line.find("%") != line.rfind("%")) {	//More than one '%'
	  size_t p1 = line.find('%');
	  while(line[p1+1] == '%') ++p1;
	  size_t p2 = line.find('%', p1+1);
	  if(p2 != string::npos) line.replace(p1, p2+1, "");
	  replace_all(line, "%%", "%");
	  }

	//Start of real command if/else if/else
	if(line.find("%") != line.rfind("%")) {		//More than one '%'
	  fprintf(stderr, CRED "#%d Error: Failed to fully expand '%s'\n" CNRM,
		body->Skill("TBAScript"), line.c_str()
		);
	  Disable();
	  return;
	  }

	else if(!strncasecmp(line.c_str(), "wait until ", 11)) {
	  int hour = 0, minute = 0, curmin = 0;
	  if(sscanf(line.c_str()+11, "%d:%d", &hour, &minute) > 0) {
	    spos = skip_line(script, spos);
	    if(hour >= 100) hour /= 100;
	    minute += hour * 60;
	    Object *world = room;
	    while(world->Parent()->Parent()) world = world->Parent();
	    if(world->Skill("Day Time") && world->Skill("Day Length")) {
	      curmin = world->Skill("Day Time");
	      curmin *= 24*60;
	      curmin /= world->Skill("Day Length");
	      }
	    if(minute > curmin) {	//Not Time Yet!
	      Suspend((minute - curmin)
		* 1000 * world->Skill("Day Length") / 24	//*60/60
		);
	      return;
	      }
	    }
	  else {
	    fprintf(stderr, CRED "#%d Error: Told '%s'\n" CNRM,
		body->Skill("TBAScript"), line.c_str()
		);
	    Disable();
	    return;
	    }
	  }

	else if(!strncasecmp(line.c_str(), "wait ", 5)) {
	  int time = 0;
	  sscanf(line.c_str()+5, "%d", &time);
	  if(time > 0) {
	    spos = skip_line(script, spos);
	    Suspend(time*1000);
	    }
	  else {
	    fprintf(stderr, CRED "#%d Error: Told '%s'\n" CNRM,
		body->Skill("TBAScript"), line.c_str()
		);
	    Disable();
	    }
	  return;
	  }

	else if(!strncasecmp(line.c_str(), "if ", 3)) {
	  if(tba_eval(line.c_str()+3)) {
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
	      else if(!strncasecmp(script.c_str()+spos, "else", 4)) {
		if(!depth) {	//Only right if all the way back
		  spos = skip_line(script, spos);
		  break;
		  }
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

	else if(!strncasecmp(line.c_str(), "else", 4)) {	//else/elseif
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

	else if(!strncasecmp(line.c_str(), "while ", 6)) {
	  int depth = 0;
	  size_t cond = 6, begin = spos, end = spos, skip = spos;
	  spos = skip_line(script, spos);
	  begin = spos;
	  while(spos != string::npos) {	//Skip to end (considering nesting)
	    PING_QUOTA();
	    if(!strncasecmp(script.c_str()+spos, "done", 4)) {
	      if(depth == 0) {	//Only done if all the way back
		end = spos;
		spos = skip_line(script, spos);
		skip = spos;
		break;
		}
	      --depth;	//Otherwise am just 1 nesting level less deep
	      }
	    else if(!strncasecmp(script.c_str()+spos, "switch ", 7)) {
	      ++depth;	//Am now 1 nesting level deeper!
	      }
	    else if(!strncasecmp(script.c_str()+spos, "while ", 6)) {
	      ++depth;	//Am now 1 nesting level deeper!
	      }
	    spos = skip_line(script, spos);
	    }
	  while(tba_eval(line.c_str() + cond)) {
	    PING_QUOTA();
	    string orig = script;
	    script = script.substr(begin, skip-begin);
	    trim_string(script);
	    spos = 0;
	    Think(istick);		//Semi-recursive to do the loop-age
	    if(type == MIND_MORON) {
	      return;
	      }
	    script = orig;
	    spos = skip;
	    }
	  }

	else if(!strncasecmp(line.c_str(), "switch ", 7)) {
	  int depth = 0;
	  size_t defl = 0;
	  string value = line.substr(7);
	  trim_string(value);
	  spos = skip_line(script, spos);
	  while(spos != string::npos) {	//Skip to end (considering nesting)
	    PING_QUOTA();
	    if(!strncasecmp(script.c_str()+spos, "done", 4)) {
	      if(depth == 0) {	//Only done if all the way back
		spos = skip_line(script, spos);
		break;
		}
	      --depth;	//Otherwise am just 1 nesting level less deep
	      }
	    else if(!strncasecmp(script.c_str()+spos, "switch ", 7)) {
	      ++depth;	//Am now 1 nesting level deeper!
	      }
	    else if(!strncasecmp(script.c_str()+spos, "while ", 6)) {
	      ++depth;	//Am now 1 nesting level deeper!
	      }
	    else if(depth == 0
			&& (!strncasecmp(script.c_str()+spos, "case ", 5))
			&& tba_eval(value + " == " + script.substr(spos+5))
			) {			//The actual case I want!
	      spos = skip_line(script, spos);
	      break;
	      }
	    else if(depth == 0
			&& (!strncasecmp(script.c_str()+spos, "default", 7))
			) {			//The actual case I want!
	      spos = skip_line(script, spos);
	      defl = spos;
	      continue;
	      }
	    spos = skip_line(script, spos);
	    }
	  if(defl != 0) spos = defl;	//Go to "default" case, if there was one
	  }

	else if(!strncasecmp(line.c_str(), "break", 5)) {//Skip to done
	  int depth = 0;
	  spos = skip_line(script, spos);
	  while(spos != string::npos) {	//Skip to end (considering nesting)
	    PING_QUOTA();
	    if(!strncasecmp(script.c_str()+spos, "done", 4)) {
	      if(depth == 0) {	//Only done if all the way back
		spos = skip_line(script, spos);
		break;
		}
	      --depth;	//Otherwise am just 1 nesting level less deep
	      }
	    else if(!strncasecmp(script.c_str()+spos, "switch ", 7)) {
	      ++depth;	//Am now 1 nesting level deeper!
	      }
	    else if(!strncasecmp(script.c_str()+spos, "while ", 6)) {
	      ++depth;	//Am now 1 nesting level deeper!
	      }
	    spos = skip_line(script, spos);
	    }
	  }

	else if(!strncasecmp(line.c_str(), "force_actor ", 12)) {
	  if(curvars.count("actor") && curvars["actor"]) {
	    Mind *amind = NULL;	//Make sure human minds see it!
	    vector<Mind *> mns = get_human_minds();
	    vector<Mind *>::iterator mn = mns.begin();
	    for(; mn != mns.end(); ++mn) {
	      if((*mn)->Body() == curvars["actor"]) {
		amind = *mn;
		}
	      }
	    handle_command(curvars["actor"], line.c_str()+12, amind);
	    }
	  spos = skip_line(script, spos);
	  }

	//Player commands Acid shares with TBA, not requiring arguments
	else if(com == COM_NORTH
		|| com == COM_SOUTH
		|| com == COM_EAST
		|| com == COM_WEST
		|| com == COM_UP
		|| com == COM_DOWN
		|| com == COM_SOCIAL
		|| com == COM_SLEEP
		|| com == COM_REST
		|| com == COM_WAKE
		|| com == COM_STAND
		|| com == COM_SIT
		|| com == COM_LIE
		|| com == COM_LOOK
		|| com == COM_PLAY
		) {
	  handle_command(body->Parent(), line.c_str());
	  spos = skip_line(script, spos);
	  }

	//Player commands Acid shares with TBA, requiring arguments
	else if(com == COM_SAY
		|| com == COM_EMOTE
		|| com == COM_LOCK
		|| com == COM_UNLOCK
		|| com == COM_OPEN
		|| com == COM_CLOSE
		|| com == COM_GET
		) {
	  size_t stuff = line.find_first_of(" ");
	  if(stuff != string::npos) {
	    stuff = line.find_first_not_of(" \t\r\n", stuff);
	    }
	  if(stuff != string::npos) {
	    handle_command(body->Parent(), line.c_str());
	    }
	  else {
	    fprintf(stderr, CRED "#%d Error: Told just '%s'\n" CNRM,
		body->Skill("TBAScript"), line.c_str()
		);
	    Disable();
	    return;
	    }
	  spos = skip_line(script, spos);
	  }

	else if((!strncasecmp(line.c_str(), "mecho ", 6))) {
	  size_t start = line.find_first_not_of(" \t\r\n", 6);
	  if(room && start != string::npos) {
	    string mes = line.substr(start);
	    trim_string(mes);
	    mes += "\n";
	    room->SendOut(0, 0, mes.c_str(), mes.c_str(), NULL, NULL);
	    }
	  spos = skip_line(script, spos);
	  }

	else if(!strncasecmp(line.c_str(), "send_actor ", 11)) {
	  if(curvars.count("actor") && curvars["actor"]) {
	    string mes = line.c_str() + 11;
	    size_t end = mes.find_first_of("\n\r");
	    if(end != string::npos) mes = mes.substr(0, end);
	    trim_string(mes);
	    mes += "\n";
	    curvars["actor"]->Send(0, 0, mes.c_str());
	    }
	  spos = skip_line(script, spos);
	  }

	else if(!strncasecmp(line.c_str(), "wdamage ", 8)) {
//	  fprintf(stderr, CGRN "#%d Debug: WDamage '%s'\n" CNRM,
//		body->Skill("TBAScript"), line.c_str()
//		);
	  int pos=0;
	  int dam = 0;
	  char buf[256];
	  if(sscanf(line.c_str() + 8, " %s %n", buf, &pos) >= 1) {
	    if(!strcasecmp(buf, "all")) { strcpy(buf, "everyone"); }
	    dam = tba_eval(line.c_str() + 8 + pos);
	    if(dam > 0) dam = (dam + 180) / 100;
	    if(dam < 0) dam = (dam - 180) / 100;
	    }
	  list<Object*> options = room->Contents();
	  list<Object*>::iterator opt = options.begin();
	  for(; opt != options.end(); ++opt) {
	    if((*opt)->Matches(buf)) {
	      if(dam > 0) {
		(*opt)->HitMent(1000, dam, 0);
//		fprintf(stderr, CGRN "#%d Debug: WDamage '%s', %d\n" CNRM,
//			body->Skill("TBAScript"), (*opt)->Name(), dam
//			);
		}
	      else if(dam < 0) {
		(*opt)->HealStun(((-dam)+1)/2);
		(*opt)->HealPhys(((-dam)+1)/2);
//		fprintf(stderr, CGRN "#%d Debug: WHeal '%s', %d\n" CNRM,
//			body->Skill("TBAScript"), (*opt)->Name(), ((-dam)+1)/2
//			);
		}
	      }
	    }
	  spos = skip_line(script, spos);
	  }

	else if(!strncasecmp(line.c_str(), "echoaround ", 11)) {
	  Object *targ = NULL;
	  char mes[256] = "";
	  sscanf(line.c_str()+11, " OBJ:%p %255[^\n\r]", &targ, mes);
	  mes[strlen(mes)] = '\n';
	  Object *troom = targ;
	  while(troom && troom->Skill("TBARoom") == 0) troom = troom->Parent();
	  if(troom && targ) troom->SendOut(0, 0, mes, "", targ, NULL);
	  spos = skip_line(script, spos);
	  }

//	else if(!strncasecmp(line.c_str(), "door ", 6)) {
//	  spos = skip_line(script, spos);
//	  }

	else if(!strncasecmp(line.c_str(), "transport ", 10)) {
	  int dnum;
	  char buf[256];
	  if(sscanf(line.c_str() + 10, "%s %d", buf, &dnum) != 2) {
	    if(!strcasecmp(buf, "all")) { strcpy(buf, "everyone"); }
	    }
	  dnum += 1000000;
	  Object *dest = curvars["self"];
	  while(dest->Parent()->Parent()) {
	    dest = dest->Parent();
	    }
	  list<Object*> options = dest->Contents();
	  list<Object*>::iterator opt = options.begin();
	  dest = NULL;
	  for(; opt != options.end(); ++opt) {
	    if((*opt)->Skill("TBARoom") == dnum) {
	      dest = (*opt);
	      break;
	      }
	    }
	  options = room->Contents();
	  opt = options.begin();
	  for(; opt != options.end(); ++opt) {
	    if((*opt)->Matches(buf)) {
	      (*opt)->Travel(dest);
	      }
	    }
//	  fprintf(stderr, CGRN "#%d Debug: Transport line: '%s'\n" CNRM,
//		body->Skill("TBAScript"), line.c_str()
//		);
	  spos = skip_line(script, spos);
	  }

	else if(!strncasecmp(line.c_str(), "load ", 5)) {
	  int vnum, params, type, loc = 0, mask = 0;
	  char buf[256] = "";
	  char targ[256] = "";
	  char where[256] = "";
	  Object *src = room;
	  Object *dest = room;
	  Object *item = NULL;
	  params = sscanf(line.c_str() + 5, " %s %d %s %s",
		buf, &vnum, targ, where);
	  type = tolower(buf[0]);
	  if((params != 2 && params != 4) || (type != 'o' && type != 'm')) {
	    fprintf(stderr, CRED "#%d Error: Gibberish script line '%s'\n" CNRM,
		body->Skill("TBAScript"), line.c_str()
		);
	    Disable();
	    return;
	    }
	  while(src->Parent()->Parent()) src = src->Parent();
	  if(type == 'o') {
	    src = src->PickObject("TBAMUD Object Room", LOC_NINJA|LOC_INTERNAL);
	    list<Object*> options = src->Contents();
	    list<Object*>::iterator opt = options.begin();
	    for(; opt != options.end(); ++opt) {
	      if((*opt)->Skill("TBAObject") == vnum + 1000000) {
		item = new Object(*(*opt));
		break;
		}
	      }
	    }
	  else if(type == 'm') {
	    src = src->PickObject("TBAMUD MOB Room", LOC_NINJA|LOC_INTERNAL);
	    list<Object*> options = src->Contents();
	    list<Object*>::iterator opt = options.begin();
	    for(; opt != options.end(); ++opt) {
	      if((*opt)->Skill("TBAMOB") == vnum + 1000000) {
		item = new Object(*(*opt));
		break;
		}
	      }
	    }
	  if(item == NULL) {
	    fprintf(stderr, CRED "#%d Error: Failed to find item '%s'\n" CNRM,
		body->Skill("TBAScript"), line.c_str()
		);
	    Disable();
	    return;
	    }
	  if(params > 2) {
	    fprintf(stderr, CGRN "#%d Debug: (%s) '%s'\n" CNRM,
		body->Skill("TBAScript"), targ, line.c_str());
	    dest = dest->PickObject(targ, LOC_NINJA|LOC_INTERNAL);
	    }
	  if(!dest) {
	    fprintf(stderr, CRED "#%d Error: Can't find target in '%s'\n" CNRM,
		body->Skill("TBAScript"), line.c_str()
		);
	    Disable();
	    return;
	    }
	  if(strcmp("rfinger", where) == 0 || strcmp("1", where) == 0) {
	    mask = item->Skill("Wearable on Right Finger");
	    loc = ACT_WEAR_RFINGER;
	    }
	  else if(strcmp("lfinger", where) == 0 || strcmp("2", where) == 0) {
	    mask = item->Skill("Wearable on Left Finger");
	    loc = ACT_WEAR_LFINGER;
	    }
	  else if(strcmp("neck1", where) == 0 || strcmp("3", where) == 0) {
	    mask = item->Skill("Wearable on Neck");
	    loc = ACT_WEAR_NECK;
	    }
	  else if(strcmp("neck2", where) == 0 || strcmp("4", where) == 0) {
	    mask = item->Skill("Wearable on Neck");
	    loc = ACT_WEAR_NECK;
	    }
	  else if(strcmp("body", where) == 0 || strcmp("5", where) == 0) {
	    mask = item->Skill("Wearable on Chest");
	    loc = ACT_WEAR_CHEST;
	    }
	  else if(strcmp("head", where) == 0 || strcmp("6", where) == 0) {
	    mask = item->Skill("Wearable on Head");
	    loc = ACT_WEAR_HEAD;
	    }
	  else if(strcmp("legs", where) == 0 || strcmp("7", where) == 0) {
	    mask = item->Skill("Wearable on Left Leg");
	    loc = ACT_WEAR_LLEG;
	    }
	  else if(strcmp("feet", where) == 0 || strcmp("8", where) == 0) {
	    mask = item->Skill("Wearable on Left Foot");
	    loc = ACT_WEAR_LFOOT;
	    }
	  else if(strcmp("hands", where) == 0 || strcmp("9", where) == 0) {
	    mask = item->Skill("Wearable on Left Hand");
	    loc = ACT_WEAR_LHAND;
	    }
	  else if(strcmp("arms", where) == 0 || strcmp("10", where) == 0) {
	    mask = item->Skill("Wearable on Left Arm");
	    loc = ACT_WEAR_LARM;
	    }
	  else if(strcmp("shield", where) == 0 || strcmp("11", where) == 0) {
	    mask = item->Skill("Wearable on Shield");
	    loc = ACT_WEAR_SHIELD;
	    }
	  else if(strcmp("about", where) == 0 || strcmp("12", where) == 0) {
	    mask = item->Skill("Wearable on Left Shoulder");
	    loc = ACT_WEAR_LSHOULDER;
	    }
	  else if(strcmp("waist", where) == 0 || strcmp("13", where) == 0) {
	    mask = item->Skill("Wearable on Waist");
	    loc = ACT_WEAR_WAIST;
	    }
	  else if(strcmp("rwrist", where) == 0 || strcmp("14", where) == 0) {
	    mask = item->Skill("Wearable on Right Wrist");
	    loc = ACT_WEAR_RWRIST;
	    }
	  else if(strcmp("lwrist", where) == 0 || strcmp("15", where) == 0) {
	    mask = item->Skill("Wearable on Left Wrist");
	    loc = ACT_WEAR_LWRIST;
	    }
	  else if(strcmp("wield", where) == 0 || strcmp("16", where) == 0) {
	    mask = item->Skill("Wearable on ");
	    loc = ACT_WIELD;
	    }
	  else if(strcmp("light", where) == 0 || strcmp("0", where) == 0) {
	    loc = ACT_HOLD;
	    }
	  else if(strcmp("hold", where) == 0 || strcmp("17", where) == 0) {
	    loc = ACT_HOLD;
	    }
	  else if(strcmp("inv", where) == 0 || strcmp("18", where) == 0) {
	    }
	  else if(params > 2) {
	    fprintf(stderr, CRED "#%d Error: Unsupported dest '%s'\n" CNRM,
		body->Skill("TBAScript"), line.c_str()
		);
	    Disable();
	    return;
	    }
	  item->SetParent(dest);
	  if(loc != 0 && loc != ACT_HOLD) {
	    dest->Drop(item);
	    fprintf(stderr, CYEL "#%d Warning: Didn't wear '%s'\n" CNRM,
		body->Skill("TBAScript"), line.c_str()
		);
	    }
	  else if(loc == ACT_HOLD) {
	    if(body->ActTarg(ACT_HOLD) == NULL
		|| body->ActTarg(ACT_HOLD) == body->ActTarg(ACT_WIELD)
		|| body->ActTarg(ACT_HOLD) == body->ActTarg(ACT_WEAR_SHIELD)) {
	      body->AddAct(ACT_HOLD, item);
	      }
	    else {
	      dest->Drop(item);
	      }
	    }
	  else if(dest != room) {
	    dest->StashOrDrop(item);
	    }
	  spos = skip_line(script, spos);
	  }

	else if(!strncasecmp(line.c_str(), "case ", 5)) {
	  //Ignore these, as we only hit them here when when running over them
	  spos = skip_line(script, spos);
	  }
	else if(!strncasecmp(line.c_str(), "default", 7)) {
	  //Ignore these, as we only hit them here when when running over them
	  spos = skip_line(script, spos);
	  }
	else if(!strncasecmp(line.c_str(), "end", 3)) {
	  //Ignore these, as we only hit them here when we're running inside if
	  spos = skip_line(script, spos);
	  }
	else if(!strncasecmp(line.c_str(), "done", 4)) {
	  //Means we should be within a while(), so return to main.
	  return;
	  }
	else if(!strncasecmp(line.c_str(), "return ", 7)) {
	  int retval = tba_eval(line.c_str()+7);
	  if(retval == 0) {
	    status = 1;			//Return with special state
	    }
	  Disable();
	  return;
	  }
	else if(!strncasecmp(line.c_str(), "halt", 4)) {
	  Disable();
	  return;
	  }
	else {		//Silently ignore the rest for now!  FIXME: Error mes.
	  fprintf(stderr, CRED "#%d Error: Gibberish script line '%s'\n" CNRM,
		body->Skill("TBAScript"), line.c_str()
		);
	  Disable();
	  return;
	  }
	}
      if(stype & 2) {		//Random Triggers
	int chance = body->Skill("TBAScriptNArg");	//Percent Chance
	if(chance > 0) {
	  int delay = 13000;	//Next try in 13 seconds.
	  while(delay < 13000000 && (rand() % 100) >= chance) delay += 13000;
	  spos = 0;		//We never die!
	  Suspend(delay);	//We'll be back!
	  return;
	  }
	}
      }
    Disable();
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

void Mind::SetSpecialPrompt(const char *newp) {
  prompt = string(newp);
  UpdatePrompt();
  }

const char *Mind::SpecialPrompt() {
  return prompt.c_str();
  }

Mind *new_mind(int tp, Object *obj, Object *obj2, string text) {
  Mind *m = NULL;
  if(recycle_bin.size() > 0) {
    m = recycle_bin.front();
    recycle_bin.pop_front();
    m->ClearStatus();		//Clear special state!
//    fprintf(stderr, "Suspending(%p)\n", m);
    }
  else {
    m = new Mind();
    }
  if(tp == MIND_TBATRIG && obj) {
    m->SetTBATrigger(obj, obj2, text);
    }
  else if(obj) {
    m->Attach(obj);
    }
  return m;
  }

int new_trigger(int msec, Object *obj, Object *tripper, string text) {
  if((!obj) || (!(obj->Parent()))) return 0;

  Mind *m = new_mind(MIND_TBATRIG, obj, tripper, text);
  if(msec == 0) {
    m->Think(1);
    if(m->Status()) return 1;		//Catch non-default term state.
    }
  else {
    m->Suspend(msec);
    }
  return 0;
  }

list<pair<int, Mind*> > Mind::waiting;
void Mind::Suspend(int msec) {
//  fprintf(stderr, "Suspening(%p)\n", this);
  waiting.push_back(make_pair(msec, this));
  }

void Mind::Disable() {
//  fprintf(stderr, "Disabled(%p)\n", this);
  if(type == MIND_REMOTE) close_socket(pers);
  type = MIND_MORON;
  Unattach();
  if(log >= 0) close(log);
  log = -1;
  list<pair<int, Mind*> >::iterator cur = waiting.begin();
  while(cur != waiting.end()) {
    list<pair<int, Mind*> >::iterator tmp = cur;
    ++cur;	//Inc cur first, in case I erase tmp.
    if(tmp->second == this) {
      waiting.erase(tmp);
      }
    }
  svars.clear();		//Reset all variables
  ovars.clear();
  recycle_bin.push_back(this);	//Ready for re-use
  }

void Mind::Resume(int passed) {
  list<pair<int, Mind*> >::iterator cur = waiting.begin();
  while(cur != waiting.end()) {
    list<pair<int, Mind*> >::iterator tmp = cur;
    ++cur;	//Inc cur first, in case I erase tmp.
    if(tmp->first <= passed) {
      Mind *mind = tmp->second;
      waiting.erase(tmp);
//      fprintf(stderr, "Resuming(%p)\n", mind);
      mind->Think(0);
      }
    else {
      tmp->first -= passed;
      }
    }
  }

int Mind::Status() const {
  return status;
  }

void Mind::ClearStatus() {
  status = 0;
  }
