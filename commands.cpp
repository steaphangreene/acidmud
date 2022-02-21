// *************************************************************************
//  This file is part of AcidMUD by Steaphan Greene
//
//  Copyright 1999-2022 Steaphan Greene <steaphan@gmail.com>
//
//  AcidMUD is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 3 of the License, or
//  (at your option) any later version.
//
//  AcidMUD is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with AcidMUD (see the file named "COPYING");
//  If not, see <http://www.gnu.org/licenses/>.
//
// *************************************************************************

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

int handle_command_ccreate(Object*, Mind*, const std::string_view, int, int);

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
  std::string_view command;
  std::string_view shortdesc;
  std::string_view longdesc;
  int sit;
};

constexpr Command static_comlist[COM_MAX] = {
    {COM_NONE, "", "", "", 0},
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
    {COM_HOLD,
     "hold",
     "Hold an item you are carrying in your off-hand.",
     "Hold an item you are carrying in your off-hand.",
     (REQ_ALERT | REQ_ACTION)},
    {COM_OFFER,
     "offer",
     "Offer to give your held item to someone, or stop offering.",
     "Offer to give your held item to someone, or stop offering.",
     (REQ_ALERT | REQ_ACTION)},

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
    {COM_ARCHETYPE,
     "archetype",
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
    {COM_PROD,
     "prod",
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
     (REQ_ALERT | REQ_NINJAMODE | CMD_FLAVORTEXT)},
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
static_assert(static_comlist[COM_OFFER].id == COM_OFFER);
static_assert(static_comlist[COM_LIGHT].id == COM_LIGHT);
static_assert(static_comlist[COM_WEAR].id == COM_WEAR);
static_assert(static_comlist[COM_REMOVE].id == COM_REMOVE);
static_assert(static_comlist[COM_LABEL].id == COM_LABEL);
static_assert(static_comlist[COM_UNLABEL].id == COM_UNLABEL);
static_assert(static_comlist[COM_HEAL].id == COM_HEAL);
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
static_assert(static_comlist[COM_EAT].id == COM_EAT);
static_assert(static_comlist[COM_DRINK].id == COM_DRINK);
static_assert(static_comlist[COM_FILL].id == COM_FILL);
static_assert(static_comlist[COM_DUMP].id == COM_DUMP);
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
static_assert(static_comlist[COM_ARCHETYPE].id == COM_ARCHETYPE);
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
static_assert(static_comlist[COM_PROD].id == COM_PROD);
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

static std::string soc_com[1024] = {};
static std::string socials[1024][13] = {};

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
      soc_com[cnum] = com;
      char buf[256] = "";
      for (int mnum = 0; mnum < 13; ++mnum) {
        fscanf(soc, " %255[^\n\r]", buf); // Skip space/newline, read line.
        fscanf(soc, "%*[^\n\r]"); // Skip rest of line.
        if (strstr(buf, "#"))
          socials[cnum][mnum] = "";
        else
          socials[cnum][mnum] = buf;
      }
      comlist[cnum].command = soc_com[cnum];
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

com_t identify_command(const std::string_view str, bool corporeal) {
  if (comlist[COM_MAX].id == COM_NONE) { // Haven't loaded commands yet
    load_commands();
  }
  for (int ctr = 1; comlist[ctr].id != COM_NONE; ++ctr) {
    if (comlist[ctr].sit & SIT_NINJAMODE)
      continue; // Don't match ninja commands here

    if (!corporeal && (!(comlist[ctr].sit & SIT_ETHEREAL)))
      continue; // Don't match between modes

    if (corporeal && (!(comlist[ctr].sit & SIT_CORPOREAL)))
      continue; // Don't match between modes

    if (str == comlist[ctr].command.substr(0, str.length())) {
      return com_t(ctr); // Not .id - as all social commands share the same one.
    }
    // Command Aliases
    if (comlist[ctr].id == COM_SAY && (str[0] == '\'' || str[0] == '"')) {
      return comlist[ctr].id;
    }
    if (comlist[ctr].id == COM_DUMP && str == std::string_view("empty").substr(0, str.length())) {
      return comlist[ctr].id;
    }
    if (comlist[ctr].id == COM_GET && str == std::string_view("take").substr(0, str.length())) {
      return comlist[ctr].id;
    }
  }
  return COM_NONE;
}

// Return values: -1: Player D/Ced
//                0: Command Understood
//                1: Command NOT Understood
//                2: Command Understood - No More Actions This Round
static int handle_single_command(Object* body, std::string line, Mind* mind) {
  if (comlist[COM_MAX].id == COM_NONE) { // Haven't loaded commands yet
    load_commands();
  }

  std::string_view cmd = line;
  std::string_view args = line;
  std::string args_buf;

  auto c1 = cmd.find_first_not_of(" \t\n\r;");
  if (c1 == std::string::npos) {
    return 1;
  }

  auto c2 = cmd.find_first_of(" \t\n\r;", c1 + 1);
  if (c2 == std::string::npos) {
    cmd = cmd.substr(c1);
  } else {
    cmd = cmd.substr(c1, c2 - c1);
  }

  // Lowercase the command portion, and only that portion, for now.
  auto clen = cmd.find_first_not_of("abcdefghijklmnopqrstuvwxyz");
  if (clen != std::string::npos && clen < cmd.length() && ascii_isupper(cmd[clen])) {
    if (c2 == std::string::npos) {
      std::transform(line.begin(), line.end(), line.begin(), ascii_tolower);
    } else {
      std::transform(line.begin(), line.begin() + c2 + 1, line.begin(), ascii_tolower);
    }
  }

  if (comlist[COM_MAX].id == COM_NONE) { // Haven't loaded commands yet
    load_commands();
  }

  if ((!body) && (!mind)) { // Nobody doing something?
    fprintf(stderr, "Warning: absolutely nobody tried to '%s'.\n", line.c_str());
    return 0;
  }

  if (cmd.empty() || cmd.front() == '#')
    return 0;

  if (mind && mind->Type() == MIND_REMOTE && (!mind->Owner())) {
    if (mind->PName() != "")
      mind->SetPPass(std::string(cmd));
    else {
      int ctr;
      for (ctr = 0; ctr < int(cmd.length()); ++ctr) {
        if (!(ascii_isalnum(cmd[ctr]) || cmd[ctr] == ' ')) {
          mind->SendF(
              "Name '%s' is invalid.\nNames can only have letters, "
              "numbers, and spaces.\n",
              std::string(cmd).c_str());
          fprintf(
              stderr,
              "Name '%s' is invalid.\nNames can only have letters, "
              "numbers, and spaces.\n",
              std::string(cmd).c_str());
          break;
        }
      }
      if (ctr == int(cmd.length()))
        mind->SetPName(std::string(cmd));
    }
    return 0;
  }

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
  if (body && body->Power(crc32c("Dark Vision"))) {
    vmode |= LOC_DARK;
  }
  if (body && body->Power(crc32c("Heat Vision"))) {
    vmode |= LOC_HEAT;
  }

  int cnum = identify_command(cmd, (body != nullptr));
  if (cnum == COM_NONE && nmode) { // Now match ninja commands (for ninjas)
    for (int ctr = 1; comlist[ctr].id != COM_NONE; ++ctr) {
      if ((comlist[ctr].sit & SIT_NINJAMODE) &&
          cmd == comlist[ctr].command.substr(0, cmd.length())) {
        cnum = ctr;
        break;
      }
      if (comlist[ctr].id == COM_CHARACTERS &&
          cmd == std::string_view("chars").substr(0, cmd.length())) {
        cnum = ctr;
        break;
      }
    }
  }

  // Lowercase the entire command, for non-flavortext commands.
  if (cnum != COM_NONE && (comlist[cnum].sit & CMD_FLAVORTEXT) == 0) {
    auto fclen = line.find_first_of("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    if (fclen != std::string::npos && fclen < line.length()) {
      std::transform(line.begin(), line.end(), line.begin(), ascii_tolower);
    }
  }

  if (c2 == std::string::npos) {
    args = "";
  } else {
    auto a1 = args.find_first_not_of(" \t\n\r;", c2 + 1);
    if (a1 == std::string::npos) {
      args = "";
    } else {
      args = args.substr(a1);
    }
  }

  if ((!nmode) && cnum != COM_RECALL && body && body->Parent()) {
    MinVec<1, Object*> items;
    Object* room = body->PickObject("here", LOC_HERE);
    auto items2 = body->PickObjects("everything", LOC_INTERNAL | LOC_NEARBY);
    auto items3 = body->PickObjects("everyone", LOC_NEARBY);
    items.reserve(items2.size() + items3.size() + (room != nullptr));
    if (room != nullptr)
      items.push_back(room);
    items.insert(items.end(), items2.begin(), items2.end());
    items.insert(items.end(), items3.begin(), items3.end());

    for (auto obj : items) {
      auto trigs = obj->PickObjects("all tbamud trigger script", LOC_NINJA | LOC_INTERNAL);
      for (auto trig : trigs) {
        if (trig->Skill(crc32c("TBAScriptType")) & 0x04) { //*-COMMAND trigs
          if ((cnum == COM_NONE && cmd == std::string_view(trig->Desc()).substr(0, cmd.length())) ||
              (cnum && cnum == identify_command(trig->Desc(), true))) {
            if (trig->Skill(crc32c("TBAScriptType")) & 0x2000000) { // OBJ
              int narg = trig->Skill(crc32c("TBAScriptNArg"));
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
            if (cnum != COM_NONE) {
              cmln = std::string(comlist[cnum].command) + std::string(" ") + std::string(args);
            } else {
              cmln = trig->Desc() + std::string(" ") + std::string(args);
            }
            if (!new_trigger(0, trig, body, cmln)) {
              return 0; // Handled, unless script says not.
            }
          }
        }
      }
    }
  }
  if (cnum == COM_NONE) { // Unknown, and not trigger-supported, command
    if (mind)
      mind->Send("Command NOT understood - type 'help' for assistance.\n");
    return 1;
  }

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
      body->DoWhenFree(std::string(comlist[cnum].command) + " " + std::string(args));
      return 0;
    }
    if (comlist[cnum].sit & (SIT_ALIVE | SIT_AWAKE | SIT_ALERT)) {
      if (body->IsAct(act_t::DYING) || body->IsAct(act_t::DEAD)) {
        if (mind)
          mind->Send("You must be alive to use that command.\n");
        return 0;
      }
    }
    if (body && (comlist[cnum].sit & SIT_CONSCIOUS)) {
      if (body->IsAct(act_t::UNCONSCIOUS)) {
        if (mind)
          mind->Send("You can't do that, you are out cold.\n");
        return 0;
      }
    }
    if ((comlist[cnum].sit & (SIT_STAND | SIT_USE)) == (SIT_STAND | SIT_USE)) {
      if (body->Pos() != pos_t::STAND && body->Pos() != pos_t::USE) {
        if (mind)
          mind->Send("You must stand up to use that command.\n");
        handle_single_command(body, "stand", mind);
        if (body->Pos() != pos_t::STAND && body->Pos() != pos_t::USE)
          return 0;
      }
    } else if ((comlist[cnum].sit & (SIT_STAND | SIT_SIT)) == (SIT_STAND | SIT_SIT)) {
      if (body->Pos() == pos_t::USE) {
        if (mind)
          mind->Send("You must stop using this skill to do that.\n");
        handle_single_command(body, "stop", mind);
        if (body->Pos() != pos_t::STAND)
          return 0;
      } else if (body->Pos() != pos_t::SIT && body->Pos() != pos_t::STAND) {
        if (mind)
          mind->Send("You must at least sit up to use that command.\n");
        handle_single_command(body, "sit", mind);
        if (body->Pos() != pos_t::SIT && body->Pos() != pos_t::STAND)
          return 0;
      }
    } else if (comlist[cnum].sit & SIT_STAND) {
      if (body->Pos() == pos_t::USE) {
        if (mind)
          mind->Send("You must stop using this skill to do that.\n");
        handle_single_command(body, "stop", mind);
        if (body->Pos() != pos_t::STAND)
          return 0;
      } else if (body->Pos() != pos_t::STAND) {
        if (mind)
          mind->Send("You must stand up to use that command.\n");
        handle_single_command(body, "stand", mind);
        if (body->Pos() != pos_t::STAND)
          return 0;
      }
    }
    if (comlist[cnum].sit & SIT_SIT) {
      if (body->Pos() != pos_t::SIT) {
        if (mind)
          mind->Send("You must sit to use that command.\n");
        handle_single_command(body, "sit", mind);
        if (body->Pos() != pos_t::SIT)
          return 0;
      }
    }
    if (comlist[cnum].sit & SIT_ALERT) {
      if (body->IsAct(act_t::SLEEP)) {
        if (mind)
          mind->Send("You must be awake to use that command.\n");
        handle_single_command(body, "wake", mind);
        if (body->IsAct(act_t::SLEEP))
          return 0;
      }
      if (body->IsAct(act_t::REST)) {
        if (mind)
          mind->Send("You must be alert to use that command.\n");
        handle_single_command(body, "rest", mind);
        if (body->IsAct(act_t::REST))
          return 0;
      }
    }
    if (comlist[cnum].sit & SIT_AWAKE) {
      if (body->IsAct(act_t::SLEEP)) {
        if (mind)
          mind->Send("You must be awake to use that command.\n");
        handle_single_command(body, "wake", mind);
        if (body->IsAct(act_t::SLEEP))
          return 0;
      }
    }
  }

  int stealth_t = 0, stealth_s = 0;

  if (body && body->IsUsing(crc32c("Stealth")) && body->Skill(crc32c("Stealth")) > 0) {
    stealth_t = body->Skill(crc32c("Stealth"));
    stealth_s = body->Roll(crc32c("Stealth"), 2);
  }

  if (cnum == COM_VERSION) {
    if (mind)
      mind->SendF(
          "Version of this MUD is %d.%d.%d-%d-%s: %s.\n",
          CurrentVersion.acidmud_version[0],
          CurrentVersion.acidmud_version[1],
          CurrentVersion.acidmud_version[2],
          CurrentVersion.acidmud_git_revs,
          CurrentVersion.acidmud_git_hash.c_str(),
          CurrentVersion.acidmud_datestamp.c_str());
    return 0;
  }

  if (cnum == COM_SHUTDOWN) {
    shutdn = 1;
    if (mind)
      mind->Send("You instruct the system to shut down.\n");
    return 0;
  }

  if (cnum == COM_RESTART) {
    shutdn = 2;
    if (mind)
      mind->Send("You instruct the system to restart.\n");
    return 0;
  }

  if (cnum == COM_SAVEALL) {
    shutdn = -1;
    if (mind)
      mind->Send("You instruct the system to save all.\n");
    return 0;
  }

  if (cnum == COM_SELECT) {
    Object* sel = mind->Owner()->Room()->PickObject(std::string(args), vmode | LOC_INTERNAL);
    if (!sel) {
      mind->Send(
          "Sorry, that character doesn't exist.\n"
          "Use the 'newcharacter' command to create a new character.\n");
      return 0;
    } else {
      mind->SendF(
          "'%s' is now selected as your currect character to work on.\n", sel->Name().c_str());
      mind->Owner()->SetCreator(sel);
      return 0;
    }
  }

  if (cnum == COM_NORTH) {
    cnum = COM_ENTER;
    args = "north";
  }
  if (cnum == COM_SOUTH) {
    cnum = COM_ENTER;
    args = "south";
  }
  if (cnum == COM_EAST) {
    cnum = COM_ENTER;
    args = "east";
  }
  if (cnum == COM_WEST) {
    cnum = COM_ENTER;
    args = "west";
  }
  if (cnum == COM_UP) {
    cnum = COM_ENTER;
    args = "up";
  }
  if (cnum == COM_DOWN) {
    cnum = COM_ENTER;
    args = "down";
  }

  if (cnum == COM_FLEE) {
    auto dirs = body->PickObjects("everywhere", vmode | LOC_NEARBY);
    dirs.erase(
        std::remove_if(
            dirs.begin(), dirs.end(), [](const Object* o) { return o->Skill(crc32c("Open")) < 1; }),
        dirs.end());
    if (dirs.size() < 1) {
      if (mind)
        mind->Send("There is nowhere go, you can't flee!\n");
      return 0;
    }

    body->StartUsing(crc32c("Sprinting"));
    body->SetSkill(crc32c("Hidden"), 0);

    auto dir = dirs.begin();
    int sel = rand() % dirs.size();
    while (sel > 0) {
      ++dir;
      --sel;
    }
    if (mind)
      mind->SendF("You try to flee %s.\n", (*dir)->ShortDescC());

    cnum = COM_ENTER;
    args_buf = (*dir)->ShortDesc();
    args = args_buf;
  }

  if (cnum == COM_ENTER) {
    if (!body) { // Implies that there is a "mind"
      if (args.empty()) {
        mind->Send("Enter which character?  Use 'enter <charname>'.\n");
        return 0;
      }
      if (!mind->Owner()) { // The Autoninja (Initial Startup)
        Object* god = new_body();
        god->SetShortDesc(std::string(args));
        mind->Attach(god);
        return 0;
      }

      body = mind->Owner()->Room()->PickObject(std::string(args), vmode | LOC_INTERNAL);
      if (!body) {
        mind->Send(
            "Sorry, that character doesn't exist.\n"
            "Use the 'newcharacter' command to create a new character.\n");
        return 0;
      }
      if (body->Skill(crc32c("Attribute Points")) > 0 || body->Skill(crc32c("Skill Points")) > 0) {
        mind->Send("You need to finish that character before you can use it.\n");
        mind->SendF(
            "'%s' is now selected as your currect character to work on.\n", body->Name().c_str());
        mind->Owner()->SetCreator(body);
        return 0;
      }
      if ((!nmode) && body->IsAct(act_t::DEAD)) { // Ninjas can autoheal
        mind->Send(
            "Sorry, that character is dead.\n"
            "Use the 'newcharacter' command to create a new character.\n");
        return 0;
      }

      // FIXME: Handle conversion of body->Skill(crc32c("Resources")).
      if (mind->Owner()->Creator() == body)
        mind->Owner()->SetCreator(nullptr);

      mind->Attach(body);

      if (nmode) {
        // This is ninja-healing and bypasses all healing mechanisms.
        body->SetSkill(crc32c("Poisoned"), 0);
        body->SetSkill(crc32c("Thirsty"), 0);
        body->SetSkill(crc32c("Hungry"), 0);
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

      if (body->IsAct(act_t::DYING)) {
        mind->Send("You can see nothing, you are too busy dying.\n");
      } else if (body->IsAct(act_t::UNCONSCIOUS)) {
        mind->Send("You can see nothing, you are out cold.\n");
      } else if (body->IsAct(act_t::SLEEP)) {
        mind->Send("You can see nothing since you are asleep.\n");
      } else {
        body->Parent()->SendDescSurround(mind, body);
      }
      mind->SendF(CMAG "You have entered: %s\n" CNRM, body->World()->ShortDescC());
      return 0;
    }
    Object* dest = body->PickObject(std::string(args), vmode | LOC_NEARBY);
    Object* rdest = dest;
    Object* veh = body;

    if (!dest) {
      if (mind)
        mind->Send("You want to go where?\n");
      return 0;
    }

    if (dest->ActTarg(act_t::SPECIAL_LINKED) && dest->ActTarg(act_t::SPECIAL_LINKED)->Parent()) {
      rdest = dest->ActTarg(act_t::SPECIAL_LINKED)->Parent();
    }

    if ((!dest->Skill(crc32c("Enterable"))) && (!ninja)) {
      if (mind)
        mind->Send("It is not possible to enter that object!\n");
    } else if ((!dest->Skill(crc32c("Enterable"))) && (!nmode)) {
      if (mind)
        mind->Send("You need to be in ninja mode to enter that object!\n");
    } else if (dest->Skill(crc32c("Open")) < 1 && (!nmode)) {
      if (mind)
        mind->SendF("Sorry, %s is closed!\n", dest->Name().c_str());
    } else if (
        dest->Parent() != body->Parent() && dest->Parent() == body->Parent()->Parent() &&
        body->Parent()->Skill(crc32c("Vehicle")) == 0) {
      if (mind)
        mind->SendF("You can't get %s to go there!\n", body->Parent()->Name(1).c_str());
    } else if (
        dest->Parent() != body->Parent() && dest->Parent() == body->Parent()->Parent() &&
        (!(body->Parent()->Skill(crc32c("Vehicle")) & 0xFFF0)) // No Land Travel!
        && body->Parent()->Parent()->Skill(crc32c("WaterDepth")) == 0 &&
        rdest->Skill(crc32c("WaterDepth")) == 0) {
      if (mind)
        mind->SendF("You can't get %s to go there!\n", body->Parent()->Name(1).c_str());
    } else {
      if (nmode) {
        // Ninja-movement can't be followed!
        if (body->Parent())
          body->Parent()->NotifyGone(body);
      }
      if (dest->Parent() != body->Parent() && dest->Parent() == body->Parent()->Parent()) {
        if (body->Parent()->Skill(crc32c("Vehicle")) == 4 && body->Skill(crc32c("Boat")) == 0) {
          if (mind)
            mind->SendF("You don't know how to operate %s!\n", body->Parent()->Name(1).c_str());
          return 0;
        }
        veh = body->Parent();
      }

      if (rdest->Skill(crc32c("WaterDepth")) == 1 && body->Skill(crc32c("Swimming")) == 0) {
        if (veh == body || (veh->Skill(crc32c("Vehicle")) & 4) == 0) { // Have boat?
          if (mind)
            mind->Send("Sorry, but you can't swim!\n");
          return 0;
        }
      } else if (rdest->Skill(crc32c("WaterDepth")) > 1) {
        if (veh == body || (veh->Skill(crc32c("Vehicle")) & 4) == 0) { // Have boat?
          if (mind)
            mind->Send("Sorry, you need a boat to go there!\n");
          return 0;
        }
      }

      int newworld = (body->World() != rdest->World());
      if (dest->ActTarg(act_t::SPECIAL_LINKED) && dest->ActTarg(act_t::SPECIAL_LINKED)->Parent()) {
        body->Parent()->SendOut(stealth_t, stealth_s, ";s leaves ;s.\n", "", body, dest);
        dest = dest->ActTarg(act_t::SPECIAL_LINKED)->Parent();
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
          mind->SendF("You enter %s\n", std::string(args).c_str());
        }

        if (mind && newworld) {
          mind->SendF(CMAG "You have entered: %s\n" CNRM, body->World()->ShortDescC());
        }
        if (stealth_t > 0) {
          body->SetSkill(crc32c("Hidden"), body->Roll(crc32c("Stealth"), 2) * 2);
        }
        if (body->Roll(crc32c("Running"), 2) < 1) { // FIXME: Terrain/Direction Mods?
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

  if (cnum == COM_QUIT) {
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

  if (cnum == COM_HELP) {
    if ((!mind) || (!mind->Owner()))
      return 0;
    if (args == "commands") {
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
    } else if (args == "socials") {
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

  if (cnum == COM_SAY) {
    if (args.empty()) {
      if (strncmp(mind->SpecialPrompt().c_str(), "say", 3)) {
        mind->SetSpecialPrompt("say");
        mind->Send("Type what your character will say - exit by just hitting ENTER:");
      } else {
        mind->SetSpecialPrompt("");
        mind->Send("Exiting out of say mode.");
      }
      return 0;
    } else {
      bool shouting = (args.length() >= 4 && !std::any_of(args.begin(), args.end(), ascii_islower));
      if (!shouting) {
        body->Parent()->SendOutF(
            ALL, 0, ";s says '%s'\n", "You say '%s'\n", body, body, std::string(args).c_str());
        body->SetSkill(crc32c("Hidden"), 0);
        return 0;
      } else {
        cnum = COM_SHOUT;
      }
    }
  }

  if (cnum == COM_SHOUT || cnum == COM_YELL || cnum == COM_CALL) {
    if (mind && args.empty()) {
      if (strncmp(mind->SpecialPrompt().c_str(), "shout", 3)) {
        mind->SetSpecialPrompt("shout");
        mind->Send(
            "Type what your character will shout - exit by just hitting "
            "ENTER:");
      } else {
        mind->SetSpecialPrompt("");
        mind->Send("Exiting out of shout mode.");
      }
    } else {
      if (args.substr(0, 4) == "for ") {
        auto prefix = args.find_first_not_of(" \t\n\r", 4);
        if (prefix == std::string::npos) {
          args = "";
        } else {
          args = args.substr(prefix);
        }
      }

      std::string mes = std::string(args);
      std::transform(mes.begin(), mes.end(), mes.begin(), ascii_toupper);
      body->Parent()->SendOutF(
          ALL, 0, ";s shouts '%s'!!!\n", "You shout '%s'!!!\n", body, body, mes.c_str());
      body->Parent()->LoudF(body->Skill(crc32c("Strength")), "someone shout '%s'!!!", mes.c_str());
    }
    body->SetSkill(crc32c("Hidden"), 0);
    return 0;
  }

  if (cnum >= COM_SOCIAL) {
    Object* targ = nullptr;
    if (body && body->Parent()) {
      std::string youmes = socials[cnum][0];
      std::string outmes = socials[cnum][1];
      std::string targmes = "";
      if (!args.empty()) {
        targ = body->PickObject(std::string(args), vmode | LOC_NEARBY | LOC_SELF | LOC_INTERNAL);
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
        outmes[0] = ascii_toupper(outmes[0]);
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
        youmes[0] = ascii_toupper(youmes[0]);
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
        targmes[0] = ascii_toupper(targmes[0]);
        targmes += "\n";
        targ->Send(0, 0, targmes.c_str());
        targ->Deafen(true);
      }
      body->Parent()->SendOut(0, 0, outmes.c_str(), youmes.c_str(), body, targ);
      if (targ)
        targ->Deafen(false);
    }
    return 0;
  }

  if (cnum == COM_EMOTE) {
    std::string dot = ".";
    if (args.back() == '.' || args.back() == '?' || args.back() == '!') {
      dot = "";
    }
    body->Parent()->SendOutF(
        ALL,
        0,
        ";s %s%s\n",
        "Your character %s%s\n",
        body,
        body,
        std::string(args).c_str(),
        dot.c_str());
    body->SetSkill(crc32c("Hidden"), 0);
    return 0;
  }

  if (cnum == COM_INVENTORY) {
    if (mind) {
      mind->SendF("You (%s) are carrying:\n", body->ShortDescC());
      body->SendExtendedActions(mind, LOC_TOUCH | vmode | 1);
    }
    return 0;
  }

  if (cnum == COM_EQUIPMENT) {
    if (mind) {
      mind->SendF("You (%s) are using:\n", body->ShortDescC());
      body->SendExtendedActions(mind, LOC_TOUCH | vmode);
    }
    return 0;
  }

  if (cnum == COM_LOOK) {
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
    if (args.substr(0, 3) == "at ") {
      auto prefix = args.find_first_not_of(" \t\n\r", 3);
      if (prefix == std::string::npos) {
        args = "";
      } else {
        args = args.substr(prefix);
      }
    } else if (args.substr(0, 3) == "in ") {
      auto prefix = args.find_first_not_of(" \t\n\r", 3);
      if (prefix == std::string::npos) {
        args = "";
      } else {
        args = args.substr(prefix);
      }
      within = 1;
    }

    MinVec<1, Object*> targs;
    if (!args.empty()) {
      targs = body->PickObjects(
          std::string(args), vmode | LOC_NEARBY | LOC_ADJACENT | LOC_SELF | LOC_INTERNAL);
    } else {
      targs.push_back(body->Parent());
    }

    if (targs.size() < 1) {
      if (mind)
        mind->Send("You don't see that here.\n");
      return 0;
    }

    for (auto targ : targs) {
      if (within && (!targ->Skill(crc32c("Container"))) &&
          (!targ->Skill(crc32c("Liquid Container")))) {
        if (mind)
          mind->SendF("You can't look inside %s, it is not a container.\n", targ->Name().c_str());
      } else if (within && (targ->Skill(crc32c("Locked")))) {
        if (mind)
          mind->SendF("You can't look inside %s, it is locked.\n", targ->Name().c_str());
      } else {
        int must_open = within;
        if (within && targ->Skill(crc32c("Open")))
          must_open = 0;

        if (must_open) {
          targ->SetSkill(crc32c("Open"), 1000);
          body->Parent()->SendOut(
              stealth_t, stealth_s, ";s opens ;s.\n", "You open ;s.\n", body, targ);
        }

        if (args.empty()) {
          body->Parent()->SendOut(stealth_t, stealth_s, ";s looks around.\n", "", body, targ);
          if (mind)
            targ->SendDescSurround(mind, body, vmode);
        } else if (
            args == "north" || args == "south" || args == "east" || args == "west" ||
            args == "up" || args == "down") {
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
          targ->SetSkill(crc32c("Open"), 0);
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

  if (cnum == COM_SEARCH) {
    if (!body->Parent())
      return 0;

    if (mind && (vmode & (LOC_NINJA | LOC_DARK)) == 0 && body->Parent()->LightLevel() < 100) {
      if (mind)
        mind->Send("It's too dark, you can't see anything.\n");
      return 0;
    }

    MinVec<1, Object*> targs;
    if (!args.empty()) {
      targs = body->PickObjects(
          std::string(args), vmode | LOC_NEARBY | LOC_ADJACENT | LOC_SELF | LOC_INTERNAL);
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
    if (body->Pos() == pos_t::USE && (!body->IsUsing(crc32c("Perception")))) {
      body->Parent()->SendOutF(
          stealth_t,
          stealth_s,
          ";s stops %s.\n",
          "You stop %s.\n",
          body,
          nullptr,
          body->UsingString().c_str());
    }
    body->StartUsing(crc32c("Perception"));
    body->SetSkill(crc32c("Hidden"), 0);
    for (auto targ : targs) {
      std::string denied = "";
      for (Object* own = targ; own; own = own->Parent()) {
        if (own->IsAnimate() && own != body && (!own->IsAct(act_t::SLEEP)) &&
            (!own->IsAct(act_t::DEAD)) && (!own->IsAct(act_t::DYING)) &&
            (!own->IsAct(act_t::UNCONSCIOUS))) {
          denied = "You would need ";
          denied += own->Name(1);
          denied += "'s permission to search ";
          denied += targ->Name(0, nullptr, own);
          denied += ".\n";
        } else if (
            own->Skill(crc32c("Container")) && (!own->Skill(crc32c("Open"))) &&
            own->Skill(crc32c("Locked"))) {
          denied = own->Name(1);
          if (own == targ) {
            denied += " is closed and locked so you can't search it.\n";
          } else {
            denied += " is closed and locked so you can't get to ";
            denied += targ->Name(1);
            denied += ".\n";
          }
          denied[0] = ascii_toupper(denied[0]);
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
        if (obj->Skill(crc32c("Hidden"))) {
          if (body->Roll(crc32c("Perception"), obj->Skill(crc32c("Hidden")))) {
            obj->SetSkill(crc32c("Hidden"), 0);
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

  if (cnum == COM_HIDE) {
    MinVec<1, Object*> targs;
    if (args.empty()) {
      targs.push_back(body);
    } else {
      targs = body->PickObjects(std::string(args), vmode | LOC_INTERNAL | LOC_NEARBY | LOC_SELF);
    }
    if (targs.size() < 1) {
      if (mind)
        mind->Send("You don't see that here.\n");
      return 0;
    }

    for (auto targ : targs) {
      std::string denied = "";
      for (Object* own = targ; own; own = own->Parent()) {
        if (own->IsAnimate() && own != body && (!own->IsAct(act_t::SLEEP)) &&
            (!own->IsAct(act_t::DEAD)) && (!own->IsAct(act_t::DYING)) &&
            (!own->IsAct(act_t::UNCONSCIOUS))) {
          denied = "You would need ";
          denied += own->Name(1);
          denied += "'s permission to hide ";
          denied += targ->Name(0, nullptr, own);
          denied += ".\n";
        } else if (
            own->Skill(crc32c("Container")) && (!own->Skill(crc32c("Open"))) &&
            own->Skill(crc32c("Locked"))) {
          if (own != targ) {
            denied = own->Name(1);
            denied += " is closed and locked so you can't get to ";
            denied += targ->Name(1);
            denied += ".\n";
            denied[0] = ascii_toupper(denied[0]);
          }
        }
      }
      if ((!nmode) && (denied.empty())) {
        if (mind)
          mind->Send(denied.c_str());
        continue;
      }
      if ((!nmode) && targ->Skill(crc32c("Obvious"))) {
        if (mind)
          mind->SendF("You could never hide %s, it's too obvious.", targ->Name(0, body).c_str());
        continue;
      }
      if ((!nmode) && targ->Skill(crc32c("Open"))) {
        if (targ->Skill(crc32c("Closeable"))) {
          if (mind)
            mind->SendF("You can't hide %s while it's open.", targ->Name(0, body).c_str());
        } else {
          if (mind)
            mind->SendF("You can't hide %s.  It's wide open.", targ->Name(0, body).c_str());
        }
        continue;
      }

      body->Parent()->SendOut(stealth_t, stealth_s, ";s hides ;s.\n", "you hide ;s.\n", body, targ);

      targ->SetSkill(crc32c("Hidden"), body->Roll(crc32c("Stealth"), 2) * 2);
    }
    return 0;
  }

  if (cnum == COM_EXAMINE) {
    Object* targ = nullptr;
    if (args.empty()) {
      if (mind)
        mind->Send("You want to examine what?\n");
      return 0;
    }
    targ = body->PickObject(std::string(args), vmode | LOC_INTERNAL | LOC_NEARBY | LOC_SELF);
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

  if (cnum == COM_CONSIDER) {
    Object* targ = nullptr;
    if (args.empty()) {
      if (mind)
        mind->Send("You want to consider what?\n");
      return 0;
    }
    targ = body->PickObject(std::string(args), vmode | LOC_INTERNAL | LOC_NEARBY | LOC_SELF);
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
      if (targ->HasSkill(crc32c("WeaponType"))) {
        handled = 1;
        Object* base = body->ActTarg(act_t::WIELD);
        if (base == targ) {
          mind->SendF("%s is your current weapon!\n", base->Name(0, body).c_str());
          mind->Send("Consider using something else for comparison.\n");
          return 0;
        }
        uint32_t sk = (get_weapon_skill(targ->Skill(crc32c("WeaponType"))));
        if (!body->HasSkill(sk)) {
          mind->SendF(
              CYEL "You don't know much about weapons like %s.\n" CNRM,
              targ->Name(1, body).c_str());
          mind->SendF(
              CYEL "You would need to learn the %s skill to know more.\n" CNRM,
              SkillName(sk).c_str());
        } else {
          int diff;
          mind->SendF("Use of this weapon would use your %s skill.\n", SkillName(sk).c_str());

          diff = body->Skill(sk);
          if (base)
            diff -= body->Skill(get_weapon_skill(base->Skill(crc32c("WeaponType"))));
          else
            diff -= body->Skill(crc32c("Punching"));
          if (diff > 0)
            mind->Send(CGRN "   ...would be a weapon you are more skilled with.\n" CNRM);
          else if (diff < 0)
            mind->Send(CYEL "   ...would be a weapon you are less skilled with.\n" CNRM);
          else
            mind->Send("   ...would be a weapon you are similarly skilled with.\n");

          diff = targ->Skill(crc32c("WeaponReach"));
          if (base)
            diff -= base->Skill(crc32c("WeaponReach"));
          if (diff > 0)
            mind->Send(CGRN "   ...would give you more reach.\n" CNRM);
          else if (diff < 0)
            mind->Send(CYEL "   ...would give you less reach.\n" CNRM);
          else
            mind->Send("   ...would give you similar reach.\n");

          diff = targ->Skill(crc32c("WeaponForce"));
          if (base)
            diff -= base->Skill(crc32c("WeaponForce"));
          if (diff > 0)
            mind->Send(CGRN "   ...would be more likely to do damage.\n" CNRM);
          else if (diff < 0)
            mind->Send(CYEL "   ...would be less likely to do damage.\n" CNRM);
          else
            mind->Send("   ...would be about as likely to do damage.\n");

          diff = targ->Skill(crc32c("WeaponSeverity"));
          if (base)
            diff -= base->Skill(crc32c("WeaponSeverity"));
          if (diff > 0)
            mind->Send(CGRN "   ...would do more damage.\n" CNRM);
          else if (diff < 0)
            mind->Send(CYEL "   ...would do less damage.\n" CNRM);
          else
            mind->Send("   ...would do similar damage.\n");

          diff = two_handed(targ->Skill(crc32c("WeaponType")));
          if (base)
            diff -= two_handed(base->Skill(crc32c("WeaponType")));
          if (diff > 0)
            mind->Send(CYEL "   ...would require both hands to use.\n" CNRM);
          else if (diff < 0)
            mind->Send(CGRN "   ...would not reqire both hands to use.\n" CNRM);
        }
      }

      // Containers
      int wtlimit = 0;
      int szlimit = 0;
      if (targ->HasSkill(crc32c("Container"))) {
        mind->SendF("%s is a container\n", targ->Name(1, body).c_str());

        wtlimit = targ->Skill(crc32c("Container"));
        szlimit = targ->Skill(crc32c("Capacity"));
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
      if (targ->HasSkill(crc32c("Liquid Container"))) {
        mind->SendF("%s is a liquid container\n", targ->Name(1, body).c_str());
        volume = targ->Skill(crc32c("Liquid Container"));
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

      Object* other = body->ActTarg(act_t::HOLD);
      if ((volume || wtlimit || szlimit) && other && other != targ) {
        // Containers
        if (szlimit && other->HasSkill(crc32c("Capacity"))) {
          if (szlimit < other->Skill(crc32c("Capacity"))) {
            mind->Send(CYEL "   ...it can't fit as much, " CNRM);
          } else if (szlimit > other->Skill(crc32c("Capacity"))) {
            mind->Send(CGRN "   ...it can fit more, " CNRM);
          } else {
            mind->Send("   ...it can fit the same, ");
          }
        }
        if (wtlimit && other->HasSkill(crc32c("Container"))) {
          if (wtlimit < other->Skill(crc32c("Container"))) {
            mind->SendF(CYEL "and can't carry as much as %s.\n" CNRM, other->Name(0, body).c_str());
          } else if (wtlimit > other->Skill(crc32c("Container"))) {
            mind->SendF(CGRN "and can carry more than %s.\n" CNRM, other->Name(0, body).c_str());
          } else {
            mind->SendF("and can carry the same as %s.\n", other->Name(0, body).c_str());
          }
        }

        // Liquid Containers
        if (volume && other->HasSkill(crc32c("Liquid Container"))) {
          if (volume < other->Skill(crc32c("Liquid Container"))) {
            mind->SendF(
                CYEL "   ...it can't hold as much as %s.\n" CNRM, other->Name(0, body).c_str());
          } else if (volume > other->Skill(crc32c("Liquid Container"))) {
            mind->SendF(
                CGRN "   ...it can hold more than %s.\n" CNRM, other->Name(0, body).c_str());
          } else {
            mind->SendF(
                "   ...it can hold about the same amount as %s.\n", other->Name(0, body).c_str());
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
        mind->SendF("%s can be worn in %d different ways:\n", targ->Name(0, body).c_str(), num);
      } else if (num == 1) {
        mind->SendF("%s can only be worn one way:\n", targ->Name(0, body).c_str());
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
              mind->SendF("   ...it would replace %s.\n", repl->Name(0, body).c_str());

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
        mind->SendF(
            "You really don't know what you would do with %s.\n", targ->Name(1, body).c_str());
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
        std::string mes = targ->Name() + "...\n";
        mes[0] = ascii_toupper(mes[0]);
        mind->Send(mes.c_str());

        if ((!targ->ActTarg(act_t::WIELD)) && (!body->ActTarg(act_t::WIELD))) {
          mind->Send("   ...is unarmed, but so are you.\n");
        } else if (!targ->ActTarg(act_t::WIELD)) {
          mind->Send(CGRN "   ...is unarmed.\n" CNRM);
        } else if (!body->ActTarg(act_t::WIELD)) {
          mind->Send(CYEL "   ...is armed, and you are not!\n" CNRM);
        }

        if (targ->HasSkill(crc32c("NaturalWeapon")) && body->HasSkill(crc32c("NaturalWeapon"))) {
          mind->Send("   ...has natural weaponry, but so do you.\n");
        } else if (body->HasSkill(crc32c("NaturalWeapon"))) {
          mind->Send(CGRN "   ...has no natural weaponry, but you do.\n" CNRM);
        } else if (targ->HasSkill(crc32c("NaturalWeapon"))) {
          mind->Send(CYEL "   ...has natural weaponry, and you do not!\n" CNRM);
        }

        diff = 0;
        if (body->ActTarg(act_t::WIELD))
          diff = (body->ActTarg(act_t::WIELD)->Skill(crc32c("WeaponReach")) > 9);
        if (targ->ActTarg(act_t::WIELD) &&
            targ->ActTarg(act_t::WIELD)->Skill(crc32c("WeaponReach")) > 9) {
          if (diff)
            mind->Send("   ...has a ranged weapon, and so do you!\n");
          else
            mind->Send(CYEL "   ...has a ranged weapon!\n" CNRM);
        } else if (diff) {
          mind->Send(CGRN "   ...doesn't have a ranged weapon, and you do!\n" CNRM);
        } else {
          diff = 0;
          if (body->ActTarg(act_t::WIELD))
            diff += body->ActTarg(act_t::WIELD)->Skill(crc32c("WeaponReach"));
          if (targ->ActTarg(act_t::WIELD))
            diff -= targ->ActTarg(act_t::WIELD)->Skill(crc32c("WeaponReach"));
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

        if ((!targ->ActTarg(act_t::WEAR_SHIELD)) && (!body->ActTarg(act_t::WEAR_SHIELD))) {
          mind->Send("   ...has no shield, but neither do you.\n");
        } else if (!targ->ActTarg(act_t::WEAR_SHIELD)) {
          mind->Send(CGRN "   ...has no shield.\n" CNRM);
        } else if (!body->ActTarg(act_t::WEAR_SHIELD)) {
          mind->Send(CYEL "   ...has a shield, and you do not!\n" CNRM);
        }

        diff = 0;
        uint32_t sk = crc32c("Punching");
        if (body->IsAct(act_t::WIELD)) {
          sk = get_weapon_skill(body->ActTarg(act_t::WIELD)->Skill(crc32c("WeaponType")));
        }
        if (body->HasSkill(sk)) {
          diff += body->Skill(sk);
        }
        sk = crc32c("Punching");
        if (targ->IsAct(act_t::WIELD)) {
          sk = get_weapon_skill(targ->ActTarg(act_t::WIELD)->Skill(crc32c("WeaponType")));
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

        if (targ->HasSkill(crc32c("TBAAction"))) {
          if ((targ->Skill(crc32c("TBAAction")) & 4128) == 0) {
            mind->Send(CGRN "   ...does not seem threatening.\n" CNRM);
          } else if ((targ->Skill(crc32c("TBAAction")) & 160) == 32) {
            mind->Send(CRED "   ...is spoiling for a fight.\n" CNRM);
          } else if ((targ->Skill(crc32c("TBAAction")) & 160) == 160) {
            mind->Send(CRED "   ...seems to be trolling for victems.\n" CNRM);
          } else if (targ->Skill(crc32c("TBAAction")) & 4096) {
            mind->Send(CYEL "   ...seems to be on the look-out for trouble.\n" CNRM);
          } else {
            mind->Send("   ...is impossible - tell the Ninjas[TM].\n");
          }
        }

        if (targ->Skill(crc32c("Accomplishment"))) {
          if (body->HasAccomplished(targ->Skill(crc32c("Accomplishment")))) {
            mind->Send(CYEL "   ...has been defeated by you already.\n" CNRM);
          } else {
            mind->Send(CGRN "   ...has never been defeated by you.\n" CNRM);
          }
        }
      }
    }
    return 0;
  }

  if (cnum == COM_SCORE) {
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

  if (cnum == COM_TIME) {
    if (!mind)
      return 0;
    if (body) {
      Object* world = body->World();
      if (world) {
        if (world->Skill(crc32c("Day Time")) && world->Skill(crc32c("Day Length"))) {
          int curtime = world->Skill(crc32c("Day Time"));
          curtime *= 24 * 60;
          curtime /= world->Skill(crc32c("Day Length"));
          mind->SendF("The time is now %d:%.2d in this world.\n", curtime / 60, curtime % 60);
        } else {
          mind->SendF("This world has no concept of time....\n");
        }
      } else {
        mind->SendF("This character is not in any world.\n");
      }
    } else {
      mind->SendF("You need to enter a character before doing this.\n");
    }
    return 0;
  }

  if (cnum == COM_WORLD) {
    if (!mind)
      return 0;
    mind->SendF("This world is called: %s\n", body->World()->ShortDescC());
    return 0;
  }

  if (cnum == COM_STATS) {
    Object* targ = nullptr;
    if ((!body) && args.empty()) {
      targ = mind->Owner()->Creator();
    } else if (!body) {
      targ = mind->Owner()->Room()->PickObject(std::string(args), vmode | LOC_INTERNAL);
    } else if (args.empty()) {
      targ = body;
    } else {
      targ = body->PickObject(
          std::string(args), vmode | LOC_INTERNAL | LOC_NEARBY | LOC_SELF | LOC_HERE);
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

  if (cnum == COM_LOCK) {
    Object* targ = nullptr;
    if (args.empty()) {
      if (mind)
        mind->Send("You want to lock what?\n");
      return 0;
    }
    targ = body->PickObject(std::string(args), vmode | LOC_INTERNAL | LOC_NEARBY);
    if (!targ) {
      if (mind)
        mind->Send("You don't see that here.\n");
    } else if (targ->Skill(crc32c("Locked"))) {
      if (mind)
        mind->Send("It is already locked!\n");
    } else if (targ->Skill(crc32c("Lock")) <= 0 && (!nmode)) {
      if (mind)
        mind->Send("It does not seem to have a keyhole!\n");
    } else {
      if (!nmode) {
        auto keys = body->PickObjects("all", vmode | LOC_INTERNAL);
        bool can_open = false;
        for (auto key : keys) {
          if (key->Skill(crc32c("Key")) == targ->Skill(crc32c("Lock"))) {
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
      targ->SetSkill(crc32c("Locked"), 1);
      body->Parent()->SendOut(stealth_t, stealth_s, ";s locks ;s.\n", "You lock ;s.\n", body, targ);
      if (targ->ActTarg(act_t::SPECIAL_MASTER)) {
        Object* targ2 = targ->ActTarg(act_t::SPECIAL_MASTER);
        targ2->Parent()->SendOut(stealth_t, stealth_s, ";s locks.\n", "", targ2, nullptr);
        targ2->SetSkill(crc32c("Locked"), 1);
      }
    }
    return 0;
  }

  if (cnum == COM_UNLOCK) {
    Object* targ = nullptr;
    if (args.empty()) {
      if (mind)
        mind->Send("You want to unlock what?\n");
      return 0;
    }
    targ = body->PickObject(std::string(args), vmode | LOC_INTERNAL | LOC_NEARBY);
    if (!targ) {
      if (mind)
        mind->Send("You don't see that here.\n");
    } else if (!targ->Skill(crc32c("Locked"))) {
      if (mind)
        mind->Send("It is not locked!\n");
    } else if (targ->Skill(crc32c("Lock")) <= 0 && (!nmode)) {
      if (mind)
        mind->Send("It does not seem to have a keyhole!\n");
    } else {
      if (!nmode) {
        auto keys = body->PickObjects("all", vmode | LOC_INTERNAL);
        bool can_open = false;
        for (auto key : keys) {
          if (key->Skill(crc32c("Key")) == targ->Skill(crc32c("Lock"))) {
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
      targ->SetSkill(crc32c("Locked"), 0);
      body->Parent()->SendOut(
          stealth_t, stealth_s, ";s unlocks ;s.\n", "You unlock ;s.\n", body, targ);
      if (targ->ActTarg(act_t::SPECIAL_MASTER)) {
        Object* targ2 = targ->ActTarg(act_t::SPECIAL_MASTER);
        targ2->Parent()->SendOut(stealth_t, stealth_s, ";s unlocks.\n", "", targ2, nullptr);
        targ2->SetSkill(crc32c("Locked"), 1);
      }
    }
    return 0;
  }

  if (cnum == COM_OPEN) {
    Object* targ = nullptr;
    if (args.empty()) {
      if (mind)
        mind->Send("You want to open what?\n");
      return 0;
    }
    targ = body->PickObject(std::string(args), vmode | LOC_INTERNAL | LOC_NEARBY);
    if (!targ) {
      if (mind)
        mind->Send("You don't see that here.\n");
    } else if (!targ->Skill(crc32c("Closeable"))) {
      if (mind)
        mind->Send("That can't be opened or closed.\n");
    } else if (targ->Skill(crc32c("Open"))) {
      if (mind)
        mind->Send("It's already open!\n");
    } else if (targ->Skill(crc32c("Locked"))) {
      if (mind)
        mind->Send("It is locked!\n");
    } else {
      targ->SetSkill(crc32c("Open"), 1000);
      body->Parent()->SendOut(stealth_t, stealth_s, ";s opens ;s.\n", "You open ;s.\n", body, targ);
      if (targ->ActTarg(act_t::SPECIAL_MASTER)) {
        Object* targ2 = targ->ActTarg(act_t::SPECIAL_MASTER);
        targ2->Parent()->SendOut(stealth_t, stealth_s, ";s opens.\n", "", targ2, nullptr);
        targ2->SetSkill(crc32c("Open"), 1000);
        targ2->SetSkill(crc32c("Locked"), 0); // FIXME: Do I want to do this?
      }
    }
    return 0;
  }

  if (cnum == COM_CLOSE) {
    Object* targ = nullptr;
    if (args.empty()) {
      if (mind)
        mind->Send("You want to close what?\n");
      return 0;
    }
    targ = body->PickObject(std::string(args), vmode | LOC_INTERNAL | LOC_NEARBY);
    if (!targ) {
      if (mind)
        mind->Send("You don't see that here.\n");
    } else if (!targ->Skill(crc32c("Closeable"))) {
      if (mind)
        mind->Send("That can't be opened or closed.\n");
    } else if (!targ->Skill(crc32c("Open"))) {
      if (mind)
        mind->Send("It's already closed!\n");
    } else if (targ->Skill(crc32c("Locked"))) {
      if (mind)
        mind->Send("It is locked!\n");
    } else {
      targ->SetSkill(crc32c("Open"), 0);
      body->Parent()->SendOut(
          stealth_t, stealth_s, ";s closes ;s.\n", "You close ;s.\n", body, targ);
      if (targ->ActTarg(act_t::SPECIAL_MASTER)) {
        Object* targ2 = targ->ActTarg(act_t::SPECIAL_MASTER);
        targ2->Parent()->SendOut(stealth_t, stealth_s, ";s closes.\n", "", targ2, nullptr);
        targ2->SetSkill(crc32c("Open"), 0);
        targ2->SetSkill(crc32c("Locked"), 0); // FIXME: Do I want to do this?
      }
    }
    return 0;
  }

  if (cnum == COM_LIST) {
    if (!mind)
      return 0;

    auto objs = body->Parent()->Contents(vmode);
    MinVec<1, Object*> shpkps;
    std::string reason = "";
    for (auto shpkp : objs) {
      if (shpkp->Skill(crc32c("Sell Profit"))) {
        if (shpkp->IsAct(act_t::DEAD)) {
          reason = "Sorry, the shopkeeper is dead!\n";
        } else if (shpkp->IsAct(act_t::DYING)) {
          reason = "Sorry, the shopkeeper is dying!\n";
        } else if (shpkp->IsAct(act_t::UNCONSCIOUS)) {
          reason = "Sorry, the shopkeeper is unconscious!\n";
        } else if (shpkp->IsAct(act_t::SLEEP)) {
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
      if (shpkp->ActTarg(act_t::WEAR_RSHOULDER) &&
          shpkp->ActTarg(act_t::WEAR_RSHOULDER)->Skill(crc32c("Vortex"))) {
        Object* vortex = shpkp->ActTarg(act_t::WEAR_RSHOULDER);
        objs = vortex->Contents(vmode);
        auto oobj = objs.front();
        for (auto obj : objs) {
          if (obj != objs.front() && obj->IsSameAs(*oobj))
            continue;
          int price = obj->Value();
          if (obj->Skill(crc32c("Money")) != obj->Value()) { // Not 1-1 Money
            price *= shpkp->Skill(crc32c("Sell Profit"));
            price += 999;
            price /= 1000;
          }
          mind->SendF("%10d gp: %s\n", price, obj->ShortDescC());
          oobj = obj;
        }
      }
    }
    return 0;
  }

  if (cnum == COM_BUY) {
    if (args.empty()) {
      if (mind)
        mind->Send("What do you want to buy?\n");
      return 0;
    }

    auto objs = body->Parent()->Contents(vmode);
    MinVec<1, Object*> shpkps;
    std::string reason = "";
    for (auto shpkp : objs) {
      if (shpkp->Skill(crc32c("Sell Profit"))) {
        if (shpkp->IsAct(act_t::DEAD)) {
          reason = "Sorry, the shopkeeper is dead!\n";
        } else if (shpkp->IsAct(act_t::DYING)) {
          reason = "Sorry, the shopkeeper is dying!\n";
        } else if (shpkp->IsAct(act_t::UNCONSCIOUS)) {
          reason = "Sorry, the shopkeeper is unconscious!\n";
        } else if (shpkp->IsAct(act_t::SLEEP)) {
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
      if (shpkp->ActTarg(act_t::WEAR_RSHOULDER) &&
          shpkp->ActTarg(act_t::WEAR_RSHOULDER)->Skill(crc32c("Vortex"))) {
        Object* vortex = shpkp->ActTarg(act_t::WEAR_RSHOULDER);

        auto targs = vortex->PickObjects(std::string(args), vmode | LOC_INTERNAL);
        if (!targs.size()) {
          if (mind)
            mind->Send("The shopkeeper doesn't have that.\n");
          return 0;
        }

        for (auto targ : targs) {
          int price = targ->Value() * targ->Quantity();
          if (price < 0) {
            if (mind)
              mind->SendF("You can't sell %s.\n", targ->Name(0, body).c_str());
            continue;
          } else if (price == 0) {
            if (mind) {
              std::string mes = targ->Name(0, body);
              mes += " is worthless.\n";
              mes[0] = ascii_toupper(mes[0]);
              mind->Send(mes.c_str());
            }
            continue;
          }

          if (targ->Skill(crc32c("Money")) != targ->Value()) { // Not 1-1 Money
            price *= shpkp->Skill(crc32c("Sell Profit"));
            price += 999;
            price /= 1000;
          }
          mind->SendF("%d gp: %s\n", price, targ->ShortDescC());

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
              ((!body->IsAct(act_t::HOLD)) ||
               body->ActTarg(act_t::HOLD) == body->ActTarg(act_t::WIELD) ||
               body->ActTarg(act_t::HOLD) == body->ActTarg(act_t::WEAR_SHIELD)) &&
              (!targ->Travel(body))) {
            if (body->IsAct(act_t::HOLD)) {
              body->Parent()->SendOut(
                  stealth_t,
                  stealth_s,
                  ";s stops holding ;s.\n",
                  "You stop holding ;s.\n",
                  body,
                  body->ActTarg(act_t::HOLD));
              body->StopAct(act_t::HOLD);
            }
            body->AddAct(act_t::HOLD, targ);
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
              mind->SendF("You can't stash or hold %s.\n", targ->Name(1).c_str());
          }
        }
      }
    }
    return 0;
  }

  if (cnum == COM_VALUE || cnum == COM_SELL) {
    if (args.empty()) {
      if (mind)
        mind->Send("What do you want to sell?\n");
      return 0;
    }

    Object* targ = body->PickObject(std::string(args), vmode | LOC_NOTWORN | LOC_INTERNAL);
    if ((!targ) && body->ActTarg(act_t::HOLD) &&
        body->ActTarg(act_t::HOLD)->Parent() != body // Dragging
        && body->ActTarg(act_t::HOLD)->Matches(std::string(args))) {
      targ = body->ActTarg(act_t::HOLD);
    }

    if (!targ) {
      if (mind)
        mind->Send("You want to sell what?\n");
      return 0;
    }

    if (targ->Skill(crc32c("Container")) || targ->Skill(crc32c("Liquid Container"))) {
      if (mind) {
        std::string mes = targ->Name(0, body);
        mes += " is a container.";
        mes += "  You can't sell containers (yet).\n";
        mes[0] = ascii_toupper(mes[0]);
        mind->Send(mes.c_str());
      }
      return 0;
    }

    if (targ->Contents(LOC_TOUCH | LOC_NOTFIXED).size() > 0) {
      if (mind) {
        std::string mes = targ->Name(0, body);
        mes += " is not empty.";
        mes += "  You must empty it before you can sell it.\n";
        mes[0] = ascii_toupper(mes[0]);
        mind->Send(mes.c_str());
      }
      return 0;
    }

    int price = targ->Value() * targ->Quantity();
    if (price < 0 || targ->HasSkill(crc32c("Priceless")) || targ->HasSkill(crc32c("Cursed"))) {
      if (mind)
        mind->SendF("You can't sell %s.\n", targ->Name(0, body).c_str());
      return 0;
    }
    if (price == 0) {
      if (mind)
        mind->SendF("%s is worthless.\n", targ->Name(0, body).c_str());
      return 0;
    }

    int wearable = 0;
    if (targ->HasSkill(crc32c("Wearable on Back")))
      wearable = 1;
    if (targ->HasSkill(crc32c("Wearable on Chest")))
      wearable = 1;
    if (targ->HasSkill(crc32c("Wearable on Head")))
      wearable = 1;
    if (targ->HasSkill(crc32c("Wearable on Neck")))
      wearable = 1;
    if (targ->HasSkill(crc32c("Wearable on Collar")))
      wearable = 1;
    if (targ->HasSkill(crc32c("Wearable on Waist")))
      wearable = 1;
    if (targ->HasSkill(crc32c("Wearable on Shield")))
      wearable = 1;
    if (targ->HasSkill(crc32c("Wearable on Left Arm")))
      wearable = 1;
    if (targ->HasSkill(crc32c("Wearable on Right Arm")))
      wearable = 1;
    if (targ->HasSkill(crc32c("Wearable on Left Finger")))
      wearable = 1;
    if (targ->HasSkill(crc32c("Wearable on Right Finger")))
      wearable = 1;
    if (targ->HasSkill(crc32c("Wearable on Left Foot")))
      wearable = 1;
    if (targ->HasSkill(crc32c("Wearable on Right Foot")))
      wearable = 1;
    if (targ->HasSkill(crc32c("Wearable on Left Hand")))
      wearable = 1;
    if (targ->HasSkill(crc32c("Wearable on Right Hand")))
      wearable = 1;
    if (targ->HasSkill(crc32c("Wearable on Left Leg")))
      wearable = 1;
    if (targ->HasSkill(crc32c("Wearable on Right Leg")))
      wearable = 1;
    if (targ->HasSkill(crc32c("Wearable on Left Wrist")))
      wearable = 1;
    if (targ->HasSkill(crc32c("Wearable on Right Wrist")))
      wearable = 1;
    if (targ->HasSkill(crc32c("Wearable on Left Shoulder")))
      wearable = 1;
    if (targ->HasSkill(crc32c("Wearable on Right Shoulder")))
      wearable = 1;
    if (targ->HasSkill(crc32c("Wearable on Left Hip")))
      wearable = 1;
    if (targ->HasSkill(crc32c("Wearable on Right Hip")))
      wearable = 1;

    auto objs = body->Parent()->Contents();
    MinVec<1, Object*> shpkps;
    std::string reason = "Sorry, nobody is buying that sort of thing here.\n";
    uint32_t skill = crc32c("None");
    for (auto shpkp : objs) {
      if (shpkp->IsAct(act_t::DEAD)) {
        reason = "Sorry, the shopkeeper is dead!\n";
      } else if (shpkp->IsAct(act_t::DYING)) {
        reason = "Sorry, the shopkeeper is dying!\n";
      } else if (shpkp->IsAct(act_t::UNCONSCIOUS)) {
        reason = "Sorry, the shopkeeper is unconscious!\n";
      } else if (shpkp->IsAct(act_t::SLEEP)) {
        reason = "Sorry, the shopkeeper is asleep!\n";
      } else if (targ->Skill(crc32c("Money")) == targ->Value()) { // 1-1 Money
        for (auto skl : shpkp->GetSkills()) {
          if (!strncmp(SkillName(skl.first).c_str(), "Buy ", 4)) {
            skill = crc32c("Money");
            break;
          }
        }
      } else if (wearable && targ->NormAttribute(0) > 0) {
        if (shpkp->HasSkill(crc32c("Buy Armor"))) {
          skill = crc32c("Buy Armor");
        }
      } else if (targ->Skill(crc32c("Vehicle")) == 4) {
        if (shpkp->HasSkill(crc32c("Buy Boat"))) {
          skill = crc32c("Buy Boat");
        }
      } else if (targ->HasSkill(crc32c("Container"))) {
        if (shpkp->HasSkill(crc32c("Buy Container"))) {
          skill = crc32c("Buy Container");
        }
      } else if (targ->HasSkill(crc32c("Food")) && (!targ->HasSkill(crc32c("Drink")))) {
        if (shpkp->HasSkill(crc32c("Buy Food"))) {
          skill = crc32c("Buy Food");
        }
      }
      //      else if(false) {				//FIXME: Implement
      //	if(shpkp->HasSkill(crc32c("Buy Light"))) {
      //	  skill = crc32c("Buy Light");
      //	  }
      //	}
      else if (targ->HasSkill(crc32c("Liquid Container"))) { // FIXME: Not Potions?
        if (shpkp->HasSkill(crc32c("Buy Liquid Container"))) {
          skill = crc32c("Buy Liquid Container");
        }
      } else if (targ->HasSkill(crc32c("Liquid Container"))) { // FIXME: Not Bottles?
        if (shpkp->HasSkill(crc32c("Buy Potion"))) {
          skill = crc32c("Buy Potion");
        }
      } else if (targ->HasSkill(crc32c("Magical Scroll"))) {
        if (shpkp->HasSkill(crc32c("Buy Scroll"))) {
          skill = crc32c("Buy Scroll");
        }
      } else if (targ->HasSkill(crc32c("Magical Staff"))) {
        if (shpkp->HasSkill(crc32c("Buy Staff"))) {
          skill = crc32c("Buy Staff");
        }
      } else if (targ->HasSkill(crc32c("Magical Wand"))) {
        if (shpkp->HasSkill(crc32c("Buy Wand"))) {
          skill = crc32c("Buy Wand");
        }
      }
      //      else if(false) {				//FIXME: Implement
      //	if(shpkp->HasSkill(crc32c("Buy Trash"))) {
      //	  skill = crc32c("Buy Trash");
      //	  }
      //	}
      //      else if(false) {				//FIXME: Implement
      //	if(shpkp->HasSkill(crc32c("Buy Treasure"))) {
      //	  skill = crc32c("Buy Treasure");
      //	  }
      //	}
      else if (targ->Skill(crc32c("WeaponType")) > 0) {
        if (shpkp->HasSkill(crc32c("Buy Weapon"))) {
          skill = crc32c("Buy Weapon");
        }
      } else if (wearable && targ->NormAttribute(0) == 0) {
        if (shpkp->HasSkill(crc32c("Buy Worn"))) {
          skill = crc32c("Buy Worn");
        }
      }
      //      else if(false) {					//FIXME:
      //      Implement
      //	if(shpkp->HasSkill(crc32c("Buy Other"))) {
      //	  skill = crc32c("Buy Other");
      //	  }
      //	}

      if (skill == crc32c("None") && shpkp->HasSkill(crc32c("Buy All"))) {
        skill = crc32c("Buy All");
      }

      if (skill != crc32c("None")) {
        shpkps.push_back(shpkp);
      }
    }
    if (shpkps.size() == 0) {
      if (mind) {
        mind->Send(reason.c_str());
      }
    } else {
      Object* shpkp = shpkps.front();
      if (shpkp->ActTarg(act_t::WEAR_RSHOULDER) &&
          shpkp->ActTarg(act_t::WEAR_RSHOULDER)->Skill(crc32c("Vortex"))) {
        Object* vortex = shpkp->ActTarg(act_t::WEAR_RSHOULDER);

        if (skill != crc32c("Money")) { // Not 1-1 Money
          price *= shpkp->Skill(skill);
          price += 0;
          price /= 1000;
          if (price <= 0)
            price = 1;
        }
        mind->SendF("I'll give you %dgp for %s\n", price, targ->ShortDescC());

        if (cnum == COM_SELL) {
          int togo = price, ord = -price;
          auto pay = shpkp->PickObjects("a gold piece", vmode | LOC_INTERNAL, &ord);
          for (auto coin : pay) {
            togo -= std::max(1, coin->Skill(crc32c("Quantity")));
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

  if (cnum == COM_DRAG) {
    if (args.empty()) {
      if (mind)
        mind->Send("What do you want to drag?\n");
      return 0;
    }

    if (body->IsAct(act_t::HOLD)) {
      if (mind)
        mind->Send("You are already holding something.  Drop it first.\n");
      return 0;
    }

    Object* targ = body->PickObject(std::string(args), vmode | LOC_NEARBY);
    if (!targ) {
      if (mind)
        mind->Send("You want to drag what?\n");
      return 0;
    }

    if (targ->Pos() == pos_t::NONE) {
      if (mind)
        mind->SendF("You can't drag %s, it is fixed in place!\n", targ->Name().c_str());
    } else if (targ->IsAnimate()) {
      std::string denied = "You would need ";
      denied += targ->Name(1);
      denied += "'s permission to drag ";
      denied += targ->Name(0, nullptr, targ);
      denied += ".\n";
      if (mind)
        mind->SendF(denied.c_str(), targ->Name().c_str());
    } else if (targ->Weight() > body->ModAttribute(2) * 50000) {
      if (mind)
        mind->SendF("You could never lift %s, it is too heavy.\n", targ->Name().c_str());
    } else {
      body->AddAct(act_t::HOLD, targ);
      body->Parent()->SendOut(
          stealth_t, stealth_s, ";s starts dragging ;s.\n", "You start dragging ;s.\n", body, targ);
    }
    return 0;
  }

  if (cnum == COM_GET) {
    if (args.empty()) {
      if (mind)
        mind->Send("What do you want to get?\n");
      return 0;
    }

    auto targs = body->PickObjects(std::string(args), vmode | LOC_NEARBY);
    if (targs.size() == 0) {
      if (mind)
        mind->Send("You want to get what?\n");
      return 0;
    }

    for (auto targ : targs) {
      auto trigs = targ->PickObjects("all tbamud trigger script", LOC_NINJA | LOC_INTERNAL);
      for (auto trig : trigs) {
        int ttype = trig->Skill(crc32c("TBAScriptType"));
        if ((ttype & 0x2000040) == 0x2000040) { // OBJ-GET trigs
          if ((rand() % 100) < trig->Skill(crc32c("TBAScriptNArg"))) { // % Chance
            if (new_trigger(0, trig, body))
              return 0; // Says fail!
            if (targ->Parent() == targ->TrashBin())
              return 0; // Purged it
          }
        }
      }

      if ((!nmode) && targ->Pos() == pos_t::NONE) {
        if (mind)
          mind->SendF("You can't get %s, it is fixed in place!\n", targ->Name().c_str());
      } else if ((!nmode) && targ->IsAnimate()) {
        if (mind)
          mind->SendF("You can't get %s, it is not inanimate.\n", targ->Name().c_str());
      } else if ((!nmode) && targ->Weight() > body->ModAttribute(2) * 50000) {
        if (mind)
          mind->SendF("You could never lift %s, it is too heavy.\n", targ->Name().c_str());
      } else if ((!nmode) && targ->Weight() > body->ModAttribute(2) * 10000) {
        if (mind)
          mind->SendF(
              "You can't carry %s, it is too heavy.  Try 'drag' instead.\n", targ->Name().c_str());
      } else {
        std::string denied = "";
        for (Object* owner = targ->Parent(); owner; owner = owner->Parent()) {
          if (owner->IsAnimate() && owner != body && (!owner->IsAct(act_t::SLEEP)) &&
              (!owner->IsAct(act_t::DEAD)) && (!owner->IsAct(act_t::DYING)) &&
              (!owner->IsAct(act_t::UNCONSCIOUS)) &&
              (owner->ActTarg(act_t::OFFER) != body || owner->ActTarg(act_t::HOLD) != targ)) {
            denied = "You would need ";
            denied += owner->Name(1);
            denied += "'s permission to get ";
            denied += targ->Name(0, nullptr, owner);
            denied += ".\n";
          } else if (
              owner->Skill(crc32c("Container")) && (!owner->Skill(crc32c("Open"))) &&
              owner->Skill(crc32c("Locked"))) {
            denied = owner->Name(1);
            denied += " is closed and locked so you can't get to ";
            denied += targ->Name(1);
            denied += ".\n";
            denied[0] = ascii_toupper(denied[0]);
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
          if (targ->HasSkill(crc32c("Perishable"))) {
            targ->Deactivate();
          }
        } else if (
            body->IsAct(act_t::HOLD) &&
            body->ActTarg(act_t::HOLD) != body->ActTarg(act_t::WEAR_SHIELD) &&
            body->ActTarg(act_t::HOLD) != body->ActTarg(act_t::WIELD)) {
          if (mind)
            mind->SendF("You have no place to stash %s.\n", targ->Name().c_str());
        } else if (targ->Skill(crc32c("Quantity")) > 1) {
          if (mind)
            mind->SendF("You have no place to stash %s.\n", targ->Name().c_str());
        } else {
          if (body->IsAct(act_t::HOLD)) {
            body->Parent()->SendOut(
                stealth_t,
                stealth_s,
                ";s stops holding ;s.\n",
                "You stop holding ;s.\n",
                body,
                body->ActTarg(act_t::HOLD));
            body->StopAct(act_t::HOLD);
          }
          targ->Travel(body);
          body->AddAct(act_t::HOLD, targ);
          body->Parent()->SendOut(
              stealth_t,
              stealth_s,
              ";s gets and holds ;s.\n",
              "You get and hold ;s.\n",
              body,
              targ);
          if (targ->HasSkill(crc32c("Perishable"))) {
            targ->Deactivate();
          }
        }
      }
    }
    return 0;
  }

  if (cnum == COM_UNLABEL) {
    if (!body->IsAct(act_t::HOLD)) {
      if (mind)
        mind->Send("You must first 'hold' the object you want label.\n");
      return 0;
    } else if (!body->ActTarg(act_t::HOLD)) {
      if (mind)
        mind->Send("What!?!?!  You are holding nothing?\n");
      return 0;
    }

    std::string name = body->ActTarg(act_t::HOLD)->ShortDesc();
    size_t start = name.find_first_of('(');
    if (start != name.npos) {
      name = name.substr(0, start);
      trim_string(name);
      body->ActTarg(act_t::HOLD)->SetShortDesc(name.c_str());
      if (mind)
        mind->SendF("%s is now unlabeled.\n", body->ActTarg(act_t::HOLD)->Name(1, body).c_str());
    } else {
      if (mind)
        mind->SendF(
            "%s does not have a label to remove.\n",
            body->ActTarg(act_t::HOLD)->Name(1, body).c_str());
    }
    return 0;
  }

  if (cnum == COM_LABEL) {
    if (!body->IsAct(act_t::HOLD)) {
      if (mind)
        mind->Send("You must first 'hold' the object you want label.\n");
      return 0;
    } else if (!body->ActTarg(act_t::HOLD)) {
      if (mind)
        mind->Send("What!?!?!  You are holding nothing?\n");
      return 0;
    }

    if (args.empty()) { // Just Checking Label
      std::string label = body->ActTarg(act_t::HOLD)->ShortDesc();
      size_t start = label.find_first_of('(');
      if (start == label.npos) {
        if (mind)
          mind->SendF("%s has no label.\n", body->ActTarg(act_t::HOLD)->Name(1, body).c_str());
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
              "%s is labeled '%s'.\n",
              body->ActTarg(act_t::HOLD)->Name(1, body).c_str(),
              label.c_str());
      }
    } else { // Adding to Label
      std::string name = body->ActTarg(act_t::HOLD)->ShortDesc();
      std::string label = name;
      size_t start = label.find_first_of('(');
      if (start == label.npos) {
        label = (std::string(args));
        trim_string(label);
      } else {
        name = label.substr(0, start);
        trim_string(name);
        label = label.substr(start + 1);
        size_t end = label.find_last_of(')');
        if (end != label.npos)
          label = label.substr(0, end);
        trim_string(label);
        if (matches(label.c_str(), std::string(args))) {
          if (mind)
            mind->SendF(
                "%s already has that on the label.\n",
                body->ActTarg(act_t::HOLD)->Name(1, body).c_str());
          return 0;
        } else {
          label += " ";
          label += (std::string(args));
          trim_string(label);
        }
      }
      body->ActTarg(act_t::HOLD)->SetShortDesc(name.c_str());
      if (mind)
        mind->SendF(
            "%s is now labeled '%s'.\n",
            body->ActTarg(act_t::HOLD)->Name(1, body).c_str(),
            label.c_str());
      body->ActTarg(act_t::HOLD)->SetShortDesc((name + " (" + label + ")").c_str());
    }

    return 0;
  }

  if (cnum == COM_PUT) {
    if (args.substr(0, 3) == "in ") {
      auto prefix = args.find_first_not_of(" \t\n\r", 3);
      if (prefix == std::string::npos) {
        args = "";
      } else {
        args = args.substr(prefix);
      }
    }

    if (!body->IsAct(act_t::HOLD)) {
      if (mind)
        mind->Send("You must first 'hold' the object you want to 'put'.\n");
      return 0;
    } else if (!body->ActTarg(act_t::HOLD)) {
      if (mind)
        mind->Send("What!?!?!  You are holding nothing?\n");
      return 0;
    }

    if (args.empty()) {
      if (mind)
        mind->SendF(
            "What do you want to put %s in?\n", body->ActTarg(act_t::HOLD)->Name(0, body).c_str());
      return 0;
    }

    Object* targ = body->PickObject(std::string(args), vmode | LOC_NEARBY | LOC_INTERNAL);
    if (!targ) {
      if (mind)
        mind->SendF(
            "I don't see '%s' to put '%s' in!\n",
            std::string(args).c_str(),
            body->ActTarg(act_t::HOLD)->Name(0, body).c_str());
    } else if (targ->IsAnimate()) {
      if (mind)
        mind->Send("You can only put things in inanimate objects!\n");
    } else if (!targ->Skill(crc32c("Container"))) {
      if (mind)
        mind->Send("You can't put anything in that, it is not a container.\n");
    } else if (targ->Skill(crc32c("Locked"))) {
      if (mind)
        mind->Send("You can't put anything in that, it is locked.\n");
    } else if (targ == body->ActTarg(act_t::HOLD)) {
      if (mind)
        mind->SendF(
            "You can't put %s into itself.\n", body->ActTarg(act_t::HOLD)->Name(0, body).c_str());
    } else if (
        (!nmode) && body->ActTarg(act_t::HOLD)->SubHasSkill(crc32c("Cursed")) &&
        targ->Owner() != body) {
      if (mind)
        mind->SendF(
            "You can't seem to part with %s.\n", body->ActTarg(act_t::HOLD)->Name(0, body).c_str());
    } else {
      int closed = 0, res = 0;
      Object* obj = body->ActTarg(act_t::HOLD);
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
        if (!targ->Skill(crc32c("Open")))
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

  if (cnum == COM_UNWIELD) {
    cnum = COM_WIELD;
    args = "";
  }

  if (cnum == COM_WIELD) {
    if (args.empty()) {
      if (body->IsAct(act_t::WIELD)) {
        Object* wield = body->ActTarg(act_t::WIELD);
        if ((!nmode) && wield && wield->SubHasSkill(crc32c("Cursed"))) {
          if (mind)
            mind->SendF("You can't seem to stop wielding %s!\n", wield->Name(0, body).c_str());
        } else if (wield && body->Stash(wield, 0)) {
          body->Parent()->SendOut(
              stealth_t,
              stealth_s,
              ";s stops wielding and stashes ;s.\n",
              "You stop wielding and stash ;s.\n",
              body,
              wield);
        } else if (
            body->IsAct(act_t::HOLD) &&
            body->ActTarg(act_t::HOLD) != body->ActTarg(act_t::WEAR_SHIELD) &&
            body->ActTarg(act_t::HOLD) != wield) {
          if (mind)
            mind->SendF(
                "You are holding %s and can't stash %s.\n"
                "Perhaps you want to 'drop' one of these items?",
                body->ActTarg(act_t::HOLD)->Name(1, body).c_str(),
                wield->Name(1, body).c_str());
        } else {
          body->StopAct(act_t::WIELD);
          body->Parent()->SendOut(
              stealth_t,
              stealth_s,
              ";s stops wielding ;s.\n",
              "You stop wielding ;s.\n",
              body,
              wield);
          if (!body->Stash(wield))
            body->AddAct(act_t::HOLD, wield);
        }
      } else {
        if (mind)
          mind->Send("You are not wielding anything.\n");
      }
      return 0;
    }

    Object* targ = body->PickObject(std::string(args), vmode | LOC_INTERNAL);
    if (!targ) {
      if (mind)
        mind->Send("You want to wield what?\n");
    } else if (targ->Skill(crc32c("WeaponType")) <= 0) {
      if (mind)
        mind->Send("You can't wield that - it's not a weapon!\n");
    } else if (
        body->ActTarg(act_t::WEAR_BACK) == targ || body->ActTarg(act_t::WEAR_CHEST) == targ ||
        body->ActTarg(act_t::WEAR_HEAD) == targ || body->ActTarg(act_t::WEAR_NECK) == targ ||
        body->ActTarg(act_t::WEAR_COLLAR) == targ || body->ActTarg(act_t::WEAR_WAIST) == targ ||
        body->ActTarg(act_t::WEAR_SHIELD) == targ || body->ActTarg(act_t::WEAR_LARM) == targ ||
        body->ActTarg(act_t::WEAR_RARM) == targ || body->ActTarg(act_t::WEAR_LFINGER) == targ ||
        body->ActTarg(act_t::WEAR_RFINGER) == targ || body->ActTarg(act_t::WEAR_LFOOT) == targ ||
        body->ActTarg(act_t::WEAR_RFOOT) == targ || body->ActTarg(act_t::WEAR_LHAND) == targ ||
        body->ActTarg(act_t::WEAR_RHAND) == targ || body->ActTarg(act_t::WEAR_LLEG) == targ ||
        body->ActTarg(act_t::WEAR_RLEG) == targ || body->ActTarg(act_t::WEAR_LWRIST) == targ ||
        body->ActTarg(act_t::WEAR_RWRIST) == targ || body->ActTarg(act_t::WEAR_LSHOULDER) == targ ||
        body->ActTarg(act_t::WEAR_RSHOULDER) == targ || body->ActTarg(act_t::WEAR_LHIP) == targ ||
        body->ActTarg(act_t::WEAR_RHIP) == targ || body->ActTarg(act_t::WEAR_FACE) == targ) {
      if (mind)
        mind->Send("You are wearing that, perhaps you want to 'remove' it?\n");
    } else {
      if (body->IsAct(act_t::WIELD) && body->IsAct(act_t::HOLD)) {
        if (body->ActTarg(act_t::HOLD) != targ) {
          if (mind)
            mind->Send(
                "You are both holding and wielding other things.\n"
                "Perhaps you want to drop one of them?\n");
          return 0;
        }
      }

      // Auto-unwield (trying to wield something else)
      Object* wield = body->ActTarg(act_t::WIELD);
      if ((!nmode) && wield && wield->SubHasSkill(crc32c("Cursed"))) {
        if (mind)
          mind->SendF("You can't seem to stop wielding %s!\n", wield->Name(0, body).c_str());
        return 0;
      }
      targ->Travel(body, 0); // Kills Holds and Wields on "targ"
      if (wield) {
        body->StopAct(act_t::WIELD);
        body->Parent()->SendOut(
            stealth_t,
            stealth_s,
            ";s stops wielding ;s.\n",
            "You stop wielding ;s.\n",
            body,
            wield);
        if (!body->Stash(wield)) { // Try to stash first
          body->AddAct(act_t::HOLD, wield); // If not, just hold it
        }
      }

      auto trigs = targ->PickObjects("all tbamud trigger script", LOC_NINJA | LOC_INTERNAL);
      for (auto trig : trigs) {
        int ttype = trig->Skill(crc32c("TBAScriptType"));
        if ((ttype & 0x2000200) == 0x2000200) { // OBJ-WEAR trigs
          if (new_trigger(0, trig, body))
            return 0; // Says FAIL!
        }
      }

      body->AddAct(act_t::WIELD, targ);
      body->Parent()->SendOut(
          stealth_t, stealth_s, ";s wields ;s.\n", "You wield ;s.\n", body, targ);
    }
    return 0;
  }

  if (cnum == COM_HOLD || cnum == COM_LIGHT) {
    if (args.empty()) {
      if (mind)
        mind->Send("What do you want to hold?\n");
      return 0;
    }

    Object* targ = body->PickObject(std::string(args), vmode | LOC_INTERNAL);
    if (!targ) {
      if (mind)
        mind->Send("You want to hold what?\n");
    }
    // FIXME - Implement Str-based Holding Capacity
    //    else if(targ->Skill(crc32c("WeaponType")) <= 0) {
    //      if(mind) mind->Send("You can't hold that - you are too weak!\n");
    //      }
    else if (body->ActTarg(act_t::HOLD) == targ) {
      if (mind)
        mind->SendF("You are already holding %s!\n", targ->Name(1, body).c_str());
    } else if (
        body->IsAct(act_t::HOLD) && body->ActTarg(act_t::HOLD) != body->ActTarg(act_t::WIELD) &&
        body->ActTarg(act_t::HOLD) != body->ActTarg(act_t::WEAR_SHIELD)) {
      if (mind)
        mind->Send("You are already holding something!\n");
    } else if (
        body->ActTarg(act_t::WIELD) == targ &&
        two_handed(body->ActTarg(act_t::WIELD)->Skill(crc32c("WeaponType")))) {
      body->AddAct(act_t::HOLD, targ);
      body->Parent()->SendOut(
          stealth_t,
          stealth_s,
          ";s puts a second hand on ;s.\n",
          "You put a second hand on ;s.\n",
          body,
          targ);
    } else if (body->ActTarg(act_t::WEAR_SHIELD) == targ) {
      body->AddAct(act_t::HOLD, targ);
      body->Parent()->SendOut(stealth_t, stealth_s, ";s holds ;s.\n", "You hold ;s.\n", body, targ);
    } else if (body->Wearing(targ) && targ->SubHasSkill(crc32c("Cursed"))) {
      if (mind) {
        if (body->ActTarg(act_t::WIELD) == targ) {
          mind->SendF("You can't seem to stop wielding %s!\n", targ->Name(0, body).c_str());
        } else {
          mind->SendF("You can't seem to remove %s.\n", targ->Name(0, body).c_str());
        }
      }
    } else {
      if (body->IsAct(act_t::HOLD)) { // Means it's a shield/2-h weapon due to above.
        body->Parent()->SendOut(
            stealth_t,
            stealth_s,
            ";s stops holding ;s.\n",
            "You stop holding ;s.\n",
            body,
            body->ActTarg(act_t::HOLD));
        body->StopAct(act_t::HOLD);
      }
      targ->Travel(body, 0); // Kills Holds, Wears and Wields on "targ"
      body->AddAct(act_t::HOLD, targ);
      if (cnum == COM_LIGHT) {
        if (targ->HasSkill(crc32c("Lightable"))) {
          body->Parent()->SendOut(
              stealth_t,
              stealth_s,
              ";s holds and lights ;s.\n",
              "You hold and light ;s.\n",
              body,
              targ);
          targ->SetSkill(crc32c("Lightable"), targ->Skill(crc32c("Lightable")) - 1);
          targ->SetSkill(crc32c("Light Source"), targ->Skill(crc32c("Brightness")));
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

  if (cnum == COM_REMOVE) {
    if (args.empty()) {
      if (mind)
        mind->Send("You want to remove what?\n");
      return 0;
    }

    auto targs = body->PickObjects(std::string(args), vmode | LOC_INTERNAL);
    if (targs.size() == 0) {
      if (mind)
        mind->Send("You want to remove what?\n");
      return 0;
    }

    for (auto targ : targs) {
      auto trigs = targ->PickObjects("all tbamud trigger script", LOC_NINJA | LOC_INTERNAL);
      for (auto trig : trigs) {
        int ttype = trig->Skill(crc32c("TBAScriptType"));
        if ((ttype & 0x2000800) == 0x2000800) { // OBJ-REMOVE trigs
          if (new_trigger(0, trig, body))
            return 0; // Says FAIL!
        }
      }

      int removed = 0;
      if ((!nmode) && targ->HasSkill(crc32c("Cursed"))) {
        if (mind)
          mind->SendF("%s won't come off!\n", targ->Name(0, body).c_str());
        return 0;
      }
      for (act_t act = act_t::WEAR_BACK; act < act_t::MAX; act = act_t(int(act) + 1)) {
        if (body->ActTarg(act) == targ) {
          removed = 1;
          break;
        }
      }
      if (!removed) {
        if (mind)
          mind->SendF("You are not wearing %s!\n", targ->Name(0, body).c_str());
      } else if (body->Stash(targ, 0, 0, 0)) {
        body->Parent()->SendOut(
            stealth_t,
            stealth_s,
            ";s removes and stashes ;s.\n",
            "You remove and stash ;s.\n",
            body,
            targ);
      } else if (
          body->IsAct(act_t::HOLD) &&
          body->ActTarg(act_t::HOLD) != body->ActTarg(act_t::WEAR_SHIELD) &&
          body->ActTarg(act_t::HOLD) != body->ActTarg(act_t::WIELD)) {
        if (mind)
          mind->SendF(
              "You are already holding something else and can't stash %s.\n",
              targ->Name(0, body).c_str());
      } else {
        if (body->IsAct(act_t::HOLD)) {
          body->Parent()->SendOut(
              stealth_t,
              stealth_s,
              ";s stops holding ;s.\n",
              "You stop holding ;s.\n",
              body,
              body->ActTarg(act_t::HOLD));
          body->StopAct(act_t::HOLD);
        }
        targ->Travel(body, 0);
        body->AddAct(act_t::HOLD, targ);
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

  if (cnum == COM_WEAR) {
    MinVec<1, Object*> targs;

    if (args.empty()) {
      if (body->ActTarg(act_t::HOLD)) {
        targs.push_back(body->ActTarg(act_t::HOLD));
      } else {
        if (mind) {
          mind->Send("What do you want to wear?  ");
          mind->Send("Use 'wear <item>' or hold the item first.\n");
        }
        return 0;
      }
    }

    if (targs.size() < 1) {
      targs = body->PickObjects(std::string(args), vmode | LOC_NOTWORN | LOC_INTERNAL);
      if (!targs.size()) {
        if (mind)
          mind->Send("You want to wear what?\n");
        return 0;
      }
    }

    int did_something = 0;
    for (auto targ : targs) {
      // fprintf(stderr, "You try to wear %s!\n", targ->Name(0, body).c_str());
      // if(mind) mind->Send("You try to wear %s!\n", targ->Name(0, body).c_str());
      if (body->ActTarg(act_t::WEAR_BACK) == targ || body->ActTarg(act_t::WEAR_CHEST) == targ ||
          body->ActTarg(act_t::WEAR_HEAD) == targ || body->ActTarg(act_t::WEAR_NECK) == targ ||
          body->ActTarg(act_t::WEAR_COLLAR) == targ || body->ActTarg(act_t::WEAR_WAIST) == targ ||
          body->ActTarg(act_t::WEAR_SHIELD) == targ || body->ActTarg(act_t::WEAR_LARM) == targ ||
          body->ActTarg(act_t::WEAR_RARM) == targ || body->ActTarg(act_t::WEAR_LFINGER) == targ ||
          body->ActTarg(act_t::WEAR_RFINGER) == targ || body->ActTarg(act_t::WEAR_LFOOT) == targ ||
          body->ActTarg(act_t::WEAR_RFOOT) == targ || body->ActTarg(act_t::WEAR_LHAND) == targ ||
          body->ActTarg(act_t::WEAR_RHAND) == targ || body->ActTarg(act_t::WEAR_LLEG) == targ ||
          body->ActTarg(act_t::WEAR_RLEG) == targ || body->ActTarg(act_t::WEAR_LWRIST) == targ ||
          body->ActTarg(act_t::WEAR_RWRIST) == targ ||
          body->ActTarg(act_t::WEAR_LSHOULDER) == targ ||
          body->ActTarg(act_t::WEAR_RSHOULDER) == targ || body->ActTarg(act_t::WEAR_LHIP) == targ ||
          body->ActTarg(act_t::WEAR_RHIP) == targ || body->ActTarg(act_t::WEAR_FACE) == targ) {
        if (mind && targs.size() == 1)
          mind->SendF("You are already wearing %s!\n", targ->Name(0, body).c_str());
      } else {
        auto trigs = targ->PickObjects("all tbamud trigger script", LOC_NINJA | LOC_INTERNAL);
        for (auto trig : trigs) {
          int ttype = trig->Skill(crc32c("TBAScriptType"));
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

  if (cnum == COM_EAT) {
    if (args.empty()) {
      if (mind)
        mind->Send("What do you want to eat?\n");
      return 0;
    }
    if (!body->HasSkill(crc32c("Hungry"))) {
      if (mind)
        mind->Send("You are not hungry, you can't eat any more.\n");
      return 0;
    }
    auto targs = body->PickObjects(std::string(args), vmode | LOC_NOTWORN | LOC_INTERNAL);
    if (body->ActTarg(act_t::HOLD) && body->ActTarg(act_t::HOLD)->Parent() != body // Dragging
        && body->ActTarg(act_t::HOLD)->Matches(std::string(args))) {
      targs.push_back(body->ActTarg(act_t::HOLD));
    }
    if (!targs.size()) {
      if (mind)
        mind->Send("You want to eat what?\n");
    } else {
      for (auto targ : targs) {
        if (!(targ->HasSkill(crc32c("Ingestible")))) {
          if (mind)
            mind->SendF("You don't want to eat %s.\n", targ->Name(0, body).c_str());
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

  if (cnum == COM_STASH) {
    Object* targ = body->ActTarg(act_t::HOLD);
    if (targ && targ->Parent() == body) {
      if (!body->Stash(targ)) {
        if (mind)
          mind->SendF("You have no place to stash %s.\n", targ->Name(0, body).c_str());
      }
    } else {
      if (mind)
        mind->Send("You are not holding anything to stash.\n");
    }
    return 0;
  }

  if (cnum == COM_DROP) {
    if (args.empty()) {
      if (mind)
        mind->Send("What do you want to drop?\n");
      return 0;
    }
    auto targs = body->PickObjects(std::string(args), vmode | LOC_NOTWORN | LOC_INTERNAL);
    if (body->ActTarg(act_t::HOLD) && body->ActTarg(act_t::HOLD)->Parent() != body // Dragging
        && body->ActTarg(act_t::HOLD)->Matches(std::string(args))) {
      targs.push_back(body->ActTarg(act_t::HOLD));
    }
    if (!targs.size()) {
      if (mind)
        mind->Send("You want to drop what?\n");
    } else {
      for (auto targ : targs) {
        auto trigs = targ->PickObjects("all tbamud trigger script", LOC_NINJA | LOC_INTERNAL);
        for (auto trig : trigs) {
          int ttype = trig->Skill(crc32c("TBAScriptType"));
          if ((ttype & 0x2000080) == 0x2000080) { // OBJ-DROP trigs
            if ((rand() % 100) < trig->Skill(crc32c("TBAScriptNArg"))) { // % Chance
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
          trigs = room->PickObjects("all tbamud trigger script", LOC_NINJA | LOC_INTERNAL);
        for (auto trig : trigs) {
          int ttype = trig->Skill(crc32c("TBAScriptType"));
          if ((ttype & 0x4000080) == 0x4000080) { // ROOM-DROP trigs
            if ((rand() % 100) < trig->Skill(crc32c("TBAScriptNArg"))) { // % Chance
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
            mind->SendF("You can't drop %s here.\n", targ->Name(0, body).c_str());
        } else if (ret == -2) { // Exceeds Capacity
          if (mind)
            mind->SendF("You can't drop %s, there isn't room.\n", targ->Name(0, body).c_str());
        } else if (ret == -3) { // Exceeds Weight Limit
          if (mind)
            mind->SendF("You can't drop %s, it's too heavy.\n", targ->Name(0, body).c_str());
        } else if (ret == -4) { // Cursed
          if (mind)
            mind->SendF("You don't seem to be able to drop %s!\n", targ->Name(0, body).c_str());
        } else if (ret != 0) { //?
          if (mind)
            mind->SendF("You can't seem to drop %s!\n", targ->Name(0, body).c_str());
        }
      }
    }
    return 0;
  }

  if (cnum == COM_DRINK) {
    if (args.empty()) {
      if (mind)
        mind->Send("What do you want to drink from?\n");
      return 0;
    }
    if (!body->HasSkill(crc32c("Thirsty"))) {
      if (mind)
        mind->Send("You are not thirsty, you can't drink any more.\n");
      return 0;
    }
    Object* targ = body->PickObject(std::string(args), vmode | LOC_NOTWORN | LOC_INTERNAL);
    if ((!targ) && body->ActTarg(act_t::HOLD) &&
        body->ActTarg(act_t::HOLD)->Parent() != body // Dragging
        && body->ActTarg(act_t::HOLD)->Matches(std::string(args))) {
      targ = body->ActTarg(act_t::HOLD);
    }
    if (!targ) {
      targ = body->PickObject(std::string(args), vmode | LOC_NEARBY);
    }
    if (!targ) {
      if (mind)
        mind->Send("You want to drink from what?\n");
    } else {
      std::string denied = "";
      for (Object* own = targ; own; own = own->Parent()) {
        if (own->IsAnimate() && own != body && (!own->IsAct(act_t::SLEEP)) &&
            (!own->IsAct(act_t::DEAD)) && (!own->IsAct(act_t::DYING)) &&
            (!own->IsAct(act_t::UNCONSCIOUS))) {
          denied = "You would need ";
          denied += own->Name(1);
          denied += "'s permission to drink from ";
          denied += targ->Name(0, nullptr, own);
          denied += ".\n";
        } else if (
            own->Skill(crc32c("Container")) && (!own->Skill(crc32c("Open"))) &&
            own->Skill(crc32c("Locked"))) {
          denied = own->Name(1);
          if (own == targ) {
            denied += " is closed and locked so you can't drink from it.\n";
          } else {
            denied += " is closed and locked so you can't get to ";
            denied += targ->Name(1);
            denied += ".\n";
          }
          denied[0] = ascii_toupper(denied[0]);
        }
      }
      if ((!nmode) && (!denied.empty())) {
        if (mind)
          mind->Send(denied.c_str());
        return 0;
      }
      if (!(targ->HasSkill(crc32c("Liquid Container")))) {
        if (mind)
          mind->SendF(
              "%s is not a liquid container.  You can't drink from it.\n",
              targ->Name(0, body).c_str());
        return 0;
      }
      if (targ->Contents(vmode).size() < 1) {
        if (mind)
          mind->SendF("%s is empty.  There is nothing to drink\n", targ->Name(0, body).c_str());
        return 0;
      }
      Object* obj = targ->Contents(vmode).front();
      if (targ->HasSkill(crc32c("Liquid Source")) && obj->Skill(crc32c("Quantity")) < 2) {
        if (mind)
          mind->SendF(
              "%s is almost empty.  There is nothing to drink\n", targ->Name(0, body).c_str());
        return 0;
      }
      if ((!(obj->HasSkill(crc32c("Ingestible"))))) {
        if (mind)
          mind->SendF("You don't want to drink what's in %s.\n", targ->Name(0, body).c_str());
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

      if (obj->Skill(crc32c("Quantity")) < 2) {
        obj->Recycle();
        if (targ->HasSkill(crc32c("Perishable"))) { // One-Use Vials
          targ->Recycle();
        }
      } else {
        obj->SetSkill(crc32c("Quantity"), obj->Skill(crc32c("Quantity")) - 1);
      }
    }
    return 0;
  }

  if (cnum == COM_DUMP) {
    if (args.empty()) {
      if (mind)
        mind->Send("What do you want to dump?\n");
      return 0;
    }
    auto targs = body->PickObjects(std::string(args), vmode | LOC_NOTWORN | LOC_INTERNAL);
    if (body->ActTarg(act_t::HOLD) && body->ActTarg(act_t::HOLD)->Parent() != body // Dragging
        && body->ActTarg(act_t::HOLD)->Matches(std::string(args))) {
      targs.push_back(body->ActTarg(act_t::HOLD));
    }
    if (!targs.size()) {
      if (mind)
        mind->Send("You want to dump what?\n");
    } else {
      for (auto targ : targs) {
        if (!(targ->HasSkill(crc32c("Liquid Container")))) {
          if (mind)
            mind->SendF(
                "%s is not a liquid container.  It can't be dumped.\n",
                targ->Name(0, body).c_str());
          continue;
        }
        if (targ->Contents(LOC_TOUCH).size() < 1) {
          if (mind)
            mind->SendF("%s is empty.  There is nothing to dump\n", targ->Name(0, body).c_str());
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

  if (cnum == COM_FILL) {
    if (args.substr(0, 5) == "from ") {
      auto prefix = args.find_first_not_of(" \t\n\r", 5);
      if (prefix == std::string::npos) {
        args = "";
      } else {
        args = args.substr(prefix);
      }
    }

    if (!body->IsAct(act_t::HOLD)) {
      if (mind)
        mind->Send("You must first 'hold' the object you want to 'fill'.\n");
      return 0;
    } else if (!body->ActTarg(act_t::HOLD)) {
      if (mind)
        mind->Send("What!?!?!  You are holding nothing?\n");
      return 0;
    }

    if (args.empty()) {
      if (mind)
        mind->SendF(
            "Where do you want to fill %s from?\n",
            body->ActTarg(act_t::HOLD)->Name(0, body).c_str());
      return 0;
    }

    Object* src = body->PickObject(std::string(args), vmode | LOC_NEARBY | LOC_INTERNAL);
    Object* dst = body->ActTarg(act_t::HOLD);
    if (!src) {
      if (mind)
        mind->SendF(
            "I don't see '%s' to fill %s from!\n",
            std::string(args).c_str(),
            dst->Name(0, body).c_str());
    } else if (!dst->HasSkill(crc32c("Liquid Container"))) {
      if (mind)
        mind->SendF(
            "You can not fill %s, it is not a liquid container.\n", dst->Name(0, body).c_str());
    } else if (src->IsAnimate()) {
      if (mind)
        mind->Send("You can only fill things from inanimate objects!\n");
    } else if (!src->HasSkill(crc32c("Liquid Container"))) {
      if (mind)
        mind->Send(
            "You can't fill anything from that, it's not a liquid "
            "container.\n");
    } else if (dst->Skill(crc32c("Locked"))) {
      if (mind)
        mind->SendF("You can't fill %s, it is locked.\n", dst->Name(0, body).c_str());
    } else if (src->Skill(crc32c("Locked"))) {
      if (mind)
        mind->Send("You can't fill anything from that, it is locked.\n");
    } else if (src == dst) {
      if (mind)
        mind->SendF("You can't fill %s from itself.\n", dst->Name(0, body).c_str());
    } else if (src->Contents(vmode).size() < 1) {
      if (mind)
        mind->Send("You can't fill anything from that, it is empty.\n");
    } else if (
        src->HasSkill(crc32c("Liquid Source")) &&
        src->Contents(vmode).front()->Skill(crc32c("Quantity")) < 2) {
      if (mind)
        mind->Send("You can't fill anything from that, it is almost empty.\n");
    } else {
      int myclosed = 0, itclosed = 0;

      int sqty = 1;
      int dqty = dst->Skill(crc32c("Capacity"));
      if (src->Contents(vmode).front()->Skill(crc32c("Quantity")) > 0) {
        sqty = src->Contents(vmode).front()->Skill(crc32c("Quantity"));
      }

      if (src->HasSkill(crc32c("Liquid Source"))) {
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
      liq->SetSkill(crc32c("Quantity"), dqty);
      if (sqty > 0) {
        src->Contents(vmode).front()->SetSkill(crc32c("Quantity"), sqty);
      } else {
        src->Contents(vmode).front()->Recycle();
      }

      if (!src->Skill(crc32c("Open")))
        itclosed = 1;
      if (!dst->Skill(crc32c("Open")))
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

  if (cnum == COM_LEAVE) {
    Object* oldp = body->Parent();
    if (!body->Parent()->Parent()) {
      if (mind)
        mind->Send("It is not possible to leave this object!\n");
    } else if ((!body->Parent()->Skill(crc32c("Enterable"))) && (!ninja)) {
      if (mind)
        mind->Send("It is not possible to leave this object!\n");
    } else if ((!body->Parent()->Skill(crc32c("Enterable"))) && (!nmode)) {
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

  if (cnum == COM_SLEEP) {
    if (body->IsAct(act_t::SLEEP)) {
      if (mind)
        mind->Send("You are already sleeping!\n");
      return 0;
    }
    int lied = 0;
    if (body->Pos() != pos_t::LIE) {
      body->SetPos(pos_t::LIE);
      lied = 1;
    }
    if (body->ActTarg(act_t::WIELD)) {
      Object* item = body->ActTarg(act_t::WIELD);
      body->StashOrDrop(item);
    }
    if (body->ActTarg(act_t::HOLD) // Shield held & worn
        && body->ActTarg(act_t::HOLD) == body->ActTarg(act_t::WEAR_SHIELD)) {
      body->Parent()->SendOut(
          stealth_t,
          stealth_s,
          ";s stops holding ;s.\n",
          "You stop holding ;s.\n",
          body,
          body->ActTarg(act_t::HOLD));
      body->StopAct(act_t::HOLD);
    } else if (
        body->ActTarg(act_t::HOLD) // Dragging an item
        && body->ActTarg(act_t::HOLD)->Parent() != body) {
      body->Drop(body->ActTarg(act_t::HOLD));
    } else if (body->ActTarg(act_t::HOLD)) { // Regular held item
      Object* item = body->ActTarg(act_t::HOLD);
      body->StashOrDrop(item);
    }
    body->Collapse();
    body->AddAct(act_t::SLEEP);
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

  if (cnum == COM_WAKE) {
    if (!body->IsAct(act_t::SLEEP)) {
      if (mind)
        mind->Send("But you aren't asleep!\n");
    } else {
      body->StopAct(act_t::SLEEP);
      body->Parent()->SendOut(
          stealth_t, stealth_s, ";s wakes up.\n", "You wake up.\n", body, nullptr);
    }
    return 0;
  }

  if (cnum == COM_REST) {
    if (body->IsAct(act_t::REST)) {
      body->StopAct(act_t::REST);
      body->Parent()->SendOut(
          stealth_t, stealth_s, ";s stops resting.\n", "You stop resting.\n", body, nullptr);
      return 0;
    } else if (body->IsAct(act_t::SLEEP)) {
      body->StopAct(act_t::REST);
      body->AddAct(act_t::REST);
      body->Parent()->SendOut(
          stealth_t,
          stealth_s,
          ";s wakes up and starts resting.\n",
          "You wake up and start resting.\n",
          body,
          nullptr);
    } else if (body->Pos() == pos_t::LIE || body->Pos() == pos_t::SIT) {
      body->AddAct(act_t::REST);
      body->Parent()->SendOut(
          stealth_t, stealth_s, ";s starts resting.\n", "You start resting.\n", body, nullptr);
    } else {
      body->AddAct(act_t::REST);
      if (body->Pos() != pos_t::LIE)
        body->SetPos(pos_t::SIT);
      body->Parent()->SendOut(
          stealth_t,
          stealth_s,
          ";s sits down and rests.\n",
          "You sit down and rest.\n",
          body,
          nullptr);
    }
    if (body->IsAct(act_t::FOLLOW)) {
      if (body->ActTarg(act_t::FOLLOW) && mind)
        body->Parent()->SendOut(
            stealth_t,
            stealth_s,
            ";s stop following ;s.\n",
            "You stop following ;s.\n",
            body,
            body->ActTarg(act_t::FOLLOW));
      body->StopAct(act_t::FOLLOW);
    }
    return 0;
  }

  if (cnum == COM_STAND) {
    if (body->Pos() == pos_t::STAND || body->Pos() == pos_t::USE) {
      if (mind)
        mind->Send("But you are already standing!\n");
    } else if (body->IsAct(act_t::SLEEP)) {
      body->SetPos(pos_t::STAND);
      body->StopAct(act_t::SLEEP);
      body->Parent()->SendOut(
          stealth_t,
          stealth_s,
          ";s wakes up and stands.\n",
          "You wake up and stand.\n",
          body,
          nullptr);
    } else if (body->IsAct(act_t::REST)) {
      body->StopAct(act_t::REST);
      body->SetPos(pos_t::STAND);
      body->Parent()->SendOut(
          stealth_t,
          stealth_s,
          ";s stops resting and stands up.\n",
          "You stop resting and stand up.\n",
          body,
          nullptr);
    } else {
      body->SetPos(pos_t::STAND);
      body->Parent()->SendOut(
          stealth_t, stealth_s, ";s stands up.\n", "You stand up.\n", body, nullptr);
    }
    return 0;
  }

  if (cnum == COM_SIT) {
    if (body->Pos() == pos_t::SIT) {
      if (mind)
        mind->Send("But you are already sitting!\n");
    } else if (body->IsAct(act_t::SLEEP)) {
      body->StopAct(act_t::SLEEP);
      body->SetPos(pos_t::SIT);
      body->Parent()->SendOut(
          stealth_t,
          stealth_s,
          ";s awaken and sit up.\n",
          "You awaken and sit up.\n",
          body,
          nullptr);
    } else if (body->Pos() == pos_t::LIE) {
      body->SetPos(pos_t::SIT);
      body->Parent()->SendOut(
          stealth_t, stealth_s, ";s sits up.\n", "You sit up.\n", body, nullptr);
    } else {
      body->SetPos(pos_t::SIT);
      body->Parent()->SendOut(
          stealth_t, stealth_s, ";s sits down.\n", "You sit down.\n", body, nullptr);
    }
    if (body->IsAct(act_t::FOLLOW)) {
      if (body->ActTarg(act_t::FOLLOW) && mind)
        body->Parent()->SendOut(
            stealth_t,
            stealth_s,
            ";s stop following ;s.\n",
            "You stop following ;s.\n",
            body,
            body->ActTarg(act_t::FOLLOW));
      body->StopAct(act_t::FOLLOW);
    }
    return 0;
  }

  if (cnum == COM_LIE) {
    if (body->Pos() == pos_t::LIE) {
      if (mind)
        mind->Send("But you are already lying down!\n");
    } else {
      body->SetPos(pos_t::LIE);
      body->Parent()->SendOut(
          stealth_t, stealth_s, ";s lies down.\n", "You lie down.\n", body, nullptr);
    }
    if (body->IsAct(act_t::FOLLOW)) {
      if (body->ActTarg(act_t::FOLLOW) && mind)
        body->Parent()->SendOut(
            stealth_t,
            stealth_s,
            ";s stop following ;s.\n",
            "You stop following ;s.\n",
            body,
            body->ActTarg(act_t::FOLLOW));
      body->StopAct(act_t::FOLLOW);
    }
    return 0;
  }

  if (cnum == COM_CAST) {
    if (args.empty()) {
      if (mind)
        mind->Send("What spell do you want to cast?\n");
      return 0;
    }

    int defself = 0;
    int special = 0;
    int freehand = 0;
    uint32_t spname = crc32c("None");
    if (args == std::string_view("identify").substr(0, args.length())) {
      special = 1;
      spname = crc32c("Identify");
    } else if (args == std::string_view("create food").substr(0, args.length())) {
      defself = -1;
      special = 2;
      freehand = 1;
      spname = crc32c("Create Food");
    } else if (args == std::string_view("force sword").substr(0, args.length())) {
      defself = -1;
      special = 2;
      freehand = 1;
      spname = crc32c("Force Sword");
    } else if (args == std::string_view("heat vision").substr(0, args.length())) {
      defself = 1;
      spname = crc32c("Heat Vision");
    } else if (args == std::string_view("dark vision").substr(0, args.length())) {
      defself = 1;
      spname = crc32c("Dark Vision");
    } else if (args == std::string_view("recall").substr(0, args.length())) {
      defself = 1;
      spname = crc32c("Recall");
    } else if (args == std::string_view("teleport").substr(0, args.length())) {
      defself = 1;
      spname = crc32c("Teleport");
    } else if (args == std::string_view("resurrect").substr(0, args.length())) {
      defself = 1;
      spname = crc32c("Resurrect");
    } else if (args == std::string_view("remove curse").substr(0, args.length())) {
      defself = 1;
      spname = crc32c("Remove Curse");
    } else if (args == std::string_view("cure poison").substr(0, args.length())) {
      defself = 1;
      spname = crc32c("Cure Poison");
    } else if (args == std::string_view("sleep other").substr(0, args.length())) {
      spname = crc32c("Sleep Other");
    } else {
      if (mind)
        mind->Send("Never heard of that spell.\n");
      return 0;
    }

    Object* src = nullptr;
    if (!nmode)
      src = body->NextHasSkill(crc32c(SkillName(spname) + " Spell"));
    if ((!nmode) && (!src)) {
      if (mind)
        mind->SendF(
            "You don't know the %s Spell and have no items enchanted with it.\n",
            SkillName(spname).c_str());
      return 0;
    }

    if ((!defself) && (!body->ActTarg(act_t::POINT))) {
      if (mind)
        mind->Send("That spell requires you to first point at your target.\n");
      return 0;
    }

    if (freehand) {
      if (body->ActTarg(act_t::HOLD)) {
        body->StashOrDrop(body->ActTarg(act_t::HOLD));
      }
    }

    Object* targ = body->ActTarg(act_t::POINT);
    if (!targ)
      targ = body; // Defaults to SELF if not, caught above!)
    if (src && src != body) {
      std::string youmes = "You use ;s to cast " + SkillName(spname) + ".\n";
      body->Parent()->SendOut(
          stealth_t, stealth_s, ";s uses ;s to cast a spell.\n", youmes.c_str(), body, src);
    }
    if (defself >= 0) { // Targeted
      std::string youmes = "You cast " + SkillName(spname) + " on ;s.\n";
      body->Parent()->SendOut(
          stealth_t, stealth_s, ";s casts a spell on ;s.\n", youmes.c_str(), body, targ);
    } else { // Not Targeted
      std::string youmes = "You cast " + SkillName(spname) + ".\n";
      body->Parent()->SendOut(
          stealth_t, stealth_s, ";s casts a spell.\n", youmes.c_str(), body, nullptr);
    }

    int force = 1000; // FIXME: Magic Force!
    if (src)
      force = src->Skill(crc32c(SkillName(spname) + " Spell"));
    if (!special) { // Effect Person/Creature Spells
      Object* spell = new Object();
      spell->SetSkill(SkillName(spname) + " Spell", force);
      targ->Consume(spell);
      delete (spell);
    } else if (special == 2) { // Temporary Object Creation Spells
      Object* obj = new Object(body);
      if (spname == crc32c("Create Food")) {
        obj->SetShortDesc("a piece of magical food");
        obj->SetSkill(crc32c("Food"), force * 100);
        obj->SetSkill(crc32c("Ingestible"), force);
      } else if (spname == crc32c("Force Sword")) {
        obj->SetShortDesc("a sword of force");
        obj->SetSkill(crc32c("WeaponType"), 13);
        obj->SetSkill(crc32c("WeaponReach"), 1);
        obj->SetSkill(crc32c("WeaponSeverity"), 2);
        obj->SetSkill(crc32c("WeaponForce"), std::min(100, force));
      }
      obj->SetWeight(1);
      obj->SetVolume(1);
      obj->SetSize(1);
      obj->SetSkill(crc32c("Magical"), force);
      obj->SetSkill(crc32c("Light Source"), 10);
      obj->SetSkill(crc32c("Temporary"), force);
      obj->Activate();
      obj->SetPos(pos_t::LIE);
      body->AddAct(act_t::HOLD, obj);
      body->Parent()->SendOutF(
          0,
          0,
          "%s appears in ;s's hand.\n",
          "%s appears in your hand.\n",
          body,
          nullptr,
          obj->Name().c_str());
    } else if (spname == crc32c("Identify")) { // Other kinds of spells
      if (mind) {
        mind->Send(CCYN);
        targ->SendFullSituation(mind, body);
        targ->SendActions(mind);
        mind->Send(CNRM);
        targ->SendScore(mind, body);
      }
    }

    if (src) {
      if (src->Skill(crc32c("Quantity")) > 1) {
        src->Split(src->Skill(crc32c("Quantity")) - 1); // Split off the rest
      }
      if (src->HasSkill(crc32c("Magical Charges"))) {
        if (src->Skill(crc32c("Magical Charges")) > 1) {
          src->SetSkill(crc32c("Magical Charges"), src->Skill(crc32c("Magical Charges")) - 1);
        } else {
          delete (src);
        }
      }
    }

    return 0;
  }

  if (cnum == COM_PRAY) {
    if (!mind)
      return 0;

    if (args.empty()) {
      mind->Send("What command do you want to pray for?\n");
      mind->Send("You need to include the command, like 'pray kill Joe'.\n");
    } else {
      // FIXME: Tell sub-command you're praying!
      body->Parent()->SendOut(stealth_t, stealth_s, ";s prays.\n", "You pray.\n", body, nullptr);
      handle_single_command(body, std::string(args), mind);
    }
    return 0;
  }

  if (cnum == COM_STOP) { // Alias "stop" to "use"
    cnum = COM_USE;
    args = "";
  }
  if (cnum == COM_USE) {
    if (args.empty()) {
      if (body->Pos() != pos_t::USE) {
        mind->Send("You're not using a skill.  Try 'use <skillname>' to start.\n");
      } else {
        body->Parent()->SendOutF(
            stealth_t,
            stealth_s,
            ";s stops %s.\n",
            "You stop %s.\n",
            body,
            nullptr,
            body->UsingString().c_str());
        body->SetPos(pos_t::STAND);
        return 2;
      }
      return 0;
    }

    int longterm = 0; // Long-running skills for results
    auto skill = get_skill(std::string(args));
    if (skill == crc32c("None")) {
      mind->Send("Don't know what skill you're trying to use.\n");
      return 0;
    }

    if (skill == crc32c("Lumberjack")) {
      if (!body->HasSkill(skill)) {
        mind->SendF("%sYou don't know how to do that.%s\n", CYEL, CNRM);
        return 0;
      }
      if (!body->Parent()) { // You're nowhere?!?
        mind->SendF("%sThere are no trees here.%s\n", CYEL, CNRM);
        return 0;
      }
      if (strcasestr(body->Parent()->Name().c_str(), "forest") &&
          body->Parent()->HasSkill(crc32c("TBAZone")) &&
          (!body->Parent()->HasSkill(crc32c("Mature Trees")))) {
        body->Parent()->SetSkill(crc32c("Mature Trees"), 100);
        body->Parent()->Activate();
      }
      if (!body->Parent()->HasSkill(crc32c("Mature Trees"))) {
        mind->SendF("%sThere are no trees here.%s\n", CYEL, CNRM);
        return 0;
      } else if (body->Parent()->Skill(crc32c("Mature Trees")) < 10) {
        mind->SendF("%sThere are too few trees to harvest here.%s\n", CYEL, CNRM);
        return 0;
      } else {
        longterm = 3000; // FIXME: Temporary - should take longer!
      }
      if (body->IsUsing(crc32c("Lumberjack"))) { // Already been doing it
        if (body->Roll(skill, 10) > 0) { // Succeeded!
          body->Parent()->SendOut(
              ALL, 0, ";s shouts 'TIMBER'!!!\n", "You shout 'TIMBER'!!!\n", body, body);
          body->Parent()->Loud(body->Skill(crc32c("Strength")), "someone shout 'TIMBER'!!!");
          body->Parent()->SetSkill(
              crc32c("Mature Trees"), body->Parent()->Skill(crc32c("Mature Trees")) - 1);
          body->SetSkill(crc32c("Hidden"), 0);

          Object* log = new Object(body->Parent());
          log->SetShortDesc("a log");
          log->SetDesc("a fallen tree.");
          log->SetLongDesc("This is a tree that has recently been cut down.");
          log->SetPos(pos_t::LIE);
          log->SetValue(10);
          log->SetVolume(1000);
          log->SetWeight(220000);
          log->SetSize(8000);
          log->SetSkill(crc32c("Made of Wood"), 200000);
        }
        body->Parent()->SendOut(
            ALL,
            0,
            ";s continues chopping down trees.\n",
            "You continue chopping down trees.\n",
            body,
            body);
        body->Parent()->Loud(body->Skill(crc32c("Strength")) / 2, "loud chopping sounds.");
        body->SetSkill(crc32c("Hidden"), 0);
      }
    }

    if (!body->IsUsing(skill)) { // Only if really STARTING to use skill.
      body->StartUsing(skill);

      // In case Stealth was started, re-calc (to hide going into stealth).
      stealth_t = 0;
      stealth_s = 0;
      if (body->IsUsing(crc32c("Stealth")) && body->Skill(crc32c("Stealth")) > 0) {
        stealth_t = body->Skill(crc32c("Stealth"));
        stealth_s = body->Roll(crc32c("Stealth"), 2);
      }

      if (body->Pos() != pos_t::STAND && body->Pos() != pos_t::USE) { // FIXME: Unused
        body->Parent()->SendOutF(
            stealth_t,
            stealth_s,
            ";s stands and starts %s.\n",
            "You stand up and start %s.\n",
            body,
            nullptr,
            body->UsingString().c_str());
      } else {
        body->Parent()->SendOutF(
            stealth_t,
            stealth_s,
            ";s starts %s.\n",
            "You start %s.\n",
            body,
            nullptr,
            body->UsingString().c_str());
      }
      if (!body->HasSkill(skill)) {
        mind->SendF(
            "%s...you don't have the '%s' skill, so you're bad at this.%s\n",
            CYEL,
            SkillName(skill).c_str(),
            CNRM);
      }
    } else if (longterm == 0) {
      mind->SendF("You are already using %s\n", SkillName(skill).c_str());
    }

    if (longterm > 0) { // Long-running skills for results
      body->BusyFor(longterm, "use Lumberjack"); // FIXME: Temporary!
      return 2; // Full-round (and more) action!
    }
    return 0;
  }

  if (cnum == COM_POINT) {
    if (!args.empty()) {
      Object* targ =
          body->PickObject(std::string(args), vmode | LOC_NEARBY | LOC_SELF | LOC_INTERNAL);
      if (!targ) {
        if (mind)
          mind->Send("You don't see that here.\n");
      } else {
        body->AddAct(act_t::POINT, targ);
        body->Parent()->SendOut(
            stealth_t,
            stealth_s,
            ";s starts pointing at ;s.\n",
            "You start pointing at ;s.\n",
            body,
            targ);
      }
    } else if (body->IsAct(act_t::POINT)) {
      Object* targ = body->ActTarg(act_t::POINT);
      body->StopAct(act_t::POINT);
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

  if (cnum == COM_OFFER) {
    if (!args.empty()) {
      Object* targ = body->PickObject(std::string(args), vmode | LOC_NEARBY);
      if (!targ) {
        if (mind)
          mind->Send("You don't see that person here.\n");
      } else if (!body->ActTarg(act_t::HOLD)) {
        if (mind)
          mind->Send("You aren't holding anything to offer.\n");
      } else if (!targ->IsAnimate()) {
        if (mind)
          mind->Send("You can't offer anything to that.\n");
      } else {
        body->AddAct(act_t::OFFER, targ);
        body->Parent()->SendOut(
            stealth_t,
            stealth_s,
            ";s offers something to ;s.\n",
            "You offer something to ;s.\n",
            body,
            targ);

        auto trigs = targ->PickObjects("all tbamud trigger script", LOC_NINJA | LOC_INTERNAL);
        for (auto trig : trigs) {
          if ((trig->Skill(crc32c("TBAScriptType")) & 0x1000200) ==
              0x1000200) { // MOB-RECEIVE trigs
            if (!new_trigger(0, trig, body, body->ActTarg(act_t::HOLD))) {
              body->ActTarg(act_t::OFFER)->Travel(targ, 0);
              return 0; // Handled, unless script says not.
            }
          }
        }
      }
    } else if (body->IsAct(act_t::OFFER)) {
      Object* targ = body->ActTarg(act_t::OFFER);
      body->StopAct(act_t::OFFER);
      body->Parent()->SendOut(
          stealth_t,
          stealth_s,
          ";s stops offering something to ;s.\n",
          "You stop offering something to ;s.\n",
          body,
          targ);
    } else {
      if (mind)
        mind->Send("But, you aren't offering anything to anyone!\n");
    }
    return 0;
  }

  if (cnum == COM_FOLLOW) {
    if (!args.empty()) {
      Object* targ = body->PickObject(std::string(args), vmode | LOC_NEARBY | LOC_SELF);
      if (!targ) {
        if (mind)
          mind->Send("You don't see that here.\n");
      } else {
        body->AddAct(act_t::FOLLOW, targ);
        body->Parent()->SendOut(
            stealth_t,
            stealth_s,
            ";s starts following ;s.\n",
            "You start following ;s.\n",
            body,
            targ);
      }
    } else if (body->IsAct(act_t::FOLLOW)) {
      Object* targ = body->ActTarg(act_t::FOLLOW);
      body->StopAct(act_t::FOLLOW);
      body->Parent()->SendOut(
          stealth_t, stealth_s, ";s stops following ;s.\n", "You stop following ;s.\n", body, targ);
    } else {
      if (mind)
        mind->Send("But, you aren't following anyone!\n");
    }
    return 0;
  }

  if (cnum == COM_ATTACK || cnum == COM_KILL || cnum == COM_PUNCH || cnum == COM_KICK) {
    // fprintf(stderr, "Handling attack command from %p of '%s'\n", mind,
    // args.c_str());

    int attacknow = 1;
    if (!body->IsAct(act_t::FIGHT))
      attacknow = 0;

    Object* targ = nullptr;
    if (!args.empty()) {
      targ = body->PickObject(std::string(args), vmode | LOC_NEARBY | LOC_CONSCIOUS);
      if (!targ)
        targ = body->PickObject(std::string(args), vmode | LOC_NEARBY | LOC_ALIVE);
      if (!targ)
        targ = body->PickObject(std::string(args), vmode | LOC_NEARBY);
      if (!targ) {
        if (mind)
          mind->Send("You don't see that here.\n");
        return 0;
      }
    } else {
      if (body->IsAct(act_t::FIGHT)) {
        targ = body->ActTarg(act_t::FIGHT);
        if (!body->IsNearBy(targ)) {
          if (mind)
            mind->Send("Your target is gone!\n");
          body->StopAct(act_t::FIGHT);
          return 0;
        }
      } else {
        if (mind)
          mind->Send("Who did you want to hit?\n");
        return 0;
      }
    }

    if (cnum == COM_ATTACK &&
        (!targ->IsAnimate() || targ->IsAct(act_t::DEAD) || targ->IsAct(act_t::DYING) ||
         targ->IsAct(act_t::UNCONSCIOUS))) {
      if (mind)
        mind->Send("No need, target is down!\n");
      body->StopAct(act_t::FIGHT);
      return 0;
    }

    if (is_pc(body) && is_pc(targ)) {
      if (mind) {
        mind->SendF(
            "You can't attack %s, %s is a PC (no PvP)!\n",
            targ->Name(0, body).c_str(),
            targ->Name(0, body).c_str());
      }
      return 0;
    }

    if ((!body->Parent()) || body->Parent()->HasSkill(crc32c("Peaceful"))) {
      if (mind) {
        mind->Send("You can't fight here.  This is a place of peace.\n");
      }
      return 0;
    }

    body->BusyFor(3000); // Overridden below if is alive/animate

    if (!(!targ->IsAnimate() || targ->IsAct(act_t::DEAD) || targ->IsAct(act_t::DYING) ||
          targ->IsAct(act_t::UNCONSCIOUS))) {
      body->AddAct(act_t::FIGHT, targ);
      body->BusyFor(3000, body->Tactics().c_str());
      if (!targ->IsAct(act_t::FIGHT)) {
        targ->BusyFor(3000, body->Tactics().c_str());
        targ->AddAct(act_t::FIGHT, body);
      } else if (targ->StillBusy()) {
        body->BusyWith(targ, body->Tactics().c_str());
      }
    } else {
      attacknow = 1; // Uncontested.
    }

    // Free your off-hand if needed (if it's not a shield or weapon)
    if (body->ActTarg(act_t::HOLD) // FIXME: Don't drop offhand weapons?!?
        && body->ActTarg(act_t::HOLD) != body->ActTarg(act_t::WEAR_SHIELD) &&
        body->ActTarg(act_t::HOLD) != body->ActTarg(act_t::WIELD) &&
        (body->ActTarg(act_t::WEAR_SHIELD) // Need Off-Hand for shield
         || (body->ActTarg(act_t::WIELD) //...or for two-hander
             && two_handed(body->ActTarg(act_t::WIELD)->Skill(crc32c("WeaponType")))))) {
      if (body->DropOrStash(body->ActTarg(act_t::HOLD))) {
        if (mind)
          mind->SendF(
              "Oh, no!  You can't drop or stash %s, but you need your off-hand!",
              body->ActTarg(act_t::HOLD)->Name(0, body).c_str());
      }
    }

    // Hold your 2-hander, even if you have to let go of your shield
    if (body->ActTarg(act_t::WIELD) // Half-Wielding a 2-Hander
        && body->ActTarg(act_t::HOLD) != body->ActTarg(act_t::WIELD) &&
        two_handed(body->ActTarg(act_t::WIELD)->Skill(crc32c("WeaponType")))) {
      if (body->ActTarg(act_t::HOLD) // Some non-shield stuck in other hand!
          && body->ActTarg(act_t::HOLD) != body->ActTarg(act_t::WEAR_SHIELD)) {
        if (mind)
          mind->SendF(
              "Oh, no!  You can't use %s - it's two-handed!\n",
              body->ActTarg(act_t::WIELD)->Name(0, body).c_str());
        if (body->DropOrStash(body->ActTarg(act_t::WIELD))) {
          if (mind)
            mind->SendF(
                "Oh, no!  You can't drop or stash %s!\n",
                body->ActTarg(act_t::WIELD)->Name(0, body).c_str());
        }
      } else {
        if (body->ActTarg(act_t::HOLD)) { // Unhold your shield
          body->Parent()->SendOut(
              stealth_t,
              stealth_s,
              ";s stops holding ;s.\n",
              "You stop holding ;s.\n",
              body,
              body->ActTarg(act_t::HOLD));
          body->StopAct(act_t::HOLD);
        }
        Object* weap = body->ActTarg(act_t::WIELD); // Hold your 2-hander
        body->AddAct(act_t::HOLD, weap);
        body->Parent()->SendOut(
            stealth_t, stealth_s, ";s holds ;s.\n", "You hold ;s.\n", body, weap);
      }
    }

    // If you're hand's now free, and you have a shield, use it.
    if (body->ActTarg(act_t::WEAR_SHIELD) && (!body->IsAct(act_t::HOLD))) {
      Object* shield = body->ActTarg(act_t::WEAR_SHIELD);

      body->AddAct(act_t::HOLD, shield);
      body->Parent()->SendOut(
          stealth_t, stealth_s, ";s holds ;s.\n", "You hold ;s.\n", body, shield);
    } else if (
        mind && body->ActTarg(act_t::WEAR_SHIELD) &&
        body->ActTarg(act_t::WEAR_SHIELD) != body->ActTarg(act_t::HOLD)) {
      mind->SendF(
          "Oh, no!  You can't use %s - your off-hand is not free!\n",
          body->ActTarg(act_t::WEAR_SHIELD)->Name(0, body).c_str());
    }

    if (!attacknow) {
      body->Parent()->SendOut(
          stealth_t, stealth_s, ";s moves to attack ;s.\n", "You move to attack ;s.\n", body, targ);

      // HACK!  Make this command used first rnd!
      body->BusyWith(body, std::string(comlist[cnum].command) + " " + std::string(args));

      return 2; // No more actions until next round!
    }

    // Attacking, or being attacked removes hidden-ness.
    body->SetSkill(crc32c("Hidden"), 0);
    targ->SetSkill(crc32c("Hidden"), 0);

    int reachmod = 0;
    auto sk1 = crc32c("Punching");
    auto sk2 = crc32c("Punching");

    if (cnum == COM_KICK) {
      sk1 = crc32c("Kicking");
      sk2 = crc32c("Kicking");
    }

    else if (
        body->ActTarg(act_t::WIELD) // Not Holding your 2-Hander
        && two_handed(body->ActTarg(act_t::WIELD)->Skill(crc32c("WeaponType"))) &&
        body->ActTarg(act_t::WIELD) != body->ActTarg(act_t::HOLD)) {
      sk1 = crc32c("Kicking");
      sk2 = crc32c("Kicking");
    }

    else {
      if (body->IsAct(act_t::WIELD)) {
        sk1 = get_weapon_skill(body->ActTarg(act_t::WIELD)->Skill(crc32c("WeaponType")));
        reachmod += std::max(0, body->ActTarg(act_t::WIELD)->Skill(crc32c("WeaponReach")));
        if (reachmod > 9)
          reachmod = 0;
      }
      if (body->ActTarg(act_t::HOLD) == body->ActTarg(act_t::WEAR_SHIELD) &&
          body->ActTarg(act_t::HOLD)) {
        sk2 = crc32c("None"); // Occupy opponent's primary weapon, so they can't use it to defend.
      }
      if (targ->ActTarg(act_t::HOLD) == targ->ActTarg(act_t::WEAR_SHIELD) &&
          targ->ActTarg(act_t::HOLD)) {
        sk2 = crc32c("Shields");
        reachmod = 0; // Shield neutralizes reach
      } else if (
          targ->ActTarg(act_t::WIELD) // Not Holding their 2-Hander
          && two_handed(targ->ActTarg(act_t::WIELD)->Skill(crc32c("WeaponType"))) &&
          targ->ActTarg(act_t::WIELD) != targ->ActTarg(act_t::HOLD)) {
        sk2 = crc32c("None"); // ...so they can't defend with it
      } else if (targ->ActTarg(act_t::WIELD)) {
        if (sk2 == crc32c("Punching"))
          sk2 = get_weapon_skill(targ->ActTarg(act_t::WIELD)->Skill(crc32c("WeaponType")));
        reachmod -= std::max(0, targ->ActTarg(act_t::WIELD)->Skill(crc32c("WeaponReach")));
        if (reachmod < -9)
          reachmod = 0;
      }
    }

    if (!targ->HasSkill(sk2)) { // Not equipped to defend with a weapon skill, dodge instead
      if (targ->HasSkill(crc32c("Dodge"))) {
        sk2 = crc32c("Dodge");
        reachmod = 0; // Skilled dodge neutralizes reach
      } else {
        sk2 = crc32c("Acrobatics");
      }
    }

    std::string rolls_offense = "";
    std::string rolls_defense = "";
    int offense = body->Roll(
        sk1,
        targ->SkillTarget(sk2) - reachmod + body->Modifier("Accuracy") - targ->Modifier("Evasion"),
        (is_pc(body) || is_pc(targ)) ? &rolls_offense : nullptr);
    int defense = targ->Roll(
        sk2,
        body->SkillTarget(sk1) + reachmod,
        (is_pc(body) || is_pc(targ)) ? &rolls_defense : nullptr);
    int succ = offense - defense;

    int loc = rand() % 100;
    act_t loca = act_t::WEAR_CHEST;
    std::string locm = "";
    int stage = 0;
    if (loc < 50) {
      loca = act_t::WEAR_CHEST;
      locm = " in the chest";
    } else if (loc < 56) {
      loca = act_t::WEAR_BACK;
      locm = " in the back";
    } else if (loc < 59) {
      loca = act_t::WEAR_HEAD;
      locm = " in the head";
      stage = 1;
    } else if (loc < 60) {
      loca = act_t::WEAR_FACE;
      locm = " in the face";
      stage = 2;
    } else if (loc < 61) {
      loca = act_t::WEAR_NECK;
      locm = " in the neck";
      stage = 2;
    } else if (loc < 62) {
      loca = act_t::WEAR_COLLAR;
      locm = " in the throat";
      stage = 2;
    } else if (loc < 72) {
      loca = act_t::WEAR_LARM;
      locm = " in the left arm";
      stage = -1;
    } else if (loc < 82) {
      loca = act_t::WEAR_RARM;
      locm = " in the right arm";
      stage = -1;
    } else if (loc < 86) {
      loca = act_t::WEAR_LLEG;
      locm = " in the left leg";
      stage = -1;
    } else if (loc < 90) {
      loca = act_t::WEAR_RLEG;
      locm = " in the right leg";
      stage = -1;
    } else if (loc < 93) {
      loca = act_t::WEAR_LHAND;
      locm = " in the left hand";
      stage = -2;
    } else if (loc < 96) {
      loca = act_t::WEAR_RHAND;
      locm = " in the right hand";
      stage = -2;
    } else if (loc < 98) {
      loca = act_t::WEAR_LFOOT;
      locm = " in the left foot";
      stage = -2;
    } else {
      loca = act_t::WEAR_RFOOT;
      locm = " in the right foot";
      stage = -2;
    }

    std::string verb = "punch"; // Non-weapon verb
    std::string verb3 = "punches"; // 3rd Person
    if (body->Skill(crc32c("NaturalWeapon")) == 14) { // Natural Weapon: stab
      verb = "stab";
      verb3 = "stabs";
    } else if (body->Skill(crc32c("NaturalWeapon")) == 13) { // Natural Weapon: hit
      verb = "hit";
      verb3 = "hits";
    } else if (body->Skill(crc32c("NaturalWeapon")) == 12) { // Natural Weapon: blast
      verb = "blast";
      verb3 = "blasts";
    } else if (body->Skill(crc32c("NaturalWeapon")) == 11) { // Natural Weapon: pierce
      verb = "pierce";
      verb3 = "pierces";
    } else if (body->Skill(crc32c("NaturalWeapon")) == 10) { // Natural Weapon: thrash
      verb = "thrash";
      verb3 = "thrashes";
    } else if (body->Skill(crc32c("NaturalWeapon")) == 9) { // Natural Weapon: maul
      verb = "maul";
      verb3 = "mauls";
    } else if (body->Skill(crc32c("NaturalWeapon")) == 8) { // Natural Weapon: claw
      verb = "claw";
      verb3 = "claws";
    } else if (body->Skill(crc32c("NaturalWeapon")) == 7) { // Natural Weapon: pound
      verb = "pound";
      verb3 = "pounds";
    } else if (body->Skill(crc32c("NaturalWeapon")) == 6) { // Natural Weapon: crush
      verb = "crush";
      verb3 = "crushes";
    } else if (body->Skill(crc32c("NaturalWeapon")) == 5) { // Natural Weapon: bludgeon
      verb = "bludgeon";
      verb3 = "bludgeons";
    } else if (body->Skill(crc32c("NaturalWeapon")) == 4) { // Natural Weapon: bite
      verb = "bite";
      verb3 = "bites";
    } else if (body->Skill(crc32c("NaturalWeapon")) == 3) { // Natural Weapon: slash
      verb = "slash";
      verb3 = "slashes";
    } else if (body->Skill(crc32c("NaturalWeapon")) == 2) { // Natural Weapon: whip
      verb = "whip";
      verb3 = "whips";
    } else if (body->Skill(crc32c("NaturalWeapon")) == 1) { // Natural Weapon: sting
      verb = "sting";
      verb3 = "stings";
    }

    int defense_armor = 0;
    std::string rolls_armor = "";
    if (succ > 0) {
      int stun = 0;
      if (sk1 == crc32c("Kicking")) { // Kicking Action
        stun = 1;
        body->Parent()->SendOutF(
            ALL, -1, "*;s kicks ;s%s.\n", "*You kick ;s%s.\n", body, targ, locm.c_str());
      } else if (
          body->IsAct(act_t::WIELD) // Ranged Weapon
          && body->ActTarg(act_t::WIELD)->Skill(crc32c("WeaponReach")) > 9) {
        body->Parent()->SendOutF(
            ALL,
            -1,
            "*;s throws %s and hits ;s%s.\n",
            "*You throw %s and hit ;s%s.\n",
            body,
            targ,
            body->ActTarg(act_t::WIELD)->ShortDescC(),
            locm.c_str());
        body->ActTarg(act_t::WIELD)->Travel(body->Parent()); // FIXME: Get Another
        body->StopAct(act_t::WIELD); // FIXME: Bows/Guns!
      } else if (body->IsAct(act_t::WIELD)) { // Melee Weapon
        body->Parent()->SendOutF(
            ALL,
            -1,
            "*;s hits ;s%s with %s.\n",
            "*You hit ;s%s with %s.\n",
            body,
            targ,
            locm.c_str(),
            body->ActTarg(act_t::WIELD)->ShortDescC());
      } else { // No Weapon or Natural Weapon
        if (!body->HasSkill(crc32c("NaturalWeapon")))
          stun = 1;
        char mes[128] = "";
        char mes3[128] = "";
        sprintf(mes, "*You %s ;s%s.\n", verb.c_str(), locm.c_str());
        sprintf(mes3, "*;s %s ;s%s.\n", verb3.c_str(), locm.c_str());
        body->Parent()->SendOut(ALL, -1, mes3, mes, body, targ);
      }

      int sev = 0;
      int force = body->ModAttribute(2) + body->Modifier("Damage");

      if (cnum == COM_KICK) {
        force -= 2;
        stage += 2;
      }
      if (body->IsAct(act_t::WIELD)) {
        force += body->ActTarg(act_t::WIELD)->Skill(crc32c("WeaponForce"));
        if (two_handed(body->ActTarg(act_t::WIELD)->Skill(crc32c("WeaponType")))) {
          force += body->ModAttribute(2);
        }
        stage += body->ActTarg(act_t::WIELD)->Skill(crc32c("WeaponSeverity"));
      } else {
        force -= 1;
        stage += 1;
      }

      if (targ->ActTarg(loca)) {
        // FIXME: Implement the rest of the Armor Effect types
        defense_armor = targ->ActTarg(loca)->Roll(crc32c("Body"), force, &rolls_armor);
        succ -= defense_armor;
      }

      if (stun) {
        sev = targ->HitStun(force, stage, succ);
      } else {
        sev = targ->HitPhys(force, stage, succ);
      }
      if (body->Skill(crc32c("Poisonous")) > 0) { // Injects poison!
        targ->SetSkill(
            crc32c("Poisoned"), targ->Skill(crc32c("Poisoned")) + body->Skill(crc32c("Poisonous")));
      }

      if (sev <= 0) {
        if (mind)
          mind->Send("You hit - but didn't do much.\n"); // FIXME - Real
        // Messages
      }
    } else {
      if (cnum == COM_KICK) { // Kicking Action
        body->Parent()->SendOut(
            ALL,
            -1,
            ";s tries to kick ;s, but misses.\n",
            "You try to kick ;s, but miss.\n",
            body,
            targ);
      } else if (
          body->IsAct(act_t::WIELD) // Ranged Weapon
          && body->ActTarg(act_t::WIELD)->Skill(crc32c("WeaponReach")) > 9) {
        body->Parent()->SendOutF(
            ALL,
            -1,
            "*;s throws %s at ;s, but misses.\n",
            "*You throw %s at ;s, but miss.\n",
            body,
            targ,
            body->ActTarg(act_t::WIELD)->ShortDescC());
        body->ActTarg(act_t::WIELD)->Travel(body->Parent()); // FIXME: Get Another
        body->StopAct(act_t::WIELD); // FIXME: Bows/Guns!
      } else if (body->IsAct(act_t::WIELD)) { // Melee Weapon
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
            verb.c_str());
      }
    }

    if (!targ->IsAnimate() || targ->IsAct(act_t::DEAD) || targ->IsAct(act_t::DYING) ||
        targ->IsAct(act_t::UNCONSCIOUS)) {
      body->StopAct(act_t::FIGHT);
      body->BusyFor(3000);
      if (targ->Skill(crc32c("Accomplishment"))) {
        body->Accomplish(targ->Skill(crc32c("Accomplishment")), "this victory");
        targ->SetSkill(crc32c("Accomplishment"), 0);
      }
    }

    if (rolls_armor.empty()) {
      body->SendF(
          CHANNEL_ROLLS,
          CMAG "[%d] = %s - %s\n" CNRM,
          succ,
          rolls_offense.c_str(),
          rolls_defense.c_str());
      targ->SendF(
          CHANNEL_ROLLS,
          CMAG "[%d] = %s - %s\n" CNRM,
          succ,
          rolls_offense.c_str(),
          rolls_defense.c_str());
    } else {
      body->SendF(
          CHANNEL_ROLLS,
          CMAG "[%d] = %s - %s - %s\n" CNRM,
          succ,
          rolls_offense.c_str(),
          rolls_defense.c_str(),
          rolls_armor.c_str());
      targ->SendF(
          CHANNEL_ROLLS,
          CMAG "[%d] = %s - %s - %s\n" CNRM,
          succ,
          rolls_offense.c_str(),
          rolls_defense.c_str(),
          rolls_armor.c_str());
    }

    return 0;
  }

  static const std::string statnames[] = {
      "Body", "Quickness", "Strength", "Charisma", "Intelligence", "Willpower"};

  if (cnum == COM_RESETCHARACTER) {
    Object* chr = mind->Owner()->Creator();
    if (!chr) {
      mind->Send(
          "You need to be working on a character first (use 'select "
          "<character>'.\n");
      return 0;
    } else if (!args.empty()) {
      mind->SendF(
          "Just type 'reset' to undo all your work and start over on %s\n", chr->ShortDescC());
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

    mind->SendF("You reset all chargen work for '%s'.\n", body->ShortDescC());
    return 0;
  }

  if (cnum == COM_RANDOMIZE) {
    Object* chr = mind->Owner()->Creator();
    if (!chr) {
      mind->Send(
          "You need to be working on a character first (use 'select "
          "<character>'.\n");
      return 0;
    } else if (!args.empty()) {
      mind->SendF("Just type 'randomize' to randomly spend all points for %s\n", chr->ShortDescC());
      return 0;
    }

    if (chr->Exp() > 0) {
      mind->Send("This is not a new character, you can't modify the chargen steps anymore.\n");
      return 0;
    }

    while (chr->Skill(crc32c("Attribute Points")) > 0) {
      int which = (rand() % 6);
      if (chr->NormAttribute(which) < 6) {
        chr->SetSkill(crc32c("Attribute Points"), chr->Skill(crc32c("Attribute Points")) - 1);
        chr->SetAttribute(which, chr->NormAttribute(which) + 1);
      }
    }

    auto skills = get_skills("all");
    while (chr->Skill(crc32c("Skill Points"))) {
      int which = (rand() % skills.size());
      auto skl = skills.begin();
      while (which) {
        ++skl;
        --which;
      }
      if (chr->Skill(*skl) < (chr->NormAttribute(get_linked(*skl)) + 1) / 2 &&
          chr->Skill(crc32c("Skill Points")) > chr->Skill(*skl)) {
        chr->SetSkill(*skl, chr->Skill(*skl) + 1);
        chr->SetSkill(
            crc32c("Skill Points"), chr->Skill(crc32c("Skill Points")) - chr->Skill(*skl));
      }
    }
    mind->SendF("You randomly spend all remaining points for '%s'.\n", chr->ShortDescC());
    return 0;
  }

  if (cnum == COM_ARCHETYPE) {
    Object* chr = mind->Owner()->Creator();
    if (!chr) {
      mind->Send(
          "You need to be working on a character first (use 'select "
          "<character>'.\n");
      return 0;
    } else if (args.empty()) {
      mind->SendF("You need to select an archetype to apply to %s.\n", chr->ShortDescC());
      mind->SendF("Supported archetypes are:\n");
      mind->SendF("  1. Fighter\n");
      return 0;
    }

    if (chr->Exp() > 0) {
      mind->Send("This is not a new character, you can't modify the chargen steps anymore.\n");
      return 0;
    }

    if (args == std::string_view("fighter").substr(0, args.length())) {
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
      body->SetSkill(crc32c("Attribute Points"), 0);

      body->SetSkill(crc32c("Long Blades"), 3);
      body->SetSkill(crc32c("Two-Handed Blades"), 3);
      body->SetSkill(crc32c("Short Piercing"), 3);
      body->SetSkill(crc32c("Shields"), 3);
      body->SetSkill(crc32c("Running"), 3);
      body->SetSkill(crc32c("Climbing"), 2);
      body->SetSkill(crc32c("Sprinting"), 2);
      body->SetSkill(crc32c("Swimming"), 2);
      body->SetSkill(crc32c("Lifting"), 2);
      body->SetSkill(crc32c("Acrobatics"), 2);
      body->SetSkill(crc32c("Punching"), 3);
      body->SetSkill(crc32c("Kicking"), 3);
      body->SetSkill(crc32c("Grappling"), 3);
      body->SetSkill(crc32c("Intimidation"), 1);
      body->SetSkill(crc32c("Skill Points"), 0);

      auto weap = new Object(body);
      weap->SetShortDesc("a dull arming sword");
      weap->SetDesc("This sword really isn't that great.  Is even metal?  It's sure not steel.");
      weap->SetSkill(crc32c("WeaponType"), get_weapon_type("Long Blades"));
      weap->SetSkill(crc32c("WeaponForce"), -2);
      weap->SetSkill(crc32c("WeaponSeverity"), 1);
      weap->SetSkill(crc32c("WeaponReach"), 1);
      weap->SetPos(pos_t::LIE);
      body->AddAct(act_t::WIELD, weap);

      auto shi = new Object(body);
      shi->SetShortDesc("a cracked leather shield");
      shi->SetDesc("This shield has seen better days... but, it was pretty bad back then too.");
      shi->SetSkill(crc32c("Wearable on Shield"), 1);
      shi->SetAttribute(0, 1);
      shi->SetPos(pos_t::LIE);
      body->AddAct(act_t::WEAR_SHIELD, shi);

      auto arm = new Object(body);
      arm->SetShortDesc("a full suit of old padded armor");
      arm->SetDesc("This armor smells pretty bad, but it's better than nothing... maybe.");
      arm->SetSkill(crc32c("Wearable on Back"), 1);
      arm->SetSkill(crc32c("Wearable on Chest"), 1);
      arm->SetSkill(crc32c("Wearable on Left Arm"), 1);
      arm->SetSkill(crc32c("Wearable on Right Arm"), 1);
      arm->SetSkill(crc32c("Wearable on Left Leg"), 1);
      arm->SetSkill(crc32c("Wearable on Right Leg"), 1);
      arm->SetAttribute(0, 1);
      arm->SetPos(pos_t::LIE);
      body->AddAct(act_t::WEAR_BACK, arm);
      body->AddAct(act_t::WEAR_CHEST, arm);
      body->AddAct(act_t::WEAR_LARM, arm);
      body->AddAct(act_t::WEAR_RARM, arm);
      body->AddAct(act_t::WEAR_LLEG, arm);
      body->AddAct(act_t::WEAR_RLEG, arm);

      auto helm = new Object(body);
      helm->SetShortDesc("a soft leather cap");
      helm->SetDesc("This is... armor... probably.");
      helm->SetSkill(crc32c("Wearable on Head"), 1);
      helm->SetAttribute(0, 1);
      helm->SetPos(pos_t::LIE);
      body->AddAct(act_t::WEAR_HEAD, helm);

      body->SetSkill(crc32c("Status Points"), 0);

      mind->SendF("You reform '%s' into a %s.\n", body->ShortDescC(), "Fighter");
      mind->SendF("You can now adjust things from here, or just enter the game.\n");

    } else {
      mind->SendF(
          "You need to select a *supported* archetype to apply to %s.\n", chr->ShortDescC());
      mind->SendF("Supported archetypes are:\n");
      mind->SendF("  1. Fighter\n");
    }

    return 0;
  }

  if (cnum == COM_LOWER) {
    if ((!mind) || (!mind->Owner()))
      return 0;

    Object* chr = mind->Owner()->Creator();
    if (!chr) {
      mind->Send("You need to be working on a character first (use 'select <character>'.\n");
      return 0;
    }

    if (chr->Exp() > 0) {
      mind->Send("This is not a new character, you can't modify the chargen steps anymore.\n");
      return 0;
    }

    if (args == std::string_view("body").substr(0, args.length()) ||
        args == std::string_view("quickness").substr(0, args.length()) ||
        args == std::string_view("strength").substr(0, args.length()) ||
        args == std::string_view("charisma").substr(0, args.length()) ||
        args == std::string_view("intelligence").substr(0, args.length()) ||
        args == std::string_view("willpower").substr(0, args.length())) {
      int attr = 0;
      if (ascii_toupper(args[0]) == 'B')
        attr = 0;
      else if (ascii_toupper(args[0]) == 'Q')
        attr = 1;
      else if (ascii_toupper(args[0]) == 'S')
        attr = 2;
      else if (ascii_toupper(args[0]) == 'C')
        attr = 3;
      else if (ascii_toupper(args[0]) == 'I')
        attr = 4;
      else if (ascii_toupper(args[0]) == 'W')
        attr = 5;

      if (chr->NormAttribute(attr) < 3) {
        mind->SendF("Your %s is already at the minimum.\n", statnames[attr].c_str());
        return 0;
      } else {
        chr->SetAttribute(attr, chr->NormAttribute(attr) - 1);
        chr->SetSkill(crc32c("Attribute Points"), chr->Skill(crc32c("Attribute Points")) + 1);
        mind->SendF("You lower your %s.\n", statnames[attr].c_str());
      }
    } else {
      auto skill = get_skill(std::string(args));
      if (skill != crc32c("None")) {
        if (chr->Skill(skill) < 1) {
          mind->SendF("You don't have %s.\n", SkillName(skill).c_str());
          return 0;
        }
        chr->SetSkill(skill, chr->Skill(skill) - 1);
        chr->SetSkill(
            crc32c("Skill Points"), chr->Skill(crc32c("Skill Points")) + chr->Skill(skill) + 1);
        mind->SendF("You lower your %s skill.\n", SkillName(skill).c_str());
      } else {
        mind->Send("I'm not sure what you are trying to lower.\n");
      }
    }
    return 0;
  }

  if (cnum == COM_RAISE) {
    if ((!mind) || (!mind->Owner()))
      return 0;
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
    if (args.empty()) {
      mind->Send("What do you want to buy?\n");
      return 0;
    }

    if (args == std::string_view("body").substr(0, args.length()) ||
        args == std::string_view("quickness").substr(0, args.length()) ||
        args == std::string_view("strength").substr(0, args.length()) ||
        args == std::string_view("charisma").substr(0, args.length()) ||
        args == std::string_view("intelligence").substr(0, args.length()) ||
        args == std::string_view("willpower").substr(0, args.length())) {
      if ((!body) && (chr->Skill(crc32c("Attribute Points")) < 1)) {
        mind->Send("You have no free attribute points left.\n");
        return 0;
      }
      int attr = 0;
      if (ascii_toupper(args[0]) == 'B')
        attr = 0;
      else if (ascii_toupper(args[0]) == 'Q')
        attr = 1;
      else if (ascii_toupper(args[0]) == 'S')
        attr = 2;
      else if (ascii_toupper(args[0]) == 'C')
        attr = 3;
      else if (ascii_toupper(args[0]) == 'I')
        attr = 4;
      else if (ascii_toupper(args[0]) == 'W')
        attr = 5;

      if (body && chr->TotalExp() < 20) {
        mind->SendF(
            "You don't have enough experience to raise your %s.\n"
            "You need 20, but you only have %d\n",
            statnames[attr].c_str(),
            chr->TotalExp());
        return 0;
      }

      uint32_t maxask[6] = {
          crc32c("MaxBody"),
          crc32c("MaxQuickness"),
          crc32c("MaxStrength"),
          crc32c("MaxCharisma"),
          crc32c("MaxIntelligence"),
          crc32c("MaxWillpower")};
      if ((body) && (!body->Skill(maxask[attr]))) {
        body->SetSkill(maxask[attr], (body->NormAttribute(attr) * 3) / 2);
      }

      if ((!body) && chr->NormAttribute(attr) >= 6) {
        mind->SendF("Your %s is already at the maximum.\n", statnames[attr].c_str());
      } else if (body && chr->NormAttribute(attr) >= body->Skill(maxask[attr])) {
        mind->SendF("Your %s is already at the maximum.\n", statnames[attr].c_str());
      } else {
        if (!body)
          chr->SetSkill(crc32c("Attribute Points"), chr->Skill(crc32c("Attribute Points")) - 1);
        else
          chr->SpendExp(20);
        chr->SetAttribute(attr, chr->NormAttribute(attr) + 1);
        mind->SendF("You raise your %s.\n", statnames[attr].c_str());
      }
    } else {
      auto skill = get_skill(std::string(args));
      if (skill != crc32c("None")) {
        if (body && (chr->Skill(skill) >= (chr->NormAttribute(get_linked(skill)) * 3 + 1) / 2)) {
          mind->SendF("Your %s is already at the maximum.\n", SkillName(skill).c_str());
          return 0;
        } else if (
            (!body) && (chr->Skill(skill) >= (chr->NormAttribute(get_linked(skill)) + 1) / 2)) {
          mind->SendF("Your %s is already at the maximum.\n", SkillName(skill).c_str());
          return 0;
        }
        int cost = (chr->Skill(skill) + 1);
        if (body) {
          if (cost > chr->NormAttribute(get_linked(skill)))
            cost *= 2;
          if (chr->TotalExp() < (cost * 2)) {
            mind->SendF(
                "You don't have enough experience to raise your %s.\n"
                "You need %d, but you only have %d\n",
                SkillName(skill).c_str(),
                cost * 2,
                chr->TotalExp());
            return 0;
          }
        } else if (!chr->Skill(crc32c("Skill Points"))) {
          mind->Send("You have no free skill points left.\n");
          return 0;
        } else if (chr->Skill(crc32c("Skill Points")) < cost) {
          mind->Send("You don't have enough free skill points left.\n");
          return 0;
        }
        if (body)
          chr->SpendExp(cost * 2);
        else
          chr->SetSkill(crc32c("Skill Points"), chr->Skill(crc32c("Skill Points")) - cost);
        chr->SetSkill(skill, chr->Skill(skill) + 1);
        mind->SendF("You raise your %s skill.\n", SkillName(skill).c_str());
      } else {
        mind->Send("I'm not sure what you are trying to raise.\n");
      }
    }
    return 0;
  }

  if (cnum == COM_NEWCHARACTER) {
    if (!mind)
      return 0; // FIXME: Should never happen!
    if (args.empty()) {
      mind->Send("What's the character's name?  Use 'newcharacter <charname>'.\n");
      return 0;
    }

    if (!std::all_of(args.begin(), args.end(), ascii_isalpha)) {
      mind->Send(
          "Sorry, character names can only contain letters.\n"
          "Pick another name.\n");
      return 0;
    }

    body = mind->Owner()->Room()->PickObject(std::string(args), vmode | LOC_INTERNAL);
    if (body) {
      mind->Send(
          "Sorry, you already have a character with that name.\n"
          "Pick another name.\n");
      return 0;
    }
    body = new_body();
    body->SetShortDesc(std::string(args));
    mind->Owner()->AddChar(body);
    mind->SendF("You created %s.\n", std::string(args).c_str());
    return 0;
  }

  if (cnum == COM_RECALL) {
    if (body->Phys() || body->Stun()) {
      if (mind)
        mind->Send("You must be uninjured to use that command!\n");
    } else {
      Object* dest = body;
      while ((!dest->ActTarg(act_t::SPECIAL_HOME)) && dest->Parent()) {
        dest = dest->Parent();
      }
      if (dest->ActTarg(act_t::SPECIAL_HOME)) {
        dest = dest->ActTarg(act_t::SPECIAL_HOME);
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

  if (cnum == COM_TELEPORT) {
    if (args.empty()) {
      mind->Send("Where do you want to teleport to?.\n");
      return 0;
    }

    Object* src = nullptr;
    if (!nmode)
      src = body->NextHasSkill(crc32c("Restricted Item"));
    while (src) {
      if (!src->HasSkill(crc32c("Teleport"))) {
        src = body->NextHasSkill(crc32c("Restricted Item"), src);
        continue;
      }
      std::string comline = "teleport ";
      comline += (std::string(args));
      comline += "\n";
      if (!strcasestr(src->LongDescC(), comline.c_str())) {
        src = body->NextHasSkill(crc32c("Restricted Item"), src);
        continue;
      }
      break;
    }

    if ((!nmode) && (!src) && body->Skill(crc32c("Teleport")) < 1) {
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
    if (nmode && args == "universe") {
      dest = dest->Universe();
    } else if (nmode && args == "trashbin") {
      dest = dest->TrashBin();
    } else { // Only Ninjas can teleport to "Universe"/"TrashBin"
      while (dest->Parent()->Parent()) {
        dest = dest->Parent();
      }
      dest = dest->PickObject(std::string(args), vmode | LOC_INTERNAL);
    }

    if (!dest) {
      if (mind)
        mind->Send("No such teleportation destination found.\n");
    } else {
      body->SetSkill(crc32c("Teleport"), 0); // Use it up
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

  if (cnum == COM_RESURRECT) {
    if ((!nmode) && body->Skill(crc32c("Resurrect")) < 1) {
      if (mind)
        mind->Send("You don't have the power to resurrect!\n");
      return 0;
    }
    if (args.empty()) {
      mind->Send("Who do you want to resurrect?.\n");
      return 0;
    }

    std::vector<Player*> pls = get_all_players();
    for (auto pl : pls) {
      auto chs = pl->Room()->Contents();
      for (auto ch : chs) {
        if (ch->Matches(std::string(args))) {
          if (ch->IsActive()) {
            if (mind)
              mind->SendF("%s is not long dead (yet).\n", ch->Name().c_str());
          } else {
            body->SetSkill(crc32c("Resurrect"), 0); // Use it up
            ch->SetSkill(crc32c("Poisoned"), 0);
            ch->SetSkill(crc32c("Thirsty"), 0);
            ch->SetSkill(crc32c("Hungry"), 0);
            ch->SetPhys(0);
            ch->SetStun(0);
            ch->SetStru(0);
            ch->UpdateDamage();
            ch->Activate();
            ch->Parent()->SendOut(
                stealth_t, stealth_s, ";s has been resurrected!.\n", "", ch, nullptr);
            if (mind)
              mind->SendF("%s has been resurrected!\n", ch->Name().c_str());
          }
          return 0;
        }
      }
    }
    if (mind)
      mind->SendF("%s isn't a character on this MUD\n", std::string(args).c_str());
    return 0;
  }

  if (cnum == COM_SKILLLIST) {
    if (!mind)
      return 0;

    std::string skills;
    std::vector<uint32_t> skls;
    if (args.empty()) {
      skills = "Here are all the skill categories (use 'skill <Category>' to see the skills):\n";
      skls = get_skills();
    } else {
      std::string cat = get_skill_cat(std::string(args));
      if (args != "all") {
        if (cat == "") {
          mind->SendF("There is no skill category called '%s'.\n", std::string(args).c_str());
          return 0;
        }
        skills = "Total " + cat + " in play on this MUD:\n";
      } else {
        skills = "Total skills in play on this MUD:\n";
        cat = "all";
      }
      skls = get_skills(cat);
    }

    std::vector<std::string> sknms;
    for (auto skl : skls) {
      sknms.push_back(SkillName(skl));
    }
    std::sort(sknms.begin(), sknms.end());
    for (auto skn : sknms) {
      skills += skn;
      skills += "\n";
    }
    mind->Send(skills.c_str());

    return 0;
  }

  if (cnum == COM_TOGGLE) {
    if (!mind)
      return 0;
    Player* pl = mind->Owner();
    if (!pl)
      return 0;

    if (!args.empty() && args == std::string_view("restore").substr(0, args.length())) {
      mind->SetSVars(pl->Vars());
      mind->Send("Your settings have been reset to your defaults.\n");
      args = ""; // Show current settings too
    }

    if (args.empty()) {
      mind->Send("Your current settings:\n");

      if (mind->IsSVar("combatinfo")) {
        mind->Send("  CombatInfo is " CYEL "on" CNRM ".\n");
      } else {
        mind->Send("  CombatInfo is " CYEL "off" CNRM ".\n");
      }

    } else if (args == std::string_view("combatinfo").substr(0, args.length())) {
      if (mind->IsSVar("combatinfo")) {
        mind->ClearSVar("combatinfo");
        mind->Send("CombatInfo is now " CYEL "off" CNRM ".\n");
      } else {
        mind->SetSVar("combatinfo", "1");
        mind->Send("CombatInfo is now " CYEL "on" CNRM ".\n");
      }
    } else if (args == std::string_view("save").substr(0, args.length())) {
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

  if (cnum == COM_WHO) {
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

  if (cnum == COM_OOC) {
    if (!mind)
      return 0;
    if (args.empty()) {
      if (strncmp(mind->SpecialPrompt().c_str(), "ooc", 3)) {
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
      std::string mes = std::string("OOC: <") + name + "> " + (std::string(args)) + "\n";
      std::vector<Mind*> mns = get_human_minds();
      for (auto mn : mns) {
        mn->Send(mes.c_str());
      }
    }
    return 0;
  }

  if (cnum == COM_NEWBIE) {
    if (!mind)
      return 0;
    if (args.empty()) {
      if (strncmp(mind->SpecialPrompt().c_str(), "newbie", 3)) {
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
      std::string mes = std::string("NEWBIE: <") + name + "> " + (std::string(args)) + "\n";
      std::vector<Mind*> mns = get_human_minds();
      for (auto mn : mns) {
        mn->Send(mes.c_str());
      }
    }
    return 0;
  }

  // Ninja Commands

  if (cnum == COM_NINJAMODE) {
    if (!mind)
      return 0;

    Player* pl = mind->Owner();
    if (args.empty()) {
      mind->Send("What command do you want to run in Ninja Mode[TM]?\n");
      mind->Send("You need to include the command, like 'ninja junk boat'.\n");
    } else if (!pl) {
      mind->Send("Sorry, you don't seem to be a player!\n");
    } else {
      pl->Set(PLAYER_NINJAMODE);
      mind->Send("Ninja mode activated.\n");
      handle_single_command(body, std::string(args), mind);
      pl->UnSet(PLAYER_NINJAMODE);
      mind->Send("Ninja mode deactivated.\n");
    }
    return 0;
  }

  if (cnum == COM_MAKENINJA) {
    if (!mind)
      return 0;

    Player* pl = get_player(std::string(args));
    if (args.empty()) {
      mind->Send("You can only make/unmake other True Ninjas[TM]\n");
      return 0;
    }
    if (!pl) {
      mind->SendF("There is no PLAYER named '%s'\n", std::string(args).c_str());
      return 0;
    }
    if (pl == mind->Owner()) {
      mind->Send("You can only make/unmake other True Ninjas[TM]\n");
      return 0;
    }

    if (pl->Is(PLAYER_SUPERNINJA)) {
      mind->SendF(
          "'%s' is already a Super Ninja[TM] - this is irrevocable.\n", std::string(args).c_str());
    } else if (pl->Is(PLAYER_NINJA)) {
      pl->UnSet(PLAYER_NINJA);
      pl->UnSet(PLAYER_NINJAMODE);
      mind->SendF("Now '%s' is no longer a True Ninja[TM].\n", std::string(args).c_str());
    } else {
      pl->Set(PLAYER_NINJA);
      mind->SendF("You made '%s' into a True Ninja[TM].\n", std::string(args).c_str());
    }

    return 0;
  }

  if (cnum == COM_MAKESUPERNINJA) {
    if (!mind)
      return 0;

    Player* pl = get_player(std::string(args));
    if (args.empty()) {
      mind->Send("You can only make other Super Ninjas[TM]\n");
      return 0;
    }
    if (!pl) {
      mind->SendF("There is no PLAYER named '%s'\n", std::string(args).c_str());
      return 0;
    }
    if (pl == mind->Owner()) {
      mind->Send("You can only make other Super Ninjas[TM]\n");
      return 0;
    }

    if (pl->Is(PLAYER_SUPERNINJA)) {
      mind->SendF(
          "'%s' is already a Super Ninja[TM] - this is irrevocable.\n", std::string(args).c_str());
    } else if (!pl->Is(PLAYER_NINJA)) {
      mind->SendF(
          "'%s' isn't even a True Ninja[TM] yet!\n"
          "Be careful - Super Ninja[TM] status is irrevocable.\n",
          std::string(args).c_str());
    } else {
      pl->Set(PLAYER_SUPERNINJA);
      mind->SendF("You made '%s' into a Super Ninja[TM].\n", std::string(args).c_str());
    }

    return 0;
  }

  if (cnum == COM_MAKESTART) {
    if (!mind)
      return 0;
    Object* world = body->World();
    world->AddAct(act_t::SPECIAL_HOME, body->Parent());
    world->Parent()->AddAct(act_t::SPECIAL_HOME, body->Parent());
    mind->Send("You make this the default starting room for players.\n");
    return 0;
  }

  if (cnum == COM_SETSTATS) {
    if (!mind)
      return 0;
    Object* targ = body->ActTarg(act_t::POINT);
    if (!targ) {
      mind->Send("You need to be pointing at your target.\n");
      return 0;
    }
    if (!args.empty()) {
      char gender = 0;
      int num, weight = 0, size = 0, volume = 0, value = 0;
      num = sscanf(
          std::string(args).c_str(), "%d %d %d %d %c;\n", &weight, &size, &volume, &value, &gender);
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

  if (cnum == COM_NAME) {
    if (!mind)
      return 0;
    Object* targ = body->ActTarg(act_t::POINT);
    if (!targ) {
      mind->Send("You need to be pointing at your target.\n");
      return 0;
    }
    if (!args.empty()) {
      std::string oldn = targ->ShortDesc();
      targ->SetShortDesc(std::string(args));
      mind->SendF(
          "You rename '%s' to '%s'\n",
          oldn.c_str(),
          targ->ShortDescC()); // FIXME - Real Message
    } else {
      mind->Send("Rename it to what?\n");
    }
    return 0;
  }

  if (cnum == COM_UNDESCRIBE) {
    if (!mind)
      return 0;
    Object* targ = body->ActTarg(act_t::POINT);
    if (!targ) {
      mind->Send("You need to be pointing at your target.\n");
      return 0;
    }
    targ->SetDesc("");
    mind->SendF("You remove the description from '%s'\n", targ->Name(0, body).c_str());
    return 0;
  }

  if (cnum == COM_DESCRIBE) {
    if (!mind)
      return 0;
    Object* targ = body->ActTarg(act_t::POINT);
    if (!targ) {
      mind->Send("You need to be pointing at your target.\n");
      return 0;
    }
    if (!args.empty()) {
      if (targ->Desc() == targ->ShortDesc()) {
        targ->SetDesc(std::string(args));
        mind->SendF("You add a description to '%s'\n", targ->Name(0, body).c_str());
      } else {
        targ->SetDesc(targ->Desc() + std::string("\n") + (std::string(args)));
        mind->SendF("You add to the description of '%s'\n", targ->Name(0, body).c_str());
      }
    } else {
      if (strncmp(mind->SpecialPrompt().c_str(), "nin des", 7)) {
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

  if (cnum == COM_UNDEFINE) {
    if (!mind)
      return 0;
    Object* targ = body->ActTarg(act_t::POINT);
    if (!targ) {
      mind->Send("You need to be pointing at your target.\n");
      return 0;
    }
    targ->SetLongDesc("");
    mind->SendF("You remove the definition from '%s'\n", targ->Name(0, body).c_str());
    return 0;
  }

  if (cnum == COM_DEFINE) {
    if (!mind)
      return 0;
    Object* targ = body->ActTarg(act_t::POINT);
    if (!targ) {
      mind->Send("You need to be pointing at your target.\n");
      return 0;
    }
    if (!args.empty()) {
      if (targ->LongDesc() == targ->Desc()) {
        targ->SetLongDesc(std::string(args));
        mind->SendF("You add a definition to '%s'\n", targ->Name(0, body).c_str());
      } else {
        targ->SetLongDesc(targ->LongDesc() + std::string("\n") + (std::string(args)));
        mind->SendF("You add to the definition of '%s'\n", targ->Name(0, body).c_str());
      }
    } else {
      if (strncmp(mind->SpecialPrompt().c_str(), "nin def", 7)) {
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

  if (cnum == COM_CONTROL) {
    if (!mind)
      return 0;
    Object* targ = body->PickObject(std::string(args), vmode | LOC_NEARBY);
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

  if (cnum == COM_COMMAND) {
    if (!mind)
      return 0;
    Object* targ = body->ActTarg(act_t::POINT);
    if (!targ) {
      mind->Send("You need to be pointing at your target.\n");
    } else if (args.empty()) {
      mind->SendF("Command %s to do what?\n", targ->ShortDescC());
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
          std::string(args).c_str());

      if (handle_command(targ, std::string(args)) > 0)
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

  if (cnum == COM_CONNECT) {
    Object *src = body->ActTarg(act_t::POINT), *dest = nullptr;
    if (!src) {
      mind->Send("You need to be pointing at your source.\n");
      return 0;
    }
    dest = body->PickObject(
        std::string(args), vmode | LOC_INTERNAL | LOC_NEARBY | LOC_SELF | LOC_HERE);
    if (!dest) {
      if (mind)
        mind->Send("I can't find that destination.\n");
    } else {
      Object* exit = new Object(dest);
      exit->SetShortDesc("a passage exit");
      exit->SetDesc("A passage exit.");
      exit->SetSkill(crc32c("Invisible"), 1000);
      src->SetSkill(crc32c("Open"), 1000);
      src->SetSkill(crc32c("Enterable"), 1);
      src->AddAct(act_t::SPECIAL_LINKED, exit);
      exit->AddAct(act_t::SPECIAL_MASTER, src);
      if (mind) {
        mind->SendF(
            "You link %s to %s.\n", src->Name(0, body).c_str(), dest->Name(0, body).c_str());
      }
    }
    return 0;
  }

  if (cnum == COM_CREATE) {
    if (!mind)
      return 0;
    Object* obj = new Object(body->Parent());
    if (!args.empty()) {
      obj->SetShortDesc(std::string(args));
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

  if (cnum == COM_DCREATE) {
    if (!mind)
      return 0;
    if (args.empty()) {
      mind->Send("You need to specify in what direction!\n");
    } else {
      Object* box = new Object(body->Parent()->Parent());
      Object* next = new Object(box);
      std::string dirb = "south";
      std::string dir = "north";
      if (args == "north") {
      } else if (args == "south") {
        dirb = "north";
        dir = "south";
      } else if (args == "west") {
        dirb = "east";
        dir = "west";
      } else if (args == "east") {
        dirb = "west";
        dir = "east";
      } else if (args == "up") {
        dirb = "down";
        dir = "up";
      } else if (args == "down") {
        dirb = "up";
        dir = "down";
      } else {
        mind->SendF("Direction '%s' not meaningful!\n", std::string(args).c_str());
        return 0;
      }

      box->SetShortDesc("a dynamic dungeon");
      next->SetShortDesc("An Entrance to a Large Mining Tunnel");
      next->SetDesc(
          "This tunnel looks to have been carved centuries ago.  It is so well crafted\n"
          "that you think it will stand as-is for another millenium.\n");
      next->SetSkill(crc32c("DynamicInit"), 1);
      next->SetSkill(crc32c("DynamicPhase"), 0); // Entrance
      next->SetSkill(crc32c("DynamicMojo"), 1000000);

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
          dir.c_str());
    }
    return 0;
  }

#define NUM_AVS 5 // 28 Max!
#define NUM_STS 5 // No Max
  if (cnum == COM_CCREATE) {
    if (!mind)
      return 0;
    return handle_command_ccreate(body, mind, args, stealth_t, stealth_s);
  }

  static Object* anchor = nullptr;

  if (cnum == COM_ANCHOR) {
    if (!mind)
      return 0;
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

  if (cnum == COM_LINK) {
    if (!mind)
      return 0;
    if (anchor == nullptr) {
      mind->Send("You need to make an anchor before you can link to it!\n");
    } else if (args.empty()) {
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
          std::string(args).c_str());

      link = new Object(body->Parent());
      link->SetShortDesc(std::string(args));
      link->AddAct(act_t::SPECIAL_LINKED, anchor);
      link->AddAct(act_t::SPECIAL_MASTER, anchor);
      link->SetSkill(crc32c("Open"), 1000);
      link->SetSkill(crc32c("Enterable"), 1);
      anchor->AddAct(act_t::SPECIAL_LINKED, link);
      anchor->AddAct(act_t::SPECIAL_MASTER, link);
      anchor->SetSkill(crc32c("Open"), 1000);
      anchor->SetSkill(crc32c("Enterable"), 1);
      std::string other = std::string(args);
      if (args == "east") {
        other = "west";
        other += args.substr(0, 4);
      } else if (args == "west") {
        other = "east";
        other += args.substr(0, 4);
      } else if (args == "north") {
        other = "south";
        other += args.substr(0, 5);
      } else if (args == "south") {
        other = "north";
        other += args.substr(0, 5);
      } else if (args == "up") {
        other = "down";
        other += args.substr(0, 2);
      } else if (args == "down") {
        other = "up";
        other += args.substr(0, 4);
      }
      anchor->SetShortDesc(other);
      anchor = nullptr;
    }
    return 0;
  }

  if (cnum == COM_DELPLAYER) {
    if (args.empty()) {
      if (mind)
        mind->Send("You want to delete which player?\n");
    } else {
      Player* pl = get_player(std::string(args));
      if (!pl) {
        if (mind)
          mind->Send("That player doesn't seem to exist.\n");
      } else {
        if (mind)
          mind->SendF("You delete the player '%s'.\n", pl->Name().c_str());
        delete pl;
      }
    }
    return 0;
  }

  if (cnum == COM_PLAYERS) {
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

  if (cnum == COM_CHARACTERS) {
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

  if (cnum == COM_RESET) {
    if (!mind)
      return 0;
    Object* targ =
        body->PickObject(std::string(args), vmode | LOC_NEARBY | LOC_INTERNAL | LOC_SELF);
    if (!targ) {
      mind->Send("You want to reset what?\n");
    } else {
      auto cont = targ->Contents();
      for (auto item : cont)
        item->Recycle();

      Object* dest = targ;
      while ((!dest->ActTarg(act_t::SPECIAL_HOME)) && dest->Parent()) {
        dest = dest->Parent();
      }
      if (dest->ActTarg(act_t::SPECIAL_HOME)) {
        dest = dest->ActTarg(act_t::SPECIAL_HOME);
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

  if (cnum == COM_MIRROR) {
    if (!mind)
      return 0;
    Object* targ = body->PickObject(std::string(args), vmode | LOC_NEARBY | LOC_INTERNAL);
    if (!targ) {
      mind->Send("You want to mirror what?\n");
    } else {
      Object* nobj = new Object(*targ);
      nobj->SetParent(targ->Parent());

      nobj->SetSkill(crc32c("Wearable on Left Arm"), targ->Skill(crc32c("Wearable on Right Arm")));
      nobj->SetSkill(
          crc32c("Wearable on Left Finger"), targ->Skill(crc32c("Wearable on Right Finger")));
      nobj->SetSkill(
          crc32c("Wearable on Left Foot"), targ->Skill(crc32c("Wearable on Right Foot")));
      nobj->SetSkill(
          crc32c("Wearable on Left Hand"), targ->Skill(crc32c("Wearable on Right Hand")));
      nobj->SetSkill(crc32c("Wearable on Left Leg"), targ->Skill(crc32c("Wearable on Right Leg")));
      nobj->SetSkill(
          crc32c("Wearable on Left Wrist"), targ->Skill(crc32c("Wearable on Right Wrist")));
      nobj->SetSkill(
          crc32c("Wearable on Left Shoulder"), targ->Skill(crc32c("Wearable on Right Shoulder")));
      nobj->SetSkill(crc32c("Wearable on Left Hip"), targ->Skill(crc32c("Wearable on Right Hip")));

      nobj->SetSkill(crc32c("Wearable on Right Arm"), targ->Skill(crc32c("Wearable on Left Arm")));
      nobj->SetSkill(
          crc32c("Wearable on Right Finger"), targ->Skill(crc32c("Wearable on Left Finger")));
      nobj->SetSkill(
          crc32c("Wearable on Right Foot"), targ->Skill(crc32c("Wearable on Left Foot")));
      nobj->SetSkill(
          crc32c("Wearable on Right Hand"), targ->Skill(crc32c("Wearable on Left Hand")));
      nobj->SetSkill(crc32c("Wearable on Right Leg"), targ->Skill(crc32c("Wearable on Left Leg")));
      nobj->SetSkill(
          crc32c("Wearable on Right Wrist"), targ->Skill(crc32c("Wearable on Left Wrist")));
      nobj->SetSkill(
          crc32c("Wearable on Right Shoulder"), targ->Skill(crc32c("Wearable on Left Shoulder")));
      nobj->SetSkill(crc32c("Wearable on Right Hip"), targ->Skill(crc32c("Wearable on Left Hip")));

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

  if (cnum == COM_CLONE) {
    if (!mind)
      return 0;
    Object* targ =
        body->PickObject(std::string(args), vmode | LOC_NEARBY | LOC_INTERNAL | LOC_SELF);
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

  if (cnum == COM_PROD) {
    if (!mind)
      return 0;
    auto targs = body->PickObjects(std::string(args), vmode | LOC_NEARBY);
    if (targs.size() == 0) {
      mind->Send("You want to prod what?\n");
      return 0;
    }
    for (auto targ : targs) {
      body->Parent()->SendOut(
          stealth_t,
          stealth_s,
          ";s prods ;s with Ninja Powers[TM].\n",
          "You prod ;s.\n",
          body,
          targ);
      targ->Activate();
    }
    return 0;
  }

  if (cnum == COM_JUNK) {
    if (!mind)
      return 0;
    auto targs = body->PickObjects(std::string(args), vmode | LOC_NEARBY);
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

  if (cnum == COM_HEAL) {
    if (!mind)
      return 0;
    Object* targ =
        body->PickObject(std::string(args), vmode | LOC_NEARBY | LOC_INTERNAL | LOC_SELF);

    int finished = 0;
    if (body->IsAct(act_t::HEAL) // Finish Previous Healing
        || body->IsUsing(crc32c("Healing")) || body->IsUsing(crc32c("First Aid")) ||
        body->IsUsing(crc32c("Treatment"))) {
      finished = 1;
      if (body->IsAct(act_t::HEAL)) {
        if (body->IsUsing(crc32c("Healing"))) {
          mind->Send("You complete your healing efforts.\n");
          int phys = body->ActTarg(act_t::HEAL)->Phys();
          phys -= body->Roll(crc32c("Healing"), phys + 2);
          if (phys < 0)
            phys = 0;
          body->ActTarg(act_t::HEAL)->SetPhys(phys);
          int pois = body->ActTarg(act_t::HEAL)->Skill(crc32c("Poisoned"));
          pois -= body->Roll(crc32c("Healing"), pois + 2);
          if (pois < 0)
            pois = 0;
          body->ActTarg(act_t::HEAL)->SetSkill(crc32c("Poisoned"), pois);
        } else if (body->IsUsing(crc32c("First Aid"))) {
          mind->Send("You complete your first-aid efforts.\n");
          int phys = body->ActTarg(act_t::HEAL)->Phys();
          phys -= body->Roll(crc32c("First Aid"), phys);
          if (phys < 0)
            phys = 0;
          body->ActTarg(act_t::HEAL)->SetPhys(phys);
        } else if (body->IsUsing(crc32c("Treatment"))) {
          mind->Send("You complete your treatment efforts.\n");
          int pois = body->ActTarg(act_t::HEAL)->Skill(crc32c("Poisoned"));
          pois -= body->Roll(crc32c("Treatment"), pois);
          if (pois < 0)
            pois = 0;
          body->ActTarg(act_t::HEAL)->SetSkill(crc32c("Poisoned"), pois);
        }
        body->StopUsing();
        body->StopAct(act_t::HEAL);
      }
    }
    if (!targ) {
      if (!finished)
        mind->Send("You want to heal what?\n");
    } else if (targ->NormAttribute(2) < 1) {
      mind->SendF("You can't heal %s, it is not alive.\n", targ->Name(0, body).c_str());
    } else if (nmode) {
      // This is ninja-healing and bypasses all healing mechanisms.
      targ->SetSkill(crc32c("Poisoned"), 0);
      targ->SetSkill(crc32c("Thirsty"), 0);
      targ->SetSkill(crc32c("Hungry"), 0);
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
        (!body->HasSkill(crc32c("Healing"))) && (!body->HasSkill(crc32c("First Aid"))) &&
        (!body->HasSkill(crc32c("Treatment")))) {
      if (mind) {
        mind->SendF("You don't know how to help %s.\n", targ->Name(0, body).c_str());
      }
    } else {
      int duration = 0;
      auto skill = crc32c("None");
      body->Parent()->SendOut(
          stealth_t, stealth_s, ";s tries to heal ;s.\n", "You try to heal ;s.\n", body, targ);
      if (body->HasSkill(crc32c("First Aid"))) {
        if (targ->Phys() < 1) {
          mind->SendF("%s is not injured.\n", targ->Name().c_str());
        } else {
          mind->SendF("%s is injured.\n", targ->Name().c_str());
          skill = crc32c("First Aid");
          duration = 3000;
        }
      } else if (body->HasSkill(crc32c("Healing"))) {
        if (targ->Phys() < 1) {
          mind->SendF("%s is not injured.\n", targ->Name().c_str());
        } else {
          mind->SendF("%s is injured.\n", targ->Name().c_str());
          skill = crc32c("Healing");
          duration = 3000;
        }
      }
      if (body->HasSkill(crc32c("Treatment"))) {
        if (targ->Skill(crc32c("Poisoned")) < 1) {
          mind->SendF("%s does not need other help.\n", targ->Name().c_str());
        } else {
          mind->SendF("%s is poisoned.\n", targ->Name().c_str());
          skill = crc32c("Treatment");
          duration = 3000;
        }
      } else if (body->HasSkill(crc32c("Healing"))) {
        if (targ->Skill(crc32c("Poisoned")) < 1) {
          mind->SendF("%s does not need other help.\n", targ->Name().c_str());
        } else {
          mind->SendF("%s is poisoned.\n", targ->Name().c_str());
          skill = crc32c("Healing");
          duration = 3000;
        }
      }
      if (skill != crc32c("None")) {
        body->AddAct(act_t::HEAL, targ);
        body->StartUsing(skill);
      }
      if (duration > 0) {
        body->BusyFor(duration, "heal");
      }
    }
    return 0;
  }

  if (cnum == COM_JACK) {
    if (!mind)
      return 0;
    Object* targ = body->ActTarg(act_t::POINT);
    if (!targ) {
      mind->Send("You need to be pointing at your target.\n");
      return 0;
    }
    int stat = 0;
    if (ascii_toupper(args[0]) == 'Q')
      stat = 1;
    if (ascii_toupper(args[0]) == 'S')
      stat = 2;
    if (ascii_toupper(args[0]) == 'C')
      stat = 3;
    if (ascii_toupper(args[0]) == 'I')
      stat = 4;
    if (ascii_toupper(args[0]) == 'W')
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
        statnames[stat].c_str());

    return 0;
  }

  if (cnum == COM_CHUMP) {
    if (!mind)
      return 0;
    Object* targ = body->ActTarg(act_t::POINT);
    if (!targ) {
      mind->Send("You need to be pointing at your target.\n");
      return 0;
    }
    int stat = 0;
    if (ascii_toupper(args[0]) == 'Q')
      stat = 1;
    if (ascii_toupper(args[0]) == 'S')
      stat = 2;
    if (ascii_toupper(args[0]) == 'C')
      stat = 3;
    if (ascii_toupper(args[0]) == 'I')
      stat = 4;
    if (ascii_toupper(args[0]) == 'W')
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
        statnames[stat].c_str());

    return 0;
  }

  if (cnum == COM_INCREMENT) {
    if (!mind)
      return 0;
    Object* targ = body->ActTarg(act_t::POINT);
    if (!targ) {
      mind->Send("You need to be pointing at your target.\n");
      return 0;
    }

    int amt = 1;
    if (isdigit(args[0])) {
      amt = atoi(std::string(args).c_str());
      auto prefix = args.find_first_not_of("0123456789");
      if (prefix == std::string::npos) {
        args = "";
      } else {
        args = args.substr(prefix);
      }
    }

    if (!is_skill(crc32c(std::string(args)))) {
      mind->SendF("Warning, '%s' is not a real skill name!\n", std::string(args).c_str());
    }

    targ->SetSkill(std::string(args), std::max(targ->Skill(crc32c(std::string(args))), 0) + amt);

    body->Parent()->SendOutF(
        stealth_t,
        stealth_s,
        ";s increments the %s of ;s with Ninja Powers[TM].\n",
        "You increment the %s of ;s.\n",
        body,
        targ,
        std::string(args).c_str());

    return 0;
  }

  if (cnum == COM_DECREMENT) {
    if (!mind)
      return 0;
    Object* targ = body->ActTarg(act_t::POINT);
    if (!targ) {
      mind->Send("You need to be pointing at your target.\n");
      return 0;
    }

    int amt = 1;
    if (isdigit(args[0])) {
      amt = atoi(std::string(args).c_str());
      auto prefix = args.find_first_not_of("0123456789");
      if (prefix == std::string::npos) {
        args = "";
      } else {
        args = args.substr(prefix);
      }
    }

    targ->SetSkill(std::string(args), targ->Skill(crc32c(std::string(args))) - amt);

    body->Parent()->SendOutF(
        stealth_t,
        stealth_s,
        ";s decrements the %s of ;s with Ninja Powers[TM].\n",
        "You decrement the %s of ;s.\n",
        body,
        targ,
        std::string(args).c_str());

    return 0;
  }

  if (cnum == COM_DOUBLE) {
    if (!mind)
      return 0;
    auto targs = body->PickObjects(std::string(args), vmode | LOC_NEARBY | LOC_INTERNAL);
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
      if (targ->Skill(crc32c("Quantity")) > 1) {
        targ->SetSkill(crc32c("Quantity"), targ->Skill(crc32c("Quantity")) * 2);
      } else {
        targ->SetSkill(crc32c("Quantity"), 2);
      }
    }
    return 0;
  }

  if (cnum == COM_TLOAD) {
    if (!mind)
      return 0;
    if (args.empty()) {
      Object* world = new Object(body->Parent());
      world->SetShortDesc("The tbaMUD World");
      world->SetSkill(crc32c("Light Source"), 1000);
      world->SetSkill(crc32c("Day Length"), 240);
      world->SetSkill(crc32c("Day Time"), 120);
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
      sprintf(buf, "tba/wld/%s.wld", std::string(args).c_str());
      body->Parent()->TBALoadWLD(buf);
      sprintf(buf, "tba/obj/%s.obj", std::string(args).c_str());
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

  if (cnum == COM_TCLEAN) {
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

  if (mind)
    mind->Send("Sorry, that command's not yet implemented.\n");
  return 0;
}

int handle_command(Object* body, const std::string& cl, Mind* mind) {
  int ret = 0;

  if (mind && !mind->SpecialPrompt().empty()) {
    std::string cmd = mind->SpecialPrompt() + " " + cl;
    ret = handle_single_command(body, cmd, mind);
    return ret;
  }

  auto start = cl.find_first_not_of("; \t\r\n");
  auto end = start;
  while (start != std::string::npos) {
    end = cl.find_first_of(";\r\n", start + 1);

    std::string_view single(cl);
    if (end != std::string::npos) {
      single = single.substr(start, end - start);
    } else {
      single = single.substr(start);
    }

    int stat = handle_single_command(body, std::string(single), mind);
    if (stat < 0)
      return stat;
    else if (ret == 0)
      ret = stat;
    if (end != std::string::npos) {
      start = cl.find_first_not_of("; \t\r\n", end + 1);
    } else {
      start = std::string::npos;
    }
  }
  return ret;
}
