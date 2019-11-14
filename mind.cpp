#include <map>
#include <string>

#include <arpa/telnet.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "color.hpp"
#include "commands.hpp"
#include "mind.hpp"
#include "net.hpp"
#include "object.hpp"
#include "player.hpp"
#include "utils.hpp"

std::list<Mind*> recycle_bin;

static const char* bstr[2] = {"0", "1"};

static std::string itos(int val) {
  char buf[256];
  sprintf(buf, "%d", val);
  return std::string(buf);
}

static int tba_bitvec(const std::string& val) {
  int ret = atoi(val.c_str());
  if (ret == 0) { // Works fine for "0" too
    for (size_t idx = 0; idx < val.length(); ++idx) {
      ret |= 1 << ((val[idx] & 31) - 1);
    }
  }
  return ret;
}

std::string Mind::TBAComp(std::string expr) {
  size_t end = expr.find_first_of("\n\r");
  if (end != std::string::npos)
    expr = expr.substr(0, end);
  trim_string(expr);
  if (0
      //	|| body->Skill("TBAScript") == 5000099
      //	|| strcasestr(expr.c_str(), "tea")
      //	|| strcasestr(expr.c_str(), "wake")
  ) {
    fprintf(stderr, "Expr: '%s'\n", expr.c_str());
  }

  if (expr[0] == '(') {
    size_t cls = expr.find(')');
    size_t opn = expr.find('(', 1);
    while (cls != std::string::npos && opn != std::string::npos && opn < cls) {
      cls = expr.find(')', cls + 1);
      opn = expr.find('(', opn + 1);
    }
    if (cls == std::string::npos)
      return TBAComp(expr.substr(1));
    expr = TBAComp(expr.substr(1, cls - 1)) + " " + expr.substr(cls + 1);
    trim_string(expr);
  }

  size_t skip = 0;
  if (expr[0] == '-')
    skip = 1; // Skip Leading "-"
  if (expr[0] == '!')
    skip = 1; // Skip Leading "!"
  size_t op = expr.find_first_of("|&=!<>/-+*", skip);
  while (op != std::string::npos && expr[op] == '-' && isalpha(expr[op - 1]) &&
         isalpha(expr[op + 1])) {
    op = expr.find_first_of("|&=!<>/-+*", op + 1); // Skip Hyphens
  }
  if (op == std::string::npos)
    return expr; // No ops, just val

  int oper = 0; // Positive for 2-char ops, negative for 1-char
  int weak = 0; // Reverse-Precedence!  Hack!!!
  if (!strncmp(expr.c_str() + op, "/=", 2)) {
    oper = 1;
  } else if (!strncmp(expr.c_str() + op, "==", 2)) {
    oper = 2;
  } else if (!strncmp(expr.c_str() + op, "!=", 2)) {
    oper = 3;
  } else if (!strncmp(expr.c_str() + op, "<=", 2)) {
    oper = 4;
  } else if (!strncmp(expr.c_str() + op, ">=", 2)) {
    oper = 5;
  } else if (!strncmp(expr.c_str() + op, "&&", 2)) {
    oper = 6;
    weak = 1;
  } else if (!strncmp(expr.c_str() + op, "||", 2)) {
    oper = 7;
    weak = 1;
  } else if (!strncmp(expr.c_str() + op, "<", 1)) {
    oper = -1;
  } else if (!strncmp(expr.c_str() + op, ">", 1)) {
    oper = -2;
  } else if (!strncmp(expr.c_str() + op, "+", 1)) {
    oper = -3;
  } else if (!strncmp(expr.c_str() + op, "-", 1)) {
    oper = -4;
  } else if (!strncmp(expr.c_str() + op, "*", 1)) {
    oper = -5;
  } else if (!strncmp(expr.c_str() + op, "/", 1)) {
    oper = -6;
  }

  if (oper != 0) {
    std::string arg1 = expr.substr(0, op);
    trim_string(arg1);
    if (oper > 0)
      expr = expr.substr(op + 2); // 2-char
    else
      expr = expr.substr(op + 1); // 1-char
    trim_string(expr);
    std::string arg2;
    if (expr[0] == '(') {
      size_t cls = expr.find(")"); // FIXME: Nested
      if (cls == std::string::npos)
        arg2 = TBAComp(expr.substr(1));
      else {
        arg2 = TBAComp(expr.substr(1, cls - 1));
        expr.replace(0, cls + 1, arg2);
      }
    }
    if (weak) {
      arg2 = TBAComp(expr);
      expr = "";
    } else {
      arg2 = expr;
      op = expr.find_first_of("|&=!<>/-+*)\n\r");
      if (op != std::string::npos) {
        arg2 = expr.substr(0, op);
        expr = expr.substr(op);
      } else {
        expr = "";
      }
    }
    trim_string(arg2);

    int res = 0;
    std::string comp = "0";
    if (oper == 1 && strcasestr(arg1.c_str(), arg2.c_str()))
      comp = "1";
    else if (oper == 2 && (!strcasecmp(arg1.c_str(), arg2.c_str())))
      comp = "1";
    else if (oper == 3 && strcasecmp(arg1.c_str(), arg2.c_str()))
      comp = "1";
    else if (oper == 4 && (TBAEval(arg1) <= TBAEval(arg2)))
      comp = "1";
    else if (oper == 5 && (TBAEval(arg1) >= TBAEval(arg2)))
      comp = "1";
    else if (oper == -1 && (TBAEval(arg1) < TBAEval(arg2)))
      comp = "1";
    else if (oper == -2 && (TBAEval(arg1) > TBAEval(arg2)))
      comp = "1";
    else if (oper == 6 && (TBAEval(arg1) && TBAEval(arg2)))
      comp = "1";
    else if (oper == 7 && (TBAEval(arg1) || TBAEval(arg2)))
      comp = "1";
    else if (oper == -3)
      res = TBAEval(arg1) + TBAEval(arg2);
    else if (oper == -4)
      res = TBAEval(arg1) - TBAEval(arg2);
    else if (oper == -5)
      res = TBAEval(arg1) * TBAEval(arg2);
    else if (oper == -6) { // Protect from div by zero
      int val2 = TBAEval(arg2);
      res = TBAEval(arg1);
      if (val2 != 0)
        res /= val2;
    }

    if (oper <= -3) { // Non-Boolean - actual numeric value
      comp = itos(res);
      // fprintf(stderr, "RES: %s\n", buf);
    }

    if (expr != "") {
      expr = comp + " " + expr;
      return TBAComp(expr);
    }
    return comp;
  }

  return "0";
}

int Mind::TBAEval(std::string expr) {
  std::string base = TBAComp(expr);
  trim_string(base);
  if (0
      //	|| body->Skill("TBAScript") == 5000099
      //	|| strcasestr(expr.c_str(), "tea")
      //	|| strcasestr(expr.c_str(), "wake")
  ) {
    fprintf(stderr, "Base: '%s'\n", base.c_str());
  }
  if (base.length() == 0)
    return 0; // Null
  if (base.length() == 1 && base[0] == '!')
    return 1; //! Null

  int ret = 0, len = 0;
  sscanf(base.c_str(), " %d %n", &ret, &len);
  if (len == int(base.length()))
    return ret; // Numeric
  sscanf(base.c_str(), " ! %d %n", &ret, &len);
  if (len == int(base.length()))
    return !ret; //! Numeric

  Object* holder;
  sscanf(base.c_str(), " OBJ:%p %n", &holder, &len);
  if (len == int(base.length()))
    return (holder != nullptr); // Object
  sscanf(base.c_str(), " ! OBJ:%p %n", &holder, &len);
  if (len == int(base.length()))
    return (holder == nullptr); //! Object

  if (base[0] == '!')
    return 0; //! Non-Numberic, Non-nullptr, Non-Object
  return 1; // Non-Numberic, Non-nullptr, Non-Object
}

std::map<std::string, std::string> Mind::cvars;

void Mind::Init() {
  body = nullptr;
  player = nullptr;
  ovars.clear();
  svars.clear();
}

Mind::Mind() {
  type = MIND_MORON;
  Init();
  pers = 0;
  log = -1;
}

Mind::Mind(int fd) {
  type = MIND_MORON;
  Init();
  log = -1;
  SetRemote(fd);
}

Mind::Mind(int fd, int l) {
  type = MIND_MORON;
  Init();
  log = l;
  SetRemote(fd);
}

Mind::~Mind() {
  if (type == MIND_REMOTE)
    close_socket(pers);
  type = MIND_MORON;
  Unattach();
  if (log >= 0)
    close(log);
}

void Mind::SetRemote(int fd) {
  type = MIND_REMOTE;
  pers = fd;
  char buf[256];

  if (log >= 0)
    return;

  static unsigned long lognum = 0;
  struct stat st;
  sprintf(buf, "logs/%.8lX.log%c", lognum, 0);
  while (!stat(buf, &st)) {
    ++lognum;
    sprintf(buf, "logs/%.8lX.log%c", lognum, 0);
  }
  log = open(buf, O_WRONLY | O_CREAT | O_TRUNC | O_SYNC, S_IRUSR | S_IWUSR);
}

void Mind::SetMob() {
  type = MIND_MOB;
  pers = fileno(stderr);
}

void Mind::SetTBATrigger(Object* tr, Object* tripper, Object* targ, std::string text) {
  if ((!tr) || (!(tr->Parent())))
    return;

  type = MIND_TBATRIG;
  if (cvars.size() < 1) {
    cvars["damage"] = "wdamage";
    cvars["echo"] = "mecho";
    cvars["send"] = "send";
    cvars["force"] = "force";
    cvars["echoaround"] = "echoaround";
    cvars["teleport"] = "transport";
    cvars["zoneecho"] = "zoneecho";
    cvars["asound"] = "asound";
    cvars["door"] = "door";
    cvars["load"] = "load";
    cvars["purge"] = "purge";
    cvars["at"] = "at";
    cvars[""] = "%";
  }
  svars = cvars;
  pers = fileno(stderr);
  Attach(tr);
  spos_s.clear();
  spos_s.push_front(0);
  script = body->LongDesc();
  script += "\n";
  if (tripper)
    ovars["actor"] = tripper;

  int stype = body->Skill("TBAScriptType");
  if (stype & 0x0000008) { //-SPEECH Triggers
    svars["speech"] = text;
  }
  if ((stype & 0x4000040) == 0x4000040 // ROOM-ENTER Triggers
      || (stype & 0x1000040) == 0x1000040 // MOB-GREET Triggers
      || stype & 0x0010000) { //*-LEAVE Triggers
    svars["direction"] = text;
  }
  if ((stype & 0x4000080) == 0x4000080) { // ROOM-DROP Triggers
    ovars["object"] = targ;
  }
  if (stype & 0x0000004) { //-COMMAND Triggers
    size_t part = text.find_first_of(" \t\n\r");
    if (part == std::string::npos)
      svars["cmd"] = text;
    else {
      svars["cmd"] = text.substr(0, part);
      part = text.find_first_not_of(" \t\n\r", part);
      if (part != std::string::npos)
        svars["arg"] = text.substr(part);
    }
  }
}

void Mind::SetTBAMob() {
  type = MIND_TBAMOB;
  pers = fileno(stderr);
}

void Mind::SetNPC() {
  type = MIND_NPC; // FIXME: Implement these!
  pers = fileno(stderr);
}

void Mind::SetSystem() {
  type = MIND_SYSTEM;
  pers = fileno(stderr);
}

static const char* sevs_p[] = {"-", "L", "L", "M", "M", "M", "S", "S", "S", "S", "D"};
static const char* sevs_s[] = {"-", "l", "l", "m", "m", "m", "s", "s", "s", "s", "u"};
void Mind::UpdatePrompt() {
  static char buf[65536];
  if (!Owner()) {
    SetPrompt(pers, "Player Name: ");
    if (pname.length() >= 1)
      SetPrompt(pers, "Password: ");
  } else if (prompt.length() > 0) {
    sprintf(buf, "%s> %c", prompt.c_str(), 0);
    SetPrompt(pers, buf);
  } else if (Body()) {
    sprintf(
        buf,
        "[%s][%s] %s> %c",
        sevs_p[MIN(10, Body()->Phys())],
        sevs_s[MIN(10, Body()->Stun())],
        Body()->ShortDesc(),
        0);
    SetPrompt(pers, buf);
  } else
    SetPrompt(pers, "No Character> ");
}

void Mind::Attach(Object* bod) {
  body = bod;
  if (body)
    body->Attach(this);
}

void Mind::Unattach() {
  Object* bod = body;
  body = nullptr;
  if (bod)
    bod->Unattach(this);
}

void Mind::Send(const char* mes) {
  if (type == MIND_REMOTE) {
    SendOut(pers, mes);
  } else if (type == MIND_MOB) {
    // Think(); //Reactionary actions (NO!).
  } else if (type == MIND_TBAMOB) {
    // HELPER TBA Mobs
    if (body && body->Parent() && (body->Skill("TBAAction") & 4096) // Helpers
        && ((body->Skill("TBAAction") & 2) == 0) // NON-SENTINEL
        && body->Stun() < 6 // I'm not stunned
        && body->Phys() < 6 // I'm not injured
        && (!body->IsAct(ACT_SLEEP)) // I'm not asleep
        && (!body->IsAct(ACT_REST)) // I'm not resting
        && (!body->IsAct(ACT_FIGHT)) // I'm not already fighting
    ) {
      if ((!strncasecmp(mes, "From ", 5)) &&
          (strcasestr(mes, " you hear someone shout '") != nullptr) &&
          ((strstr(mes, "HELP")) || (strstr(mes, "ALARM")))) {
        char buf[256] = "                                               ";
        sscanf(mes + 4, "%128s", buf);

        Object* door = body->PickObject(buf, LOC_NEARBY);

        if (door && door->ActTarg(ACT_SPECIAL_LINKED) &&
            door->ActTarg(ACT_SPECIAL_LINKED)->Parent() &&
            TBACanWanderTo(door->ActTarg(ACT_SPECIAL_LINKED)->Parent())) {
          char buf[256] = "enter                                          ";
          sscanf(mes + 4, "%128s", buf + 6);
          body->BusyFor(500, buf);
        }
        return;
      }
    }
    Think(); // Reactionary actions (HACK!).
  } else if (type == MIND_SYSTEM) {
    std::string newmes = "";
    if (body)
      newmes += body->ShortDesc();
    newmes += ": ";
    newmes += mes;

    std::string::iterator chr = newmes.begin();
    for (; chr != newmes.end(); ++chr) {
      if ((*chr) == '\n' || (*chr) == '\r')
        (*chr) = ' ';
    }
    newmes += "\n";

    write(pers, newmes.c_str(), newmes.length());
  }
}

void Mind::SendRaw(const char* mes) {
  SendOut(pers, mes);
}

static char buf[65536];
void Mind::SendF(const char* mes, ...) {
  memset(buf, 0, 65536);
  va_list stuff;
  va_start(stuff, mes);
  vsprintf(buf, mes, stuff);
  va_end(stuff);

  Send(buf);
}

void Mind::SendRawF(const char* mes, ...) {
  memset(buf, 0, 65536);
  va_list stuff;
  va_start(stuff, mes);
  vsprintf(buf, mes, stuff);
  va_end(stuff);

  SendRaw(buf);
}

void Mind::SetPName(std::string pn) {
  pname = pn;
  if (player_exists(pname))
    SendF("%c%c%cReturning player - welcome back!\n", IAC, WILL, TELOPT_ECHO);
  else
    SendF(
        "%c%c%cNew player (%s) - enter SAME new password twice.\n",
        IAC,
        WILL,
        TELOPT_ECHO,
        pname.c_str());
}

void Mind::SetPPass(std::string ppass) {
  if (player_exists(pname)) {
    player = player_login(pname, ppass);
    if (player == nullptr) {
      if (player_exists(pname))
        SendF("%c%c%cName and/or password is incorrect.\n", IAC, WONT, TELOPT_ECHO);
      else
        SendF("%c%c%cPasswords do not match - try again.\n", IAC, WONT, TELOPT_ECHO);
      pname = "";
      return;
    }
  } else if (!player) {
    new Player(pname, ppass);
    SendF("%c%c%cEnter password again for verification.\n", IAC, WILL, TELOPT_ECHO);
    return;
  }

  SendRawF("%c%c%c", IAC, WONT, TELOPT_ECHO);
  svars = player->Vars();
  player->Room()->SendDesc(this);
  player->Room()->SendContents(this);
}

void Mind::SetPlayer(std::string pn) {
  if (player_exists(pn)) {
    pname = pn;
    player = get_player(pname);
    svars = player->Vars();
  }
}

std::string Mind::Tactics(int phase) {
  if (type == MIND_TBAMOB) {
    // NON-HELPER and NON-AGGRESSIVE TBA Mobs (Innocent MOBs)
    if (body && (body->Skill("TBAAction") & 4128) == 0) {
      if (phase == -1) {
        return "call HELP; attack";
      }
    }
  }
  return "attack";
}

void Mind::TBAVarSub(std::string& line) {
  size_t cur = line.find('%');
  size_t end;
  while (cur != std::string::npos) {
    end = line.find_first_of("%. \t", cur + 1);
    if (end == std::string::npos)
      end = line.length();
    if (0
        //	|| body->Skill("TBAScript") == 5000099
        //	|| line.find("eval loc ") != std::string::npos
        //	|| line.find("set first ") != std::string::npos
        //	|| line.find("exclaim") != std::string::npos
        //	|| line.find("speech") != std::string::npos
    ) {
      fprintf(stderr, CGRN "#%d Debug: '%s'\n" CNRM, body->Skill("TBAScript"), line.c_str());
    }
    std::string vname = line.substr(cur + 1, end - cur - 1);
    Object* obj = nullptr;
    std::string val = "";
    int is_obj = 0;
    if (ovars.count(vname) > 0) {
      obj = ovars[vname];
      is_obj = 1;
    } else if (svars.count(vname) > 0) {
      val = svars[vname];
    } else if (!strncasecmp(line.c_str() + cur, "%time.hour%", 11)) {
      Object* world = body->World();
      if (world->Skill("Day Time") && world->Skill("Day Length")) {
        int hour = world->Skill("Day Time");
        hour *= 24;
        hour /= world->Skill("Day Length");
        val = itos(hour);
      }
      end = line.find_first_of("% \t", cur + 1); // Done.  Replace All.
    } else if (!strncasecmp(line.c_str() + cur, "%random.char%", 13)) {
      std::list<Object*> others;
      if (ovars["self"]->HasSkill("TBARoom")) {
        others = ovars["self"]->PickObjects("everyone", LOC_INTERNAL);
      } else if (ovars["self"]->Owner()) {
        others = ovars["self"]->Owner()->PickObjects("everyone", LOC_NEARBY);
      } else {
        others = ovars["self"]->PickObjects("everyone", LOC_NEARBY);
      }
      if (others.size() > 0) {
        int num = rand() % others.size();
        std::list<Object*>::iterator item = others.begin();
        for (; num > 0; --num) {
          ++item;
        }
        obj = (*item);
        if (0
            //		|| script.find("%damage% %actor% -%actor.level%") !=
            // std::string::npos
        ) {
          fprintf(stderr, CGRN "#%d Random: '%s'\n" CNRM, body->Skill("TBAScript"), obj->Name());
        }
      } else {
        obj = nullptr;
      }
      is_obj = 1;
      end = line.find_first_of("% \t", cur + 1); // Done.  Replace All.
    } else if (!strncasecmp(line.c_str() + cur, "%random.dir%", 12)) {
      Object* room = ovars["self"];
      while (room && room->Skill("TBARoom") == 0)
        room = room->Parent();
      if (room) {
        std::set<Object*> options;
        options.insert(room->PickObject("north", LOC_INTERNAL));
        options.insert(room->PickObject("south", LOC_INTERNAL));
        options.insert(room->PickObject("east", LOC_INTERNAL));
        options.insert(room->PickObject("west", LOC_INTERNAL));
        options.insert(room->PickObject("up", LOC_INTERNAL));
        options.insert(room->PickObject("down", LOC_INTERNAL));
        options.erase(nullptr);
        if (options.size() > 0) {
          int num = rand() % options.size();
          std::set<Object*>::iterator item = options.begin();
          for (; num > 0; --num) {
            ++item;
          }
          val = (*item)->ShortDesc();
        }
      }
      end = line.find_first_of("% \t", cur + 1); // Done.  Replace All.
    } else if (!strncasecmp(line.c_str() + cur, "%random.", 8)) {
      if (isdigit(line[cur + 8])) {
        size_t vend = line.find_first_not_of("0123456789", cur + 8);
        if (vend != std::string::npos && line[vend] == '%') {
          val = itos((rand() % atoi(line.c_str() + cur + 8)) + 1);
        }
      }
      end = line.find_first_of("% \t", cur + 1); // Done.  Replace All.
    } else { // Undefined base var
      end = line.find_first_of("% \t", cur + 1); // Done.  Replace All.
    }
    while (line[end] == '.') {
      size_t start = end + 1;
      end = line.find_first_of("%. \t", start);
      if (end == std::string::npos)
        end = line.length();
      std::string field = line.substr(start, end - start);
      if (is_obj) {
        if (!strcasecmp(field.c_str(), "id")) {
          // obj is already right
        } else if (!strcasecmp(field.c_str(), "vnum")) {
          int vnum = 0;
          if (obj) {
            obj->Skill("TBAMOB");
            if (vnum < 1)
              vnum = obj->Skill("TBAObject");
            if (vnum < 1)
              vnum = obj->Skill("TBARoom");
            if (vnum > 0)
              vnum %= 1000000; // Convert from Acid number
          }
          val = itos(vnum);
          obj = nullptr;
          is_obj = 0;
        } else if (!strcasecmp(field.c_str(), "gold")) {
          int gold = 0;
          if (obj) {
            std::list<Object*> pay = obj->PickObjects("all a gold piece", LOC_INTERNAL);
            std::list<Object*>::const_iterator coin;
            for (coin = pay.begin(); coin != pay.end(); ++coin) {
              gold += (*coin)->Quantity();
            }
          }
          val = itos(gold);
          obj = nullptr;
          is_obj = 0;
        } else if (!strcasecmp(field.c_str(), "type")) {
          val = "OTHER";
          if (obj) {
            if (obj->HasSkill("Conatiner"))
              val = "CONTAINER";
            else if (obj->HasSkill("Liquid Source"))
              val = "FOUNTAIN";
            else if (obj->HasSkill("Liquid Container"))
              val = "LIQUID CONTAINER";
            else if (obj->HasSkill("Ingestible") <= 0)
              val = "FOOD";
            else if (obj->Value() <= 0)
              val = "TRASH";
            // FIXME: More Types!
          }
          obj = nullptr;
          is_obj = 0;
        } else if (!strcasecmp(field.c_str(), "cost_per_day")) {
          val = "0";
          obj = nullptr;
          is_obj = 0;
        } else if (!strcasecmp(field.c_str(), "cost")) {
          val = "";
          if (obj)
            val = itos(obj->Value());
          obj = nullptr;
          is_obj = 0;
        } else if (!strcasecmp(field.c_str(), "count")) {
          val = "";
          if (obj)
            val = itos(obj->Quantity());
          obj = nullptr;
          is_obj = 0;
        } else if (!strcasecmp(field.c_str(), "weight")) {
          val = "";
          if (obj)
            val = itos(obj->Weight());
          obj = nullptr;
          is_obj = 0;
        } else if (!strcasecmp(field.c_str(), "sex")) {
          val = "";
          if (obj) {
            if (obj->Gender() == 'M')
              val = "male";
            else if (obj->Gender() == 'F')
              val = "female";
            else
              val = "none";
          }
          obj = nullptr;
          is_obj = 0;
        } else if (!strcasecmp(field.c_str(), "race")) {
          val = "";
          if (obj && obj->BaseAttribute(1) > 0) {
            val = "human"; // FIXME: Implement Race!
          }
          obj = nullptr;
          is_obj = 0;
        } else if (!strcasecmp(field.c_str(), "level")) {
          val = "";
          if (obj)
            val = itos(obj->Exp() / 10 + 1);
          obj = nullptr;
          is_obj = 0;
        } else if (!strcasecmp(field.c_str(), "name")) {
          val = "";
          if (obj)
            val = obj->Name();
          obj = nullptr;
          is_obj = 0;
        } else if (!strcasecmp(field.c_str(), "shortdesc")) {
          val = "";
          if (obj)
            val = obj->ShortDesc();
          obj = nullptr;
          is_obj = 0;
        } else if (!strcasecmp(field.c_str(), "alias")) {
          val = "";
          if (obj)
            val = obj->ShortDesc();
          obj = nullptr;
          is_obj = 0;
        } else if (!strcasecmp(field.c_str(), "heshe")) {
          val = "";
          if (obj)
            val = obj->Pron();
          obj = nullptr;
          is_obj = 0;
        } else if (!strcasecmp(field.c_str(), "hisher")) {
          val = "";
          if (obj)
            val = obj->Poss();
          obj = nullptr;
          is_obj = 0;
        } else if (!strcasecmp(field.c_str(), "himher")) {
          val = "";
          if (obj)
            val = obj->Obje();
          obj = nullptr;
          is_obj = 0;
        } else if (!strcasecmp(field.c_str(), "maxhitp")) {
          val = itos(1000); // Everybody has 1000 HP.
          obj = nullptr;
          is_obj = 0;
        } else if (!strcasecmp(field.c_str(), "hitp")) {
          val = "";
          if (obj)
            val = itos(1000 - 50 * (obj->Phys() + obj->Stun()));
          obj = nullptr;
          is_obj = 0;
        } else if (!strcasecmp(field.c_str(), "align")) {
          val = "";
          if (obj) {
            int align = 0;
            align = obj->Skill("Honor");
            if (align == 0)
              align = -(obj->Skill("Dishonor"));
            val = itos(align);
          }
          obj = nullptr;
          is_obj = 0;
        } else if (!strcasecmp(field.c_str(), "is_pc")) {
          val = "";
          if (obj)
            val = bstr[is_pc(obj)];
          obj = nullptr;
          is_obj = 0;
        } else if (!strcasecmp(field.c_str(), "is_killer")) {
          val = "0"; // FIXME: Real value?
          obj = nullptr;
          is_obj = 0;
        } else if (!strcasecmp(field.c_str(), "is_thief")) {
          val = "0"; // FIXME: Real value?
          obj = nullptr;
          is_obj = 0;
        } else if (!strcasecmp(field.c_str(), "con")) {
          val = "";
          if (obj)
            val = itos((obj->Attribute(0) - 2) * 3);
          obj = nullptr;
          is_obj = 0;
        } else if (!strcasecmp(field.c_str(), "dex")) {
          val = "";
          if (obj)
            val = itos((obj->Attribute(1) - 2) * 3);
          obj = nullptr;
          is_obj = 0;
        } else if (!strcasecmp(field.c_str(), "str")) {
          val = "";
          if (obj)
            val = itos((obj->Attribute(2) - 2) * 3);
          obj = nullptr;
          is_obj = 0;
        } else if (!strcasecmp(field.c_str(), "stradd")) { // D&D is Dumb
          val = "0";
          obj = nullptr;
          is_obj = 0;
        } else if (!strcasecmp(field.c_str(), "cha")) {
          val = "";
          if (obj)
            val = itos((obj->Attribute(3) - 2) * 3);
          obj = nullptr;
          is_obj = 0;
        } else if (!strcasecmp(field.c_str(), "int")) {
          val = "";
          if (obj)
            val = itos((obj->Attribute(4) - 2) * 3);
          obj = nullptr;
          is_obj = 0;
        } else if (!strcasecmp(field.c_str(), "wis")) {
          val = "";
          if (obj)
            val = itos((obj->Attribute(5) - 2) * 3);
          obj = nullptr;
          is_obj = 0;
        } else if (!strcasecmp(field.c_str(), "pos")) {
          val = "";
          if (obj) {
            if (obj->IsAct(ACT_SLEEP))
              val = "sleeping";
            else if (obj->IsAct(ACT_REST))
              val = "resting";
            else if (obj->IsAct(ACT_FIGHT))
              val = "fighting";
            else if (obj->Pos() == POS_LIE)
              val = "resting";
            else if (obj->Pos() == POS_SIT)
              val = "sitting";
            else if (obj->Pos() == POS_STAND)
              val = "standing";
          }
          obj = nullptr;
          is_obj = 0;
        } else if (!strcasecmp(field.c_str(), "title")) {
          val = "";
          obj = nullptr;
          is_obj = 0;
        } else if (!strcasecmp(field.c_str(), "val0")) { // FIXME: Implement?
          val = "0";
          obj = nullptr;
          is_obj = 0;
        } else if (!strcasecmp(field.c_str(), "val1")) { // FIXME: Implement?
          val = "0";
          obj = nullptr;
          is_obj = 0;
        } else if (!strcasecmp(field.c_str(), "val2")) { // FIXME: Implement?
          val = "0";
          obj = nullptr;
          is_obj = 0;
        } else if (!strcasecmp(field.c_str(), "val3")) { // FIXME: Implement?
          val = "0";
          obj = nullptr;
          is_obj = 0;
        } else if (!strcasecmp(field.c_str(), "timer")) {
          val = "";
          if (obj)
            val = itos(obj->Skill("Temporary")); // FIXME: More Kinds?
          obj = nullptr;
          is_obj = 0;
        } else if (!strcasecmp(field.c_str(), "move")) {
          val = "";
          if (obj)
            val = itos(10 - obj->Stun());
          obj = nullptr;
          is_obj = 0;
        } else if (!strcasecmp(field.c_str(), "maxmove")) {
          val = "";
          if (obj)
            val = "10";
          obj = nullptr;
          is_obj = 0;
        } else if (!strcasecmp(field.c_str(), "mana")) {
          val = "";
          if (obj) {
            if (obj->HasSkill("Faith")) {
              val = itos(obj->Skill("Faith Remaining"));
            } else {
              val = itos(10 - obj->Stun());
            }
          }
          obj = nullptr;
          is_obj = 0;
        } else if (!strcasecmp(field.c_str(), "maxmana")) {
          val = "";
          if (obj) {
            if (obj->HasSkill("Faith")) {
              val = itos(obj->Skill("Faith") * obj->Skill("Faith"));
            } else {
              val = "10";
            }
          }
          obj = nullptr;
          is_obj = 0;
        } else if (!strcasecmp(field.c_str(), "saving_para")) {
          val = "";
          if (obj)
            val = "0";
          obj = nullptr;
          is_obj = 0;
        } else if (!strcasecmp(field.c_str(), "saving_rod")) {
          val = "";
          if (obj)
            val = "0";
          obj = nullptr;
          is_obj = 0;
        } else if (!strcasecmp(field.c_str(), "saving_petri")) {
          val = "";
          if (obj)
            val = "0";
          obj = nullptr;
          is_obj = 0;
        } else if (!strcasecmp(field.c_str(), "saving_breath")) {
          val = "";
          if (obj)
            val = "0";
          obj = nullptr;
          is_obj = 0;
        } else if (!strcasecmp(field.c_str(), "saving_spell")) {
          val = "";
          if (obj)
            val = "0";
          obj = nullptr;
          is_obj = 0;
        } else if (!strcasecmp(field.c_str(), "prac")) {
          val = "";
          if (obj)
            val = "0";
          obj = nullptr;
          is_obj = 0;
        } else if (!strcasecmp(field.c_str(), "questpoints")) {
          val = "";
          if (obj)
            val = "0";
          obj = nullptr;
          is_obj = 0;
        } else if (!strcasecmp(field.c_str(), "exp")) {
          val = "";
          if (obj)
            val = itos(obj->Exp());
          obj = nullptr;
          is_obj = 0;
        } else if (!strcasecmp(field.c_str(), "hunger")) {
          val = "";
          if (obj)
            val = itos(obj->Skill("Hungry")); // FIXME: Convert
          obj = nullptr;
          is_obj = 0;
        } else if (!strcasecmp(field.c_str(), "thirst")) {
          val = "";
          if (obj)
            val = itos(obj->Skill("Thirsty")); // FIXME: Convert
          obj = nullptr;
          is_obj = 0;
        } else if (!strcasecmp(field.c_str(), "drunk")) {
          val = "";
          if (obj)
            val = "0"; // FIXME: Query Drunkenness Here
          obj = nullptr;
          is_obj = 0;
        } else if (!strcasecmp(field.c_str(), "class")) {
          val = "";
          if (obj) {
            if (obj->HasSkill("Spellcasting") || obj->HasSkill("Spellcraft")) {
              val = "magic user";
            } else if (obj->HasSkill("Perception") || obj->HasSkill("Stealth")) {
              val = "thief";
            } else if (obj->HasSkill("Faith")) {
              val = "priest";
            } else {
              val = "warrior";
            }
          }
          obj = nullptr;
          is_obj = 0;
        } else if (!strcasecmp(field.c_str(), "canbeseen")) {
          val = "";
          if (obj)
            val = bstr[!(obj->HasSkill("Invisible") || obj->HasSkill("Hidden"))];
          obj = nullptr;
          is_obj = 0;
        } else if (!strcasecmp(field.c_str(), "affect")) {
          val = ""; // FIXME: Translate & List Spell Effects?
          obj = nullptr;
          is_obj = 0;
        } else if (!strcasecmp(field.c_str(), "fighting")) {
          if (obj)
            obj = obj->ActTarg(ACT_FIGHT);
        } else if (!strcasecmp(field.c_str(), "worn_by")) {
          if (obj) {
            Object* owner = obj->Owner();
            if (owner && owner->Wearing(obj))
              obj = owner;
            else
              obj = nullptr;
          } else
            obj = nullptr;
        } else if (!strcasecmp(field.c_str(), "room")) {
          while (obj && obj->Skill("TBARoom") == 0)
            obj = obj->Parent();
        } else if (!strcasecmp(field.c_str(), "people")) {
          if (obj)
            obj = obj->PickObject("someone", LOC_INTERNAL);
        } else if (!strcasecmp(field.c_str(), "contents")) {
          if (obj)
            obj = obj->PickObject("something", LOC_INTERNAL);
        } else if (!strcasecmp(field.c_str(), "inventory")) {
          if (obj)
            obj = obj->PickObject("something", LOC_INTERNAL | LOC_NOTWORN);
        } else if ((!strcasecmp(field.c_str(), "eq(*)")) || (!strcasecmp(field.c_str(), "eq"))) {
          if (obj)
            obj = obj->PickObject("something", LOC_INTERNAL | LOC_NOTUNWORN);
        } else if (
            (!strcasecmp(field.c_str(), "eq(light)")) || (!strcasecmp(field.c_str(), "eq(hold)")) ||
            (!strcasecmp(field.c_str(), "eq(0)")) || (!strcasecmp(field.c_str(), "eq(17)"))) {
          if (obj)
            obj = obj->ActTarg(ACT_HOLD);
        } else if (
            (!strcasecmp(field.c_str(), "eq(wield)")) || (!strcasecmp(field.c_str(), "eq(16)"))) {
          if (obj)
            obj = obj->ActTarg(ACT_WIELD);
        } else if (
            (!strcasecmp(field.c_str(), "eq(rfinger)")) || (!strcasecmp(field.c_str(), "eq(1)"))) {
          if (obj)
            obj = obj->ActTarg(ACT_WEAR_RFINGER);
        } else if (
            (!strcasecmp(field.c_str(), "eq(lfinger)")) || (!strcasecmp(field.c_str(), "eq(2)"))) {
          if (obj)
            obj = obj->ActTarg(ACT_WEAR_LFINGER);
        } else if (
            (!strcasecmp(field.c_str(), "eq(neck1)")) || (!strcasecmp(field.c_str(), "eq(3)"))) {
          if (obj)
            obj = obj->ActTarg(ACT_WEAR_NECK);
        } else if (
            (!strcasecmp(field.c_str(), "eq(neck2)")) || (!strcasecmp(field.c_str(), "eq(4)"))) {
          if (obj)
            obj = obj->ActTarg(ACT_WEAR_COLLAR);
        } else if (
            (!strcasecmp(field.c_str(), "eq(body)")) || (!strcasecmp(field.c_str(), "eq(5)"))) {
          if (obj)
            obj = obj->ActTarg(ACT_WEAR_CHEST);
        } else if (
            (!strcasecmp(field.c_str(), "eq(head)")) || (!strcasecmp(field.c_str(), "eq(6)"))) {
          if (obj)
            obj = obj->ActTarg(ACT_WEAR_HEAD);
        } else if (
            (!strcasecmp(field.c_str(), "eq(legs)")) || (!strcasecmp(field.c_str(), "eq(7)"))) {
          if (obj)
            obj = obj->ActTarg(ACT_WEAR_LLEG);
        } else if (
            (!strcasecmp(field.c_str(), "eq(feet)")) || (!strcasecmp(field.c_str(), "eq(8)"))) {
          if (obj)
            obj = obj->ActTarg(ACT_WEAR_LFOOT);
        } else if (
            (!strcasecmp(field.c_str(), "eq(hands)")) || (!strcasecmp(field.c_str(), "eq(9)"))) {
          if (obj)
            obj = obj->ActTarg(ACT_WEAR_LHAND);
        } else if (
            (!strcasecmp(field.c_str(), "eq(arms)")) || (!strcasecmp(field.c_str(), "eq(10)"))) {
          if (obj)
            obj = obj->ActTarg(ACT_WEAR_LARM);
        } else if (
            (!strcasecmp(field.c_str(), "eq(shield)")) || (!strcasecmp(field.c_str(), "eq(11)"))) {
          if (obj)
            obj = obj->ActTarg(ACT_WEAR_SHIELD);
        } else if (
            (!strcasecmp(field.c_str(), "eq(about)")) || (!strcasecmp(field.c_str(), "eq(12)"))) {
          if (obj)
            obj = obj->ActTarg(ACT_WEAR_LSHOULDER);
        } else if (
            (!strcasecmp(field.c_str(), "eq(waits)")) || (!strcasecmp(field.c_str(), "eq(13)"))) {
          if (obj)
            obj = obj->ActTarg(ACT_WEAR_WAIST);
        } else if (
            (!strcasecmp(field.c_str(), "eq(rwrist)")) || (!strcasecmp(field.c_str(), "eq(14)"))) {
          if (obj)
            obj = obj->ActTarg(ACT_WEAR_RWRIST);
        } else if (
            (!strcasecmp(field.c_str(), "eq(lwrist)")) || (!strcasecmp(field.c_str(), "eq(15)"))) {
          if (obj)
            obj = obj->ActTarg(ACT_WEAR_LWRIST);
        } else if (!strcasecmp(field.c_str(), "carried_by")) {
          if (obj)
            obj = obj->Owner();
        } else if (!strcasecmp(field.c_str(), "next_in_list")) {
          if (obj) {
            Object* par = obj->Owner();
            if (!par)
              par = obj->Parent();
            if (par) {
              std::list<Object*> stf = par->PickObjects("everything", LOC_INTERNAL);
              std::list<Object*>::iterator item = stf.begin();
              while (item != stf.end() && (*item) != obj)
                ++item;
              if (item != stf.end())
                ++item;
              if (item != stf.end())
                obj = (*item);
              else
                obj = nullptr;
            } else
              obj = nullptr;
          }
        } else if (!strcasecmp(field.c_str(), "next_in_room")) {
          if (obj) {
            Object* room = obj->Parent();
            while (room && room->Skill("TBARoom") == 0)
              room = room->Parent();
            if (room) {
              std::list<Object*> stf = room->PickObjects("everyone", LOC_INTERNAL);
              std::list<Object*>::iterator item = stf.begin();
              while (item != stf.end() && (*item) != obj)
                ++item;
              if (item != stf.end())
                ++item;
              if (item != stf.end())
                obj = (*item);
              else
                obj = nullptr;
            } else
              obj = nullptr;
          }
        } else if (!strcasecmp(field.c_str(), "master")) {
          if (obj)
            obj = obj->ActTarg(ACT_FOLLOW); // FIXME: More Kinds?
        } else if (!strcasecmp(field.c_str(), "follower")) {
          if (obj) {
            std::set<Object*> touch = obj->Touching();
            std::set<Object*>::iterator tent = touch.begin();
            for (; tent != touch.end(); ++tent) {
              if ((*tent)->ActTarg(ACT_FOLLOW) == obj) {
                obj = (*tent);
                break;
              }
            }
            if (tent == touch.end())
              obj = nullptr;
          } else
            obj = nullptr;
        } else if (!strncasecmp(field.c_str(), "skill(", 6)) {
          val = "";
          if (obj) {
            size_t num = field.find_first_of(") .%", 6);
            std::string skl = get_skill(field.substr(6, num - 6));
            val = itos(obj->Skill(skl));
          }
          obj = nullptr;
          is_obj = 0;
        } else if (!strncasecmp(field.c_str(), "varexists(", 10)) {
          val = "";
          if (obj) {
            size_t num = field.find_first_of(") .%", 10);
            std::string var = field.substr(10, num - 10); // FIXME: Variables!
            val = bstr[obj->HasSkill(std::string("TBA:") + var)];
          }
          obj = nullptr;
          is_obj = 0;
        } else if (!strncasecmp(field.c_str(), "has_item(", 9)) {
          int vnum = -1;
          size_t num = field.find_first_not_of("0123456789", 9);
          sscanf(field.c_str() + 9, "%d", &vnum);
          val = "";
          if (obj && vnum != -1 && field[num] == ')') {
            vnum += 2000000;
            std::list<Object*> pos = obj->PickObjects("all", LOC_INTERNAL);
            std::list<Object*>::iterator item = pos.begin();
            for (; item != pos.end(); ++item) {
              if (vnum == (*item)->Skill("TBAObject")) {
                val = "1";
                break;
              }
            }
          }
          obj = nullptr;
          is_obj = 0;
        } else if (!strncasecmp(field.c_str(), "vnum(", 5)) {
          val = "0"; // Default - in case it doesn't have a vnum
          if (obj) {
            int vnum = obj->Skill("TBAMOB");
            if (vnum < 1)
              vnum = obj->Skill("TBAObject");
            if (vnum < 1)
              vnum = obj->Skill("TBARoom");
            if (vnum > 0) {
              vnum %= 1000000; // Convert from Acid number
              int qnum = TBAEval(field.c_str() + 5);
              val = bstr[(vnum == qnum)];
            }
          }
          obj = nullptr;
          is_obj = 0;
        } else if (!strcasecmp(field.c_str(), "pos(sleeping)")) {
          if (obj) {
            obj->SetPos(POS_LIE);
            obj->StopAct(ACT_REST);
            obj->AddAct(ACT_SLEEP);
          }
          obj = nullptr;
          val = "";
          is_obj = 0;
        } else if (!strcasecmp(field.c_str(), "pos(resting)")) {
          if (obj) {
            obj->StopAct(ACT_SLEEP);
            obj->SetPos(POS_SIT);
            obj->AddAct(ACT_REST);
          }
          obj = nullptr;
          val = "";
          is_obj = 0;
        } else if (!strcasecmp(field.c_str(), "pos(sitting)")) {
          if (obj) {
            obj->StopAct(ACT_SLEEP);
            obj->StopAct(ACT_REST);
            obj->SetPos(POS_SIT);
          }
          obj = nullptr;
          val = "";
          is_obj = 0;
        } else if (!strcasecmp(field.c_str(), "pos(fighting)")) {
          if (obj) {
            obj->StopAct(ACT_SLEEP);
            obj->StopAct(ACT_REST);
            obj->SetPos(POS_STAND);
          }
          obj = nullptr;
          val = "";
          is_obj = 0;
        } else if (!strcasecmp(field.c_str(), "pos(standing)")) {
          if (obj) {
            obj->StopAct(ACT_SLEEP);
            obj->StopAct(ACT_REST);
            obj->SetPos(POS_STAND);
          }
          obj = nullptr;
          val = "";
          is_obj = 0;
          // Is no general fighting state, must fight someone!
        } else {
          fprintf(
              stderr,
              CRED "#%d Error: Bad sub-obj '%s' in '%s'\n" CNRM,
              body->Skill("TBAScript"),
              field.c_str(),
              line.c_str());
          Disable();
          return;
        }
      } else {
        if (!strcasecmp(field.c_str(), "mudcommand")) {
          // val is already right
        } else if (!strcasecmp(field.c_str(), "car")) {
          size_t apos = val.find_first_of(" \t\n\r");
          if (apos != std::string::npos) {
            val = val.substr(0, apos);
          }
        } else if (!strcasecmp(field.c_str(), "cdr")) {
          size_t apos = val.find_first_of(" \t\n\r");
          if (apos != std::string::npos) {
            apos = val.find_first_not_of(" \t\n\r", apos);
            if (apos != std::string::npos)
              val = val.substr(apos);
            else
              val = "";
          } else
            val = "";
        } else if (!strcasecmp(field.c_str(), "trim")) {
          trim_string(val);
        } else {
          fprintf(
              stderr,
              CRED "#%d Error: Bad sub-str '%s' in '%s'\n" CNRM,
              body->Skill("TBAScript"),
              field.c_str(),
              line.c_str());
          Disable();
          return;
        }
      }
    }
    if (end == std::string::npos)
      end = line.length();
    else if (line[end] == '%')
      ++end;
    if (is_obj) {
      char buf[256] = "";
      sprintf(buf, "OBJ:%p", obj);
      line.replace(cur, end - cur, buf);
    } else { // std::string OR ""
      line.replace(cur, end - cur, val);
    }
    cur = line.find('%', cur + 1);
  }
  if (0
      //	|| body->Skill("TBAScript") == 5000099
      //	|| line.find("eval loc ") != std::string::npos
      //	|| line.find("set first ") != std::string::npos
      //	|| line.find("exclaim") != std::string::npos
      //	|| line.find("speech") != std::string::npos
  ) {
    fprintf(stderr, CGRN "#%d Debug: '%s' <-Final\n" CNRM, body->Skill("TBAScript"), line.c_str());
  }
}

#define QUOTAERROR1 CRED "#%d Error: script quota exceeded - killed.\n" CNRM
#define QUOTAERROR2 body->Skill("TBAScript")
#define PING_QUOTA()                             \
  {                                              \
    --quota;                                     \
    if (quota < 1) {                             \
      fprintf(stderr, QUOTAERROR1, QUOTAERROR2); \
      Disable();                                 \
      return;                                    \
    }                                            \
  }

// Return 0 to continue running, 1 to be done now (error/suspend/done).
int Mind::TBARunLine(std::string line) {
  if (0
      //	|| body->Skill("TBAScript") == 5000099
      //	|| line.find("eval loc ") != std::string::npos
      //	|| line.find("set first ") != std::string::npos
      //	|| line.find("exclaim") != std::string::npos
      //	|| line.find("speech") != std::string::npos
  ) {
    fprintf(stderr, CGRN "#%d Debug: Running '%s'\n" CNRM, body->Skill("TBAScript"), line.c_str());
  }
  Object* room = ovars["self"];
  while (room && room->Skill("TBARoom") == 0) {
    if (room->Skill("Invisible") > 999)
      room = nullptr; // Not really there
    else
      room = room->Parent();
  }
  if (!room) { // Not in a room (dup clone, in a popper, etc...).
    //    fprintf(stderr, CRED "#%d Error: No room in '%s'\n" CNRM,
    //	body->Skill("TBAScript"), line.c_str()
    //	);
    Disable();
    return 1;
  }
  // Needs to be alive! MOB & MOB-* (Not -DEATH or -GLOBAL)
  if ((body->Skill("TBAScriptType") & 0x103FFDE) > 0x1000000) {
    if (ovars["self"]->IsAct(ACT_DEAD) || ovars["self"]->IsAct(ACT_DYING) ||
        ovars["self"]->IsAct(ACT_UNCONSCIOUS)) {
      //      fprintf(stderr, CGRN "#%d Debug: Triggered on downed MOB.\n" CNRM,
      //	body->Skill("TBAScript")
      //	);
      spos_s.front() = std::string::npos; // Jump to End
      return 0; // Allow re-run (in case of resurrection/waking/etc...).
    }
  }

  size_t spos = spos_s.front();
  int vnum = body->Skill("TBAScript");
  TBAVarSub(line);
  if (type == MIND_MORON) {
    fprintf(stderr, CRED "#%d Error: VarSub failed in '%s'\n" CNRM, vnum, line.c_str());
    return 1;
  }

  com_t com = identify_command(line); // ComNum for Pass-Through

  //  //Start of script command if/else if/else
  //  if(line.find("%") != line.rfind("%")) {		//More than one '%'
  //    fprintf(stderr, CRED "#%d Error: Failed to fully expand '%s'\n" CNRM,
  //	body->Skill("TBAScript"), line.c_str()
  //	);
  //    Disable();
  //    return 1;
  //    }

  if (!strncasecmp(line.c_str(), "unset ", 6)) {
    size_t lpos = line.find_first_not_of(" \t", 6);
    if (lpos != std::string::npos) {
      std::string var = line.substr(lpos);
      trim_string(var);
      svars.erase(var);
      ovars.erase(var);
      ovars["context"]->SetSkill(std::string("TBA:") + var, 0);
    } else {
      fprintf(
          stderr,
          CRED "#%d Error: Malformed unset '%s'\n" CNRM,
          body->Skill("TBAScript"),
          line.c_str());
      Disable();
      return 1;
    }
    return 0;
  }

  else if ((!strncasecmp(line.c_str(), "eval ", 5)) || (!strncasecmp(line.c_str(), "set ", 4))) {
    char coml = tolower(line[0]); // To tell eval from set later.
    size_t lpos = line.find_first_not_of(" \t", 4);
    if (lpos != std::string::npos) {
      line = line.substr(lpos);
      size_t end1 = line.find_first_of(" \t\n\r");
      if (end1 != std::string::npos) {
        std::string var = line.substr(0, end1);
        lpos = line.find_first_not_of(" \t", end1 + 1);
        if (lpos != std::string::npos) {
          std::string val = line.substr(lpos);
          if (0
              //		|| var.find("midgaard") != std::string::npos
              //		|| var.find("exclaim") != std::string::npos
              //		|| var.find("speech") != std::string::npos
          ) {
            fprintf(
                stderr,
                CGRN "#%d Debug: '%s' = '%s'\n" CNRM,
                body->Skill("TBAScript"),
                var.c_str(),
                val.c_str());
          }
          if (coml == 'e') {
            int vnum = body->Skill("TBAScript");
            TBAVarSub(val);
            if (type == MIND_MORON) {
              fprintf(stderr, CRED "#%d Error: Eval failed in '%s'\n" CNRM, vnum, line.c_str());
              return 1;
            }
            val = TBAComp(val);
          }
          if (!strncmp(val.c_str(), "OBJ:", 4)) { // Encoded Object
            ovars[var] = nullptr;
            sscanf(val.c_str(), "OBJ:%p", &(ovars[var]));
            svars.erase(var);
          } else {
            svars[var] = val;
            ovars.erase(var);
          }
        } else { // Only space after varname
          svars[var] = "";
          ovars.erase(var);
        }
      } else { // Nothing after varname
        svars[line] = "";
        ovars.erase(line);
      }
    }
    return 0;
  }

  else if ((!strncasecmp(line.c_str(), "extract ", 8))) {
    size_t lpos = line.find_first_not_of(" \t", 8);
    if (lpos != std::string::npos) {
      line = line.substr(lpos);
      size_t end1 = line.find_first_of(" \t\n\r");
      if (end1 != std::string::npos) {
        std::string var = line.substr(0, end1);
        lpos = line.find_first_not_of(" \t", end1 + 1);
        if (lpos != std::string::npos) {
          int wnum = atoi(line.c_str() + lpos) - 1; // Start at 0, -1=fail
          lpos = line.find_first_of(" \t", lpos);
          if (lpos != std::string::npos)
            lpos = line.find_first_not_of(" \t", lpos);
          if (wnum >= 0 && lpos != std::string::npos) {
            while (wnum > 0 && lpos != std::string::npos) {
              lpos = line.find_first_of(" \t", lpos);
              if (lpos != std::string::npos) {
                lpos = line.find_first_not_of(" \t", lpos);
              }
              --wnum;
            }
            if (lpos != std::string::npos) {
              end1 = line.find_first_of(" \t\n\r", lpos);
              if (end1 == std::string::npos)
                end1 = line.length();
              svars[var] = line.substr(lpos, end1 - lpos);
            } else {
              svars[var] = "";
            }
            ovars.erase(var);
          } else if (wnum < 0) { // Bad number after varname
            fprintf(
                stderr,
                CRED "#%d Error: Malformed extract '%s'\n" CNRM,
                body->Skill("TBAScript"),
                line.c_str());
            Disable();
            return 1;
          }
        } else { // Only space after varname
          fprintf(
              stderr,
              CRED "#%d Error: Malformed extract '%s'\n" CNRM,
              body->Skill("TBAScript"),
              line.c_str());
          Disable();
          return 1;
        }
      } else { // Nothing after varname
        fprintf(
            stderr,
            CRED "#%d Error: Malformed extract '%s'\n" CNRM,
            body->Skill("TBAScript"),
            line.c_str());
        Disable();
        return 1;
      }
    }
    return 0;
  }

  else if (!strncasecmp(line.c_str(), "at ", 3)) {
    int dnum, pos;
    if (sscanf(line.c_str(), "at %d %n", &dnum, &pos) < 1) {
      fprintf(
          stderr,
          CRED "#%d Error: Malformed at '%s'\n" CNRM,
          body->Skill("TBAScript"),
          line.c_str());
      Disable();
      return 1;
    }
    room = room->World();
    std::list<Object*> options = room->Contents();
    std::list<Object*>::iterator opt = options.begin();
    room = nullptr;
    for (; opt != options.end(); ++opt) {
      int tnum = (*opt)->Skill("TBARoom");
      if (tnum > 0 && (tnum % 1000000) == dnum) {
        room = (*opt);
        break;
      }
    }
    if (!room) {
      fprintf(
          stderr,
          CRED "#%d Error: Can't find room in '%s'\n" CNRM,
          body->Skill("TBAScript"),
          line.c_str());
      Disable();
      return 1;
    }
    Object* oldp = nullptr;
    if (ovars["self"]->Parent() != room) {
      oldp = ovars["self"]->Parent();
      oldp->RemoveLink(ovars["self"]);
      ovars["self"]->SetParent(room);
    }
    int ret = TBARunLine(line.substr(pos));
    if (oldp) {
      ovars["self"]->Parent()->RemoveLink(ovars["self"]);
      ovars["self"]->SetParent(oldp);
    }
    return ret;
  }

  else if (!strncasecmp(line.c_str(), "context ", 8)) {
    Object* con = nullptr;
    if (sscanf(line.c_str() + 8, " OBJ:%p", &con) >= 1 && con != nullptr) {
      ovars["context"] = con;
    } else {
      fprintf(
          stderr,
          CRED "#%d Error: No Context Object '%s'\n" CNRM,
          body->Skill("TBAScript"),
          line.c_str());
      return 1;
    }
  }

  else if (!strncasecmp(line.c_str(), "rdelete ", 8)) {
    Object* con = nullptr;
    char var[256] = "";
    if (sscanf(line.c_str() + 7, " %s OBJ:%p", var, &con) >= 2 && con != nullptr) {
      con->SetSkill(std::string("TBA:") + var, 0);
      //      fprintf(stderr, CGRN "#%d Debug: RDelete '%s'\n" CNRM,
      //		body->Skill("TBAScript"), line.c_str());
    } else {
      fprintf(
          stderr,
          CRED "#%d Error: No RDelete VarName/ID '%s'\n" CNRM,
          body->Skill("TBAScript"),
          line.c_str());
      return 1;
    }
  }

  else if (!strncasecmp(line.c_str(), "remote ", 7)) {
    Object* con = nullptr;
    char var[256] = "";
    if (sscanf(line.c_str() + 7, " %s OBJ:%p", var, &con) >= 2 && con != nullptr) {
      if (svars.count(var) > 0) {
        int val = atoi(svars[var].c_str());
        con->SetSkill(std::string("TBA:") + var, val);
        //	fprintf(stderr, CGRN "#%d Debug: Remote %s=%d '%s'\n" CNRM,
        //		body->Skill("TBAScript"), var, val, line.c_str());
      } else {
        fprintf(
            stderr,
            CRED "#%d Error: Non-Existent Remote %s '%s'\n" CNRM,
            body->Skill("TBAScript"),
            var,
            line.c_str());
        return 1;
      }
    } else {
      fprintf(
          stderr,
          CRED "#%d Error: No Remote VarName/ID '%s'\n" CNRM,
          body->Skill("TBAScript"),
          line.c_str());
      return 1;
    }
  }

  else if (!strncasecmp(line.c_str(), "global ", 7)) {
    Object* con = ovars["context"];
    char var[256] = "";
    if (sscanf(line.c_str() + 7, " %s", var) >= 1 && con != nullptr) {
      if (svars.count(var) > 0) {
        int val = atoi(svars[var].c_str());
        con->SetSkill(std::string("TBA:") + var, val);
        //	fprintf(stderr, CGRN "#%d Debug: Global %s=%d '%s'\n" CNRM,
        //		body->Skill("TBAScript"), var, val, line.c_str());
      } else {
        fprintf(
            stderr,
            CRED "#%d Error: Non-Existent Global %s '%s'\n" CNRM,
            body->Skill("TBAScript"),
            var,
            line.c_str());
        return 1;
      }
    } else {
      fprintf(
          stderr,
          CRED "#%d Error: No Global VarName '%s'\n" CNRM,
          body->Skill("TBAScript"),
          line.c_str());
      return 1;
    }
  }

  else if (!strncasecmp(line.c_str(), "wait until ", 11)) {
    int hour = 0, minute = 0, cur = 0;
    if (sscanf(line.c_str() + 11, "%d:%d", &hour, &minute) > 0) {
      if (hour >= 100)
        hour /= 100;
      minute += hour * 60;
      Object* world = room->World();
      if (world->Skill("Day Time") && world->Skill("Day Length")) {
        cur = world->Skill("Day Time");
        cur *= 24 * 60;
        cur /= world->Skill("Day Length");
      }
      if (minute < cur)
        minute += 24 * 60; // Not Time Until Tomorrow!
      if (minute > cur) { // Not Time Yet!
        Suspend((minute - cur) * 1000 * world->Skill("Day Length") / 24);
        // Note: The above calculation removed the *60 and the /60
        return 1;
      }
    } else {
      fprintf(stderr, CRED "#%d Error: Told '%s'\n" CNRM, body->Skill("TBAScript"), line.c_str());
      Disable();
      return 1;
    }
  }

  else if (!strncasecmp(line.c_str(), "wait ", 5)) {
    int time = 0;
    sscanf(line.c_str() + 5, "%d", &time);
    if (time > 0) {
      Suspend(time * 1000);
      return 1;
    } else {
      fprintf(stderr, CRED "#%d Error: Told '%s'\n" CNRM, body->Skill("TBAScript"), line.c_str());
      Disable();
      return 1;
    }
  }

  else if (
      (!strncasecmp(line.c_str(), "oset ", 5)) || (!strncasecmp(line.c_str(), "osetval ", 8))) {
    int v1, v2;
    size_t end = line.find(" ");
    if (sscanf(line.c_str() + end, " %d %d", &v1, &v2) != 2) {
      fprintf(stderr, CRED "#%d Error: Told '%s'\n" CNRM, body->Skill("TBAScript"), line.c_str());
      Disable();
      return 1;
    } else if (ovars["self"]->Skill("Liquid Source") && v1 == 0) {
      if (v2 < 0)
        v2 = 1 << 30;
      ovars["self"]->SetSkill("Liquid Source", v2 + 1);
    } else if (ovars["self"]->Skill("Liquid Source") && v1 == 1) {
      if (ovars["self"]->Contents().size() < 1) {
        fprintf(
            stderr,
            CYEL "#%d Warning: Empty fountain '%s'\n" CNRM,
            body->Skill("TBAScript"),
            line.c_str());
        Disable();
        return 1;
      }
      ovars["self"]->Contents().front()->SetSkill("Quantity", v2 + 1);
    } else {
      fprintf(
          stderr,
          CRED "#%d Error: Unimplemented oset '%s'\n" CNRM,
          body->Skill("TBAScript"),
          line.c_str());
      Disable();
      return 1;
    }
  }

  else if (!strncasecmp(line.c_str(), "if ", 3)) {
    if (!TBAEval(line.c_str() + 3)) { // Was false
      int depth = 0;
      while (spos != std::string::npos) { // Skip to end/elseif
        //        PING_QUOTA();
        if ((!depth) && (!strncasecmp(script.c_str() + spos, "elseif ", 7))) {
          spos += 4; // Make it into an "if" and go
          break;
        } else if (!strncasecmp(script.c_str() + spos, "else", 4)) {
          if (!depth) { // Only right if all the way back
            spos = skip_line(script, spos);
            break;
          }
        } else if (!strncasecmp(script.c_str() + spos, "end", 3)) {
          if (!depth) { // Only done if all the way back
            spos = skip_line(script, spos);
            break;
          }
          --depth; // Otherwise am just 1 nesting level less deep
        } else if (!strncasecmp(script.c_str() + spos, "if ", 3)) {
          ++depth; // Am now 1 nesting level deeper!
        }
        spos = skip_line(script, spos);
      }
      spos_s.front() = spos; // Save skip-to position in real PC
    }
  }

  else if (!strncasecmp(line.c_str(), "else", 4)) { // else/elseif
    int depth = 0;
    while (spos != std::string::npos) { // Skip to end (considering nesting)
      //      PING_QUOTA();
      if (!strncasecmp(script.c_str() + spos, "end", 3)) {
        if (depth == 0) { // Only done if all the way back
          spos = skip_line(script, spos);
          break;
        }
        --depth; // Otherwise am just 1 nesting level less deep
      } else if (!strncasecmp(script.c_str() + spos, "if ", 3)) {
        ++depth; // Am now 1 nesting level deeper!
      }
      spos = skip_line(script, spos);
    }
    spos_s.front() = spos; // Save skip-to position in real PC
  }

  else if (!strncasecmp(line.c_str(), "while ", 6)) {
    int depth = 0;
    size_t rep = prev_line(script, spos);
    size_t begin = spos;
    while (spos != std::string::npos) { // Skip to end (considering nesting)
      //      PING_QUOTA();
      if (!strncasecmp(script.c_str() + spos, "done", 4)) {
        if (depth == 0) { // Only done if all the way back
          spos = skip_line(script, spos);
          break;
        }
        --depth; // Otherwise am just 1 nesting level less deep
      } else if (!strncasecmp(script.c_str() + spos, "switch ", 7)) {
        ++depth; // Am now 1 nesting level deeper!
      } else if (!strncasecmp(script.c_str() + spos, "while ", 6)) {
        ++depth; // Am now 1 nesting level deeper!
      }
      spos = skip_line(script, spos);
    }
    if (TBAEval(line.c_str() + 6)) {
      spos_s.front() = rep; // Will repeat the "while"
      spos_s.push_front(begin); // But run the inside of the loop first.
    } else {
      spos_s.front() = spos; // Save after-done position in real PC
    }
  }

  else if (!strncasecmp(line.c_str(), "switch ", 7)) {
    int depth = 0;
    size_t targ = 0;
    std::string value = line.substr(7);
    trim_string(value);
    while (spos != std::string::npos) { // Skip to end (considering nesting)
      //      PING_QUOTA();
      if (!strncasecmp(script.c_str() + spos, "done", 4)) {
        if (depth == 0) { // Only done if all the way back
          spos = skip_line(script, spos);
          break;
        }
        --depth; // Otherwise am just 1 nesting level less deep
      } else if (!strncasecmp(script.c_str() + spos, "switch ", 7)) {
        ++depth; // Am now 1 nesting level deeper!
      } else if (!strncasecmp(script.c_str() + spos, "while ", 6)) {
        ++depth; // Am now 1 nesting level deeper!
      } else if (
          depth == 0 && (!strncasecmp(script.c_str() + spos, "case ", 5)) &&
          TBAEval(value + " == " + script.substr(spos + 5))) { // The actual case I want!
        spos = skip_line(script, spos);
        targ = spos;
        continue;
      } else if (
          depth == 0 &&
          (!strncasecmp(
              script.c_str() + spos,
              "default",
              7))) { // Maybe the case I want
        if (targ == 0) {
          spos = skip_line(script, spos);
          targ = spos;
          continue;
        }
      }
      spos = skip_line(script, spos);
    }
    spos_s.front() = spos; // Save after-done position in real PC
    if (targ != 0) { // Got a case to go to
      spos_s.push_front(targ); // Push jump-to position above real PC
    }
  }

  else if (!strncasecmp(line.c_str(), "break", 5)) { // Skip to done
    int depth = 0;
    while (spos != std::string::npos) { // Skip to end (considering nesting)
      //      PING_QUOTA();
      if (!strncasecmp(script.c_str() + spos, "done", 4)) {
        if (depth == 0) { // Only done if all the way back
          break;
        }
        --depth; // Otherwise am just 1 nesting level less deep
      } else if (!strncasecmp(script.c_str() + spos, "switch ", 7)) {
        ++depth; // Am now 1 nesting level deeper!
      } else if (!strncasecmp(script.c_str() + spos, "while ", 6)) {
        ++depth; // Am now 1 nesting level deeper!
      }
      spos = skip_line(script, spos);
    }
    spos_s.front() = spos; // Save done position in real PC
  }

  else if ((!strncasecmp(line.c_str(), "asound ", 7))) {
    size_t start = line.find_first_not_of(" \t\r\n", 7);
    if (room && start != std::string::npos) {
      std::string mes = line.substr(start);
      trim_string(mes);
      replace_all(mes, "You hear ", "");
      replace_all(mes, " can be heard close by", "");
      replace_all(mes, " is heard close by", "");
      replace_all(mes, " from close by", "");
      replace_all(mes, " from nearby", "");
      mes += "\n";
      room->Loud(2, mes.c_str()); // 2 will go through 1 closed door.
    }
  }

  else if ((!strncasecmp(line.c_str(), "zoneecho ", 9))) {
    size_t start = line.find_first_not_of(" \t\r\n", 9);
    start = line.find_first_of(" \t\r\n", start);
    start = line.find_first_not_of(" \t\r\n", start);
    if (room && start != std::string::npos) {
      std::string mes = line.substr(start);
      trim_string(mes);
      mes += "\n";
      room->SendIn(ALL, 0, mes.c_str(), "", nullptr, nullptr);
      room->Loud(8, mes.c_str()); // 8 will go 4-8 rooms.
    }
  }

  else if (!strncasecmp(line.c_str(), "echoaround ", 11)) {
    Object* targ = nullptr;
    char mes[256] = "";
    sscanf(line.c_str() + 11, " OBJ:%p %254[^\n\r]", &targ, mes);
    mes[strlen(mes) + 1] = 0;
    mes[strlen(mes)] = '\n';
    Object* troom = targ;
    while (troom && troom->Skill("TBARoom") == 0)
      troom = troom->Parent();
    if (troom && targ)
      troom->SendOut(0, 0, mes, "", targ, nullptr);
  }

  else if ((!strncasecmp(line.c_str(), "mecho ", 6))) {
    size_t start = line.find_first_not_of(" \t\r\n", 6);
    if (room && start != std::string::npos) {
      std::string mes = line.substr(start);
      trim_string(mes);
      mes += "\n";
      room->SendOut(0, 0, mes.c_str(), mes.c_str(), nullptr, nullptr);
    }
  }

  else if (!strncasecmp(line.c_str(), "send ", 5)) {
    Object* targ = nullptr;
    char mes[1024] = "";
    sscanf(line.c_str() + 5, " OBJ:%p %1022[^\n\r]", &targ, mes);
    mes[strlen(mes) + 1] = 0;
    mes[strlen(mes)] = '\n';
    if (targ)
      targ->Send(0, 0, mes);
  }

  else if (!strncasecmp(line.c_str(), "force ", 6)) {
    Object* targ = nullptr;
    char tstr[256] = "", cmd[1024] = "";
    if (sscanf(line.c_str() + 6, " OBJ:%p %1023[^\n\r]", &targ, cmd) < 2) {
      if (sscanf(line.c_str() + 6, " %255s %1023[^\n\r]", tstr, cmd) >= 2) {
        targ = room->PickObject(tstr, LOC_NINJA | LOC_INTERNAL);
      }
    }
    if (targ) {
      Mind* amind = nullptr; // Make sure human minds see it!
      std::vector<Mind*> mns = get_human_minds();
      std::vector<Mind*>::iterator mn = mns.begin();
      for (; mn != mns.end(); ++mn) {
        if ((*mn)->Body() == targ) {
          amind = *mn;
          break;
        }
      }
      handle_command(targ, cmd, amind);
    }
  }

  else if (!strncasecmp(line.c_str(), "wdamage ", 8)) {
    //    fprintf(stderr, CGRN "#%d Debug: WDamage '%s'\n" CNRM,
    //	body->Skill("TBAScript"), line.c_str()
    //	);
    int pos = 0;
    int dam = 0;
    char buf[256];
    if (sscanf(line.c_str() + 8, " %s %n", buf, &pos) >= 1) {
      if (!strcasecmp(buf, "all")) {
        strcpy(buf, "everyone");
      }
      dam = TBAEval(line.c_str() + 8 + pos);
      if (dam > 0)
        dam = (dam + 180) / 100;
      if (dam < 0)
        dam = (dam - 180) / 100;
    }
    std::list<Object*> options = room->Contents();
    std::list<Object*>::iterator opt = options.begin();
    for (; opt != options.end(); ++opt) {
      if ((*opt)->Matches(buf)) {
        if (dam > 0) {
          (*opt)->HitMent(1000, dam, 0);
          //	  fprintf(stderr, CGRN "#%d Debug: WDamage '%s', %d\n" CNRM,
          //		body->Skill("TBAScript"), (*opt)->Name(), dam
          //		);
        } else if (dam < 0) {
          (*opt)->HealStun(((-dam) + 1) / 2);
          (*opt)->HealPhys(((-dam) + 1) / 2);
          //	  fprintf(stderr, CGRN "#%d Debug: WHeal '%s', %d\n" CNRM,
          //		body->Skill("TBAScript"), (*opt)->Name(), ((-dam)+1)/2
          //		);
        }
      }
    }
  }

  else if (!strncasecmp(line.c_str(), "door ", 5)) {
    int rnum, tnum, len;
    char dname[16], xtra;
    const char* args = line.c_str() + 5;
    if (sscanf(args, "%d %s", &rnum, dname) < 2) {
      fprintf(
          stderr,
          CRED "#%d Error: short door command '%s'\n" CNRM,
          body->Skill("TBAScript"),
          line.c_str());
      Disable();
      return 1;
    }

    // Handle abbreviated standard directions.
    const char* dir = dname;
    if (!strncasecmp("north", dir, strlen(dir)))
      dir = "north";
    else if (!strncasecmp("south", dir, strlen(dir)))
      dir = "south";
    else if (!strncasecmp("east", dir, strlen(dir)))
      dir = "east";
    else if (!strncasecmp("west", dir, strlen(dir)))
      dir = "west";
    else if (!strncasecmp("up", dir, strlen(dir)))
      dir = "up";
    else if (!strncasecmp("down", dir, strlen(dir)))
      dir = "down";

    std::list<Object*> options = room->World()->Contents();
    std::list<Object*>::iterator opt = options.begin();
    room = nullptr;
    for (; opt != options.end(); ++opt) {
      int tnum = (*opt)->Skill("TBARoom");
      if (tnum > 0 && (tnum % 1000000) == rnum) {
        room = (*opt);
        break;
      }
    }
    if (!room) {
      fprintf(
          stderr,
          CRED "#%d Error: can't find target in '%s'\n" CNRM,
          body->Skill("TBAScript"),
          line.c_str());
      Disable();
      return 1;
    }

    Object* door = room->PickObject(dir, LOC_NINJA | LOC_INTERNAL);

    if (sscanf(args, "%*d %*s descriptio%c %n", &xtra, &len) >= 1) {
      //      fprintf(stderr, CGRN "#%d Debug: door redesc '%s'\n" CNRM,
      //	body->Skill("TBAScript"), line.c_str()
      //	);
      if (door)
        door->SetDesc(line.substr(len + 5));
    } else if (sscanf(args, "%*d %*s flag%c %n", &xtra, &len) >= 1) {
      if (!door) {
        fprintf(
            stderr,
            CRED "#%d Error: No %s door to reflag in '%s'\n" CNRM,
            body->Skill("TBAScript"),
            dir,
            line.c_str());
        Disable();
        return 1;
      }
      int newfl = tba_bitvec(line.substr(len));
      if ((newfl & 0xF) == 0) {
        fprintf(
            stderr,
            CRED "#%d Error: bad door reflag (%d) in '%s'\n" CNRM,
            body->Skill("TBAScript"),
            newfl,
            line.c_str());
        Disable();
        return 1;
      }
      if (newfl & 1) { // Can Open/Close
        door->SetSkill("Open", 1000);
        door->SetSkill("Closeable", 1);
        door->SetSkill("Locked", 0);
        door->SetSkill("Lockable", 1);
        door->SetSkill("Pickable", 4);
        //	fprintf(stderr, CGRN "#%d Debug: %s door can open/close in
        //'%s'\n" CNRM,
        //		body->Skill("TBAScript"), dir, line.c_str()
        //		);
      }
      if (newfl & 2) { // Closed
        door->SetSkill("Open", 0);
        //	fprintf(stderr, CGRN "#%d Debug: %s door is closed in '%s'\n"
        // CNRM,
        //		body->Skill("TBAScript"), dir, line.c_str()
        //		);
      }
      if (newfl & 4) { // Locked
        door->SetSkill("Locked", 1);
        door->SetSkill("Lockable", 1);
        door->SetSkill("Pickable", 4);
        //	fprintf(stderr, CGRN "#%d Debug: %s door is locked in '%s'\n"
        // CNRM,
        //		body->Skill("TBAScript"), dir, line.c_str()
        //		);
      }
      if (newfl & 8) { // Pick-Proof
        door->SetSkill("Pickable", 1000);
        //	fprintf(stderr, CGRN "#%d Debug: %s door is pick-proof in
        //'%s'\n" CNRM,
        //		body->Skill("TBAScript"), dir, line.c_str()
        //		);
      }
    } else if (sscanf(args, "%*d %*s nam%c %n", &xtra, &len) >= 1) {
      //      fprintf(stderr, CGRN "#%d Debug: door rename '%s'\n" CNRM,
      //	body->Skill("TBAScript"), line.c_str()
      //	);
      if (door) {
        std::string newname = door->ShortDesc();
        size_t end = newname.find("(");
        if (end != std::string::npos) {
          end = newname.find_last_not_of(" \t", end - 1);
          if (end != std::string::npos)
            newname = newname.substr(0, end);
        }
        door->SetShortDesc(newname + " (" + line.substr(len + 5) + ")");
      }
    } else if (sscanf(args, "%*d %*s room %d", &tnum) == 1) {
      //      fprintf(stderr, CGRN "#%d Debug: door relink '%s'\n" CNRM,
      //	body->Skill("TBAScript"), line.c_str()
      //	);
      if (door)
        door->Recycle();
      std::list<Object*>::iterator opt = options.begin();
      Object* toroom = nullptr;
      for (; opt != options.end(); ++opt) {
        int onum = (*opt)->Skill("TBARoom");
        if (tnum > 0 && (onum % 1000000) == tnum) {
          toroom = (*opt);
          break;
        }
      }
      if (!toroom) {
        fprintf(
            stderr,
            CRED "#%d Error: can't find dest in '%s'\n" CNRM,
            body->Skill("TBAScript"),
            line.c_str());
        Disable();
        return 1;
      }
      door = new Object(room);
      door->SetShortDesc(dir);
      Object* odoor = new Object(toroom);
      odoor->SetShortDesc("a passage exit");
      door->SetSkill("Enterable", 1);
      door->SetSkill("Open", 1000);
      door->AddAct(ACT_SPECIAL_LINKED, odoor);
      odoor->SetSkill("Enterable", 1);
      odoor->SetSkill("Open", 1000);
      odoor->SetSkill("Invisible", 1000);
      odoor->AddAct(ACT_SPECIAL_MASTER, door);
    } else if (sscanf(args, "%*d %*s key %d", &tnum) == 1) {
      if (!door) {
        fprintf(
            stderr,
            CRED "#%d Error: No %s door to re-key in '%s'\n" CNRM,
            body->Skill("TBAScript"),
            dir,
            line.c_str());
        Disable();
        return 1;
      }
      door->SetSkill("Lockable", 1);
      door->SetSkill("Key", 2000000 + tnum);
      if (door->Skill("Pickable") < 1)
        door->SetSkill("Pickable", 4);
      //      fprintf(stderr, CGRN "#%d Debug: %s door re-keyed (%d) in '%s'\n"
      //      CNRM,
      //	body->Skill("TBAScript"), dir, tnum, line.c_str()
      //	);
    } else if (sscanf(args, "%*d %*s purg%c", &xtra) == 1) {
      //      fprintf(stderr, CGRN "#%d Debug: door purge '%s'\n" CNRM,
      //	body->Skill("TBAScript"), line.c_str()
      //	);
      if (door)
        door->Recycle();
    } else {
      fprintf(
          stderr,
          CRED "#%d Error: bad door command '%s'\n" CNRM,
          body->Skill("TBAScript"),
          line.c_str());
      Disable();
      return 1;
    }
  }

  else if (!strncasecmp(line.c_str(), "transport ", 10)) {
    int dnum;
    int nocheck = 0;
    char buf[256];
    if (sscanf(line.c_str() + 10, "%s %d", buf, &dnum) != 2) {
      fprintf(
          stderr,
          CRED "#%d Error: Bad teleport line '%s'\n" CNRM,
          body->Skill("TBAScript"),
          line.c_str());
      Disable();
      return 1;
    }
    if (!strcasecmp(buf, "all")) {
      strcpy(buf, "everyone");
      nocheck = 1;
    }
    Object* dest = ovars["self"]->World();
    std::list<Object*> options = dest->Contents();
    std::list<Object*>::iterator opt = options.begin();
    dest = nullptr;
    for (; opt != options.end(); ++opt) {
      int tnum = (*opt)->Skill("TBARoom");
      if (tnum > 0 && (tnum % 1000000) == dnum) {
        dest = (*opt);
        break;
      }
    }
    options = room->Contents();
    opt = options.begin();
    for (; opt != options.end(); ++opt) {
      if ((*opt)->Matches(buf)) {
        (*opt)->Parent()->RemoveLink(*opt);
        (*opt)->SetParent(dest);
        nocheck = 1;
      }
    }
    if (!nocheck) { // Check for a MOB by that UNIQUE name ANYWHERE.
      room = room->Parent();
      Object* targ = room->PickObject((std::string("all's ") + buf).c_str(), LOC_INTERNAL);
      if (targ) {
        targ->Parent()->RemoveLink(targ);
        targ->SetParent(dest);
      }
    }
    //    fprintf(stderr, CGRN "#%d Debug: Transport line: '%s'\n" CNRM,
    //	body->Skill("TBAScript"), line.c_str()
    //	);
  }

  else if (!strncasecmp(line.c_str(), "purge", 5)) {
    Object* targ = nullptr;
    sscanf(line.c_str() + 5, " OBJ:%p", &targ);
    if (targ) {
      if (!is_pc(targ))
        targ->Recycle();
    } else if (line.length() <= 6) { // No Args: Nuke All (but dirs and PCs)!
      std::list<Object*> tokill = room->PickObjects("everyone", LOC_DARK | LOC_HEAT | LOC_INTERNAL);
      std::list<Object*> tokill2 =
          room->PickObjects("everything", LOC_DARK | LOC_HEAT | LOC_INTERNAL);
      tokill.splice(tokill.end(), tokill2);
      std::list<Object*>::iterator item = tokill.begin();
      for (; item != tokill.end(); ++item) {
        if (!is_pc(*item))
          (*item)->Recycle();
      }
    } else {
      fprintf(
          stderr,
          CRED "#%d Error: Bad purge target '%s'\n" CNRM,
          body->Skill("TBAScript"),
          line.c_str());
      return 1;
    }
    if (!body) { // No longer connected, must have purged self or parent.
      Disable();
      return 1;
    }
  }

  else if (!strncasecmp(line.c_str(), "load ", 5)) {
    int vnum, params, type, loc = 0, mask = 0;
    char buf[256] = "";
    char targ[256] = "";
    char where[256] = "";
    Object* dest = ovars["self"];
    Object* item = nullptr;
    params = sscanf(line.c_str() + 5, " %s %d %s %s", buf, &vnum, targ, where);
    type = tolower(buf[0]);
    if ((params != 2 && params != 4) || (type != 'o' && type != 'm')) {
      fprintf(
          stderr,
          CRED "#%d Error: Gibberish script line '%s'\n" CNRM,
          body->Skill("TBAScript"),
          line.c_str());
      Disable();
      return 1;
    }
    Object* src = room->World();
    if (type == 'o') {
      src = src->PickObject("TBAMUD Object Room", LOC_NINJA | LOC_INTERNAL);
      if (src == nullptr) {
        fprintf(
            stderr,
            CRED "#%d Error: Can't find Object room '%s'\n" CNRM,
            body->Skill("TBAScript"),
            line.c_str());
        return 1;
      }
      std::list<Object*> options = src->Contents();
      std::list<Object*>::iterator opt = options.begin();
      for (; opt != options.end(); ++opt) {
        if ((*opt)->Skill("TBAObject") == vnum + 2000000) {
          item = new Object(*(*opt));
          break;
        }
      }
    } else if (type == 'm') {
      dest = room;
      src = src->PickObject("TBAMUD MOB Room", LOC_NINJA | LOC_INTERNAL);
      if (src == nullptr) {
        fprintf(
            stderr,
            CRED "#%d Error: Can't find MOB room '%s'\n" CNRM,
            body->Skill("TBAScript"),
            line.c_str());
        return 1;
      }
      std::list<Object*> options = src->Contents();
      std::list<Object*>::iterator opt = options.begin();
      for (; opt != options.end(); ++opt) {
        if ((*opt)->Skill("TBAMOB") == vnum + 1000000) {
          item = new Object(*(*opt));
          break;
        }
      }
    }
    if (item == nullptr) {
      fprintf(
          stderr,
          CRED "#%d Error: Failed to find item '%s'\n" CNRM,
          body->Skill("TBAScript"),
          line.c_str());
      Disable();
      return 1;
    }
    if (params > 2) {
      //      fprintf(stderr, CGRN "#%d Debug: (%s) '%s'\n" CNRM,
      //	body->Skill("TBAScript"), targ, line.c_str());
      dest = room->PickObject(targ, LOC_NINJA | LOC_INTERNAL);
    }
    if (!dest) {
      fprintf(
          stderr,
          CRED "#%d Error: Can't find target in '%s'\n" CNRM,
          body->Skill("TBAScript"),
          line.c_str());
      Disable();
      return 1;
    }
    if (strcmp("rfinger", where) == 0 || strcmp("1", where) == 0) {
      mask = item->Skill("Wearable on Right Finger");
      loc = ACT_WEAR_RFINGER;
    } else if (strcmp("lfinger", where) == 0 || strcmp("2", where) == 0) {
      mask = item->Skill("Wearable on Left Finger");
      loc = ACT_WEAR_LFINGER;
    } else if (strcmp("neck1", where) == 0 || strcmp("3", where) == 0) {
      mask = item->Skill("Wearable on Neck");
      mask |= item->Skill("Wearable on Collar");
      loc = ACT_WEAR_NECK;
    } else if (strcmp("neck2", where) == 0 || strcmp("4", where) == 0) {
      mask = item->Skill("Wearable on Neck");
      mask |= item->Skill("Wearable on Collar");
      loc = ACT_WEAR_COLLAR;
    } else if (strcmp("body", where) == 0 || strcmp("5", where) == 0) {
      mask = item->Skill("Wearable on Chest");
      mask &= item->Skill("Wearable on Back"); // Both
      loc = ACT_WEAR_CHEST;
    } else if (strcmp("head", where) == 0 || strcmp("6", where) == 0) {
      mask = item->Skill("Wearable on Head");
      loc = ACT_WEAR_HEAD;
    } else if (strcmp("legs", where) == 0 || strcmp("7", where) == 0) {
      mask = item->Skill("Wearable on Left Leg");
      mask |= item->Skill("Wearable on Right Leg");
      loc = ACT_WEAR_LLEG;
    } else if (strcmp("feet", where) == 0 || strcmp("8", where) == 0) {
      mask = item->Skill("Wearable on Left Foot");
      mask |= item->Skill("Wearable on Right Foot");
      loc = ACT_WEAR_LFOOT;
    } else if (strcmp("hands", where) == 0 || strcmp("9", where) == 0) {
      mask = item->Skill("Wearable on Left Hand");
      mask |= item->Skill("Wearable on Right Hand");
      loc = ACT_WEAR_LHAND;
    } else if (strcmp("arms", where) == 0 || strcmp("10", where) == 0) {
      mask = item->Skill("Wearable on Left Arm");
      mask |= item->Skill("Wearable on Right Arm");
      loc = ACT_WEAR_LARM;
    } else if (strcmp("shield", where) == 0 || strcmp("11", where) == 0) {
      mask = item->Skill("Wearable on Shield");
      loc = ACT_WEAR_SHIELD;
    } else if (strcmp("about", where) == 0 || strcmp("12", where) == 0) {
      mask = item->Skill("Wearable on Left Shoulder");
      mask &= item->Skill("Wearable on Right Shoulder"); // Both
      loc = ACT_WEAR_LSHOULDER;
    } else if (strcmp("waist", where) == 0 || strcmp("13", where) == 0) {
      mask = item->Skill("Wearable on Waist");
      loc = ACT_WEAR_WAIST;
    } else if (strcmp("rwrist", where) == 0 || strcmp("14", where) == 0) {
      mask = item->Skill("Wearable on Right Wrist");
      loc = ACT_WEAR_RWRIST;
    } else if (strcmp("lwrist", where) == 0 || strcmp("15", where) == 0) {
      mask = item->Skill("Wearable on Left Wrist");
      loc = ACT_WEAR_LWRIST;
    } else if (strcmp("wield", where) == 0 || strcmp("16", where) == 0) {
      loc = ACT_WIELD;
    } else if (strcmp("light", where) == 0 || strcmp("0", where) == 0) {
      loc = ACT_HOLD;
    } else if (strcmp("hold", where) == 0 || strcmp("17", where) == 0) {
      loc = ACT_HOLD;
    } else if (strcmp("inv", where) == 0 || strcmp("18", where) == 0) {
    } else if (params > 2) {
      fprintf(
          stderr,
          CRED "#%d Error: Unsupported dest '%s'\n" CNRM,
          body->Skill("TBAScript"),
          line.c_str());
      Disable();
      return 1;
    }
    item->SetParent(dest);
    if (loc != 0 && loc != ACT_HOLD && loc != ACT_WIELD) { // Wear it
      if (!dest->Wear(item, mask))
        dest->StashOrDrop(item);
    } else if (loc == ACT_WIELD) { // Wield it
      if (dest->ActTarg(ACT_WIELD) == nullptr) {
        dest->AddAct(ACT_WIELD, item);
      } else {
        dest->StashOrDrop(item);
      }
    } else if (loc == ACT_HOLD) { // Hold it
      if (dest->ActTarg(ACT_HOLD) == nullptr ||
          dest->ActTarg(ACT_HOLD) == dest->ActTarg(ACT_WIELD) ||
          dest->ActTarg(ACT_HOLD) == dest->ActTarg(ACT_WEAR_SHIELD)) {
        dest->AddAct(ACT_HOLD, item);
      } else {
        dest->StashOrDrop(item);
      }
    } else if (dest != room) { // Have it
      dest->StashOrDrop(item);
    }
  }

  else if (!strncasecmp(line.c_str(), "case ", 5)) {
    // Ignore these, as we only hit them here when when running over them
  } else if (!strncasecmp(line.c_str(), "default", 7)) {
    // Ignore these, as we only hit them here when when running over them
  } else if (!strncasecmp(line.c_str(), "end", 3)) {
    // Ignore these, as we only hit them here when we're running inside if
  } else if (!strncasecmp(line.c_str(), "nop ", 4)) {
    // Ignore these, as the varsub should have done all that's needed
  } else if (!strncasecmp(line.c_str(), "done", 4)) {
    // Means we should be within a while(), pop up a level.
    if (spos_s.size() < 2) {
      fprintf(
          stderr,
          CRED "#%d Error: Not in while/switch, but '%s'\n" CNRM,
          body->Skill("TBAScript"),
          line.c_str());
      Disable();
      return 1;
    }
    spos_s.pop_front();
  } else if (!strncasecmp(line.c_str(), "return ", 7)) {
    int retval = TBAEval(line.c_str() + 7);
    if (retval == 0) {
      status = 1; // Set special state
    }
  } else if (!strncasecmp(line.c_str(), "halt", 4)) {
    Disable();
    return 1;
  }
  // Player commands Acid shares with TBA, requiring arguments
  else if (
      com == COM_SAY || com == COM_SHOUT || com == COM_EMOTE || com == COM_LOCK ||
      com == COM_UNLOCK || com == COM_OPEN || com == COM_CLOSE || com == COM_GET ||
      com == COM_DROP) {
    size_t stuff = line.find_first_of(" ");
    if (stuff != std::string::npos) {
      stuff = line.find_first_not_of(" \t\r\n", stuff);
    }
    if (stuff != std::string::npos) {
      handle_command(ovars["self"], line.c_str());
    } else {
      fprintf(
          stderr, CRED "#%d Error: Told just '%s'\n" CNRM, body->Skill("TBAScript"), line.c_str());
      Disable();
      return 1;
    }
  }

  // Player commands Acid shares with TBA, not requiring arguments
  else if (
      com == COM_NORTH || com == COM_SOUTH || com == COM_EAST || com == COM_WEST || com == COM_UP ||
      com == COM_DOWN || com == COM_SOCIAL || com == COM_SLEEP || com == COM_REST ||
      com == COM_WAKE || com == COM_STAND || com == COM_SIT || com == COM_LIE || com == COM_LOOK ||
      com == COM_FLEE) {
    handle_command(ovars["self"], line.c_str());
  }

  // Trigger-Supported (only) commands (not shared with real acid commands).
  else if (com == COM_NONE && handle_command(ovars["self"], line.c_str()) != 1) {
    // Do Nothing, as handle_command already did it.
  }

  else {
    fprintf(
        stderr,
        CRED "#%d Error: Gibberish script line '%s'\n" CNRM,
        body->Skill("TBAScript"),
        line.c_str());
    Disable();
    return 1;
  }
  return 0;
}

static const char* dirnames[4] = {"north", "south", "east", "west"};
static const char* items[8] = {"Food",
                               "Hungry", // Order is Most to Least Important
                               "Rest",
                               "Tired",
                               "Fun",
                               "Bored",
                               "Stuff",
                               "Needy"};
void Mind::Think(int istick) {
  if (type == MIND_MOB) {
    if (body->Skill("Personality") & 1) { // Group Mind
      //      body->TryCombine();	// I AM a group, after all.
      int qty = body->Skill("Quantity"), req = -1;
      if (qty < 1)
        qty = 1;
      for (int item = 0; item < 8; item += 2) {
        if (body->Parent()->Skill(items[item])) {
          int val = body->Skill(items[item + 1]);
          val -= body->Parent()->Skill(items[item]) * qty * 100;
          if (val < 0)
            val = 0;
          body->SetSkill(items[item + 1], val);
        } else {
          if ((item & 2) == 0) { // Food & Fun grow faster
            body->SetSkill(items[item + 1], body->Skill(items[item + 1]) + qty * 2);
          } else {
            body->SetSkill(items[item + 1], body->Skill(items[item + 1]) + qty);
          }
        }
        if (req < 0 && body->Skill(items[item + 1]) >= 10000) {
          req = item;
        }
      }
      if (req >= 0) { // Am I already getting what I most need?
        if (body->Parent()->Skill(items[req]) > 0)
          req = -1;
      }
      if (req >= 0) {
        std::vector<const char*> dirs;
        for (int i = 0; i < 4; ++i) {
          Object* dir = body->PickObject(dirnames[i], LOC_NEARBY);
          if (dir && dir->Skill("Open") > 0) {
            dirs.push_back(dirnames[i]);
          }
        }
        random_shuffle(dirs.begin(), dirs.end());

        int orig = body->Skill(items[req + 1]);
        int leave = orig / 10000;
        if (dirs.size() > 0 && leave >= qty) {
          body->BusyFor(0, dirs.front());
        } else {
          if (dirs.size() > 0) {
            Object* trav = body->Split(leave);
            trav->SetSkill(items[req + 1], leave * 10000);
            body->SetSkill(items[req + 1], orig - leave * 10000);
            trav->BusyFor(0, dirs.front());
          }
          body->BusyFor(8000 + (rand() & 0xFFF));
        }
      } else {
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
  } else if (type == MIND_TBATRIG) {
    if (body && body->Parent() && spos_s.size() > 0 && spos_s.front() < script.length()) {
      //      fprintf(stderr, CGRN "#%d Debug: Running Trigger.\n" CNRM,
      //	body->Skill("TBAScript")
      //	);
      ovars["self"] = body->Parent();
      ovars["context"] = ovars["self"]; // Initial global var context

      int quota = 1024;
      int stype = body->Skill("TBAScriptType");
      while (spos_s.size() > 0 && spos_s.front() != std::string::npos) {
        std::string line;
        size_t endl = script.find_first_of("\n\r", spos_s.front());
        if (endl == std::string::npos)
          line = script.substr(spos_s.front());
        else
          line = script.substr(spos_s.front(), endl - spos_s.front());

        spos_s.front() = skip_line(script, spos_s.front());

        if (line[0] == '*')
          continue; // Comments

        PING_QUOTA();

        if (TBARunLine(line))
          return;
      }
      if (type == MIND_MORON) { // Disabled
        return;
      }
      if (stype & 2) { // Random Triggers
        int chance = body->Skill("TBAScriptNArg"); // Percent Chance
        if (chance > 0) {
          int delay = 13000; // Next try in 13 seconds.
          while (delay < 13000000 && (rand() % 100) >= chance)
            delay += 13000;
          spos_s.clear();
          spos_s.push_front(0); // We never die!
          Suspend(delay); // We'll be back!
          return;
        }
      }
    }
    Disable();
  } else if (type == MIND_TBAMOB) {
    if ((!body) || (istick >= 0 && body->StillBusy()))
      return;

    // Temporary
    if (body && body->ActTarg(ACT_WEAR_SHIELD) && (!body->IsAct(ACT_HOLD))) {
      std::string command = std::string("hold ") + body->ActTarg(ACT_WEAR_SHIELD)->ShortDesc();
      body->BusyFor(500, command.c_str());
      return;
    } else if (
        body && body->ActTarg(ACT_WEAR_SHIELD) && body->ActTarg(ACT_HOLD) &&
        body->ActTarg(ACT_WEAR_SHIELD) != body->ActTarg(ACT_HOLD)) {
      Object* targ = body->ActTarg(ACT_HOLD);
      if (body->Stash(targ, 0)) {
        if (body->Parent())
          body->Parent()->SendOut(ALL, 0, ";s stashes ;s.\n", "", body, targ);
        std::string command = std::string("hold ") + body->ActTarg(ACT_WEAR_SHIELD)->ShortDesc();
        body->BusyFor(500, command.c_str());
      } else {
        // fprintf(stderr, "Warning: %s can't use his shield!\n", body->Name());
      }
      return;
    }

    // AGGRESSIVE and WIMPY TBA Mobs
    if (body && body->Parent() && (body->Skill("TBAAction") & 160) == 160 &&
        (!body->IsAct(ACT_FIGHT))) {
      auto others = body->PickObjects("everyone", LOC_NEARBY);
      for (auto other = others.begin(); other != others.end(); ++other) {
        if ((!(*other)->Skill("TBAAction")) // FIXME: Other mobs?
            && body->Stun() < 6 // I'm not stunned
            && body->Phys() < 6 // I'm not injured
            && (!body->IsAct(ACT_SLEEP)) // I'm not asleep
            && (!body->IsAct(ACT_REST)) // I'm not resting
            && (*other)->IsAct(ACT_SLEEP) // It's not awake (wuss!)
            && (*other)->Attribute(1) // It's not a rock
            && (!(*other)->IsAct(ACT_UNCONSCIOUS)) // It's not already KOed
            && (!(*other)->IsAct(ACT_DYING)) // It's not already dying
            && (!(*other)->IsAct(ACT_DEAD)) // It's not already dead
        ) {
          std::string command = std::string("attack ") + (*other)->ShortDesc();
          body->BusyFor(500, command.c_str());
          // fprintf(stderr, "%s: Tried '%s'\n", body->ShortDesc(),
          // command.c_str());
          return;
        }
      }
      if (istick == 1 && body->IsUsing("Perception")) {
        body->BusyFor(500, "stop");
      } else if (
          istick == 0 // Triggered Only
          && (!body->IsUsing("Perception")) // Not already searching
          && (!body->StillBusy()) // Not already responding
          && body->Stun() < 6 // I'm not stunned
          && body->Phys() < 6 // I'm not injured
          && (!body->IsAct(ACT_SLEEP)) // I'm not asleep
          && (!body->IsAct(ACT_REST)) // I'm not resting
      ) {
        body->BusyFor(500, "search");
      }
    }
    // AGGRESSIVE and (!WIMPY) TBA Mobs
    else if (
        body && body->Parent() && (body->Skill("TBAAction") & 160) == 32 &&
        (!body->IsAct(ACT_FIGHT))) {
      auto others = body->PickObjects("everyone", LOC_NEARBY);
      for (auto other = others.begin(); other != others.end(); ++other) {
        if ((!(*other)->Skill("TBAAction")) // FIXME: Other mobs?
            && body->Stun() < 6 // I'm not stunned
            && body->Phys() < 6 // I'm not injured
            && (!body->IsAct(ACT_SLEEP)) // I'm not asleep
            && (!body->IsAct(ACT_REST)) // I'm not resting
            && (*other)->Attribute(1) // It's not a rock
            && (!(*other)->IsAct(ACT_UNCONSCIOUS)) // It's not already KOed
            && (!(*other)->IsAct(ACT_DYING)) // It's not already dying
            && (!(*other)->IsAct(ACT_DEAD)) // It's not already dead
        ) {
          std::string command = std::string("attack ") + (*other)->ShortDesc();
          body->BusyFor(500, command.c_str());
          // fprintf(stderr, "%s: Tried '%s'\n", body->ShortDesc(),
          // command.c_str());
          return;
        }
      }
      if (istick == 1 && body->IsUsing("Perception")) {
        body->BusyFor(500, "stop");
      } else if (
          istick == 0 // Triggered Only
          && (!body->IsUsing("Perception")) // Not already searching
          && (!body->StillBusy()) // Not already responding
          && body->Stun() < 6 // I'm not stunned
          && body->Phys() < 6 // I'm not injured
          && (!body->IsAct(ACT_SLEEP)) // I'm not asleep
          && (!body->IsAct(ACT_REST)) // I'm not resting
      ) {
        body->BusyFor(500, "search");
      }
    }
    // HELPER TBA Mobs
    if (body && body->Parent() && (body->Skill("TBAAction") & 4096) && (!body->IsAct(ACT_FIGHT))) {
      auto others = body->PickObjects("everyone", LOC_NEARBY);
      for (auto other = others.begin(); other != others.end(); ++other) {
        if ((!(*other)->Skill("TBAAction")) // FIXME: Other mobs?
            && body->Stun() < 6 // I'm not stunned
            && body->Phys() < 6 // I'm not injured
            && (!body->IsAct(ACT_SLEEP)) // I'm not asleep
            && (!body->IsAct(ACT_REST)) // I'm not resting
            && (*other)->Attribute(1) // It's not a rock
            && (!(*other)->IsAct(ACT_DEAD)) // It's not already dead
            && (*other)->IsAct(ACT_FIGHT) // It's figting someone
            && (*other)->ActTarg(ACT_FIGHT)->HasSkill("TBAAction")
            //...against another MOB
        ) {
          std::string command = std::string("call ALARM; attack ") + (*other)->ShortDesc();
          body->BusyFor(500, command.c_str());
          // fprintf(stderr, "%s: Tried '%s'\n", body->ShortDesc(),
          // command.c_str());
          return;
        }
      }
      if (!body->IsUsing("Perception")) { // Don't let guard down!
        body->BusyFor(500, "search");
      } else if (
          istick == 1 // Perioidic searching
          && (!body->StillBusy()) // Not already responding
          && body->Stun() < 6 // I'm not stunned
          && body->Phys() < 6 // I'm not injured
          && (!body->IsAct(ACT_SLEEP)) // I'm not asleep
          && (!body->IsAct(ACT_REST)) // I'm not resting
      ) {
        body->BusyFor(500, "search");
      }
    }
    // NON-SENTINEL TBA Mobs
    if (body && body->Parent() && ((body->Skill("TBAAction") & 2) == 0) &&
        (!body->IsAct(ACT_FIGHT)) && (istick == 1) && (!body->IsAct(ACT_REST)) &&
        (!body->IsAct(ACT_SLEEP)) && body->Stun() < 6 && body->Phys() < 6 &&
        body->Roll("Willpower", 9)) {
      std::map<Object*, const char*> cons;
      cons[body->PickObject("north", LOC_NEARBY)] = "north";
      cons[body->PickObject("south", LOC_NEARBY)] = "south";
      cons[body->PickObject("east", LOC_NEARBY)] = "east";
      cons[body->PickObject("west", LOC_NEARBY)] = "west";
      cons[body->PickObject("up", LOC_NEARBY)] = "up";
      cons[body->PickObject("down", LOC_NEARBY)] = "down";
      cons.erase(nullptr);

      std::map<Object*, const char*> cons2 = cons;
      std::map<Object*, const char*>::iterator dir = cons2.begin();
      for (; dir != cons2.end(); ++dir) {
        if ((!dir->first->ActTarg(ACT_SPECIAL_LINKED)) ||
            (!dir->first->ActTarg(ACT_SPECIAL_LINKED)->Parent())) {
          cons.erase(dir->first);
          continue;
        }

        Object* dest = dir->first->ActTarg(ACT_SPECIAL_LINKED)->Parent();
        if (!TBACanWanderTo(dest)) {
          cons.erase(dir->first);
        }
      }

      if (cons.size()) {
        int res = rand() % cons.size();
        std::map<Object*, const char*>::iterator dir = cons.begin();
        while (res > 0) {
          ++dir;
          --res;
        }
        if (body->StillBusy()) { // Already doing something (from above)
          body->DoWhenFree(dir->second);
        } else {
          body->BusyFor(500, dir->second);
        }
        return;
      }
    }
  }
}

int Mind::TBACanWanderTo(Object* dest) {
  if (dest->Skill("TBAZone") == 999999) { // NO_MOBS TBA Zone
    return 0; // Don't Enter NO_MOBS Zone!
  } else if (body->Skill("TBAAction") & 64) { // STAY_ZONE TBA MOBs
    if (dest->Skill("TBAZone") != body->Parent()->Skill("TBAZone")) {
      return 0; // Don't Leave Zone!
    }
  } else if (body->Skill("Swimming") == 0) { // Can't Swim?
    if (dest->Skill("WaterDepth") == 1) {
      return 0; // Can't swim!
    }
  } else if (!(body->Skill("TBAAffection") & 64)) { // Can't Waterwalk?
    if (dest->Skill("WaterDepth") >= 1) { // Need boat!
      return 0; // FIXME: Have boat?
    }
  }
  return 1;
}

void Mind::SetSpecialPrompt(const char* newp) {
  prompt = std::string(newp);
  UpdatePrompt();
}

const char* Mind::SpecialPrompt() {
  return prompt.c_str();
}

Mind* new_mind(int tp, Object* obj, Object* obj2, Object* obj3, std::string text) {
  Mind* m = nullptr;
  if (recycle_bin.size() > 0) {
    m = recycle_bin.front();
    recycle_bin.pop_front();
    m->ClearStatus(); // Clear special state!
    //    fprintf(stderr, "Suspending(%p)\n", m);
  } else {
    m = new Mind();
  }
  if (tp == MIND_TBATRIG && obj) {
    m->SetTBATrigger(obj, obj2, obj3, text);
  } else if (obj) {
    m->Attach(obj);
  }
  return m;
}

int new_trigger(int msec, Object* obj, Object* tripper, std::string text) {
  return new_trigger(msec, obj, tripper, nullptr, text);
}

int new_trigger(int msec, Object* obj, Object* tripper, Object* targ, std::string text) {
  if ((!obj) || (!(obj->Parent())))
    return 0;

  Mind* m = new_mind(MIND_TBATRIG, obj, tripper, targ, text);
  if (msec == 0) {
    m->Think(1);
    if (m->Status())
      return 1; // Catch non-default term state.
  } else {
    m->Suspend(msec);
  }
  return 0;
}

std::list<std::pair<int, Mind*>> Mind::waiting;
void Mind::Suspend(int msec) {
  //  fprintf(stderr, "Suspening(%p)\n", this);
  waiting.push_back(std::make_pair(msec, this));
}

void Mind::Disable() {
  //  fprintf(stderr, "Disabled(%p)\n", this);
  if (type == MIND_REMOTE)
    close_socket(pers);
  type = MIND_MORON;
  Unattach();
  if (log >= 0)
    close(log);
  log = -1;
  std::list<std::pair<int, Mind*>>::iterator cur = waiting.begin();
  while (cur != waiting.end()) {
    std::list<std::pair<int, Mind*>>::iterator tmp = cur;
    ++cur; // Inc cur first, in case I erase tmp.
    if (tmp->second == this) {
      waiting.erase(tmp);
    }
  }
  svars = cvars; // Reset all variables
  ovars.clear();
  recycle_bin.push_back(this); // Ready for re-use
}

void Mind::Resume(int passed) {
  std::list<std::pair<int, Mind*>>::iterator cur = waiting.begin();
  while (cur != waiting.end()) {
    std::list<std::pair<int, Mind*>>::iterator tmp = cur;
    ++cur; // Inc cur first, in case I erase tmp.
    if (tmp->first <= passed) {
      Mind* mind = tmp->second;
      waiting.erase(tmp);
      //      fprintf(stderr, "Resuming(%p)\n", mind);
      mind->Think(0);
    } else {
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

static const std::string blank = "";
void Mind::SetSVar(const std::string& var, const std::string& val) {
  svars[var] = val;
}

void Mind::ClearSVar(const std::string& var) {
  svars.erase(var);
}

const std::string& Mind::SVar(const std::string& var) const {
  if (svars.count(var) <= 0)
    return blank;
  return svars.at(var);
}

int Mind::IsSVar(const std::string& var) const {
  return (svars.count(var) > 0);
}

const std::map<std::string, std::string> Mind::SVars() const {
  return svars;
}

void Mind::SetSVars(const std::map<std::string, std::string>& sv) {
  svars = sv;
}
