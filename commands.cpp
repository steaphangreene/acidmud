#include <cstdio>
#include <ctype.h>
#include <cstring>
#include <unistd.h>

#include "net.h"
#include "main.h"
#include "mind.h"
#include "stats.h"
#include "object.h"
#include "commands.h"

#define SIT_ETHEREAL	1
#define SIT_CORPOREAL	2
#define SIT_ALIVE	4
#define SIT_CONSCIOUS	8
#define SIT_AWAKE	16
#define SIT_ALERT	32
#define SIT_LIE		64
#define SIT_SIT		128
#define SIT_STAND	256
#define SIT_ACTION	4096
#define SIT_NINJAMODE	8192
#define SIT_NINJA	16384
#define SIT_SUPERNINJA	32768

#define REQ_ETHEREAL	(SIT_ETHEREAL)
#define REQ_CORPOREAL	(SIT_CORPOREAL)
#define REQ_ALIVE	(SIT_ALIVE | REQ_CORPOREAL)
#define REQ_CONSCIOUS	(SIT_CONSCIOUS | REQ_ALIVE)
#define REQ_AWAKE	(SIT_AWAKE | REQ_CONSCIOUS)
#define REQ_ALERT	(SIT_ALERT | REQ_AWAKE)
#define REQ_LIE		(SIT_LIE | REQ_CORPOREAL)
#define REQ_SIT		(SIT_SIT | REQ_CORPOREAL)
#define REQ_STAND	(SIT_STAND | REQ_CORPOREAL)
#define REQ_ACTION	(SIT_ACTION | REQ_CORPOREAL)
#define REQ_NINJAMODE	(SIT_NINJAMODE)
#define REQ_NINJA	(SIT_NINJA)
#define REQ_SUPERNINJA	(SIT_SUPERNINJA|SIT_NINJA)

#define REQ_ANY		(SIT_CORPOREAL|SIT_ETHEREAL)

struct Command {
  int id;
  char *command;
  char *shortdesc;
  char *longdesc;
  int sit;
  };

enum {	COM_HELP=0,
	COM_QUIT,

	COM_LOOK,
	COM_EXAMINE,

	COM_OPEN,
	COM_CLOSE,

	COM_GET,
	COM_PUT,
	COM_DROP,
	COM_WIELD,
	COM_HOLD,
	COM_WEAR,
	COM_REMOVE,

	COM_LEAVE,
	COM_ENTER,
	COM_GO,

	COM_SLEEP,
	COM_WAKE,
	COM_LIE,
	COM_REST,
	COM_SIT,
	COM_STAND,

	COM_SAY,
	COM_EMOTE,

	COM_POINT,
	COM_ATTACK,
	COM_KILL,
	COM_PUNCH,
	COM_KICK,

	COM_USERS,
	COM_CHARS,
	COM_WHO,
	COM_OOC,
	COM_NEWBIE,

	COM_NEWCHAR,
	COM_SKILLLIST,

	COM_NINJAMODE,
	COM_MAKENINJA,
	COM_MAKESUPERNINJA,

	COM_RESET,
	COM_CREATE,
	COM_COMMAND,
	COM_CONTROL,
	COM_CLONE,
	COM_JUNK,
	COM_HEAL,
	COM_JACK,
	COM_CHUMP,
	COM_INCREMENT,
	COM_DECREMENT,

	COM_SHUTDOWN,
	COM_RESTART,
	COM_MAKESTART,
	COM_NAME,
	COM_DESCRIBE,
	COM_DEFINE,

	COM_CLOAD,
	COM_CCLEAN,
	};

Command comlist[] = {
  { COM_HELP, "help",
    "Get help for a topic or command.",
    "Get help for a topic or command.",
    (REQ_ANY)
    },
  { COM_QUIT, "quit",
    "Quit the game.",
    "Quit the game.",
    (REQ_ANY)
    },

  { COM_LOOK, "look",
    "Look around, look in a direction, or look at an object or creature.",
    "Look around, look in a direction, or look at an object or creature.",
    (REQ_AWAKE|REQ_ACTION|REQ_ETHEREAL)
    },
  { COM_EXAMINE, "examine",
    "Examine an object or creature.",
    "Examine an object or creature.",
    (REQ_ALERT|REQ_ACTION)
    },

  { COM_OPEN, "open",
    "Open a door or container.",
    "Open a door or container.",
    (REQ_ALERT|REQ_STAND|REQ_ACTION)
    },
  { COM_CLOSE, "close",
    "Close a door or container.",
    "Close a door or container.",
    (REQ_ALERT|REQ_STAND|REQ_ACTION)
    },

  { COM_GET, "get",
    "Get an item from your surroundings.",
    "Get an item from your surroundings.",
    (REQ_ALERT|REQ_STAND|REQ_ACTION)
    },
  { COM_PUT, "put",
    "Put a held item in or on something.",
    "Put a held item in or on something.",
    (REQ_ALERT|REQ_STAND|REQ_ACTION)
    },
  { COM_DROP, "drop",
    "Drop an item you are carrying.",
    "Drop an item you are carrying.",
    (REQ_ALERT|REQ_ACTION)
    },
  { COM_WIELD, "wield",
    "Wield a weapon you are carrying.",
    "Wield a weapon you are carrying.",
    (REQ_ALERT|REQ_ACTION)
    },
  { COM_HOLD, "hold",
    "Hold an item you are carrying.",
    "Hold an item you are carrying.",
    (REQ_ALERT|REQ_ACTION)
    },
  { COM_WEAR, "wear",
    "Wear an item you are carrying.",
    "Wear an item you are carrying.",
    (REQ_ALERT|REQ_ACTION)
    },
  { COM_REMOVE, "remove",
    "Remove an item you are wearing.",
    "Remove an item you are wearing.",
    (REQ_ALERT|REQ_ACTION)
    },

  { COM_LEAVE, "leave",
    "Leave an object.",
    "Leave an object.",
    (REQ_ALERT|REQ_STAND|REQ_ACTION)
    },
  { COM_ENTER, "enter",
    "Enter an object (or enter the game).",
    "Enter an object (or enter the game).",
    (REQ_ETHEREAL|REQ_ALERT|REQ_STAND|REQ_ACTION)
    },
  { COM_GO, "go",
    "Go in a direction (or just use 'north', 'up', 'west', etc...).",
    "Go in a direction (or just use 'north', 'up', 'west', etc...).",
    (REQ_ALERT|REQ_STAND|REQ_ACTION)
    },

  { COM_SLEEP, "sleep",
    "Go to sleep.",
    "Go to sleep.",
    (REQ_CONSCIOUS|REQ_ACTION)
    },
  { COM_WAKE, "wake",
    "Wake up, or wake someone else up.",
    "Wake up, or wake someone else up.",
    (REQ_CONSCIOUS|REQ_ACTION)
    },
  { COM_LIE, "lie",
    "Lie down.",
    "Lie down.",
    (REQ_CONSCIOUS|REQ_ACTION)
    },
  { COM_REST, "rest",
    "Relax and rest.",
    "Relax and rest.",
    (REQ_CONSCIOUS|REQ_ACTION)
    },
  { COM_SIT, "sit",
    "Sit down.",
    "Sit down.",
    (REQ_CONSCIOUS|REQ_ACTION)
    },
  { COM_STAND, "stand",
    "Stand up.",
    "Stand up.",
    (REQ_CONSCIOUS|REQ_ACTION)
    },

  { COM_SAY, "say",
    "Say something.",
    "Say something.",
    (REQ_AWAKE)
    },
  { COM_EMOTE, "emote",
    "Indicate to others that you are doing something.",
    "Indicate to others that you are doing something.",
    (REQ_AWAKE)
    },

  { COM_POINT, "point",
    "Point at an object, or stop pointing.",
    "Point at an object, or stop pointing.",
    (REQ_ALERT)
    },
  { COM_ATTACK, "attack",
    "Attack somebody, or an object.",
    "Attack somebody, or an object.",
    (REQ_ALERT|REQ_ACTION|REQ_STAND)
    },
  { COM_KILL, "kill",
    "Try to kill somebody, or smash an object.",
    "Try to kill somebody, or smash an object.",
    (REQ_ALERT|REQ_ACTION|REQ_STAND)
    },
  { COM_PUNCH, "punch",
    "Punch somebody, or an object.",
    "Punch somebody, or an object.",
    (REQ_ALERT|REQ_ACTION|REQ_STAND)
    },
  { COM_KICK, "kick",
    "Kick somebody, or an object.",
    "Kick somebody, or an object.",
    (REQ_ALERT|REQ_ACTION|REQ_STAND)
    },

  { COM_WHO, "who",
    "Get a list of who is on the mud right now.",
    "Get a list of who is on the mud right now.",
    (REQ_ANY)
    },
  { COM_OOC, "ooc",
    "Toggle or use ooc (Out-Of-Character) chat.",
    "Toggle or use ooc (Out-Of-Character) chat.",
    (REQ_ANY)
    },
  { COM_NEWBIE, "newbie",
    "Toggle or use newbie (New Player Help) chat.",
    "Toggle or use newbie (New Player Help) chat.",
    (REQ_ANY)
    },

  { COM_NEWCHAR, "newchar",
    "Create a new character.",
    "Create a new character.",
    (REQ_ETHEREAL)
    },
  { COM_SKILLLIST, "skilllist",
    "Create a new character.",
    "Create a new character.",
    (REQ_ANY)
    },

  { COM_NINJAMODE, "ninjamode", 
    "Ninja command: run a command in Ninja Mode[TM].",
    "Ninja command: run a command in Ninja Mode[TM].",
    (REQ_ANY|REQ_ACTION|REQ_NINJA)
    },
  { COM_MAKENINJA, "makeninja", 
    "Ninja command: make (or unmake) another player into a True Ninja[TM].",
    "Ninja command: make (or unmake) another player into a True Ninja[TM].",
    (REQ_ANY|REQ_SUPERNINJA|REQ_NINJAMODE)
    },
  { COM_MAKESUPERNINJA, "makesuperninja", 
    "Ninja command: make (or unmake) a True Ninja into a Super Ninja[TM].",
    "Ninja command: make (or unmake) a True Ninja into a Super Ninja[TM].",
    (REQ_ANY|REQ_SUPERNINJA|REQ_NINJAMODE)
    },

  { COM_CREATE, "create",
    "Ninja command.",
    "Ninja command - ninjas only!",
    (REQ_ALERT|REQ_NINJAMODE)
    },
  { COM_COMMAND, "command",
    "Ninja command.",
    "Ninja command - ninjas only!",
    (REQ_CORPOREAL|REQ_NINJAMODE)
    },
  { COM_CONTROL, "control",
    "Ninja command.",
    "Ninja command - ninjas only!",
    (REQ_CORPOREAL|REQ_NINJAMODE)
    },
  { COM_CLONE, "clone",
    "Ninja command.",
    "Ninja command - ninjas only!",
    (REQ_ALERT|REQ_NINJAMODE)
    },
  { COM_JUNK, "junk",
    "Ninja command.",
    "Ninja command - ninjas only!",
    (REQ_ALERT|REQ_NINJAMODE)
    },
  { COM_RESET, "reset",
    "Ninja command.",
    "Ninja command - ninjas only!",
    (REQ_ALERT|REQ_NINJAMODE)
    },

  { COM_USERS, "users",
    "Ninja command.",
    "Ninja command - ninjas only!",
    (REQ_ANY|REQ_NINJAMODE)
    },
  { COM_CHARS, "chars",
    "Ninja command.",
    "Ninja command - ninjas only!",
    (REQ_ANY|REQ_NINJAMODE)
    },

  { COM_HEAL, "heal",
    "Ninja command.",
    "Ninja command - ninjas only!",
    (REQ_CORPOREAL|REQ_NINJAMODE)
    },
  { COM_JACK, "jack",
    "Ninja command.",
    "Ninja command - ninjas only!",
    (REQ_ALERT|REQ_NINJAMODE)
    },
  { COM_CHUMP, "chump",
    "Ninja command.",
    "Ninja command - ninjas only!",
    (REQ_ALERT|REQ_NINJAMODE)
    },
  { COM_INCREMENT, "increment",
    "Ninja command.",
    "Ninja command - ninjas only!",
    (REQ_ALERT|REQ_NINJAMODE)
    },
  { COM_DECREMENT, "decrement",
    "Ninja command.",
    "Ninja command - ninjas only!",
    (REQ_ALERT|REQ_NINJAMODE)
    },

  { COM_SHUTDOWN, "shutdown", 
    "Ninja command: shutdown the entire mud.",
    "Ninja command: shutdown the entire mud.",
    (REQ_ANY|REQ_NINJAMODE)
    },
  { COM_RESTART, "restart", 
    "Ninja command: restart the entire mud - preserving connections.",
    "Ninja command: restart the entire mud - preserving connections.",
    (REQ_ANY|REQ_NINJAMODE)
    },
  { COM_MAKESTART, "makestart",
    "Ninja command.",
    "Ninja command - ninjas only!",
    (REQ_ALERT|REQ_NINJAMODE)
    },
  { COM_NAME, "name",
    "Ninja command.",
    "Ninja command - ninjas only!",
    (REQ_ALERT|REQ_NINJAMODE)
    },
  { COM_DESCRIBE, "describe",
    "Ninja command.",
    "Ninja command - ninjas only!",
    (REQ_ALERT|REQ_NINJAMODE)
    },
  { COM_DEFINE, "define",
    "Ninja command.",
    "Ninja command - ninjas only!",
    (REQ_ALERT|REQ_NINJAMODE)
    },

  { COM_CLOAD, "cload",
    "Ninja command.",
    "Ninja command - ninjas only!",
    (REQ_ALERT|REQ_NINJAMODE)
    },
  { COM_CCLEAN, "cclean",
    "Ninja command.",
    "Ninja command - ninjas only!",
    (REQ_ALERT|REQ_NINJAMODE)
    },
  };
static const int comnum = sizeof(comlist)/sizeof(Command);

int handle_single_command(Object *body, const char *cl, Mind *mind) {
  int len;
  char *comline = (char*)cl;
  static char buf[2048];

  if((!body) && (!mind)) { // Nobody doing something?
    fprintf(stderr, "Warning: absolutely nobody tried to '%s'.\n", comline);
    return 0;
    }

  while((!isgraph(*comline)) && (*comline)) ++comline;

  if((*comline) == 0 || (*comline) == '#') return 0;

  if(mind && mind->Type() == MIND_REMOTE && (!mind->Owner())) {
    if(mind->PName() != "") mind->SetPPass(comline);
    else {
      int ctr;
      for(ctr=0; ctr<int(strlen(comline)); ++ctr) {
	if(!(isalnum(comline[ctr]) || comline[ctr] == ' ')) {
	  mind->Send("Name '%s' is invalid.\nNames can only have letters, numbers, and spaces.\n", comline);
	  fprintf(stderr, "Name '%s' is invalid.\nNames can only have letters, numbers, and spaces.\n", comline);
	  break;
	  }
	}
      if(ctr == int(strlen(comline))) mind->SetPName(comline);
      }
    return 0;
    }

  //fprintf(stderr, "Handling command from %p[%p] of '%s'\n", mind, body, cl);

  for(len=0; isgraph(comline[len]); ++len); 

  if(len == 0) return 0;

  //Aliases
  if(!strncasecmp(comline, "north", len)) { comline = "go north"; len = 2; }
  if(!strncasecmp(comline, "south", len)) { comline = "go south"; len = 2; }
  if(!strncasecmp(comline, "west", len)) { comline = "go west"; len = 2; }
  if(!strncasecmp(comline, "east", len)) { comline = "go east"; len = 2; }
  if(!strncasecmp(comline, "up", len)) { comline = "go up"; len = 2; }
  if(!strncasecmp(comline, "down", len)) { comline = "go down"; len = 2; }

  int com = -1, cnum = -1;
  for(int ctr=0; ctr < comnum; ++ctr) {
    if(!strncasecmp(comline, comlist[ctr].command, len))
      { com = comlist[ctr].id; cnum = ctr; break; }
    }
  if(com == -1) {
    if(mind) mind->Send("Command NOT understood - type 'help' for assistance.\n");
    return 1;
    }

  if(((!mind) || (!mind->Owner()) || (!mind->Owner()->Is(PLAYER_SUPERNINJA)))
	&& (comlist[cnum].sit & SIT_SUPERNINJA)) {
    if(mind) mind->Send("Sorry, that command is for Super Ninjas only!\n");
    return 0;
    }

  if(((!mind) || (!mind->Owner()) || (!mind->Owner()->Is(PLAYER_NINJA)))
	&& (comlist[cnum].sit & SIT_NINJA)) {
    if(mind) mind->Send("Sorry, that command is for True Ninjas only!\n");
    return 0;
    }

  if(((!mind) || (!mind->Owner()) || (!mind->Owner()->Is(PLAYER_NINJAMODE)))
	&& (comlist[cnum].sit & SIT_NINJAMODE)) {
    if(mind && mind->Owner() && mind->Owner()->Is(PLAYER_NINJA))
      mind->Send("Sorry, you need to be in Ninja Mode[TM] to do that.\n");
    else if(mind)
      mind->Send("Sorry, that command is for Ninjas only!\n");
    return 0;
    }

  if((!(comlist[cnum].sit & SIT_ETHEREAL)) && (!body)) {
    if(mind) mind->Send("You can't use that command until you join the game - with the \"enter\" command.\n");
    return 0;
    }

  if((!(comlist[cnum].sit & SIT_CORPOREAL)) && (body)) {
    if(mind) mind->Send("You can't use that command until you quit the game - with the \"quit\" command.\n");
    return 0;
    }

  if(body) {
    if(body->StillBusy() && (comlist[cnum].sit & SIT_ACTION)) {
      body->DoWhenFree(comline);
      return 0;
      }
    if(comlist[cnum].sit & (SIT_ALIVE|SIT_AWAKE|SIT_ALERT)) {
      if(body->IsAct(ACT_DYING) || body->IsAct(ACT_DEAD)) {
        if(mind) mind->Send("You must be alive to use that command.\n");
        return 0;
        }
      }
    if(body && (comlist[cnum].sit & SIT_CONSCIOUS)) {
      if(body->IsAct(ACT_UNCONSCIOUS)) {
        if(mind) mind->Send("You can't do that, you are out cold.\n");
        return 0;
        }
      }
    if((comlist[cnum].sit & (SIT_STAND|SIT_SIT)) == (SIT_STAND|SIT_SIT)) {
      if(body->Pos() != POS_SIT && body->Pos() != POS_STAND) {
        if(mind) mind->Send("You must at least sit up to use that command.\n");
	handle_single_command(body, "sit", mind);
	if(body->Pos() != POS_SIT && body->Pos() != POS_STAND)
	  return 0;
        }
      }
    if(comlist[cnum].sit & SIT_STAND) {
      if(body->Pos() != POS_STAND) {
        if(mind) mind->Send("You must stand up to use that command.\n");
	handle_single_command(body, "stand", mind);
	if(body->Pos() != POS_STAND) return 0;
	}
      }
    if(comlist[cnum].sit & SIT_SIT) {
      if(body->Pos() != POS_SIT) {
        if(mind) mind->Send("You must sit to use that command.\n");
	handle_single_command(body, "sit", mind);
	if(body->Pos() != POS_SIT) return 0;
        }
      }
    if(comlist[cnum].sit & SIT_ALERT) {
      if(body->IsAct(ACT_ASLEEP)) {
        if(mind) mind->Send("You must be awake to use that command.\n");
	handle_single_command(body, "wake", mind);
        if(body->IsAct(ACT_ASLEEP)) return 0;
        }
      if(body->IsAct(ACT_REST)) {
        if(mind) mind->Send("You must be awake to use that command.\n");
	handle_single_command(body, "rest", mind);
        if(body->IsAct(ACT_REST)) return 0;
        }
      }
    if(comlist[cnum].sit & SIT_AWAKE) {
      if(body->IsAct(ACT_ASLEEP)) {
        if(mind) mind->Send("You must be awake to use that command.\n");
	handle_single_command(body, "wake", mind);
        if(body->IsAct(ACT_ASLEEP)) return 0;
        }
      }
    }

  if(com == COM_SHUTDOWN) {
    shutdn = 1;
    if(mind) mind->Send("You instruct the system to shut down.\n");
    return 0;
    }

  if(com == COM_RESTART) {
    shutdn = 2;
    if(mind) mind->Send("You instruct the system to restart.\n");
    return 0;
    }

  if(com == COM_GO) {
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    Object *dest = body->PickObject(comline+len, LOC_ADJACENT);
    if(!dest) {
      if(mind) mind->Send("You can't go that way as far as you can tell.\n");
      }
    else {
      if(body->Parent()) body->Parent()->SendOut(
	";s leaves %s.\n", "", body, NULL, comline+len);
      body->Travel(dest);
      body->Parent()->SendOut(
	";s arrives.\n", "", body, NULL);
      if(mind && mind->Type() == MIND_REMOTE)
	body->Parent()->SendDescSurround(body, body);
      else if(mind && mind->Type() == MIND_SYSTEM)
	mind->Send("You enter %s\n", comline+len);
      }
    return 0;
    }

  if(com == COM_ENTER) {
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    if(!body) {  // Implies that there is a "mind"
      if(!comline[len]) {
	mind->Send("Enter which character?  Use 'enter <charname>'.\n");
	return 0;
	}
      if(!mind->Owner()) {
	Object *body = new_body();
	body->SetShortDesc(comline+len);
	mind->Attach(body);
	return 0;
	}

      Object *body =
		mind->Owner()->Room()->PickObject(comline+len, LOC_INTERNAL);
      if(!body) {
	mind->Send("Sorry, that character doesn't exist.\n"
		"Use the 'newchar' command to create a new character.\n");
	return 0;
	}
      if(body->IsAct(ACT_DEAD)) {
	if(mind && mind->Owner() && mind->Owner()->Is(PLAYER_NINJAMODE)) {
	  // Allow entry to ninjas - autoheal!
	  }  
	else {
	  mind->Send("Sorry, that character is dead.\n"
		"Use the 'newchar' command to create a new character.\n");
	  return 0;
	  }
	}
      mind->Attach(body);

      body->Parent()->SendOut(
	";s heals and repairs ;s with Ninja Powers[TM].\n", "You heal ;s.\n",
	body, body);

      //This is ninja-healing and bypasses all healing mechanisms.
      stats_t st = (*(body->Stats()));
      st.phys = 0;
      st.stun = 0;
      st.stru = 0;
      body->SetStats(st);

      if(body->IsAct(ACT_DYING))
	mind->Send("You can see nothing, you are too busy dying.\n");
      else if(body->IsAct(ACT_ASLEEP))
	mind->Send("You can see nothing since you are asleep.\n");
      else if(body->IsAct(ACT_UNCONSCIOUS))
	mind->Send("You can see nothing, you are out cold.\n");
      else
	body->Parent()->SendDescSurround(mind, body);
      return 0;
      }
    Object *dest = body->PickObject(comline+len, LOC_NEARBY);
    if(!dest) {
      if(mind) mind->Send("You want to go where?\n");
      }
    else if((!dest->Stats()->GetSkill("Enterable"))
	&& ((!mind) || (!mind->Owner()) || (!mind->Owner()->Is(PLAYER_NINJA)))) {
      if(mind) mind->Send("It is not possible to enter that object!\n");
      }
    else if((!dest->Stats()->GetSkill("Enterable"))
	&& ((!mind) || (!mind->Owner()) || (!mind->Owner()->Is(PLAYER_NINJAMODE)))) {
      if(mind) mind->Send("You need to be in ninja mode to enter that object!\n");
      }
    else {
      if(body->Parent()) body->Parent()->SendOut(
	";s enters %s.\n", "", body, NULL, comline+len);
      body->Travel(dest);
      body->Parent()->SendOut(
	";s arrives.\n", "", body, NULL);
      if(mind && mind->Type() == MIND_REMOTE)
	body->Parent()->SendDescSurround(body, body);
      else if(mind && mind->Type() == MIND_SYSTEM)
	mind->Send("You enter %s\n", comline+len);
      }
    return 0;
    }

  if(com == COM_QUIT) {
    if(!body) {
      delete mind;
      return -1; //Player Disconnected
      }
    //if(body) delete body;
    if(mind) mind->Unattach();

    if(mind && mind->Owner() && mind->Owner()->Room())
      mind->Owner()->Room()->SendDesc(mind);
    else
      if(mind) mind->Send("Use \"Enter\" to return to the game.\n");

    return 0;
    }

  if(com == COM_HELP) {
    if((!mind) || (!mind->Owner())) return 0;
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    if(!strcmp(comline+len, "commands")) {
      string mes = "";
      for(int ctr=0; ctr < comnum; ++ctr) {
        if((comlist[ctr].sit & SIT_NINJAMODE)
		&& (!mind->Owner()->Is(PLAYER_NINJAMODE))) continue;
        if((!(comlist[ctr].sit & SIT_NINJAMODE))
		&& (mind->Owner()->Is(PLAYER_NINJAMODE))) continue;
        if((comlist[ctr].sit & SIT_NINJA)
		&& (!mind->Owner()->Is(PLAYER_NINJA))) continue;
        if((comlist[ctr].sit & SIT_SUPERNINJA)
		&& (!mind->Owner()->Is(PLAYER_SUPERNINJA))) continue;
        if((!(comlist[ctr].sit & SIT_CORPOREAL)) && (body)) continue;
        if((!(comlist[ctr].sit & SIT_ETHEREAL)) && (!body)) continue;
        mes += comlist[ctr].command;
        mes += " - ";
        mes += comlist[ctr].shortdesc;
        mes += '\n';
        }
      mind->Send(mes.c_str());
      return 0;
      }
    mind->Send("Well, the help command's not yet implemented :P\n"
	"Try 'help commands' command for a list of commands at least.\n");
    return 0;
    }

  if(com == COM_SAY) {
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    body->Parent()->SendOut(";s says '%s'\n", "You say '%s'\n",
	body, body, comline+len);
    return 0;
    }

  if(com == COM_EMOTE) {
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    body->Parent()->SendOut(";s %s\n", "Your character %s\n",
	body, body, comline+len);
    return 0;
    }

  if(com == COM_LOOK) {
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    if(!body) {
      mind->Owner()->Room()->SendDesc(mind);
      return 0;
      }

    Object *targ = body->Parent();
    int within = 0;

    if(!strncasecmp(comline+len, "at ", 3)) len += 3;
    if(!strncasecmp(comline+len, "in ", 3)) { len += 3; within = 1; }

    if(strlen(comline+len) > 0) {
      targ = body->PickObject(comline+len,
	LOC_NEARBY|LOC_ADJACENT|LOC_SELF|LOC_INTERNAL);
      }
    if(!targ) {
      if(mind) mind->Send("You don't see that here.\n");
      }
    else {
      if(within && (!targ->Stats()->GetSkill("Container"))) {
	if(mind) mind->Send("You can't look inside that, it is not a container.\n");
	}
      else if(within && (targ->Stats()->GetSkill("Locked"))) {
	if(mind) mind->Send("You can't look inside that, it is locked.\n");
	}
      else {
	int must_open = within;
	stats_t stats = (*(targ->Stats()));
	if(within && stats.GetSkill("Transparent")) must_open = 0;

	if(must_open) {
	  stats.SetSkill("Transparent", 1);
	  targ->SetStats(stats);
	  body->Parent()->SendOut(
		";s opens ;s.\n", "You open ;s.\n", body, targ);
	  }

	if(strlen(comline+len) > 0 && within) {
		body->Parent()->SendOut(
		";s looks inside ;s.\n", "", body, targ);
	  if(mind) targ->SendDesc(mind, body);
	  }
	else if(strlen(comline+len) > 0) {
		body->Parent()->SendOut(
		";s looks at ;s.\n", "", body, targ);
	  if(mind) targ->SendDesc(mind, body);
	  }
	else {
		body->Parent()->SendOut(
		";s looks around.\n", "", body, targ);
	  if(mind) targ->SendDescSurround(mind, body);
	  }

	if(must_open) {
	  stats.SetSkill("Transparent", 0);
	  targ->SetStats(stats);
	  body->Parent()->SendOut(
		";s closes ;s.\n", "You close ;s.\n", body, targ);
	  }
	}
      }
    return 0;
    }

  if(com == COM_EXAMINE) {
    Object *targ = NULL;
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    if(strlen(comline+len) < 0) {
      if(mind) mind->Send("You want to examine what?\n");
      return 0;
      }
    targ = body->PickObject(comline+len,
		LOC_INTERNAL|LOC_NEARBY|LOC_SELF);
    if(!targ) {
      if(mind) mind->Send("You don't see that here.\n");
      }
    else {
      body->Parent()->SendOut(
	";s examines ;s.\n", "", body, targ);
      if(mind) targ->SendLongDesc(mind, body);
      }
    return 0;
    }

  if(com == COM_OPEN) {
    Object *targ = NULL;
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    if(strlen(comline+len) < 0) {
      if(mind) mind->Send("You want to open what?\n");
      return 0;
      }
    targ = body->PickObject(comline+len, LOC_INTERNAL|LOC_NEARBY);
    if(!targ) {
      if(mind) mind->Send("You don't see that here.\n");
      }
    else if(!targ->Stats()->GetSkill("Closeable")) {
      if(mind) mind->Send("That can't be opened or closed.\n");
      }
    else if(targ->Stats()->GetSkill("Transparent")) {
      if(mind) mind->Send("It's already open!\n");
      }
    else {
      body->Parent()->SendOut(
	";s opens ;s.\n", "You open ;s.", body, targ);
      stats_t stats = (*(targ->Stats()));
      stats.SetSkill("Transparent", 1);
      targ->SetStats(stats);
      }
    return 0;
    }

  if(com == COM_CLOSE) {
    Object *targ = NULL;
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    if(strlen(comline+len) < 0) {
      if(mind) mind->Send("You want to close what?\n");
      return 0;
      }
    targ = body->PickObject(comline+len, LOC_INTERNAL|LOC_NEARBY);
    if(!targ) {
      if(mind) mind->Send("You don't see that here.\n");
      }
    else if(!targ->Stats()->GetSkill("Closeable")) {
      if(mind) mind->Send("That can't be opened or closed.\n");
      }
    else if(!targ->Stats()->GetSkill("Transparent")) {
      if(mind) mind->Send("It's already closed!\n");
      }
    else {
      body->Parent()->SendOut(
	";s closes ;s.\n", "You close ;s.\n", body, targ);
      stats_t stats = (*(targ->Stats()));
      stats.SetSkill("Transparent", 0);
      targ->SetStats(stats);
      }
    return 0;
    }

  if(com == COM_GET) {
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    Object *targ = body->PickObject(comline+len, LOC_NEARBY);
    if(!targ) {
      if(mind) mind->Send("You want to get what?\n");
      }
    else if(targ->Pos() == POS_NONE) {
      if(mind) mind->Send("You can't get that, it is fixed in place!\n");
      }
    else if(targ->Stats()->GetAttribute(1)) {
      if(mind) mind->Send("You can only get inanimate objects!\n");
      }
    else if(body->IsAct(ACT_HOLD)) {
      if(mind) mind->Send("You are already holding something else!\n");
      }
    else {
      targ->Travel(body);
      body->Parent()->SendOut(
	";s gets ;s.\n", "You grab ;s.\n", body, targ);
      body->AddAct(ACT_HOLD, targ);
      }
    return 0;
    }

  if(com == COM_PUT) {
    while((!isgraph(comline[len])) && (comline[len])) ++len;

    if(!strncasecmp(comline+len, "in ", 3)) len += 3;

    Object *targ =
	body->PickObject(comline+len, LOC_NEARBY|LOC_INTERNAL);
    if(!body->IsAct(ACT_HOLD)) {
      if(mind) mind->Send("You must first 'hold' the object you want to 'put'.\n");
      }
    else if(!targ) {
      if(mind) mind->Send("I don't see '%s' to put '%s' in!\n", comline+len,
	body->ActTarg(ACT_HOLD)->Name());
      }
    else if(targ->Stats()->GetAttribute(1)) {
      if(mind) mind->Send("You can only put things in inanimate objects!\n");
      }
    else if(!targ->Stats()->GetSkill("Container")) {
      if(mind) mind->Send("You can't put anything in that, it is not a container.\n");
      }
    else if(targ->Stats()->GetSkill("Locked")) {
      if(mind) mind->Send("You can't put anything in that, it is locked.\n");
      }
    else {
      int closed = 0, res = 0;
      Object *obj = body->ActTarg(ACT_HOLD);
      res = obj->Travel(targ);
      if(res == -2) {
	if(mind) mind->Send("It won't fit in there.\n");
	}
      else if(res == -3) {
	if(mind) mind->Send("It's too heavy to put in there.\n");
	}
      else if(res) {
	if(mind) mind->Send("You can't put it in there.\n");
	}
      else {
	if(!targ->Stats()->GetSkill("Transparent")) closed = 1;
	if(closed) body->Parent()->SendOut(
		";s opens ;s.\n", "You open ;s.\n", body, targ);
	body->Parent()->SendOut(
		";s puts %s into ;s.\n", "You put %s into ;s.\n",
		body, targ, obj->Name());
	if(closed) body->Parent()->SendOut(
		";s close ;s.\n", "You close ;s.\n", body, targ);
	}
      }
    return 0;
    }

  if(com == COM_WIELD) {
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    if(!comline[len]) {
      if(body->IsAct(ACT_WIELD)) {
	if(body->IsAct(ACT_HOLD)) {
	  if(mind) mind->Send("You are holding something else.\n"
		"Perhaps you want to 'drop' one of these?");
	  return 0;
	  }
	body->Parent()->SendOut(";s stops wielding ;s.\n",
		"You stop wielding ;s.\n", body,
		body->ActTarg(ACT_WIELD));
	body->AddAct(ACT_HOLD, body->ActTarg(ACT_WIELD));
	body->StopAct(ACT_WIELD);
	return 0;
	}
      else {
	if(mind) mind->Send("You are not wielding anything.\n");
	return 0;
	}      
      }

    Object *targ = body->PickObject(comline+len, LOC_INTERNAL);
    if(!targ) {
      if(mind) mind->Send("You want to wield what?\n");
      }
    else if(targ->Stats()->GetSkill("WeaponType") <= 0) {
      if(mind) mind->Send("You can't wield that - it's not a weapon!\n");
      }
    else if(body->ActTarg(ACT_WEAR_BACK) == targ
	|| body->ActTarg(ACT_WEAR_CHEST) == targ
	|| body->ActTarg(ACT_WEAR_HEAD) == targ
	|| body->ActTarg(ACT_WEAR_NECK) == targ
	|| body->ActTarg(ACT_WEAR_WAIST) == targ
	|| body->ActTarg(ACT_WEAR_SHIELD) == targ
	|| body->ActTarg(ACT_WEAR_LARM) == targ
	|| body->ActTarg(ACT_WEAR_RARM) == targ
	|| body->ActTarg(ACT_WEAR_LFINGER) == targ
	|| body->ActTarg(ACT_WEAR_RFINGER) == targ
	|| body->ActTarg(ACT_WEAR_LFOOT) == targ
	|| body->ActTarg(ACT_WEAR_RFOOT) == targ
	|| body->ActTarg(ACT_WEAR_LHAND) == targ
	|| body->ActTarg(ACT_WEAR_RHAND) == targ
	|| body->ActTarg(ACT_WEAR_LLEG) == targ
	|| body->ActTarg(ACT_WEAR_RLEG) == targ
	|| body->ActTarg(ACT_WEAR_LWRIST) == targ
	|| body->ActTarg(ACT_WEAR_RWRIST) == targ
	|| body->ActTarg(ACT_WEAR_LSHOULDER) == targ
	|| body->ActTarg(ACT_WEAR_RSHOULDER) == targ
	) {
      if(mind) mind->Send("You are wearing that, perhaps you want to 'remove' it?\n");
      }
    else {
      if(body->IsAct(ACT_WIELD) && body->IsAct(ACT_HOLD)) {
	if(body->ActTarg(ACT_HOLD) != targ) {
	  if(mind) mind->Send("You are both holding and wielding other things.\n"
		"Perhaps you want to drop one of them?");
	  return 0;
	  }
	}
      targ->Travel(body); // Kills Holds and Wields on "targ"
      if(body->ActTarg(ACT_HOLD) == targ) {
	body->StopAct(ACT_HOLD);
	}
      if(body->IsAct(ACT_WIELD)) {
	body->AddAct(ACT_HOLD, body->ActTarg(ACT_WIELD));
	body->Parent()->SendOut(";s stops wielding ;s.\n",
		"You stop wielding ;s.\n", body,
		body->ActTarg(ACT_WIELD));
	body->StopAct(ACT_WIELD);
	}
      body->AddAct(ACT_WIELD, targ);
      body->Parent()->SendOut(
	";s wields ;s.\n", "You wield ;s.\n", body, targ);
      }
    return 0;
    }

  if(com == COM_HOLD) {
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    if(!comline[len]) {
      if(mind) mind->Send("You want to hold what?\n");
      return 0;
//      if(body->IsAct(ACT_HOLD)) {
//	body->Parent()->SendOut(";s stops holding ;s.\n",
//		"You stop holding ;s.\n", body,
//		body->ActTarg(ACT_HOLD));
//	body->StopAct(ACT_HOLD);
//	return 0;
//	}
//      else {
//	if(mind) mind->Send("You are not holding anything.\n");
//	return 0;
//	}      
      }

    Object *targ = body->PickObject(comline+len, LOC_INTERNAL);
    if(!targ) {
      if(mind) mind->Send("You want to hold what?\n");
      }
    else if(body->ActTarg(ACT_WEAR_BACK) == targ
	|| body->ActTarg(ACT_WEAR_CHEST) == targ
	|| body->ActTarg(ACT_WEAR_HEAD) == targ
	|| body->ActTarg(ACT_WEAR_NECK) == targ
	|| body->ActTarg(ACT_WEAR_WAIST) == targ
	|| body->ActTarg(ACT_WEAR_SHIELD) == targ
	|| body->ActTarg(ACT_WEAR_LARM) == targ
	|| body->ActTarg(ACT_WEAR_RARM) == targ
	|| body->ActTarg(ACT_WEAR_LFINGER) == targ
	|| body->ActTarg(ACT_WEAR_RFINGER) == targ
	|| body->ActTarg(ACT_WEAR_LFOOT) == targ
	|| body->ActTarg(ACT_WEAR_RFOOT) == targ
	|| body->ActTarg(ACT_WEAR_LHAND) == targ
	|| body->ActTarg(ACT_WEAR_RHAND) == targ
	|| body->ActTarg(ACT_WEAR_LLEG) == targ
	|| body->ActTarg(ACT_WEAR_RLEG) == targ
	|| body->ActTarg(ACT_WEAR_LWRIST) == targ
	|| body->ActTarg(ACT_WEAR_RWRIST) == targ
	|| body->ActTarg(ACT_WEAR_LSHOULDER) == targ
	|| body->ActTarg(ACT_WEAR_RSHOULDER) == targ
	) {
      if(mind) mind->Send("You are wearing that, perhaps you want to 'remove' it?\n");
      }
//FIXME - Implement Str-based Holding Capacity
//    else if(targ->Stats()->GetSkill("WeaponType") <= 0) {
//      if(mind) mind->Send("You can't hold that - you are too weak!\n");
//      }
    else if(body->IsAct(ACT_HOLD)) {
      if(mind) mind->Send("You are already holding something!\n");
      }
    else {
      targ->Travel(body); // Kills Holds and Wields on "targ"
//      if(body->ActTarg(ACT_WIELD) == targ) {
//	body->Parent()->SendOut(";s stops wielding ;s.\n",
//		"You stop wielding ;s.\n", body,
//		body->ActTarg(ACT_WIELD));
//	body->StopAct(ACT_WIELD);
//	}
      body->AddAct(ACT_HOLD, targ);
      body->Parent()->SendOut(
	";s holds ;s.\n", "You hold ;s.\n", body, targ);
      }
    return 0;
    }

  if(com == COM_REMOVE) {
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    if(!comline[len]) {
      if(mind) mind->Send("You want to remove what?\n");
      return 0;
      }

    Object *targ = body->PickObject(comline+len, LOC_INTERNAL);
    if(!targ) {
      if(mind) mind->Send("You want to remove what?\n");
      }
    else if(body->IsAct(ACT_HOLD)) {
      if(mind) mind->Send("You are already holding something else, do you mean to 'drop' this?\n");
      }
    else {
      int removed = 0;
      for(act_t act = ACT_WEAR_BACK; act < ACT_MAX; ++((int&)(act))) {
	if(body->ActTarg(act) == targ) {
	  body->StopAct(act);  removed = 1;
	  }
	}
      if(!removed) {
	if(mind) mind->Send("You are not wearing that!\n");
	}
      else {
	body->Parent()->SendOut(
		";s removes ;s.\n", "You remove ;s.\n", body, targ);
	body->AddAct(ACT_HOLD, targ);
	}
      }
    return 0;
    }

  if(com == COM_WEAR) {
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    if(!comline[len]) {
      if(mind) mind->Send("You want to wear what?\n");
      return 0;
      }

    Object *targ = body->PickObject(comline+len, LOC_INTERNAL);
    if(!targ) {
      if(mind) mind->Send("You want to wear what?\n");
      }
//    else if(targ->Stats()->GetSkill("WeaponType") <= 0) {
//      if(mind) mind->Send("You can't wear that - it's not wearable!\n");
//      }
    else if(body->ActTarg(ACT_WEAR_BACK) == targ
	|| body->ActTarg(ACT_WEAR_CHEST) == targ
	|| body->ActTarg(ACT_WEAR_HEAD) == targ
	|| body->ActTarg(ACT_WEAR_NECK) == targ
	|| body->ActTarg(ACT_WEAR_WAIST) == targ
	|| body->ActTarg(ACT_WEAR_SHIELD) == targ
	|| body->ActTarg(ACT_WEAR_LARM) == targ
	|| body->ActTarg(ACT_WEAR_RARM) == targ
	|| body->ActTarg(ACT_WEAR_LFINGER) == targ
	|| body->ActTarg(ACT_WEAR_RFINGER) == targ
	|| body->ActTarg(ACT_WEAR_LFOOT) == targ
	|| body->ActTarg(ACT_WEAR_RFOOT) == targ
	|| body->ActTarg(ACT_WEAR_LHAND) == targ
	|| body->ActTarg(ACT_WEAR_RHAND) == targ
	|| body->ActTarg(ACT_WEAR_LLEG) == targ
	|| body->ActTarg(ACT_WEAR_RLEG) == targ
	|| body->ActTarg(ACT_WEAR_LWRIST) == targ
	|| body->ActTarg(ACT_WEAR_RWRIST) == targ
	|| body->ActTarg(ACT_WEAR_LSHOULDER) == targ
	|| body->ActTarg(ACT_WEAR_RSHOULDER) == targ
	) {
      if(mind) mind->Send("You are already wearing that!\n");
      }
    else {
      int success = 0;
      unsigned long mask = 1;
      while(!success) {
	set<act_t> locations;

	if(targ->Stats()->GetSkill("Wearable on Back") & mask)
		locations.insert(ACT_WEAR_BACK);

	if(targ->Stats()->GetSkill("Wearable on Chest") & mask)
		locations.insert(ACT_WEAR_CHEST);

	if(targ->Stats()->GetSkill("Wearable on Head") & mask)
		locations.insert(ACT_WEAR_HEAD);

	if(targ->Stats()->GetSkill("Wearable on Neck") & mask)
		locations.insert(ACT_WEAR_NECK);

	if(targ->Stats()->GetSkill("Wearable on Waist") & mask)
		locations.insert(ACT_WEAR_WAIST);

	if(targ->Stats()->GetSkill("Wearable on Shield") & mask)
		locations.insert(ACT_WEAR_SHIELD);

	if(targ->Stats()->GetSkill("Wearable on Left Arm") & mask)
		locations.insert(ACT_WEAR_LARM);

	if(targ->Stats()->GetSkill("Wearable on Right Arm") & mask)
		locations.insert(ACT_WEAR_RARM);

	if(targ->Stats()->GetSkill("Wearable on Left Finger") & mask)
		locations.insert(ACT_WEAR_LFINGER);

	if(targ->Stats()->GetSkill("Wearable on Right Finger") & mask)
		locations.insert(ACT_WEAR_RFINGER);

	if(targ->Stats()->GetSkill("Wearable on Left Foot") & mask)
		locations.insert(ACT_WEAR_LFOOT);

	if(targ->Stats()->GetSkill("Wearable on Right Foot") & mask)
		locations.insert(ACT_WEAR_RFOOT);

	if(targ->Stats()->GetSkill("Wearable on Left Hand") & mask)
		locations.insert(ACT_WEAR_LHAND);

	if(targ->Stats()->GetSkill("Wearable on Right Hand") & mask)
		locations.insert(ACT_WEAR_RHAND);

	if(targ->Stats()->GetSkill("Wearable on Left Leg") & mask)
		locations.insert(ACT_WEAR_LLEG);

	if(targ->Stats()->GetSkill("Wearable on Right Leg") & mask)
		locations.insert(ACT_WEAR_RLEG);

	if(targ->Stats()->GetSkill("Wearable on Left Wrist") & mask)
		locations.insert(ACT_WEAR_LWRIST);

	if(targ->Stats()->GetSkill("Wearable on Right Wrist") & mask)
		locations.insert(ACT_WEAR_RWRIST);

	if(targ->Stats()->GetSkill("Wearable on Left Shoulder") & mask)
		locations.insert(ACT_WEAR_LSHOULDER);

	if(targ->Stats()->GetSkill("Wearable on Right Shoulder") & mask)
		locations.insert(ACT_WEAR_RSHOULDER);

	if(locations.size() < 1) break;
	success = 1;
	mask *= 2;

	set<act_t>::iterator loc;
	for(loc = locations.begin(); loc != locations.end(); ++loc) {
	  if(body->IsAct(*loc)) { success = 0; break; }
	  }
	if(success) {
	  targ->Travel(body); // Kills Holds and Wields on "targ"
	  for(loc = locations.begin(); loc != locations.end(); ++loc) {
	    body->AddAct(*loc, targ);
	    }
	  body->Parent()->SendOut(
		";s puts on ;s.\n", "You put on ;s.\n", body, targ);
	  }
	}

      if(!success) {
	if(mind) mind->Send("You can't wear that with what you have on already!\n");
	return 0;
	}
      }
    return 0;
    }

  if(com == COM_DROP) {
    Object *targ = body->PickObject(comline+len, LOC_INTERNAL);
    if(!targ) {
      if(mind) mind->Send("You want to drop what?\n");
      }
    else {
      targ->Travel(body->Parent());
      body->Parent()->SendOut(
	";s drops ;s.\n", "You drop ;s.\n", body, targ);
      }
    return 0;
    }

  if(com == COM_LEAVE) {
    Object *oldp = body->Parent();
    if(!body->Parent()->Parent()) {
      if(mind) mind->Send("It is not possible to leave this object!\n");
      }
    else if((!body->Parent()->Stats()->GetSkill("Enterable"))
	&& ((!mind) || (!mind->Owner()) || (!mind->Owner()->Is(PLAYER_NINJA)))) {
      if(mind) mind->Send("It is not possible to leave this object!\n");
      }
    else if((!body->Parent()->Stats()->GetSkill("Enterable"))
	&& ((!mind) || (!mind->Owner()) || (!mind->Owner()->Is(PLAYER_NINJAMODE)))) {
      if(mind) mind->Send("You need to be in ninja mode to leave this object!\n");
      }
    else {
      body->Travel(body->Parent()->Parent());
      if(oldp) oldp->SendOut(";s leaves.\n", "", body, NULL);
      body->Parent()->SendDescSurround(body, body);
      body->Parent()->SendOut(
	";s arrives.\n", "", body, NULL);
      }
    return 0;
    }

  if(com == COM_SLEEP) {
    if(body->IsAct(ACT_ASLEEP)) {
      if(mind) mind->Send("But you are already sleeping!\n");
      }
    else if(body->Pos() == POS_LIE) {
      body->Parent()->SendOut(
	";s goes to sleep.\n", "You go to sleep.\n", body, NULL);
      }
    else {
      body->Parent()->SendOut(
	";s lies down and goes to sleep.\n", "You lie down and go to sleep.\n",
	body, NULL);
      }
    body->SetPos(POS_LIE);
    body->Collapse();
    body->AddAct(ACT_ASLEEP);
    return 0;
    }

  if(com == COM_WAKE) {
    if(!body->IsAct(ACT_ASLEEP)) {
      if(mind) mind->Send("But you aren't asleep!\n");
      }
    else {
      body->Parent()->SendOut(
	";s wakes up.\n", "You wake up.\n", body, NULL);
      body->StopAct(ACT_ASLEEP);
      }
    return 0;
    }

  if(com == COM_REST) {
    if(body->IsAct(ACT_REST)) {
      body->Parent()->SendOut(
	";s stops resting.\n", "You stop resting.\n", body, NULL);
      body->StopAct(ACT_REST);
      return 0;
      }
    else if(body->IsAct(ACT_ASLEEP)) {
      body->Parent()->SendOut(
	";s wakes up and starts resting.\n", "You wake up and start resting.\n",
	body, NULL);
      }
    else if(body->Pos() == POS_LIE || body->Pos() == POS_SIT) {
      body->Parent()->SendOut(
	";s starts resting.\n", "You start resting.\n", body, NULL);
      }
    else {
      body->Parent()->SendOut(
	";s sits down and rests.\n", "You sit down and rest.\n",
	body, NULL);
      }
    body->AddAct(ACT_REST);
    body->Collapse();
    if(body->Pos() != POS_LIE) body->SetPos(POS_SIT);
    return 0;
    }

  if(com == COM_STAND) {
    if(body->Pos() == POS_STAND) {
      if(mind) mind->Send("But you are already standing!\n");
      }
    else if(body->IsAct(ACT_ASLEEP)) {
      body->StopAct(ACT_ASLEEP);
      body->Parent()->SendOut(
	";s wakes up and stands.\n", "You wake up and stand.\n",
	body, NULL);
      }
    else if(body->IsAct(ACT_REST)) {
      body->StopAct(ACT_REST);
      body->Parent()->SendOut(
	";s stops resting and stands up.\n", "You stop resting and stand up.\n",
	body, NULL);
      }
    else {
      body->Parent()->SendOut(
	";s stands up.\n", "You stand up.\n", body, NULL);
      }
    body->SetPos(POS_STAND);
    return 0;
    }

  if(com == COM_SIT) {
    if(body->Pos() == POS_SIT) {
      if(mind) mind->Send("But you are already sitting!\n");
      }
    else if(body->IsAct(ACT_ASLEEP)) {
      body->StopAct(ACT_ASLEEP);
      body->Parent()->SendOut(
	";s awaken and sit up.\n", "You awaken and sit up.\n",
	body, NULL);
      }
    else if(body->Pos() == POS_LIE) {
      body->Parent()->SendOut(
	";s sits up.\n", "You sit up.\n", body, NULL);
      }
    else {
      body->Parent()->SendOut(
	";s sits down.\n", "You sit down.\n", body, NULL);
      }
    body->SetPos(POS_SIT);
    return 0;
    }

  if(com == COM_LIE) {
    if(body->Pos() == POS_LIE) {
      if(mind) mind->Send("But you are already lying down!\n");
      }
    else {
      body->Parent()->SendOut(
	";s lies down.\n", "You lie down.\n", body, NULL);
      body->SetPos(POS_LIE);
      }
    return 0;
    }

  if(com == COM_POINT) {
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    if(strlen(comline+len) > 0) {
      Object *targ = body->PickObject(comline+len, LOC_NEARBY|LOC_SELF);
      if(!targ) {
	if(mind) mind->Send("You don't see that here.\n");
	}
      else {
	body->AddAct(ACT_POINT, targ);
	body->Parent()->SendOut(
		";s starts pointing at ;s.\n", "You start pointing at ;s.\n",
		body, targ);
	}
      }
    else if(body->IsAct(ACT_POINT)) {
      Object *targ = body->ActTarg(ACT_POINT);
      body->Parent()->SendOut(
	";s stops pointing at ;s.\n", "You stop pointing at ;s.\n",
	body, targ);
      body->StopAct(ACT_POINT);
      }
    else {
      if(mind) mind->Send("But, you aren't pointing at anyting!\n");
      }
    return 0;
    }

  if(com == COM_ATTACK || com == COM_KILL || com == COM_PUNCH || com == COM_KICK) {
    //fprintf(stderr, "Handling attack command from %p of '%s'\n", mind, cl);

    Object *targ = NULL;
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    if(strlen(comline+len) > 0) {
      targ = body->PickObject(comline+len, LOC_NEARBY);
      if(!targ) { 
	if(mind) mind->Send("You don't see that here.\n");
	return 0;
	}
      }
    else {
      if(body->IsAct(ACT_FIGHT)) {
	targ = body->ActTarg(ACT_FIGHT);
	if(!body->IsNearBy(targ)) {
	  if(mind) mind->Send("Your target is gone!\n");
	  body->StopAct(ACT_FIGHT);
	  return 0;
	  }
	}
      else {
	if(mind) mind->Send("Who did you want to hit?\n");
	return 0;
	}
      }

    if(com == COM_ATTACK && (targ->Stats()->GetAttribute(1) <= 0
	|| targ->IsAct(ACT_DEAD) || targ->IsAct(ACT_DYING)
	|| targ->IsAct(ACT_UNCONSCIOUS))) {
      if(mind) mind->Send("No need, target is down!\n");
      body->StopAct(ACT_FIGHT);
      return 0;
      }

    body->BusyFor(3000); //Overridden below if is alive/animate

    if(!(targ->Stats()->GetAttribute(1) <= 0 || targ->IsAct(ACT_DEAD)
	|| targ->IsAct(ACT_DYING) || targ->IsAct(ACT_UNCONSCIOUS))) {
      body->AddAct(ACT_FIGHT, targ);
      body->BusyFor(3000, "attack");
      if(!targ->IsAct(ACT_FIGHT)) {
	targ->BusyFor(3000, "attack");
	targ->AddAct(ACT_FIGHT, body);
	}
      else if(targ->StillBusy()) {
	body->BusyWith(targ, "attack");
	}
      }

//    int succ = roll(body->Stats()->att[1], targ->Stats()->att[1]);
    int succ; string res;  //FIXME: res if ONLY for debugging!

    int reachmod = 0;
    string sk1 = "Punching", sk2 = "Punching";
    if(com == COM_KICK) { sk1 = "Kicking"; sk2 = "Kicking"; }
    else {
      if(body->IsAct(ACT_WIELD)) {
        sk1 = get_weapon_skill(body->ActTarg(ACT_WIELD)
		->Stats()->GetSkill("WeaponType"));
	reachmod += (0 >? body->ActTarg(ACT_WIELD)
		->Stats()->GetSkill("WeaponReach"));
	}
      if(targ->IsAct(ACT_WIELD)) {
        sk2 = get_weapon_skill(targ->ActTarg(ACT_WIELD)
		->Stats()->GetSkill("WeaponType"));
	reachmod -= (0 >? targ->ActTarg(ACT_WIELD)
		->Stats()->GetSkill("WeaponReach"));
	}
      }

    succ = body->Stats()->Roll(sk1, targ->Stats(), sk2, reachmod, &res);

    if(succ > 0) {
      //FIXME: Remove debugging stuff ("succ" and "res") from these messages.
      if(com == COM_KICK)
	body->Parent()->SendOut(
		";s kicks ;s. [%d] %s\n", "You kick ;s. [%d] %s\n",
		body, targ, succ, res.c_str());
      else if(body->IsAct(ACT_WIELD))
	body->Parent()->SendOut(";s hits ;s with %s. [%d] %s\n",
		"You hit ;s with %s. [%d] %s\n", body, targ,
		body->ActTarg(ACT_WIELD)->ShortDesc(),
		succ, res.c_str());
      else
	body->Parent()->SendOut(
		";s punches ;s. [%d] %s\n", "You punch ;s. [%d] %s\n",
		body, targ, succ, res.c_str());
      int sev = 0;

      if(com == COM_KICK)
	sev = targ->HitStun(body->Stats()->GetAttribute(2), 2, succ);
      else if(body->IsAct(ACT_WIELD))
	sev = targ->HitPhys(body->Stats()->GetAttribute(2)
	  + (0 >? body->ActTarg(ACT_WIELD)->Stats()->GetSkill("WeaponForce")),
	  (0 >? body->ActTarg(ACT_WIELD)->Stats()->GetSkill("WeaponSeverity")),
	  succ);
      else
	sev = targ->HitStun(body->Stats()->GetAttribute(2), 1, succ);

      if(sev <= 0) {
	if(mind) mind->Send("You hit - but didn't do much.\n");  //FIXME - Real Messages
	}
      }
    else {
      if(com == COM_KICK) body->Parent()->SendOut(
	";s tries to kick ;s, but misses. [%d] %s\n", "You missed. [%d] %s\n",
	body, targ, succ, res.c_str());
      else if(body->IsAct(ACT_WIELD))body->Parent()->SendOut(
	";s tries to attack ;s, but misses. [%d] %s\n", "You missed. [%d] %s\n",
	body, targ, succ, res.c_str());
      else body->Parent()->SendOut(
	";s tries to punch ;s, but misses. [%d] %s\n", "You missed. [%d] %s\n",
	body, targ, succ, res.c_str());
      }

    if(targ->Stats()->GetAttribute(1) <= 0 || targ->IsAct(ACT_DEAD)
	|| targ->IsAct(ACT_DYING) || targ->IsAct(ACT_UNCONSCIOUS)) {
      body->StopAct(ACT_FIGHT);
      body->BusyFor(3000);
      }

    return 0;
    }

  if(com == COM_NEWCHAR) {
    if(!mind) return 0; //FIXME: Should never happen!
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    if(!comline[len]) {
      mind->Send("What's the character's name?  Use 'newchar <charname>'.\n");
      return 0;
      }
    Object *body = mind->Owner()->Room()->PickObject(comline+len, LOC_INTERNAL);
    if(body) {
      mind->Send("Sorry, you already have a character with that name.\n"
			"Pick another name.\n");
      return 0;
      }
    body = new_body();
    body->SetShortDesc(comline+len);
    mind->Owner()->AddChar(body);
    mind->Send("You created %s.\n", comline+len);
    return 0;
    }

  if(com == COM_SKILLLIST) {
    if(!mind) return 0;
    string skills = "Total Skills in play on this mud:\n";
    map<string,int> skls = get_skills();

    map<string,int>::iterator skl = skls.begin();
    for(; skl != skls.end(); ++skl) {
      skills += skl->first;
      skills += "\n";
      }
    mind->Send(skills.c_str());

    return 0;
    }

  if(com == COM_WHO) {
    if(!mind) return 0;
    string users = "Currently on this mud:\n";
    vector<Mind *> mns = get_human_minds();

    vector<Mind *>::iterator mn = mns.begin();
    for(; mn != mns.end(); ++mn) {
      users += (*mn)->Owner()->Name();
      if((*mn)->Body())
	users = users + " as " + (*mn)->Body()->ShortDesc() + ".\n";
      else
	users += " in character room.\n";
      }
    mind->Send(users.c_str());

    return 0;
    }

  if(com == COM_OOC) {
    if(!mind) return 0;
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    if(strlen(comline+len) <= 0) {
      mind->Send("Toggling OOC doesn't work yet, you MUST LISTEN!!!\n");
      }
    else {
      string name = "Unknown";
      if(mind->Owner()) name = mind->Owner()->Name();
      string mes = string("OOC: <") + name + "> " + (comline+len) + "\n";
      vector<Mind *> mns = get_human_minds();
      vector<Mind *>::iterator mn = mns.begin();
      for(; mn != mns.end(); ++mn) {
	(*mn)->Send(mes.c_str());
	}
      }
    return 0;
    }

  if(com == COM_NEWBIE) {
    if(!mind) return 0;
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    if(strlen(comline+len) <= 0) {
      mind->Send("Toggling NEWBIE doesn't work yet, you MUST LISTEN!!!\n");
      }
    else {
      string name = "Unknown";
      if(mind->Owner()) name = mind->Owner()->Name();
      string mes = string("NEWBIE: <") + name + "> " + (comline+len) + "\n";
      vector<Mind *> mns = get_human_minds();
      vector<Mind *>::iterator mn = mns.begin();
      for(; mn != mns.end(); ++mn) {
	(*mn)->Send(mes.c_str());
	}
      }
    return 0;
    }

  //Ninja Commands

  if(com == COM_NINJAMODE) {
    if(!mind) return 0;
    while((!isgraph(comline[len])) && (comline[len])) ++len;

    Player *pl = mind->Owner();
    if(strlen(comline+len) <= 0) {
      mind->Send("What command do you want to run in Ninja Mode[TM]?\n");
      mind->Send("You need to include the command, like 'ninja junk boat'.\n");
      }
    else if(!pl) {
      mind->Send("Sorry, you don't seem to be a player!\n");
      }
    else {
      pl->Set(PLAYER_NINJAMODE);
      mind->Send("Ninja mode activated.\n");
      handle_single_command(body, comline+len, mind);
      pl->UnSet(PLAYER_NINJAMODE);
      mind->Send("Ninja mode deactivated.\n");
      }
    return 0;
    }

  if(com == COM_MAKENINJA) {
    if(!mind) return 0;
    while((!isgraph(comline[len])) && (comline[len])) ++len;

    Player *pl = get_player(comline+len);
    if(strlen(comline+len) <= 0) {
      mind->Send("You can only make/unmake other True Ninjas[TM]\n");
      return 0;
      }
    if(!pl) {
      mind->Send("There is no PLAYER named '%s'\n", comline+len);
      return 0;
      }
    if(pl == mind->Owner()) {
      mind->Send("You can only make/unmake other True Ninjas[TM]\n");
      return 0;
      }

    if(pl->Is(PLAYER_SUPERNINJA)) {
      mind->Send("'%s' is already a Super Ninja[TM] - this is irrevocable.\n", comline+len);
      }
    else if(pl->Is(PLAYER_NINJA)) {
      pl->UnSet(PLAYER_NINJA);
      pl->UnSet(PLAYER_NINJAMODE);
      mind->Send("Now '%s' is no longer a True Ninja[TM].\n", comline+len);
      }
    else {
      pl->Set(PLAYER_NINJA);
      mind->Send("You made '%s' into a True Ninja[TM].\n", comline+len);
      }

    return 0;
    }

  if(com == COM_MAKESUPERNINJA) {
    if(!mind) return 0;
    while((!isgraph(comline[len])) && (comline[len])) ++len;

    Player *pl = get_player(comline+len);
    if(strlen(comline+len) <= 0) {
      mind->Send("You can only make other Super Ninjas[TM]\n");
      return 0;
      }
    if(!pl) {
      mind->Send("There is no PLAYER named '%s'\n", comline+len);
      return 0;
      }
    if(pl == mind->Owner()) {
      mind->Send("You can only make other Super Ninjas[TM]\n");
      return 0;
      }

    if(pl->Is(PLAYER_SUPERNINJA)) {
      mind->Send("'%s' is already a Super Ninja[TM] - this is irrevocable.\n", comline+len);
      }
    else if(!pl->Is(PLAYER_NINJA)) {
      mind->Send("'%s' isn't even a True Ninja[TM] yet!\n"
	"Be careful - Super Ninja[TM] status is irrevocable.\n", comline+len);
      }
    else {
      pl->Set(PLAYER_SUPERNINJA);
      mind->Send("You made '%s' into a Super Ninja[TM].\n", comline+len);
      }

    return 0;
    }

  if(com == COM_MAKESTART) {
    if(!mind) return 0;
    set_start_room(body->Parent());
    mind->Send("You make this the default starting room for players.\n");
    return 0;
    }

  if(com == COM_NAME) {
    if(!mind) return 0;
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    if(comline[len] != 0) {
      string oldn = body->Parent()->ShortDesc();
      body->Parent()->SetShortDesc(comline+len);
      mind->Send("You rename \"%s\" to \"%s\"\n", oldn.c_str(),
	body->Parent()->ShortDesc());	//FIXME - Real Message
      }
    else {
      mind->Send("Rename it to what?\n");
      }
    return 0;
    }

  if(com == COM_DESCRIBE) {
    if(!mind) return 0;
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    if(comline[len] != 0) {
      string oldn = body->Parent()->ShortDesc();
      body->Parent()->SetDesc(comline+len);
      mind->Send("You redescribe \"%s\" as \"%s\"\n", oldn.c_str(),
	body->Parent()->Desc());	//FIXME - Real Message
      }
    else {
      mind->Send("Describe it as what?\n");
      }
    return 0;
    }

  if(com == COM_DEFINE) {
    if(!mind) return 0;
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    if(comline[len] != 0) {
      string oldn = body->Parent()->ShortDesc();
      body->Parent()->SetLongDesc(comline+len);
      mind->Send("You redefine \"%s\" as \"%s\"\n", oldn.c_str(),
	body->Parent()->LongDesc());	//FIXME - Real Message
      }
    else {
      mind->Send("Define it as what?\n");
      }
    return 0;
    }

  if(com == COM_CONTROL) {
    if(!mind) return 0;
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    Object *targ = body->PickObject(comline+len, LOC_NEARBY|LOC_INTERNAL);
    if(!targ) {
      mind->Send("You want to control who?\n");
      }
    else if(targ->Stats()->GetAttribute(1) <= 0) {
      mind->Send("You can't control inanimate objects!\n");
      }
    else {
      body->Parent()->SendOut(
	";s controls ;s with Ninja Powers[TM].\n", "You control ;s.\n",
	body, targ);
      mind->Unattach();
      mind->Attach(targ);
      }
    return 0;
    }

  if(com == COM_COMMAND) {
    if(!mind) return 0;
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    Object *targ = body->ActTarg(ACT_POINT);
    if(!targ) {
      mind->Send("You need to be pointing at your target.\n");
      }
    else if(comline[len] == 0) {
      mind->Send("Command %s to do what?\n", targ->ShortDesc());
      }
    else if(targ->Stats()->GetAttribute(5) <= 0) {
      mind->Send("You can't command an object that has no will of its own.\n");
      }
    else {
      body->Parent()->SendOut(
	";s commands ;s to '%s' with Ninja Powers[TM].\n",
	"You command ;s to '%s'.\n",
	body, targ, comline+len);

      if(handle_command(targ, comline+len) > 0)
	body->Parent()->SendOut(
		";s did not understand the command.\n",
		";s did not understand the command.\n",
		targ, body);
      }
    return 0;
    }

  if(com == COM_CREATE) {
    if(!mind) return 0;
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    if(comline[len] == 0) {
      new Object(body->Parent());
      body->Parent()->SendOut(
	";s creates a new object with Ninja Powers[TM].\n",
	"You create a new object.\n", body, NULL);
      }
    else {
      body->Parent()->LinkToNew(comline+len);
      body->Parent()->SendOut(
	";s creates a new object with Ninja Powers[TM].\n",
	"You create a new object.\n", body, NULL);
      }
    return 0;
    }

  if(com == COM_USERS) {
    if(!mind) return 0;
    string users = "Current accounts on this mud:\n";
    vector<Player *> pls = get_all_players();

    vector<Player *>::iterator pl = pls.begin();
    for(; pl != pls.end(); ++pl) {
      users += (*pl)->Name();
      users += "\n";
      }
    mind->Send(users.c_str());

    return 0;
    }

  if(com == COM_CHARS) {
    if(!mind) return 0;
    string chars = "Current characters on this mud:\n";
    vector<Player *> pls = get_all_players();

    vector<Player *>::iterator pl = pls.begin();
    for(; pl != pls.end(); ++pl) {
      set<Object *> chs = (*pl)->Room()->Contents();
      set<Object *>::iterator ch = chs.begin();
      for(; ch != chs.end(); ++ch) {
	chars += (*pl)->Name();
	chars += ": ";
	chars += (*ch)->ShortDesc();
	chars += "\n";
	}
      }
    mind->Send(chars.c_str());

    return 0;
    }

  if(com == COM_RESET) {
    if(!mind) return 0;
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    Object *targ = body->PickObject(comline+len, LOC_NEARBY|LOC_INTERNAL|LOC_SELF);
    if(!targ) {
      mind->Send("You want to reset what?\n");
      }
    else {
      body->Parent()->SendOut(
	";s resets ;s with Ninja Powers[TM].\n", "You reset ;s.\n",
	body, targ);

      set<Object *> contents = targ->Contents();
      set<Object *>::iterator item = contents.begin();
      for(; item != contents.end(); ++item) delete (*item);
      targ->Travel(get_start_room());
      }
    return 0;
    }

  if(com == COM_CLONE) {
    if(!mind) return 0;
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    Object *targ = body->PickObject(comline+len, LOC_NEARBY|LOC_INTERNAL);
    if(!targ) {
      mind->Send("You want to clone what?\n");
      }
    else {
      body->Parent()->SendOut(
	";s clones ;s with Ninja Powers[TM].\n", "You clone ;s.\n",
	body, targ);
      Object *nobj = new Object(*targ);
      nobj->SetParent(targ->Parent());
      }
    return 0;
    }

  if(com == COM_JUNK) {
    if(!mind) return 0;
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    Object *targ = body->PickObject(comline+len, LOC_NEARBY|LOC_INTERNAL);
    if(!targ) {
      mind->Send("You want to destroy what?\n");
      }
    else {
      body->Parent()->SendOut(
	";s destroys ;s with Ninja Powers[TM].\n", "You destroy ;s.\n",
	body, targ);
      delete(targ);
      }
    return 0;
    }

  if(com == COM_HEAL) {
    if(!mind) return 0;
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    Object *targ = body->PickObject(comline+len, LOC_NEARBY|LOC_INTERNAL|LOC_SELF);
    if(!targ) {
      mind->Send("You want to heal what?\n");
      }
    else {
      body->Parent()->SendOut(
	";s heals and repairs ;s with Ninja Powers[TM].\n", "You heal ;s.\n",
	body, targ);

      //This is ninja-healing and bypasses all healing mechanisms.
      stats_t st = (*(targ->Stats()));
      st.phys = 0;
      st.stun = 0;
      st.stru = 0;
      targ->SetStats(st);
      }
    return 0;
    }

  const char *statnames[] = {
	"Body", "Quickness", "Strength",
	"Charisma", "Intelligence", "Willpower"
	};

  if(com == COM_JACK) {
    if(!mind) return 0;
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    Object *targ = body->ActTarg(ACT_POINT);
    if(!targ) {
      mind->Send("You need to be pointing at your target.\n");
      return 0;
      }
    int stat = 0;
    if(toupper(comline[len]) == 'Q') stat = 1;
    if(toupper(comline[len]) == 'S') stat = 2;
    if(toupper(comline[len]) == 'C') stat = 3;
    if(toupper(comline[len]) == 'I') stat = 4;
    if(toupper(comline[len]) == 'W') stat = 5;

    if(targ->Stats()->GetAttribute(stat) == 0) {
      mind->Send("This object doesn't have that stat.\n");
      return 0;
      }

    body->Parent()->SendOut(
	";s jacks the %s of ;s with Ninja Powers[TM].\n",
	"You jack the %s of ;s.\n",
	body, targ, statnames[stat]);

    stats_t st = (*(targ->Stats()));
    st.SetAttribute(stat, st.GetAttribute(stat) + 1);
    targ->SetStats(st);
    return 0;
    }

  if(com == COM_CHUMP) {
    if(!mind) return 0;
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    Object *targ = body->ActTarg(ACT_POINT);
    if(!targ) {
      mind->Send("You need to be pointing at your target.\n");
      return 0;
      }
    int stat = 0;
    if(toupper(comline[len]) == 'Q') stat = 1;
    if(toupper(comline[len]) == 'S') stat = 2;
    if(toupper(comline[len]) == 'C') stat = 3;
    if(toupper(comline[len]) == 'I') stat = 4;
    if(toupper(comline[len]) == 'W') stat = 5;

    if(targ->Stats()->GetAttribute(stat) == 0) {
      mind->Send("This object doesn't have that stat.\n");
      return 0;
      }
    if(targ->Stats()->GetAttribute(stat) == 1) {
      mind->Send("It is already a 1 (the minimum!).\n");
      return 0;
      }

    body->Parent()->SendOut(
	";s chumps the %s of ;s with Ninja Powers[TM].\n",
	"You chump the %s of ;s.\n",
	body, targ, statnames[stat]);

    stats_t st = (*(targ->Stats()));
    st.SetAttribute(stat, st.GetAttribute(stat) - 1);
    targ->SetStats(st);
    return 0;
    }

  if(com == COM_INCREMENT) {
    if(!mind) return 0;
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    Object *targ = body->ActTarg(ACT_POINT);
    if(!targ) {
      mind->Send("You need to be pointing at your target.\n");
      return 0;
      }

    map<string,int> skls = get_skills();
    if(!skls.count(comline+len)) {
      mind->Send("Sorry, '%s' is not implemented at this point.\n",
		comline+len);
      return 0;
      }

    stats_t st = (*(targ->Stats()));
    st.SetSkill(comline+len, (st.GetSkill(comline+len)>?0) + 1);
    targ->SetStats(st);

    body->Parent()->SendOut(
	";s increments the %s of ;s with Ninja Powers[TM].\n",
	"You increment the %s of ;s.\n",
	body, targ, comline+len);

    return 0;
    }

  if(com == COM_DECREMENT) {
    if(!mind) return 0;
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    Object *targ = body->ActTarg(ACT_POINT);
    if(!targ) {
      mind->Send("You need to be pointing at your target.\n");
      return 0;
      }

    stats_t st = (*(targ->Stats()));
    st.SetSkill(comline+len, st.GetSkill(comline+len) - 1);
    targ->SetStats(st);

    body->Parent()->SendOut(
	";s decrements the %s of ;s with Ninja Powers[TM].\n",
	"You decrement the %s of ;s.\n",
	body, targ, comline+len);

    return 0;
    }

  if(com == COM_CLOAD) {
    if(!mind) return 0;
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    if(!comline[len]) {
      body->Parent()->CircleLoadAll();
      body->Parent()->SendOut(
	";s loads the entire Circle world with Ninja Powers[TM].\n",
	"You load the entire Circle world.\n", body, NULL);
      }
    else {
      sprintf(buf, "circle/wld/%s.wld", comline+len);
      body->Parent()->CircleLoad(buf);
      sprintf(buf, "circle/obj/%s.obj", comline+len);
      body->Parent()->CircleLoadObj(buf);
      body->Parent()->SendOut(
	";s loads a Circle world ;s with Ninja Powers[TM].\n",
	"You load a Circle world.\n", body, NULL);
      }
    return 0;
    }

  if(com == COM_CCLEAN) {
    if(!mind) return 0;
    body->CircleCleanup();
    body->Parent()->SendOut(
	";s cleans up after loading Circle worlds.\n",
	"You clean up after loading Circle worlds.\n",
	body, NULL);
    return 0;
    }

  mind->Send("Sorry, that command's not yet implemented.\n");
  return 0;
  }

int handle_command(Object *body, const char *cl, Mind *mind) {
  static char *buf = NULL;
  static int bsize = -1;
  int ret = 0;
  char *start = (char*)cl, *end = (char*)cl;

  while(1) {
    if((*end) == '\n' || (*end) == '\r' || (*end) == ';' || (*end) == 0) {
      if(end > start) {
	int len = end-start;
	if(bsize < len) {
	  bsize = ((len+4095)/4096)*4096;
	  if(buf) delete buf;
	  buf = new char[bsize];
	  }
	memcpy(buf, start, len);
	buf[len] = 0;

	int stat = handle_single_command(body, buf, mind);
	if(stat < 0) return stat;
	else if(ret == 0) ret = stat;
	}
      start = end;  ++start;
      }
    if((*end) == 0) break;
    ++end;
    }
  return ret;
  }

//int handle_command(Object *obj, const char *cl) {
////  fprintf(stderr, "Doing '%s' for %p\n", cl, obj);
//
//  static Mind mind;
//  mind.Attach(obj);
//  int ret = handle_command(obj, cl, &mind);
//  mind.Unattach();
//  return ret;
//  }
