#include <ctype.h>
#include <unistd.h>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "color.hpp"
#include "commands.hpp"
#include "main.hpp"
#include "mind.hpp"
#include "net.hpp"
#include "object.hpp"
#include "utils.hpp"
#include "version.hpp"

#define SIT_ETHEREAL 1
#define SIT_CORPOREAL 2
#define SIT_ALIVE 4
#define SIT_CONSCIOUS 8
#define SIT_AWAKE 16
#define SIT_ALERT 32
#define SIT_LIE 64
#define SIT_SIT 128
#define SIT_STAND 256
#define SIT_USE 512
#define SIT_ACTION 4096
#define SIT_NINJAMODE 8192
#define SIT_NINJA 16384
#define SIT_SUPERNINJA 32768

#define CMD_FLAVORTEXT 65536

#define REQ_ETHEREAL (SIT_ETHEREAL)
#define REQ_CORPOREAL (SIT_CORPOREAL)
#define REQ_ALIVE (SIT_ALIVE | REQ_CORPOREAL)
#define REQ_CONSCIOUS (SIT_CONSCIOUS | REQ_ALIVE)
#define REQ_AWAKE (SIT_AWAKE | REQ_CONSCIOUS)
#define REQ_ALERT (SIT_ALERT | REQ_AWAKE)
#define REQ_LIE (SIT_LIE | REQ_CORPOREAL)
#define REQ_SIT (SIT_SIT | REQ_CORPOREAL)
#define REQ_STAND (SIT_STAND | REQ_CORPOREAL)
#define REQ_UP (SIT_STAND | SIT_USE | REQ_CORPOREAL)

#define REQ_ACTION (SIT_ACTION | REQ_CORPOREAL)

#define REQ_NINJAMODE (SIT_NINJAMODE)
#define REQ_NINJA (SIT_NINJA)
#define REQ_SUPERNINJA (SIT_SUPERNINJA | SIT_NINJA)

#define REQ_ANY (SIT_CORPOREAL | SIT_ETHEREAL)

int handle_command_ccreate(Object*, Mind*, const char*, int, int, int);

static int count_ones(int mask) {
  int ret = 0;
  while (mask) {
    ++ret;
    mask &= (mask - 1);
  }
  return ret;
}

struct Command {
  com_t id;
  const char* command;
  const char* shortdesc;
  const char* longdesc;
  int sit;
};

constexpr Command static_comlist[COM_MAX] = {
    {COM_NONE, nullptr, nullptr, nullptr, 0},
    {COM_HELP,
     "help",
     "Get help for a topic or command.",
     "Get help for a topic or command.",
     (REQ_ANY)},
    {COM_QUIT, "quit", "Quit the game.", "Quit the game.", (REQ_ANY)},

    {COM_NORTH, "north", "Travel north.", "Travel north.", (REQ_ALERT | REQ_UP | REQ_ACTION)},
    {COM_SOUTH, "south", "Travel south.", "Travel south.", (REQ_ALERT | REQ_UP | REQ_ACTION)},
    {COM_EAST, "east", "Travel east.", "Travel east.", (REQ_ALERT | REQ_UP | REQ_ACTION)},
    {COM_WEST, "west", "Travel west.", "Travel west.", (REQ_ALERT | REQ_UP | REQ_ACTION)},
    {COM_UP, "up", "Travel up.", "Travel up.", (REQ_ALERT | REQ_UP | REQ_ACTION)},
    {COM_DOWN, "down", "Travel down.", "Travel down.", (REQ_ALERT | REQ_UP | REQ_ACTION)},

    {COM_LOOK,
     "look",
     "Look around, look in a direction, or look at an object or creature.",
     "Look around, look in a direction, or look at an object or creature.",
     (REQ_AWAKE | REQ_ACTION | REQ_ETHEREAL)},
    {COM_EXAMINE,
     "examine",
     "Examine an object or creature.",
     "Examine an object or creature.",
     (REQ_ALERT | REQ_ACTION)},
    {COM_CONSIDER,
     "consider",
     "Consider attacking someone, or using something, and size it up.",
     "Consider attacking someone, or using something, and size it up.",
     (REQ_ALERT | REQ_ACTION)},
    {COM_INVENTORY,
     "inventory",
     "Check what you have on you.",
     "Check what you have on you.",
     (REQ_AWAKE | REQ_ACTION)},
    {COM_EQUIPMENT,
     "equipment",
     "Check what you are wearing and using.",
     "Check what you are wearing and using.",
     (REQ_AWAKE | REQ_ACTION)},
    {COM_SEARCH,
     "search",
     "Search an area, object or creature.",
     "Search an area, object or creature.",
     (REQ_ALERT | REQ_UP | REQ_ACTION)},
    {COM_HIDE,
     "hide",
     "Hide an object, or yourself.",
     "Hide an object, or yourself.",
     (REQ_ALERT | REQ_UP | REQ_ACTION)},

    {COM_LEAVE, "leave", "Leave an object.", "Leave an object.", (REQ_ALERT | REQ_UP | REQ_ACTION)},
    {COM_ENTER,
     "enter",
     "Enter an object (or enter the game).",
     "Enter an object (or enter the game).",
     (REQ_ETHEREAL | REQ_ALERT | REQ_UP | REQ_ACTION)},
    {COM_SELECT,
     "select",
     "Select an unfinished character.",
     "Select an unfinished character.",
     (REQ_ETHEREAL)},

    {COM_OPEN,
     "open",
     "Open a door or container.",
     "Open a door or container.",
     (REQ_ALERT | REQ_UP | REQ_ACTION)},
    {COM_CLOSE,
     "close",
     "Close a door or container.",
     "Close a door or container.",
     (REQ_ALERT | REQ_UP | REQ_ACTION)},
    {COM_UNLOCK,
     "unlock",
     "Unlock a door or container.",
     "Unlock a door or container.",
     (REQ_ALERT | REQ_UP | REQ_ACTION)},
    {COM_LOCK,
     "lock",
     "Lock a door or container.",
     "Lock a door or container.",
     (REQ_ALERT | REQ_UP | REQ_ACTION)},

    {COM_GET,
     "get",
     "Get an item from your surroundings.",
     "Get an item from your surroundings.",
     (REQ_ALERT | REQ_UP | REQ_ACTION)},
    {COM_DRAG,
     "drag",
     "Drag a heavy item with you when you next move.",
     "Drag a heavy item with you when you next move.",
     (REQ_ALERT | REQ_STAND | REQ_ACTION)},
    {COM_PUT,
     "put",
     "Put a held item in or on something.",
     "Put a held item in or on something.",
     (REQ_ALERT | REQ_UP | REQ_ACTION)},
    {COM_DROP,
     "drop",
     "Drop an item you are carrying.",
     "Drop an item you are carrying.",
     (REQ_ALERT)},
    {COM_STASH,
     "stash",
     "Store an item you are holding in one of your containers.",
     "Store an item you are holding in one of your containers.",
     (REQ_ALERT | REQ_ACTION)},
    {COM_WIELD,
     "wield",
     "Wield a weapon you are carrying.",
     "Wield a weapon you are carrying.",
     (REQ_ALERT | REQ_ACTION)},
    {COM_UNWIELD,
     "unwield",
     "Unwield the weapon you are currently wielding.",
     "Unwield the weapon you are currently wielding.",
     (REQ_ALERT | REQ_ACTION)},
    {COM_HOLD,
     "hold",
     "Hold an item you are carrying in your off-hand.",
     "Hold an item you are carrying in your off-hand.",
     (REQ_ALERT | REQ_ACTION)},
    {COM_LIGHT,
     "light",
     "Light an item you are carrying and hold it in your off-hand.",
     "Light an item you are carrying and hold it in your off-hand.",
     (REQ_ALERT | REQ_ACTION)},
    {COM_WEAR,
     "wear",
     "Wear an item you are carrying.",
     "Wear an item you are carrying.",
     (REQ_ALERT | REQ_ACTION)},
    {COM_REMOVE,
     "remove",
     "Remove an item you are wearing.",
     "Remove an item you are wearing.",
     (REQ_ALERT | REQ_ACTION)},
    {COM_LABEL,
     "label",
     "Label, or read the label of, an item you are holding.",
     "Label, or read the label of, an item you are holding.",
     (REQ_ALERT | REQ_ACTION | CMD_FLAVORTEXT)},
    {COM_UNLABEL,
     "unlabel",
     "Remove the label the item you are holding.",
     "Remove the label the item you are holding.",
     (REQ_ALERT | REQ_ACTION)},
    {COM_HEAL,
     "heal",
     "Use healing/first-aid skills to help another, or yourself.",
     "Use healing/first-aid skills to help another, or yourself.",
     (REQ_CORPOREAL | REQ_ACTION)},

    {COM_EAT,
     "eat",
     "Eat an item you are carrying.",
     "Eat an item you are carrying.",
     (REQ_ALERT | REQ_ACTION)},
    {COM_DRINK,
     "drink",
     "Drink from an item you are carrying.",
     "Drink from an item you are carrying.",
     (REQ_ALERT | REQ_ACTION)},
    {COM_FILL,
     "fill",
     "Fill a held liquid container from another.",
     "Fill a held liquid container from another.",
     (REQ_ALERT | REQ_ACTION)},
    {COM_DUMP,
     "dump",
     "Dump all liquid out of a container you are carrying.",
     "Dump all liquid out of a container you are carrying.",
     (REQ_ALERT | REQ_ACTION)},

    {COM_SLEEP, "sleep", "Go to sleep.", "Go to sleep.", (REQ_CONSCIOUS | REQ_ACTION)},
    {COM_WAKE,
     "wake",
     "Wake up, or wake someone else up.",
     "Wake up, or wake someone else up.",
     (REQ_CONSCIOUS | REQ_ACTION)},
    {COM_LIE, "lie", "Lie down.", "Lie down.", (REQ_CONSCIOUS | REQ_ACTION)},
    {COM_REST, "rest", "Relax and rest.", "Relax and rest.", (REQ_CONSCIOUS | REQ_ACTION)},
    {COM_SIT, "sit", "Sit down.", "Sit down.", (REQ_CONSCIOUS | REQ_ACTION)},
    {COM_STAND, "stand", "Stand up.", "Stand up.", (REQ_CONSCIOUS | REQ_ACTION)},
    {COM_USE,
     "use",
     "Start, or stop using a skill.",
     "Start, or stop using a skill.",
     (REQ_ALERT | REQ_UP | REQ_ACTION)},
    {COM_STOP,
     "stop",
     "Stop using a skill.",
     "Stop using a skill.",
     (REQ_ALERT | REQ_UP | REQ_ACTION)},
    {COM_CAST, "cast", "Cast a spell.", "Cast a spell.", (REQ_ALERT | REQ_UP | REQ_ACTION)},
    {COM_PRAY,
     "pray",
     "Use faith to enhance another command.",
     "Use faith to enhance another command.",
     (REQ_ALERT | REQ_UP | REQ_ACTION)},

    {COM_SHOUT,
     "shout",
     "Shout something to all nearby.",
     "Shout something to all nearby.",
     (REQ_AWAKE | CMD_FLAVORTEXT)},
    {COM_YELL,
     "yell",
     "Shout something to all nearby.",
     "Shout something to all nearby.",
     (REQ_AWAKE | CMD_FLAVORTEXT)},
    {COM_CALL,
     "call",
     "Shout something to all nearby.",
     "Shout something to all nearby.",
     (REQ_AWAKE | CMD_FLAVORTEXT)},
    {COM_SAY, "say", "Say something.", "Say something.", (REQ_AWAKE | CMD_FLAVORTEXT)},
    {COM_EMOTE,
     "emote",
     "Indicate to others that you are doing something.",
     "Indicate to others that you are doing something.",
     (REQ_AWAKE | CMD_FLAVORTEXT)},

    {COM_POINT,
     "point",
     "Point at an object, or stop pointing.",
     "Point at an object, or stop pointing.",
     (REQ_ALERT)},
    {COM_FOLLOW,
     "follow",
     "Follow someone, or stop following someone.",
     "Follow someone, or stop following someone.",
     (REQ_ALERT | REQ_UP)},
    {COM_ATTACK,
     "attack",
     "Attack somebody, or an object.",
     "Attack somebody, or an object.",
     (REQ_ALERT | REQ_ACTION | REQ_STAND)},
    {COM_KILL,
     "kill",
     "Try to kill somebody, or smash an object.",
     "Try to kill somebody, or smash an object.",
     (REQ_ALERT | REQ_ACTION | REQ_STAND)},
    {COM_PUNCH,
     "punch",
     "Punch somebody, or an object.",
     "Punch somebody, or an object.",
     (REQ_ALERT | REQ_ACTION | REQ_STAND)},
    {COM_KICK,
     "kick",
     "Kick somebody, or an object.",
     "Kick somebody, or an object.",
     (REQ_ALERT | REQ_ACTION | REQ_STAND)},
    {COM_FLEE,
     "flee",
     "Escape the area as fast as you can, in a random direction.",
     "Escape the area as fast as you can, in a random direction.",
     (REQ_ALERT | REQ_ACTION | REQ_UP)},

    {COM_LIST,
     "list",
     "List items available at a shop.",
     "List items available at a shop.",
     (REQ_ALERT | REQ_ACTION)},
    {COM_BUY, "buy", "Buy an item at a shop.", "Buy an item at a shop.", (REQ_ALERT | REQ_ACTION)},
    {COM_VALUE,
     "value",
     "Find out how much a shop will give you for an item.",
     "Find out how much a shop will give you for an item.",
     (REQ_ALERT | REQ_ACTION)},
    {COM_SELL,
     "sell",
     "Sell an item at a shop.",
     "Sell an item at a shop.",
     (REQ_ALERT | REQ_ACTION)},

    {COM_TOGGLE,
     "toggle",
     "Turn options on and off, or show current options setting.",
     "Turn options on and off, or show current options setting.",
     (REQ_ANY)},

    {COM_WHO,
     "who",
     "Get a list of who is on the MUD right now.",
     "Get a list of who is on the MUD right now.",
     (REQ_ANY)},
    {COM_OOC,
     "ooc",
     "Toggle or use ooc (Out-Of-Character) chat.",
     "Toggle or use ooc (Out-Of-Character) chat.",
     (REQ_ANY | CMD_FLAVORTEXT)},
    {COM_NEWBIE,
     "newbie",
     "Toggle or use newbie (new player) chat (use this to ask for help).",
     "Toggle or use newbie (new player) chat (use this to ask for help).",
     (REQ_ANY | CMD_FLAVORTEXT)},

    {COM_NEWCHARACTER,
     "newcharacter",
     "Create a new character.",
     "Create a new character.",
     (REQ_ETHEREAL | CMD_FLAVORTEXT)},
    {COM_RAISE,
     "raise",
     "Spend a skill or attribute point of current character.",
     "Spend a skill or attribute point of current character.",
     (REQ_ETHEREAL | REQ_CORPOREAL)},
    {COM_LOWER,
     "lower",
     "Lower a skill or attribute point of current in-progress character.",
     "Lower a skill or attribute point of current in-progress character.",
     (REQ_ETHEREAL)},
    {COM_RESETCHARACTER,
     "resetcharacter",
     "Undo (erase!) *all* work on in-progress character.",
     "Undo (erase!) *all* work on in-progress character.",
     (REQ_ETHEREAL)},
    {COM_RANDOMIZE,
     "randomize",
     "Spend all remaining points of current character randomly.",
     "Spend all remaining points of current character randomly.",
     (REQ_ETHEREAL)},
    {COM_ARCHTYPE,
     "archtype",
     "Replace (erase!) *all* work on in-progress character with preset.",
     "Replace (erase!) *all* work on in-progress character with preset.",
     (REQ_ETHEREAL)},
    {COM_SCORE,
     "score",
     "Get your current character and/or player's stats and score.",
     "Get your current character and/or player's stats and score.",
     (REQ_ETHEREAL | REQ_CORPOREAL)},

    {COM_TIME,
     "time",
     "Get the current world's MUD time.",
     "Get the current world's MUD time.",
     (REQ_ETHEREAL | REQ_CORPOREAL)},
    {COM_WORLD,
     "world",
     "Get the name of the current world.",
     "Get the name of the current world.",
     (REQ_ETHEREAL | REQ_CORPOREAL)},
    {COM_VERSION,
     "version",
     "Query the version information of running AcidMUD.",
     "Query the version information of running AcidMUD.",
     (REQ_ANY)},

    {COM_SKILLLIST,
     "skilllist",
     "List all available skill categories, or all skills in a category.",
     "List all available skill categories, or all skills in a category.",
     (REQ_ANY)},

    {COM_RECALL,
     "recall",
     "Teleport back to the start when you are uninjured.",
     "Teleport back to the start when you are uninjured.",
     (REQ_STAND)},
    {COM_TELEPORT,
     "teleport",
     "Teleport to a named location (requires a power to enable).",
     "Teleport to a named location (requires a power to enable).",
     (REQ_STAND)},
    {COM_RESURRECT,
     "resurrect",
     "Resurrect a long-dead character (one with no corpse left).",
     "Resurrect a long-dead character (one with no corpse left).",
     (REQ_STAND)},

    {COM_NINJAMODE,
     "ninjamode",
     "Ninja command: run a command in Ninja Mode[TM].",
     "Ninja command: run a command in Ninja Mode[TM].",
     (REQ_ETHEREAL | REQ_ANY | REQ_ACTION | REQ_NINJA | CMD_FLAVORTEXT)},
    {COM_MAKENINJA,
     "makeninja",
     "Ninja command: make (or unmake) another player into a True Ninja[TM].",
     "Ninja command: make (or unmake) another player into a True Ninja[TM].",
     (REQ_ANY | REQ_SUPERNINJA | REQ_NINJAMODE)},
    {COM_MAKESUPERNINJA,
     "makesuperninja",
     "Ninja command: make (or unmake) a True Ninja into a Super Ninja[TM].",
     "Ninja command: make (or unmake) a True Ninja into a Super Ninja[TM].",
     (REQ_ANY | REQ_SUPERNINJA | REQ_NINJAMODE)},

    {COM_CREATE,
     "create",
     "Ninja command.",
     "Ninja command - ninjas only!",
     (REQ_ALERT | REQ_NINJAMODE | CMD_FLAVORTEXT)},
    {COM_DCREATE,
     "dcreate",
     "Ninja command.",
     "Ninja command - ninjas only!",
     (REQ_ALERT | REQ_NINJAMODE)},
    {COM_CCREATE,
     "ccreate",
     "Ninja command.",
     "Ninja command - ninjas only!",
     (REQ_ALERT | REQ_NINJAMODE)},
    {COM_ANCHOR,
     "anchor",
     "Ninja command.",
     "Ninja command - ninjas only!",
     (REQ_ALERT | REQ_NINJAMODE)},
    {COM_LINK,
     "link",
     "Ninja command.",
     "Ninja command - ninjas only!",
     (REQ_ALERT | REQ_NINJAMODE)},
    {COM_CONNECT,
     "connect",
     "Ninja command.",
     "Ninja command - ninjas only!",
     (REQ_ALERT | REQ_NINJAMODE)},
    {COM_COMMAND,
     "command",
     "Ninja command.",
     "Ninja command - ninjas only!",
     (REQ_CORPOREAL | REQ_NINJAMODE | CMD_FLAVORTEXT)},
    {COM_CONTROL,
     "control",
     "Ninja command.",
     "Ninja command - ninjas only!",
     (REQ_CORPOREAL | REQ_NINJAMODE)},
    {COM_CLONE,
     "clone",
     "Ninja command.",
     "Ninja command - ninjas only!",
     (REQ_ALERT | REQ_NINJAMODE)},
    {COM_MIRROR,
     "mirror",
     "Ninja command.",
     "Ninja command - ninjas only!",
     (REQ_ALERT | REQ_NINJAMODE)},
    {COM_JUNK,
     "junk",
     "Ninja command.",
     "Ninja command - ninjas only!",
     (REQ_ALERT | REQ_NINJAMODE)},
    {COM_RESET,
     "reset",
     "Ninja command.",
     "Ninja command - ninjas only!",
     (REQ_ALERT | REQ_NINJAMODE)},

    {COM_PLAYERS,
     "players",
     "Ninja command.",
     "Ninja command - ninjas only!",
     (REQ_ANY | REQ_NINJAMODE)},
    {COM_DELPLAYER,
     "delplayer",
     "Ninja command.",
     "Ninja command - ninjas only!",
     (REQ_ANY | REQ_NINJAMODE)},
    {COM_CHARACTERS,
     "characters",
     "Ninja command.",
     "Ninja command - ninjas only!",
     (REQ_ANY | REQ_NINJAMODE)},

    {COM_JACK,
     "jack",
     "Ninja command.",
     "Ninja command - ninjas only!",
     (REQ_ALERT | REQ_NINJAMODE)},
    {COM_CHUMP,
     "chump",
     "Ninja command.",
     "Ninja command - ninjas only!",
     (REQ_ALERT | REQ_NINJAMODE)},
    {COM_INCREMENT,
     "increment",
     "Ninja command.",
     "Ninja command - ninjas only!",
     (REQ_ALERT | REQ_NINJAMODE | CMD_FLAVORTEXT)},
    {COM_DECREMENT,
     "decrement",
     "Ninja command.",
     "Ninja command - ninjas only!",
     (REQ_ALERT | REQ_NINJAMODE | CMD_FLAVORTEXT)},
    {COM_DOUBLE,
     "double",
     "Ninja command.",
     "Ninja command - ninjas only!",
     (REQ_ALERT | REQ_NINJAMODE)},

    {COM_SETSTATS,
     "setstats",
     "Ninja command.",
     "Ninja command - ninjas only!",
     (REQ_ALERT | REQ_NINJAMODE)},
    {COM_NAME,
     "name",
     "Ninja command.",
     "Ninja command - ninjas only!",
     (REQ_ALERT | REQ_NINJAMODE)},
    {COM_UNDESCRIBE,
     "undescribe",
     "Ninja command.",
     "Ninja command - ninjas only!",
     (REQ_ALERT | REQ_NINJAMODE)},
    {COM_DESCRIBE,
     "describe",
     "Ninja command.",
     "Ninja command - ninjas only!",
     (REQ_ALERT | REQ_NINJAMODE | CMD_FLAVORTEXT)},
    {COM_UNDEFINE,
     "undefine",
     "Ninja command.",
     "Ninja command - ninjas only!",
     (REQ_ALERT | REQ_NINJAMODE)},
    {COM_DEFINE,
     "define",
     "Ninja command.",
     "Ninja command - ninjas only!",
     (REQ_ALERT | REQ_NINJAMODE | CMD_FLAVORTEXT)},

    {COM_STATS,
     "stats",
     "Ninja command: Get stats of a character, object or creature.",
     "Ninja command: Get stats of a character, object or creature.",
     (REQ_ALERT | REQ_NINJAMODE)},

    {COM_SHUTDOWN,
     "shutdown",
     "Ninja command: shutdown the entire MUD.",
     "Ninja command: shutdown the entire MUD.",
     (REQ_ANY | REQ_NINJAMODE)},
    {COM_RESTART,
     "restart",
     "Ninja command: restart the entire MUD - preserving connections.",
     "Ninja command: restart the entire MUD - preserving connections.",
     (REQ_ANY | REQ_NINJAMODE)},
    {COM_SAVEALL,
     "saveall",
     "Ninja command: save the entire MUD Universe - preserving connections.",
     "Ninja command: save the entire MUD Universe - preserving connections.",
     (REQ_ANY | REQ_NINJAMODE)},

    {COM_MAKESTART,
     "makestart",
     "Ninja command.",
     "Ninja command - ninjas only!",
     (REQ_ALERT | REQ_NINJAMODE)},

    {COM_TLOAD,
     "tload",
     "Ninja command.",
     "Ninja command - ninjas only!",
     (REQ_ALERT | REQ_NINJAMODE | CMD_FLAVORTEXT)},
    {COM_TCLEAN,
     "tclean",
     "Ninja command.",
     "Ninja command - ninjas only!",
     (REQ_ALERT | REQ_NINJAMODE)},
};

static_assert(static_comlist[COM_HELP].id == COM_HELP);
static_assert(static_comlist[COM_QUIT].id == COM_QUIT);
static_assert(static_comlist[COM_NORTH].id == COM_NORTH);
static_assert(static_comlist[COM_SOUTH].id == COM_SOUTH);
static_assert(static_comlist[COM_EAST].id == COM_EAST);
static_assert(static_comlist[COM_WEST].id == COM_WEST);
static_assert(static_comlist[COM_UP].id == COM_UP);
static_assert(static_comlist[COM_DOWN].id == COM_DOWN);
static_assert(static_comlist[COM_LOOK].id == COM_LOOK);
static_assert(static_comlist[COM_EXAMINE].id == COM_EXAMINE);
static_assert(static_comlist[COM_CONSIDER].id == COM_CONSIDER);
static_assert(static_comlist[COM_INVENTORY].id == COM_INVENTORY);
static_assert(static_comlist[COM_EQUIPMENT].id == COM_EQUIPMENT);
static_assert(static_comlist[COM_SEARCH].id == COM_SEARCH);
static_assert(static_comlist[COM_HIDE].id == COM_HIDE);
static_assert(static_comlist[COM_LEAVE].id == COM_LEAVE);
static_assert(static_comlist[COM_ENTER].id == COM_ENTER);
static_assert(static_comlist[COM_SELECT].id == COM_SELECT);
static_assert(static_comlist[COM_OPEN].id == COM_OPEN);
static_assert(static_comlist[COM_CLOSE].id == COM_CLOSE);
static_assert(static_comlist[COM_UNLOCK].id == COM_UNLOCK);
static_assert(static_comlist[COM_LOCK].id == COM_LOCK);
static_assert(static_comlist[COM_GET].id == COM_GET);
static_assert(static_comlist[COM_DRAG].id == COM_DRAG);
static_assert(static_comlist[COM_PUT].id == COM_PUT);
static_assert(static_comlist[COM_DROP].id == COM_DROP);
static_assert(static_comlist[COM_STASH].id == COM_STASH);
static_assert(static_comlist[COM_WIELD].id == COM_WIELD);
static_assert(static_comlist[COM_UNWIELD].id == COM_UNWIELD);
static_assert(static_comlist[COM_HOLD].id == COM_HOLD);
static_assert(static_comlist[COM_LIGHT].id == COM_LIGHT);
static_assert(static_comlist[COM_WEAR].id == COM_WEAR);
static_assert(static_comlist[COM_REMOVE].id == COM_REMOVE);
static_assert(static_comlist[COM_LABEL].id == COM_LABEL);
static_assert(static_comlist[COM_UNLABEL].id == COM_UNLABEL);
static_assert(static_comlist[COM_HEAL].id == COM_HEAL);
static_assert(static_comlist[COM_EAT].id == COM_EAT);
static_assert(static_comlist[COM_DRINK].id == COM_DRINK);
static_assert(static_comlist[COM_FILL].id == COM_FILL);
static_assert(static_comlist[COM_DUMP].id == COM_DUMP);
static_assert(static_comlist[COM_SLEEP].id == COM_SLEEP);
static_assert(static_comlist[COM_WAKE].id == COM_WAKE);
static_assert(static_comlist[COM_LIE].id == COM_LIE);
static_assert(static_comlist[COM_REST].id == COM_REST);
static_assert(static_comlist[COM_SIT].id == COM_SIT);
static_assert(static_comlist[COM_STAND].id == COM_STAND);
static_assert(static_comlist[COM_USE].id == COM_USE);
static_assert(static_comlist[COM_STOP].id == COM_STOP);
static_assert(static_comlist[COM_CAST].id == COM_CAST);
static_assert(static_comlist[COM_PRAY].id == COM_PRAY);
static_assert(static_comlist[COM_SHOUT].id == COM_SHOUT);
static_assert(static_comlist[COM_YELL].id == COM_YELL);
static_assert(static_comlist[COM_CALL].id == COM_CALL);
static_assert(static_comlist[COM_SAY].id == COM_SAY);
static_assert(static_comlist[COM_EMOTE].id == COM_EMOTE);
static_assert(static_comlist[COM_POINT].id == COM_POINT);
static_assert(static_comlist[COM_FOLLOW].id == COM_FOLLOW);
static_assert(static_comlist[COM_ATTACK].id == COM_ATTACK);
static_assert(static_comlist[COM_KILL].id == COM_KILL);
static_assert(static_comlist[COM_PUNCH].id == COM_PUNCH);
static_assert(static_comlist[COM_KICK].id == COM_KICK);
static_assert(static_comlist[COM_FLEE].id == COM_FLEE);
static_assert(static_comlist[COM_LIST].id == COM_LIST);
static_assert(static_comlist[COM_BUY].id == COM_BUY);
static_assert(static_comlist[COM_VALUE].id == COM_VALUE);
static_assert(static_comlist[COM_SELL].id == COM_SELL);
static_assert(static_comlist[COM_TOGGLE].id == COM_TOGGLE);
static_assert(static_comlist[COM_WHO].id == COM_WHO);
static_assert(static_comlist[COM_OOC].id == COM_OOC);
static_assert(static_comlist[COM_NEWBIE].id == COM_NEWBIE);
static_assert(static_comlist[COM_NEWCHARACTER].id == COM_NEWCHARACTER);
static_assert(static_comlist[COM_RAISE].id == COM_RAISE);
static_assert(static_comlist[COM_LOWER].id == COM_LOWER);
static_assert(static_comlist[COM_RESETCHARACTER].id == COM_RESETCHARACTER);
static_assert(static_comlist[COM_RANDOMIZE].id == COM_RANDOMIZE);
static_assert(static_comlist[COM_ARCHTYPE].id == COM_ARCHTYPE);
static_assert(static_comlist[COM_SCORE].id == COM_SCORE);
static_assert(static_comlist[COM_TIME].id == COM_TIME);
static_assert(static_comlist[COM_WORLD].id == COM_WORLD);
static_assert(static_comlist[COM_VERSION].id == COM_VERSION);
static_assert(static_comlist[COM_SKILLLIST].id == COM_SKILLLIST);
static_assert(static_comlist[COM_RECALL].id == COM_RECALL);
static_assert(static_comlist[COM_TELEPORT].id == COM_TELEPORT);
static_assert(static_comlist[COM_RESURRECT].id == COM_RESURRECT);
static_assert(static_comlist[COM_NINJAMODE].id == COM_NINJAMODE);
static_assert(static_comlist[COM_MAKENINJA].id == COM_MAKENINJA);
static_assert(static_comlist[COM_MAKESUPERNINJA].id == COM_MAKESUPERNINJA);
static_assert(static_comlist[COM_CREATE].id == COM_CREATE);
static_assert(static_comlist[COM_DCREATE].id == COM_DCREATE);
static_assert(static_comlist[COM_CCREATE].id == COM_CCREATE);
static_assert(static_comlist[COM_ANCHOR].id == COM_ANCHOR);
static_assert(static_comlist[COM_LINK].id == COM_LINK);
static_assert(static_comlist[COM_CONNECT].id == COM_CONNECT);
static_assert(static_comlist[COM_COMMAND].id == COM_COMMAND);
static_assert(static_comlist[COM_CONTROL].id == COM_CONTROL);
static_assert(static_comlist[COM_CLONE].id == COM_CLONE);
static_assert(static_comlist[COM_MIRROR].id == COM_MIRROR);
static_assert(static_comlist[COM_JUNK].id == COM_JUNK);
static_assert(static_comlist[COM_RESET].id == COM_RESET);
static_assert(static_comlist[COM_PLAYERS].id == COM_PLAYERS);
static_assert(static_comlist[COM_DELPLAYER].id == COM_DELPLAYER);
static_assert(static_comlist[COM_CHARACTERS].id == COM_CHARACTERS);
static_assert(static_comlist[COM_JACK].id == COM_JACK);
static_assert(static_comlist[COM_CHUMP].id == COM_CHUMP);
static_assert(static_comlist[COM_INCREMENT].id == COM_INCREMENT);
static_assert(static_comlist[COM_DECREMENT].id == COM_DECREMENT);
static_assert(static_comlist[COM_DOUBLE].id == COM_DOUBLE);
static_assert(static_comlist[COM_SETSTATS].id == COM_SETSTATS);
static_assert(static_comlist[COM_NAME].id == COM_NAME);
static_assert(static_comlist[COM_UNDESCRIBE].id == COM_UNDESCRIBE);
static_assert(static_comlist[COM_DESCRIBE].id == COM_DESCRIBE);
static_assert(static_comlist[COM_UNDEFINE].id == COM_UNDEFINE);
static_assert(static_comlist[COM_DEFINE].id == COM_DEFINE);
static_assert(static_comlist[COM_STATS].id == COM_STATS);
static_assert(static_comlist[COM_SHUTDOWN].id == COM_SHUTDOWN);
static_assert(static_comlist[COM_RESTART].id == COM_RESTART);
static_assert(static_comlist[COM_SAVEALL].id == COM_SAVEALL);
static_assert(static_comlist[COM_MAKESTART].id == COM_MAKESTART);
static_assert(static_comlist[COM_TLOAD].id == COM_TLOAD);
static_assert(static_comlist[COM_TCLEAN].id == COM_TCLEAN);

Command comlist[1024] = {};

const char* socials[1024][13] = {};

static void load_commands() {
  int cnum = 1;
  while (cnum < COM_MAX) {
    comlist[cnum] = static_comlist[cnum];
    ++cnum;
  }
  FILE* soc = fopen("tba/socials.new", "r");
  if (soc) {
    // fprintf(stderr, "There were %d commands!\n", cnum);
    char com[64] = "";
    int v1, v2, v3, v4;
    while (fscanf(soc, " ~%s %*s %d %d %d %d", com, &v1, &v2, &v3, &v4) == 5) {
      char buf[256] = "";
      for (int mnum = 0; mnum < 13; ++mnum) {
        fscanf(soc, " %255[^\n\r]", buf); // Skip space/newline, read line.
        fscanf(soc, "%*[^\n\r]"); // Skip rest of line.
        if (strstr(buf, "#"))
          socials[cnum][mnum] = "";
        else
          socials[cnum][mnum] = strdup(buf);
      }
      comlist[cnum].command = strdup(com);
      comlist[cnum].id = COM_SOCIAL;
      comlist[cnum].shortdesc = "Social command.";
      comlist[cnum].longdesc = "Social command.";
      comlist[cnum].sit = (REQ_ALERT | CMD_FLAVORTEXT); // FIXME: Import This?
      ++cnum;
    }
    // fprintf(stderr, "There are now %d commands!\n", cnum);
    fclose(soc);
  }
}

com_t identify_command(const std::string& str) {
  int len;
  if (comlist[COM_MAX].id == COM_NONE) { // Haven't loaded commands yet
    load_commands();
  }
  if (str[0] == '\'' || str[0] == '"') { // Command Alias: "say"
    len = 1;
  } else {
    for (len = 0; isgraph(str[len]); ++len)
      ;
  }
  if (len == 0)
    return COM_NONE;
  for (int ctr = 1; comlist[ctr].id != COM_NONE; ++ctr) {
    if (comlist[ctr].sit & SIT_NINJAMODE)
      continue; // Don't match ninjas

    if (!strncmp(str.c_str(), comlist[ctr].command, len)) {
      return comlist[ctr].id;
    }
    // Command Aliases
    if (comlist[ctr].id == COM_SAY && (str[0] == '\'' || str[0] == '"')) {
      return comlist[ctr].id;
    }
    if (comlist[ctr].id == COM_DUMP && (!strncmp(str.c_str(), "empty", std::max(len, 3)))) {
      return comlist[ctr].id;
    }
    if (comlist[ctr].id == COM_GET && (!strncmp(str.c_str(), "take", std::max(len, 1)))) {
      return comlist[ctr].id;
    }
  }
  return COM_NONE;
}

// Return values: -1: Player D/Ced
//                0: Command Understood
//                1: Command NOT Understood
//                2: Command Understood - No More Actions This Round
int handle_single_command(Object* body, const char* inpline, Mind* mind) {
  if (comlist[COM_MAX].id == COM_NONE) { // Haven't loaded commands yet
    load_commands();
  }

  std::string cmd = inpline;
  trim_string(cmd);

  // Lowercase the command portion, and only that portion, for now.
  auto clen = cmd.find_first_not_of("abcdefghijklmnopqrstuvwxyz");
  if (clen != std::string::npos && clen < cmd.length() && std::isupper(cmd[clen])) {
    clen = cmd.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
    if (clen == std::string::npos || clen >= cmd.length()) {
      std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);
    } else {
      std::transform(cmd.begin(), cmd.begin() + clen, cmd.begin(), ::tolower);
    }
  }

  if (comlist[COM_MAX].id == COM_NONE) { // Haven't loaded commands yet
    load_commands();
  }

  if ((!body) && (!mind)) { // Nobody doing something?
    fprintf(stderr, "Warning: absolutely nobody tried to '%s'.\n", cmd.c_str());
    return 0;
  }

  if (cmd.empty() || cmd.front() == '#')
    return 0;

  if (mind && mind->Type() == MIND_REMOTE && (!mind->Owner())) {
    if (mind->PName() != "")
      mind->SetPPass(cmd);
    else {
      int ctr;
      for (ctr = 0; ctr < int(cmd.length()); ++ctr) {
        if (!(isalnum(cmd[ctr]) || cmd[ctr] == ' ')) {
          mind->SendF(
              "Name '%s' is invalid.\nNames can only have letters, "
              "numbers, and spaces.\n",
              cmd.c_str());
          fprintf(
              stderr,
              "Name '%s' is invalid.\nNames can only have letters, "
              "numbers, and spaces.\n",
              cmd.c_str());
          break;
        }
      }
      if (ctr == int(cmd.length()))
        mind->SetPName(cmd.c_str());
    }
    return 0;
  }

  //  if(body && body->Matches("Glumgold")) {
  //    fprintf(stderr, "Handling command: '%s' (%d)\n",
  //	cmd.c_str(), body->StillBusy());
  //    }

  int len;
  if (cmd.front() == '\'' || cmd.front() == '"') { // Command Alias: "say"
    len = 1;
  } else {
    for (len = 0; isgraph(cmd[len]); ++len)
      ;
  }

  if (len == 0)
    return 0;

  int ninja = 0, sninja = 0, nmode = 0, vmode = 0;

  if (mind && mind->Owner() && mind->Owner()->Is(PLAYER_SUPERNINJA)) {
    sninja = 1;
    ninja = 1;
  }
  if (mind && mind->Owner() && mind->Owner()->Is(PLAYER_NINJA))
    ninja = 1;
  if (mind && mind->Owner() && mind->Owner()->Is(PLAYER_NINJAMODE)) {
    nmode = LOC_NINJA;
    vmode |= LOC_NINJA;
  }
  if (body && body->Power("Dark Vision Spell")) {
    vmode |= LOC_DARK;
  }
  if (body && body->Power("Heat Vision Spell")) {
    vmode |= LOC_HEAT;
  }

  int com = COM_NONE, cnum = -1;
  for (int ctr = 1; comlist[ctr].id != COM_NONE; ++ctr) {
    // Always match ninjas last (and only in ninja mode)
    if (comlist[ctr].sit & SIT_NINJAMODE)
      continue;

    if (!strncmp(cmd.c_str(), comlist[ctr].command, len)) {
      com = comlist[ctr].id;
      cnum = ctr;
      break;
    }

    // Command Aliases
    if (comlist[ctr].id == COM_SAY && (cmd.front() == '\'' || cmd.front() == '"')) {
      com = comlist[ctr].id;
      cnum = ctr;
      break;
    }
    if (comlist[ctr].id == COM_DUMP && (!strncmp(cmd.c_str(), "empty", std::max(len, 3)))) {
      com = comlist[ctr].id;
      cnum = ctr;
      break;
    }
    if (comlist[ctr].id == COM_GET && (!strncmp(cmd.c_str(), "take", std::max(len, 1)))) {
      com = comlist[ctr].id;
      cnum = ctr;
      break;
    }
  }

  if (com == COM_NONE && ninja) { // Now match ninja commands (for ninjas)
    for (int ctr = 1; comlist[ctr].id != COM_NONE; ++ctr) {
      if (!strncmp(cmd.c_str(), comlist[ctr].command, len)) {
        com = comlist[ctr].id;
        cnum = ctr;
        break;
      }
      if (comlist[ctr].id == COM_CHARACTERS && (!strncmp(cmd.c_str(), "chars", std::max(len, 5)))) {
        com = comlist[ctr].id;
        cnum = ctr;
        break;
      }
    }
  }

  // Lowercase the entire command, for non-flavortext commands.
  if (com != COM_NONE && (comlist[com].sit & CMD_FLAVORTEXT) == 0) {
    auto fclen = cmd.find_first_of("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    if (fclen != std::string::npos && fclen < cmd.length()) {
      std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);
    }
  }

  cmd = cmd.substr(len);
  trim_string(cmd);

  if ((!nmode) && com != COM_RECALL && body && body->Parent()) {
    std::vector<Object*> items;
    Object* room = body->PickObject("here", LOC_HERE);
    auto items2 = body->PickObjects("everything", LOC_INTERNAL | LOC_NEARBY);
    auto items3 = body->PickObjects("everyone", LOC_NEARBY);
    items.reserve(items2.size() + items3.size() + (room != nullptr));
    if (room != nullptr)
      items.push_back(room);
    items.insert(items.end(), items2.begin(), items2.end());
    items.insert(items.end(), items3.begin(), items3.end());

    for (auto obj : items) {
      auto trigs = obj->PickObjects("all tbaMUD trigger script", LOC_NINJA | LOC_INTERNAL);
      for (auto trig : trigs) {
        if (trig->Skill("TBAScriptType") & 0x04) { //*-COMMAND trigs
          if ((com == COM_NONE && (!strncmp(inpline, trig->Desc(), len))) ||
              (com && com == identify_command(trig->Desc()))) {
            if (trig->Skill("TBAScriptType") & 0x2000000) { // OBJ
              int narg = trig->Skill("TBAScriptNArg");
              if ((narg & 3) == 0 && body->HasWithin(obj)) {
                continue;
              } else if ((narg & 3) == 2 && body->Wearing(obj)) {
                continue;
              } else if ((narg & 3) == 1 && (!body->Wearing(obj))) {
                continue;
              }
              if ((narg & 4) == 0 && (!body->HasWithin(obj))) {
                continue;
              }
            }
            std::string cmln;
            if (com != COM_NONE) {
              cmln = comlist[cnum].command + std::string(" ") + cmd;
            } else {
              cmln = trig->Desc() + std::string(" ") + cmd;
            }
            if (!new_trigger(0, trig, body, cmln)) {
              return 0; // Handled, unless script says not.
            }
          }
        }
      }
    }
  }
  if (com == COM_NONE) { // Unknown, and not trigger-supported, command
    if (mind)
      mind->Send("Command NOT understood - type 'help' for assistance.\n");
    return 1;
  }

  len = 0;

  if ((!sninja) && (comlist[cnum].sit & SIT_SUPERNINJA)) {
    if (mind)
      mind->Send("Sorry, that command is for Super Ninjas only!\n");
    return 0;
  }

  if ((!ninja) && (comlist[cnum].sit & SIT_NINJA)) {
    if (mind)
      mind->Send("Sorry, that command is for True Ninjas only!\n");
    return 0;
  }

  if ((!nmode) && (comlist[cnum].sit & SIT_NINJAMODE)) {
    if (mind && ninja)
      mind->Send("Sorry, you need to be in Ninja Mode[TM] to do that.\n");
    else if (mind)
      mind->Send("Sorry, that command is for Ninjas only!\n");
    return 0;
  }

  if ((!(comlist[cnum].sit & SIT_ETHEREAL)) && (!body)) {
    if (mind)
      mind->Send(
          "You can't use that command until you join the game - with "
          "the 'enter' command.\n");
    return 0;
  }

  if ((!(comlist[cnum].sit & SIT_CORPOREAL)) && (body)) {
    if (mind)
      mind->Send(
          "You can't use that command until you quit the game - with "
          "the 'quit' command.\n");
    return 0;
  }

  if (body) {
    if (body->StillBusy() && (comlist[cnum].sit & SIT_ACTION)) {
      body->DoWhenFree(inpline);
      return 0;
    }
    if (comlist[cnum].sit & (SIT_ALIVE | SIT_AWAKE | SIT_ALERT)) {
      if (body->IsAct(ACT_DYING) || body->IsAct(ACT_DEAD)) {
        if (mind)
          mind->Send("You must be alive to use that command.\n");
        return 0;
      }
    }
    if (body && (comlist[cnum].sit & SIT_CONSCIOUS)) {
      if (body->IsAct(ACT_UNCONSCIOUS)) {
        if (mind)
          mind->Send("You can't do that, you are out cold.\n");
        return 0;
      }
    }
    if ((comlist[cnum].sit & (SIT_STAND | SIT_USE)) == (SIT_STAND | SIT_USE)) {
      if (body->Pos() != POS_STAND && body->Pos() != POS_USE) {
        if (mind)
          mind->Send("You must stand up to use that command.\n");
        handle_single_command(body, "stand", mind);
        if (body->Pos() != POS_STAND && body->Pos() != POS_USE)
          return 0;
      }
    } else if ((comlist[cnum].sit & (SIT_STAND | SIT_SIT)) == (SIT_STAND | SIT_SIT)) {
      if (body->Pos() == POS_USE) {
        if (mind)
          mind->Send("You must stop using this skill to do that.\n");
        handle_single_command(body, "stop", mind);
        if (body->Pos() != POS_STAND)
          return 0;
      } else if (body->Pos() != POS_SIT && body->Pos() != POS_STAND) {
        if (mind)
          mind->Send("You must at least sit up to use that command.\n");
        handle_single_command(body, "sit", mind);
        if (body->Pos() != POS_SIT && body->Pos() != POS_STAND)
          return 0;
      }
    } else if (comlist[cnum].sit & SIT_STAND) {
      if (body->Pos() == POS_USE) {
        if (mind)
          mind->Send("You must stop using this skill to do that.\n");
        handle_single_command(body, "stop", mind);
        if (body->Pos() != POS_STAND)
          return 0;
      } else if (body->Pos() != POS_STAND) {
        if (mind)
          mind->Send("You must stand up to use that command.\n");
        handle_single_command(body, "stand", mind);
        if (body->Pos() != POS_STAND)
          return 0;
      }
    }
    if (comlist[cnum].sit & SIT_SIT) {
      if (body->Pos() != POS_SIT) {
        if (mind)
          mind->Send("You must sit to use that command.\n");
        handle_single_command(body, "sit", mind);
        if (body->Pos() != POS_SIT)
          return 0;
      }
    }
    if (comlist[cnum].sit & SIT_ALERT) {
      if (body->IsAct(ACT_SLEEP)) {
        if (mind)
          mind->Send("You must be awake to use that command.\n");
        handle_single_command(body, "wake", mind);
        if (body->IsAct(ACT_SLEEP))
          return 0;
      }
      if (body->IsAct(ACT_REST)) {
        if (mind)
          mind->Send("You must be alert to use that command.\n");
        handle_single_command(body, "rest", mind);
        if (body->IsAct(ACT_REST))
          return 0;
      }
    }
    if (comlist[cnum].sit & SIT_AWAKE) {
      if (body->IsAct(ACT_SLEEP)) {
        if (mind)
          mind->Send("You must be awake to use that command.\n");
        handle_single_command(body, "wake", mind);
        if (body->IsAct(ACT_SLEEP))
          return 0;
      }
    }
  }

  int stealth_t = 0, stealth_s = 0;

  if (body && body->IsUsing("Stealth") && body->Skill("Stealth") > 0) {
    stealth_t = body->Skill("Stealth");
    stealth_s = body->Roll("Stealth", 2);
  }

  if (com == COM_VERSION) {
    if (mind)
      mind->SendF(
          "Version of this MUD is %d.%d.%d-%d: %s.\n",
          CurrentVersion.acidmud_version[0],
          CurrentVersion.acidmud_version[1],
          CurrentVersion.acidmud_version[2],
          CurrentVersion.acidmud_svnstamp,
          CurrentVersion.acidmud_datestamp);
    return 0;
  }

  if (com == COM_SHUTDOWN) {
    shutdn = 1;
    if (mind)
      mind->Send("You instruct the system to shut down.\n");
    return 0;
  }

  if (com == COM_RESTART) {
    shutdn = 2;
    if (mind)
      mind->Send("You instruct the system to restart.\n");
    return 0;
  }

  if (com == COM_SAVEALL) {
    shutdn = -1;
    if (mind)
      mind->Send("You instruct the system to save all.\n");
    return 0;
  }

  if (com == COM_SELECT) {
    Object* sel = mind->Owner()->Room()->PickObject(cmd.c_str() + len, vmode | LOC_INTERNAL);
    if (!sel) {
      mind->Send(
          "Sorry, that character doesn't exist.\n"
          "Use the 'newcharacter' command to create a new character.\n");
      return 0;
    } else {
      mind->SendF("'%s' is now selected as your currect character to work on.\n", sel->Name());
      mind->Owner()->SetCreator(sel);
      return 0;
    }
  }

  if (com == COM_NORTH) {
    com = COM_ENTER;
    len = 6;
    cmd = "enter north";
  }
  if (com == COM_SOUTH) {
    com = COM_ENTER;
    len = 6;
    cmd = "enter south";
  }
  if (com == COM_EAST) {
    com = COM_ENTER;
    len = 6;
    cmd = "enter east";
  }
  if (com == COM_WEST) {
    com = COM_ENTER;
    len = 6;
    cmd = "enter west";
  }
  if (com == COM_UP) {
    com = COM_ENTER;
    len = 6;
    cmd = "enter up";
  }
  if (com == COM_DOWN) {
    com = COM_ENTER;
    len = 6;
    cmd = "enter down";
  }

  if (com == COM_FLEE) {
    auto dirs = body->PickObjects("everywhere", vmode | LOC_NEARBY);
    dirs.erase(
        std::remove_if(
            dirs.begin(), dirs.end(), [](const Object* o) { return o->Skill("Open") < 1; }),
        dirs.end());
    if (dirs.size() < 1) {
      if (mind)
        mind->Send("There is nowhere go, you can't flee!\n");
      return 0;
    }

    body->StartUsing("Sprinting");
    body->SetSkill("Hidden", 0);

    auto dir = dirs.begin();
    int sel = rand() % dirs.size();
    while (sel > 0) {
      ++dir;
      --sel;
    }
    if (mind)
      mind->SendF("You try to flee %s.\n", (*dir)->ShortDesc());

    com = COM_ENTER;
    cmd = (*dir)->ShortDesc();
    len = 0;
  }

  if (com == COM_ENTER) {
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;
    if (!body) { // Implies that there is a "mind"
      if (len >= int(cmd.length())) {
        mind->Send("Enter which character?  Use 'enter <charname>'.\n");
        return 0;
      }
      if (!mind->Owner()) { // The Autoninja (Initial Startup)
        Object* god = new_body();
        god->SetShortDesc(cmd.c_str() + len);
        mind->Attach(god);
        return 0;
      }

      body = mind->Owner()->Room()->PickObject(cmd.c_str() + len, vmode | LOC_INTERNAL);
      if (!body) {
        mind->Send(
            "Sorry, that character doesn't exist.\n"
            "Use the 'newcharacter' command to create a new character.\n");
        return 0;
      }
      if (body->Skill("Attribute Points") > 0 || body->Skill("Skill Points") > 0) {
        mind->Send("You need to finish that character before you can use it.\n");
        mind->SendF("'%s' is now selected as your currect character to work on.\n", body->Name());
        mind->Owner()->SetCreator(body);
        return 0;
      }
      if ((!nmode) && body->IsAct(ACT_DEAD)) { // Ninjas can autoheal
        mind->Send(
            "Sorry, that character is dead.\n"
            "Use the 'newcharacter' command to create a new character.\n");
        return 0;
      }

      // FIXME: Handle conversion of body->Skill("Resources").
      if (mind->Owner()->Creator() == body)
        mind->Owner()->SetCreator(nullptr);

      mind->Attach(body);

      if (nmode) {
        // This is ninja-healing and bypasses all healing mechanisms.
        body->SetSkill("Poisoned", 0);
        body->SetSkill("Thirsty", 0);
        body->SetSkill("Hungry", 0);
        body->SetStun(0);
        body->SetPhys(0);
        body->SetStru(0);
        body->UpdateDamage();
        body->Parent()->SendOut(
            stealth_t,
            stealth_s,
            ";s heals and repairs ;s with Ninja Powers[TM].\n",
            "You heal ;s.\n",
            body,
            body);
      }

      if (body->IsAct(ACT_DYING)) {
        mind->Send("You can see nothing, you are too busy dying.\n");
      } else if (body->IsAct(ACT_UNCONSCIOUS)) {
        mind->Send("You can see nothing, you are out cold.\n");
      } else if (body->IsAct(ACT_SLEEP)) {
        mind->Send("You can see nothing since you are asleep.\n");
      } else {
        body->Parent()->SendDescSurround(mind, body);
      }
      mind->SendF(CMAG "You have entered: %s\n" CNRM, body->World()->ShortDesc());
      return 0;
    }
    Object* dest = body->PickObject(cmd.c_str() + len, vmode | LOC_NEARBY);
    Object* rdest = dest;
    Object* veh = body;

    if (!dest) {
      if (mind)
        mind->Send("You want to go where?\n");
      return 0;
    }

    if (dest->ActTarg(ACT_SPECIAL_LINKED) && dest->ActTarg(ACT_SPECIAL_LINKED)->Parent()) {
      rdest = dest->ActTarg(ACT_SPECIAL_LINKED)->Parent();
    }

    if ((!dest->Skill("Enterable")) && (!ninja)) {
      if (mind)
        mind->Send("It is not possible to enter that object!\n");
    } else if ((!dest->Skill("Enterable")) && (!nmode)) {
      if (mind)
        mind->Send("You need to be in ninja mode to enter that object!\n");
    } else if (dest->Skill("Open") < 1 && (!nmode)) {
      if (mind)
        mind->SendF("Sorry, %s is closed!\n", dest->Name());
    } else if (
        dest->Parent() != body->Parent() && dest->Parent() == body->Parent()->Parent() &&
        body->Parent()->Skill("Vehicle") == 0) {
      if (mind)
        mind->SendF("You can't get %s to go there!\n", body->Parent()->Name(1));
    } else if (
        dest->Parent() != body->Parent() && dest->Parent() == body->Parent()->Parent() &&
        (!(body->Parent()->Skill("Vehicle") & 0xFFF0)) // No Land Travel!
        && body->Parent()->Parent()->Skill("WaterDepth") == 0 && rdest->Skill("WaterDepth") == 0) {
      if (mind)
        mind->SendF("You can't get %s to go there!\n", body->Parent()->Name(1));
    } else {
      if (nmode) {
        // Ninja-movement can't be followed!
        if (body->Parent())
          body->Parent()->NotifyGone(body);
      }
      if (dest->Parent() != body->Parent() && dest->Parent() == body->Parent()->Parent()) {
        if (body->Parent()->Skill("Vehicle") == 4 && body->Skill("Boat") == 0) {
          if (mind)
            mind->SendF("You don't know how to operate %s!\n", body->Parent()->Name(1));
          return 0;
        }
        veh = body->Parent();
      }

      if (rdest->Skill("WaterDepth") == 1 && body->Skill("Swimming") == 0) {
        if (veh == body || (veh->Skill("Vehicle") & 4) == 0) { // Have boat?
          if (mind)
            mind->Send("Sorry, but you can't swim!\n");
          return 0;
        }
      } else if (rdest->Skill("WaterDepth") > 1) {
        if (veh == body || (veh->Skill("Vehicle") & 4) == 0) { // Have boat?
          if (mind)
            mind->Send("Sorry, you need a boat to go there!\n");
          return 0;
        }
      }

      int newworld = (body->World() != rdest->World());
      if (dest->ActTarg(ACT_SPECIAL_LINKED) && dest->ActTarg(ACT_SPECIAL_LINKED)->Parent()) {
        body->Parent()->SendOut(stealth_t, stealth_s, ";s leaves ;s.\n", "", body, dest);
        dest = dest->ActTarg(ACT_SPECIAL_LINKED)->Parent();
      } else if (body->Parent()) {
        body->Parent()->SendOut(stealth_t, stealth_s, ";s enters ;s.\n", "", body, dest);
      }

      int reas = 0;
      if ((!nmode) && (reas = veh->Travel(dest))) {
        if (reas < 0) { // If it's not a script-prevent (which handles alert)
          body->Parent()->SendOut(
              stealth_t,
              stealth_s,
              "...but ;s didn't seem to fit!\n",
              "You could not fit!\n",
              body,
              nullptr);
        }
      } else {
        if (nmode) {
          body->Parent()->RemoveLink(body);
          body->SetParent(dest);
        }
        body->Parent()->SendOut(stealth_t, stealth_s, ";s arrives.\n", "", body, nullptr);
        if (mind && (vmode & (LOC_NINJA | LOC_DARK)) == 0 && body->Parent()->LightLevel() < 100) {
          mind->Send("It's too dark, you can't see anything.\n");
        } else if (mind && mind->Type() == MIND_REMOTE) {
          body->Parent()->SendDescSurround(body, body, vmode);
        } else if (mind && mind->Type() == MIND_SYSTEM) {
          mind->SendF("You enter %s\n", cmd.c_str() + len);
        }

        if (mind && newworld) {
          mind->SendF(CMAG "You have entered: %s\n" CNRM, body->World()->ShortDesc());
        }
        if (stealth_t > 0) {
          body->SetSkill("Hidden", body->Roll("Stealth", 2) * 2);
        }
        if (body->Roll("Running", 2) < 1) { // FIXME: Terrain/Direction Mods?
          if (mind) {
            mind->Send(CRED "\nYou are winded, and have to catch your breath." CNRM
                            "  Raise the " CMAG "Running" CNRM " skill.\n");
          }
          body->BusyFor(3000);
        }
      }
    }
    return 0;
  }

  if (com == COM_QUIT) {
    if (!body) {
      delete mind;
      return -1; // Player Disconnected
    }
    // if(body) delete body;
    if (mind)
      mind->Unattach();

    if (mind && mind->Owner() && mind->Owner()->Room()) {
      mind->Owner()->Room()->SendDesc(mind);
      mind->Owner()->Room()->SendContents(mind);
    } else if (mind)
      mind->Send("Use 'Enter' to return to the game.\n");

    return 0;
  }

  if (com == COM_HELP) {
    if ((!mind) || (!mind->Owner()))
      return 0;
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;
    if (!strcmp(cmd.c_str() + len, "commands")) {
      std::string mes = "";
      for (int ctr = 1; comlist[ctr].id != COM_NONE; ++ctr) {
        if (comlist[ctr].id == COM_SOCIAL)
          continue;
        if ((comlist[ctr].sit & SIT_NINJAMODE) && (!nmode))
          continue;
        if ((!(comlist[ctr].sit & SIT_NINJAMODE)) && nmode)
          continue;
        if ((comlist[ctr].sit & SIT_NINJA) && (!ninja))
          continue;
        if ((comlist[ctr].sit & SIT_SUPERNINJA) && (!sninja))
          continue;
        if ((!(comlist[ctr].sit & SIT_CORPOREAL)) && (body))
          continue;
        if ((!(comlist[ctr].sit & SIT_ETHEREAL)) && (!body))
          continue;
        mes += comlist[ctr].command;
        mes += " - ";
        mes += comlist[ctr].shortdesc;
        mes += '\n';
      }
      mind->Send(mes.c_str());
      return 0;
    } else if (!strcmp(cmd.c_str() + len, "socials")) {
      std::string mes = "";
      for (int ctr = 1; comlist[ctr].id != COM_NONE; ++ctr) {
        if (comlist[ctr].id != COM_SOCIAL)
          continue;
        if ((comlist[ctr].sit & SIT_NINJAMODE) && (!nmode))
          continue;
        if ((!(comlist[ctr].sit & SIT_NINJAMODE)) && nmode)
          continue;
        if ((comlist[ctr].sit & SIT_NINJA) && (!ninja))
          continue;
        if ((comlist[ctr].sit & SIT_SUPERNINJA) && (!sninja))
          continue;
        if ((!(comlist[ctr].sit & SIT_CORPOREAL)) && (body))
          continue;
        if ((!(comlist[ctr].sit & SIT_ETHEREAL)) && (!body))
          continue;
        mes += comlist[ctr].command;
        mes += " - ";
        mes += comlist[ctr].shortdesc;
        mes += '\n';
      }
      mind->Send(mes.c_str());
      return 0;
    }
    mind->Send(
        "Well, the help command's not really implemented yet :P\n"
        "Try 'help commands' for a list of general commands.\n"
        "Try 'help socials' for a list of social commands.\n");
    return 0;
  }

  if (com == COM_SAY) {
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;
    if (strlen(cmd.c_str() + len) <= 0) {
      if (strncmp(mind->SpecialPrompt(), "say", 3)) {
        mind->SetSpecialPrompt("say");
        mind->Send("Type what your character will say - exit by just hitting ENTER:");
      } else {
        mind->SetSpecialPrompt("");
        mind->Send("Exiting out of say mode.");
      }
      return 0;
    } else {
      int shouting = 1;
      if (strlen(cmd.c_str() + len) < 4)
        shouting = 0;
      for (const char* chr = cmd.c_str() + len; shouting && *chr != 0; ++chr) {
        if (islower(*chr))
          shouting = 0;
      }
      if (!shouting) {
        body->Parent()->SendOutF(
            ALL, 0, ";s says '%s'\n", "You say '%s'\n", body, body, cmd.c_str() + len);
        body->SetSkill("Hidden", 0);
        return 0;
      } else {
        com = COM_SHOUT;
      }
    }
  }

  if (com == COM_SHOUT || com == COM_YELL || com == COM_CALL) {
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;
    if (strlen(cmd.c_str() + len) <= 0) {
      if (strncmp(mind->SpecialPrompt(), "shout", 3)) {
        mind->SetSpecialPrompt("shout");
        mind->Send(
            "Type what your character will shout - exit by just hitting "
            "ENTER:");
      } else {
        mind->SetSpecialPrompt("");
        mind->Send("Exiting out of shout mode.");
      }
    } else {
      if (!strncmp(cmd.c_str() + len, "for ", 4))
        len += 4;

      char* mes = strdup(cmd.c_str() + len);
      for (char* chr = mes; *chr != 0; ++chr) {
        *chr = toupper(*chr);
      }
      body->Parent()->SendOutF(
          ALL, 0, ";s shouts '%s'!!!\n", "You shout '%s'!!!\n", body, body, mes);
      body->Parent()->LoudF(body->Skill("Strength"), "someone shout '%s'!!!", mes);
      free(mes);
    }
    body->SetSkill("Hidden", 0);
    return 0;
  }

  if (com == COM_SOCIAL) {
    Object* targ = nullptr;
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;
    if (body && body->Parent()) {
      std::string youmes = socials[cnum][0];
      std::string outmes = socials[cnum][1];
      std::string targmes = "";
      if (strlen(cmd.c_str() + len) > 0) {
        targ = body->PickObject(cmd.c_str() + len, vmode | LOC_NEARBY | LOC_SELF | LOC_INTERNAL);
        if (!targ) {
          youmes = socials[cnum][5];
          outmes = "";
        } else if (targ == body) {
          youmes = socials[cnum][6];
          outmes = socials[cnum][7];
        } else {
          youmes = socials[cnum][2];
          outmes = socials[cnum][3];
          targmes = socials[cnum][4];
        }
      }
      if (outmes[0]) {
        replace_all(outmes, "$e", body->Pron());
        replace_all(outmes, "$m", body->Obje());
        replace_all(outmes, "$s", body->Poss());
        replace_all(outmes, "$n", body->Name(1));
        if (targ) {
          replace_all(outmes, "$E", targ->Pron());
          replace_all(outmes, "$M", targ->Obje());
          replace_all(outmes, "$S", targ->Poss());
          replace_all(outmes, "$N", targ->Name(1));
          replace_all(outmes, "$t", targ->Name(1));
          replace_all(outmes, "$p", targ->Name(1));
        }
        outmes[0] = toupper(outmes[0]);
        outmes += "\n";
      }
      if (youmes[0]) {
        replace_all(youmes, "$e", body->Pron());
        replace_all(youmes, "$m", body->Obje());
        replace_all(youmes, "$s", body->Poss());
        replace_all(youmes, "$n", body->Name(1));
        if (targ) {
          replace_all(youmes, "$E", targ->Pron());
          replace_all(youmes, "$M", targ->Obje());
          replace_all(youmes, "$S", targ->Poss());
          replace_all(youmes, "$N", targ->Name(1));
          replace_all(youmes, "$t", targ->Name(1));
          replace_all(youmes, "$p", targ->Name(1));
        }
        youmes[0] = toupper(youmes[0]);
        youmes += "\n";
      }
      if (targ) {
        replace_all(targmes, "$e", body->Pron());
        replace_all(targmes, "$m", body->Obje());
        replace_all(targmes, "$s", body->Poss());
        replace_all(targmes, "$n", body->Name(1));
        if (targ) {
          replace_all(targmes, "$E", targ->Pron());
          replace_all(targmes, "$M", targ->Obje());
          replace_all(targmes, "$S", targ->Poss());
          replace_all(targmes, "$N", targ->Name(1));
          replace_all(targmes, "$t", targ->Name(1));
          replace_all(targmes, "$p", targ->Name(1));
        }
        targmes[0] = toupper(targmes[0]);
        targmes += "\n";
        targ->Send(0, 0, targmes.c_str());
        targ->Deafen(1);
      }
      body->Parent()->SendOut(0, 0, outmes.c_str(), youmes.c_str(), body, targ);
      if (targ)
        targ->Deafen(0);
    }
    return 0;
  }

  if (com == COM_EMOTE) {
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;
    const char* dot = ".";
    if ((cmd.back() == '.') || (cmd.back() == '?') || (cmd.back() == '!')) {
      dot = "";
    }
    body->Parent()->SendOutF(
        ALL, 0, ";s %s%s\n", "Your character %s%s\n", body, body, cmd.c_str() + len, dot);
    body->SetSkill("Hidden", 0);
    return 0;
  }

  if (com == COM_INVENTORY) {
    if (mind) {
      mind->SendF("You (%s) are carrying:\n", body->ShortDesc());
      body->SendExtendedActions(mind, LOC_TOUCH | vmode | 1);
    }
    return 0;
  }

  if (com == COM_EQUIPMENT) {
    if (mind) {
      mind->SendF("You (%s) are using:\n", body->ShortDesc());
      body->SendExtendedActions(mind, LOC_TOUCH | vmode);
    }
    return 0;
  }

  if (com == COM_LOOK) {
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;
    if (!body) {
      mind->Owner()->Room()->SendDesc(mind);
      mind->Owner()->Room()->SendContents(mind, nullptr, vmode);
      return 0;
    }

    if (!body->Parent()) {
      return 0;
    }

    if (mind && (vmode & (LOC_NINJA | LOC_DARK)) == 0 && body->Parent()->LightLevel() < 100) {
      if (mind)
        mind->Send("It's too dark, you can't see anything.\n");
      return 0;
    }

    int within = 0;
    if (!strncmp(cmd.c_str() + len, "at ", 3))
      len += 3;
    if (!strncmp(cmd.c_str() + len, "in ", 3)) {
      len += 3;
      within = 1;
    }

    std::vector<Object*> targs;
    if (strlen(cmd.c_str() + len) > 0) {
      targs = body->PickObjects(
          cmd.c_str() + len, vmode | LOC_NEARBY | LOC_ADJACENT | LOC_SELF | LOC_INTERNAL);
    } else {
      targs.push_back(body->Parent());
    }

    if (targs.size() < 1) {
      if (mind)
        mind->Send("You don't see that here.\n");
      return 0;
    }

    for (auto targ : targs) {
      if (within && (!targ->Skill("Container")) && (!targ->Skill("Liquid Container"))) {
        if (mind)
          mind->SendF("You can't look inside %s, it is not a container.\n", targ->Name());
      } else if (within && (targ->Skill("Locked"))) {
        if (mind)
          mind->SendF("You can't look inside %s, it is locked.\n", targ->Name());
      } else {
        int must_open = within;
        if (within && targ->Skill("Open"))
          must_open = 0;

        if (must_open) {
          targ->SetSkill("Open", 1000);
          body->Parent()->SendOut(
              stealth_t, stealth_s, ";s opens ;s.\n", "You open ;s.\n", body, targ);
        }

        if (strlen(cmd.c_str() + len) <= 0) {
          body->Parent()->SendOut(stealth_t, stealth_s, ";s looks around.\n", "", body, targ);
          if (mind)
            targ->SendDescSurround(mind, body, vmode);
        } else if (
            (!strcmp(cmd.c_str() + len, "north")) || (!strcmp(cmd.c_str() + len, "south")) ||
            (!strcmp(cmd.c_str() + len, "east")) || (!strcmp(cmd.c_str() + len, "west")) ||
            (!strcmp(cmd.c_str() + len, "up")) || (!strcmp(cmd.c_str() + len, "down"))) {
          body->Parent()->SendOut(stealth_t, stealth_s, ";s looks ;s.\n", "", body, targ);
          if (mind) {
            targ->SendDesc(mind, body);
            targ->SendExtendedActions(mind, vmode);
          }
        } else if (within) {
          body->Parent()->SendOut(stealth_t, stealth_s, ";s looks inside ;s.\n", "", body, targ);
          if (mind) {
            targ->SendDesc(mind, body);
            targ->SendExtendedActions(mind, vmode);
            targ->SendContents(mind, nullptr, vmode);
          }
        } else {
          body->Parent()->SendOut(stealth_t, stealth_s, ";s looks at ;s.\n", "", body, targ);
          if (mind) {
            targ->SendDesc(mind, body);
            targ->SendExtendedActions(mind, vmode);
          }
        }

        if (must_open) {
          targ->SetSkill("Open", 0);
          body->Parent()->SendOut(
              stealth_t, stealth_s, ";s closes ;s.\n", "You close ;s.\n", body, targ);
        }
      }
    }

    if (targs.size() == 1 && targs.front() != body->Parent()) {
      targs.front()->TryCombine();
    }
    return 0;
  }

  if (com == COM_SEARCH) {
    if (!body->Parent())
      return 0;

    if (mind && (vmode & (LOC_NINJA | LOC_DARK)) == 0 && body->Parent()->LightLevel() < 100) {
      if (mind)
        mind->Send("It's too dark, you can't see anything.\n");
      return 0;
    }

    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;

    std::vector<Object*> targs;
    if (strlen(cmd.c_str() + len) > 0) {
      targs = body->PickObjects(
          cmd.c_str() + len, vmode | LOC_NEARBY | LOC_ADJACENT | LOC_SELF | LOC_INTERNAL);
      if (targs.size() == 0) {
        if (mind)
          mind->Send("You don't see that here.\n");
        return 0;
      }
    } else {
      targs.push_back(body->Parent());
    }

    stealth_t = 0;
    stealth_s = 0;
    if (body->Pos() == POS_USE && (!body->IsUsing("Perception"))) {
      body->Parent()->SendOutF(
          stealth_t,
          stealth_s,
          ";s stops %s.\n",
          "You stop %s.\n",
          body,
          nullptr,
          body->UsingString());
    }
    body->StartUsing("Perception");
    body->SetSkill("Hidden", 0);
    for (auto targ : targs) {
      std::string denied = "";
      for (Object* own = targ; own; own = own->Parent()) {
        if (own->IsAnimate() && own != body && (!own->IsAct(ACT_SLEEP)) &&
            (!own->IsAct(ACT_DEAD)) && (!own->IsAct(ACT_DYING)) && (!own->IsAct(ACT_UNCONSCIOUS))) {
          denied = "You would need ";
          denied += own->Name(1);
          denied += "'s permission to search ";
          denied += targ->Name(0, nullptr, own);
          denied += ".\n";
        } else if (own->Skill("Container") && (!own->Skill("Open")) && own->Skill("Locked")) {
          denied = own->Name(1);
          if (own == targ) {
            denied += " is closed and locked so you can't search it.\n";
          } else {
            denied += " is closed and locked so you can't get to ";
            denied += targ->Name(1);
            denied += ".\n";
          }
          denied[0] = toupper(denied[0]);
        }
      }
      if ((!nmode) && (!denied.empty())) {
        if (mind)
          mind->Send(denied.c_str());
        continue;
      }

      body->Parent()->SendOut(
          stealth_t, stealth_s, ";s searches ;s.\n", "you search ;s.\n", body, targ);

      auto objs = targ->Contents(vmode);
      for (auto obj : objs) {
        if (obj->Skill("Hidden")) {
          if (body->Roll("Perception", obj->Skill("Hidden"))) {
            obj->SetSkill("Hidden", 0);
            body->Parent()->SendOut(
                stealth_t, stealth_s, ";s reveals ;s.\n", "you reveal ;s.\n", body, obj);
          }
        }
      }

      if (mind) {
        targ->SendExtendedActions(mind, LOC_TOUCH | vmode | 1);
        targ->SendContents(mind, body, LOC_TOUCH | vmode | 1);
      }
    }
    return 0;
  }

  if (com == COM_HIDE) {
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;

    std::vector<Object*> targs;
    if (strlen(cmd.c_str() + len) <= 0) {
      targs.push_back(body);
    } else {
      targs = body->PickObjects(cmd.c_str() + len, vmode | LOC_INTERNAL | LOC_NEARBY | LOC_SELF);
    }
    if (targs.size() < 1) {
      if (mind)
        mind->Send("You don't see that here.\n");
      return 0;
    }

    for (auto targ : targs) {
      std::string denied = "";
      for (Object* own = targ; own; own = own->Parent()) {
        if (own->IsAnimate() && own != body && (!own->IsAct(ACT_SLEEP)) &&
            (!own->IsAct(ACT_DEAD)) && (!own->IsAct(ACT_DYING)) && (!own->IsAct(ACT_UNCONSCIOUS))) {
          denied = "You would need ";
          denied += own->Name(1);
          denied += "'s permission to hide ";
          denied += targ->Name(0, nullptr, own);
          denied += ".\n";
        } else if (own->Skill("Container") && (!own->Skill("Open")) && own->Skill("Locked")) {
          if (own != targ) {
            denied = own->Name(1);
            denied += " is closed and locked so you can't get to ";
            denied += targ->Name(1);
            denied += ".\n";
            denied[0] = toupper(denied[0]);
          }
        }
      }
      if ((!nmode) && (denied.empty())) {
        if (mind)
          mind->Send(denied.c_str());
        continue;
      }
      if ((!nmode) && targ->Skill("Obvious")) {
        if (mind)
          mind->SendF("You could never hide %s, it's too obvious.", targ->Name(0, body));
        continue;
      }
      if ((!nmode) && targ->Skill("Open")) {
        if (targ->Skill("Closeable")) {
          if (mind)
            mind->SendF("You can't hide %s while it's open.", targ->Name(0, body));
        } else {
          if (mind)
            mind->SendF("You can't hide %s.  It's wide open.", targ->Name(0, body));
        }
        continue;
      }

      body->Parent()->SendOut(stealth_t, stealth_s, ";s hides ;s.\n", "you hide ;s.\n", body, targ);

      targ->SetSkill("Hidden", body->Roll("Stealth", 2) * 2);
    }
    return 0;
  }

  if (com == COM_EXAMINE) {
    Object* targ = nullptr;
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;
    if (strlen(cmd.c_str() + len) <= 0) {
      if (mind)
        mind->Send("You want to examine what?\n");
      return 0;
    }
    targ = body->PickObject(cmd.c_str() + len, vmode | LOC_INTERNAL | LOC_NEARBY | LOC_SELF);
    if (!targ) {
      if (mind)
        mind->Send("You don't see that here.\n");
    } else {
      body->Parent()->SendOut(stealth_t, stealth_s, ";s examines ;s.\n", "", body, targ);
      if (mind)
        targ->SendLongDesc(mind, body);
    }
    return 0;
  }

  if (com == COM_CONSIDER) {
    Object* targ = nullptr;
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;
    if (strlen(cmd.c_str() + len) <= 0) {
      if (mind)
        mind->Send("You want to consider what?\n");
      return 0;
    }
    targ = body->PickObject(cmd.c_str() + len, vmode | LOC_INTERNAL | LOC_NEARBY | LOC_SELF);
    if (!targ) {
      if (mind)
        mind->Send("You don't see that here.\n");
    } else if (!targ->IsAnimate()) { // Inanimate Object (Consider Using)
      body->Parent()->SendOut(
          stealth_t, stealth_s, ";s considers using ;s.\n", "You consider using ;s.\n", body, targ);
      if (!mind)
        return 0;

      int handled = 0;

      // Weapons
      if (targ->HasSkill("WeaponType")) {
        handled = 1;
        Object* base = body->ActTarg(ACT_WIELD);
        if (base == targ) {
          mind->SendF("%s is your current weapon!\n", base->Name(0, body));
          mind->Send("Consider using something else for comparison.\n");
          return 0;
        }
        std::string sk = (get_weapon_skill(targ->Skill("WeaponType")));
        if (!body->HasSkill(sk)) {
          mind->SendF(
              CYEL "You don't know much about weapons like %s.\n" CNRM, targ->Name(1, body));
          mind->SendF(CYEL "You would need to learn the %s skill to know more.\n" CNRM, sk.c_str());
        } else {
          int diff;
          mind->SendF("Use of this weapon would use your %s skill.\n", sk.c_str());

          diff = body->Skill(sk);
          if (base)
            diff -= body->Skill(get_weapon_skill(base->Skill("WeaponType")));
          else
            diff -= body->Skill("Punching");
          if (diff > 0)
            mind->Send(CGRN "   ...would be a weapon you are more skilled with.\n" CNRM);
          else if (diff < 0)
            mind->Send(CYEL "   ...would be a weapon you are less skilled with.\n" CNRM);
          else
            mind->Send("   ...would be a weapon you are similarly skilled with.\n");

          diff = targ->Skill("WeaponReach");
          if (base)
            diff -= base->Skill("WeaponReach");
          if (diff > 0)
            mind->Send(CGRN "   ...would give you more reach.\n" CNRM);
          else if (diff < 0)
            mind->Send(CYEL "   ...would give you less reach.\n" CNRM);
          else
            mind->Send("   ...would give you similar reach.\n");

          diff = targ->Skill("WeaponForce");
          if (base)
            diff -= base->Skill("WeaponForce");
          if (diff > 0)
            mind->Send(CGRN "   ...would be more likely to do damage.\n" CNRM);
          else if (diff < 0)
            mind->Send(CYEL "   ...would be less likely to do damage.\n" CNRM);
          else
            mind->Send("   ...would be about as likely to do damage.\n");

          diff = targ->Skill("WeaponSeverity");
          if (base)
            diff -= base->Skill("WeaponSeverity");
          if (diff > 0)
            mind->Send(CGRN "   ...would do more damage.\n" CNRM);
          else if (diff < 0)
            mind->Send(CYEL "   ...would do less damage.\n" CNRM);
          else
            mind->Send("   ...would do similar damage.\n");

          diff = two_handed(targ->Skill("WeaponType"));
          if (base)
            diff -= two_handed(base->Skill("WeaponType"));
          if (diff > 0)
            mind->Send(CYEL "   ...would require both hands to use.\n" CNRM);
          else if (diff < 0)
            mind->Send(CGRN "   ...would not reqire both hands to use.\n" CNRM);
        }
      }

      // Containers
      int wtlimit = 0;
      int szlimit = 0;
      if (targ->HasSkill("Container")) {
        mind->SendF("%s is a container\n", targ->Name(1, body));

        wtlimit = targ->Skill("Container");
        szlimit = targ->Skill("Capacity");
        if (targ->Contents(vmode).size() == 0) {
          mind->Send("   ...it appears to be empty.\n");
        } else {
          if (targ->ContainedVolume() < szlimit / 10) {
            mind->Send("   ...it is nearly empty, ");
          } else if (targ->ContainedVolume() < szlimit / 2) {
            mind->Send("   ...it is less than half full, ");
          } else if (targ->ContainedVolume() < szlimit * 9 / 10) {
            mind->Send("   ...it is more than half full, ");
          } else if (targ->ContainedVolume() < szlimit) {
            mind->Send("   ...it is nearly full, ");
          } else {
            mind->Send("   ...it is full, ");
          }

          if (targ->ContainedWeight() < wtlimit / 10) {
            mind->Send("and is nearly unloaded.\n");
          } else if (targ->ContainedWeight() < wtlimit / 2) {
            mind->Send("and is less than half loaded.\n");
          } else if (targ->ContainedWeight() < wtlimit * 9 / 10) {
            mind->Send("and is more than half loaded.\n");
          } else if (targ->ContainedWeight() < wtlimit) {
            mind->Send("and is heavily laden.\n");
          } else {
            mind->Send("and can hold no more.\n");
          }
        }
      }

      // Liquid Containers
      int volume = 0;
      if (targ->HasSkill("Liquid Container")) {
        mind->SendF("%s is a liquid container\n", targ->Name(1, body));
        volume = targ->Skill("Liquid Container");
        if (targ->Contents(vmode).size() == 0) {
          mind->Send("   ...it appears to be empty.\n");
        } else {
          if (targ->Contents(vmode).front()->Quantity() < volume / 10) {
            mind->Send("   ...it is nearly empty.\n");
          } else if (targ->Contents(vmode).front()->Quantity() < volume / 2) {
            mind->Send("   ...it is less than half full.\n");
          } else if (targ->Contents(vmode).front()->Quantity() < volume * 9 / 10) {
            mind->Send("   ...it is more than half full.\n");
          } else if (targ->Contents(vmode).front()->Quantity() < volume) {
            mind->Send("   ...it is nearly full.\n");
          } else {
            mind->Send("   ...it is full.\n");
          }
        }
      }

      Object* other = body->ActTarg(ACT_HOLD);
      if ((volume || wtlimit || szlimit) && other && other != targ) {
        // Containers
        if (szlimit && other->HasSkill("Capacity")) {
          if (szlimit < other->Skill("Capacity")) {
            mind->Send(CYEL "   ...it can't fit as much, " CNRM);
          } else if (szlimit > other->Skill("Capacity")) {
            mind->Send(CGRN "   ...it can fit more, " CNRM);
          } else {
            mind->Send("   ...it can fit the same, ");
          }
        }
        if (wtlimit && other->HasSkill("Container")) {
          if (wtlimit < other->Skill("Container")) {
            mind->SendF(CYEL "and can't carry as much as %s.\n" CNRM, other->Name(0, body));
          } else if (wtlimit > other->Skill("Container")) {
            mind->SendF(CGRN "and can carry more than %s.\n" CNRM, other->Name(0, body));
          } else {
            mind->SendF("and can carry the same as %s.\n", other->Name(0, body));
          }
        }

        // Liquid Containers
        if (volume && other->HasSkill("Liquid Container")) {
          if (volume < other->Skill("Liquid Container")) {
            mind->SendF(CYEL "   ...it can't hold as much as %s.\n" CNRM, other->Name(0, body));
          } else if (volume > other->Skill("Liquid Container")) {
            mind->SendF(CGRN "   ...it can hold more than %s.\n" CNRM, other->Name(0, body));
          } else {
            mind->SendF("   ...it can hold about the same amount as %s.\n", other->Name(0, body));
          }
        }
      } else if ((volume || wtlimit || szlimit) && other) {
        mind->Send("      (hold another of your containers to compare it with)\n");
      } else if (volume || wtlimit || szlimit) {
        mind->Send("      (hold one of your containers to compare it with)\n");
      }

      // Armor/Clothing
      int all = targ->WearMask();
      int num = count_ones(all);
      if (num > 1) {
        mind->SendF("%s can be worn in %d different ways:\n", targ->Name(0, body), num);
      } else if (num == 1) {
        mind->SendF("%s can only be worn one way:\n", targ->Name(0, body));
      }
      for (int mask = 1; mask <= all; mask <<= 1) {
        std::set<act_t> locs = targ->WearSlots(mask);
        if (locs.size() > 0) {
          mind->SendF("It can be worn on %s.\n", targ->WearNames(locs).c_str());
          handled = 1;

          std::set<Object*> repls;
          for (const auto loc : locs) {
            if (body->ActTarg(loc))
              repls.insert(body->ActTarg(loc));
          }

          for (const auto repl : repls) {
            if (repl != targ) {
              mind->SendF("   ...it would replace %s.\n", repl->Name(0, body));

              int diff = targ->NormAttribute(0);
              diff -= repl->NormAttribute(0);
              if (diff > 0) {
                mind->Send(CGRN "      ...and would provide better protection.\n" CNRM);
              } else if (diff < 0) {
                mind->Send(CYEL "      ...and would not provide as much protection.\n" CNRM);
              } else {
                mind->Send("      ...and would provide similar protection.\n");
              }
            } else if (repls.size() == 1) {
              mind->Send("   ...it is already being worn there.\n");
            }
          }
        }
      }

      // Other
      if (!handled) {
        mind->SendF("You really don't know what you would do with %s.\n", targ->Name(1, body));
      }
    } else { // Animate Opponent (Consider Attacking)
      body->Parent()->SendOut(
          stealth_t,
          stealth_s,
          ";s considers attacking ;s.\n",
          "You consider attacking ;s.\n",
          body,
          targ);
      if (mind) {
        int diff;
        std::string mes = std::string(targ->Name()) + "...\n";
        mes[0] = toupper(mes[0]);
        mind->Send(mes.c_str());

        if ((!targ->ActTarg(ACT_WIELD)) && (!body->ActTarg(ACT_WIELD))) {
          mind->Send("   ...is unarmed, but so are you.\n");
        } else if (!targ->ActTarg(ACT_WIELD)) {
          mind->Send(CGRN "   ...is unarmed.\n" CNRM);
        } else if (!body->ActTarg(ACT_WIELD)) {
          mind->Send(CYEL "   ...is armed, and you are not!\n" CNRM);
        }

        if (targ->HasSkill("NaturalWeapon") && body->HasSkill("NaturalWeapon")) {
          mind->Send("   ...has natural weaponry, but so do you.\n");
        } else if (body->HasSkill("NaturalWeapon")) {
          mind->Send(CGRN "   ...has no natural weaponry, but you do.\n" CNRM);
        } else if (targ->HasSkill("NaturalWeapon")) {
          mind->Send(CYEL "   ...has natural weaponry, and you do not!\n" CNRM);
        }

        diff = 0;
        if (body->ActTarg(ACT_WIELD))
          diff = (body->ActTarg(ACT_WIELD)->Skill("WeaponReach") > 9);
        if (targ->ActTarg(ACT_WIELD) && targ->ActTarg(ACT_WIELD)->Skill("WeaponReach") > 9) {
          if (diff)
            mind->Send("   ...has a ranged weapon, and so do you!\n");
          else
            mind->Send(CYEL "   ...has a ranged weapon!\n" CNRM);
        } else if (diff) {
          mind->Send(CGRN "   ...doesn't have a ranged weapon, and you do!\n" CNRM);
        } else {
          diff = 0;
          if (body->ActTarg(ACT_WIELD))
            diff += body->ActTarg(ACT_WIELD)->Skill("WeaponReach");
          if (targ->ActTarg(ACT_WIELD))
            diff -= targ->ActTarg(ACT_WIELD)->Skill("WeaponReach");
          if (diff < -5)
            mind->Send(CRED "   ...outreaches you by a mile.\n" CNRM);
          else if (diff < -2)
            mind->Send(CRED "   ...has much greater reach than you.\n" CNRM);
          else if (diff < -1)
            mind->Send(CYEL "   ...has greater reach than you.\n" CNRM);
          else if (diff < 0)
            mind->Send(CYEL "   ...has a bit greater reach than you.\n" CNRM);
          else if (diff > 5)
            mind->Send(CGRN "   ...has a mile less reach than you.\n" CNRM);
          else if (diff > 2)
            mind->Send(CGRN "   ...has much less reach than you.\n" CNRM);
          else if (diff > 1)
            mind->Send(CGRN "   ...has less reach than you.\n" CNRM);
          else if (diff > 0)
            mind->Send(CGRN "   ...has a bit less reach than you.\n" CNRM);
          else
            mind->Send("   ...has about your reach.\n");
        }

        if ((!targ->ActTarg(ACT_WEAR_SHIELD)) && (!body->ActTarg(ACT_WEAR_SHIELD))) {
          mind->Send("   ...has no shield, but neither do you.\n");
        } else if (!targ->ActTarg(ACT_WEAR_SHIELD)) {
          mind->Send(CGRN "   ...has no shield.\n" CNRM);
        } else if (!body->ActTarg(ACT_WEAR_SHIELD)) {
          mind->Send(CYEL "   ...has a shield, and you do not!\n" CNRM);
        }

        diff = 0;
        std::string sk = "Punching";
        if (body->IsAct(ACT_WIELD)) {
          sk = get_weapon_skill(body->ActTarg(ACT_WIELD)->Skill("WeaponType"));
        }
        if (body->HasSkill(sk)) {
          diff += body->Skill(sk);
        }
        sk = "Punching";
        if (targ->IsAct(ACT_WIELD)) {
          sk = get_weapon_skill(targ->ActTarg(ACT_WIELD)->Skill("WeaponType"));
        }
        if (targ->HasSkill(sk)) {
          diff -= targ->Skill(sk);
        }
        if (diff < -5)
          mind->Send(CRED "   ...is far more skilled than you.\n" CNRM);
        else if (diff < -2)
          mind->Send(CRED "   ...is much more skilled than you.\n" CNRM);
        else if (diff < -1)
          mind->Send(CYEL "   ...is more skilled than you.\n" CNRM);
        else if (diff < 0)
          mind->Send(CYEL "   ...is a bit more skilled than you.\n" CNRM);
        else if (diff > 5)
          mind->Send(CGRN "   ...is far less skilled than you.\n" CNRM);
        else if (diff > 2)
          mind->Send(CGRN "   ...is much less skilled than you.\n" CNRM);
        else if (diff > 1)
          mind->Send(CGRN "   ...is less skilled than you.\n" CNRM);
        else if (diff > 0)
          mind->Send(CGRN "   ...is a bit less skilled than you.\n" CNRM);
        else
          mind->Send("   ...is about as skilled as you.\n");

        diff = body->NormAttribute(0) - targ->NormAttribute(0);
        if (diff < -10)
          mind->Send(CRED "   ...is titanic.\n" CNRM);
        else if (diff < -5)
          mind->Send(CRED "   ...is gargantuan.\n" CNRM);
        else if (diff < -2)
          mind->Send(CRED "   ...is much larger than you.\n" CNRM);
        else if (diff < -1)
          mind->Send(CYEL "   ...is larger than you.\n" CNRM);
        else if (diff < 0)
          mind->Send(CYEL "   ...is a bit larger than you.\n" CNRM);
        else if (diff > 10)
          mind->Send(CGRN "   ...is an ant compared to you.\n" CNRM);
        else if (diff > 5)
          mind->Send(CGRN "   ...is tiny compared to you.\n" CNRM);
        else if (diff > 2)
          mind->Send(CGRN "   ...is much smaller than you.\n" CNRM);
        else if (diff > 1)
          mind->Send(CGRN "   ...is smaller than you.\n" CNRM);
        else if (diff > 0)
          mind->Send(CGRN "   ...is a bit smaller than you.\n" CNRM);
        else
          mind->Send("   ...is about your size.\n");

        diff = body->NormAttribute(1) - targ->NormAttribute(1);
        if (diff < -10)
          mind->Send(CRED "   ...is a blur of speed.\n" CNRM);
        else if (diff < -5)
          mind->Send(CRED "   ...is lightning fast.\n" CNRM);
        else if (diff < -2)
          mind->Send(CRED "   ...is much faster than you.\n" CNRM);
        else if (diff < -1)
          mind->Send(CYEL "   ...is faster than you.\n" CNRM);
        else if (diff < 0)
          mind->Send(CYEL "   ...is a bit faster than you.\n" CNRM);
        else if (diff > 10)
          mind->Send(CGRN "   ...is a turtle on valium.\n" CNRM);
        else if (diff > 5)
          mind->Send(CGRN "   ...is slower than dial-up.\n" CNRM);
        else if (diff > 2)
          mind->Send(CGRN "   ...is much slower than you.\n" CNRM);
        else if (diff > 1)
          mind->Send(CGRN "   ...is slower than you.\n" CNRM);
        else if (diff > 0)
          mind->Send(CGRN "   ...is a bit slower than you.\n" CNRM);
        else
          mind->Send("   ...is about your speed.\n");

        diff = body->NormAttribute(2) - targ->NormAttribute(2);
        if (diff < -10)
          mind->Send(CRED "   ...is the strongest thing you've ever seen.\n" CNRM);
        else if (diff < -5)
          mind->Send(CRED "   ...is super-strong.\n" CNRM);
        else if (diff < -2)
          mind->Send(CRED "   ...is much stronger than you.\n" CNRM);
        else if (diff < -1)
          mind->Send(CYEL "   ...is stronger than you.\n" CNRM);
        else if (diff < 0)
          mind->Send(CYEL "   ...is a bit stronger than you.\n" CNRM);
        else if (diff > 10)
          mind->Send(CGRN "   ...is a complete push-over.\n" CNRM);
        else if (diff > 5)
          mind->Send(CGRN "   ...is a wimp compared to you.\n" CNRM);
        else if (diff > 2)
          mind->Send(CGRN "   ...is much weaker than you.\n" CNRM);
        else if (diff > 1)
          mind->Send(CGRN "   ...is weaker than you.\n" CNRM);
        else if (diff > 0)
          mind->Send(CGRN "   ...is a bit weaker than you.\n" CNRM);
        else
          mind->Send("   ...is about your strength.\n");

        if (targ->HasSkill("TBAAction")) {
          if ((targ->Skill("TBAAction") & 4128) == 0) {
            mind->Send(CGRN "   ...does not seem threatening.\n" CNRM);
          } else if ((targ->Skill("TBAAction") & 160) == 32) {
            mind->Send(CRED "   ...is spoiling for a fight.\n" CNRM);
          } else if ((targ->Skill("TBAAction") & 160) == 160) {
            mind->Send(CRED "   ...seems to be trolling for victems.\n" CNRM);
          } else if (targ->Skill("TBAAction") & 4096) {
            mind->Send(CYEL "   ...seems to be on the look-out for trouble.\n" CNRM);
          } else {
            mind->Send("   ...is impossible - tell the Ninjas[TM].\n");
          }
        }
      }
    }
    return 0;
  }

  if (com == COM_SCORE) {
    if (mind) {
      if (!body) {
        body = mind->Owner()->Creator();
      }
      if (body) {
        mind->Send(CCYN);
        body->SendFullSituation(mind, body);
        body->SendActions(mind);
        mind->Send(CNRM);
        body->SendScore(mind, body);
      } else {
        mind->Send("You need to select a character first.\n");
      }
    }
    return 0;
  }

  if (com == COM_TIME) {
    if (!mind)
      return 0;
    if (body) {
      Object* world = body->World();
      if (world) {
        if (world->Skill("Day Time") && world->Skill("Day Length")) {
          int curtime = world->Skill("Day Time");
          curtime *= 24 * 60;
          curtime /= world->Skill("Day Length");
          mind->SendF("The time is now %d:%.2d in this world.\n", curtime / 60, curtime % 60);
        } else {
          mind->SendF("This world as no concept of time....\n");
        }
      } else {
        mind->SendF("This character is not in any world.\n");
      }
    } else {
      mind->SendF("You need to enter a character before doing this.\n");
    }
    return 0;
  }

  if (com == COM_WORLD) {
    if (!mind)
      return 0;
    mind->SendF("This world is called: %s\n", body->World()->ShortDesc());
    return 0;
  }

  if (com == COM_STATS) {
    Object* targ = nullptr;
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;
    if ((!body) && strlen(cmd.c_str() + len) <= 0) {
      targ = mind->Owner()->Creator();
    } else if (!body) {
      targ = mind->Owner()->Room()->PickObject(cmd.c_str() + len, vmode | LOC_INTERNAL);
    } else if (strlen(cmd.c_str() + len) <= 0) {
      targ = body;
    } else {
      targ = body->PickObject(
          cmd.c_str() + len, vmode | LOC_INTERNAL | LOC_NEARBY | LOC_SELF | LOC_HERE);
    }
    if (!targ) {
      if (mind)
        mind->Send("You don't see that here.\n");
    } else {
      if (mind) {
        mind->Send(CCYN);
        targ->SendFullSituation(mind, body);
        targ->SendActions(mind);
        mind->Send(CNRM);
        targ->SendScore(mind, body);
      }
    }
    if (targ && body && targ != body && targ != body->Parent()) {
      targ->TryCombine();
    }
    return 0;
  }

  if (com == COM_LOCK) {
    Object* targ = nullptr;
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;
    if (strlen(cmd.c_str() + len) <= 0) {
      if (mind)
        mind->Send("You want to lock what?\n");
      return 0;
    }
    targ = body->PickObject(cmd.c_str() + len, vmode | LOC_INTERNAL | LOC_NEARBY);
    if (!targ) {
      if (mind)
        mind->Send("You don't see that here.\n");
    } else if (targ->Skill("Locked")) {
      if (mind)
        mind->Send("It is already locked!\n");
    } else if (targ->Skill("Lock") <= 0 && (!nmode)) {
      if (mind)
        mind->Send("It does not seem to have a keyhole!\n");
    } else {
      if (!nmode) {
        auto keys = body->PickObjects("all", vmode | LOC_INTERNAL);
        bool can_open = false;
        for (auto key : keys) {
          if (key->Skill("Key") == targ->Skill("Lock")) {
            can_open = true;
            break;
          }
        }
        if (!can_open) {
          if (mind)
            mind->Send("You don't seem to have the right key.\n");
          return 0;
        }
      }
      targ->SetSkill("Locked", 1);
      body->Parent()->SendOut(stealth_t, stealth_s, ";s locks ;s.\n", "You lock ;s.\n", body, targ);
      if (targ->ActTarg(ACT_SPECIAL_MASTER)) {
        Object* targ2 = targ->ActTarg(ACT_SPECIAL_MASTER);
        targ2->Parent()->SendOut(stealth_t, stealth_s, ";s locks.\n", "", targ2, nullptr);
        targ2->SetSkill("Locked", 1);
      }
    }
    return 0;
  }

  if (com == COM_UNLOCK) {
    Object* targ = nullptr;
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;
    if (strlen(cmd.c_str() + len) <= 0) {
      if (mind)
        mind->Send("You want to unlock what?\n");
      return 0;
    }
    targ = body->PickObject(cmd.c_str() + len, vmode | LOC_INTERNAL | LOC_NEARBY);
    if (!targ) {
      if (mind)
        mind->Send("You don't see that here.\n");
    } else if (!targ->Skill("Locked")) {
      if (mind)
        mind->Send("It is not locked!\n");
    } else if (targ->Skill("Lock") <= 0 && (!nmode)) {
      if (mind)
        mind->Send("It does not seem to have a keyhole!\n");
    } else {
      if (!nmode) {
        auto keys = body->PickObjects("all", vmode | LOC_INTERNAL);
        bool can_open = false;
        for (auto key : keys) {
          if (key->Skill("Key") == targ->Skill("Lock")) {
            can_open = true;
            break;
          }
        }
        if (!can_open) {
          if (mind)
            mind->Send("You don't seem to have the right key.\n");
          return 0;
        }
      }
      targ->SetSkill("Locked", 0);
      body->Parent()->SendOut(
          stealth_t, stealth_s, ";s unlocks ;s.\n", "You unlock ;s.\n", body, targ);
      if (targ->ActTarg(ACT_SPECIAL_MASTER)) {
        Object* targ2 = targ->ActTarg(ACT_SPECIAL_MASTER);
        targ2->Parent()->SendOut(stealth_t, stealth_s, ";s unlocks.\n", "", targ2, nullptr);
        targ2->SetSkill("Locked", 1);
      }
    }
    return 0;
  }

  if (com == COM_OPEN) {
    Object* targ = nullptr;
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;
    if (strlen(cmd.c_str() + len) <= 0) {
      if (mind)
        mind->Send("You want to open what?\n");
      return 0;
    }
    targ = body->PickObject(cmd.c_str() + len, vmode | LOC_INTERNAL | LOC_NEARBY);
    if (!targ) {
      if (mind)
        mind->Send("You don't see that here.\n");
    } else if (!targ->Skill("Closeable")) {
      if (mind)
        mind->Send("That can't be opened or closed.\n");
    } else if (targ->Skill("Open")) {
      if (mind)
        mind->Send("It's already open!\n");
    } else if (targ->Skill("Locked")) {
      if (mind)
        mind->Send("It is locked!\n");
    } else {
      targ->SetSkill("Open", 1000);
      body->Parent()->SendOut(stealth_t, stealth_s, ";s opens ;s.\n", "You open ;s.\n", body, targ);
      if (targ->ActTarg(ACT_SPECIAL_MASTER)) {
        Object* targ2 = targ->ActTarg(ACT_SPECIAL_MASTER);
        targ2->Parent()->SendOut(stealth_t, stealth_s, ";s opens.\n", "", targ2, nullptr);
        targ2->SetSkill("Open", 1000);
        targ2->SetSkill("Locked", 0); // FIXME: Do I want to do this?
      }
    }
    return 0;
  }

  if (com == COM_CLOSE) {
    Object* targ = nullptr;
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;
    if (strlen(cmd.c_str() + len) <= 0) {
      if (mind)
        mind->Send("You want to close what?\n");
      return 0;
    }
    targ = body->PickObject(cmd.c_str() + len, vmode | LOC_INTERNAL | LOC_NEARBY);
    if (!targ) {
      if (mind)
        mind->Send("You don't see that here.\n");
    } else if (!targ->Skill("Closeable")) {
      if (mind)
        mind->Send("That can't be opened or closed.\n");
    } else if (!targ->Skill("Open")) {
      if (mind)
        mind->Send("It's already closed!\n");
    } else if (targ->Skill("Locked")) {
      if (mind)
        mind->Send("It is locked!\n");
    } else {
      targ->SetSkill("Open", 0);
      body->Parent()->SendOut(
          stealth_t, stealth_s, ";s closes ;s.\n", "You close ;s.\n", body, targ);
      if (targ->ActTarg(ACT_SPECIAL_MASTER)) {
        Object* targ2 = targ->ActTarg(ACT_SPECIAL_MASTER);
        targ2->Parent()->SendOut(stealth_t, stealth_s, ";s closes.\n", "", targ2, nullptr);
        targ2->SetSkill("Open", 0);
        targ2->SetSkill("Locked", 0); // FIXME: Do I want to do this?
      }
    }
    return 0;
  }

  if (com == COM_LIST) {
    if (!mind)
      return 0;

    auto objs = body->Parent()->Contents(vmode);
    std::vector<Object*> shpkps;
    std::string reason = "";
    for (auto shpkp : objs) {
      if (shpkp->Skill("Sell Profit")) {
        if (shpkp->IsAct(ACT_DEAD)) {
          reason = "Sorry, the shopkeeper is dead!\n";
        } else if (shpkp->IsAct(ACT_DYING)) {
          reason = "Sorry, the shopkeeper is dying!\n";
        } else if (shpkp->IsAct(ACT_UNCONSCIOUS)) {
          reason = "Sorry, the shopkeeper is unconscious!\n";
        } else if (shpkp->IsAct(ACT_SLEEP)) {
          reason = "Sorry, the shopkeeper is asleep!\n";
        } else {
          shpkps.push_back(shpkp);
        }
      }
    }
    if (shpkps.size() == 0) {
      if (mind) {
        mind->Send(reason.c_str());
        mind->Send("You can only do that around a shopkeeper.\n");
      }
    } else {
      Object* shpkp = shpkps.front();
      if (shpkp->ActTarg(ACT_WEAR_RSHOULDER) &&
          shpkp->ActTarg(ACT_WEAR_RSHOULDER)->Skill("Vortex")) {
        Object* vortex = shpkp->ActTarg(ACT_WEAR_RSHOULDER);
        objs = vortex->Contents(vmode);
        auto oobj = objs.front();
        for (auto obj : objs) {
          if (obj != objs.front() && (*obj) == (*oobj))
            continue;
          int price = obj->Value();
          if (obj->Skill("Money") != obj->Value()) { // Not 1-1 Money
            price *= shpkp->Skill("Sell Profit");
            price += 999;
            price /= 1000;
          }
          mind->SendF("%10d gp: %s\n", price, obj->ShortDesc());
          oobj = obj;
        }
      }
    }
    return 0;
  }

  if (com == COM_BUY) {
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;
    if (len >= int(cmd.length())) {
      if (mind)
        mind->Send("What do you want to buy?\n");
      return 0;
    }

    auto objs = body->Parent()->Contents(vmode);
    std::vector<Object*> shpkps;
    std::string reason = "";
    for (auto shpkp : objs) {
      if (shpkp->Skill("Sell Profit")) {
        if (shpkp->IsAct(ACT_DEAD)) {
          reason = "Sorry, the shopkeeper is dead!\n";
        } else if (shpkp->IsAct(ACT_DYING)) {
          reason = "Sorry, the shopkeeper is dying!\n";
        } else if (shpkp->IsAct(ACT_UNCONSCIOUS)) {
          reason = "Sorry, the shopkeeper is unconscious!\n";
        } else if (shpkp->IsAct(ACT_SLEEP)) {
          reason = "Sorry, the shopkeeper is asleep!\n";
        } else {
          shpkps.push_back(shpkp);
        }
      }
    }
    if (shpkps.size() == 0) {
      if (mind) {
        mind->Send(reason.c_str());
        mind->Send("You can only do that around a shopkeeper.\n");
      }
    } else {
      Object* shpkp = shpkps.front();
      if (shpkp->ActTarg(ACT_WEAR_RSHOULDER) &&
          shpkp->ActTarg(ACT_WEAR_RSHOULDER)->Skill("Vortex")) {
        Object* vortex = shpkp->ActTarg(ACT_WEAR_RSHOULDER);

        auto targs = vortex->PickObjects(cmd.c_str() + len, vmode | LOC_INTERNAL);
        if (!targs.size()) {
          if (mind)
            mind->Send("The shopkeeper doesn't have that.\n");
          return 0;
        }

        for (auto targ : targs) {
          int price = targ->Value() * targ->Quantity();
          if (price < 0) {
            if (mind)
              mind->SendF("You can't sell %s.\n", targ->Name(0, body));
            continue;
          } else if (price == 0) {
            if (mind) {
              std::string mes = targ->Name(0, body);
              mes += " is worthless.\n";
              mes[0] = toupper(mes[0]);
              mind->Send(mes.c_str());
            }
            continue;
          }

          if (targ->Skill("Money") != targ->Value()) { // Not 1-1 Money
            price *= shpkp->Skill("Sell Profit");
            price += 999;
            price /= 1000;
          }
          mind->SendF("%d gp: %s\n", price, targ->ShortDesc());

          int togo = price, ord = -price;
          auto pay = body->PickObjects("a gold piece", vmode | LOC_INTERNAL, &ord);
          for (auto coin : pay) {
            togo -= coin->Quantity();
          }

          if (togo > 0) {
            if (mind)
              mind->SendF("You can't afford the %d gold (short %d).\n", price, togo);
          } else if (body->Stash(targ, 0, 0, 0)) {
            body->Parent()->SendOut(
                stealth_t,
                stealth_s,
                ";s buys and stashes ;s.\n",
                "You buy and stash ;s.\n",
                body,
                targ);
            for (auto coin : pay) {
              shpkp->Stash(coin, 0, 1);
            }
          } else if (
              ((!body->IsAct(ACT_HOLD)) || body->ActTarg(ACT_HOLD) == body->ActTarg(ACT_WIELD) ||
               body->ActTarg(ACT_HOLD) == body->ActTarg(ACT_WEAR_SHIELD)) &&
              (!targ->Travel(body))) {
            if (body->IsAct(ACT_HOLD)) {
              body->Parent()->SendOut(
                  stealth_t,
                  stealth_s,
                  ";s stops holding ;s.\n",
                  "You stop holding ;s.\n",
                  body,
                  body->ActTarg(ACT_HOLD));
              body->StopAct(ACT_HOLD);
            }
            body->AddAct(ACT_HOLD, targ);
            body->Parent()->SendOut(
                stealth_t,
                stealth_s,
                ";s buys and holds ;s.\n",
                "You buy and hold ;s.\n",
                body,
                targ);
            for (auto coin : pay) {
              shpkp->Stash(coin, 0, 1);
            }
          } else {
            if (mind)
              mind->SendF("You can't stash or hold %s.\n", targ->Name(1));
          }
        }
      }
    }
    return 0;
  }

  if (com == COM_VALUE || com == COM_SELL) {
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;
    if (len >= int(cmd.length())) {
      if (mind)
        mind->Send("What do you want to sell?\n");
      return 0;
    }

    Object* targ = body->PickObject(cmd.c_str() + len, vmode | LOC_NOTWORN | LOC_INTERNAL);
    if ((!targ) && body->ActTarg(ACT_HOLD) && body->ActTarg(ACT_HOLD)->Parent() != body // Dragging
        && body->ActTarg(ACT_HOLD)->Matches(cmd.c_str() + len)) {
      targ = body->ActTarg(ACT_HOLD);
    }

    if (!targ) {
      if (mind)
        mind->Send("You want to sell what?\n");
      return 0;
    }

    if (targ->Skill("Container") || targ->Skill("Liquid Container")) {
      if (mind) {
        std::string mes = targ->Name(0, body);
        mes += " is a container.";
        mes += "  You can't sell containers (yet).\n";
        mes[0] = toupper(mes[0]);
        mind->Send(mes.c_str());
      }
      return 0;
    }

    if (targ->Contents(LOC_TOUCH | LOC_NOTFIXED).size() > 0) {
      if (mind) {
        std::string mes = targ->Name(0, body);
        mes += " is not empty.";
        mes += "  You must empty it before you can sell it.\n";
        mes[0] = toupper(mes[0]);
        mind->Send(mes.c_str());
      }
      return 0;
    }

    int price = targ->Value() * targ->Quantity();
    if (price < 0 || targ->HasSkill("Priceless") || targ->HasSkill("Cursed")) {
      if (mind)
        mind->SendF("You can't sell %s.\n", targ->Name(0, body));
      return 0;
    }
    if (price == 0) {
      if (mind)
        mind->SendF("%s is worthless.\n", targ->Name(0, body));
      return 0;
    }

    int wearable = 0;
    if (targ->HasSkill("Wearable on Back"))
      wearable = 1;
    if (targ->HasSkill("Wearable on Chest"))
      wearable = 1;
    if (targ->HasSkill("Wearable on Head"))
      wearable = 1;
    if (targ->HasSkill("Wearable on Neck"))
      wearable = 1;
    if (targ->HasSkill("Wearable on Collar"))
      wearable = 1;
    if (targ->HasSkill("Wearable on Waist"))
      wearable = 1;
    if (targ->HasSkill("Wearable on Shield"))
      wearable = 1;
    if (targ->HasSkill("Wearable on Left Arm"))
      wearable = 1;
    if (targ->HasSkill("Wearable on Right Arm"))
      wearable = 1;
    if (targ->HasSkill("Wearable on Left Finger"))
      wearable = 1;
    if (targ->HasSkill("Wearable on Right Finger"))
      wearable = 1;
    if (targ->HasSkill("Wearable on Left Foot"))
      wearable = 1;
    if (targ->HasSkill("Wearable on Right Foot"))
      wearable = 1;
    if (targ->HasSkill("Wearable on Left Hand"))
      wearable = 1;
    if (targ->HasSkill("Wearable on Right Hand"))
      wearable = 1;
    if (targ->HasSkill("Wearable on Left Leg"))
      wearable = 1;
    if (targ->HasSkill("Wearable on Right Leg"))
      wearable = 1;
    if (targ->HasSkill("Wearable on Left Wrist"))
      wearable = 1;
    if (targ->HasSkill("Wearable on Right Wrist"))
      wearable = 1;
    if (targ->HasSkill("Wearable on Left Shoulder"))
      wearable = 1;
    if (targ->HasSkill("Wearable on Right Shoulder"))
      wearable = 1;
    if (targ->HasSkill("Wearable on Left Hip"))
      wearable = 1;
    if (targ->HasSkill("Wearable on Right Hip"))
      wearable = 1;

    auto objs = body->Parent()->Contents();
    std::vector<Object*> shpkps;
    std::string reason = "Sorry, nobody is buying that sort of thing here.\n";
    std::string skill = "";
    for (auto shpkp : objs) {
      if (shpkp->IsAct(ACT_DEAD)) {
        reason = "Sorry, the shopkeeper is dead!\n";
      } else if (shpkp->IsAct(ACT_DYING)) {
        reason = "Sorry, the shopkeeper is dying!\n";
      } else if (shpkp->IsAct(ACT_UNCONSCIOUS)) {
        reason = "Sorry, the shopkeeper is unconscious!\n";
      } else if (shpkp->IsAct(ACT_SLEEP)) {
        reason = "Sorry, the shopkeeper is asleep!\n";
      } else if (targ->Skill("Money") == targ->Value()) { // 1-1 Money
        for (auto skl : shpkp->GetSkills()) {
          if (!strncmp(skl.first.c_str(), "Buy ", 4)) {
            skill = "Money";
            break;
          }
        }
      } else if (wearable && targ->NormAttribute(0) > 0) {
        if (shpkp->HasSkill("Buy Armor")) {
          skill = "Buy Armor";
        }
      } else if (targ->Skill("Vehicle") == 4) {
        if (shpkp->HasSkill("Buy Boat")) {
          skill = "Buy Boat";
        }
      } else if (targ->HasSkill("Container")) {
        if (shpkp->HasSkill("Buy Container")) {
          skill = "Buy Container";
        }
      } else if (targ->HasSkill("Food") && (!targ->HasSkill("Drink"))) {
        if (shpkp->HasSkill("Buy Food")) {
          skill = "Buy Food";
        }
      }
      //      else if(false) {				//FIXME: Implement
      //	if(shpkp->HasSkill("Buy Light")) {
      //	  skill = "Buy Light";
      //	  }
      //	}
      else if (targ->HasSkill("Liquid Container")) { // FIXME: Not Potions?
        if (shpkp->HasSkill("Buy Liquid Container")) {
          skill = "Buy Liquid Container";
        }
      } else if (targ->HasSkill("Liquid Container")) { // FIXME: Not Bottles?
        if (shpkp->HasSkill("Buy Potion")) {
          skill = "Buy Potion";
        }
      } else if (targ->HasSkill("Magical Scroll")) {
        if (shpkp->HasSkill("Buy Scroll")) {
          skill = "Buy Scroll";
        }
      } else if (targ->HasSkill("Magical Staff")) {
        if (shpkp->HasSkill("Buy Staff")) {
          skill = "Buy Staff";
        }
      } else if (targ->HasSkill("Magical Wand")) {
        if (shpkp->HasSkill("Buy Wand")) {
          skill = "Buy Wand";
        }
      }
      //      else if(false) {				//FIXME: Implement
      //	if(shpkp->HasSkill("Buy Trash")) {
      //	  skill = "Buy Trash";
      //	  }
      //	}
      //      else if(false) {				//FIXME: Implement
      //	if(shpkp->HasSkill("Buy Treasure")) {
      //	  skill = "Buy Treasure";
      //	  }
      //	}
      else if (targ->Skill("WeaponType") > 0) {
        if (shpkp->HasSkill("Buy Weapon")) {
          skill = "Buy Weapon";
        }
      } else if (wearable && targ->NormAttribute(0) == 0) {
        if (shpkp->HasSkill("Buy Worn")) {
          skill = "Buy Worn";
        }
      }
      //      else if(false) {					//FIXME:
      //      Implement
      //	if(shpkp->HasSkill("Buy Other")) {
      //	  skill = "Buy Other";
      //	  }
      //	}

      if (skill.empty() && shpkp->HasSkill("Buy All")) {
        skill = "Buy All";
      }

      if ((!skill.empty())) {
        shpkps.push_back(shpkp);
      }
    }
    if (shpkps.size() == 0) {
      if (mind) {
        mind->Send(reason.c_str());
      }
    } else {
      Object* shpkp = shpkps.front();
      if (shpkp->ActTarg(ACT_WEAR_RSHOULDER) &&
          shpkp->ActTarg(ACT_WEAR_RSHOULDER)->Skill("Vortex")) {
        Object* vortex = shpkp->ActTarg(ACT_WEAR_RSHOULDER);

        if (skill != "Money") { // Not 1-1 Money
          price *= shpkp->Skill(skill);
          price += 0;
          price /= 1000;
          if (price <= 0)
            price = 1;
        }
        mind->SendF("I'll give you %dgp for %s\n", price, targ->ShortDesc());

        if (com == COM_SELL) {
          int togo = price, ord = -price;
          auto pay = shpkp->PickObjects("a gold piece", vmode | LOC_INTERNAL, &ord);
          for (auto coin : pay) {
            togo -= std::max(1, coin->Skill("Quantity"));
          }

          if (togo <= 0) {
            body->Parent()->SendOut(
                stealth_t, stealth_s, ";s sells ;s.\n", "You sell ;s.\n", body, targ);
            Object* payment = new Object;
            for (auto coin : pay) {
              coin->Travel(payment);
            }
            if (body->Stash(payment->Contents().front())) {
              targ->Travel(vortex);
            } else { // Keeper gets it back
              shpkp->Stash(payment->Contents().front(), 0, 1);
              if (mind)
                mind->SendF("You couldn't stash %d gold!\n", price);
            }
            delete payment;
          } else {
            if (mind)
              mind->SendF("I can't afford the %d gold.\n", price);
          }
        }
      }
    }
    return 0;
  }

  if (com == COM_DRAG) {
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;
    if (len >= int(cmd.length())) {
      if (mind)
        mind->Send("What do you want to drag?\n");
      return 0;
    }

    if (body->IsAct(ACT_HOLD)) {
      if (mind)
        mind->Send("You are already holding something.  Drop it first.\n");
      return 0;
    }

    Object* targ = body->PickObject(cmd.c_str() + len, vmode | LOC_NEARBY);
    if (!targ) {
      if (mind)
        mind->Send("You want to drag what?\n");
      return 0;
    }

    if (targ->Pos() == POS_NONE) {
      if (mind)
        mind->SendF("You can't drag %s, it is fixed in place!\n", targ->Name());
    } else if (targ->IsAnimate()) {
      std::string denied = "You would need ";
      denied += targ->Name(1);
      denied += "'s permission to drag ";
      denied += targ->Name(0, nullptr, targ);
      denied += ".\n";
      if (mind)
        mind->SendF(denied.c_str(), targ->Name());
    } else if (targ->Weight() > body->ModAttribute(2) * 50000) {
      if (mind)
        mind->SendF("You could never lift %s, it is too heavy.\n", targ->Name());
    } else {
      body->AddAct(ACT_HOLD, targ);
      body->Parent()->SendOut(
          stealth_t, stealth_s, ";s starts dragging ;s.\n", "You start dragging ;s.\n", body, targ);
    }
    return 0;
  }

  if (com == COM_GET) {
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;
    if (len >= int(cmd.length())) {
      if (mind)
        mind->Send("What do you want to get?\n");
      return 0;
    }

    auto targs = body->PickObjects(cmd.c_str() + len, vmode | LOC_NEARBY);
    if (targs.size() == 0) {
      if (mind)
        mind->Send("You want to get what?\n");
      return 0;
    }

    for (auto targ : targs) {
      auto trigs = targ->PickObjects("all tbaMUD trigger script", LOC_NINJA | LOC_INTERNAL);
      for (auto trig : trigs) {
        int ttype = trig->Skill("TBAScriptType");
        if ((ttype & 0x2000040) == 0x2000040) { // OBJ-GET trigs
          if ((rand() % 100) < trig->Skill("TBAScriptNArg")) { // % Chance
            if (new_trigger(0, trig, body))
              return 0; // Says fail!
            if (targ->Parent() == targ->TrashBin())
              return 0; // Purged it
          }
        }
      }

      if ((!nmode) && targ->Pos() == POS_NONE) {
        if (mind)
          mind->SendF("You can't get %s, it is fixed in place!\n", targ->Name());
      } else if ((!nmode) && targ->IsAnimate()) {
        if (mind)
          mind->SendF("You can't get %s, it is not inanimate.\n", targ->Name());
      } else if ((!nmode) && targ->Weight() > body->ModAttribute(2) * 50000) {
        if (mind)
          mind->SendF("You could never lift %s, it is too heavy.\n", targ->Name());
      } else if ((!nmode) && targ->Weight() > body->ModAttribute(2) * 10000) {
        if (mind)
          mind->SendF("You can't carry %s, it is too heavy.  Try 'drag' instead.\n", targ->Name());
      } else {
        std::string denied = "";
        for (Object* owner = targ->Parent(); owner; owner = owner->Parent()) {
          if (owner->IsAnimate() && owner != body && (!owner->IsAct(ACT_SLEEP)) &&
              (!owner->IsAct(ACT_DEAD)) && (!owner->IsAct(ACT_DYING)) &&
              (!owner->IsAct(ACT_UNCONSCIOUS))) {
            denied = "You would need ";
            denied += owner->Name(1);
            denied += "'s permission to get ";
            denied += targ->Name(0, nullptr, owner);
            denied += ".\n";
          } else if (
              owner->Skill("Container") && (!owner->Skill("Open")) && owner->Skill("Locked")) {
            denied = owner->Name(1);
            denied += " is closed and locked so you can't get to ";
            denied += targ->Name(1);
            denied += ".\n";
            denied[0] = toupper(denied[0]);
          }
        }

        if ((!nmode) && denied != "") {
          if (mind)
            mind->Send(denied.c_str());
        } else if (body->Stash(targ, 0)) {
          body->Parent()->SendOut(
              stealth_t,
              stealth_s,
              ";s gets and stashes ;s.\n",
              "You get and stash ;s.\n",
              body,
              targ);
          if (targ->HasSkill("Perishable")) {
            targ->Deactivate();
          }
        } else if (
            body->IsAct(ACT_HOLD) && body->ActTarg(ACT_HOLD) != body->ActTarg(ACT_WEAR_SHIELD) &&
            body->ActTarg(ACT_HOLD) != body->ActTarg(ACT_WIELD)) {
          if (mind)
            mind->SendF("You have no place to stash %s.\n", targ->Name());
        } else if (targ->Skill("Quantity") > 1) {
          if (mind)
            mind->SendF("You have no place to stash %s.\n", targ->Name());
        } else {
          if (body->IsAct(ACT_HOLD)) {
            body->Parent()->SendOut(
                stealth_t,
                stealth_s,
                ";s stops holding ;s.\n",
                "You stop holding ;s.\n",
                body,
                body->ActTarg(ACT_HOLD));
            body->StopAct(ACT_HOLD);
          }
          targ->Travel(body);
          body->AddAct(ACT_HOLD, targ);
          body->Parent()->SendOut(
              stealth_t,
              stealth_s,
              ";s gets and holds ;s.\n",
              "You get and hold ;s.\n",
              body,
              targ);
          if (targ->HasSkill("Perishable")) {
            targ->Deactivate();
          }
        }
      }
    }
    return 0;
  }

  if (com == COM_UNLABEL) {
    if (!body->IsAct(ACT_HOLD)) {
      if (mind)
        mind->Send("You must first 'hold' the object you want label.\n");
      return 0;
    } else if (!body->ActTarg(ACT_HOLD)) {
      if (mind)
        mind->Send("What!?!?!  You are holding nothing?\n");
      return 0;
    }

    std::string name = body->ActTarg(ACT_HOLD)->ShortDesc();
    size_t start = name.find_first_of('(');
    if (start != name.npos) {
      name = name.substr(0, start);
      trim_string(name);
      body->ActTarg(ACT_HOLD)->SetShortDesc(name.c_str());
      if (mind)
        mind->SendF("%s is now unlabeled.\n", body->ActTarg(ACT_HOLD)->Name(1, body));
    } else {
      if (mind)
        mind->SendF(
            "%s does not have a label to remove.\n", body->ActTarg(ACT_HOLD)->Name(1, body));
    }
    return 0;
  }

  if (com == COM_LABEL) {
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;

    if (!body->IsAct(ACT_HOLD)) {
      if (mind)
        mind->Send("You must first 'hold' the object you want label.\n");
      return 0;
    } else if (!body->ActTarg(ACT_HOLD)) {
      if (mind)
        mind->Send("What!?!?!  You are holding nothing?\n");
      return 0;
    }

    if (len >= int(cmd.length())) { // Just Checking Label
      std::string label = body->ActTarg(ACT_HOLD)->ShortDesc();
      size_t start = label.find_first_of('(');
      if (start == label.npos) {
        if (mind)
          mind->SendF("%s has no label.\n", body->ActTarg(ACT_HOLD)->Name(1, body));
      } else {
        label = label.substr(start + 1);
        trim_string(label);
        size_t end = label.find_last_of(')');
        if (end != label.npos) {
          label = label.substr(0, end);
          trim_string(label);
        }
        if (mind)
          mind->SendF(
              "%s is labeled '%s'.\n", body->ActTarg(ACT_HOLD)->Name(1, body), label.c_str());
      }
    } else { // Adding to Label
      std::string name = body->ActTarg(ACT_HOLD)->ShortDesc();
      std::string label = name;
      size_t start = label.find_first_of('(');
      if (start == label.npos) {
        label = (cmd.c_str() + len);
        trim_string(label);
      } else {
        name = label.substr(0, start);
        trim_string(name);
        label = label.substr(start + 1);
        size_t end = label.find_last_of(')');
        if (end != label.npos)
          label = label.substr(0, end);
        trim_string(label);
        if (matches(label.c_str(), cmd.c_str() + len)) {
          if (mind)
            mind->SendF(
                "%s already has that on the label.\n", body->ActTarg(ACT_HOLD)->Name(1, body));
          return 0;
        } else {
          label += " ";
          label += (cmd.c_str() + len);
          trim_string(label);
        }
      }
      body->ActTarg(ACT_HOLD)->SetShortDesc(name.c_str());
      if (mind)
        mind->SendF(
            "%s is now labeled '%s'.\n", body->ActTarg(ACT_HOLD)->Name(1, body), label.c_str());
      body->ActTarg(ACT_HOLD)->SetShortDesc((name + " (" + label + ")").c_str());
    }

    return 0;
  }

  if (com == COM_PUT) {
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;

    if (!strncmp(cmd.c_str() + len, "in ", 3))
      len += 3;

    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;

    if (!body->IsAct(ACT_HOLD)) {
      if (mind)
        mind->Send("You must first 'hold' the object you want to 'put'.\n");
      return 0;
    } else if (!body->ActTarg(ACT_HOLD)) {
      if (mind)
        mind->Send("What!?!?!  You are holding nothing?\n");
      return 0;
    }

    if (len >= int(cmd.length())) {
      if (mind)
        mind->SendF("What do you want to put %s in?\n", body->ActTarg(ACT_HOLD)->Name(0, body));
      return 0;
    }

    Object* targ = body->PickObject(cmd.c_str() + len, vmode | LOC_NEARBY | LOC_INTERNAL);
    if (!targ) {
      if (mind)
        mind->SendF(
            "I don't see '%s' to put '%s' in!\n",
            cmd.c_str() + len,
            body->ActTarg(ACT_HOLD)->Name(0, body));
    } else if (targ->IsAnimate()) {
      if (mind)
        mind->Send("You can only put things in inanimate objects!\n");
    } else if (!targ->Skill("Container")) {
      if (mind)
        mind->Send("You can't put anything in that, it is not a container.\n");
    } else if (targ->Skill("Locked")) {
      if (mind)
        mind->Send("You can't put anything in that, it is locked.\n");
    } else if (targ == body->ActTarg(ACT_HOLD)) {
      if (mind)
        mind->SendF("You can't put %s into itself.\n", body->ActTarg(ACT_HOLD)->Name(0, body));
    } else if (
        (!nmode) && body->ActTarg(ACT_HOLD)->SubHasSkill("Cursed") && targ->Owner() != body) {
      if (mind)
        mind->SendF("You can't seem to part with %s.\n", body->ActTarg(ACT_HOLD)->Name(0, body));
    } else {
      int closed = 0, res = 0;
      Object* obj = body->ActTarg(ACT_HOLD);
      res = obj->Travel(targ);
      if (res == -2) {
        if (mind)
          mind->Send("It won't fit in there.\n");
      } else if (res == -3) {
        if (mind)
          mind->Send("It's too heavy to put in there.\n");
      } else if (res) {
        if (mind)
          mind->Send("You can't put it in there.\n");
      } else {
        if (!targ->Skill("Open"))
          closed = 1;
        if (closed)
          body->Parent()->SendOut(
              stealth_t,
              stealth_s, // FIXME: Really open/close stuff!
              ";s opens ;s.\n",
              "You open ;s.\n",
              body,
              targ);
        std::string safety = obj->Name(0, body);
        body->Parent()->SendOutF(
            stealth_t,
            stealth_s,
            ";s puts %s into ;s.\n",
            "You put %s into ;s.\n",
            body,
            targ,
            safety.c_str());
        if (closed)
          body->Parent()->SendOut(
              stealth_t, stealth_s, ";s close ;s.\n", "You close ;s.\n", body, targ);
      }
    }
    return 0;
  }

  if (com == COM_UNWIELD) {
    com = COM_WIELD;
    cmd = "wield";
    len = 5;
  }

  if (com == COM_WIELD) {
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;
    if (len >= int(cmd.length())) {
      if (body->IsAct(ACT_WIELD)) {
        Object* wield = body->ActTarg(ACT_WIELD);
        if ((!nmode) && wield && wield->SubHasSkill("Cursed")) {
          if (mind)
            mind->SendF("You can't seem to stop wielding %s!\n", wield->Name(0, body));
        } else if (wield && body->Stash(wield, 0)) {
          body->Parent()->SendOut(
              stealth_t,
              stealth_s,
              ";s stops wielding and stashes ;s.\n",
              "You stop wielding and stash ;s.\n",
              body,
              wield);
        } else if (
            body->IsAct(ACT_HOLD) && body->ActTarg(ACT_HOLD) != body->ActTarg(ACT_WEAR_SHIELD) &&
            body->ActTarg(ACT_HOLD) != wield) {
          if (mind)
            mind->SendF(
                "You are holding %s and can't stash %s.\n"
                "Perhaps you want to 'drop' one of these items?",
                body->ActTarg(ACT_HOLD)->Name(1, body),
                wield->Name(1, body));
        } else {
          body->StopAct(ACT_WIELD);
          body->Parent()->SendOut(
              stealth_t,
              stealth_s,
              ";s stops wielding ;s.\n",
              "You stop wielding ;s.\n",
              body,
              wield);
          if (!body->Stash(wield))
            body->AddAct(ACT_HOLD, wield);
        }
      } else {
        if (mind)
          mind->Send("You are not wielding anything.\n");
      }
      return 0;
    }

    Object* targ = body->PickObject(cmd.c_str() + len, vmode | LOC_INTERNAL);
    if (!targ) {
      if (mind)
        mind->Send("You want to wield what?\n");
    } else if (targ->Skill("WeaponType") <= 0) {
      if (mind)
        mind->Send("You can't wield that - it's not a weapon!\n");
    } else if (
        body->ActTarg(ACT_WEAR_BACK) == targ || body->ActTarg(ACT_WEAR_CHEST) == targ ||
        body->ActTarg(ACT_WEAR_HEAD) == targ || body->ActTarg(ACT_WEAR_NECK) == targ ||
        body->ActTarg(ACT_WEAR_COLLAR) == targ || body->ActTarg(ACT_WEAR_WAIST) == targ ||
        body->ActTarg(ACT_WEAR_SHIELD) == targ || body->ActTarg(ACT_WEAR_LARM) == targ ||
        body->ActTarg(ACT_WEAR_RARM) == targ || body->ActTarg(ACT_WEAR_LFINGER) == targ ||
        body->ActTarg(ACT_WEAR_RFINGER) == targ || body->ActTarg(ACT_WEAR_LFOOT) == targ ||
        body->ActTarg(ACT_WEAR_RFOOT) == targ || body->ActTarg(ACT_WEAR_LHAND) == targ ||
        body->ActTarg(ACT_WEAR_RHAND) == targ || body->ActTarg(ACT_WEAR_LLEG) == targ ||
        body->ActTarg(ACT_WEAR_RLEG) == targ || body->ActTarg(ACT_WEAR_LWRIST) == targ ||
        body->ActTarg(ACT_WEAR_RWRIST) == targ || body->ActTarg(ACT_WEAR_LSHOULDER) == targ ||
        body->ActTarg(ACT_WEAR_RSHOULDER) == targ || body->ActTarg(ACT_WEAR_LHIP) == targ ||
        body->ActTarg(ACT_WEAR_RHIP) == targ) {
      if (mind)
        mind->Send("You are wearing that, perhaps you want to 'remove' it?\n");
    } else {
      if (body->IsAct(ACT_WIELD) && body->IsAct(ACT_HOLD)) {
        if (body->ActTarg(ACT_HOLD) != targ) {
          if (mind)
            mind->Send(
                "You are both holding and wielding other things.\n"
                "Perhaps you want to drop one of them?\n");
          return 0;
        }
      }

      // Auto-unwield (trying to wield something else)
      Object* wield = body->ActTarg(ACT_WIELD);
      if ((!nmode) && wield && wield->SubHasSkill("Cursed")) {
        if (mind)
          mind->SendF("You can't seem to stop wielding %s!\n", wield->Name(0, body));
        return 0;
      }
      targ->Travel(body, 0); // Kills Holds and Wields on "targ"
      if (wield) {
        body->StopAct(ACT_WIELD);
        body->Parent()->SendOut(
            stealth_t,
            stealth_s,
            ";s stops wielding ;s.\n",
            "You stop wielding ;s.\n",
            body,
            wield);
        if (!body->Stash(wield)) { // Try to stash first
          body->AddAct(ACT_HOLD, wield); // If not, just hold it
        }
      }

      auto trigs = targ->PickObjects("all tbaMUD trigger script", LOC_NINJA | LOC_INTERNAL);
      for (auto trig : trigs) {
        int ttype = trig->Skill("TBAScriptType");
        if ((ttype & 0x2000200) == 0x2000200) { // OBJ-WEAR trigs
          if (new_trigger(0, trig, body))
            return 0; // Says FAIL!
        }
      }

      body->AddAct(ACT_WIELD, targ);
      body->Parent()->SendOut(
          stealth_t, stealth_s, ";s wields ;s.\n", "You wield ;s.\n", body, targ);
    }
    return 0;
  }

  if (com == COM_HOLD || com == COM_LIGHT) {
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;
    if (len >= int(cmd.length())) {
      if (mind)
        mind->Send("What do you want to hold?\n");
      return 0;
    }

    Object* targ = body->PickObject(cmd.c_str() + len, vmode | LOC_INTERNAL);
    if (!targ) {
      if (mind)
        mind->Send("You want to hold what?\n");
    }
    // FIXME - Implement Str-based Holding Capacity
    //    else if(targ->Skill("WeaponType") <= 0) {
    //      if(mind) mind->Send("You can't hold that - you are too weak!\n");
    //      }
    else if (body->ActTarg(ACT_HOLD) == targ) {
      if (mind)
        mind->SendF("You are already holding %s!\n", targ->Name(1, body));
    } else if (
        body->IsAct(ACT_HOLD) && body->ActTarg(ACT_HOLD) != body->ActTarg(ACT_WIELD) &&
        body->ActTarg(ACT_HOLD) != body->ActTarg(ACT_WEAR_SHIELD)) {
      if (mind)
        mind->Send("You are already holding something!\n");
    } else if (
        body->ActTarg(ACT_WIELD) == targ &&
        two_handed(body->ActTarg(ACT_WIELD)->Skill("WeaponType"))) {
      body->AddAct(ACT_HOLD, targ);
      body->Parent()->SendOut(
          stealth_t,
          stealth_s,
          ";s puts a second hand on ;s.\n",
          "You put a second hand on ;s.\n",
          body,
          targ);
    } else if (body->ActTarg(ACT_WEAR_SHIELD) == targ) {
      body->AddAct(ACT_HOLD, targ);
      body->Parent()->SendOut(stealth_t, stealth_s, ";s holds ;s.\n", "You hold ;s.\n", body, targ);
    } else if (body->Wearing(targ) && targ->SubHasSkill("Cursed")) {
      if (mind) {
        if (body->ActTarg(ACT_WIELD) == targ) {
          mind->SendF("You can't seem to stop wielding %s!\n", targ->Name(0, body));
        } else {
          mind->SendF("You can't seem to remove %s.\n", targ->Name(0, body));
        }
      }
    } else {
      if (body->IsAct(ACT_HOLD)) { // Means it's a shield/2-h weapon due to above.
        body->Parent()->SendOut(
            stealth_t,
            stealth_s,
            ";s stops holding ;s.\n",
            "You stop holding ;s.\n",
            body,
            body->ActTarg(ACT_HOLD));
        body->StopAct(ACT_HOLD);
      }
      targ->Travel(body, 0); // Kills Holds, Wears and Wields on "targ"
      body->AddAct(ACT_HOLD, targ);
      if (com == COM_LIGHT) {
        if (targ->HasSkill("Lightable")) {
          body->Parent()->SendOut(
              stealth_t,
              stealth_s,
              ";s holds and lights ;s.\n",
              "You hold and light ;s.\n",
              body,
              targ);
          targ->SetSkill("Lightable", targ->Skill("Lightable") - 1);
          targ->SetSkill("Light Source", targ->Skill("Brightness"));
          targ->Activate();
        } else {
          body->Parent()->SendOut(
              stealth_t,
              stealth_s,
              ";s holds ;s.\n",
              "You hold ;s, but it can't be lit.\n",
              body,
              targ);
        }
      } else {
        body->Parent()->SendOut(
            stealth_t, stealth_s, ";s holds ;s.\n", "You hold ;s.\n", body, targ);
      }
    }
    return 0;
  }

  if (com == COM_REMOVE) {
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;
    if (len >= int(cmd.length())) {
      if (mind)
        mind->Send("You want to remove what?\n");
      return 0;
    }

    auto targs = body->PickObjects(cmd.c_str() + len, vmode | LOC_INTERNAL);
    if (targs.size() == 0) {
      if (mind)
        mind->Send("You want to remove what?\n");
      return 0;
    }

    for (auto targ : targs) {
      auto trigs = targ->PickObjects("all tbaMUD trigger script", LOC_NINJA | LOC_INTERNAL);
      for (auto trig : trigs) {
        int ttype = trig->Skill("TBAScriptType");
        if ((ttype & 0x2000800) == 0x2000800) { // OBJ-REMOVE trigs
          if (new_trigger(0, trig, body))
            return 0; // Says FAIL!
        }
      }

      int removed = 0;
      if ((!nmode) && targ->HasSkill("Cursed")) {
        if (mind)
          mind->SendF("%s won't come off!\n", targ->Name(0, body));
        return 0;
      }
      for (act_t act = ACT_WEAR_BACK; act < ACT_MAX; act = act_t(int(act) + 1)) {
        if (body->ActTarg(act) == targ) {
          removed = 1;
          break;
        }
      }
      if (!removed) {
        if (mind)
          mind->SendF("You are not wearing %s!\n", targ->Name(0, body));
      } else if (body->Stash(targ, 0, 0, 0)) {
        body->Parent()->SendOut(
            stealth_t,
            stealth_s,
            ";s removes and stashes ;s.\n",
            "You remove and stash ;s.\n",
            body,
            targ);
      } else if (
          body->IsAct(ACT_HOLD) && body->ActTarg(ACT_HOLD) != body->ActTarg(ACT_WEAR_SHIELD) &&
          body->ActTarg(ACT_HOLD) != body->ActTarg(ACT_WIELD)) {
        if (mind)
          mind->SendF(
              "You are already holding something else and can't stash %s.\n", targ->Name(0, body));
      } else {
        if (body->IsAct(ACT_HOLD)) {
          body->Parent()->SendOut(
              stealth_t,
              stealth_s,
              ";s stops holding ;s.\n",
              "You stop holding ;s.\n",
              body,
              body->ActTarg(ACT_HOLD));
          body->StopAct(ACT_HOLD);
        }
        targ->Travel(body, 0);
        body->AddAct(ACT_HOLD, targ);
        body->Parent()->SendOut(
            stealth_t,
            stealth_s,
            ";s removes and holds ;s.\n",
            "You remove and hold ;s.\n",
            body,
            targ);
      }
    }
    return 0;
  }

  if (com == COM_WEAR) {
    std::vector<Object*> targs;

    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;
    if (len >= int(cmd.length())) {
      if (body->ActTarg(ACT_HOLD)) {
        targs.push_back(body->ActTarg(ACT_HOLD));
      } else {
        if (mind) {
          mind->Send("What do you want to wear?  ");
          mind->Send("Use 'wear <item>' or hold the item first.\n");
        }
        return 0;
      }
    }

    if (targs.size() < 1) {
      targs = body->PickObjects(cmd.c_str() + len, vmode | LOC_NOTWORN | LOC_INTERNAL);
      if (!targs.size()) {
        if (mind)
          mind->Send("You want to wear what?\n");
        return 0;
      }
    }

    int did_something = 0;
    for (auto targ : targs) {
      // fprintf(stderr, "You try to wear %s!\n", targ->Name(0, body));
      // if(mind) mind->Send("You try to wear %s!\n", targ->Name(0, body));
      if (body->ActTarg(ACT_WEAR_BACK) == targ || body->ActTarg(ACT_WEAR_CHEST) == targ ||
          body->ActTarg(ACT_WEAR_HEAD) == targ || body->ActTarg(ACT_WEAR_NECK) == targ ||
          body->ActTarg(ACT_WEAR_COLLAR) == targ || body->ActTarg(ACT_WEAR_WAIST) == targ ||
          body->ActTarg(ACT_WEAR_SHIELD) == targ || body->ActTarg(ACT_WEAR_LARM) == targ ||
          body->ActTarg(ACT_WEAR_RARM) == targ || body->ActTarg(ACT_WEAR_LFINGER) == targ ||
          body->ActTarg(ACT_WEAR_RFINGER) == targ || body->ActTarg(ACT_WEAR_LFOOT) == targ ||
          body->ActTarg(ACT_WEAR_RFOOT) == targ || body->ActTarg(ACT_WEAR_LHAND) == targ ||
          body->ActTarg(ACT_WEAR_RHAND) == targ || body->ActTarg(ACT_WEAR_LLEG) == targ ||
          body->ActTarg(ACT_WEAR_RLEG) == targ || body->ActTarg(ACT_WEAR_LWRIST) == targ ||
          body->ActTarg(ACT_WEAR_RWRIST) == targ || body->ActTarg(ACT_WEAR_LSHOULDER) == targ ||
          body->ActTarg(ACT_WEAR_RSHOULDER) == targ || body->ActTarg(ACT_WEAR_LHIP) == targ ||
          body->ActTarg(ACT_WEAR_RHIP) == targ) {
        if (mind && targs.size() == 1)
          mind->SendF("You are already wearing %s!\n", targ->Name(0, body));
      } else {
        auto trigs = targ->PickObjects("all tbaMUD trigger script", LOC_NINJA | LOC_INTERNAL);
        for (auto trig : trigs) {
          int ttype = trig->Skill("TBAScriptType");
          if ((ttype & 0x2000200) == 0x2000200) { // OBJ-WEAR trigs
            if (new_trigger(0, trig, body))
              return 0; // Says FAIL!
          }
        }

        if (body->Wear(targ, ~(0UL), 0))
          did_something = 1;
      }
    }
    if (!did_something)
      if (mind)
        mind->Send("You don't seem to have anything (else) to wear.\n");
    return 0;
  }

  if (com == COM_EAT) {
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;
    if (len >= int(cmd.length())) {
      if (mind)
        mind->Send("What do you want to eat?\n");
      return 0;
    }
    if (!body->HasSkill("Hungry")) {
      if (mind)
        mind->Send("You are not hungry, you can't eat any more.\n");
      return 0;
    }
    auto targs = body->PickObjects(cmd.c_str() + len, vmode | LOC_NOTWORN | LOC_INTERNAL);
    if (body->ActTarg(ACT_HOLD) && body->ActTarg(ACT_HOLD)->Parent() != body // Dragging
        && body->ActTarg(ACT_HOLD)->Matches(cmd.c_str() + len)) {
      targs.push_back(body->ActTarg(ACT_HOLD));
    }
    if (!targs.size()) {
      if (mind)
        mind->Send("You want to eat what?\n");
    } else {
      for (auto targ : targs) {
        if (!(targ->HasSkill("Ingestible"))) {
          if (mind)
            mind->SendF("You don't want to eat %s.\n", targ->Name(0, body));
        } else {
          body->Parent()->SendOut(
              stealth_t, stealth_s, ";s eats ;s.\n", "You eat ;s.\n", body, targ);

          // Hunger/Thirst/Posion/Potion Effects
          body->Consume(targ);

          delete (targ);
        }
      }
    }
    return 0;
  }

  if (com == COM_STASH) {
    Object* targ = body->ActTarg(ACT_HOLD);
    if (targ && targ->Parent() == body) {
      if (!body->Stash(targ)) {
        if (mind)
          mind->SendF("You have no place to stash %s.\n", targ->Name(0, body));
      }
    } else {
      if (mind)
        mind->Send("You are not holding anything to stash.\n");
    }
    return 0;
  }

  if (com == COM_DROP) {
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;
    if (len >= int(cmd.length())) {
      if (mind)
        mind->Send("What do you want to drop?\n");
      return 0;
    }
    auto targs = body->PickObjects(cmd.c_str() + len, vmode | LOC_NOTWORN | LOC_INTERNAL);
    if (body->ActTarg(ACT_HOLD) && body->ActTarg(ACT_HOLD)->Parent() != body // Dragging
        && body->ActTarg(ACT_HOLD)->Matches(cmd.c_str() + len)) {
      targs.push_back(body->ActTarg(ACT_HOLD));
    }
    if (!targs.size()) {
      if (mind)
        mind->Send("You want to drop what?\n");
    } else {
      for (auto targ : targs) {
        auto trigs = targ->PickObjects("all tbaMUD trigger script", LOC_NINJA | LOC_INTERNAL);
        for (auto trig : trigs) {
          int ttype = trig->Skill("TBAScriptType");
          if ((ttype & 0x2000080) == 0x2000080) { // OBJ-DROP trigs
            if ((rand() % 100) < trig->Skill("TBAScriptNArg")) { // % Chance
              if (new_trigger(0, trig, body))
                return 0; // Says FAIL!
              if (targ->Parent() == targ->TrashBin())
                return 0; // Purged it
            }
          }
        }

        Object* room = body->PickObject("here", LOC_HERE);
        trigs.clear();
        if (room)
          trigs = room->PickObjects("all tbaMUD trigger script", LOC_NINJA | LOC_INTERNAL);
        for (auto trig : trigs) {
          int ttype = trig->Skill("TBAScriptType");
          if ((ttype & 0x4000080) == 0x4000080) { // ROOM-DROP trigs
            if ((rand() % 100) < trig->Skill("TBAScriptNArg")) { // % Chance
              if (new_trigger(0, trig, body, targ))
                return 0; // Says FAIL!
              if (targ->Parent() == targ->TrashBin())
                return 0; // Purged it
            }
          }
        }

        int ret = body->Drop(targ, 1, vmode);
        if (ret == -1) { // Totally Failed
          if (mind)
            mind->SendF("You can't drop %s here.\n", targ->Name(0, body));
        } else if (ret == -2) { // Exceeds Capacity
          if (mind)
            mind->SendF("You can't drop %s, there isn't room.\n", targ->Name(0, body));
        } else if (ret == -3) { // Exceeds Weight Limit
          if (mind)
            mind->SendF("You can't drop %s, it's too heavy.\n", targ->Name(0, body));
        } else if (ret == -4) { // Cursed
          if (mind)
            mind->SendF("You don't seem to be able to drop %s!\n", targ->Name(0, body));
        } else if (ret != 0) { //?
          if (mind)
            mind->SendF("You can't seem to drop %s!\n", targ->Name(0, body));
        }
      }
    }
    return 0;
  }

  if (com == COM_DRINK) {
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;
    if (len >= int(cmd.length())) {
      if (mind)
        mind->Send("What do you want to drink from?\n");
      return 0;
    }
    if (!body->HasSkill("Thirsty")) {
      if (mind)
        mind->Send("You are not thirsty, you can't drink any more.\n");
      return 0;
    }
    Object* targ = body->PickObject(cmd.c_str() + len, vmode | LOC_NOTWORN | LOC_INTERNAL);
    if ((!targ) && body->ActTarg(ACT_HOLD) && body->ActTarg(ACT_HOLD)->Parent() != body // Dragging
        && body->ActTarg(ACT_HOLD)->Matches(cmd.c_str() + len)) {
      targ = body->ActTarg(ACT_HOLD);
    }
    if (!targ) {
      targ = body->PickObject(cmd.c_str() + len, vmode | LOC_NEARBY);
    }
    if (!targ) {
      if (mind)
        mind->Send("You want to drink from what?\n");
    } else {
      std::string denied = "";
      for (Object* own = targ; own; own = own->Parent()) {
        if (own->IsAnimate() && own != body && (!own->IsAct(ACT_SLEEP)) &&
            (!own->IsAct(ACT_DEAD)) && (!own->IsAct(ACT_DYING)) && (!own->IsAct(ACT_UNCONSCIOUS))) {
          denied = "You would need ";
          denied += own->Name(1);
          denied += "'s permission to drink from ";
          denied += targ->Name(0, nullptr, own);
          denied += ".\n";
        } else if (own->Skill("Container") && (!own->Skill("Open")) && own->Skill("Locked")) {
          denied = own->Name(1);
          if (own == targ) {
            denied += " is closed and locked so you can't drink from it.\n";
          } else {
            denied += " is closed and locked so you can't get to ";
            denied += targ->Name(1);
            denied += ".\n";
          }
          denied[0] = toupper(denied[0]);
        }
      }
      if ((!nmode) && (!denied.empty())) {
        if (mind)
          mind->Send(denied.c_str());
        return 0;
      }
      if (!(targ->HasSkill("Liquid Container"))) {
        if (mind)
          mind->SendF(
              "%s is not a liquid container.  You can't drink from it.\n", targ->Name(0, body));
        return 0;
      }
      if (targ->Contents(vmode).size() < 1) {
        if (mind)
          mind->SendF("%s is empty.  There is nothing to drink\n", targ->Name(0, body));
        return 0;
      }
      Object* obj = targ->Contents(vmode).front();
      if (targ->HasSkill("Liquid Source") && obj->Skill("Quantity") < 2) {
        if (mind)
          mind->SendF("%s is almost empty.  There is nothing to drink\n", targ->Name(0, body));
        return 0;
      }
      if ((!(obj->HasSkill("Ingestible")))) {
        if (mind)
          mind->SendF("You don't want to drink what's in %s.\n", targ->Name(0, body));
        return 0;
      }

      body->Parent()->SendOut(
          stealth_t,
          stealth_s,
          ";s drinks some liquid out of ;s.\n",
          "You drink some liquid out of ;s.\n",
          body,
          targ);

      // Hunger/Thirst/Posion/Potion Effects
      body->Consume(obj);

      if (obj->Skill("Quantity") < 2) {
        obj->Recycle();
        if (targ->HasSkill("Perishable")) { // One-Use Vials
          targ->Recycle();
        }
      } else {
        obj->SetSkill("Quantity", obj->Skill("Quantity") - 1);
      }
    }
    return 0;
  }

  if (com == COM_DUMP) {
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;
    if (len >= int(cmd.length())) {
      if (mind)
        mind->Send("What do you want to dump?\n");
      return 0;
    }
    auto targs = body->PickObjects(cmd.c_str() + len, vmode | LOC_NOTWORN | LOC_INTERNAL);
    if (body->ActTarg(ACT_HOLD) && body->ActTarg(ACT_HOLD)->Parent() != body // Dragging
        && body->ActTarg(ACT_HOLD)->Matches(cmd.c_str() + len)) {
      targs.push_back(body->ActTarg(ACT_HOLD));
    }
    if (!targs.size()) {
      if (mind)
        mind->Send("You want to dump what?\n");
    } else {
      for (auto targ : targs) {
        if (!(targ->HasSkill("Liquid Container"))) {
          if (mind)
            mind->SendF(
                "%s is not a liquid container.  It can't be dumped.\n", targ->Name(0, body));
          continue;
        }
        if (targ->Contents(LOC_TOUCH).size() < 1) {
          if (mind)
            mind->SendF("%s is empty.  There is nothing to dump\n", targ->Name(0, body));
          continue;
        }
        body->Parent()->SendOut(
            stealth_t,
            stealth_s,
            ";s dumps all the liquid out of ;s.\n",
            "You dump all the liquid out of ;s.\n",
            body,
            targ);
        while (targ->Contents(LOC_TOUCH).size() >= 1) {
          Object* nuke = targ->Contents(LOC_TOUCH).front();
          delete nuke;
        }
      }
    }
    return 0;
  }

  if (com == COM_FILL) {
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;

    if (!strncmp(cmd.c_str() + len, "from ", 5))
      len += 5;

    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;

    if (!body->IsAct(ACT_HOLD)) {
      if (mind)
        mind->Send("You must first 'hold' the object you want to 'fill'.\n");
      return 0;
    } else if (!body->ActTarg(ACT_HOLD)) {
      if (mind)
        mind->Send("What!?!?!  You are holding nothing?\n");
      return 0;
    }

    if (len >= int(cmd.length())) {
      if (mind)
        mind->SendF("Where do you want to fill %s from?\n", body->ActTarg(ACT_HOLD)->Name(0, body));
      return 0;
    }

    Object* src = body->PickObject(cmd.c_str() + len, vmode | LOC_NEARBY | LOC_INTERNAL);
    Object* dst = body->ActTarg(ACT_HOLD);
    if (!src) {
      if (mind)
        mind->SendF("I don't see '%s' to fill %s from!\n", cmd.c_str() + len, dst->Name(0, body));
    } else if (!dst->HasSkill("Liquid Container")) {
      if (mind)
        mind->SendF("You can not fill %s, it is not a liquid container.\n", dst->Name(0, body));
    } else if (src->IsAnimate()) {
      if (mind)
        mind->Send("You can only fill things from inanimate objects!\n");
    } else if (!src->HasSkill("Liquid Container")) {
      if (mind)
        mind->Send(
            "You can't fill anything from that, it's not a liquid "
            "container.\n");
    } else if (dst->Skill("Locked")) {
      if (mind)
        mind->SendF("You can't fill %s, it is locked.\n", dst->Name(0, body));
    } else if (src->Skill("Locked")) {
      if (mind)
        mind->Send("You can't fill anything from that, it is locked.\n");
    } else if (src == dst) {
      if (mind)
        mind->SendF("You can't fill %s from itself.\n", dst->Name(0, body));
    } else if (src->Contents(vmode).size() < 1) {
      if (mind)
        mind->Send("You can't fill anything from that, it is empty.\n");
    } else if (
        src->HasSkill("Liquid Source") && src->Contents(vmode).front()->Skill("Quantity") < 2) {
      if (mind)
        mind->Send("You can't fill anything from that, it is almost empty.\n");
    } else {
      int myclosed = 0, itclosed = 0;

      int sqty = 1;
      int dqty = dst->Skill("Capacity");
      if (src->Contents(vmode).front()->Skill("Quantity") > 0) {
        sqty = src->Contents(vmode).front()->Skill("Quantity");
      }

      if (src->HasSkill("Liquid Source")) {
        if (dqty > (sqty - 1))
          dqty = (sqty - 1);
      } else {
        if (dqty > sqty)
          dqty = sqty;
      }
      sqty -= dqty;

      Object* liq;
      if (dst->Contents(vmode).size() > 0) {
        liq = dst->Contents(vmode).front();
      } else {
        liq = new Object(dst);
      }
      (*liq) = (*(src->Contents(vmode).front()));
      liq->SetSkill("Quantity", dqty);
      if (sqty > 0) {
        src->Contents(vmode).front()->SetSkill("Quantity", sqty);
      } else {
        src->Contents(vmode).front()->Recycle();
      }

      if (!src->Skill("Open"))
        itclosed = 1;
      if (!dst->Skill("Open"))
        myclosed = 1;

      // FIXME: Really open/close stuff!
      if (itclosed)
        body->Parent()->SendOut(
            stealth_t, stealth_s, ";s opens ;s.\n", "You open ;s.\n", body, src);
      if (myclosed)
        body->Parent()->SendOut(
            stealth_t, stealth_s, ";s opens ;s.\n", "You open ;s.\n", body, dst);

      std::string safety = dst->Name(0, body);
      body->Parent()->SendOutF(
          stealth_t,
          stealth_s,
          ";s dumps out and fills %s from ;s.\n",
          "You dump out and fill %s from ;s.\n",
          body,
          src,
          safety.c_str());

      // FIXME: Really open/close stuff!
      if (myclosed)
        body->Parent()->SendOut(
            stealth_t, stealth_s, ";s close ;s.\n", "You close ;s.\n", body, dst);
      if (itclosed)
        body->Parent()->SendOut(
            stealth_t, stealth_s, ";s close ;s.\n", "You close ;s.\n", body, src);
    }
    return 0;
  }

  if (com == COM_LEAVE) {
    Object* oldp = body->Parent();
    if (!body->Parent()->Parent()) {
      if (mind)
        mind->Send("It is not possible to leave this object!\n");
    } else if ((!body->Parent()->Skill("Enterable")) && (!ninja)) {
      if (mind)
        mind->Send("It is not possible to leave this object!\n");
    } else if ((!body->Parent()->Skill("Enterable")) && (!nmode)) {
      if (mind)
        mind->Send("You need to be in ninja mode to leave this object!\n");
    } else {
      if (nmode) {
        // Ninja-movement can't be followed or blocked!
        body->Parent()->NotifyGone(body);
        body->Parent()->RemoveLink(body);
        body->SetParent(body->Parent()->Parent());
      } else {
        body->Travel(body->Parent()->Parent());
      }
      if (oldp)
        oldp->SendOut(stealth_t, stealth_s, ";s leaves.\n", "", body, nullptr);
      body->Parent()->SendDescSurround(body, body);
      body->Parent()->SendOut(stealth_t, stealth_s, ";s arrives.\n", "", body, nullptr);
    }
    return 0;
  }

  if (com == COM_SLEEP) {
    if (body->IsAct(ACT_SLEEP)) {
      if (mind)
        mind->Send("You are already sleeping!\n");
      return 0;
    }
    int lied = 0;
    if (body->Pos() != POS_LIE) {
      body->SetPos(POS_LIE);
      lied = 1;
    }
    if (body->ActTarg(ACT_WIELD)) {
      Object* item = body->ActTarg(ACT_WIELD);
      body->StashOrDrop(item);
    }
    if (body->ActTarg(ACT_HOLD) // Shield held & worn
        && body->ActTarg(ACT_HOLD) == body->ActTarg(ACT_WEAR_SHIELD)) {
      body->Parent()->SendOut(
          stealth_t,
          stealth_s,
          ";s stops holding ;s.\n",
          "You stop holding ;s.\n",
          body,
          body->ActTarg(ACT_HOLD));
      body->StopAct(ACT_HOLD);
    } else if (
        body->ActTarg(ACT_HOLD) // Dragging an item
        && body->ActTarg(ACT_HOLD)->Parent() != body) {
      body->Drop(body->ActTarg(ACT_HOLD));
    } else if (body->ActTarg(ACT_HOLD)) { // Regular held item
      Object* item = body->ActTarg(ACT_HOLD);
      body->StashOrDrop(item);
    }
    body->Collapse();
    body->AddAct(ACT_SLEEP);
    if (lied) {
      body->Parent()->SendOut(
          stealth_t,
          stealth_s,
          ";s lies down and goes to sleep.\n",
          "You lie down and go to sleep.\n",
          body,
          nullptr);
    } else {
      body->Parent()->SendOut(
          stealth_t, stealth_s, ";s goes to sleep.\n", "You go to sleep.\n", body, nullptr);
    }
    return 0;
  }

  if (com == COM_WAKE) {
    if (!body->IsAct(ACT_SLEEP)) {
      if (mind)
        mind->Send("But you aren't asleep!\n");
    } else {
      body->StopAct(ACT_SLEEP);
      body->Parent()->SendOut(
          stealth_t, stealth_s, ";s wakes up.\n", "You wake up.\n", body, nullptr);
    }
    return 0;
  }

  if (com == COM_REST) {
    if (body->IsAct(ACT_REST)) {
      body->StopAct(ACT_REST);
      body->Parent()->SendOut(
          stealth_t, stealth_s, ";s stops resting.\n", "You stop resting.\n", body, nullptr);
      return 0;
    } else if (body->IsAct(ACT_SLEEP)) {
      body->StopAct(ACT_REST);
      body->AddAct(ACT_REST);
      body->Parent()->SendOut(
          stealth_t,
          stealth_s,
          ";s wakes up and starts resting.\n",
          "You wake up and start resting.\n",
          body,
          nullptr);
    } else if (body->Pos() == POS_LIE || body->Pos() == POS_SIT) {
      body->AddAct(ACT_REST);
      body->Parent()->SendOut(
          stealth_t, stealth_s, ";s starts resting.\n", "You start resting.\n", body, nullptr);
    } else {
      body->AddAct(ACT_REST);
      if (body->Pos() != POS_LIE)
        body->SetPos(POS_SIT);
      body->Parent()->SendOut(
          stealth_t,
          stealth_s,
          ";s sits down and rests.\n",
          "You sit down and rest.\n",
          body,
          nullptr);
    }
    if (body->IsAct(ACT_FOLLOW)) {
      if (body->ActTarg(ACT_FOLLOW) && mind)
        body->Parent()->SendOut(
            stealth_t,
            stealth_s,
            ";s stop following ;s.\n",
            "You stop following ;s.\n",
            body,
            body->ActTarg(ACT_FOLLOW));
      body->StopAct(ACT_FOLLOW);
    }
    return 0;
  }

  if (com == COM_STAND) {
    if (body->Pos() == POS_STAND || body->Pos() == POS_USE) {
      if (mind)
        mind->Send("But you are already standing!\n");
    } else if (body->IsAct(ACT_SLEEP)) {
      body->SetPos(POS_STAND);
      body->StopAct(ACT_SLEEP);
      body->Parent()->SendOut(
          stealth_t,
          stealth_s,
          ";s wakes up and stands.\n",
          "You wake up and stand.\n",
          body,
          nullptr);
    } else if (body->IsAct(ACT_REST)) {
      body->StopAct(ACT_REST);
      body->SetPos(POS_STAND);
      body->Parent()->SendOut(
          stealth_t,
          stealth_s,
          ";s stops resting and stands up.\n",
          "You stop resting and stand up.\n",
          body,
          nullptr);
    } else {
      body->SetPos(POS_STAND);
      body->Parent()->SendOut(
          stealth_t, stealth_s, ";s stands up.\n", "You stand up.\n", body, nullptr);
    }
    return 0;
  }

  if (com == COM_SIT) {
    if (body->Pos() == POS_SIT) {
      if (mind)
        mind->Send("But you are already sitting!\n");
    } else if (body->IsAct(ACT_SLEEP)) {
      body->StopAct(ACT_SLEEP);
      body->SetPos(POS_SIT);
      body->Parent()->SendOut(
          stealth_t,
          stealth_s,
          ";s awaken and sit up.\n",
          "You awaken and sit up.\n",
          body,
          nullptr);
    } else if (body->Pos() == POS_LIE) {
      body->SetPos(POS_SIT);
      body->Parent()->SendOut(
          stealth_t, stealth_s, ";s sits up.\n", "You sit up.\n", body, nullptr);
    } else {
      body->SetPos(POS_SIT);
      body->Parent()->SendOut(
          stealth_t, stealth_s, ";s sits down.\n", "You sit down.\n", body, nullptr);
    }
    if (body->IsAct(ACT_FOLLOW)) {
      if (body->ActTarg(ACT_FOLLOW) && mind)
        body->Parent()->SendOut(
            stealth_t,
            stealth_s,
            ";s stop following ;s.\n",
            "You stop following ;s.\n",
            body,
            body->ActTarg(ACT_FOLLOW));
      body->StopAct(ACT_FOLLOW);
    }
    return 0;
  }

  if (com == COM_LIE) {
    if (body->Pos() == POS_LIE) {
      if (mind)
        mind->Send("But you are already lying down!\n");
    } else {
      body->SetPos(POS_LIE);
      body->Parent()->SendOut(
          stealth_t, stealth_s, ";s lies down.\n", "You lie down.\n", body, nullptr);
    }
    if (body->IsAct(ACT_FOLLOW)) {
      if (body->ActTarg(ACT_FOLLOW) && mind)
        body->Parent()->SendOut(
            stealth_t,
            stealth_s,
            ";s stop following ;s.\n",
            "You stop following ;s.\n",
            body,
            body->ActTarg(ACT_FOLLOW));
      body->StopAct(ACT_FOLLOW);
    }
    return 0;
  }

  if (com == COM_CAST) {
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;
    if (len >= int(cmd.length())) {
      if (mind)
        mind->Send("What spell you want to cast?\n");
      return 0;
    }

    int defself = 0;
    int special = 0;
    int freehand = 0;
    std::string spname = "";
    if (!strncmp("identify", cmd.c_str() + len, strlen(cmd.c_str() + len))) {
      special = 1;
      spname = "Identify";
    } else if (!strncmp("create food", cmd.c_str() + len, strlen(cmd.c_str() + len))) {
      defself = -1;
      special = 2;
      freehand = 1;
      spname = "Create Food";
    } else if (!strncmp("force sword", cmd.c_str() + len, strlen(cmd.c_str() + len))) {
      defself = -1;
      special = 2;
      freehand = 1;
      spname = "Force Sword";
    } else if (!strncmp("heat vision", cmd.c_str() + len, strlen(cmd.c_str() + len))) {
      defself = 1;
      spname = "Heat Vision";
    } else if (!strncmp("dark vision", cmd.c_str() + len, strlen(cmd.c_str() + len))) {
      defself = 1;
      spname = "Dark Vision";
    } else if (!strncmp("recall", cmd.c_str() + len, strlen(cmd.c_str() + len))) {
      defself = 1;
      spname = "Recall";
    } else if (!strncmp("teleport", cmd.c_str() + len, strlen(cmd.c_str() + len))) {
      defself = 1;
      spname = "Teleport";
    } else if (!strncmp("resurrect", cmd.c_str() + len, strlen(cmd.c_str() + len))) {
      defself = 1;
      spname = "Resurrect";
    } else if (!strncmp("remove curse", cmd.c_str() + len, strlen(cmd.c_str() + len))) {
      defself = 1;
      spname = "Remove Curse";
    } else if (!strncmp("cure poison", cmd.c_str() + len, strlen(cmd.c_str() + len))) {
      defself = 1;
      spname = "Cure Poison";
    } else if (!strncmp("sleep other", cmd.c_str() + len, strlen(cmd.c_str() + len))) {
      spname = "Sleep Other";
    } else {
      if (mind)
        mind->Send("Never heard of that spell.\n");
      return 0;
    }

    Object* src = nullptr;
    if (!nmode)
      src = body->NextHasSkill(spname + " Spell");
    if ((!nmode) && (!src)) {
      if (mind)
        mind->SendF(
            "You don't know the %s Spell and have no items enchanted "
            "with it.\n",
            spname.c_str());
      return 0;
    }

    if ((!defself) && (!body->ActTarg(ACT_POINT))) {
      if (mind)
        mind->Send("That spell requires you to first point at your target.\n");
      return 0;
    }

    if (freehand) {
      if (body->ActTarg(ACT_HOLD)) {
        body->StashOrDrop(body->ActTarg(ACT_HOLD));
      }
    }

    Object* targ = body->ActTarg(ACT_POINT);
    if (!targ)
      targ = body; // Defaults to SELF if not, caught above!)
    if (src) {
      std::string youmes = "You use ;s to cast " + spname + ".\n";
      body->Parent()->SendOut(
          stealth_t, stealth_s, ";s uses ;s to cast a spell.\n", youmes.c_str(), body, src);
    }
    if (defself >= 0) { // Targeted
      std::string youmes = "You cast " + spname + " on ;s.\n";
      body->Parent()->SendOut(
          stealth_t, stealth_s, ";s casts a spell on ;s.\n", youmes.c_str(), body, targ);
    } else { // Not Targeted
      std::string youmes = "You cast " + spname + ".\n";
      body->Parent()->SendOut(
          stealth_t, stealth_s, ";s casts a spell.\n", youmes.c_str(), body, nullptr);
    }

    int force = 1000; // FIXME: Magic Force!
    if (src)
      force = src->Skill(spname + " Spell");
    if (!special) { // Effect Person/Creature Spells
      Object* spell = new Object();
      spell->SetSkill(spname + " Spell", force);
      targ->Consume(spell);
      delete (spell);
    } else if (special == 2) { // Temporary Object Creation Spells
      Object* obj = new Object(body);
      if (spname == "Create Food") {
        obj->SetShortDesc("a piece of magical food");
        obj->SetSkill("Food", force * 100);
        obj->SetSkill("Ingestible", force);
      } else if (spname == "Force Sword") {
        obj->SetShortDesc("a sword of force");
        obj->SetSkill("WeaponType", 13);
        obj->SetSkill("WeaponReach", 1);
        obj->SetSkill("WeaponSeverity", 2);
        obj->SetSkill("WeaponForce", std::min(100, force));
      }
      obj->SetWeight(1);
      obj->SetVolume(1);
      obj->SetSize(1);
      obj->SetSkill("Magical", force);
      obj->SetSkill("Light Source", 10);
      obj->SetSkill("Temporary", force);
      obj->Activate();
      obj->SetPos(POS_LIE);
      body->AddAct(ACT_HOLD, obj);
      body->Parent()->SendOutF(
          0,
          0,
          "%s appears in ;s's hand.\n",
          "%s appears in your hand.\n",
          body,
          nullptr,
          obj->Name());
    } else if (spname == "Identify") { // Other kinds of spells
      if (mind) {
        mind->Send(CCYN);
        targ->SendFullSituation(mind, body);
        targ->SendActions(mind);
        mind->Send(CNRM);
        targ->SendScore(mind, body);
      }
    }

    if (src) { // FIXME: Handle Multi-Charged Items (Rod/Staff/Wand)
      if (src->Skill("Quantity") > 1) {
        src->Split(src->Skill("Quantity") - 1); // Split off the rest
      }
      if (src->Skill("Magical Charges") > 1) {
        src->SetSkill("Magical Charges", src->Skill("Magical Charges") - 1);
      } else {
        delete (src);
      }
    }

    return 0;
  }

  if (com == COM_PRAY) {
    if (!mind)
      return 0;
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;

    if (strlen(cmd.c_str() + len) <= 0) {
      mind->Send("What command do you want to pray for?\n");
      mind->Send("You need to include the command, like 'pray kill Joe'.\n");
    } else {
      // FIXME: Tell sub-command you're praying!
      body->Parent()->SendOut(stealth_t, stealth_s, ";s prays.\n", "You pray.\n", body, nullptr);
      handle_single_command(body, cmd.c_str() + len, mind);
    }
    return 0;
  }

  if (com == COM_STOP) { // Alias "stop" to "use"
    com = COM_USE;
    cmd = "";
    len = 0;
  }
  if (com == COM_USE) {
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;
    if (strlen(cmd.c_str() + len) == 0) {
      if (body->Pos() != POS_USE) {
        mind->Send("You're not using a skill.  Try 'use <skillname>' to start.\n");
      } else {
        body->Parent()->SendOutF(
            stealth_t,
            stealth_s,
            ";s stops %s.\n",
            "You stop %s.\n",
            body,
            nullptr,
            body->UsingString());
        body->SetPos(POS_STAND);
        return 2;
      }
      return 0;
    }

    int longterm = 0; // Long-running skills for results
    std::string skill = get_skill(cmd.c_str() + len);
    if (skill == "") {
      mind->Send("Don't know what skill you're trying to use.\n");
      return 0;
    }

    if (skill == "Lumberjack") {
      if (!body->HasSkill(skill)) {
        mind->SendF("%sYou don't know how to do that.%s\n", CYEL, CNRM);
        return 0;
      }
      if (!body->Parent()) { // You're nowhere?!?
        mind->SendF("%sThere are no trees here.%s\n", CYEL, CNRM);
        return 0;
      }
      if (strcasestr(body->Parent()->Name(), "forest") && body->Parent()->HasSkill("TBAZone") &&
          (!body->Parent()->HasSkill("Mature Trees"))) {
        body->Parent()->SetSkill("Mature Trees", 100);
        body->Parent()->Activate();
      }
      if (!body->Parent()->HasSkill("Mature Trees")) {
        mind->SendF("%sThere are no trees here.%s\n", CYEL, CNRM);
        return 0;
      } else if (body->Parent()->Skill("Mature Trees") < 10) {
        mind->SendF("%sThere are too few trees to harvest here.%s\n", CYEL, CNRM);
        return 0;
      } else {
        longterm = 3000; // FIXME: Temporary - should take longer!
      }
      if (body->IsUsing("Lumberjack")) { // Already been doing it
        if (body->Roll(skill, 10) > 0) { // Succeeded!
          body->Parent()->SendOut(
              ALL, 0, ";s shouts 'TIMBER'!!!\n", "You shout 'TIMBER'!!!\n", body, body);
          body->Parent()->Loud(body->Skill("Strength"), "someone shout 'TIMBER'!!!");
          body->Parent()->SetSkill("Mature Trees", body->Parent()->Skill("Mature Trees") - 1);
          body->SetSkill("Hidden", 0);

          Object* log = new Object(body->Parent());
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
        body->Parent()->SendOut(
            ALL,
            0,
            ";s continues chopping down trees.\n",
            "You continue chopping down trees.\n",
            body,
            body);
        body->Parent()->Loud(body->Skill("Strength") / 2, "loud chopping sounds.");
        body->SetSkill("Hidden", 0);
      }
    }

    if (!body->IsUsing(skill)) { // Only if really STARTING to use skill.
      body->StartUsing(skill);

      // In case Stealth was started, re-calc (to hide going into stealth).
      stealth_t = 0;
      stealth_s = 0;
      if (body->IsUsing("Stealth") && body->Skill("Stealth") > 0) {
        stealth_t = body->Skill("Stealth");
        stealth_s = body->Roll("Stealth", 2);
      }

      if (body->Pos() != POS_STAND && body->Pos() != POS_USE) { // FIXME: Unused
        body->Parent()->SendOutF(
            stealth_t,
            stealth_s,
            ";s stands and starts %s.\n",
            "You stand up and start %s.\n",
            body,
            nullptr,
            body->UsingString());
      } else {
        body->Parent()->SendOutF(
            stealth_t,
            stealth_s,
            ";s starts %s.\n",
            "You start %s.\n",
            body,
            nullptr,
            body->UsingString());
      }
      if (!body->HasSkill(skill)) {
        mind->SendF(
            "%s...you don't have the '%s' skill, so you're bad at this.%s\n",
            CYEL,
            skill.c_str(),
            CNRM);
      }
    } else if (longterm == 0) {
      mind->SendF("You are already using %s\n", skill.c_str());
    }

    if (longterm > 0) { // Long-running skills for results
      body->BusyFor(longterm, "use Lumberjack"); // FIXME: Temporary!
      return 2; // Full-round (and more) action!
    }
    return 0;
  }

  if (com == COM_POINT) {
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;
    if (strlen(cmd.c_str() + len) > 0) {
      Object* targ =
          body->PickObject(cmd.c_str() + len, vmode | LOC_NEARBY | LOC_SELF | LOC_INTERNAL);
      if (!targ) {
        if (mind)
          mind->Send("You don't see that here.\n");
      } else {
        body->AddAct(ACT_POINT, targ);
        body->Parent()->SendOut(
            stealth_t,
            stealth_s,
            ";s starts pointing at ;s.\n",
            "You start pointing at ;s.\n",
            body,
            targ);
      }
    } else if (body->IsAct(ACT_POINT)) {
      Object* targ = body->ActTarg(ACT_POINT);
      body->StopAct(ACT_POINT);
      body->Parent()->SendOut(
          stealth_t,
          stealth_s,
          ";s stops pointing at ;s.\n",
          "You stop pointing at ;s.\n",
          body,
          targ);
    } else {
      if (mind)
        mind->Send("But, you aren't pointing at anyting!\n");
    }
    return 0;
  }

  if (com == COM_FOLLOW) {
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;
    if (strlen(cmd.c_str() + len) > 0) {
      Object* targ = body->PickObject(cmd.c_str() + len, vmode | LOC_NEARBY | LOC_SELF);
      if (!targ) {
        if (mind)
          mind->Send("You don't see that here.\n");
      } else {
        body->AddAct(ACT_FOLLOW, targ);
        body->Parent()->SendOut(
            stealth_t,
            stealth_s,
            ";s starts following ;s.\n",
            "You start following ;s.\n",
            body,
            targ);
      }
    } else if (body->IsAct(ACT_FOLLOW)) {
      Object* targ = body->ActTarg(ACT_FOLLOW);
      body->StopAct(ACT_FOLLOW);
      body->Parent()->SendOut(
          stealth_t, stealth_s, ";s stops following ;s.\n", "You stop following ;s.\n", body, targ);
    } else {
      if (mind)
        mind->Send("But, you aren't following anyone!\n");
    }
    return 0;
  }

  if (com == COM_ATTACK || com == COM_KILL || com == COM_PUNCH || com == COM_KICK) {
    // fprintf(stderr, "Handling attack command from %p of '%s'\n", mind,
    // cmd.c_str());

    int attacknow = 1;
    if (!body->IsAct(ACT_FIGHT))
      attacknow = 0;

    Object* targ = nullptr;
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;
    if (strlen(cmd.c_str() + len) > 0) {
      targ = body->PickObject(cmd.c_str() + len, vmode | LOC_NEARBY | LOC_CONSCIOUS);
      if (!targ)
        targ = body->PickObject(cmd.c_str() + len, vmode | LOC_NEARBY | LOC_ALIVE);
      if (!targ)
        targ = body->PickObject(cmd.c_str() + len, vmode | LOC_NEARBY);
      if (!targ) {
        if (mind)
          mind->Send("You don't see that here.\n");
        return 0;
      }
    } else {
      if (body->IsAct(ACT_FIGHT)) {
        targ = body->ActTarg(ACT_FIGHT);
        if (!body->IsNearBy(targ)) {
          if (mind)
            mind->Send("Your target is gone!\n");
          body->StopAct(ACT_FIGHT);
          return 0;
        }
      } else {
        if (mind)
          mind->Send("Who did you want to hit?\n");
        return 0;
      }
    }

    if (com == COM_ATTACK &&
        (!targ->IsAnimate() || targ->IsAct(ACT_DEAD) || targ->IsAct(ACT_DYING) ||
         targ->IsAct(ACT_UNCONSCIOUS))) {
      if (mind)
        mind->Send("No need, target is down!\n");
      body->StopAct(ACT_FIGHT);
      return 0;
    }

    if (is_pc(body) && is_pc(targ)) {
      if (mind) {
        mind->SendF(
            "You can't attack %s, %s is a PC (no PvP)!\n",
            targ->Name(0, body),
            targ->Name(0, body));
      }
      return 0;
    }

    if ((!body->Parent()) || body->Parent()->HasSkill("Peaceful")) {
      if (mind) {
        mind->Send("You can't fight here.  This is a place of peace.\n");
      }
      return 0;
    }

    body->BusyFor(3000); // Overridden below if is alive/animate

    if (!(!targ->IsAnimate() || targ->IsAct(ACT_DEAD) || targ->IsAct(ACT_DYING) ||
          targ->IsAct(ACT_UNCONSCIOUS))) {
      body->AddAct(ACT_FIGHT, targ);
      body->BusyFor(3000, body->Tactics().c_str());
      if (!targ->IsAct(ACT_FIGHT)) {
        targ->BusyFor(3000, body->Tactics().c_str());
        targ->AddAct(ACT_FIGHT, body);
      } else if (targ->StillBusy()) {
        body->BusyWith(targ, body->Tactics().c_str());
      }
    } else {
      attacknow = 1; // Uncontested.
    }

    // Free your off-hand if needed (if it's not a shield or weapon)
    if (body->ActTarg(ACT_HOLD) // FIXME: Don't drop offhand weapons?!?
        && body->ActTarg(ACT_HOLD) != body->ActTarg(ACT_WEAR_SHIELD) &&
        body->ActTarg(ACT_HOLD) != body->ActTarg(ACT_WIELD) &&
        (body->ActTarg(ACT_WEAR_SHIELD) // Need Off-Hand for shield
         || (body->ActTarg(ACT_WIELD) //...or for two-hander
             && two_handed(body->ActTarg(ACT_WIELD)->Skill("WeaponType"))))) {
      if (body->DropOrStash(body->ActTarg(ACT_HOLD))) {
        if (mind)
          mind->SendF(
              "Oh, no!  You can't drop or stash %s, "
              "but you need your off-hand!",
              body->ActTarg(ACT_HOLD)->Name(0, body));
      }
    }

    // Hold your 2-hander, even if you have to let go of your shield
    if (body->ActTarg(ACT_WIELD) // Half-Wielding a 2-Hander
        && body->ActTarg(ACT_HOLD) != body->ActTarg(ACT_WIELD) &&
        two_handed(body->ActTarg(ACT_WIELD)->Skill("WeaponType"))) {
      if (body->ActTarg(ACT_HOLD) // Some non-shield stuck in other hand!
          && body->ActTarg(ACT_HOLD) != body->ActTarg(ACT_WEAR_SHIELD)) {
        if (mind)
          mind->SendF(
              "Oh, no!  You can't use %s - it's two-handed!\n",
              body->ActTarg(ACT_WIELD)->Name(0, body));
        if (body->DropOrStash(body->ActTarg(ACT_WIELD))) {
          if (mind)
            mind->SendF(
                "Oh, no!  You can't drop or stash %s!\n", body->ActTarg(ACT_WIELD)->Name(0, body));
        }
      } else {
        if (body->ActTarg(ACT_HOLD)) { // Unhold your shield
          body->Parent()->SendOut(
              stealth_t,
              stealth_s,
              ";s stops holding ;s.\n",
              "You stop holding ;s.\n",
              body,
              body->ActTarg(ACT_HOLD));
          body->StopAct(ACT_HOLD);
        }
        Object* weap = body->ActTarg(ACT_WIELD); // Hold your 2-hander
        body->AddAct(ACT_HOLD, weap);
        body->Parent()->SendOut(
            stealth_t, stealth_s, ";s holds ;s.\n", "You hold ;s.\n", body, weap);
      }
    }

    // If you're hand's now free, and you have a shield, use it.
    if (body->ActTarg(ACT_WEAR_SHIELD) && (!body->IsAct(ACT_HOLD))) {
      Object* shield = body->ActTarg(ACT_WEAR_SHIELD);

      body->AddAct(ACT_HOLD, shield);
      body->Parent()->SendOut(
          stealth_t, stealth_s, ";s holds ;s.\n", "You hold ;s.\n", body, shield);
    } else if (
        mind && body->ActTarg(ACT_WEAR_SHIELD) &&
        body->ActTarg(ACT_WEAR_SHIELD) != body->ActTarg(ACT_HOLD)) {
      mind->SendF(
          "Oh, no!  You can't use %s - your off-hand is not free!\n",
          body->ActTarg(ACT_WEAR_SHIELD)->Name(0, body));
    }

    if (!attacknow) {
      body->Parent()->SendOut(
          stealth_t, stealth_s, ";s moves to attack ;s.\n", "You move to attack ;s.\n", body, targ);
      body->BusyWith(body, inpline); // HACK!  Make this command used first rnd!
      return 2; // No more actions until next round!
    }

    // Attacking, or being attacked removes hidden-ness.
    body->SetSkill("Hidden", 0);
    targ->SetSkill("Hidden", 0);

    int reachmod = 0;
    std::string sk1 = "Punching", sk2 = "Punching";

    if (com == COM_KICK) {
      sk1 = "Kicking";
      sk2 = "Kicking";
    }

    else if (
        body->ActTarg(ACT_WIELD) // Not Holding your 2-Hander
        && two_handed(body->ActTarg(ACT_WIELD)->Skill("WeaponType")) &&
        body->ActTarg(ACT_WIELD) != body->ActTarg(ACT_HOLD)) {
      sk1 = "Kicking";
      sk2 = "Kicking";
    }

    else {
      if (body->IsAct(ACT_WIELD)) {
        sk1 = get_weapon_skill(body->ActTarg(ACT_WIELD)->Skill("WeaponType"));
        reachmod += std::max(0, body->ActTarg(ACT_WIELD)->Skill("WeaponReach"));
        if (reachmod > 9)
          reachmod = 0;
      }
      if (body->ActTarg(ACT_HOLD) == body->ActTarg(ACT_WEAR_SHIELD) && body->ActTarg(ACT_HOLD)) {
        sk2 = "";
      }
      if (targ->ActTarg(ACT_HOLD) == targ->ActTarg(ACT_WEAR_SHIELD) && targ->ActTarg(ACT_HOLD)) {
        sk2 = "Shields";
        reachmod = 0;
      } else if (
          targ->ActTarg(ACT_WIELD) // Not Holding his 2-Hander
          && two_handed(targ->ActTarg(ACT_WIELD)->Skill("WeaponType")) &&
          targ->ActTarg(ACT_WIELD) != targ->ActTarg(ACT_HOLD)) {
        sk2 = ""; // Can't defend himself
      } else if (targ->ActTarg(ACT_WIELD)) {
        if (sk2 == "Punching")
          sk2 = get_weapon_skill(targ->ActTarg(ACT_WIELD)->Skill("WeaponType"));
        reachmod -= std::max(0, targ->ActTarg(ACT_WIELD)->Skill("WeaponReach"));
        if (reachmod < -9)
          reachmod = 0;
      }
    }

    int succ = 0;
    succ = roll(body->Skill(sk1) + body->Modifier("Accuracy"), targ->Skill(sk2) - reachmod);
    succ -= roll(targ->Skill(sk2) + targ->Modifier("Evasion"), body->Skill(sk1) + reachmod);

    int loc = rand() % 100;
    act_t loca = ACT_WEAR_CHEST;
    std::string locm = "";
    int stage = 0;
    if (loc < 50) {
      loca = ACT_WEAR_CHEST;
      locm = " in the chest";
    } else if (loc < 56) {
      loca = ACT_WEAR_BACK;
      locm = " in the back";
    } else if (loc < 58) {
      loca = ACT_WEAR_HEAD;
      locm = " in the head";
      stage = 1;
    } else if (loc < 59) {
      loca = ACT_WEAR_NECK;
      locm = " in the neck";
      stage = 2;
    } else if (loc < 69) {
      loca = ACT_WEAR_COLLAR;
      locm = " in the throat";
      stage = 2;
    } else if (loc < 70) {
      loca = ACT_WEAR_LARM;
      locm = " in the left arm";
      stage = -1;
    } else if (loc < 80) {
      loca = ACT_WEAR_RARM;
      locm = " in the right arm";
      stage = -1;
    } else if (loc < 85) {
      loca = ACT_WEAR_LLEG;
      locm = " in the left leg";
      stage = -1;
    } else if (loc < 90) {
      loca = ACT_WEAR_RLEG;
      locm = " in the right leg";
      stage = -1;
    } else if (loc < 93) {
      loca = ACT_WEAR_LHAND;
      locm = " in the left hand";
      stage = -2;
    } else if (loc < 96) {
      loca = ACT_WEAR_RHAND;
      locm = " in the right hand";
      stage = -2;
    } else if (loc < 98) {
      loca = ACT_WEAR_LFOOT;
      locm = " in the left foot";
      stage = -2;
    } else {
      loca = ACT_WEAR_RFOOT;
      locm = " in the right foot";
      stage = -2;
    }

    const char* verb = "punch"; // Non-weapon verb
    const char* verb3 = "punches"; // 3rd Person
    if (body->Skill("NaturalWeapon") == 14) { // Natural Weapon: stab
      verb = "stab";
      verb3 = "stabs";
    } else if (body->Skill("NaturalWeapon") == 13) { // Natural Weapon: hit
      verb = "hit";
      verb3 = "hits";
    } else if (body->Skill("NaturalWeapon") == 12) { // Natural Weapon: blast
      verb = "blast";
      verb3 = "blasts";
    } else if (body->Skill("NaturalWeapon") == 11) { // Natural Weapon: pierce
      verb = "pierce";
      verb3 = "pierces";
    } else if (body->Skill("NaturalWeapon") == 10) { // Natural Weapon: thrash
      verb = "thrash";
      verb3 = "thrashes";
    } else if (body->Skill("NaturalWeapon") == 9) { // Natural Weapon: maul
      verb = "maul";
      verb3 = "mauls";
    } else if (body->Skill("NaturalWeapon") == 8) { // Natural Weapon: claw
      verb = "claw";
      verb3 = "claws";
    } else if (body->Skill("NaturalWeapon") == 7) { // Natural Weapon: pound
      verb = "pound";
      verb3 = "pounds";
    } else if (body->Skill("NaturalWeapon") == 6) { // Natural Weapon: crush
      verb = "crush";
      verb3 = "crushes";
    } else if (body->Skill("NaturalWeapon") == 5) { // Natural Weapon: bludgeon
      verb = "bludgeon";
      verb3 = "bludgeons";
    } else if (body->Skill("NaturalWeapon") == 4) { // Natural Weapon: bite
      verb = "bite";
      verb3 = "bites";
    } else if (body->Skill("NaturalWeapon") == 3) { // Natural Weapon: slash
      verb = "slash";
      verb3 = "slashes";
    } else if (body->Skill("NaturalWeapon") == 2) { // Natural Weapon: whip
      verb = "whip";
      verb3 = "whips";
    } else if (body->Skill("NaturalWeapon") == 1) { // Natural Weapon: sting
      verb = "sting";
      verb3 = "stings";
    }
    if (succ > 0) {
      int stun = 0;
      if (sk1 == "Kicking") { // Kicking Action
        stun = 1;
        body->Parent()->SendOutF(
            ALL, -1, "*;s kicks ;s%s.\n", "*You kick ;s%s.\n", body, targ, locm.c_str());
      } else if (
          body->IsAct(ACT_WIELD) // Ranged Weapon
          && body->ActTarg(ACT_WIELD)->Skill("WeaponReach") > 9) {
        body->Parent()->SendOutF(
            ALL,
            -1,
            "*;s throws %s and hits ;s%s.\n",
            "*You throw %s and hit ;s%s.\n",
            body,
            targ,
            body->ActTarg(ACT_WIELD)->ShortDesc(),
            locm.c_str());
        body->ActTarg(ACT_WIELD)->Travel(body->Parent()); // FIXME: Get Another
        body->StopAct(ACT_WIELD); // FIXME: Bows/Guns!
      } else if (body->IsAct(ACT_WIELD)) { // Melee Weapon
        body->Parent()->SendOutF(
            ALL,
            -1,
            "*;s hits ;s%s with %s.\n",
            "*You hit ;s%s with %s.\n",
            body,
            targ,
            locm.c_str(),
            body->ActTarg(ACT_WIELD)->ShortDesc());
      } else { // No Weapon or Natural Weapon
        if (!body->HasSkill("NaturalWeapon"))
          stun = 1;
        char mes[128] = "";
        char mes3[128] = "";
        sprintf(mes, "*You %s ;s%s.\n", verb, locm.c_str());
        sprintf(mes3, "*;s %s ;s%s.\n", verb3, locm.c_str());
        body->Parent()->SendOut(ALL, -1, mes3, mes, body, targ);
      }

      int sev = 0;
      int force = body->ModAttribute(2) + body->Modifier("Damage");

      if (com == COM_KICK) {
        force -= 2;
        stage += 2;
      }
      if (body->IsAct(ACT_WIELD)) {
        force += body->ActTarg(ACT_WIELD)->Skill("WeaponForce");
        if (two_handed(body->ActTarg(ACT_WIELD)->Skill("WeaponType"))) {
          force += body->ModAttribute(2);
        }
        stage += body->ActTarg(ACT_WIELD)->Skill("WeaponSeverity");
      } else {
        force -= 1;
        stage += 1;
      }

      if (targ->ActTarg(loca)) {
        // FIXME: Implement the rest of the Armor Effect types
        succ -= roll(targ->ActTarg(loca)->ModAttribute(0), force);
      }

      if (stun) {
        sev = targ->HitStun(force, stage, succ);
      } else {
        sev = targ->HitPhys(force, stage, succ);
      }
      if (body->Skill("Poisonous") > 0) { // Injects poison!
        targ->SetSkill("Poisoned", targ->Skill("Poisoned") + body->Skill("Poisonous"));
      }

      if (sev <= 0) {
        if (mind)
          mind->Send("You hit - but didn't do much.\n"); // FIXME - Real
        // Messages
      }
    } else {
      if (com == COM_KICK) { // Kicking Action
        body->Parent()->SendOut(
            ALL,
            -1,
            ";s tries to kick ;s, but misses.\n",
            "You try to kick ;s, but miss.\n",
            body,
            targ);
      } else if (
          body->IsAct(ACT_WIELD) // Ranged Weapon
          && body->ActTarg(ACT_WIELD)->Skill("WeaponReach") > 9) {
        body->Parent()->SendOutF(
            ALL,
            -1,
            "*;s throws %s at ;s, but misses.\n",
            "*You throw %s at ;s, but miss.\n",
            body,
            targ,
            body->ActTarg(ACT_WIELD)->ShortDesc());
        body->ActTarg(ACT_WIELD)->Travel(body->Parent()); // FIXME: Get Another
        body->StopAct(ACT_WIELD); // FIXME: Bows/Guns!
      } else if (body->IsAct(ACT_WIELD)) { // Melee Weapon
        body->Parent()->SendOut(
            ALL,
            -1,
            ";s tries to attack ;s, but misses.\n",
            "You try to attack ;s, but miss.\n",
            body,
            targ);
      } else { // Unarmed
        body->Parent()->SendOutF(
            ALL,
            -1,
            ";s tries to %s ;s, but misses.\n",
            "You try to %s ;s, but miss.\n",
            body,
            targ,
            verb);
      }
    }

    if (!targ->IsAnimate() || targ->IsAct(ACT_DEAD) || targ->IsAct(ACT_DYING) ||
        targ->IsAct(ACT_UNCONSCIOUS)) {
      body->StopAct(ACT_FIGHT);
      body->BusyFor(3000);
      if (targ->Skill("Accomplishment")) {
        if (body->Accomplish(targ->Skill("Accomplishment"))) {
          if (mind) {
            mind->SendF(
                "%sYour character gains an experience point for this "
                "victory!\n%s",
                CYEL,
                CNRM);
          }
        }
        targ->SetSkill("Accomplishment", 0);
      }
    }

    return 0;
  }

  const char* statnames[] = {
      "Body", "Quickness", "Strength", "Charisma", "Intelligence", "Willpower"};

  if (com == COM_RESETCHARACTER) {
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;

    Object* chr = mind->Owner()->Creator();
    if (!chr) {
      mind->Send(
          "You need to be working on a character first (use 'select "
          "<character>'.\n");
      return 0;
    } else if (strlen(cmd.c_str() + len) > 0) {
      mind->SendF(
          "Just type 'reset' to undo all your work and start over on %s\n", chr->ShortDesc());
      return 0;
    }

    if (chr->Exp() > 0) {
      mind->Send("This is not a new character, you can't modify the chargen steps anymore.\n");
      return 0;
    }

    body = new_body();
    body->SetShortDesc(chr->ShortDesc());
    mind->Owner()->AddChar(body);
    delete chr;

    mind->SendF("You reset all chargen work for '%s'.\n", body->ShortDesc());
    return 0;
  }

  if (com == COM_RANDOMIZE) {
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;

    Object* chr = mind->Owner()->Creator();
    if (!chr) {
      mind->Send(
          "You need to be working on a character first (use 'select "
          "<character>'.\n");
      return 0;
    } else if (strlen(cmd.c_str() + len) > 0) {
      mind->SendF("Just type 'randomize' to randomly spend all points for %s\n", chr->ShortDesc());
      return 0;
    }

    if (chr->Exp() > 0) {
      mind->Send("This is not a new character, you can't modify the chargen steps anymore.\n");
      return 0;
    }

    while (chr->Skill("Attribute Points") > 0) {
      int which = (rand() % 6);
      if (chr->NormAttribute(which) < 6) {
        chr->SetSkill("Attribute Points", chr->Skill("Attribute Points") - 1);
        chr->SetAttribute(which, chr->NormAttribute(which) + 1);
      }
    }

    auto skills = get_skills("all");
    while (chr->Skill("Skill Points")) {
      int which = (rand() % skills.size());
      auto skl = skills.begin();
      while (which) {
        ++skl;
        --which;
      }
      if (chr->Skill(*skl) < (chr->NormAttribute(get_linked(*skl)) + 1) / 2 &&
          chr->Skill("Skill Points") > chr->Skill(*skl)) {
        chr->SetSkill(*skl, chr->Skill(*skl) + 1);
        chr->SetSkill("Skill Points", chr->Skill("Skill Points") - chr->Skill(*skl));
      }
    }
    mind->SendF("You randomly spend all remaining points for '%s'.\n", chr->ShortDesc());
    return 0;
  }

  if (com == COM_ARCHTYPE) {
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;

    Object* chr = mind->Owner()->Creator();
    if (!chr) {
      mind->Send(
          "You need to be working on a character first (use 'select "
          "<character>'.\n");
      return 0;
    } else if (strlen(cmd.c_str() + len) <= 0) {
      mind->SendF("You need to select an archtype to apply to %s.\n", chr->ShortDesc());
      mind->SendF("Supported archtypes are:\n");
      mind->SendF("  1. Fighter\n");
      return 0;
    }

    if (chr->Exp() > 0) {
      mind->Send("This is not a new character, you can't modify the chargen steps anymore.\n");
      return 0;
    }

    if (!strncmp(cmd.c_str() + len, "fighter", strlen(cmd.c_str() + len))) {
      body = new_body();
      body->SetShortDesc(chr->ShortDesc());
      mind->Owner()->AddChar(body);
      delete chr;

      body->SetAttribute(0, 5);
      body->SetAttribute(1, 6);
      body->SetAttribute(2, 6);
      body->SetAttribute(3, 2);
      body->SetAttribute(4, 6);
      body->SetAttribute(5, 5);
      body->SetSkill("Attribute Points", 0);

      body->SetSkill("Long Blades", 3);
      body->SetSkill("Two-Handed Blades", 3);
      body->SetSkill("Short Piercing", 3);
      body->SetSkill("Shields", 3);
      body->SetSkill("Running", 2);
      body->SetSkill("Climbing", 2);
      body->SetSkill("Sprinting", 2);
      body->SetSkill("Swimming", 2);
      body->SetSkill("Lifting", 2);
      body->SetSkill("Acrobatics", 2);
      body->SetSkill("Punching", 3);
      body->SetSkill("Kicking", 3);
      body->SetSkill("Grappling", 3);
      body->SetSkill("Skill Points", 4);

      auto weap = new Object(body);
      weap->SetShortDesc("a dull arming sword");
      weap->SetDesc("This sword really isn't that great.  Is even metal?  It's sure not steel.");
      weap->SetSkill("WeaponType", get_weapon_type("Long Blades"));
      weap->SetSkill("WeaponForce", -2);
      weap->SetSkill("WeaponSeverity", 1);
      weap->SetSkill("WeaponReach", 1);
      body->AddAct(ACT_WIELD, weap);

      auto shi = new Object(body);
      shi->SetShortDesc("a cracked leather shield");
      shi->SetDesc("This shield has seen better days... but, it was pretty bad back then too.");
      shi->SetSkill("Wearable on Shield", 1);
      shi->SetAttribute(0, 1);
      body->AddAct(ACT_WEAR_SHIELD, shi);

      auto arm = new Object(body);
      arm->SetShortDesc("a full suit of old padded armor");
      arm->SetDesc("This armor smells pretty bad, but it's better than nothing... maybe.");
      arm->SetSkill("Wearable on Back", 1);
      arm->SetSkill("Wearable on Chest", 1);
      arm->SetSkill("Wearable on Left Arm", 1);
      arm->SetSkill("Wearable on Right Arm", 1);
      arm->SetSkill("Wearable on Left Leg", 1);
      arm->SetSkill("Wearable on Right Leg", 1);
      arm->SetAttribute(0, 1);
      body->AddAct(ACT_WEAR_BACK, arm);
      body->AddAct(ACT_WEAR_CHEST, arm);
      body->AddAct(ACT_WEAR_LARM, arm);
      body->AddAct(ACT_WEAR_RARM, arm);
      body->AddAct(ACT_WEAR_LLEG, arm);
      body->AddAct(ACT_WEAR_RLEG, arm);

      auto helm = new Object(body);
      helm->SetShortDesc("a soft leather cap");
      helm->SetDesc("This is... armor... probably.");
      helm->SetSkill("Wearable on Head", 1);
      helm->SetAttribute(0, 1);
      body->AddAct(ACT_WEAR_HEAD, helm);

      body->SetSkill("Status Points", 0);

      mind->SendF("You reform '%s' into a %s.\n", body->ShortDesc(), "Fighter");
      mind->SendF("You now have only a few points left to spend.\n");

    } else {
      mind->SendF("You need to select a *supported* archtype to apply to %s.\n", chr->ShortDesc());
      mind->SendF("Supported archtypes are:\n");
      mind->SendF("  1. Fighter\n");
    }

    return 0;
  }

  if (com == COM_LOWER) {
    if ((!mind) || (!mind->Owner()))
      return 0;

    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;

    Object* chr = mind->Owner()->Creator();
    if (!chr) {
      mind->Send("You need to be working on a character first (use 'select <character>'.\n");
      return 0;
    }

    if (chr->Exp() > 0) {
      mind->Send("This is not a new character, you can't modify the chargen steps anymore.\n");
      return 0;
    }

    if ((!strncmp(cmd.c_str() + len, "body", strlen(cmd.c_str() + len))) ||
        (!strncmp(cmd.c_str() + len, "quickness", strlen(cmd.c_str() + len))) ||
        (!strncmp(cmd.c_str() + len, "strength", strlen(cmd.c_str() + len))) ||
        (!strncmp(cmd.c_str() + len, "charisma", strlen(cmd.c_str() + len))) ||
        (!strncmp(cmd.c_str() + len, "intelligence", strlen(cmd.c_str() + len))) ||
        (!strncmp(cmd.c_str() + len, "willpower", strlen(cmd.c_str() + len)))) {
      int attr = 0;
      if (toupper(*(cmd.c_str() + len)) == 'B')
        attr = 0;
      else if (toupper(*(cmd.c_str() + len)) == 'Q')
        attr = 1;
      else if (toupper(*(cmd.c_str() + len)) == 'S')
        attr = 2;
      else if (toupper(*(cmd.c_str() + len)) == 'C')
        attr = 3;
      else if (toupper(*(cmd.c_str() + len)) == 'I')
        attr = 4;
      else if (toupper(*(cmd.c_str() + len)) == 'W')
        attr = 5;

      if (chr->NormAttribute(attr) < 3) {
        mind->SendF("Your %s is already at the minimum.\n", statnames[attr]);
        return 0;
      } else {
        chr->SetAttribute(attr, chr->NormAttribute(attr) - 1);
        chr->SetSkill("Attribute Points", chr->Skill("Attribute Points") + 1);
        mind->SendF("You lower your %s.\n", statnames[attr]);
      }
    } else {
      std::string skill = get_skill(cmd.c_str() + len);
      if (skill != "") {
        if (chr->Skill(skill) < 1) {
          mind->SendF("You don't have %s.\n", skill.c_str());
          return 0;
        }
        chr->SetSkill(skill, chr->Skill(skill) - 1);
        chr->SetSkill("Skill Points", chr->Skill("Skill Points") + chr->Skill(skill) + 1);
        mind->SendF("You lower your %s skill.\n", skill.c_str());
      } else {
        mind->Send("I'm not sure what you are trying to lower.\n");
      }
    }
    return 0;
  }

  if (com == COM_RAISE) {
    if ((!mind) || (!mind->Owner()))
      return 0;
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;
    Object* chr = body;
    if (!chr) {
      chr = mind->Owner()->Creator();
      if (!chr) {
        mind->Send("You need to be working on a character first (use 'select <character>'.\n");
        return 0;
      } else if (chr->Exp() > 0) {
        mind->Send("This is not a new character, you can't modify the chargen steps anymore.\n");
        return 0;
      }
    }
    if (strlen(cmd.c_str() + len) <= 0) {
      mind->Send("What do you want to buy?\n");
      return 0;
    }

    if ((!strncmp(cmd.c_str() + len, "body", strlen(cmd.c_str() + len))) ||
        (!strncmp(cmd.c_str() + len, "quickness", strlen(cmd.c_str() + len))) ||
        (!strncmp(cmd.c_str() + len, "strength", strlen(cmd.c_str() + len))) ||
        (!strncmp(cmd.c_str() + len, "charisma", strlen(cmd.c_str() + len))) ||
        (!strncmp(cmd.c_str() + len, "intelligence", strlen(cmd.c_str() + len))) ||
        (!strncmp(cmd.c_str() + len, "willpower", strlen(cmd.c_str() + len)))) {
      if ((!body) && (chr->Skill("Attribute Points") < 1)) {
        mind->Send("You have no free attribute points left.\n");
        return 0;
      }
      int attr = 0;
      if (toupper(*(cmd.c_str() + len)) == 'B')
        attr = 0;
      else if (toupper(*(cmd.c_str() + len)) == 'Q')
        attr = 1;
      else if (toupper(*(cmd.c_str() + len)) == 'S')
        attr = 2;
      else if (toupper(*(cmd.c_str() + len)) == 'C')
        attr = 3;
      else if (toupper(*(cmd.c_str() + len)) == 'I')
        attr = 4;
      else if (toupper(*(cmd.c_str() + len)) == 'W')
        attr = 5;

      if (body && chr->TotalExp(mind->Owner()) < 20) {
        mind->SendF(
            "You don't have enough experience to raise your %s.\n"
            "You need 20, but you only have %d\n",
            statnames[attr],
            chr->TotalExp(mind->Owner()));
        return 0;
      }

      const char* maxask[6] = {"MaxBody",
                               "MaxQuickness",
                               "MaxStrength",
                               "MaxCharisma",
                               "MaxIntelligence",
                               "MaxWillpower"};
      if ((body) && (!body->Skill(maxask[attr]))) {
        body->SetSkill(maxask[attr], (body->NormAttribute(attr) * 3) / 2);
      }

      if ((!body) && chr->NormAttribute(attr) >= 6) {
        mind->SendF("Your %s is already at the maximum.\n", statnames[attr]);
      } else if (body && chr->NormAttribute(attr) >= body->Skill(maxask[attr])) {
        mind->SendF("Your %s is already at the maximum.\n", statnames[attr]);
      } else {
        if (!body)
          chr->SetSkill("Attribute Points", chr->Skill("Attribute Points") - 1);
        else
          chr->SpendExp(20);
        chr->SetAttribute(attr, chr->NormAttribute(attr) + 1);
        mind->SendF("You raise your %s.\n", statnames[attr]);
      }
    } else {
      std::string skill = get_skill(cmd.c_str() + len);
      if (skill != "") {
        if (body && (chr->Skill(skill) >= (chr->NormAttribute(get_linked(skill)) * 3 + 1) / 2)) {
          mind->SendF("Your %s is already at the maximum.\n", skill.c_str());
          return 0;
        } else if (
            (!body) && (chr->Skill(skill) >= (chr->NormAttribute(get_linked(skill)) + 1) / 2)) {
          mind->SendF("Your %s is already at the maximum.\n", skill.c_str());
          return 0;
        }
        int cost = (chr->Skill(skill) + 1);
        if (body) {
          if (cost > chr->NormAttribute(get_linked(skill)))
            cost *= 2;
          if (chr->TotalExp(mind->Owner()) < (cost * 2)) {
            mind->SendF(
                "You don't have enough experience to raise your %s.\n"
                "You need %d, but you only have %d\n",
                skill.c_str(),
                cost * 2,
                chr->TotalExp(mind->Owner()));
            return 0;
          }
        } else if (!chr->Skill("Skill Points")) {
          mind->Send("You have no free skill points left.\n");
          return 0;
        } else if (chr->Skill("Skill Points") < cost) {
          mind->Send("You don't have enough free skill points left.\n");
          return 0;
        }
        if (body)
          chr->SpendExp(cost * 2);
        else
          chr->SetSkill("Skill Points", chr->Skill("Skill Points") - cost);
        chr->SetSkill(skill, chr->Skill(skill) + 1);
        mind->SendF("You raise your %s skill.\n", skill.c_str());
      } else {
        mind->Send("I'm not sure what you are trying to raise.\n");
      }
    }
    return 0;
  }

  if (com == COM_NEWCHARACTER) {
    if (!mind)
      return 0; // FIXME: Should never happen!
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;
    if (len >= int(cmd.length())) {
      mind->Send("What's the character's name?  Use 'newcharacter <charname>'.\n");
      return 0;
    }

    const char* ch = cmd.c_str() + len;
    while ((*ch) && isalpha(*ch))
      ++ch;
    if (*ch) {
      mind->Send(
          "Sorry, character names can only contain letters.\n"
          "Pick another name.\n");
      return 0;
    }

    body = mind->Owner()->Room()->PickObject(cmd.c_str() + len, vmode | LOC_INTERNAL);
    if (body) {
      mind->Send(
          "Sorry, you already have a character with that name.\n"
          "Pick another name.\n");
      return 0;
    }
    body = new_body();
    body->SetShortDesc(cmd.c_str() + len);
    mind->Owner()->AddChar(body);
    mind->SendF("You created %s.\n", cmd.c_str() + len);
    return 0;
  }

  if (com == COM_RECALL) {
    if (body->Phys() || body->Stun()) {
      if (mind)
        mind->Send("You must be uninjured to use that command!\n");
    } else {
      Object* dest = body;
      while ((!dest->ActTarg(ACT_SPECIAL_HOME)) && dest->Parent()) {
        dest = dest->Parent();
      }
      if (dest->ActTarg(ACT_SPECIAL_HOME)) {
        dest = dest->ActTarg(ACT_SPECIAL_HOME);
      }
      body->Parent()->SendOut(
          0,
          0, // Not Stealthy!
          "BAMF! ;s teleports away.\n",
          "BAMF! You teleport home.\n",
          body,
          nullptr);
      body->Travel(dest, 0);
      body->Parent()->SendOut(
          0,
          0, // Not Stealthy!
          "BAMF! ;s teleports here.\n",
          "",
          body,
          nullptr);
      if (mind && mind->Type() == MIND_REMOTE)
        body->Parent()->SendDescSurround(body, body);
    }
    return 0;
  }

  if (com == COM_TELEPORT) {
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;
    if (len >= int(cmd.length())) {
      mind->Send("Where do you want to teleport to?.\n");
      return 0;
    }

    Object* src = nullptr;
    if (!nmode)
      src = body->NextHasSkill("Restricted Item");
    while (src) {
      if (!src->HasSkill("Teleport")) {
        src = body->NextHasSkill("Restricted Item", src);
        continue;
      }
      std::string comline = "teleport ";
      comline += (cmd.c_str() + len);
      comline += "\n";
      if (!strcasestr(src->LongDesc(), comline.c_str())) {
        src = body->NextHasSkill("Restricted Item", src);
        continue;
      }
      break;
    }

    if ((!nmode) && (!src) && body->Skill("Teleport") < 1) {
      if (mind)
        mind->Send("You don't have the power to teleport!\n");
      return 0;
    }
    if ((!body->Parent()) || (!body->Parent()->Parent())) {
      if (mind)
        mind->Send("You can't teleport from here!\n");
      return 0;
    }

    Object* dest = body->Parent();
    if (nmode && (!strcmp(cmd.c_str() + len, "universe"))) {
      dest = dest->Universe();
    } else if (nmode && (!strcmp(cmd.c_str() + len, "trashbin"))) {
      dest = dest->TrashBin();
    } else { // Only Ninjas can teleport to "Universe"/"TrashBin"
      while (dest->Parent()->Parent()) {
        dest = dest->Parent();
      }
      dest = dest->PickObject(cmd.c_str() + len, vmode | LOC_INTERNAL);
    }

    if (!dest) {
      if (mind)
        mind->Send("No such teleportation destination found.\n");
    } else {
      body->SetSkill("Teleport", 0); // Use it up
      body->Parent()->SendOut(
          0,
          0, // Not Stealthy!
          "BAMF! ;s teleports away.\n",
          "BAMF! You teleport.\n",
          body,
          nullptr);
      body->Travel(dest, 0);
      body->Parent()->SendOut(
          0,
          0, // Not Stealthy!
          "BAMF! ;s teleports here.\n",
          "",
          body,
          nullptr);
      if (mind && mind->Type() == MIND_REMOTE)
        body->Parent()->SendDescSurround(body, body);
    }
    return 0;
  }

  if (com == COM_RESURRECT) {
    if ((!nmode) && body->Skill("Resurrect") < 1) {
      if (mind)
        mind->Send("You don't have the power to resurrect!\n");
      return 0;
    }
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;
    if (len >= int(cmd.length())) {
      mind->Send("Who do you want to resurrect?.\n");
      return 0;
    }

    std::vector<Player*> pls = get_all_players();
    for (auto pl : pls) {
      auto chs = pl->Room()->Contents();
      for (auto ch : chs) {
        if (ch->Matches(cmd.c_str() + len)) {
          if (ch->IsActive()) {
            if (mind)
              mind->SendF("%s is not long dead (yet).\n", ch->Name());
          } else {
            body->SetSkill("Resurrect", 0); // Use it up
            ch->SetSkill("Poisoned", 0);
            ch->SetSkill("Thirsty", 0);
            ch->SetSkill("Hungry", 0);
            ch->SetPhys(0);
            ch->SetStun(0);
            ch->SetStru(0);
            ch->UpdateDamage();
            ch->Activate();
            ch->Parent()->SendOut(
                stealth_t, stealth_s, ";s has been resurrected!.\n", "", ch, nullptr);
            if (mind)
              mind->SendF("%s has been resurrected!\n", ch->Name());
          }
          return 0;
        }
      }
    }
    if (mind)
      mind->SendF("%s isn't a character on this MUD\n", cmd.c_str() + len);
    return 0;
  }

  if (com == COM_SKILLLIST) {
    if (!mind)
      return 0;

    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;

    std::string skills;
    std::vector<std::string> skls;
    if (len >= int(cmd.length())) {
      skills =
          "Here are all the skill categories (use 'skill <Category>' to "
          "see the skills):\n";
      skls = get_skills();
    } else {
      std::string cat = get_skill_cat(cmd.c_str() + len);
      if (std::string(cmd.c_str() + len) != "all") {
        if (cat == "") {
          mind->SendF("There is no skill category called '%s'.\n", cmd.c_str() + len);
          return 0;
        }
        skills = "Total " + cat + " in play on this MUD:\n";
      } else {
        skills = "Total skills in play on this MUD:\n";
        cat = "all";
      }
      skls = get_skills(cat);
    }

    for (auto skl : skls) {
      skills += skl;
      skills += "\n";
    }
    mind->Send(skills.c_str());

    return 0;
  }

  if (com == COM_TOGGLE) {
    if (!mind)
      return 0;
    Player* pl = mind->Owner();
    if (!pl)
      return 0;

    if (!cmd.empty() && (!strncmp(cmd.c_str(), "restore", cmd.length()))) {
      mind->SetSVars(pl->Vars());
      mind->Send("Your settings have been reset to your defaults.\n");
      cmd = ""; // Show current settings too
    }

    if (!cmd.empty()) {
      mind->Send("Your current settings:\n");

      if (mind->IsSVar("combatinfo")) {
        mind->Send("  CombatInfo is " CYEL "on" CNRM ".\n");
      } else {
        mind->Send("  CombatInfo is " CYEL "off" CNRM ".\n");
      }

    } else if (!strncmp(cmd.c_str(), "combatinfo", cmd.length())) {
      if (mind->IsSVar("combatinfo")) {
        mind->ClearSVar("combatinfo");
        mind->Send("CombatInfo is now " CYEL "off" CNRM ".\n");
      } else {
        mind->SetSVar("combatinfo", "1");
        mind->Send("CombatInfo is now " CYEL "on" CNRM ".\n");
      }
    } else if (!strncmp(cmd.c_str(), "save", cmd.length())) {
      pl->SetVars(mind->SVars());
      mind->Send("Your current settings have been saved as your defaults.\n");
    } else {
      mind->Send("Don't know what setting you want to change.\n");
      mind->Send("Just type " CYEL "toggle" CNRM " to see a full list.\n");
      return 0;
    }
    if (mind->SVars() != pl->Vars()) {
      mind->Send(CYEL "\nYour current settings are not all saved:\n" CNRM);
      mind->Send("  Type '" CMAG "toggle save" CNRM "' to make this change permanent.\n");
      mind->Send("  Type '" CMAG "toggle restore" CNRM "' to restore your default settings.\n");
    }
    return 0;
  }

  if (com == COM_WHO) {
    if (!mind)
      return 0;
    std::string users = "Currently on this MUD:\n";
    std::vector<Mind*> mns = get_human_minds();

    for (auto mn : mns) {
      users += mn->Owner()->Name();
      if (mn->Body())
        users = users + " as " + mn->Body()->ShortDesc() + ".\n";
      else
        users += " in character room.\n";
    }
    mind->Send(users.c_str());

    return 0;
  }

  if (com == COM_OOC) {
    if (!mind)
      return 0;
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;
    if (strlen(cmd.c_str() + len) <= 0) {
      if (strncmp(mind->SpecialPrompt(), "ooc", 3)) {
        mind->SetSpecialPrompt("ooc");
        mind->Send("Type your out-of-character text - exit by just hitting ENTER:");
      } else {
        mind->SetSpecialPrompt("");
        mind->Send("Exiting out of out-of-character mode.");
      }
    } else {
      std::string name = "Unknown";
      if (mind->Owner())
        name = mind->Owner()->Name();
      std::string mes = std::string("OOC: <") + name + "> " + (cmd.c_str() + len) + "\n";
      std::vector<Mind*> mns = get_human_minds();
      for (auto mn : mns) {
        mn->Send(mes.c_str());
      }
    }
    return 0;
  }

  if (com == COM_NEWBIE) {
    if (!mind)
      return 0;
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;
    if (strlen(cmd.c_str() + len) <= 0) {
      if (strncmp(mind->SpecialPrompt(), "newbie", 3)) {
        mind->SetSpecialPrompt("newbie");
        mind->Send("Type your newbie-chat text - exit by just hitting ENTER:");
      } else {
        mind->SetSpecialPrompt("");
        mind->Send("Exiting out of newbie-chat mode.");
      }
    } else {
      std::string name = "Unknown";
      if (mind->Owner())
        name = mind->Owner()->Name();
      std::string mes = std::string("NEWBIE: <") + name + "> " + (cmd.c_str() + len) + "\n";
      std::vector<Mind*> mns = get_human_minds();
      for (auto mn : mns) {
        mn->Send(mes.c_str());
      }
    }
    return 0;
  }

  // Ninja Commands

  if (com == COM_NINJAMODE) {
    if (!mind)
      return 0;
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;

    Player* pl = mind->Owner();
    if (strlen(cmd.c_str() + len) <= 0) {
      mind->Send("What command do you want to run in Ninja Mode[TM]?\n");
      mind->Send("You need to include the command, like 'ninja junk boat'.\n");
    } else if (!pl) {
      mind->Send("Sorry, you don't seem to be a player!\n");
    } else {
      pl->Set(PLAYER_NINJAMODE);
      mind->Send("Ninja mode activated.\n");
      handle_single_command(body, cmd.c_str() + len, mind);
      pl->UnSet(PLAYER_NINJAMODE);
      mind->Send("Ninja mode deactivated.\n");
    }
    return 0;
  }

  if (com == COM_MAKENINJA) {
    if (!mind)
      return 0;
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;

    Player* pl = get_player(cmd.c_str() + len);
    if (strlen(cmd.c_str() + len) <= 0) {
      mind->Send("You can only make/unmake other True Ninjas[TM]\n");
      return 0;
    }
    if (!pl) {
      mind->SendF("There is no PLAYER named '%s'\n", cmd.c_str() + len);
      return 0;
    }
    if (pl == mind->Owner()) {
      mind->Send("You can only make/unmake other True Ninjas[TM]\n");
      return 0;
    }

    if (pl->Is(PLAYER_SUPERNINJA)) {
      mind->SendF("'%s' is already a Super Ninja[TM] - this is irrevocable.\n", cmd.c_str() + len);
    } else if (pl->Is(PLAYER_NINJA)) {
      pl->UnSet(PLAYER_NINJA);
      pl->UnSet(PLAYER_NINJAMODE);
      mind->SendF("Now '%s' is no longer a True Ninja[TM].\n", cmd.c_str() + len);
    } else {
      pl->Set(PLAYER_NINJA);
      mind->SendF("You made '%s' into a True Ninja[TM].\n", cmd.c_str() + len);
    }

    return 0;
  }

  if (com == COM_MAKESUPERNINJA) {
    if (!mind)
      return 0;
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;

    Player* pl = get_player(cmd.c_str() + len);
    if (strlen(cmd.c_str() + len) <= 0) {
      mind->Send("You can only make other Super Ninjas[TM]\n");
      return 0;
    }
    if (!pl) {
      mind->SendF("There is no PLAYER named '%s'\n", cmd.c_str() + len);
      return 0;
    }
    if (pl == mind->Owner()) {
      mind->Send("You can only make other Super Ninjas[TM]\n");
      return 0;
    }

    if (pl->Is(PLAYER_SUPERNINJA)) {
      mind->SendF("'%s' is already a Super Ninja[TM] - this is irrevocable.\n", cmd.c_str() + len);
    } else if (!pl->Is(PLAYER_NINJA)) {
      mind->SendF(
          "'%s' isn't even a True Ninja[TM] yet!\n"
          "Be careful - Super Ninja[TM] status is irrevocable.\n",
          cmd.c_str() + len);
    } else {
      pl->Set(PLAYER_SUPERNINJA);
      mind->SendF("You made '%s' into a Super Ninja[TM].\n", cmd.c_str() + len);
    }

    return 0;
  }

  if (com == COM_MAKESTART) {
    if (!mind)
      return 0;
    Object* world = body->World();
    world->AddAct(ACT_SPECIAL_HOME, body->Parent());
    world->Parent()->AddAct(ACT_SPECIAL_HOME, body->Parent());
    mind->Send("You make this the default starting room for players.\n");
    return 0;
  }

  if (com == COM_SETSTATS) {
    if (!mind)
      return 0;
    Object* targ = body->ActTarg(ACT_POINT);
    if (!targ) {
      mind->Send("You need to be pointing at your target.\n");
      return 0;
    }
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;
    if (len < int(cmd.length())) {
      char gender = 0;
      int num, weight = 0, size = 0, volume = 0, value = 0;
      num =
          sscanf(cmd.c_str() + len, "%d %d %d %d %c;\n", &weight, &size, &volume, &value, &gender);
      if (num > 0)
        targ->SetWeight(weight);
      if (num > 1)
        targ->SetSize(size);
      if (num > 3)
        targ->SetVolume(volume);
      if (num > 3)
        targ->SetValue(value);
      if (gender == 'N' || gender == 'M' || gender == 'F') {
        targ->SetGender(gender);
      }
    } else {
      mind->Send("Set stats to what?\n");
    }
    return 0;
  }

  if (com == COM_NAME) {
    if (!mind)
      return 0;
    Object* targ = body->ActTarg(ACT_POINT);
    if (!targ) {
      mind->Send("You need to be pointing at your target.\n");
      return 0;
    }
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;
    if (len < int(cmd.length())) {
      std::string oldn = targ->ShortDesc();
      targ->SetShortDesc(cmd.c_str() + len);
      mind->SendF(
          "You rename '%s' to '%s'\n",
          oldn.c_str(),
          targ->ShortDesc()); // FIXME - Real Message
    } else {
      mind->Send("Rename it to what?\n");
    }
    return 0;
  }

  if (com == COM_UNDESCRIBE) {
    if (!mind)
      return 0;
    Object* targ = body->ActTarg(ACT_POINT);
    if (!targ) {
      mind->Send("You need to be pointing at your target.\n");
      return 0;
    }
    targ->SetDesc("");
    mind->SendF("You remove the description from '%s'\n", targ->Name(0, body));
    return 0;
  }

  if (com == COM_DESCRIBE) {
    if (!mind)
      return 0;
    Object* targ = body->ActTarg(ACT_POINT);
    if (!targ) {
      mind->Send("You need to be pointing at your target.\n");
      return 0;
    }
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;
    if (len < int(cmd.length())) {
      if (targ->Desc() == targ->ShortDesc()) {
        targ->SetDesc(cmd.c_str() + len);
        mind->SendF("You add a description to '%s'\n", targ->Name(0, body));
      } else {
        targ->SetDesc(targ->Desc() + std::string("\n") + (cmd.c_str() + len));
        mind->SendF("You add to the description of '%s'\n", targ->Name(0, body));
      }
    } else {
      if (strncmp(mind->SpecialPrompt(), "nin des", 7)) {
        mind->SetSpecialPrompt("nin des");
        mind->Send("Type the description - exit by just hitting ENTER:");
      } else {
        mind->SetSpecialPrompt("");
        mind->Send("Exiting out of describe mode.");
      }
      return 0;
    }
    return 0;
  }

  if (com == COM_UNDEFINE) {
    if (!mind)
      return 0;
    Object* targ = body->ActTarg(ACT_POINT);
    if (!targ) {
      mind->Send("You need to be pointing at your target.\n");
      return 0;
    }
    targ->SetLongDesc("");
    mind->SendF("You remove the definition from '%s'\n", targ->Name(0, body));
    return 0;
  }

  if (com == COM_DEFINE) {
    if (!mind)
      return 0;
    Object* targ = body->ActTarg(ACT_POINT);
    if (!targ) {
      mind->Send("You need to be pointing at your target.\n");
      return 0;
    }
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;
    if (len < int(cmd.length())) {
      if (targ->LongDesc() == targ->Desc()) {
        targ->SetLongDesc(cmd.c_str() + len);
        mind->SendF("You add a definition to '%s'\n", targ->Name(0, body));
      } else {
        targ->SetLongDesc(targ->LongDesc() + std::string("\n") + (cmd.c_str() + len));
        mind->SendF("You add to the definition of '%s'\n", targ->Name(0, body));
      }
    } else {
      if (strncmp(mind->SpecialPrompt(), "nin def", 7)) {
        mind->SetSpecialPrompt("nin def");
        mind->Send("Type the definition - exit by just hitting ENTER:");
      } else {
        mind->SetSpecialPrompt("");
        mind->Send("Exiting out of define mode.");
      }
      return 0;
    }
    return 0;
  }

  if (com == COM_CONTROL) {
    if (!mind)
      return 0;
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;
    Object* targ = body->PickObject(cmd.c_str() + len, vmode | LOC_NEARBY);
    if (!targ) {
      mind->Send("You want to control who?\n");
    } else if (!targ->IsAnimate()) {
      mind->Send("You can't control inanimate objects!\n");
    } else {
      body->Parent()->SendOut(
          stealth_t,
          stealth_s,
          ";s controls ;s with Ninja Powers[TM].\n",
          "You control ;s.\n",
          body,
          targ);
      mind->Unattach();
      mind->Attach(targ);
    }
    return 0;
  }

  if (com == COM_COMMAND) {
    if (!mind)
      return 0;
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;
    Object* targ = body->ActTarg(ACT_POINT);
    if (!targ) {
      mind->Send("You need to be pointing at your target.\n");
    } else if (len >= int(cmd.length())) {
      mind->SendF("Command %s to do what?\n", targ->ShortDesc());
    } else if (targ->NormAttribute(5) <= 0) {
      mind->Send("You can't command an object that has no will of its own.\n");
    } else {
      body->Parent()->SendOutF(
          stealth_t,
          stealth_s,
          ";s commands ;s to '%s' with Ninja Powers[TM].\n",
          "You command ;s to '%s'.\n",
          body,
          targ,
          cmd.c_str() + len);

      if (handle_command(targ, cmd.c_str() + len) > 0)
        body->Parent()->SendOut(
            stealth_t,
            stealth_s,
            ";s did not understand the command.\n",
            ";s did not understand the command.\n",
            targ,
            body);
    }
    return 0;
  }

  if (com == COM_CONNECT) {
    Object *src = body->ActTarg(ACT_POINT), *dest = nullptr;
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;
    if (!src) {
      mind->Send("You need to be pointing at your source.\n");
      return 0;
    }
    dest = body->PickObject(
        cmd.c_str() + len, vmode | LOC_INTERNAL | LOC_NEARBY | LOC_SELF | LOC_HERE);
    if (!dest) {
      if (mind)
        mind->Send("I can't find that destination.\n");
    } else {
      Object* exit = new Object(dest);
      exit->SetShortDesc("a passage exit");
      exit->SetDesc("A passage exit.");
      exit->SetSkill("Invisible", 1000);
      src->SetSkill("Open", 1000);
      src->SetSkill("Enterable", 1);
      src->AddAct(ACT_SPECIAL_LINKED, exit);
      exit->AddAct(ACT_SPECIAL_MASTER, src);
      if (mind) {
        mind->SendF("You link %s to %s.\n", src->Name(0, body), dest->Name(0, body));
      }
    }
    return 0;
  }

  if (com == COM_CREATE) {
    if (!mind)
      return 0;
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;
    Object* obj = new Object(body->Parent());
    if (len < int(cmd.length())) {
      obj->SetShortDesc(cmd.c_str() + len);
    }
    body->Parent()->SendOut(
        stealth_t,
        stealth_s,
        ";s creates ;s with Ninja Powers[TM].\n",
        "You create ;s.\n",
        body,
        obj);
    return 0;
  }

  if (com == COM_DCREATE) {
    if (!mind)
      return 0;
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;
    if (len >= int(cmd.length())) {
      mind->Send("You need to specify in what direction!\n");
    } else {
      Object* box = new Object(body->Parent()->Parent());
      Object* next = new Object(box);
      const char* dirb = "south";
      const char* dir = "north";
      if (!strcmp(cmd.c_str() + len, "north")) {
      } else if (!strcmp(cmd.c_str() + len, "south")) {
        dirb = "north";
        dir = "south";
      } else if (!strcmp(cmd.c_str() + len, "west")) {
        dirb = "east";
        dir = "west";
      } else if (!strcmp(cmd.c_str() + len, "east")) {
        dirb = "west";
        dir = "east";
      } else if (!strcmp(cmd.c_str() + len, "up")) {
        dirb = "down";
        dir = "up";
      } else if (!strcmp(cmd.c_str() + len, "down")) {
        dirb = "up";
        dir = "down";
      } else {
        mind->SendF("Direction '%s' not meaningful!\n", cmd.c_str() + len);
        return 0;
      }

      box->SetShortDesc("a dynamic dungeon");
      next->SetShortDesc("An Entrance to a Large Mining Tunnel");
      next->SetDesc(
          "This tunnel looks to have been carved centuries ago.  It is so well "
          "crafted\n"
          "that you think it will stand as-is for another millenium.\n");
      next->SetSkill("DynamicInit", 1);
      next->SetSkill("DynamicPhase", 0); // Entrance
      next->SetSkill("DynamicMojo", 1000000);

      body->Parent()->Link(
          next,
          dir,
          std::string("You see a solid passage leading ") + dir + ".\n",
          dirb,
          std::string("You see a solid passage leading ") + dirb + ".\n");

      body->Parent()->SendOutF(
          stealth_t,
          stealth_s,
          ";s creates a new dynamic dungeon '%s' with Ninja Powers[TM].\n",
          "You create a new dynamic dungeon '%s'.\n",
          body,
          nullptr,
          dir);
    }
    return 0;
  }

#define NUM_AVS 5 // 28 Max!
#define NUM_STS 5 // No Max
  if (com == COM_CCREATE) {
    if (!mind)
      return 0;
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;
    return handle_command_ccreate(body, mind, cmd.c_str(), len, stealth_t, stealth_s);
  }

  static Object* anchor = nullptr;

  if (com == COM_ANCHOR) {
    if (!mind)
      return 0;
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;
    anchor = new Object(body->Parent());
    anchor->SetShortDesc("a shimmering portal");
    anchor->SetDesc(
        "This portal could only have been created by a True "
        "Ninja[TM].  You wonder where it leads.");
    body->Parent()->SendOut(
        stealth_t,
        stealth_s,
        ";s creates a shimmering portal with Ninja Powers[TM].\n",
        "You create a shimmering portal.\n",
        body,
        nullptr);
    return 0;
  }

  if (com == COM_LINK) {
    if (!mind)
      return 0;
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;
    if (anchor == nullptr) {
      mind->Send("You need to make an anchor before you can link to it!\n");
    } else if (len >= int(cmd.length())) {
      mind->Send("You need to specify what the portal will be named!\n");
    } else {
      Object* link;

      body->Parent()->SendOutF(
          stealth_t,
          stealth_s,
          ";s creates a shimmering portal '%s' with Ninja Powers[TM].\n",
          "You create a shimmering portal '%s'.\n",
          body,
          nullptr,
          cmd.c_str() + len);

      link = new Object(body->Parent());
      link->SetShortDesc(cmd.c_str() + len);
      link->AddAct(ACT_SPECIAL_LINKED, anchor);
      link->AddAct(ACT_SPECIAL_MASTER, anchor);
      link->SetSkill("Open", 1000);
      link->SetSkill("Enterable", 1);
      anchor->AddAct(ACT_SPECIAL_LINKED, link);
      anchor->AddAct(ACT_SPECIAL_MASTER, link);
      anchor->SetSkill("Open", 1000);
      anchor->SetSkill("Enterable", 1);
      std::string other = cmd.c_str() + len;
      if (!strncmp(cmd.c_str() + len, "east", 4)) {
        other = "west";
        other += cmd.c_str() + len + 4;
      } else if (!strncmp(cmd.c_str() + len, "west", 4)) {
        other = "east";
        other += cmd.c_str() + len + 4;
      } else if (!strncmp(cmd.c_str() + len, "north", 5)) {
        other = "south";
        other += cmd.c_str() + len + 5;
      } else if (!strncmp(cmd.c_str() + len, "south", 5)) {
        other = "north";
        other += cmd.c_str() + len + 5;
      } else if (!strncmp(cmd.c_str() + len, "up", 2)) {
        other = "down";
        other += cmd.c_str() + len + 2;
      } else if (!strncmp(cmd.c_str() + len, "down", 4)) {
        other = "up";
        other += cmd.c_str() + len + 4;
      }
      anchor->SetShortDesc(other.c_str());
      anchor = nullptr;
    }
    return 0;
  }

  if (com == COM_DELPLAYER) {
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;
    if (len >= int(cmd.length())) {
      if (mind)
        mind->Send("You want to delete which player?\n");
    } else {
      Player* pl = get_player(cmd.c_str() + len);
      if (!pl) {
        if (mind)
          mind->Send("That player doesn't seem to exist.\n");
      } else {
        if (mind)
          mind->SendF("You delete the player '%s'.\n", pl->Name());
        delete pl;
      }
    }
    return 0;
  }

  if (com == COM_PLAYERS) {
    if (!mind)
      return 0;
    std::string users = "Current accounts on this MUD:\n";
    std::vector<Player*> pls = get_all_players();

    for (auto pl : pls) {
      users += pl->Name();
      users += "\n";
    }
    mind->Send(users.c_str());

    return 0;
  }

  if (com == COM_CHARACTERS) {
    if (!mind)
      return 0;
    std::string chars = "Current characters on this MUD:\n";
    std::vector<Player*> pls = get_all_players();

    for (auto pl : pls) {
      auto chs = pl->Room()->Contents();
      for (auto ch : chs) {
        chars += pl->Name();
        chars += ": ";
        chars += ch->ShortDesc();
        chars += " (";
        Object* top = ch;
        while (top->Parent() && top->Parent()->Parent())
          top = top->Parent();
        chars += top->ShortDesc();
        chars += ")\n";
      }
    }
    mind->Send(chars.c_str());

    return 0;
  }

  if (com == COM_RESET) {
    if (!mind)
      return 0;
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;
    Object* targ =
        body->PickObject(cmd.c_str() + len, vmode | LOC_NEARBY | LOC_INTERNAL | LOC_SELF);
    if (!targ) {
      mind->Send("You want to reset what?\n");
    } else {
      auto cont = targ->Contents();
      for (auto item : cont)
        item->Recycle();

      Object* dest = targ;
      while ((!dest->ActTarg(ACT_SPECIAL_HOME)) && dest->Parent()) {
        dest = dest->Parent();
      }
      if (dest->ActTarg(ACT_SPECIAL_HOME)) {
        dest = dest->ActTarg(ACT_SPECIAL_HOME);
      }
      targ->Travel(dest, 0);

      body->Parent()->SendOut(
          stealth_t,
          stealth_s,
          ";s resets ;s with Ninja Powers[TM].\n",
          "You reset ;s.\n",
          body,
          targ);
    }
    return 0;
  }

  if (com == COM_MIRROR) {
    if (!mind)
      return 0;
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;
    Object* targ = body->PickObject(cmd.c_str() + len, vmode | LOC_NEARBY | LOC_INTERNAL);
    if (!targ) {
      mind->Send("You want to mirror what?\n");
    } else {
      Object* nobj = new Object(*targ);
      nobj->SetParent(targ->Parent());

      nobj->SetSkill("Wearable on Left Arm", targ->Skill("Wearable on Right Arm"));
      nobj->SetSkill("Wearable on Left Finger", targ->Skill("Wearable on Right Finger"));
      nobj->SetSkill("Wearable on Left Foot", targ->Skill("Wearable on Right Foot"));
      nobj->SetSkill("Wearable on Left Hand", targ->Skill("Wearable on Right Hand"));
      nobj->SetSkill("Wearable on Left Leg", targ->Skill("Wearable on Right Leg"));
      nobj->SetSkill("Wearable on Left Wrist", targ->Skill("Wearable on Right Wrist"));
      nobj->SetSkill("Wearable on Left Shoulder", targ->Skill("Wearable on Right Shoulder"));
      nobj->SetSkill("Wearable on Left Hip", targ->Skill("Wearable on Right Hip"));

      nobj->SetSkill("Wearable on Right Arm", targ->Skill("Wearable on Left Arm"));
      nobj->SetSkill("Wearable on Right Finger", targ->Skill("Wearable on Left Finger"));
      nobj->SetSkill("Wearable on Right Foot", targ->Skill("Wearable on Left Foot"));
      nobj->SetSkill("Wearable on Right Hand", targ->Skill("Wearable on Left Hand"));
      nobj->SetSkill("Wearable on Right Leg", targ->Skill("Wearable on Left Leg"));
      nobj->SetSkill("Wearable on Right Wrist", targ->Skill("Wearable on Left Wrist"));
      nobj->SetSkill("Wearable on Right Shoulder", targ->Skill("Wearable on Left Shoulder"));
      nobj->SetSkill("Wearable on Right Hip", targ->Skill("Wearable on Left Hip"));

      int start;
      std::string name = nobj->ShortDesc();
      start = name.find(std::string("(left)"));
      if (start < int(name.length()) && start >= 0) {
        name = name.substr(0, start) + "(right)" + name.substr(start + 6);
      } else {
        start = name.find(std::string("(right)"));
        if (start < int(name.length()) && start >= 0) {
          name = name.substr(0, start) + "(left)" + name.substr(start + 7);
        }
      }
      nobj->SetShortDesc(name.c_str());

      body->Parent()->SendOut(
          stealth_t,
          stealth_s,
          ";s mirrors ;s with Ninja Powers[TM].\n",
          "You mirror ;s.\n",
          body,
          targ);
    }
    return 0;
  }

  if (com == COM_CLONE) {
    if (!mind)
      return 0;
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;
    Object* targ =
        body->PickObject(cmd.c_str() + len, vmode | LOC_NEARBY | LOC_INTERNAL | LOC_SELF);
    if (!targ) {
      mind->Send("You want to clone what?\n");
    } else {
      Object* nobj = new Object(*targ);
      nobj->SetParent(targ->Parent());

      body->Parent()->SendOut(
          stealth_t,
          stealth_s,
          ";s clones ;s with Ninja Powers[TM].\n",
          "You clone ;s.\n",
          body,
          targ);
    }
    return 0;
  }

  if (com == COM_JUNK) {
    if (!mind)
      return 0;
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;
    auto targs = body->PickObjects(cmd.c_str() + len, vmode | LOC_NEARBY);
    if (targs.size() == 0) {
      mind->Send("You want to destroy what?\n");
      return 0;
    }
    for (auto targ : targs) {
      body->Parent()->SendOut(
          stealth_t,
          stealth_s,
          ";s destroys ;s with Ninja Powers[TM].\n",
          "You destroy ;s.\n",
          body,
          targ);
      targ->Recycle();
    }
    return 0;
  }

  if (com == COM_HEAL) {
    if (!mind)
      return 0;
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;
    Object* targ =
        body->PickObject(cmd.c_str() + len, vmode | LOC_NEARBY | LOC_INTERNAL | LOC_SELF);

    int finished = 0;
    if (body->IsAct(ACT_HEAL) // Finish Previous Healing
        || body->IsUsing("Healing") || body->IsUsing("First Aid") || body->IsUsing("Treatment")) {
      finished = 1;
      if (body->IsAct(ACT_HEAL)) {
        if (body->IsUsing("Healing")) {
          mind->Send("You complete your healing efforts.\n");
          int phys = body->ActTarg(ACT_HEAL)->Phys();
          phys -= body->Roll("Healing", phys + 2);
          if (phys < 0)
            phys = 0;
          body->ActTarg(ACT_HEAL)->SetPhys(phys);
          int pois = body->ActTarg(ACT_HEAL)->Skill("Poisoned");
          pois -= body->Roll("Healing", pois + 2);
          if (pois < 0)
            pois = 0;
          body->ActTarg(ACT_HEAL)->SetSkill("Poisoned", pois);
        } else if (body->IsUsing("First Aid")) {
          mind->Send("You complete your first-aid efforts.\n");
          int phys = body->ActTarg(ACT_HEAL)->Phys();
          phys -= body->Roll("First Aid", phys);
          if (phys < 0)
            phys = 0;
          body->ActTarg(ACT_HEAL)->SetPhys(phys);
        } else if (body->IsUsing("Treatment")) {
          mind->Send("You complete your treatment efforts.\n");
          int pois = body->ActTarg(ACT_HEAL)->Skill("Poisoned");
          pois -= body->Roll("Treatment", pois);
          if (pois < 0)
            pois = 0;
          body->ActTarg(ACT_HEAL)->SetSkill("Poisoned", pois);
        }
        body->StopUsing();
        body->StopAct(ACT_HEAL);
      }
    }
    if (!targ) {
      if (!finished)
        mind->Send("You want to heal what?\n");
    } else if (targ->NormAttribute(2) < 1) {
      mind->SendF("You can't heal %s, it is not alive.\n", targ->Name(0, body));
    } else if (nmode) {
      // This is ninja-healing and bypasses all healing mechanisms.
      targ->SetSkill("Poisoned", 0);
      targ->SetSkill("Thirsty", 0);
      targ->SetSkill("Hungry", 0);
      targ->SetPhys(0);
      targ->SetStun(0);
      targ->SetStru(0);
      targ->UpdateDamage();

      body->Parent()->SendOut(
          stealth_t,
          stealth_s,
          ";s heals and repairs ;s with Ninja Powers[TM].\n",
          "You heal ;s.\n",
          body,
          targ);
    } else if (
        (!body->HasSkill("Healing")) && (!body->HasSkill("First Aid")) &&
        (!body->HasSkill("Treatment"))) {
      if (mind) {
        mind->SendF("You don't know how to help %s.\n", targ->Name(0, body));
      }
    } else {
      int duration = 0;
      std::string skill = "";
      body->Parent()->SendOut(
          stealth_t, stealth_s, ";s tries to heal ;s.\n", "You try to heal ;s.\n", body, targ);
      if (body->HasSkill("First Aid")) {
        if (targ->Phys() < 1) {
          mind->SendF("%s is not injured.\n", targ->Name());
        } else {
          mind->SendF("%s is injured.\n", targ->Name());
          skill = "First Aid";
          duration = 3000;
        }
      } else if (body->HasSkill("Healing")) {
        if (targ->Phys() < 1) {
          mind->SendF("%s is not injured.\n", targ->Name());
        } else {
          mind->SendF("%s is injured.\n", targ->Name());
          skill = "Healing";
          duration = 3000;
        }
      }
      if (body->HasSkill("Treatment")) {
        if (targ->Skill("Poisoned") < 1) {
          mind->SendF("%s does not need other help.\n", targ->Name());
        } else {
          mind->SendF("%s is poisoned.\n", targ->Name());
          skill = "Treatment";
          duration = 3000;
        }
      } else if (body->HasSkill("Healing")) {
        if (targ->Skill("Poisoned") < 1) {
          mind->SendF("%s does not need other help.\n", targ->Name());
        } else {
          mind->SendF("%s is poisoned.\n", targ->Name());
          skill = "Healing";
          duration = 3000;
        }
      }
      if (skill != "") {
        body->AddAct(ACT_HEAL, targ);
        body->StartUsing(skill);
      }
      if (duration > 0) {
        body->BusyFor(duration, "heal");
      }
    }
    return 0;
  }

  if (com == COM_JACK) {
    if (!mind)
      return 0;
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;
    Object* targ = body->ActTarg(ACT_POINT);
    if (!targ) {
      mind->Send("You need to be pointing at your target.\n");
      return 0;
    }
    int stat = 0;
    if (toupper(cmd[len]) == 'Q')
      stat = 1;
    if (toupper(cmd[len]) == 'S')
      stat = 2;
    if (toupper(cmd[len]) == 'C')
      stat = 3;
    if (toupper(cmd[len]) == 'I')
      stat = 4;
    if (toupper(cmd[len]) == 'W')
      stat = 5;

    if (targ->NormAttribute(stat) == 0) {
      mind->Send("This object doesn't have that stat.\n");
      return 0;
    }

    targ->SetAttribute(stat, targ->NormAttribute(stat) + 1);

    body->Parent()->SendOutF(
        stealth_t,
        stealth_s,
        ";s jacks the %s of ;s with Ninja Powers[TM].\n",
        "You jack the %s of ;s.\n",
        body,
        targ,
        statnames[stat]);

    return 0;
  }

  if (com == COM_CHUMP) {
    if (!mind)
      return 0;
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;
    Object* targ = body->ActTarg(ACT_POINT);
    if (!targ) {
      mind->Send("You need to be pointing at your target.\n");
      return 0;
    }
    int stat = 0;
    if (toupper(cmd[len]) == 'Q')
      stat = 1;
    if (toupper(cmd[len]) == 'S')
      stat = 2;
    if (toupper(cmd[len]) == 'C')
      stat = 3;
    if (toupper(cmd[len]) == 'I')
      stat = 4;
    if (toupper(cmd[len]) == 'W')
      stat = 5;

    if (targ->NormAttribute(stat) == 0) {
      mind->Send("This object doesn't have that stat.\n");
      return 0;
    }
    if (targ->NormAttribute(stat) == 1) {
      mind->Send("It is already a 1 (the minimum!).\n");
      return 0;
    }

    targ->SetAttribute(stat, targ->NormAttribute(stat) - 1);

    body->Parent()->SendOutF(
        stealth_t,
        stealth_s,
        ";s chumps the %s of ;s with Ninja Powers[TM].\n",
        "You chump the %s of ;s.\n",
        body,
        targ,
        statnames[stat]);

    return 0;
  }

  if (com == COM_INCREMENT) {
    if (!mind)
      return 0;
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;
    Object* targ = body->ActTarg(ACT_POINT);
    if (!targ) {
      mind->Send("You need to be pointing at your target.\n");
      return 0;
    }

    int amt = 1;
    if (isdigit(cmd[len])) {
      amt = atoi(cmd.c_str() + len);
      while (isdigit(cmd[len]))
        ++len;
      while (len < int(cmd.length()) && (!isgraph(cmd[len])))
        ++len;
    }

    if (!is_skill(cmd.c_str() + len)) {
      mind->SendF("Warning, '%s' is not a real skill name!\n", cmd.c_str() + len);
    }

    targ->SetSkill(cmd.c_str() + len, std::max(targ->Skill(cmd.c_str() + len), 0) + amt);

    body->Parent()->SendOutF(
        stealth_t,
        stealth_s,
        ";s increments the %s of ;s with Ninja Powers[TM].\n",
        "You increment the %s of ;s.\n",
        body,
        targ,
        cmd.c_str() + len);

    return 0;
  }

  if (com == COM_DECREMENT) {
    if (!mind)
      return 0;
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;
    Object* targ = body->ActTarg(ACT_POINT);
    if (!targ) {
      mind->Send("You need to be pointing at your target.\n");
      return 0;
    }

    int amt = 1;
    if (isdigit(cmd[len])) {
      amt = atoi(cmd.c_str() + len);
      while (isdigit(cmd[len]))
        ++len;
      while (len < int(cmd.length()) && (!isgraph(cmd[len])))
        ++len;
    }

    targ->SetSkill(cmd.c_str() + len, targ->Skill(cmd.c_str() + len) - amt);

    body->Parent()->SendOutF(
        stealth_t,
        stealth_s,
        ";s decrements the %s of ;s with Ninja Powers[TM].\n",
        "You decrement the %s of ;s.\n",
        body,
        targ,
        cmd.c_str() + len);

    return 0;
  }

  if (com == COM_DOUBLE) {
    if (!mind)
      return 0;
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;
    auto targs = body->PickObjects(cmd.c_str() + len, vmode | LOC_NEARBY | LOC_INTERNAL);
    if (targs.size() == 0) {
      mind->Send("You want to double what?\n");
      return 0;
    }
    for (auto targ : targs) {
      body->Parent()->SendOut(
          stealth_t,
          stealth_s,
          ";s doubles ;s with Ninja Powers[TM].\n",
          "You double ;s.\n",
          body,
          targ);
      if (targ->Skill("Quantity") > 1) {
        targ->SetSkill("Quantity", targ->Skill("Quantity") * 2);
      } else {
        targ->SetSkill("Quantity", 2);
      }
    }
    return 0;
  }

  if (com == COM_TLOAD) {
    if (!mind)
      return 0;
    while (len < int(cmd.length()) && (!isgraph(cmd[len])))
      ++len;
    if (len >= int(cmd.length())) {
      Object* world = new Object(body->Parent());
      world->SetShortDesc("The tbaMUD World");
      world->SetSkill("Light Source", 1000);
      world->SetSkill("Day Length", 240);
      world->SetSkill("Day Time", 120);
      world->TBALoadAll();
      body->Parent()->SendOut(
          stealth_t,
          stealth_s,
          ";s loads the entire TBA world with Ninja Powers[TM].\n",
          "You load the entire TBA world.\n",
          body,
          nullptr);
      world->Activate();
    } else {
      char buf[2048];
      sprintf(buf, "tba/wld/%s.wld", cmd.c_str() + len);
      body->Parent()->TBALoadWLD(buf);
      sprintf(buf, "tba/obj/%s.obj", cmd.c_str() + len);
      body->Parent()->TBALoadOBJ(buf);
      body->Parent()->SendOut(
          stealth_t,
          stealth_s,
          ";s loads a TBA world ;s with Ninja Powers[TM].\n",
          "You load a TBA world.\n",
          body,
          nullptr);
    }
    return 0;
  }

  if (com == COM_TCLEAN) {
    if (!mind)
      return 0;
    body->TBACleanup();
    body->Parent()->SendOut(
        stealth_t,
        stealth_s,
        ";s cleans up after loading TBA worlds.\n",
        "You clean up after loading TBA worlds.\n",
        body,
        nullptr);
    return 0;
  }

  mind->Send("Sorry, that command's not yet implemented.\n");
  return 0;
}

int handle_command(Object* body, const std::string& cl, Mind* mind) {
  int ret = 0;
  const char *start = cl.c_str(), *end = cl.c_str();

  if (mind && mind->SpecialPrompt()[0] != 0) {
    std::string cmd = std::string(mind->SpecialPrompt()) + " " + cl.c_str();
    ret = handle_single_command(body, cmd.c_str(), mind);
    return ret;
  }

  while (1) {
    if ((*end) == '\n' || (*end) == '\r' || (*end) == ';' || (*end) == 0) {
      if (end > start) {
        int len = end - start;
        char buf[len + 1];
        memcpy(buf, start, len);
        buf[len] = 0;

        int stat = handle_single_command(body, buf, mind);
        if (stat < 0)
          return stat;
        else if (ret == 0)
          ret = stat;
      }
      start = end;
      ++start;
    }
    if ((*end) == 0)
      break;
    ++end;
  }
  return ret;
}

// int handle_command(Object *obj, const char *cl) {
////  fprintf(stderr, "Doing '%s' for %p\n", cl, obj);
//
//  static Mind mind;
//  mind.Attach(obj);
//  int ret = handle_command(obj, cl, &mind);
//  mind.Unattach();
//  return ret;
//  }
