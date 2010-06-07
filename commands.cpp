#include <algorithm>
#include <cstdio>
#include <ctype.h>
#include <cstring>
#include <unistd.h>

using namespace std;

#include "utils.h"
#include "net.h"
#include "main.h"
#include "mind.h"
#include "object.h"
#include "commands.h"
#include "color.h"
#include "version.h"

#define SIT_ETHEREAL	1
#define SIT_CORPOREAL	2
#define SIT_ALIVE	4
#define SIT_CONSCIOUS	8
#define SIT_AWAKE	16
#define SIT_ALERT	32
#define SIT_LIE		64
#define SIT_SIT		128
#define SIT_STAND	256
#define SIT_USE		512
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
#define REQ_UP		(SIT_STAND | SIT_USE | REQ_CORPOREAL)

#define REQ_ACTION	(SIT_ACTION | REQ_CORPOREAL)

#define REQ_NINJAMODE	(SIT_NINJAMODE)
#define REQ_NINJA	(SIT_NINJA)
#define REQ_SUPERNINJA	(SIT_SUPERNINJA|SIT_NINJA)

#define REQ_ANY		(SIT_CORPOREAL|SIT_ETHEREAL)

int handle_command_ccreate(Object *, Mind *, const char *, int, int, int);

struct Command {
  int id;
  const char *command;
  const char *shortdesc;
  const char *longdesc;
  int sit;
  };

enum {	COM_HELP=0,
	COM_QUIT,

	COM_NORTH,
	COM_SOUTH,
	COM_EAST,
	COM_WEST,
	COM_UP,
	COM_DOWN,

	COM_LOOK,
	COM_EXAMINE,
	COM_CONSIDER,
	COM_INVENTORY,
	COM_EQUIPMENT,
	COM_SEARCH,
	COM_HIDE,

	COM_LEAVE,
	COM_ENTER,
	COM_SELECT,

	COM_OPEN,
	COM_CLOSE,
	COM_UNLOCK,
	COM_LOCK,

	COM_GET,
	COM_DRAG,
	COM_PUT,
	COM_DROP,
	COM_WIELD,
	COM_HOLD,
	COM_WEAR,
	COM_REMOVE,

	COM_EAT,
	COM_DRINK,
	COM_FILL,
	COM_DUMP,

	COM_SLEEP,
	COM_WAKE,
	COM_LIE,
	COM_REST,
	COM_SIT,
	COM_STAND,
	COM_USE,
	COM_STOP,

	COM_SHOUT,
	COM_YELL,
	COM_CALL,
	COM_SAY,
	COM_EMOTE,

	COM_POINT,
	COM_FOLLOW,
	COM_ATTACK,
	COM_KILL,
	COM_PUNCH,
	COM_KICK,

	COM_LIST,
	COM_BUY,
	COM_VALUE,
	COM_SELL,

	COM_PLAYERS,
	COM_DELPLAYER,
	COM_CHARS,
	COM_WHO,
	COM_OOC,
	COM_NEWBIE,

	COM_NEWCHARACTER,
	COM_RAISE,
	COM_RANDOMIZE,

	COM_SKILLLIST,

	COM_RECALL,

	COM_NINJAMODE,
	COM_MAKENINJA,
	COM_MAKESUPERNINJA,

	COM_RESET,
	COM_CREATE,
	COM_DCREATE,
	COM_CCREATE,
	COM_ANCHOR,
	COM_LINK,
	COM_COMMAND,
	COM_CONTROL,
	COM_CLONE,
	COM_MIRROR,
	COM_JUNK,
	COM_HEAL,
	COM_JACK,
	COM_CHUMP,
	COM_INCREMENT,
	COM_DECREMENT,
	COM_DOUBLE,

	COM_SCORE,

	COM_STATS,
	COM_SHUTDOWN,
	COM_RESTART,
	COM_SAVEALL,
	COM_VERSION,

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

  { COM_NORTH, "north",
    "Travel north.",
    "Travel north.",
    (REQ_ALERT|REQ_UP|REQ_ACTION)
    },
  { COM_SOUTH, "south",
    "Travel south.",
    "Travel south.",
    (REQ_ALERT|REQ_UP|REQ_ACTION)
    },
  { COM_EAST, "east",
    "Travel east.",
    "Travel east.",
    (REQ_ALERT|REQ_UP|REQ_ACTION)
    },
  { COM_WEST, "west",
    "Travel west.",
    "Travel west.",
    (REQ_ALERT|REQ_UP|REQ_ACTION)
    },
  { COM_UP, "up",
    "Travel up.",
    "Travel up.",
    (REQ_ALERT|REQ_UP|REQ_ACTION)
    },
  { COM_DOWN, "down",
    "Travel down.",
    "Travel down.",
    (REQ_ALERT|REQ_UP|REQ_ACTION)
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
  { COM_CONSIDER, "consider",
    "Consider attacking a person or creature and size it up.",
    "Consider attacking a person or creature and size it up.",
    (REQ_ALERT|REQ_ACTION)
    },
  { COM_INVENTORY, "inventory",
    "Check what you have on you.",
    "Check what you have on you.",
    (REQ_AWAKE|REQ_ACTION)
    },
  { COM_EQUIPMENT, "equipment",
    "Check what you are wearing and using.",
    "Check what you are wearing and using.",
    (REQ_AWAKE|REQ_ACTION)
    },
  { COM_SEARCH, "search",
    "Search an area, object or creature.",
    "Search an area, object or creature.",
    (REQ_ALERT|REQ_UP|REQ_ACTION)
    },
  { COM_HIDE, "hide",
    "Hide an object, or yourself.",
    "Hide an object, or yourself.",
    (REQ_ALERT|REQ_UP|REQ_ACTION)
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
  { COM_SELECT, "select",
    "Select an unfinished character.",
    "Select an unfinished character.",
    (REQ_ETHEREAL)
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
  { COM_UNLOCK, "unlock",
    "Unlock a door or container.",
    "Unlock a door or container.",
    (REQ_ALERT|REQ_STAND|REQ_ACTION)
    },
  { COM_LOCK, "lock",
    "Lock a door or container.",
    "Lock a door or container.",
    (REQ_ALERT|REQ_STAND|REQ_ACTION)
    },

  { COM_GET, "get",
    "Get an item from your surroundings.",
    "Get an item from your surroundings.",
    (REQ_ALERT|REQ_STAND|REQ_ACTION)
    },
  { COM_DRAG, "drag",
    "Drag a heavy item with you when you next move.",
    "Drag a heavy item with you when you next move.",
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
  { COM_DUMP, "dump",
    "Dump all liquid out of a container you are carrying.",
    "Dump all liquid out of a container you are carrying.",
    (REQ_ALERT|REQ_ACTION)
    },
  { COM_FILL, "fill",
    "Fill a held liquid container from another.",
    "Fill a held liquid container from another.",
    (REQ_ALERT|REQ_ACTION)
    },
  { COM_EAT, "eat",
    "Eat an item you are carrying.",
    "Eat an item you are carrying.",
    (REQ_ALERT|REQ_ACTION)
    },
  { COM_DRINK, "drink",
    "Drink from an item you are carrying.",
    "Drink from an item you are carrying.",
    (REQ_ALERT|REQ_ACTION)
    },
  { COM_WIELD, "wield",
    "Wield a weapon you are carrying.",
    "Wield a weapon you are carrying.",
    (REQ_ALERT|REQ_ACTION)
    },
  { COM_HOLD, "hold",
    "Hold an item you are carrying in your off-hand.",
    "Hold an item you are carrying in your off-hand.",
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
  { COM_USE, "use",
    "Start, or stop using a skill.",
    "Start, or stop using a skill.",
    (REQ_ALERT|REQ_UP|REQ_ACTION)
    },
  { COM_STOP, "stop",
    "Stop using a skill.",
    "Stop using a skill.",
    (REQ_ALERT|REQ_UP|REQ_ACTION)
    },

  { COM_SHOUT, "shout",
    "Shout something to all nearby.",
    "Shout something to all nearby.",
    (REQ_AWAKE)
    },
  { COM_YELL, "yell",
    "Shout something to all nearby.",
    "Shout something to all nearby.",
    (REQ_AWAKE)
    },
  { COM_CALL, "call",
    "Shout something to all nearby.",
    "Shout something to all nearby.",
    (REQ_AWAKE)
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
  { COM_FOLLOW, "follow",
    "Follow someone, or stop following someone.",
    "Follow someone, or stop following someone.",
    (REQ_ALERT|REQ_UP)
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

  { COM_LIST, "list",
    "List items available at a shop.",
    "List items available at a shop.",
    (REQ_ALERT|REQ_ACTION)
    },
  { COM_BUY, "buy",
    "Buy an item at a shop.",
    "Buy an item at a shop.",
    (REQ_ALERT|REQ_ACTION)
    },
  { COM_VALUE, "value",
    "Find out how much a shop will give you for an item.",
    "Find out how much a shop will give you for an item.",
    (REQ_ALERT|REQ_ACTION)
    },
  { COM_SELL, "sell",
    "Sell an item at a shop.",
    "Sell an item at a shop.",
    (REQ_ALERT|REQ_ACTION)
    },

  { COM_WHO, "who",
    "Get a list of who is on the MUD right now.",
    "Get a list of who is on the MUD right now.",
    (REQ_ANY)
    },
  { COM_OOC, "ooc",
    "Toggle or use ooc (Out-Of-Character) chat.",
    "Toggle or use ooc (Out-Of-Character) chat.",
    (REQ_ANY)
    },
  { COM_NEWBIE, "newbie",
    "Toggle or use newbie (new player) chat (use this to ask for help).",
    "Toggle or use newbie (new player) chat (use this to ask for help).",
    (REQ_ANY)
    },

  { COM_NEWCHARACTER, "newcharacter",
    "Create a new character.",
    "Create a new character.",
    (REQ_ETHEREAL)
    },
  { COM_RAISE, "raise",
    "Spend a skill or attribute point of current character.",
    "Spend a skill or attribute point of current character.",
    (REQ_ETHEREAL|REQ_CORPOREAL)
    },
  { COM_RANDOMIZE, "randomize",
    "Spend all remaining points of current character randomly.",
    "Spend all remaining points of current character randomly.",
    (REQ_ETHEREAL)
    },
  { COM_SCORE, "score",
    "Get your current character and/or player's stats and score.",
    "Get your current character and/or player's stats and score.",
    (REQ_CORPOREAL)
    },

  { COM_SKILLLIST, "skilllist",
    "List all available skill categories, or all skills in a category.",
    "List all available skill categories, or all skills in a category.",
    (REQ_ANY)
    },

  { COM_RECALL, "recall",
    "Teleport back to the start when you are uninjured.",
    "Teleport back to the start when you are uninjured.",
    (REQ_STAND)
    },

  { COM_NINJAMODE, "ninjamode", 
    "Ninja command: run a command in Ninja Mode[TM].",
    "Ninja command: run a command in Ninja Mode[TM].",
    (REQ_ETHEREAL|REQ_ANY|REQ_ACTION|REQ_NINJA)
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

  { COM_STATS, "stats",
    "Ninja command: Get stats of a character, object or creature.",
    "Ninja command: Get stats of a character, object or creature.",
    (REQ_ALERT|REQ_NINJAMODE)
    },
  { COM_CREATE, "create",
    "Ninja command.",
    "Ninja command - ninjas only!",
    (REQ_ALERT|REQ_NINJAMODE)
    },
  { COM_DCREATE, "dcreate",
    "Ninja command.",
    "Ninja command - ninjas only!",
    (REQ_ALERT|REQ_NINJAMODE)
    },
  { COM_CCREATE, "ccreate",
    "Ninja command.",
    "Ninja command - ninjas only!",
    (REQ_ALERT|REQ_NINJAMODE)
    },
  { COM_ANCHOR, "anchor",
    "Ninja command.",
    "Ninja command - ninjas only!",
    (REQ_ALERT|REQ_NINJAMODE)
    },
  { COM_LINK, "link",
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
  { COM_MIRROR, "mirror",
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

  { COM_PLAYERS, "players",
    "Ninja command.",
    "Ninja command - ninjas only!",
    (REQ_ANY|REQ_NINJAMODE)
    },
  { COM_DELPLAYER, "delplayer",
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
  { COM_DOUBLE, "double",
    "Ninja command.",
    "Ninja command - ninjas only!",
    (REQ_ALERT|REQ_NINJAMODE)
    },

  { COM_SHUTDOWN, "shutdown", 
    "Ninja command: shutdown the entire MUD.",
    "Ninja command: shutdown the entire MUD.",
    (REQ_ANY|REQ_NINJAMODE)
    },
  { COM_RESTART, "restart", 
    "Ninja command: restart the entire MUD - preserving connections.",
    "Ninja command: restart the entire MUD - preserving connections.",
    (REQ_ANY|REQ_NINJAMODE)
    },
  { COM_SAVEALL, "saveall", 
    "Ninja command: save the entire MUD Universe - preserving connections.",
    "Ninja command: save the entire MUD Universe - preserving connections.",
    (REQ_ANY|REQ_NINJAMODE)
    },
  { COM_VERSION, "version", 
    "Ninja command: query the version information of running AcidMUD.",
    "Ninja command: query the version information of running AcidMUD.",
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


//Return values: -1: Player D/Ced
//                0: Command Understood
//                1: Command NOT Understood
//                2: Command Understood - No More Actions This Round
int handle_single_command(Object *body, const char *comline, Mind *mind) {
  int len;
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

  //fprintf(stderr, "Handling command from %p[%p] of '%s'\n", mind, body, comline);

  if(comline[0] == '\'' || comline[0] == '"') {	//Command Alias: "say"
    len = 1;
    }
  else {
    for(len=0; isgraph(comline[len]); ++len); 
    }

  if(len == 0) return 0;

  int com = -1, cnum = -1;
  for(int ctr=0; ctr < comnum; ++ctr) {
    if(!strncasecmp(comline, comlist[ctr].command, len))
      { com = comlist[ctr].id; cnum = ctr; break; }

    //Command Aliases
    if(comlist[ctr].id == COM_SAY && (comline[0]=='\'' || comline[0]=='"')) {
      com = comlist[ctr].id; cnum = ctr; break;
      }
    }
  if(com == -1) {
    if(mind) mind->Send("Command NOT understood - type 'help' for assistance.\n");
    return 1;
    }

  int ninja=0, sninja=0, nmode=0;

  if(mind && mind->Owner() && mind->Owner()->Is(PLAYER_SUPERNINJA))
    { sninja=1; ninja=1; }
  if(mind && mind->Owner() && mind->Owner()->Is(PLAYER_NINJA)) ninja=1;
  if(mind && mind->Owner() && mind->Owner()->Is(PLAYER_NINJAMODE)) nmode=1;

  if((!sninja) && (comlist[cnum].sit & SIT_SUPERNINJA)) {
    if(mind) mind->Send("Sorry, that command is for Super Ninjas only!\n");
    return 0;
    }

  if((!ninja) && (comlist[cnum].sit & SIT_NINJA)) {
    if(mind) mind->Send("Sorry, that command is for True Ninjas only!\n");
    return 0;
    }

  if((!nmode) && (comlist[cnum].sit & SIT_NINJAMODE)) {
    if(mind && ninja)
      mind->Send("Sorry, you need to be in Ninja Mode[TM] to do that.\n");
    else if(mind)
      mind->Send("Sorry, that command is for Ninjas only!\n");
    return 0;
    }

  if((!(comlist[cnum].sit & SIT_ETHEREAL)) && (!body)) {
    if(mind) mind->Send("You can't use that command until you join the game - with the 'enter' command.\n");
    return 0;
    }

  if((!(comlist[cnum].sit & SIT_CORPOREAL)) && (body)) {
    if(mind) mind->Send("You can't use that command until you quit the game - with the 'quit' command.\n");
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
    if((comlist[cnum].sit & (SIT_STAND|SIT_USE)) == (SIT_STAND|SIT_USE)) {
      if(body->Pos() != POS_STAND && body->Pos() != POS_USE) {
        if(mind) mind->Send("You must stand up to use that command.\n");
	handle_single_command(body, "stand", mind);
	if(body->Pos() != POS_STAND && body->Pos() != POS_USE)
	  return 0;
        }
      }
    else if((comlist[cnum].sit & (SIT_STAND|SIT_SIT)) == (SIT_STAND|SIT_SIT)) {
      if(body->Pos() == POS_USE) {
        if(mind) mind->Send("You must stop using this skill to do that.\n");
	handle_single_command(body, "stop", mind);
	if(body->Pos() != POS_STAND) return 0;
	}
      else if(body->Pos() != POS_SIT && body->Pos() != POS_STAND) {
        if(mind) mind->Send("You must at least sit up to use that command.\n");
	handle_single_command(body, "sit", mind);
	if(body->Pos() != POS_SIT && body->Pos() != POS_STAND)
	  return 0;
        }
      }
    else if(comlist[cnum].sit & SIT_STAND) {
      if(body->Pos() == POS_USE) {
        if(mind) mind->Send("You must stop using this skill to do that.\n");
	handle_single_command(body, "stop", mind);
	if(body->Pos() != POS_STAND) return 0;
	}
      else if(body->Pos() != POS_STAND) {
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
      if(body->IsAct(ACT_SLEEP)) {
        if(mind) mind->Send("You must be awake to use that command.\n");
	handle_single_command(body, "wake", mind);
        if(body->IsAct(ACT_SLEEP)) return 0;
        }
      if(body->IsAct(ACT_REST)) {
        if(mind) mind->Send("You must be alert to use that command.\n");
	handle_single_command(body, "rest", mind);
        if(body->IsAct(ACT_REST)) return 0;
        }
      }
    if(comlist[cnum].sit & SIT_AWAKE) {
      if(body->IsAct(ACT_SLEEP)) {
        if(mind) mind->Send("You must be awake to use that command.\n");
	handle_single_command(body, "wake", mind);
        if(body->IsAct(ACT_SLEEP)) return 0;
        }
      }
    }

  int stealth_t = 0, stealth_s = 0;

  if(body && body->IsUsing("Stealth") && body->Skill("Stealth") > 0) {
    stealth_t = body->Skill("Stealth");
    stealth_s = body->Roll("Stealth", 2);
    }

  if(com == COM_VERSION) {
    if(mind) mind->Send("Version of this MUD is %d.%d.%d-%d: %s.\n",
	CurrentVersion.acidmud_version[0], CurrentVersion.acidmud_version[1],
	CurrentVersion.acidmud_version[2], CurrentVersion.acidmud_version[3],
	CurrentVersion.acidmud_datestamp.c_str());
    return 0;
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

  if(com == COM_SAVEALL) {
    shutdn = -1;
    if(mind) mind->Send("You instruct the system to save all.\n");
    return 0;
    }

  if(com == COM_SELECT) {
    Object *body = mind->Owner()->Room()->PickObject(comline+len, LOC_INTERNAL);
    if(!body) {
      mind->Send("Sorry, that character doesn't exist.\n"
		"Use the 'newcharacter' command to create a new character.\n");
      return 0;
      }
    else {
      mind->Send("'%s' is now selected as your currect character to work on.\n",
		body->Name());
      mind->Owner()->SetCreator(body);
      return 0;
      }
    }

  if(com == COM_NORTH)	{ com = COM_ENTER; len = 6; comline = "enter north"; }
  if(com == COM_SOUTH)	{ com = COM_ENTER; len = 6; comline = "enter south"; }
  if(com == COM_EAST)	{ com = COM_ENTER; len = 6; comline = "enter east"; }
  if(com == COM_WEST)	{ com = COM_ENTER; len = 6; comline = "enter west"; }
  if(com == COM_UP)	{ com = COM_ENTER; len = 6; comline = "enter up"; }
  if(com == COM_DOWN)	{ com = COM_ENTER; len = 6; comline = "enter down"; }

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
		"Use the 'newcharacter' command to create a new character.\n");
	return 0;
	}
      if(body->Skill("Attributes") || body->Skill("Skills")) {
	mind->Send("You need to finish that character before you can use it.\n");
	mind->Send("'%s' is now selected as your currect character to work on.\n",
		body->Name());
	mind->Owner()->SetCreator(body);
	return 0;
	}
      if(body->IsAct(ACT_DEAD)) {
	if(nmode) {
	  // Allow entry to ninjas - autoheal!
	  }
	else {
	  mind->Send("Sorry, that character is dead.\n"
		"Use the 'newcharacter' command to create a new character.\n");
	  return 0;
	  }
	}

      //FIXME: Handle conversion of body->Skill("Resources").
      if(mind->Owner()->Creator() == body) mind->Owner()->SetCreator(NULL);

      mind->Attach(body);

      if(nmode) {
	//This is ninja-healing and bypasses all healing mechanisms.
	body->SetStun(0);
	body->SetPhys(0);
	body->SetStru(0);
	body->UpdateDamage();
	body->Parent()->SendOut(stealth_t, stealth_s, 
	  ";s heals and repairs ;s with Ninja Powers[TM].\n", "You heal ;s.\n",
	  body, body);
	}

      if(body->IsAct(ACT_DYING))
	mind->Send("You can see nothing, you are too busy dying.\n");
      else if(body->IsAct(ACT_SLEEP))
	mind->Send("You can see nothing since you are asleep.\n");
      else if(body->IsAct(ACT_UNCONSCIOUS))
	mind->Send("You can see nothing, you are out cold.\n");
      else
	body->Parent()->SendDescSurround(mind, body);
      return 0;
      }
    Object *dest = body->PickObject(comline+len, LOC_NEARBY);
    Object *rdest = dest;
    Object *veh = body;

    if(!dest) {
      if(mind) mind->Send("You want to go where?\n");
      return 0;
      }

    if(dest->ActTarg(ACT_SPECIAL_LINKED)
		&& dest->ActTarg(ACT_SPECIAL_LINKED)->Parent()) {
      rdest = dest->ActTarg(ACT_SPECIAL_LINKED)->Parent();
      }

    if((!dest->Skill("Enterable")) && (!ninja)) {
      if(mind) mind->Send("It is not possible to enter that object!\n");
      }
    else if((!dest->Skill("Enterable"))	&& (!nmode)) {
      if(mind) mind->Send("You need to be in ninja mode to enter that object!\n");
      }
    else if(dest->Skill("Open") < 1 && (!nmode)) {
      if(mind) mind->Send("Sorry, %s is closed!\n", dest->Name());
      }
    else if(dest->Parent() != body->Parent()
	&& dest->Parent() == body->Parent()->Parent()
	&& body->Parent()->Skill("Vehicle") == 0) {
      if(mind)
	mind->Send("You can't get %s to go there!\n", body->Parent()->Name(1));
      }
    else if(dest->Parent() != body->Parent()
	&& dest->Parent() == body->Parent()->Parent()
	&& (!(body->Parent()->Skill("Vehicle") & 0xFFF0))    //No Land Travel!
	&& body->Parent()->Parent()->Skill("WaterDepth") == 0
	&& rdest->Skill("WaterDepth") == 0) {
      if(mind)
	mind->Send("You can't get %s to go there!\n", body->Parent()->Name(1));
      }
    else {
      if(nmode) {
	//Ninja-movement can't be followed!
	if(body->Parent()) body->Parent()->NotifyGone(body);
	}
      if(dest->Parent() != body->Parent()
		&& dest->Parent() == body->Parent()->Parent()) {
	if(body->Parent()->Skill("Vehicle") == 4 && body->Skill("Boat") == 0) {
	  if(mind) mind->Send("You don't know how to operate %s!\n",
		body->Parent()->Name(1));
	  return 0;
	  }
	veh = body->Parent();
	}

      if(rdest->Skill("WaterDepth") == 1 && body->Skill("Swimming") == 0) {
	if(veh == body || (veh->Skill("Vehicle") & 4) == 0) {	//Have boat?
	  if(mind) mind->Send("Sorry, but you can't swim!\n");
	  return 0;
	  }
	}
      else if(rdest->Skill("WaterDepth") > 1) {
	if(veh == body || (veh->Skill("Vehicle") & 4) == 0) {	//Have boat?
	  if(mind) mind->Send("Sorry, you need a boat to go there!\n");
	  return 0;
	  }
	}

      if(dest->ActTarg(ACT_SPECIAL_LINKED)
		&& dest->ActTarg(ACT_SPECIAL_LINKED)->Parent()) {
	body->Parent()->SendOut(stealth_t, stealth_s, ";s leaves ;s.\n", "", body, dest);
	dest = dest->ActTarg(ACT_SPECIAL_LINKED)->Parent();
	}
      else if(body->Parent()) {
	body->Parent()->SendOut(stealth_t, stealth_s, ";s enters ;s.\n", "", body, dest);
	}

      if(veh->Travel(dest)) {
	body->Parent()->SendOut(stealth_t, stealth_s, "...but ;s didn't seem to fit!\n",
		"You could not fit!\n", body, NULL);
	}
      else {
	body->Parent()->SendOut(stealth_t, stealth_s, 
		";s arrives.\n", "", body, NULL);
	if(mind && mind->Type() == MIND_REMOTE)
	  body->Parent()->SendDescSurround(body, body);
	else if(mind && mind->Type() == MIND_SYSTEM)
	  mind->Send("You enter %s\n", comline+len);

	if(stealth_t > 0) {
	  body->SetSkill("Hidden", body->Roll("Stealth", 2) * 2);
	  }
	}
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

    if(mind && mind->Owner() && mind->Owner()->Room()) {
      mind->Owner()->Room()->SendDesc(mind);
      mind->Owner()->Room()->SendContents(mind);
      }
    else
      if(mind) mind->Send("Use 'Enter' to return to the game.\n");

    return 0;
    }

  if(com == COM_HELP) {
    if((!mind) || (!mind->Owner())) return 0;
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    if(!strcmp(comline+len, "commands")) {
      string mes = "";
      for(int ctr=0; ctr < comnum; ++ctr) {
        if((comlist[ctr].sit & SIT_NINJAMODE) && (!nmode)) continue;
        if((!(comlist[ctr].sit & SIT_NINJAMODE)) && nmode) continue;
        if((comlist[ctr].sit & SIT_NINJA) && (!ninja)) continue;
        if((comlist[ctr].sit & SIT_SUPERNINJA) && (!sninja)) continue;
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
    int shouting = 1;
    if(strlen(comline+len) < 4) shouting = 0;
    for(const char *chr = comline+len; shouting && *chr != 0; ++chr) {
      if(islower(*chr)) shouting = 0;
      }
    if(!shouting) {
      body->Parent()->SendOut(ALL, 0, ";s says '%s'\n", "You say '%s'\n",
	body, body, comline+len);
      body->SetSkill("Hidden", 0);
      return 0;
      }
    else {
      com = COM_SHOUT;
      }
    }

  if(com == COM_SHOUT || com == COM_YELL || com == COM_CALL) {
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    if(!strncasecmp(comline+len, "for ", 4)) len += 4;

    char *mes = strdup(comline+len);
    for(char *chr = mes; *chr != 0; ++chr) {
      *chr = toupper(*chr);
      }
    body->Parent()->SendOut(ALL, 0,
	";s shouts '%s'!!!\n", "You shout '%s'!!!\n",
	body, body, mes);
    body->Parent()->Loud(body->Skill("Strength"), "someone shout '%s'!!!", mes);
    free(mes);

    body->SetSkill("Hidden", 0);
    return 0;
    }

  if(com == COM_EMOTE) {
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    body->Parent()->SendOut(ALL, 0, ";s %s\n", "Your character %s\n",
	body, body, comline+len);
    body->SetSkill("Hidden", 0);
    return 0;
    }

  if(com == COM_INVENTORY) {
   if(mind) {
     mind->Send("You (%s) are carrying:\n", body->ShortDesc());
     body->SendExtendedActions(mind, 1);
     }
   return 0;
   }

  if(com == COM_EQUIPMENT) {
   if(mind) {
     mind->Send("You (%s) are using:\n", body->ShortDesc());
     body->SendExtendedActions(mind, 0);
     }
   return 0;
   }

  if(com == COM_LOOK) {
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    if(!body) {
      mind->Owner()->Room()->SendDesc(mind);
      mind->Owner()->Room()->SendContents(mind);
      return 0;
      }

    typeof(body->Contents()) targs;
    int within = 0;

    if(!strncasecmp(comline+len, "at ", 3)) len += 3;
    if(!strncasecmp(comline+len, "in ", 3)) { len += 3; within = 1; }

    if(strlen(comline+len) > 0) {
      targs = body->PickObjects(comline+len,
	LOC_NEARBY|LOC_ADJACENT|LOC_SELF|LOC_INTERNAL);
      }
    else {
      targs.push_back(body->Parent());
      }

    if(targs.size() < 1) {
      if(mind) mind->Send("You don't see that here.\n");
      return 0;
      }

    typeof(targs.begin()) targ_it;
    for(targ_it = targs.begin(); targ_it != targs.end(); ++targ_it) {
      if(within
		&& (!(*targ_it)->Skill("Container"))
		&& (!(*targ_it)->Skill("Liquid Container"))
		) {
	if(mind) mind->Send(
		"You can't look inside %s, it is not a container.\n",
		(*targ_it)->Name()
		);
	}
      else if(within && ((*targ_it)->Skill("Locked"))) {
	if(mind) mind->Send("You can't look inside %s, it is locked.\n",
		(*targ_it)->Name());
	}
      else {
	int must_open = within;
	if(within && (*targ_it)->Skill("Open")) must_open = 0;

	if(must_open) {
	  (*targ_it)->SetSkill("Open", 1);
	  body->Parent()->SendOut(stealth_t, stealth_s, 
		";s opens ;s.\n", "You open ;s.\n", body, (*targ_it));
	  }

	if(strlen(comline+len) <= 0) {
		body->Parent()->SendOut(stealth_t, stealth_s, 
		";s looks around.\n", "", body, (*targ_it));
	  if(mind) (*targ_it)->SendDescSurround(mind, body);
	  }
	else if((!strcasecmp(comline+len, "north"))
		|| (!strcasecmp(comline+len, "south"))
		|| (!strcasecmp(comline+len, "east"))
		|| (!strcasecmp(comline+len, "west"))
		|| (!strcasecmp(comline+len, "up"))
		|| (!strcasecmp(comline+len, "down"))
		) {
	  body->Parent()->SendOut(stealth_t, stealth_s, 
		";s looks ;s.\n", "", body, (*targ_it));
	  if(mind) {
	    (*targ_it)->SendDesc(mind, body);
	    (*targ_it)->SendExtendedActions(mind);
	    }
	  }
	else if(within) {
	  body->Parent()->SendOut(stealth_t, stealth_s, 
		";s looks inside ;s.\n", "", body, (*targ_it));
	  if(mind) {
	    (*targ_it)->SendDesc(mind, body);
	    (*targ_it)->SendExtendedActions(mind);
	    (*targ_it)->SendContents(mind);
	    }
	  }
	else {
	  body->Parent()->SendOut(stealth_t, stealth_s, 
		";s looks at ;s.\n", "", body, (*targ_it));
	  if(mind) {
	    (*targ_it)->SendDesc(mind, body);
	    (*targ_it)->SendExtendedActions(mind);
	    }
	  }

	if(must_open) {
	  (*targ_it)->SetSkill("Open", 0);
	  body->Parent()->SendOut(stealth_t, stealth_s, 
		";s closes ;s.\n", "You close ;s.\n", body, (*targ_it));
	  }
	}
      }

    if(targs.size() == 1 && targs.front() != body->Parent()) {
      targs.front()->TryCombine();
      }
    return 0;
    }

  if(com == COM_SEARCH) {
    if(!body->Parent()) return 0;

    typeof(body->Contents()) targs;
    while((!isgraph(comline[len])) && (comline[len])) ++len;

    if(strlen(comline+len) > 0) {
      targs = body->PickObjects(comline+len,
	LOC_NEARBY|LOC_ADJACENT|LOC_SELF|LOC_INTERNAL);
      if(targs.size() == 0) {
	if(mind) mind->Send("You don't see that here.\n");
	return 0;
	}
      }
    else {
      targs.push_back(body->Parent());
      }

    stealth_t = 0;
    stealth_s = 0;
    if(body->Pos() == POS_USE && (!body->IsUsing("Perception"))) {
      body->Parent()->SendOut(stealth_t, stealth_s,
	";s stops %s.\n", "You stop %s.\n",
	body, NULL, body->UsingString()
	);
      }
    body->StartUsing("Perception");
    body->SetSkill("Hidden", 0);
    typeof(targs.begin()) targ_it;
    for(targ_it = targs.begin(); targ_it != targs.end(); ++targ_it) {
      string denied = "";
      for(Object *own = *targ_it; own; own = own->Parent()) {
	if(own->Attribute(1) && own != body && (!own->IsAct(ACT_SLEEP))
		&& (!own->IsAct(ACT_DEAD)) && (!own->IsAct(ACT_DYING))
		&& (!own->IsAct(ACT_UNCONSCIOUS))) {
	  denied = "You would need ";
	  denied += own->Name(1);
	  denied += "'s permission to search ";
	  denied += (*targ_it)->Name(0, NULL, own);
	  denied += ".\n";
	  }
	else if(own->Skill("Container") && (!own->Skill("Open"))
		&& own->Skill("Locked")) {
	  denied = own->Name(1);
	  if(own == *targ_it) {
	    denied += " is closed and locked so you can't search it.\n";
	    }
	  else {
	    denied += " is closed and locked so you can't get to ";
	    denied += (*targ_it)->Name(1);
	    denied += ".\n";
	    }
	  denied[0] = toupper(denied[0]);
	  }
	}
      if(denied.length() > 0) {
	if(mind) mind->Send(denied.c_str());
	continue;
	}

      body->Parent()->SendOut(stealth_t, stealth_s, 
	";s searches ;s.\n", "you search ;s.\n", body, *targ_it);

      typeof(body->Contents()) objs;
      objs = (*targ_it)->Contents();
      typeof(objs.begin()) obj_it;
      for(obj_it = objs.begin(); obj_it != objs.end(); ++obj_it) {
	if((*obj_it)->Skill("Hidden")) {
	  if(body->Roll("Perception", (*obj_it)->Skill("Hidden"))) {
	    (*obj_it)->SetSkill("Hidden", 0);
	    body->Parent()->SendOut(stealth_t, stealth_s, 
		";s reveals ;s.\n", "you reveal ;s.\n", body, *obj_it);
	    }
	  }
	}

      if(mind) {
	(*targ_it)->SendExtendedActions(mind, 1);
	(*targ_it)->SendContents(mind, body, 1);
	}
      }
    return 0;
    }

  if(com == COM_HIDE) {
    typeof(body->Contents()) targs;
    while((!isgraph(comline[len])) && (comline[len])) ++len;

    if(strlen(comline+len) <= 0) {
      targs.push_back(body);
      }
    else {
      targs = body->PickObjects(comline+len,
		LOC_INTERNAL|LOC_NEARBY|LOC_SELF);
      }
    if(targs.size() < 1) {
      if(mind) mind->Send("You don't see that here.\n");
      return 0;
      }

    typeof(targs.begin()) targ_it;
    for(targ_it = targs.begin(); targ_it != targs.end(); ++targ_it) {
      string denied = "";
      for(Object *own = *targ_it; own; own = own->Parent()) {
	if(own->Attribute(1) && own != body && (!own->IsAct(ACT_SLEEP))
		&& (!own->IsAct(ACT_DEAD)) && (!own->IsAct(ACT_DYING))
		&& (!own->IsAct(ACT_UNCONSCIOUS))) {
	  denied = "You would need ";
	  denied += own->Name(1);
	  denied += "'s permission to hide ";
	  denied += (*targ_it)->Name(0, NULL, own);
	  denied += ".\n";
	  }
	else if(own->Skill("Container") && (!own->Skill("Open"))
		&& own->Skill("Locked")) {
	  if(own != *targ_it) {
	    denied = own->Name(1);
	    denied += " is closed and locked so you can't get to ";
	    denied += (*targ_it)->Name(1);
	    denied += ".\n";
	    denied[0] = toupper(denied[0]);
	    }
	  }
	}
      if(denied.length() > 0) {
	if(mind) mind->Send(denied.c_str());
	continue;
	}

      body->Parent()->SendOut(stealth_t, stealth_s, 
	";s hides ;s.\n", "you hide ;s.\n", body, *targ_it);

      (*targ_it)->SetSkill("Hidden", body->Roll("Stealth", 2) * 2);
      }
    return 0;
    }

  if(com == COM_EXAMINE) {
    Object *targ = NULL;
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    if(strlen(comline+len) <= 0) {
      if(mind) mind->Send("You want to examine what?\n");
      return 0;
      }
    targ = body->PickObject(comline+len,
		LOC_INTERNAL|LOC_NEARBY|LOC_SELF);
    if(!targ) {
      if(mind) mind->Send("You don't see that here.\n");
      }
    else {
      body->Parent()->SendOut(stealth_t, stealth_s, 
	";s examines ;s.\n", "", body, targ);
      if(mind) targ->SendLongDesc(mind, body);
      }
    return 0;
    }

  if(com == COM_CONSIDER) {
    Object *targ = NULL;
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    if(strlen(comline+len) <= 0) {
      if(mind) mind->Send("You want to consider attacking what?\n");
      return 0;
      }
    targ = body->PickObject(comline+len,
		LOC_INTERNAL|LOC_NEARBY|LOC_SELF);
    if(!targ) {
      if(mind) mind->Send("You don't see that here.\n");
      }
    else {
      body->Parent()->SendOut(stealth_t, stealth_s, ";s considers attacking ;s.\n",
	"You consider attacking ;s.\n", body, targ);
      if(mind) {
	int diff;
	string mes = string(targ->Name()) + "...\n";
	mes[0] = toupper(mes[0]);
	mind->Send(mes.c_str());

	if((!targ->ActTarg(ACT_WIELD)) && (!body->ActTarg(ACT_WIELD))) {
	  mind->Send("   ...is unarmed, but so are you.\n");
	  }
	else if(!targ->ActTarg(ACT_WIELD)) {
	  mind->Send("   ...is unarmed.\n");
	  }
	else if(!body->ActTarg(ACT_WIELD)) {
	  mind->Send("   ...is armed, and you are not!\n");
	  }

	diff = 0;
	if(body->ActTarg(ACT_WIELD))
	  diff = (body->ActTarg(ACT_WIELD)->Skill("WeaponReach") > 9);
	if(targ->ActTarg(ACT_WIELD)
		&& targ->ActTarg(ACT_WIELD)->Skill("WeaponReach") > 9) {
	  if(diff) mind->Send("   ...has a ranged weapon, and so do you!\n");
	  else mind->Send("   ...has a ranged weapon!\n");
	  }
	else if(diff) {
	  mind->Send("   ...doesn't have a ranged weapon, and you do!\n");
	  }
	else {
	  diff = 0;
	  if(body->ActTarg(ACT_WIELD))
	    diff += body->ActTarg(ACT_WIELD)->Skill("WeaponReach");
	  if(targ->ActTarg(ACT_WIELD))
	    diff -= targ->ActTarg(ACT_WIELD)->Skill("WeaponReach");
	  if(diff < -5)      mind->Send("   ...outreaches you by a mile.\n");
	  else if(diff < -2) mind->Send("   ...has much greater reach than you.\n");
	  else if(diff < -1) mind->Send("   ...has greater reach than you.\n");
	  else if(diff < 0)  mind->Send("   ...has a bit greater reach than you.\n");
	  else if(diff > 5)  mind->Send("   ...has a mile less reach than you.\n");
	  else if(diff > 2)  mind->Send("   ...has much less reach than you.\n");
	  else if(diff > 1)  mind->Send("   ...has less reach than you.\n");
	  else if(diff > 0)  mind->Send("   ...has a bit less reach than you.\n");
	  else               mind->Send("   ...has about your reach.\n");
	  }

	if((!targ->ActTarg(ACT_WEAR_SHIELD)) && (!body->ActTarg(ACT_WEAR_SHIELD))) {
	  mind->Send("   ...has no shield, but neither do you.\n");
	  }
	else if(!targ->ActTarg(ACT_WEAR_SHIELD)) {
	  mind->Send("   ...has no shield.\n");
	  }
	else if(!body->ActTarg(ACT_WEAR_SHIELD)) {
	  mind->Send("   ...has a shield, and you do not!\n");
	  }

	diff = 0;
	string sk = "Punching";
	if(body->IsAct(ACT_WIELD)) {
	  sk = get_weapon_skill(body->ActTarg(ACT_WIELD)->Skill("WeaponType"));
	  }
	if(body->HasSkill(sk)) {
	  diff += body->Skill(sk);
	  }
	sk = "Punching";
	if(targ->IsAct(ACT_WIELD)) {
	  sk = get_weapon_skill(targ->ActTarg(ACT_WIELD)->Skill("WeaponType"));
	  }
	if(targ->HasSkill(sk)) {
	  diff -= targ->Skill(sk);
	  }
	if(diff < -5)      mind->Send("   ...is far more skilled than you.\n");
	else if(diff < -2) mind->Send("   ...is much more skilled than you.\n");
	else if(diff < -1) mind->Send("   ...is more skilled than you.\n");
	else if(diff < 0)  mind->Send("   ...is a bit more skilled than you.\n");
	else if(diff > 5)  mind->Send("   ...is far less skilled than you.\n");
	else if(diff > 2)  mind->Send("   ...is much less skilled than you.\n");
	else if(diff > 1)  mind->Send("   ...is less skilled than you.\n");
	else if(diff > 0)  mind->Send("   ...is a bit less skilled than you.\n");
	else               mind->Send("   ...is about as skilled as you.\n");

	diff = body->Attribute(0) - targ->Attribute(0);
	if(diff < -10)     mind->Send("   ...is titanic.\n");
	else if(diff < -5) mind->Send("   ...is gargantuan.\n");
	else if(diff < -2) mind->Send("   ...is much larger than you.\n");
	else if(diff < -1) mind->Send("   ...is larger than you.\n");
	else if(diff < 0)  mind->Send("   ...is a bit larger than you.\n");
	else if(diff > 10) mind->Send("   ...is an ant compared to you.\n");
	else if(diff > 5)  mind->Send("   ...is tiny compared to you.\n");
	else if(diff > 2)  mind->Send("   ...is much smaller than you.\n");
	else if(diff > 1)  mind->Send("   ...is smaller than you.\n");
	else if(diff > 0)  mind->Send("   ...is a bit smaller than you.\n");
	else               mind->Send("   ...is about your size.\n");

	diff = body->Attribute(1) - targ->Attribute(1);
	if(diff < -10)     mind->Send("   ...is a blur of speed.\n");
	else if(diff < -5) mind->Send("   ...is lightning fast.\n");
	else if(diff < -2) mind->Send("   ...is much faster than you.\n");
	else if(diff < -1) mind->Send("   ...is faster than you.\n");
	else if(diff < 0)  mind->Send("   ...is a bit faster than you.\n");
	else if(diff > 10) mind->Send("   ...is a turtle on valium.\n");
	else if(diff > 5)  mind->Send("   ...is slower than dial-up.\n");
	else if(diff > 2)  mind->Send("   ...is much slower than you.\n");
	else if(diff > 1)  mind->Send("   ...is slower than you.\n");
	else if(diff > 0)  mind->Send("   ...is a bit slower than you.\n");
	else               mind->Send("   ...is about your speed.\n");

	diff = body->Attribute(2) - targ->Attribute(2);
	if(diff < -10)     mind->Send("   ...is the strongest thing you've ever seen.\n");
	else if(diff < -5) mind->Send("   ...is super-strong.\n");
	else if(diff < -2) mind->Send("   ...is much stronger than you.\n");
	else if(diff < -1) mind->Send("   ...is stronger than you.\n");
	else if(diff < 0)  mind->Send("   ...is a bit stronger than you.\n");
	else if(diff > 10) mind->Send("   ...is a complete push-over.\n");
	else if(diff > 5)  mind->Send("   ...is a wuss compared to you.\n");
	else if(diff > 2)  mind->Send("   ...is much weaker than you.\n");
	else if(diff > 1)  mind->Send("   ...is weaker than you.\n");
	else if(diff > 0)  mind->Send("   ...is a bit weaker than you.\n");
	else               mind->Send("   ...is about your strength.\n");

	if((targ->Skill("CircleAction") & 4128) == 0) {
	  mind->Send("   ...does not seem threatening.\n");
	  }
	else if((targ->Skill("CircleAction") & 160) == 32) {
	  mind->Send("   ...is spoiling for a fight.\n");
	  }
	else if((targ->Skill("CircleAction") & 160) == 160) {
	  mind->Send("   ...seems to be trolling for victems.\n");
	  }
	else if(targ->Skill("CircleAction") & 4096) {
	  mind->Send("   ...seems to be on the look-out for trouble.\n");
	  }
	else {
	  mind->Send("   ...is impossible - tell the Ninjas[TM].\n");
	  }
	}
      }
    return 0;
    }

  if(com == COM_SCORE) {
    if(mind) {
      mind->Send("%s", CCYN);
      body->SendFullSituation(mind, body);
      body->SendActions(mind);
      mind->Send("%s", CNRM);
      body->SendScore(mind, body);
      }
    return 0;
    }

  if(com == COM_STATS) {
    Object *targ = NULL;
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    if((!body) && strlen(comline+len) <= 0) {
      targ = mind->Owner()->Creator();
      }
    else if(!body) {
      targ = mind->Owner()->Room()->PickObject(comline+len, LOC_INTERNAL);
      }
    else if(strlen(comline+len) <= 0) {
      targ = body;
      }
    else {
      targ = body->PickObject(comline+len,
		LOC_INTERNAL|LOC_NEARBY|LOC_SELF|LOC_HERE);
      }
    if(!targ) {
      if(mind) mind->Send("You don't see that here.\n");
      }
    else {
      if(mind) {
	mind->Send("%s", CCYN);
	targ->SendFullSituation(mind, body);
	targ->SendActions(mind);
	mind->Send("%s", CNRM);
	targ->SendScore(mind, body);
	targ->SendStats(mind, body);
	}
      }
    if(targ && body && targ != body && targ != body->Parent()) {
      targ->TryCombine();
      }
    return 0;
    }

  if(com == COM_LOCK) {
    Object *targ = NULL;
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    if(strlen(comline+len) <= 0) {
      if(mind) mind->Send("You want to lock what?\n");
      return 0;
      }
    targ = body->PickObject(comline+len, LOC_INTERNAL|LOC_NEARBY);
    if(!targ) {
      if(mind) mind->Send("You don't see that here.\n");
      }
    else if(targ->Skill("Locked")) {
      if(mind) mind->Send("It is already locked!\n");
      }
    else if(targ->Skill("Lock") <= 0 && (!nmode)) {
      if(mind) mind->Send("It does not seem to have a keyhole!\n");
      }
    else {
      if(!nmode) {
	typeof(body->Contents()) keys = body->PickObjects("all", LOC_INTERNAL);
	typeof(keys.begin()) key;
	for(key = keys.begin(); key != keys.end(); ++key) {
	  if((*key)->Skill("Key") == targ->Skill("Lock")) break;
	  }
	if(key == keys.end()) {
	  if(mind) mind->Send("You don't seem to have the right key.\n");
	  return 0;
	  }
	}
      targ->SetSkill("Locked", 1);
      body->Parent()->SendOut(stealth_t, stealth_s, ";s locks ;s.\n", "You lock ;s.\n", body, targ);
      if(targ->ActTarg(ACT_SPECIAL_MASTER)) {
	Object *targ2 = targ->ActTarg(ACT_SPECIAL_MASTER);
	targ2->Parent()->SendOut(stealth_t, stealth_s, ";s locks.\n", "", targ2, NULL);
	targ2->SetSkill("Locked", 1);
	}
      }
    return 0;
    }

  if(com == COM_UNLOCK) {
    Object *targ = NULL;
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    if(strlen(comline+len) <= 0) {
      if(mind) mind->Send("You want to unlock what?\n");
      return 0;
      }
    targ = body->PickObject(comline+len, LOC_INTERNAL|LOC_NEARBY);
    if(!targ) {
      if(mind) mind->Send("You don't see that here.\n");
      }
    else if(!targ->Skill("Locked")) {
      if(mind) mind->Send("It is not locked!\n");
      }
    else if(targ->Skill("Lock") <= 0 && (!nmode)) {
      if(mind) mind->Send("It does not seem to have a keyhole!\n");
      }
    else {
      if(!nmode) {
	typeof(body->Contents()) keys = body->PickObjects("all", LOC_INTERNAL);
	typeof(keys.begin()) key;
	for(key = keys.begin(); key != keys.end(); ++key) {
	  if((*key)->Skill("Key") == targ->Skill("Lock")) break;
	  }
	if(key == keys.end()) {
	  if(mind) mind->Send("You don't seem to have the right key.\n");
	  return 0;
	  }
	}
      targ->SetSkill("Locked", 0);
      body->Parent()->SendOut(stealth_t, stealth_s, ";s unlocks ;s.\n", "You unlock ;s.\n", body, targ);
      if(targ->ActTarg(ACT_SPECIAL_MASTER)) {
	Object *targ2 = targ->ActTarg(ACT_SPECIAL_MASTER);
	targ2->Parent()->SendOut(stealth_t, stealth_s, ";s unlocks.\n", "", targ2, NULL);
	targ2->SetSkill("Locked", 1);
	}
      }
    return 0;
    }

  if(com == COM_OPEN) {
    Object *targ = NULL;
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    if(strlen(comline+len) <= 0) {
      if(mind) mind->Send("You want to open what?\n");
      return 0;
      }
    targ = body->PickObject(comline+len, LOC_INTERNAL|LOC_NEARBY);
    if(!targ) {
      if(mind) mind->Send("You don't see that here.\n");
      }
    else if(!targ->Skill("Closeable")) {
      if(mind) mind->Send("That can't be opened or closed.\n");
      }
    else if(targ->Skill("Open")) {
      if(mind) mind->Send("It's already open!\n");
      }
    else if(targ->Skill("Locked")) {
      if(mind) mind->Send("It is locked!\n");
      }
    else {
      targ->SetSkill("Open", 1);
      body->Parent()->SendOut(stealth_t, stealth_s, ";s opens ;s.\n", "You open ;s.\n", body, targ);
      if(targ->ActTarg(ACT_SPECIAL_MASTER)) {
	Object *targ2 = targ->ActTarg(ACT_SPECIAL_MASTER);
	targ2->Parent()->SendOut(stealth_t, stealth_s, ";s opens.\n", "", targ2, NULL);
	targ2->SetSkill("Open", 1);
	targ2->SetSkill("Locked", 0);	//FIXME: Do I want to do this?
	}
      }
    return 0;
    }

  if(com == COM_CLOSE) {
    Object *targ = NULL;
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    if(strlen(comline+len) <= 0) {
      if(mind) mind->Send("You want to close what?\n");
      return 0;
      }
    targ = body->PickObject(comline+len, LOC_INTERNAL|LOC_NEARBY);
    if(!targ) {
      if(mind) mind->Send("You don't see that here.\n");
      }
    else if(!targ->Skill("Closeable")) {
      if(mind) mind->Send("That can't be opened or closed.\n");
      }
    else if(!targ->Skill("Open")) {
      if(mind) mind->Send("It's already closed!\n");
      }
    else if(targ->Skill("Locked")) {
      if(mind) mind->Send("It is locked!\n");
      }
    else {
      targ->SetSkill("Open", 0);
      body->Parent()->SendOut(stealth_t, stealth_s, 
	";s closes ;s.\n", "You close ;s.\n", body, targ);
      if(targ->ActTarg(ACT_SPECIAL_MASTER)) {
	Object *targ2 = targ->ActTarg(ACT_SPECIAL_MASTER);
	targ2->Parent()->SendOut(stealth_t, stealth_s, ";s closes.\n", "", targ2, NULL);
	targ2->SetSkill("Open", 0);
	targ2->SetSkill("Locked", 0);	//FIXME: Do I want to do this?
	}
      }
    return 0;
    }

  if(com == COM_LIST) {
    if(!mind) return 0;

    typeof(body->Parent()->Contents()) objs = body->Parent()->Contents();
    typeof(objs.begin()) shpkp_i;
    typeof(body->Parent()->Contents()) shpkps;
    string reason = "";
    for(shpkp_i = objs.begin(); shpkp_i != objs.end(); ++shpkp_i) {
      if((*shpkp_i)->Skill("Sell Profit")) {
	if((*shpkp_i)->IsAct(ACT_DEAD)) {
	  reason = "Sorry, the shopkeeper is dead!\n";
	  }
	else if((*shpkp_i)->IsAct(ACT_DYING)) {
	  reason = "Sorry, the shopkeeper is dying!\n";
	  }
	else if((*shpkp_i)->IsAct(ACT_UNCONSCIOUS)) {
	  reason = "Sorry, the shopkeeper is unconscious!\n";
	  }
	else if((*shpkp_i)->IsAct(ACT_SLEEP)) {
	  reason = "Sorry, the shopkeeper is asleep!\n";
	  }
	else {
	  shpkps.push_back(*shpkp_i);
	  }
	}
      }
    if(shpkps.size() == 0) {
      if(mind) {
	mind->Send("%s", reason.c_str());
	mind->Send("You can only do that around a shopkeeper.\n");
	}
      }
    else {
      Object *shpkp = shpkps.front();
      if(shpkp->ActTarg(ACT_WEAR_RSHOULDER)
		&& shpkp->ActTarg(ACT_WEAR_RSHOULDER)->Skill("Vortex")) {
	Object *vortex = shpkp->ActTarg(ACT_WEAR_RSHOULDER);
	objs = vortex->Contents();
	typeof(objs.begin()) obj;
	typeof(objs.begin()) oobj = objs.begin();
	for(obj = objs.begin(); obj != objs.end(); ++obj) {
	  if(obj != objs.begin() && (*(*obj)) == (*(*oobj))) continue;
	  int price = (*obj)->Value();
 	  price *= shpkp->Skill("Sell Profit");
	  price += 999;  price /= 1000;
	  mind->Send("%10d gp: %s\n", price, (*obj)->ShortDesc());
	  oobj = obj;
	  }
	}
      }
    return 0;
    }

  if(com == COM_BUY) {
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    if(!comline[len]) {
      if(mind) mind->Send("What do you want to buy?\n");
      return 0;
      }

    typeof(body->Parent()->Contents()) objs = body->Parent()->Contents();
    typeof(objs.begin()) shpkp_i;
    typeof(body->Parent()->Contents()) shpkps;
    string reason = "";
    for(shpkp_i = objs.begin(); shpkp_i != objs.end(); ++shpkp_i) {
      if((*shpkp_i)->Skill("Sell Profit")) {
	if((*shpkp_i)->IsAct(ACT_DEAD)) {
	  reason = "Sorry, the shopkeeper is dead!\n";
	  }
	else if((*shpkp_i)->IsAct(ACT_DYING)) {
	  reason = "Sorry, the shopkeeper is dying!\n";
	  }
	else if((*shpkp_i)->IsAct(ACT_UNCONSCIOUS)) {
	  reason = "Sorry, the shopkeeper is unconscious!\n";
	  }
	else if((*shpkp_i)->IsAct(ACT_SLEEP)) {
	  reason = "Sorry, the shopkeeper is asleep!\n";
	  }
	else {
	  shpkps.push_back(*shpkp_i);
	  }
	}
      }
    if(shpkps.size() == 0) {
      if(mind) {
	mind->Send("%s", reason.c_str());
	mind->Send("You can only do that around a shopkeeper.\n");
	}
      }
    else {
      Object *shpkp = shpkps.front();
      if(shpkp->ActTarg(ACT_WEAR_RSHOULDER)
		&& shpkp->ActTarg(ACT_WEAR_RSHOULDER)->Skill("Vortex")) {
	Object *vortex = shpkp->ActTarg(ACT_WEAR_RSHOULDER);

        typeof(vortex->Contents()) targs
		= vortex->PickObjects(comline+len, LOC_INTERNAL);
	if(!targs.size()) {
	  if(mind) mind->Send("The shopkeeper doesn't have that.\n");
	  return 0;
	  }

	typeof(targs.begin()) targ_i;
	for(targ_i = targs.begin(); targ_i != targs.end(); ++targ_i) {
	  Object *targ = (*targ_i);

	  int price = targ->Value() * MAX(1, targ->Skill("Quantity"));
	  if(price < 0) {
	    if(mind) mind->Send("You can't sell %s.\n", targ->Name(0, body));
	    continue;
	    }
	  else if(price == 0) {
	    if(mind) {
	      string mes = targ->Name(0, body);
	      mes += " is worthless.\n";
	      mes[0] = toupper(mes[0]);
	      mind->Send(mes.c_str());
	      }
	    continue;
	    }

 	  price *= shpkp->Skill("Sell Profit");
	  price += 999;  price /= 1000;
	  mind->Send("%d gp: %s\n", price, targ->ShortDesc());

	  int togo = price, ord = -price;
	  typeof(body->Contents()) pay
		= body->PickObjects("a gold piece", LOC_INTERNAL, &ord);
	  typeof(pay.begin()) coin;
	  for(coin = pay.begin(); coin != pay.end(); ++coin) {
	    togo -= MAX(1, (*coin)->Skill("Quantity"));
	    }

	  if(togo > 0) {
	    if(mind) mind->Send("You can't afford the %d gold (short %d).\n",
		price, togo);
	    }
	  else if(body->Stash(targ, 0)) {
	    body->Parent()->SendOut(stealth_t, stealth_s, 
		";s buys and stashes ;s.\n", "You buy and stash ;s.\n", body, targ);
	    for(coin = pay.begin(); coin != pay.end(); ++coin) {
	      shpkp->Stash(*coin);
	      }
	    }
	  else if(((!body->IsAct(ACT_HOLD)) 
		|| body->ActTarg(ACT_HOLD) == body->ActTarg(ACT_WIELD)
		|| body->ActTarg(ACT_HOLD) == body->ActTarg(ACT_WEAR_SHIELD)
		) && (!targ->Travel(body))) {
	    if(body->IsAct(ACT_HOLD)) {
	      body->Parent()->SendOut(stealth_t, stealth_s, ";s stops holding ;s.\n",
		"You stop holding ;s.\n", body, body->ActTarg(ACT_HOLD));
	      body->StopAct(ACT_HOLD);
	      }
	    body->AddAct(ACT_HOLD, targ);
	    body->Parent()->SendOut(stealth_t, stealth_s, 
		";s buys and holds ;s.\n", "You buy and hold ;s.\n", body, targ);
	    for(coin = pay.begin(); coin != pay.end(); ++coin) {
	      shpkp->Stash(*coin);
	      }
	    }
	  else {
	    if(mind) mind->Send("You can't stash or hold %s.\n", targ->Name(1));
	    }
	  }
	}
      }
    return 0;
    }

  if(com == COM_VALUE) {
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    if(!comline[len]) {
      if(mind) mind->Send("What do you want to value?\n");
      return 0;
      }

    typeof(body->Parent()->Contents()) objs = body->Parent()->Contents();
    typeof(objs.begin()) shpkp_i;
    typeof(body->Parent()->Contents()) shpkps;
    string reason = "";
    for(shpkp_i = objs.begin(); shpkp_i != objs.end(); ++shpkp_i) {
      if((*shpkp_i)->Skill("Sell Profit")) {
	if((*shpkp_i)->IsAct(ACT_DEAD)) {
	  reason = "Sorry, the shopkeeper is dead!\n";
	  }
	else if((*shpkp_i)->IsAct(ACT_DYING)) {
	  reason = "Sorry, the shopkeeper is dying!\n";
	  }
	else if((*shpkp_i)->IsAct(ACT_UNCONSCIOUS)) {
	  reason = "Sorry, the shopkeeper is unconscious!\n";
	  }
	else if((*shpkp_i)->IsAct(ACT_SLEEP)) {
	  reason = "Sorry, the shopkeeper is asleep!\n";
	  }
	else {
	  shpkps.push_back(*shpkp_i);
	  }
	}
      }
    if(shpkps.size() == 0) {
      if(mind) {
	mind->Send("%s", reason.c_str());
	mind->Send("You can only do that around a shopkeeper.\n");
	}
      }
    else {
      Object *shpkp = shpkps.front();
      typeof(body->Contents()) targs = body->PickObjects(comline+len, LOC_INTERNAL);
      if(!targs.size()) {
	if(mind) mind->Send("You want to value what?\n");
	}
      else if(shpkp->ActTarg(ACT_WEAR_RSHOULDER)
		&& shpkp->ActTarg(ACT_WEAR_RSHOULDER)->Skill("Vortex")) {
	typeof(targs.begin()) targ_i;
	for(targ_i = targs.begin(); targ_i != targs.end(); ++targ_i) {
	  Object *targ = (*targ_i);
	  int price = targ->Value() * MAX(1, targ->Skill("Quantity"));
	  if(price < 0) {
	    if(mind) mind->Send("You can't sell %s.\n", targ->Name(0, body));
	    continue;
	    }
	  else if(price == 0) {
	    if(mind) {
	      string mes = targ->Name(0, body);
	      mes += " is worthless.\n";
	      mes[0] = toupper(mes[0]);
	      mind->Send(mes.c_str());
	      }
	    continue;
	    }
 	  price *= shpkp->Skill("Buy Profit");
	  price += 0;  price /= 1000;
	  string mes = targ->Name(0, body);
	  mes[0] = toupper(mes[0]);
	  mind->Send(mes.c_str());
	  mind->Send(" is worth %d gp here.\n", price);
	  }
	}
      }
    return 0;
    }

  if(com == COM_SELL) {
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    if(!comline[len]) {
      if(mind) mind->Send("What do you want to sell?\n");
      return 0;
      }

    typeof(body->Parent()->Contents()) objs = body->Parent()->Contents();
    typeof(objs.begin()) shpkp_i;
    typeof(body->Parent()->Contents()) shpkps;
    string reason = "";
    for(shpkp_i = objs.begin(); shpkp_i != objs.end(); ++shpkp_i) {
      if((*shpkp_i)->Skill("Sell Profit")) {
	if((*shpkp_i)->IsAct(ACT_DEAD)) {
	  reason = "Sorry, the shopkeeper is dead!\n";
	  }
	else if((*shpkp_i)->IsAct(ACT_DYING)) {
	  reason = "Sorry, the shopkeeper is dying!\n";
	  }
	else if((*shpkp_i)->IsAct(ACT_UNCONSCIOUS)) {
	  reason = "Sorry, the shopkeeper is unconscious!\n";
	  }
	else if((*shpkp_i)->IsAct(ACT_SLEEP)) {
	  reason = "Sorry, the shopkeeper is asleep!\n";
	  }
	else {
	  shpkps.push_back(*shpkp_i);
	  }
	}
      }
    if(shpkps.size() == 0) {
      if(mind) {
	mind->Send("%s", reason.c_str());
	mind->Send("You can only do that around a shopkeeper.\n");
	}
      }
    else {
      Object *shpkp = shpkps.front();
      typeof(body->Contents()) targs =
		body->PickObjects(comline+len, LOC_NOTWORN|LOC_INTERNAL);
      if(body->ActTarg(ACT_HOLD)
		&& body->ActTarg(ACT_HOLD)->Parent() != body	//Dragging
		&& body->ActTarg(ACT_HOLD)->Matches(comline+len)) {
	targs.push_back(body->ActTarg(ACT_HOLD));
	}
      if(!targs.size()) {
	if(mind) mind->Send("You want to sell what?\n");
	}
      else if(shpkp->ActTarg(ACT_WEAR_RSHOULDER)
		&& shpkp->ActTarg(ACT_WEAR_RSHOULDER)->Skill("Vortex")) {
	Object *vortex = shpkp->ActTarg(ACT_WEAR_RSHOULDER);

	typeof(targs.begin()) targ_i;
	for(targ_i = targs.begin(); targ_i != targs.end(); ++targ_i) {
	  Object *targ = (*targ_i);

	  if(targ->Contents().size() > 0) {
	    if(mind) {
	      string mes = targ->Name(0, body);
	      mes += " is not empty.";
	      mes += "  You must empty it before you can sell it.\n";
	      mes[0] = toupper(mes[0]);
	      mind->Send(mes.c_str());
	      }
	    continue;
	    }

	  if(targ->Skill("Container") || targ->Skill("Liquid Container")) {
	    if(mind) {
	      string mes = targ->Name(0, body);
	      mes += " is a container.";
	      mes += "  You can't sell containers because Stea is a lazy bitch.\n";
	      mes[0] = toupper(mes[0]);
	      mind->Send(mes.c_str());
	      }
	    continue;
	    }

	  int price = targ->Value() * MAX(1, targ->Skill("Quantity"));
	  if(price < 0) {
	    if(mind) mind->Send("You can't sell %s.\n", targ->Name(0, body));
	    continue;
	    }
	  else if(price == 0) {
	    if(mind) {
	      string mes = targ->Name(0, body);
	      mes += " is worthless.\n";
	      mes[0] = toupper(mes[0]);
	      mind->Send(mes.c_str());
	      }
	    continue;
	    }

 	  price *= shpkp->Skill("Buy Profit");
	  price += 0;  price /= 1000;
	  mind->Send("%d gp: %s\n", price, targ->ShortDesc());

	  int togo = price, ord = -price;
	  typeof(body->Contents()) pay
		= shpkp->PickObjects("a gold piece", LOC_INTERNAL, &ord);
	  typeof(pay.begin()) coin;
	  for(coin = pay.begin(); coin != pay.end(); ++coin) {
	    togo -= MAX(1, (*coin)->Skill("Quantity"));
	    }

	  if(togo <= 0) {
	    body->Parent()->SendOut(stealth_t, stealth_s, 
		";s sells ;s.\n", "You sell ;s.\n", body, targ);
	    Object *payment = new Object;
	    for(coin = pay.begin(); coin != pay.end(); ++coin) {
	      (*coin)->Travel(payment);
	      }
	    if(body->Stash(payment->Contents().front())) {
	      targ->Travel(vortex);
	      }
	    else {
	      shpkp->Stash(payment->Contents().front());  //Keeper gets it back
	      if(mind) mind->Send("You couldn't stash %d gold!\n", price);
	      }
	    delete payment;
	    }
	  else {
	    if(mind) mind->Send("I can't afford the %d gold.\n", price);
	    }
	  }
	}
      }
    return 0;
    }

  if(com == COM_DRAG) {
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    if(!comline[len]) {
      if(mind) mind->Send("What do you want to drag?\n");
      return 0;
      }

    if(body->IsAct(ACT_HOLD)) {
      if(mind) mind->Send("You are already holding something.  Drop it first.\n");
      return 0;
      }

    Object *targ = body->PickObject(comline+len, LOC_NEARBY);
    if(!targ) {
      if(mind) mind->Send("You want to drag what?\n");
      return 0;
      }

    if(targ->Pos() == POS_NONE) {
      if(mind) mind->Send("You can't drag %s, it is fixed in place!\n",
		targ->Name());
      }
    else if(targ->Attribute(1)) {
      string denied = "You would need ";
      denied += targ->Name(1);
      denied += "'s permission to drag ";
      denied += targ->Name(0, NULL, targ);
      denied += ".\n";
      if(mind) mind->Send(denied.c_str(), targ->Name());
      }
    else if(targ->Weight() > body->Attribute(2) * 50000) {
      if(mind) mind->Send("You could never lift %s, it is too heavy.\n",
		targ->Name());
      }
    else {
      body->AddAct(ACT_HOLD, targ);
      body->Parent()->SendOut(stealth_t, stealth_s,
	";s starts dragging ;s.\n", "You start dragging ;s.\n",
	body, targ
	);
      }
    return 0;
    }

  if(com == COM_GET) {
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    if(!comline[len]) {
      if(mind) mind->Send("What do you want to get?\n");
      return 0;
      }

    typeof(body->Contents()) targs = body->PickObjects(comline+len, LOC_NEARBY);
    if(targs.size() == 0) {
      if(mind) mind->Send("You want to get what?\n");
      return 0;
      }

    typeof(targs.begin()) ind;
    for(ind = targs.begin(); ind != targs.end(); ++ind) {
      Object *targ = *ind;

      if(targ->Pos() == POS_NONE) {
	if(mind) mind->Send("You can't get %s, it is fixed in place!\n",
		targ->Name());
	}
      else if(targ->Attribute(1)) {
	if(mind) mind->Send("You can't get %s, it is not inanimate.\n",
		targ->Name());
	}
      else if(targ->Weight() > body->Attribute(2) * 50000) {
	if(mind) mind->Send("You could never lift %s, it is too heavy.\n",
		targ->Name());
	}
      else if(targ->Weight() > body->Attribute(2) * 10000) {
	if(mind) mind->Send(
		"You can't carry %s, it is too heavy.  Try 'drag' instead.\n",
		targ->Name());
	}
      else {
	string denied="";
	for(Object *owner = targ->Parent(); owner; owner = owner->Parent()) {
	  if(owner->Attribute(1) && owner != body && (!owner->IsAct(ACT_SLEEP))
		&& (!owner->IsAct(ACT_DEAD)) && (!owner->IsAct(ACT_DYING))
		&& (!owner->IsAct(ACT_UNCONSCIOUS))) {
	    denied = "You would need ";
	    denied += owner->Name(1);
	    denied += "'s permission to get ";
	    denied += targ->Name(0, NULL, owner);
	    denied += ".\n";
	    }
	  else if(owner->Skill("Container") && (!owner->Skill("Open"))
		&& owner->Skill("Locked")) {
	    denied = owner->Name(1);
	    denied += " is closed and locked so you can't get to ";
	    denied += targ->Name(1);
	    denied += ".\n";
	    denied[0] = toupper(denied[0]);
	    }
	  }

	if(denied != "") {
	  if(mind) mind->Send(denied.c_str());
	  }
	else if(body->Stash(targ)) {
	  body->Parent()->SendOut(stealth_t, stealth_s, ";s gets and stashes ;s.\n",
		"You get and stash ;s.\n", body, targ);
	  }
	else if(body->IsAct(ACT_HOLD)
		&& body->ActTarg(ACT_HOLD) != body->ActTarg(ACT_WEAR_SHIELD)
		&& body->ActTarg(ACT_HOLD) != body->ActTarg(ACT_WIELD)) {
	  if(mind) mind->Send("You have no place to stash %s.\n", targ->Name());
	  }
	else if(targ->Skill("Quantity") > 1) {
	  if(mind) mind->Send("You have no place to stash %s.\n", targ->Name());
	  }
	else {
	  if(body->IsAct(ACT_HOLD)) {
	    body->Parent()->SendOut(stealth_t, stealth_s,
		";s stops holding ;s.\n", "You stop holding ;s.\n",
		body, body->ActTarg(ACT_HOLD));
	    body->StopAct(ACT_HOLD);
	    }
	  targ->Travel(body);
	  body->AddAct(ACT_HOLD, targ);
	  body->Parent()->SendOut(stealth_t, stealth_s,
		";s gets and holds ;s.\n", "You get and hold ;s.\n",
		body, targ);
	  }
	}
      }
    return 0;
    }

  if(com == COM_PUT) {
    while((!isgraph(comline[len])) && (comline[len])) ++len;

    if(!strncasecmp(comline+len, "in ", 3)) len += 3;

    while((!isgraph(comline[len])) && (comline[len])) ++len;

    if(!body->IsAct(ACT_HOLD)) {
      if(mind) mind->Send("You must first 'hold' the object you want to 'put'.\n");
      return 0;
      }
    else if(!body->ActTarg(ACT_HOLD)) {
      if(mind) mind->Send("What!?!?!  You are holding nothing?\n");
      return 0;
      }

    if(!comline[len]) {
      if(mind) mind->Send("What do you want to put %s in?\n",
	body->ActTarg(ACT_HOLD)->Name(0, body));
      return 0;
      }

    Object *targ =
	body->PickObject(comline+len, LOC_NEARBY|LOC_INTERNAL);
    if(!targ) {
      if(mind) mind->Send("I don't see '%s' to put '%s' in!\n", comline+len,
	body->ActTarg(ACT_HOLD)->Name(0, body));
      }
    else if(targ->Attribute(1)) {
      if(mind) mind->Send("You can only put things in inanimate objects!\n");
      }
    else if(!targ->Skill("Container")) {
      if(mind) mind->Send("You can't put anything in that, it is not a container.\n");
      }
    else if(targ->Skill("Locked")) {
      if(mind) mind->Send("You can't put anything in that, it is locked.\n");
      }
    else if(targ == body->ActTarg(ACT_HOLD)) {
      if(mind) mind->Send("You can't put %s into itself.\n",
	body->ActTarg(ACT_HOLD)->Name(0, body));
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
	if(!targ->Skill("Open")) closed = 1;
	if(closed) body->Parent()->SendOut(stealth_t, stealth_s,  //FIXME: Really open/close stuff!
		";s opens ;s.\n", "You open ;s.\n", body, targ);
	string safety = obj->Name(0, body);
	body->Parent()->SendOut(stealth_t, stealth_s, 
		";s puts %s into ;s.\n", "You put %s into ;s.\n",
		body, targ, safety.c_str());
	if(closed) body->Parent()->SendOut(stealth_t, stealth_s, 
		";s close ;s.\n", "You close ;s.\n", body, targ);
	}
      }
    return 0;
    }

  if(com == COM_WIELD) {
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    if(!comline[len]) {
      if(body->IsAct(ACT_WIELD)) {
	Object *wield = body->ActTarg(ACT_WIELD);
	if(wield && body->Stash(wield)) {
	  body->Parent()->SendOut(stealth_t, stealth_s, ";s stops wielding and stash ;s.\n",
		"You stop wielding and stash ;s.\n", body, wield);
	  }
	else if(body->IsAct(ACT_HOLD)
		&& body->ActTarg(ACT_HOLD) != body->ActTarg(ACT_WEAR_SHIELD)
		&& body->ActTarg(ACT_HOLD) != wield) {
	  if(mind) mind->Send(
		"You are holding %s and can't stash %s.\n"
		"Perhaps you want to 'drop' one of these items?",
		body->ActTarg(ACT_HOLD)->Name(1, body), wield->Name(1, body));
	  }
	else {
	  body->AddAct(ACT_HOLD, wield);
	  body->StopAct(ACT_WIELD);
	  body->Parent()->SendOut(stealth_t, stealth_s, ";s stops wielding ;s.\n",
		"You stop wielding ;s.\n", body, wield);
	  }
	}
      else {
	if(mind) mind->Send("You are not wielding anything.\n");
	}      
      return 0;
      }

    Object *targ = body->PickObject(comline+len, LOC_INTERNAL);
    if(!targ) {
      if(mind) mind->Send("You want to wield what?\n");
      }
    else if(targ->Skill("WeaponType") <= 0) {
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
	|| body->ActTarg(ACT_WEAR_LHIP) == targ
	|| body->ActTarg(ACT_WEAR_RHIP) == targ
	) {
      if(mind) mind->Send("You are wearing that, perhaps you want to 'remove' it?\n");
      }
    else {
      if(body->IsAct(ACT_WIELD) && body->IsAct(ACT_HOLD)) {
	if(body->ActTarg(ACT_HOLD) != targ) {
	  if(mind) mind->Send("You are both holding and wielding other things.\n"
		"Perhaps you want to drop one of them?\n");
	  return 0;
	  }
	}
      targ->Travel(body, 0); // Kills Holds and Wields on "targ"
      if(body->IsAct(ACT_WIELD)) {
	Object *wield = body->ActTarg(ACT_WIELD);
	body->AddAct(ACT_HOLD, wield);
	body->StopAct(ACT_WIELD);
	body->Parent()->SendOut(stealth_t, stealth_s, ";s stops wielding ;s.\n",
		"You stop wielding ;s.\n", body, wield);
	}
      body->AddAct(ACT_WIELD, targ);
      body->Parent()->SendOut(stealth_t, stealth_s, 
	";s wields ;s.\n", "You wield ;s.\n", body, targ);
      }
    return 0;
    }

  if(com == COM_HOLD) {
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    if(!comline[len]) {
      if(mind) mind->Send("You want to hold what?\n");
      return 0;
      }

    Object *targ = body->PickObject(comline+len, LOC_INTERNAL);
    if(!targ) {
      if(mind) mind->Send("You want to hold what?\n");
      }
//FIXME - Implement Str-based Holding Capacity
//    else if(targ->Skill("WeaponType") <= 0) {
//      if(mind) mind->Send("You can't hold that - you are too weak!\n");
//      }
    else if(body->ActTarg(ACT_HOLD) == targ) {
      if(mind) mind->Send("You are already holding %s!\n",
	targ->Name(1, body));
      }
    else if(body->IsAct(ACT_HOLD)
	&& body->ActTarg(ACT_HOLD) != body->ActTarg(ACT_WIELD)
	&& body->ActTarg(ACT_HOLD) != body->ActTarg(ACT_WEAR_SHIELD)) {
      if(mind) mind->Send("You are already holding something!\n");
      }
    else if(body->ActTarg(ACT_WIELD) == targ
	&& two_handed(body->ActTarg(ACT_WIELD)->Skill("WeaponType"))
	) {
      body->AddAct(ACT_HOLD, targ);
      body->Parent()->SendOut(stealth_t, stealth_s, ";s puts a second hand on ;s.\n",
	"You put a second hand on ;s.\n", body, targ);
      }
    else if(body->ActTarg(ACT_WEAR_SHIELD) == targ) {
      body->AddAct(ACT_HOLD, targ);
      body->Parent()->SendOut(stealth_t, stealth_s, 
	";s holds ;s.\n", "You hold ;s.\n", body, targ);
      }
    else {
      if(body->IsAct(ACT_HOLD)) { //Means it's a shield/2-h weapon due to above.
	body->Parent()->SendOut(stealth_t, stealth_s, ";s stops holding ;s.\n",
	  "You stop holding ;s.\n", body, body->ActTarg(ACT_HOLD));
	body->StopAct(ACT_HOLD);
	}
      targ->Travel(body, 0); // Kills Holds, Wears and Wields on "targ"
      body->AddAct(ACT_HOLD, targ);
      body->Parent()->SendOut(stealth_t, stealth_s, 
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

    typeof(body->Contents()) targs = body->PickObjects(comline+len, LOC_INTERNAL);
    if(targs.size() == 0) {
      if(mind) mind->Send("You want to remove what?\n");
      return 0;
      }

    typeof(targs.begin()) targ_it;
    for(targ_it = targs.begin(); targ_it != targs.end(); ++targ_it) {
      Object *targ = (*targ_it);

      int removed = 0;
      for(act_t act = ACT_WEAR_BACK; act < ACT_MAX; act = act_t(int(act)+1)) {
	if(body->ActTarg(act) == targ) { removed = 1; break; }
	}
      if(!removed) {
        if(mind) mind->Send("You are not wearing %s!\n", targ->Name(0, body));
	}
      else if(body->Stash(targ, 0)) {
	body->Parent()->SendOut(stealth_t, stealth_s,
		";s removes and stashes ;s.\n", "You remove and stash ;s.\n",
		body, targ);
	}
      else if(body->IsAct(ACT_HOLD)
		&& body->ActTarg(ACT_HOLD) != body->ActTarg(ACT_WEAR_SHIELD)
		&& body->ActTarg(ACT_HOLD) != body->ActTarg(ACT_WIELD)) {
	if(mind) mind->Send(
		"You are already holding something else and can't stash %s.\n",
		targ->Name(0, body)
		);
	}
      else {
	if(body->IsAct(ACT_HOLD)) {
	  body->Parent()->SendOut(stealth_t, stealth_s, ";s stops holding ;s.\n",
		"You stop holding ;s.\n", body, body->ActTarg(ACT_HOLD));
	  body->StopAct(ACT_HOLD);
	  }
	targ->Travel(body, 0);
	body->AddAct(ACT_HOLD, targ);
	body->Parent()->SendOut(stealth_t, stealth_s, ";s removes and holds ;s.\n",
		"You remove and hold ;s.\n", body, targ);
	}
      }
    return 0;
    }

  if(com == COM_WEAR) {
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    if(!comline[len]) {
      if(mind) mind->Send("What do you want to wear?\n");
      return 0;
      }

    typeof(body->Contents()) targs
	= body->PickObjects(comline+len, LOC_NOTWORN|LOC_INTERNAL);
    if(!targs.size()) {
      if(mind) mind->Send("You want to wear what?\n");
      return 0;
      }

    int did_something = 0;
    typeof(targs.begin()) targ_it;
    for(targ_it = targs.begin(); targ_it != targs.end(); ++targ_it) {
      Object *targ = (*targ_it);

      //fprintf(stderr, "You try to wear %s!\n", targ->Name(0, body));
      //if(mind) mind->Send("You try to wear %s!\n", targ->Name(0, body));
      if(body->ActTarg(ACT_WEAR_BACK) == targ
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
		|| body->ActTarg(ACT_WEAR_LHIP) == targ
		|| body->ActTarg(ACT_WEAR_RHIP) == targ
		) {
	if(mind && targs.size() == 1)
	  mind->Send("You are already wearing %s!\n", targ->Name(0, body));
	}
      else {
	int success = 0;
	unsigned long mask = 1;
	while(!success) {
	  set<act_t> locations;

	  if(targ->Skill("Wearable on Back") & mask)
		locations.insert(ACT_WEAR_BACK);

	  if(targ->Skill("Wearable on Chest") & mask)
		locations.insert(ACT_WEAR_CHEST);

	  if(targ->Skill("Wearable on Head") & mask)
		locations.insert(ACT_WEAR_HEAD);

	  if(targ->Skill("Wearable on Neck") & mask)
		locations.insert(ACT_WEAR_NECK);

	  if(targ->Skill("Wearable on Waist") & mask)
		locations.insert(ACT_WEAR_WAIST);

	  if(targ->Skill("Wearable on Shield") & mask)
		locations.insert(ACT_WEAR_SHIELD);

	  if(targ->Skill("Wearable on Left Arm") & mask)
		locations.insert(ACT_WEAR_LARM);

	  if(targ->Skill("Wearable on Right Arm") & mask)
		locations.insert(ACT_WEAR_RARM);

	  if(targ->Skill("Wearable on Left Finger") & mask)
		locations.insert(ACT_WEAR_LFINGER);

	  if(targ->Skill("Wearable on Right Finger") & mask)
		locations.insert(ACT_WEAR_RFINGER);

	  if(targ->Skill("Wearable on Left Foot") & mask)
		locations.insert(ACT_WEAR_LFOOT);

	  if(targ->Skill("Wearable on Right Foot") & mask)
		locations.insert(ACT_WEAR_RFOOT);

	  if(targ->Skill("Wearable on Left Hand") & mask)
		locations.insert(ACT_WEAR_LHAND);

	  if(targ->Skill("Wearable on Right Hand") & mask)
		locations.insert(ACT_WEAR_RHAND);

	  if(targ->Skill("Wearable on Left Leg") & mask)
		locations.insert(ACT_WEAR_LLEG);

	  if(targ->Skill("Wearable on Right Leg") & mask)
		locations.insert(ACT_WEAR_RLEG);

	  if(targ->Skill("Wearable on Left Wrist") & mask)
		locations.insert(ACT_WEAR_LWRIST);

	  if(targ->Skill("Wearable on Right Wrist") & mask)
		locations.insert(ACT_WEAR_RWRIST);

	  if(targ->Skill("Wearable on Left Shoulder") & mask)
		locations.insert(ACT_WEAR_LSHOULDER);

	  if(targ->Skill("Wearable on Right Shoulder") & mask)
		locations.insert(ACT_WEAR_RSHOULDER);

	  if(targ->Skill("Wearable on Left Hip") & mask)
		locations.insert(ACT_WEAR_LHIP);

	  if(targ->Skill("Wearable on Right Hip") & mask)
		locations.insert(ACT_WEAR_RHIP);

	  if(locations.size() < 1) {
	    if(mask == 1) {
	      if(mind && targs.size() == 1)
		mind->Send("You can't wear %s - it's not wearable!\n",
							targ->Name(0, body));
	      }
	    else {
	      if(mind) mind->Send(
		"You can't wear %s with what you are already wearing!\n",
		targ->Name(0, body));
	      }
	    break;
	    }
	  success = 1;
	  mask *= 2;

	  if(targ->Skill("Quantity") > 1) {	//One at a time!
	    --targ_it;
	    targ = targ->Split(1);
	    }

	  set<act_t>::iterator loc;
	  for(loc = locations.begin(); loc != locations.end(); ++loc) {
	    if(body->IsAct(*loc)) { success = 0; break; }
	    }
	  if(success) {
	    did_something = 1;
	    targ->Travel(body, 0); // Kills Holds and Wields on "targ"
	    for(loc = locations.begin(); loc != locations.end(); ++loc) {
	      body->AddAct(*loc, targ);
	      }
	    body->Parent()->SendOut(stealth_t, stealth_s, 
		";s puts on ;s.\n", "You put on ;s.\n", body, targ);
	    }
	  }
	}
      }
    if(!did_something)
      if(mind) mind->Send("You don't seem to have anything (else) to wear.\n");
    return 0;
    }

  if(com == COM_EAT) {
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    if(!comline[len]) {
      if(mind) mind->Send("What do you want to eat?\n");
      return 0;
      }
    typeof(body->Contents()) targs
	= body->PickObjects(comline+len, LOC_NOTWORN|LOC_INTERNAL);
    if(body->ActTarg(ACT_HOLD)
	&& body->ActTarg(ACT_HOLD)->Parent() != body	//Dragging
	&& body->ActTarg(ACT_HOLD)->Matches(comline+len)) {
      targs.push_back(body->ActTarg(ACT_HOLD));
      }
    if(!targs.size()) {
      if(mind) mind->Send("You want to eat what?\n");
      }
    else {
      typeof(targs.begin()) targ;
      for(targ = targs.begin(); targ != targs.end(); ++targ) {
	if(!((*targ)->HasSkill("Ingestible"))) {
	  if(mind) mind->Send(
		"You don't want to eat %s.\n", (*targ)->Name(0, body)
		);
	  }
	else {
	  body->Parent()->SendOut(stealth_t, stealth_s, 
	  	";s eats ;s.\n", "You eat ;s.\n", body, *targ);

	  //Hunger/Thirst/Posion/Potion Effects
	  body->Consume(*targ);

	  delete (*targ);
	  }
	}
      }
    return 0;
    }

  if(com == COM_DROP) {
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    if(!comline[len]) {
      if(mind) mind->Send("What do you want to drop?\n");
      return 0;
      }
    typeof(body->Contents()) targs
	= body->PickObjects(comline+len, LOC_NOTWORN|LOC_INTERNAL);
    if(body->ActTarg(ACT_HOLD)
	&& body->ActTarg(ACT_HOLD)->Parent() != body	//Dragging
	&& body->ActTarg(ACT_HOLD)->Matches(comline+len)) {
      targs.push_back(body->ActTarg(ACT_HOLD));
      }
    if(!targs.size()) {
      if(mind) mind->Send("You want to drop what?\n");
      }
    else {
      typeof(targs.begin()) targ;
      for(targ = targs.begin(); targ != targs.end(); ++targ) {
	body->Parent()->SendOut(stealth_t, stealth_s, 
	  ";s drops ;s.\n", "You drop ;s.\n", body, *targ);
	(*targ)->Travel(body->Parent());
	}
      }
    return 0;
    }

  if(com == COM_DRINK) {
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    if(!comline[len]) {
      if(mind) mind->Send("What do you want to drink from?\n");
      return 0;
      }
    Object *targ = body->PickObject(comline+len, LOC_NOTWORN|LOC_INTERNAL);
    if((!targ) && body->ActTarg(ACT_HOLD)
	&& body->ActTarg(ACT_HOLD)->Parent() != body	//Dragging
	&& body->ActTarg(ACT_HOLD)->Matches(comline+len)) {
      targ = body->ActTarg(ACT_HOLD);
      }
    if(!targ) {
      targ = body->PickObject(comline+len, LOC_NEARBY);
      }
    if(!targ) {
      if(mind) mind->Send("You want to drink from what?\n");
      }
    else {
      string denied = "";
      for(Object *own = targ; own; own = own->Parent()) {
	if(own->Attribute(1) && own != body && (!own->IsAct(ACT_SLEEP))
		&& (!own->IsAct(ACT_DEAD)) && (!own->IsAct(ACT_DYING))
		&& (!own->IsAct(ACT_UNCONSCIOUS))) {
	  denied = "You would need ";
	  denied += own->Name(1);
	  denied += "'s permission to drink from ";
	  denied += targ->Name(0, NULL, own);
	  denied += ".\n";
	  }
	else if(own->Skill("Container") && (!own->Skill("Open"))
		&& own->Skill("Locked")) {
	  denied = own->Name(1);
	  if(own == targ) {
	    denied += " is closed and locked so you can't drink from it.\n";
	    }
	  else {
	    denied += " is closed and locked so you can't get to ";
	    denied += targ->Name(1);
	    denied += ".\n";
	    }
	  denied[0] = toupper(denied[0]);
	  }
	}
      if(denied.length() > 0) {
	if(mind) mind->Send(denied.c_str());
	return 0;
	}
      if(!(targ->HasSkill("Liquid Container"))) {
	if(mind) mind->Send(
		"%s is not a liquid container.  You can't drink from it.\n",
		targ->Name(0, body)
		);
	return 0;
	}
      if(targ->Contents().size() < 1) {
	if(mind) mind->Send("%s is empty.  There is nothing to drink\n",
		targ->Name(0, body));
	return 0;
	}
      Object *obj = targ->Contents().front();
      if(targ->HasSkill("Liquid Container") && obj->Skill("Quantity") < 2) {
	if(mind) mind->Send("%s is almost empty.  There is nothing to drink\n",
		targ->Name(0, body));
	return 0;
	}
      if((!(obj->HasSkill("Ingestible")))) {
	if(mind) mind->Send("You don't want to drink what's in %s.\n",
		targ->Name(0, body));
	return 0;
	}

      body->Parent()->SendOut(stealth_t, stealth_s, 
		";s drinks some liquid out of ;s.\n",
		"You drink some liquid out of ;s.\n",
		body, targ
		);

      //Hunger/Thirst/Posion/Potion Effects
      body->Consume(obj);

      if(obj->Skill("Quantity") < 2) {
	Object *nuke = targ->Contents().front();
	delete nuke;
	}
      else {
	obj->SetSkill("Quantity", obj->Skill("Quantity") - 1);
	}
      }
    return 0;
    }

  if(com == COM_DUMP) {
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    if(!comline[len]) {
      if(mind) mind->Send("What do you want to dump?\n");
      return 0;
      }
    typeof(body->Contents()) targs
	= body->PickObjects(comline+len, LOC_NOTWORN|LOC_INTERNAL);
    if(body->ActTarg(ACT_HOLD)
	&& body->ActTarg(ACT_HOLD)->Parent() != body	//Dragging
	&& body->ActTarg(ACT_HOLD)->Matches(comline+len)) {
      targs.push_back(body->ActTarg(ACT_HOLD));
      }
    if(!targs.size()) {
      if(mind) mind->Send("You want to dump what?\n");
      }
    else {
      typeof(targs.begin()) targ;
      for(targ = targs.begin(); targ != targs.end(); ++targ) {
	if(!((*targ)->HasSkill("Liquid Container"))) {
	  if(mind) mind->Send(
		"%s is not a liquid container.  It can't be dumped.\n",
		(*targ)->Name(0, body)
		);
	  continue;
	  }
	if((*targ)->Contents().size() < 1) {
	  if(mind) mind->Send("%s is empty.  There is nothing to dump\n",
		(*targ)->Name(0, body));
	  continue;
	  }
	body->Parent()->SendOut(stealth_t, stealth_s, 
		";s dumps all the liquid out of ;s.\n",
		"You dump all the liquid out of ;s.\n",
		body, *targ
		);
	while((*targ)->Contents().size() >= 1) {
	  Object *nuke = (*targ)->Contents().front();
	  delete nuke;
	  }
	}
      }
    return 0;
    }

  if(com == COM_FILL) {
    while((!isgraph(comline[len])) && (comline[len])) ++len;

    if(!strncasecmp(comline+len, "from ", 5)) len += 5;

    while((!isgraph(comline[len])) && (comline[len])) ++len;

    if(!body->IsAct(ACT_HOLD)) {
      if(mind) mind->Send("You must first 'hold' the object you want to 'fill'.\n");
      return 0;
      }
    else if(!body->ActTarg(ACT_HOLD)) {
      if(mind) mind->Send("What!?!?!  You are holding nothing?\n");
      return 0;
      }

    if(!comline[len]) {
      if(mind) mind->Send("Where do you want to fill %s from?\n",
	body->ActTarg(ACT_HOLD)->Name(0, body));
      return 0;
      }

    Object *src = body->PickObject(comline+len, LOC_NEARBY|LOC_INTERNAL);
    Object *dst = body->ActTarg(ACT_HOLD);
    if(!src) {
      if(mind) mind->Send("I don't see '%s' to fill %s from!\n",
	comline+len, dst->Name(0, body));
      }
    else if(!dst->HasSkill("Liquid Container")) {
      if(mind) mind->Send(
	"You can not fill %s, it is not a liquid container.\n",
	dst->Name(0, body));
      }
    else if(src->Attribute(1)) {
      if(mind) mind->Send(
	"You can only fill things from inanimate objects!\n");
      }
    else if(!src->HasSkill("Liquid Container")) {
      if(mind) mind->Send(
	"You can't fill anything from that, it's not a liquid container.\n");
      }
    else if(dst->Skill("Locked")) {
      if(mind) mind->Send(
	"You can't fill %s, it is locked.\n",
	dst->Name(0, body));
      }
    else if(src->Skill("Locked")) {
      if(mind) mind->Send(
	"You can't fill anything from that, it is locked.\n");
      }
    else if(src == dst) {
      if(mind) mind->Send(
	"You can't fill %s from itself.\n",
	dst->Name(0, body));
      }
    else if(src->Contents().size() < 1) {
      if(mind) mind->Send(
	"You can't fill anything from that, it is empty.\n");
      }
    else if(src->HasSkill("Liquid Source")
		&& src->Contents().front()->Skill("Quantity") < 2) {
      if(mind) mind->Send(
	"You can't fill anything from that, it is almost empty.\n");
      }
    else {
      int myclosed = 0, itclosed = 0;

      int sqty = 1;
      int dqty = dst->Skill("Capacity");
      if(src->Contents().front()->Skill("Quantity") > 0) {
	sqty = src->Contents().front()->Skill("Quantity");
	}

      if(src->HasSkill("Liquid Source")) {
	if(dqty > (sqty-1)) dqty = (sqty-1);
	}
      else {
	if(dqty > sqty) dqty = sqty;
	}
      sqty -= dqty;

      Object *liq;
      if(dst->Contents().size() > 0) {
	liq = dst->Contents().front();
	}
      else {
	liq = new Object(dst);
	}
      (*liq) = (*(src->Contents().front()));
      liq->SetSkill("Quantity", dqty);
      if(sqty > 0) {
	src->Contents().front()->SetSkill("Quantity", sqty);
	}
      else {
	delete(src->Contents().front());
	}

      if(!src->Skill("Open")) itclosed = 1;
      if(!dst->Skill("Open")) myclosed = 1;

	//FIXME: Really open/close stuff!
      if(itclosed) body->Parent()->SendOut(stealth_t, stealth_s,
	";s opens ;s.\n", "You open ;s.\n", body, src);
      if(myclosed) body->Parent()->SendOut(stealth_t, stealth_s,
	";s opens ;s.\n", "You open ;s.\n", body, dst);

      string safety = dst->Name(0, body);
      body->Parent()->SendOut(stealth_t, stealth_s, 
	";s dumps out and fills %s from ;s.\n",
	"You dump out and fill %s from ;s.\n",
	body, src, safety.c_str());

	//FIXME: Really open/close stuff!
      if(myclosed) body->Parent()->SendOut(stealth_t, stealth_s, 
	";s close ;s.\n", "You close ;s.\n", body, dst);
      if(itclosed) body->Parent()->SendOut(stealth_t, stealth_s, 
	";s close ;s.\n", "You close ;s.\n", body, src);
      }
    return 0;
    }

  if(com == COM_LEAVE) {
    Object *oldp = body->Parent();
    if(!body->Parent()->Parent()) {
      if(mind) mind->Send("It is not possible to leave this object!\n");
      }
    else if((!body->Parent()->Skill("Enterable")) && (!ninja)) {
      if(mind) mind->Send("It is not possible to leave this object!\n");
      }
    else if((!body->Parent()->Skill("Enterable")) && (!nmode)) {
      if(mind) mind->Send("You need to be in ninja mode to leave this object!\n");
      }
    else {
      if(nmode) {
	//Ninja-movement can't be followed!
	if(body->Parent()) body->Parent()->NotifyGone(body);
	}
      body->Travel(body->Parent()->Parent());
      if(oldp) oldp->SendOut(stealth_t, stealth_s, ";s leaves.\n", "", body, NULL);
      body->Parent()->SendDescSurround(body, body);
      body->Parent()->SendOut(stealth_t, stealth_s, 
	";s arrives.\n", "", body, NULL);
      }
    return 0;
    }

  if(com == COM_SLEEP) {
    if(body->IsAct(ACT_SLEEP)) {
      if(mind) mind->Send("You are already sleeping!\n");
      return 0;
      }
    int lied = 0;
    if(body->Pos() != POS_LIE) {
      body->SetPos(POS_LIE);
      lied = 1;
      }
    if(body->ActTarg(ACT_WIELD)) {
      Object *item = body->ActTarg(ACT_WIELD);
      if(body->Stash(item)) {		//Can stash you weapon
	body->Parent()->SendOut(stealth_t, stealth_s, 
		";s stashes ;s.\n", "You stash ;s.\n", body, item);
	}
      else {				//Can't stash your weapon
	item->Travel(body->Parent());
	body->Parent()->SendOut(stealth_t, stealth_s, 
		";s drops ;s.\n", "You drop ;s.\n", body, item);
	}
      }
    if(body->ActTarg(ACT_HOLD)		//Shield held & worn
		&& body->ActTarg(ACT_HOLD) == body->ActTarg(ACT_WEAR_SHIELD)) {
      body->Parent()->SendOut(stealth_t, stealth_s, ";s stops holding ;s.\n",
		"You stop holding ;s.\n", body, body->ActTarg(ACT_HOLD));
      body->StopAct(ACT_HOLD);
      }
    else if(body->ActTarg(ACT_HOLD)		//Dragging an item
		&& body->ActTarg(ACT_HOLD)->Parent() != body) {
      Object *item = body->ActTarg(ACT_HOLD);
      item->Travel(body->Parent());
      body->Parent()->SendOut(stealth_t, stealth_s, 
		";s drops ;s.\n", "You drop ;s.\n", body, item);
      }
    else if(body->ActTarg(ACT_HOLD)) {	//Regular held item
      Object *item = body->ActTarg(ACT_HOLD);
      if(body->Stash(item)) {		//Successfully stashed
	body->Parent()->SendOut(stealth_t, stealth_s, 
		";s stashes ;s.\n", "You stash ;s.\n", body, item);
	}
      else {				//Can't be stashed
	item->Travel(body->Parent());
	body->Parent()->SendOut(stealth_t, stealth_s, 
		";s drops ;s.\n", "You drop ;s.\n", body, item);
	}
      }
    body->Collapse();
    body->AddAct(ACT_SLEEP);
    if(lied) {
      body->Parent()->SendOut(stealth_t, stealth_s, 
	";s lies down and goes to sleep.\n", "You lie down and go to sleep.\n",
	body, NULL
	);
      }
    else {
      body->Parent()->SendOut(stealth_t, stealth_s, 
	";s goes to sleep.\n", "You go to sleep.\n",
	body, NULL
	);
      }
    return 0;
    }

  if(com == COM_WAKE) {
    if(!body->IsAct(ACT_SLEEP)) {
      if(mind) mind->Send("But you aren't asleep!\n");
      }
    else {
      body->StopAct(ACT_SLEEP);
      body->Parent()->SendOut(stealth_t, stealth_s, 
	";s wakes up.\n", "You wake up.\n", body, NULL);
      }
    return 0;
    }

  if(com == COM_REST) {
    if(body->IsAct(ACT_REST)) {
      body->StopAct(ACT_REST);
      body->Parent()->SendOut(stealth_t, stealth_s, 
	";s stops resting.\n", "You stop resting.\n", body, NULL);
      return 0;
      }
    else if(body->IsAct(ACT_SLEEP)) {
      body->StopAct(ACT_REST);
      body->AddAct(ACT_REST);
      body->Parent()->SendOut(stealth_t, stealth_s, 
	";s wakes up and starts resting.\n", "You wake up and start resting.\n",
	body, NULL);
      }
    else if(body->Pos() == POS_LIE || body->Pos() == POS_SIT) {
      body->AddAct(ACT_REST);
      body->Parent()->SendOut(stealth_t, stealth_s, 
	";s starts resting.\n", "You start resting.\n", body, NULL);
      }
    else {
      body->AddAct(ACT_REST);
      if(body->Pos() != POS_LIE) body->SetPos(POS_SIT);
      body->Parent()->SendOut(stealth_t, stealth_s, 
	";s sits down and rests.\n", "You sit down and rest.\n",
	body, NULL);
      }
    if(body->IsAct(ACT_FOLLOW)) {
      if(body->ActTarg(ACT_FOLLOW) && mind)
	body->Parent()->SendOut(stealth_t, stealth_s, 
		";s stop following ;s.\n", "You stop following ;s.\n",
		body, body->ActTarg(ACT_FOLLOW));
      body->StopAct(ACT_FOLLOW);
      }
    return 0;
    }

  if(com == COM_STAND) {
    if(body->Pos() == POS_STAND || body->Pos() == POS_USE) {
      if(mind) mind->Send("But you are already standing!\n");
      }
    else if(body->IsAct(ACT_SLEEP)) {
      body->SetPos(POS_STAND);
      body->StopAct(ACT_SLEEP);
      body->Parent()->SendOut(stealth_t, stealth_s, 
	";s wakes up and stands.\n", "You wake up and stand.\n",
	body, NULL);
      }
    else if(body->IsAct(ACT_REST)) {
      body->StopAct(ACT_REST);
      body->SetPos(POS_STAND);
      body->Parent()->SendOut(stealth_t, stealth_s, 
	";s stops resting and stands up.\n", "You stop resting and stand up.\n",
	body, NULL);
      }
    else {
      body->SetPos(POS_STAND);
      body->Parent()->SendOut(stealth_t, stealth_s, 
	";s stands up.\n", "You stand up.\n", body, NULL);
      }
    return 0;
    }

  if(com == COM_SIT) {
    if(body->Pos() == POS_SIT) {
      if(mind) mind->Send("But you are already sitting!\n");
      }
    else if(body->IsAct(ACT_SLEEP)) {
      body->StopAct(ACT_SLEEP);
      body->SetPos(POS_SIT);
      body->Parent()->SendOut(stealth_t, stealth_s, 
	";s awaken and sit up.\n", "You awaken and sit up.\n",
	body, NULL);
      }
    else if(body->Pos() == POS_LIE) {
      body->SetPos(POS_SIT);
      body->Parent()->SendOut(stealth_t, stealth_s, 
	";s sits up.\n", "You sit up.\n", body, NULL);
      }
    else {
      body->SetPos(POS_SIT);
      body->Parent()->SendOut(stealth_t, stealth_s, 
	";s sits down.\n", "You sit down.\n", body, NULL);
      }
    if(body->IsAct(ACT_FOLLOW)) {
      if(body->ActTarg(ACT_FOLLOW) && mind)
	body->Parent()->SendOut(stealth_t, stealth_s, 
		";s stop following ;s.\n", "You stop following ;s.\n",
		body, body->ActTarg(ACT_FOLLOW));
      body->StopAct(ACT_FOLLOW);
      }
    return 0;
    }

  if(com == COM_LIE) {
    if(body->Pos() == POS_LIE) {
      if(mind) mind->Send("But you are already lying down!\n");
      }
    else {
      body->SetPos(POS_LIE);
      body->Parent()->SendOut(stealth_t, stealth_s, 
	";s lies down.\n", "You lie down.\n", body, NULL);
      }
    if(body->IsAct(ACT_FOLLOW)) {
      if(body->ActTarg(ACT_FOLLOW) && mind)
	body->Parent()->SendOut(stealth_t, stealth_s, 
		";s stop following ;s.\n", "You stop following ;s.\n",
		body, body->ActTarg(ACT_FOLLOW));
      body->StopAct(ACT_FOLLOW);
      }
    return 0;
    }

  if(com == COM_STOP) {		//Alias "stop" to "use"
    com = COM_USE;
    comline = "";
    len = 0;
    }
  if(com == COM_USE) {
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    if(strlen(comline+len) == 0) {
      if(body->Pos() != POS_USE) {
	mind->Send("You're not using a skill.  Try 'use <skillname>' to start.\n");
	}
      else {
	body->Parent()->SendOut(stealth_t, stealth_s, 
		";s stops %s.\n", "You stop %s.\n",
		body, NULL, body->UsingString()
		);
	body->SetPos(POS_STAND);
	return 2;
	}
      return 0;
      }

    int longterm = 0;	//Long-running skills for results
    string skill = get_skill(comline+len);
    if(skill == "") {
      mind->Send("Don't know what skill you're trying to use.\n");
      return 0;
      }

    if(skill == "Lumberjack") {
      if(!body->HasSkill(skill)) {
	mind->Send("%sYou don't know how to do that.%s\n", CYEL, CNRM);
	return 0;
	}
      if(!body->Parent()) {		//You're nowhere?!?
	mind->Send("%sThere are no trees here.%s\n", CYEL, CNRM);
	return 0;
	}
      if(strcasestr(body->Parent()->Name(), "forest")
		&& body->Parent()->HasSkill("CircleZone")
		&& (!body->Parent()->HasSkill("Mature Trees"))) {
	body->Parent()->SetSkill("Mature Trees", 100);
	body->Parent()->Activate();
	}
      if(!body->Parent()->HasSkill("Mature Trees")) {
	mind->Send("%sThere are no trees here.%s\n", CYEL, CNRM);
	return 0;
	}
      else if(body->Parent()->Skill("Mature Trees") < 10) {
	mind->Send("%sThere are too few trees to harvest here.%s\n", CYEL, CNRM);
	return 0;
	}
      else {
	longterm = 3000;	//FIXME: Temporary - should take longer!
	}
      if(body->IsUsing("Lumberjack")) {		//Already been doing it
	if(body->Roll(skill, 10) > 0) {		//Succeeded!
	  body->Parent()->SendOut(ALL, 0,
		";s shouts 'TIMBER'!!!\n", "You shout 'TIMBER'!!!\n",
		body, body);
	  body->Parent()->Loud(body->Skill("Strength"),
		"someone shout 'TIMBER'!!!");
	  body->Parent()->SetSkill("Mature Trees",
		body->Parent()->Skill("Mature Trees") - 1);
	  body->SetSkill("Hidden", 0);

	  Object * log = new Object(body->Parent());
	  log->SetShortDesc("a log");
	  log->SetDesc("a fallen tree.");
	  log->SetLongDesc("This is a tree that has recently been cut down.");
	  log->SetPos(POS_LIE);
	  log->SetValue(10);
	  log->SetVolume(1000);
	  log->SetWeight(220000);
	  log->SetSize(8000);
	  log->SetSkill("Made of Wood", 200000);
	  }
	body->Parent()->SendOut(ALL, 0,
		";s continues chopping down trees.\n",
		"You continue chopping down trees.\n",
		body, body);
	body->Parent()->Loud(body->Skill("Strength") / 2,
		"loud chopping sounds.");
	body->SetSkill("Hidden", 0);
	}
      }

    if(!body->IsUsing(skill)) {		//Only if really STARTING to use skill.
      body->StartUsing(skill);

      //In case Stealth was started, re-calc (to hide going into stealth).
      stealth_t = 0;
      stealth_s = 0;
      if(body->IsUsing("Stealth") && body->Skill("Stealth") > 0) {
	stealth_t = body->Skill("Stealth");
	stealth_s = body->Roll("Stealth", 2);
	}

      if(body->Pos() != POS_STAND && body->Pos() != POS_USE) {	//FIXME: Unused
	body->Parent()->SendOut(stealth_t, stealth_s, 
		";s stands and starts %s.\n", "You stand up and start %s.\n",
		body, NULL, body->UsingString()
		);
	}
      else {
	body->Parent()->SendOut(stealth_t, stealth_s, 
		";s starts %s.\n", "You start %s.\n",
		body, NULL, body->UsingString()
		);
	}
      if(!body->HasSkill(skill)) {
	mind->Send(
		"%s...you don't have the '%s' skill, so you're bad at this.%s\n",
		CYEL, skill.c_str(), CNRM
		);
	}
      }
    else if(longterm == 0) {
      mind->Send("You are already using %s\n", skill.c_str());
      }

    if(longterm > 0) {	//Long-running skills for results
      body->BusyFor(longterm, "use Lumberjack");	//FIXME: Temporary!
      return 2;		//Full-round (and more) action!
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
	body->Parent()->SendOut(stealth_t, stealth_s, 
		";s starts pointing at ;s.\n", "You start pointing at ;s.\n",
		body, targ);
	}
      }
    else if(body->IsAct(ACT_POINT)) {
      Object *targ = body->ActTarg(ACT_POINT);
      body->StopAct(ACT_POINT);
      body->Parent()->SendOut(stealth_t, stealth_s, 
	";s stops pointing at ;s.\n", "You stop pointing at ;s.\n",
	body, targ);
      }
    else {
      if(mind) mind->Send("But, you aren't pointing at anyting!\n");
      }
    return 0;
    }

  if(com == COM_FOLLOW) {
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    if(strlen(comline+len) > 0) {
      Object *targ = body->PickObject(comline+len, LOC_NEARBY|LOC_SELF);
      if(!targ) {
	if(mind) mind->Send("You don't see that here.\n");
	}
      else {
	body->AddAct(ACT_FOLLOW, targ);
	body->Parent()->SendOut(stealth_t, stealth_s, 
		";s starts following ;s.\n", "You start following ;s.\n",
		body, targ);
	}
      }
    else if(body->IsAct(ACT_FOLLOW)) {
      Object *targ = body->ActTarg(ACT_FOLLOW);
      body->StopAct(ACT_FOLLOW);
      body->Parent()->SendOut(stealth_t, stealth_s, 
	";s stops following ;s.\n", "You stop following ;s.\n",
	body, targ);
      }
    else {
      if(mind) mind->Send("But, you aren't following anyone!\n");
      }
    return 0;
    }

  if(com == COM_ATTACK || com == COM_KILL || com == COM_PUNCH || com == COM_KICK) {
    //fprintf(stderr, "Handling attack command from %p of '%s'\n", mind, comline);

    int attacknow = 1;
    if(!body->IsAct(ACT_FIGHT)) attacknow = 0;

    Object *targ = NULL;
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    if(strlen(comline+len) > 0) {
      targ = body->PickObject(comline+len, LOC_NEARBY|LOC_CONSCIOUS);
      if(!targ) targ = body->PickObject(comline+len, LOC_NEARBY|LOC_ALIVE);
      if(!targ) targ = body->PickObject(comline+len, LOC_NEARBY);
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

    if(com == COM_ATTACK && (targ->Attribute(1) <= 0
	|| targ->IsAct(ACT_DEAD) || targ->IsAct(ACT_DYING)
	|| targ->IsAct(ACT_UNCONSCIOUS))) {
      if(mind) mind->Send("No need, target is down!\n");
      body->StopAct(ACT_FIGHT);
      return 0;
      }

    body->BusyFor(3000); //Overridden below if is alive/animate

    if(!(targ->Attribute(1) <= 0 || targ->IsAct(ACT_DEAD)
	|| targ->IsAct(ACT_DYING) || targ->IsAct(ACT_UNCONSCIOUS))) {
      body->AddAct(ACT_FIGHT, targ);
      body->BusyFor(3000, body->Tactics().c_str());
      if(!targ->IsAct(ACT_FIGHT)) {
	targ->BusyFor(3000, body->Tactics().c_str());
	targ->AddAct(ACT_FIGHT, body);
	}
      else if(targ->StillBusy()) {
	body->BusyWith(targ, body->Tactics().c_str());
	}
      }
    else {
      attacknow = 1; //Uncontested.
      }

    if(body->ActTarg(ACT_HOLD)		//FIXME: Don't drop offhand weapons?!?
	&& body->ActTarg(ACT_HOLD) != body->ActTarg(ACT_WEAR_SHIELD)
	&& body->ActTarg(ACT_HOLD) != body->ActTarg(ACT_WIELD)) {
      Object *targ = body->ActTarg(ACT_HOLD);
      body->Parent()->SendOut(stealth_t, stealth_s, 
	  ";s drops ;s.\n", "You drop ;s.\n", body, targ);
      targ->Travel(body->Parent());
      }

    if(body->ActTarg(ACT_WIELD)
	&& body->ActTarg(ACT_HOLD) != body->ActTarg(ACT_WIELD)
	&& two_handed(body->ActTarg(ACT_WIELD)->Skill("WeaponType"))
	) {
      if(body->ActTarg(ACT_HOLD)) {
	if(body->ActTarg(ACT_HOLD) == body->ActTarg(ACT_WEAR_SHIELD)) {
	  body->Parent()->SendOut(stealth_t, stealth_s, ";s stops holding ;s.\n",
		"You stop holding ;s.\n", body, body->ActTarg(ACT_HOLD));
	  body->StopAct(ACT_HOLD);
	  }
	else {
	  body->Parent()->SendOut(stealth_t, stealth_s, 
		";s drops ;s.\n", "You drop ;s.\n", body, body->ActTarg(ACT_HOLD));
	  body->ActTarg(ACT_HOLD)->Travel(body->Parent());
	  }
	}
      Object *targ = body->ActTarg(ACT_WIELD);
      body->AddAct(ACT_HOLD, targ);
      body->Parent()->SendOut(stealth_t, stealth_s, ";s holds ;s.\n", "You hold ;s.\n", body, targ);
      }

    if(body->ActTarg(ACT_WEAR_SHIELD) && (!body->IsAct(ACT_HOLD))) {
      Object *targ = body->ActTarg(ACT_WEAR_SHIELD);
      if(body->Roll("Shields", 4) > 0) {
	body->AddAct(ACT_HOLD, targ);
	body->Parent()->SendOut(stealth_t, stealth_s, ";s holds ;s.\n", "You hold ;s.\n", body, targ);
	}
      else {
	body->Parent()->SendOut(stealth_t, stealth_s, 
	  ";s fubmles with ;s.\n", "You fumble with ;s.\n", body, targ);
	}
      }

    if(!attacknow) {
      body->Parent()->SendOut(stealth_t, stealth_s, 
		";s moves to attack ;s.\n",
		"You move to attack ;s.\n",
		body, targ);
      body->BusyWith(body, comline); //HACK!  Make this command used first rnd!
      return 2; //No more actions until next round!
      }

    // Attacking, or being attacked removes hidden-ness.
    body->SetSkill("Hidden", 0);
    targ->SetSkill("Hidden", 0);

//    int succ = roll(body->att[1], targ->att[1]);
    int succ; string res;  //FIXME: res is ONLY for debugging!

    int reachmod = 0;
    string sk1 = "Punching", sk2 = "Punching";
    if(com == COM_KICK) { sk1 = "Kicking"; sk2 = "Kicking"; }
    else {
      if(body->IsAct(ACT_WIELD)) {
        sk1 = get_weapon_skill(body->ActTarg(ACT_WIELD)
		->Skill("WeaponType"));
	reachmod += MAX(0, body->ActTarg(ACT_WIELD)
		->Skill("WeaponReach"));
	if(reachmod > 9) reachmod = 0;
	}
      if(body->ActTarg(ACT_HOLD) == body->ActTarg(ACT_WEAR_SHIELD)
		&& body->ActTarg(ACT_HOLD)) {
	sk2 = "";
	}
      if(targ->ActTarg(ACT_HOLD) == targ->ActTarg(ACT_WEAR_SHIELD)
		&& targ->ActTarg(ACT_HOLD)) {
        sk2 = "Shields";
	reachmod = 0;
	}
      else if(targ->ActTarg(ACT_WIELD)) {
	if(sk2 == "Punching") sk2 = get_weapon_skill(targ->ActTarg(ACT_WIELD)
		->Skill("WeaponType"));
	reachmod -= MAX(0, targ->ActTarg(ACT_WIELD)
		->Skill("WeaponReach"));
	if(reachmod < -9) reachmod = 0;
	}
      }

    succ = body->Roll(sk1, targ, sk2, reachmod, &res);

    int loc = rand()%100;
    act_t loca = ACT_WEAR_CHEST;
    string locm = "";
    int stage = 0;
    if(loc < 50) {
      loca = ACT_WEAR_CHEST;
      locm = " in the chest";
      }
    else if(loc < 57) {
      loca = ACT_WEAR_BACK;
      locm = " in the back";
      }
    else if(loc < 59) {
      loca = ACT_WEAR_HEAD;
      locm = " in the head";
      stage = 1;
      }
    else if(loc < 60) {
      loca = ACT_WEAR_NECK;
      locm = " in the neck";
      stage = 2;
      }
    else if(loc < 70) {
      loca = ACT_WEAR_LARM;
      locm = " in the left arm";
      stage = -1;
      }
    else if(loc < 80) {
      loca = ACT_WEAR_RARM;
      locm = " in the right arm";
      stage = -1;
      }
    else if(loc < 85) {
      loca = ACT_WEAR_LLEG;
      locm = " in the left leg";
      stage = -1;
      }
    else if(loc < 90) {
      loca = ACT_WEAR_RLEG;
      locm = " in the right leg";
      stage = -1;
      }
    else if(loc < 93) {
      loca = ACT_WEAR_LHAND;
      locm = " in the left hand";
      stage = -2;
      }
    else if(loc < 96) {
      loca = ACT_WEAR_RHAND;
      locm = " in the right hand";
      stage = -2;
      }
    else if(loc < 98) {
      loca = ACT_WEAR_LFOOT;
      locm = " in the left foot";
      stage = -2;
      }
    else {
      loca = ACT_WEAR_RFOOT;
      locm = " in the right foot";
      stage = -2;
      }

    if(succ > 0) {
      //FIXME: Remove debugging stuff ("succ" and "res") from these messages.
      if(com == COM_KICK) {		//Kicking Action
	body->Parent()->SendOut(ALL, -1,
		"*;s kicks ;s%s. [%d] %s\n", "*You kick ;s%s. [%d] %s\n",
		body, targ, locm.c_str(), succ, res.c_str());
	}
      else if(body->IsAct(ACT_WIELD)	//Ranged Weapon
		&& body->ActTarg(ACT_WIELD)->Skill("WeaponReach") > 9) {
	body->Parent()->SendOut(ALL, -1,
		"*;s throws %s and hits ;s%s. [%d] %s\n",
		"*You throw %s and hit ;s%s. [%d] %s\n", body, targ,
		body->ActTarg(ACT_WIELD)->ShortDesc(), locm.c_str(),
		succ, res.c_str());
	body->ActTarg(ACT_WIELD)->Travel(body->Parent());//FIXME: Get Another
	body->StopAct(ACT_WIELD);			//FIXME: Bows/Guns!
	}
      else if(body->IsAct(ACT_WIELD)) {	//Melee Weapon
	body->Parent()->SendOut(ALL, -1,
		"*;s hits ;s%s with %s. [%d] %s\n",
		"*You hit ;s%s with %s. [%d] %s\n", body, targ,
		locm.c_str(), body->ActTarg(ACT_WIELD)->ShortDesc(),
		succ, res.c_str());
	}
      else {				//No Weapon
	body->Parent()->SendOut(ALL, -1,
		"*;s punches ;s%s. [%d] %s\n", "*You punch ;s%s. [%d] %s\n",
		body, targ, locm.c_str(), succ, res.c_str());
	}
      int sev = 0;

      if(com == COM_KICK) {
	if(targ->ActTarg(loca))
	  succ -= roll(targ->ActTarg(loca)->Attribute(0), body->Attribute(2));
	sev = targ->HitStun(body->Attribute(2), stage+2, succ);
	}
      else if(body->IsAct(ACT_WIELD)) {
	if(targ->ActTarg(loca))
	  succ -= roll(
		targ->ActTarg(loca)->Attribute(0),
		body->Attribute(2)
			+ MAX(0, body->ActTarg(ACT_WIELD)->Skill("WeaponForce"))
		);
	int force = body->Attribute(2)
		+ MAX(0, body->ActTarg(ACT_WIELD)->Skill("WeaponForce"));

	if(two_handed(body->ActTarg(ACT_WIELD)->Skill("WeaponType"))) {
	  force += body->Attribute(2);
	  }

	sev = targ->HitPhys(force,
	  stage + MAX(0, body->ActTarg(ACT_WIELD)->Skill("WeaponSeverity")),
	  succ);
	}
      else {
	if(targ->ActTarg(loca))
	  succ -= roll(targ->ActTarg(loca)->Attribute(0), body->Attribute(2));
	sev = targ->HitStun(body->Attribute(2), stage+1, succ);
	}

      if(sev <= 0) {
	if(mind) mind->Send("You hit - but didn't do much.\n");  //FIXME - Real Messages
	}
      }
    else {
      if(com == COM_KICK) {		//Kicking Action
	body->Parent()->SendOut(ALL, -1,
		";s tries to kick ;s, but misses. [%d] %s\n",
		"You try to kick ;s, but miss. [%d] %s\n",
		body, targ, succ, res.c_str()
		);
	}
      else if(body->IsAct(ACT_WIELD)	//Ranged Weapon
		&& body->ActTarg(ACT_WIELD)->Skill("WeaponReach") > 9) {
	body->Parent()->SendOut(ALL, -1,
		"*;s throws %s at ;s, but misses. [%d] %s\n",
		"*You throw %s at ;s, but miss. [%d] %s\n",
		body, targ, body->ActTarg(ACT_WIELD)->ShortDesc(),
		succ, res.c_str()
		);
	body->ActTarg(ACT_WIELD)->Travel(body->Parent());//FIXME: Get Another
	body->StopAct(ACT_WIELD);			//FIXME: Bows/Guns!
	}
      else if(body->IsAct(ACT_WIELD)) {	//Melee Weapon
	body->Parent()->SendOut(ALL, -1,
		";s tries to attack ;s, but misses. [%d] %s\n",
		"You try to attack ;s, but miss. [%d] %s\n",
		body, targ, succ, res.c_str()
		);
	}
      else {				//Unarmed
	body->Parent()->SendOut(ALL, -1,
		";s tries to punch ;s, but misses. [%d] %s\n",
		"You try to punch ;s, but miss. [%d] %s\n",
		body, targ, succ, res.c_str()
		);
	}
      }

    if(targ->Attribute(1) <= 0 || targ->IsAct(ACT_DEAD)
	|| targ->IsAct(ACT_DYING) || targ->IsAct(ACT_UNCONSCIOUS)) {
      body->StopAct(ACT_FIGHT);
      body->BusyFor(3000);
      if(targ->Skill("Accomplishment")) {
	if(body->Accomplish(targ->Skill("Accomplishment"))) {
	  if(mind) {
	    mind->Send("%sYour character gains an experience point for this victory!\n%s",
		CYEL, CNRM);
	    }
	  }
	targ->SetSkill("Accomplishment", 0);
	}
      }

    return 0;
    }

  const char *statnames[] = {
	"Body", "Quickness", "Strength",
	"Charisma", "Intelligence", "Willpower"
	};

  if(com == COM_RANDOMIZE) {
    while((!isgraph(comline[len])) && (comline[len])) ++len;

    Object *chr = mind->Owner()->Creator();
    if(!chr) {
      mind->Send("You need to be working on a character first (use 'select <character>'.\n");
      return 0;
      }
    else if(strlen(comline+len) > 0) {
      mind->Send("Just type 'randomize' to randomly spend all points for %s\n",
	chr->ShortDesc());
      return 0;
      }

    while(chr->Skill("Attributes")) {
      int which = (rand()%6);
      if(chr->Attribute(which) < 6) {
	chr->SetAttribute(which, chr->Attribute(which) + 1);
	chr->SetSkill("Attributes", chr->Skill("Attributes") - 1);
	}
      }
    list<string> skills = get_skills("all");
    while(chr->Skill("Skills")) {
      int which = (rand()%skills.size());
      list<string>::iterator skl = skills.begin();
      while(which) { ++skl; --which; }
      if(chr->Skill(*skl) < (chr->Attribute(get_linked(*skl))+1) / 2) {
	chr->SetSkill(*skl, chr->Skill(*skl) + 1);
	chr->SetSkill("Skills", chr->Skill("Skills") - 1);
	}
      }
    mind->Send("You randomly spend all remaining points for '%s'.\n",
	chr->ShortDesc());
    return 0;
    }

  if(com == COM_RAISE) {
    if((!mind) || (!mind->Owner())) return 0;
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    Object *chr = body;
    if(!chr) {
      chr = mind->Owner()->Creator();
      if(!chr) {
	mind->Send("You need to be working on a character first (use 'select <character>'.\n");
	return 0;
	}
      }
    if(strlen(comline+len) <= 0) {
      mind->Send("What do you want to buy?\n");
      return 0;
      }

    int cost = 1;
    if( (!strncasecmp(comline+len, "body", strlen(comline+len)))
	    || (!strncasecmp(comline+len, "quickness", strlen(comline+len)))
	    || (!strncasecmp(comline+len, "strength", strlen(comline+len)))
	    || (!strncasecmp(comline+len, "charisma", strlen(comline+len)))
	    || (!strncasecmp(comline+len, "intelligence", strlen(comline+len)))
	    || (!strncasecmp(comline+len, "willpower", strlen(comline+len))) ) {
      if((!body) && (!chr->Skill("Attributes"))) {
	mind->Send("You have no free attribute points left.\n");
	return 0;
	}
      int attr=0;
      if(toupper(*(comline+len)) == 'B') attr = 0;
      else if(toupper(*(comline+len)) == 'Q') attr = 1;
      else if(toupper(*(comline+len)) == 'S') attr = 2;
      else if(toupper(*(comline+len)) == 'C') attr = 3;
      else if(toupper(*(comline+len)) == 'I') attr = 4;
      else if(toupper(*(comline+len)) == 'W') attr = 5;

      cost = chr->Attribute(attr) + 1;

      if(body && chr->Exp(mind->Owner()) < (cost * 4)) {
	mind->Send("You don't have enough experience to raise your %s.\n"
		"You need %d, but you only have %d\n",
		statnames[attr], cost * 4, chr->Exp(mind->Owner()));
	return 0;
	}
      if((!body) && (chr->Skill("Attributes") < cost)) {
	mind->Send("You don't have enough free attribute points left.\n");
	return 0;
	}

      const char *maxask[6] = {
	"MaxBody", "MaxQuickness", "MaxStrength",
	"MaxCharisma", "MaxIntelligence", "MaxWillpower"
	};
      if((body) && (!body->Skill(maxask[attr]))) {
	body->SetSkill(maxask[attr], (body->Attribute(attr)*3)/2);
	}

      if((!body) && chr->Attribute(attr) >= 6) {
	mind->Send("Your %s is already at the maximum.\n", statnames[attr]);
	}
      else if(body && chr->Attribute(attr) >= body->Skill(maxask[attr])) {
	mind->Send("Your %s is already at the maximum.\n", statnames[attr]);
	}
      else {
	if(!body) chr->SetSkill("Attributes", chr->Skill("Attributes") - cost);
	else chr->SpendExp(cost * 4);
	chr->SetAttribute(attr, chr->Attribute(attr) + 1);
	mind->Send("You raise your %s.\n", statnames[attr]);
	}
      }
    else if((!body)
	&& (!strncasecmp(comline+len, "senses", strlen(comline+len)))) {
      if(!chr->Skill("Attributes")) {
	mind->Send("You have no free attribute points left.\n");
	return 0;
	}
      chr->SetSkill("Attributes", chr->Skill("Attributes") - 1);
      chr->SetSkill("Senses", chr->Skill("Senses") + 1);
      mind->Send("You set aside an attribute point for senses.\n");
      }
    else {
      string skill = get_skill(comline+len);
      if(skill != "") {

        if(body && (chr->Skill(skill)
		>= (chr->Attribute(get_linked(skill))*3+1) / 2)) {
	  mind->Send("Your %s is already at the maximum.\n", skill.c_str());
	  return 0;
	  }
        else if((!body) && (chr->Skill(skill)
		>= (chr->Attribute(get_linked(skill))+1) / 2)) {
	  mind->Send("Your %s is already at the maximum.\n", skill.c_str());
	  return 0;
	  }
	cost = (chr->Skill(skill) + 1);
        if(body) {
	  if(cost > chr->Attribute(get_linked(skill))) cost *= 2;
	  if(chr->Exp(mind->Owner()) < (cost * 2)) {
	    mind->Send("You don't have enough experience to raise your %s.\n"
		"You need %d, but you only have %d\n",
		skill.c_str(), cost * 2, chr->Exp(mind->Owner()));
	    return 0;
	    }
	  }
        else if(!chr->Skill("Skills")) {
	  mind->Send("You have no free skill points left.\n");
	  return 0;
	  }
        else if(chr->Skill("Skills") < cost) {
	  mind->Send("You don't have enough free skill points left.\n");
	  return 0;
	  }
	if(body) chr->SpendExp(cost * 2);
	else chr->SetSkill("Skills", chr->Skill("Skills") - cost);
	chr->SetSkill(skill, chr->Skill(skill) + 1);
	mind->Send("You raise your %s skill.\n", skill.c_str());
	}
      else {
	mind->Send("I'm not sure what you are trying to buy.\n");
	}
      }
    return 0;
    }

  if(com == COM_NEWCHARACTER) {
    if(!mind) return 0; //FIXME: Should never happen!
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    if(!comline[len]) {
      mind->Send("What's the character's name?  Use 'newcharacter <charname>'.\n");
      return 0;
      }

    const char *ch = comline+len;
    while((*ch) && isalpha(*ch)) ++ch;
    if(*ch) {
      mind->Send("Sorry, character names can only contain letters.\n"
			"Pick another name.\n");
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

  if(com == COM_RECALL) {
    if(body->Phys() || body->Stun()) {
      if(mind) mind->Send("You must be uninjured to use that command!\n");
      }
    else {
      body->Travel(get_start_room(), 0);
      body->Parent()->SendOut(0, 0, //Not Stealthy!
	"BAMF! ;s teleports home.\n",
	"BAMF! You teleport home.\n", body, NULL);
      if(mind && mind->Type() == MIND_REMOTE)
	body->Parent()->SendDescSurround(body, body);
      }
    return 0;
    }

  if(com == COM_SKILLLIST) {
    if(!mind) return 0;

    while((!isgraph(comline[len])) && (comline[len])) ++len;

    string skills;
    list<string> skls;
    if(!comline[len]) {
      skills = "Here are all the skill categories (use 'skill <Category>' to see the skills):\n";
      skls = get_skills();
      }
    else {
      string cat = get_skill_cat(comline+len);
      if(string(comline+len) != "all") {
	if(cat == "") {
	  mind->Send("There is no skill category called '%s'.\n", comline+len);
	  return 0;
	  }
	skills = "Total " + cat + " in play on this MUD:\n";
	}
      else {
	skills = "Total skills in play on this MUD:\n";
	cat = "all";
	}
      skls = get_skills(cat);
      }

    list<string>::iterator skl = skls.begin();
    for(; skl != skls.end(); ++skl) {
      skills += (*skl);
      skills += "\n";
      }
    mind->Send(skills.c_str());

    return 0;
    }

  if(com == COM_WHO) {
    if(!mind) return 0;
    string users = "Currently on this MUD:\n";
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
	(*mn)->Send("%s", mes.c_str());
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
      mind->Send("You rename '%s' to '%s'\n", oldn.c_str(),
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
      mind->Send("You redescribe '%s' as '%s'\n", oldn.c_str(),
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
      mind->Send("You redefine '%s' as '%s'\n", oldn.c_str(),
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
    Object *targ = body->PickObject(comline+len, LOC_NEARBY);
    if(!targ) {
      mind->Send("You want to control who?\n");
      }
    else if(targ->Attribute(1) <= 0) {
      mind->Send("You can't control inanimate objects!\n");
      }
    else {
      body->Parent()->SendOut(stealth_t, stealth_s, 
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
    else if(targ->Attribute(5) <= 0) {
      mind->Send("You can't command an object that has no will of its own.\n");
      }
    else {
      body->Parent()->SendOut(stealth_t, stealth_s, 
	";s commands ;s to '%s' with Ninja Powers[TM].\n",
	"You command ;s to '%s'.\n",
	body, targ, comline+len);

      if(handle_command(targ, comline+len) > 0)
	body->Parent()->SendOut(stealth_t, stealth_s, 
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
      body->Parent()->SendOut(stealth_t, stealth_s, 
	";s creates a new object with Ninja Powers[TM].\n",
	"You create a new object.\n", body, NULL);
      }
    else {
	//FIXME - this is fuxxored
      mind->Send("Argument mode is disabled for now\n");
//      body->Parent()->LinkToNew(comline+len);
//      body->Parent()->SendOut(stealth_t, stealth_s, 
//	";s creates a new object with Ninja Powers[TM].\n",
//	"You create a new object.\n", body, NULL);
      }
    return 0;
    }

  if(com == COM_DCREATE) {
    if(!mind) return 0;
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    if(comline[len] == 0) {
      mind->Send("You need to specify in what direction!\n");
      }
    else {
      Object *box = new Object(body->Parent()->Parent());
      Object *next = new Object(box);
      const char *dirb="south";
      const char *dir="north";
      if(!strcmp(comline+len, "north")) {
	}
      else if(!strcmp(comline+len, "south")) {
	dirb = "north"; dir = "south";
	}
      else if(!strcmp(comline+len, "west")) {
	dirb = "east"; dir = "west";
	}
      else if(!strcmp(comline+len, "east")) {
	dirb = "west"; dir = "east";
	}
      else if(!strcmp(comline+len, "up")) {
	dirb = "down"; dir = "up";
	}
      else if(!strcmp(comline+len, "down")) {
	dirb = "up"; dir = "down";
	}
      else {
	mind->Send("Direction '%s' not meaningful!\n", comline+len);
	return 0;
	}

      box->SetShortDesc("a dynamic dungeon");
      next->SetShortDesc("An Entrance to a Large Mining Tunnel");
      next->SetDesc(
        "This tunnel looks to have been carved centuries ago.  It is so well crafted\n"
        "that you think it will stand as-is for another millenium.\n");
      next->SetSkill("DynamicInit", 1);
      next->SetSkill("DynamicPhase", 0); //Entrance
      next->SetSkill("DynamicMojo", 1000000);

      body->Parent()->Link(next,
	dir, string("You see a solid passage leading ") + dir + ".\n",
	dirb, string("You see a solid passage leading ") + dirb + ".\n"
	);

      body->Parent()->SendOut(stealth_t, stealth_s, 
	";s creates a new dynamic dungeon '%s' with Ninja Powers[TM].\n",
	"You create a new dynamic dungeon '%s'.\n", body, NULL, dir);
      }
    return 0;
    }

#define NUM_AVS	5	// 28 Max!
#define NUM_STS	5	// No Max
  if(com == COM_CCREATE) {
    if(!mind) return 0;
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    return handle_command_ccreate(body, mind, comline, len, stealth_t, stealth_s);
    }

  static Object *anchor = NULL;

  if(com == COM_ANCHOR) {
    if(!mind) return 0;
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    anchor = new Object(body->Parent());
    anchor->SetShortDesc("a shimmering portal");
    anchor->SetDesc("This portal could only have been created by a True Ninja[TM].  You wonder where it leads.");
    body->Parent()->SendOut(stealth_t, stealth_s, 
	";s creates a shimmering portal with Ninja Powers[TM].\n",
	"You create a shimmering portal.\n", body, NULL);
    return 0;
    }

  if(com == COM_LINK) {
    if(!mind) return 0;
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    if(anchor == NULL) {
      mind->Send("You need to make an anchor before you can link to it!\n");
      }
    else if(comline[len] == 0) {
      mind->Send("You need to specify what the portal will be named!\n");
      }
    else {
      Object *link;

      body->Parent()->SendOut(stealth_t, stealth_s, 
	";s creates a shimmering portal '%s' with Ninja Powers[TM].\n",
	"You create a shimmering portal '%s'.\n", body, NULL, comline+len);

      link = new Object(body->Parent());
      link->SetShortDesc(comline+len);
      link->AddAct(ACT_SPECIAL_LINKED, anchor);
      link->AddAct(ACT_SPECIAL_MASTER, anchor);
      link->SetSkill("Open", 1);
      link->SetSkill("Enterable", 1);
      anchor->AddAct(ACT_SPECIAL_LINKED, link);
      anchor->AddAct(ACT_SPECIAL_MASTER, link);
      anchor->SetSkill("Open", 1);
      anchor->SetSkill("Enterable", 1);
      string other = comline+len;
      if(!strncmp(comline+len, "east", 4)) {
	other = "west";  other += comline+len+4;
	}
      else if(!strncmp(comline+len, "west", 4)) {
	other = "east";  other += comline+len+4;
	}
      else if(!strncmp(comline+len, "north", 5)) {
	other = "south";  other += comline+len+5;
	}
      else if(!strncmp(comline+len, "south", 5)) {
	other = "north";  other += comline+len+5;
	}
      else if(!strncmp(comline+len, "up", 2)) {
	other = "down";  other += comline+len+2;
	}
      else if(!strncmp(comline+len, "down", 4)) {
	other = "up";  other += comline+len+4;
	}
      anchor->SetShortDesc(other.c_str());
      anchor = NULL;
      }
    return 0;
    }

  if(com == COM_DELPLAYER) {
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    if(comline[len] == 0) {
      if(mind) mind->Send("You want to delete which player?\n");
      }
    else {
      Player *pl = get_player(comline+len);
      if(!pl) {
	if(mind) mind->Send("That player doesn't seem to exist.\n");
	}
      else {
	if(mind) mind->Send("You delete the player '%s'.\n", pl->Name());
	delete pl;
	}
      }
    return 0;
    }

  if(com == COM_PLAYERS) {
    if(!mind) return 0;
    string users = "Current accounts on this MUD:\n";
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
    string chars = "Current characters on this MUD:\n";
    vector<Player *> pls = get_all_players();

    vector<Player *>::iterator pl = pls.begin();
    for(; pl != pls.end(); ++pl) {
      typeof((*pl)->Room()->Contents()) chs = (*pl)->Room()->Contents();
      typeof(chs.begin()) ch = chs.begin();
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
      typeof(targ->Contents()) cont = targ->Contents();
      typeof(cont.begin()) item = cont.begin();
      for(; item != cont.end(); ++item) delete (*item);
      targ->Travel(get_start_room(), 0);

      body->Parent()->SendOut(stealth_t, stealth_s, 
	";s resets ;s with Ninja Powers[TM].\n", "You reset ;s.\n",
	body, targ);
      }
    return 0;
    }

  if(com == COM_MIRROR) {
    if(!mind) return 0;
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    Object *targ = body->PickObject(comline+len, LOC_NEARBY|LOC_INTERNAL);
    if(!targ) {
      mind->Send("You want to mirror what?\n");
      }
    else {
      Object *nobj = new Object(*targ);
      nobj->SetParent(targ->Parent());

      nobj->SetSkill("Wearable on Left Arm",
	targ->Skill("Wearable on Right Arm"));
      nobj->SetSkill("Wearable on Left Finger",
	targ->Skill("Wearable on Right Finger"));
      nobj->SetSkill("Wearable on Left Foot",
	targ->Skill("Wearable on Right Foot"));
      nobj->SetSkill("Wearable on Left Hand",
	targ->Skill("Wearable on Right Hand"));
      nobj->SetSkill("Wearable on Left Leg",
	targ->Skill("Wearable on Right Leg"));
      nobj->SetSkill("Wearable on Left Wrist",
	targ->Skill("Wearable on Right Wrist"));
      nobj->SetSkill("Wearable on Left Shoulder",
	targ->Skill("Wearable on Right Shoulder"));
      nobj->SetSkill("Wearable on Left Hip",
	targ->Skill("Wearable on Right Hip"));

      nobj->SetSkill("Wearable on Right Arm",
	targ->Skill("Wearable on Left Arm"));
      nobj->SetSkill("Wearable on Right Finger",
	targ->Skill("Wearable on Left Finger"));
      nobj->SetSkill("Wearable on Right Foot",
	targ->Skill("Wearable on Left Foot"));
      nobj->SetSkill("Wearable on Right Hand",
	targ->Skill("Wearable on Left Hand"));
      nobj->SetSkill("Wearable on Right Leg",
	targ->Skill("Wearable on Left Leg"));
      nobj->SetSkill("Wearable on Right Wrist",
	targ->Skill("Wearable on Left Wrist"));
      nobj->SetSkill("Wearable on Right Shoulder",
	targ->Skill("Wearable on Left Shoulder"));
      nobj->SetSkill("Wearable on Right Hip",
	targ->Skill("Wearable on Left Hip"));

      int start;
      string name = nobj->ShortDesc();
      start = name.find(string("(left)"));
      if(start < int(name.length()) && start >= 0) {
	name = name.substr(0, start) + "(right)" + name.substr(start+6);
	}
      else {
        start = name.find(string("(right)"));
        if(start < int(name.length()) && start >= 0) {
	  name = name.substr(0, start) + "(left)" + name.substr(start+7);
	  }
	}
      nobj->SetShortDesc(name.c_str());

      body->Parent()->SendOut(stealth_t, stealth_s, 
	";s mirrors ;s with Ninja Powers[TM].\n", "You mirror ;s.\n",
	body, targ);
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
      Object *nobj = new Object(*targ);
      nobj->SetParent(targ->Parent());

      body->Parent()->SendOut(stealth_t, stealth_s, 
	";s clones ;s with Ninja Powers[TM].\n", "You clone ;s.\n",
	body, targ);
      }
    return 0;
    }

  if(com == COM_JUNK) {
    if(!mind) return 0;
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    typeof(body->Contents()) targs
	= body->PickObjects(comline+len, LOC_NEARBY);
    if(targs.size() == 0) {
      mind->Send("You want to destroy what?\n");
      return 0;
      }
    while(targs.size() > 0) {
      Object *targ = targs.front();
      body->Parent()->SendOut(stealth_t, stealth_s, 
	";s destroys ;s with Ninja Powers[TM].\n", "You destroy ;s.\n",
	body, targ);
      delete(targ);
      targs = body->PickObjects(comline+len, LOC_NEARBY);
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
      //This is ninja-healing and bypasses all healing mechanisms.
      targ->SetPhys(0);
      targ->SetStun(0);
      targ->SetStru(0);
      targ->UpdateDamage();

      body->Parent()->SendOut(stealth_t, stealth_s, 
	";s heals and repairs ;s with Ninja Powers[TM].\n", "You heal ;s.\n",
	body, targ);
      }
    return 0;
    }

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

    if(targ->Attribute(stat) == 0) {
      mind->Send("This object doesn't have that stat.\n");
      return 0;
      }

    targ->SetAttribute(stat, targ->Attribute(stat) + 1);

    body->Parent()->SendOut(stealth_t, stealth_s, 
	";s jacks the %s of ;s with Ninja Powers[TM].\n",
	"You jack the %s of ;s.\n",
	body, targ, statnames[stat]);

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

    if(targ->Attribute(stat) == 0) {
      mind->Send("This object doesn't have that stat.\n");
      return 0;
      }
    if(targ->Attribute(stat) == 1) {
      mind->Send("It is already a 1 (the minimum!).\n");
      return 0;
      }

    targ->SetAttribute(stat, targ->Attribute(stat) - 1);

    body->Parent()->SendOut(stealth_t, stealth_s, 
	";s chumps the %s of ;s with Ninja Powers[TM].\n",
	"You chump the %s of ;s.\n",
	body, targ, statnames[stat]);

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

    if(!is_skill(comline+len)) {
      mind->Send("Warning, '%s' is not a real skill name!\n", comline+len);
      }

    targ->SetSkill(comline+len, MAX(targ->Skill(comline+len), 0) + 1);

    body->Parent()->SendOut(stealth_t, stealth_s, 
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

    targ->SetSkill(comline+len, targ->Skill(comline+len) - 1);

    body->Parent()->SendOut(stealth_t, stealth_s, 
	";s decrements the %s of ;s with Ninja Powers[TM].\n",
	"You decrement the %s of ;s.\n",
	body, targ, comline+len);

    return 0;
    }

  if(com == COM_DOUBLE) {
    if(!mind) return 0;
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    typeof(body->Contents()) targs
	= body->PickObjects(comline+len, LOC_NEARBY|LOC_INTERNAL);
    if(targs.size() == 0) {
      mind->Send("You want to double what?\n");
      return 0;
      }
    typeof(targs.begin()) targ_it;
    for(targ_it = targs.begin(); targ_it != targs.end(); ++ targ_it) {
      Object *targ = (*targ_it);
      body->Parent()->SendOut(stealth_t, stealth_s, 
	";s doubles ;s with Ninja Powers[TM].\n", "You double ;s.\n",
	body, targ);
      if(targ->Skill("Quantity") > 1) {
	targ->SetSkill("Quantity", targ->Skill("Quantity") * 2);
	}
      else {
	targ->SetSkill("Quantity", 2);
	}
      }
    return 0;
    }

  if(com == COM_CLOAD) {
    if(!mind) return 0;
    while((!isgraph(comline[len])) && (comline[len])) ++len;
    if(!comline[len]) {
      body->Parent()->CircleLoadAll();
      body->Parent()->SendOut(stealth_t, stealth_s, 
	";s loads the entire Circle world with Ninja Powers[TM].\n",
	"You load the entire Circle world.\n", body, NULL);
      }
    else {
      sprintf(buf, "circle/wld/%s.wld", comline+len);
      body->Parent()->CircleLoad(buf);
      sprintf(buf, "circle/obj/%s.obj", comline+len);
      body->Parent()->CircleLoadObj(buf);
      body->Parent()->SendOut(stealth_t, stealth_s, 
	";s loads a Circle world ;s with Ninja Powers[TM].\n",
	"You load a Circle world.\n", body, NULL);
      }
    return 0;
    }

  if(com == COM_CCLEAN) {
    if(!mind) return 0;
    body->CircleCleanup();
    body->Parent()->SendOut(stealth_t, stealth_s, 
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
