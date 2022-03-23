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

#include <algorithm>
#include <string>
#include <vector>

#include "color.hpp"
#include "commands.hpp"
#include "infile.hpp"
#include "log.hpp"
#include "main.hpp"
#include "mind.hpp"
#include "net.hpp"
#include "object.hpp"
#include "properties.hpp"
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

int handle_command_ccreate(Object*, std::shared_ptr<Mind>, const std::u8string_view, int, int);
int handle_command_wload(Object*, std::shared_ptr<Mind>, const std::u8string_view, int, int);

static int count_ones(int mask) {
  int ret = 0;
  while (mask) {
    ++ret;
    mask &= (mask - 1);
  }
  return ret;
}

struct Command {
  std::u8string_view command;
  std::u8string_view shortdesc;
  std::u8string_view longdesc;
  int sit;
  com_t id;
};

constexpr Command static_comlist[COM_MAX] = {
    {u8"", u8"", u8"", 0, COM_NONE},
    {u8"help",
     u8"Get help for a topic or command.",
     u8"Get help for a topic or command.",
     (REQ_ANY),
     COM_HELP},
    {u8"quit", u8"Quit the game.", u8"Quit the game.", (REQ_ANY), COM_QUIT},

    {u8"north", u8"Travel north.", u8"Travel north.", (REQ_ALERT | REQ_UP | REQ_ACTION), COM_NORTH},
    {u8"south", u8"Travel south.", u8"Travel south.", (REQ_ALERT | REQ_UP | REQ_ACTION), COM_SOUTH},
    {u8"east", u8"Travel east.", u8"Travel east.", (REQ_ALERT | REQ_UP | REQ_ACTION), COM_EAST},
    {u8"west", u8"Travel west.", u8"Travel west.", (REQ_ALERT | REQ_UP | REQ_ACTION), COM_WEST},
    {u8"up", u8"Travel up.", u8"Travel up.", (REQ_ALERT | REQ_UP | REQ_ACTION), COM_UP},
    {u8"down", u8"Travel down.", u8"Travel down.", (REQ_ALERT | REQ_UP | REQ_ACTION), COM_DOWN},

    {u8"look",
     u8"Look around, look in a direction, or look at an object or creature.",
     u8"Look around, look in a direction, or look at an object or creature.",
     (REQ_AWAKE | REQ_ACTION | REQ_ETHEREAL),
     COM_LOOK},
    {u8"examine",
     u8"Examine an object or creature.",
     u8"Examine an object or creature.",
     (REQ_ALERT | REQ_ACTION),
     COM_EXAMINE},
    {u8"consider",
     u8"Consider attacking someone, or using something, and size it up.",
     u8"Consider attacking someone, or using something, and size it up.",
     (REQ_ALERT | REQ_ACTION),
     COM_CONSIDER},
    {u8"inventory",
     u8"Check what you have on you.",
     u8"Check what you have on you.",
     (REQ_AWAKE | REQ_ACTION),
     COM_INVENTORY},
    {u8"equipment",
     u8"Check what you are wearing and using.",
     u8"Check what you are wearing and using.",
     (REQ_AWAKE | REQ_ACTION),
     COM_EQUIPMENT},
    {u8"search",
     u8"Search an area, object or creature.",
     u8"Search an area, object or creature.",
     (REQ_ALERT | REQ_UP | REQ_ACTION),
     COM_SEARCH},
    {u8"hide",
     u8"Hide an object, or yourself.",
     u8"Hide an object, or yourself.",
     (REQ_ALERT | REQ_UP | REQ_ACTION),
     COM_HIDE},

    {u8"leave",
     u8"Leave an object.",
     u8"Leave an object.",
     (REQ_ALERT | REQ_UP | REQ_ACTION),
     COM_LEAVE},
    {u8"enter",
     u8"Enter an object (or enter the game).",
     u8"Enter an object (or enter the game).",
     (REQ_ETHEREAL | REQ_ALERT | REQ_UP | REQ_ACTION),
     COM_ENTER},
    {u8"select",
     u8"Select an unfinished character.",
     u8"Select an unfinished character.",
     (REQ_ETHEREAL),
     COM_SELECT},

    {u8"open",
     u8"Open a door or container.",
     u8"Open a door or container.",
     (REQ_ALERT | REQ_UP | REQ_ACTION),
     COM_OPEN},
    {u8"close",
     u8"Close a door or container.",
     u8"Close a door or container.",
     (REQ_ALERT | REQ_UP | REQ_ACTION),
     COM_CLOSE},
    {u8"unlock",
     u8"Unlock a door or container.",
     u8"Unlock a door or container.",
     (REQ_ALERT | REQ_UP | REQ_ACTION),
     COM_UNLOCK},
    {u8"lock",
     u8"Lock a door or container.",
     u8"Lock a door or container.",
     (REQ_ALERT | REQ_UP | REQ_ACTION),
     COM_LOCK},

    {u8"get",
     u8"Get an item from your surroundings.",
     u8"Get an item from your surroundings.",
     (REQ_ALERT | REQ_UP | REQ_ACTION),
     COM_GET},
    {u8"hold",
     u8"Hold an item you are carrying in your off-hand.",
     u8"Hold an item you are carrying in your off-hand.",
     (REQ_ALERT | REQ_ACTION),
     COM_HOLD},
    {u8"offer",
     u8"Offer to give your held item to someone, or stop offering.",
     u8"Offer to give your held item to someone, or stop offering.",
     (REQ_ALERT | REQ_ACTION),
     COM_OFFER},

    {u8"drag",
     u8"Drag a heavy item with you when you next move.",
     u8"Drag a heavy item with you when you next move.",
     (REQ_ALERT | REQ_STAND | REQ_ACTION),
     COM_DRAG},
    {u8"put",
     u8"Put a held item in or on something.",
     u8"Put a held item in or on something.",
     (REQ_ALERT | REQ_UP | REQ_ACTION),
     COM_PUT},
    {u8"drop",
     u8"Drop an item you are carrying.",
     u8"Drop an item you are carrying.",
     (REQ_ALERT),
     COM_DROP},
    {u8"stash",
     u8"Store an item you are holding in one of your containers.",
     u8"Store an item you are holding in one of your containers.",
     (REQ_ALERT | REQ_ACTION),
     COM_STASH},
    {u8"wield",
     u8"Wield a weapon you are carrying.",
     u8"Wield a weapon you are carrying.",
     (REQ_ALERT | REQ_ACTION),
     COM_WIELD},
    {u8"unwield",
     u8"Unwield the weapon you are currently wielding.",
     u8"Unwield the weapon you are currently wielding.",
     (REQ_ALERT | REQ_ACTION),
     COM_UNWIELD},
    {u8"light",
     u8"Light an item you are carrying and hold it in your off-hand.",
     u8"Light an item you are carrying and hold it in your off-hand.",
     (REQ_ALERT | REQ_ACTION),
     COM_LIGHT},
    {u8"wear",
     u8"Wear an item you are carrying.",
     u8"Wear an item you are carrying.",
     (REQ_ALERT | REQ_ACTION),
     COM_WEAR},
    {u8"remove",
     u8"Remove an item you are wearing.",
     u8"Remove an item you are wearing.",
     (REQ_ALERT | REQ_ACTION),
     COM_REMOVE},
    {u8"label",
     u8"Label, or read the label of, an item you are holding.",
     u8"Label, or read the label of, an item you are holding.",
     (REQ_ALERT | REQ_ACTION | CMD_FLAVORTEXT),
     COM_LABEL},
    {u8"unlabel",
     u8"Remove the label the item you are holding.",
     u8"Remove the label the item you are holding.",
     (REQ_ALERT | REQ_ACTION),
     COM_UNLABEL},
    {u8"heal",
     u8"Use healing/first-aid skills to help another, or yourself.",
     u8"Use healing/first-aid skills to help another, or yourself.",
     (REQ_CORPOREAL | REQ_ACTION),
     COM_HEAL},

    {u8"sleep", u8"Go to sleep.", u8"Go to sleep.", (REQ_CONSCIOUS | REQ_ACTION), COM_SLEEP},
    {u8"wake",
     u8"Wake up, or wake someone else up.",
     u8"Wake up, or wake someone else up.",
     (REQ_CONSCIOUS | REQ_ACTION),
     COM_WAKE},
    {u8"lie", u8"Lie down.", u8"Lie down.", (REQ_CONSCIOUS | REQ_ACTION), COM_LIE},
    {u8"rest", u8"Relax and rest.", u8"Relax and rest.", (REQ_CONSCIOUS | REQ_ACTION), COM_REST},
    {u8"sit", u8"Sit down.", u8"Sit down.", (REQ_CONSCIOUS | REQ_ACTION), COM_SIT},
    {u8"stand", u8"Stand up.", u8"Stand up.", (REQ_CONSCIOUS | REQ_ACTION), COM_STAND},
    {u8"use",
     u8"Start, or stop using a skill.",
     u8"Start, or stop using a skill.",
     (REQ_ALERT | REQ_UP | REQ_ACTION),
     COM_USE},
    {u8"stop",
     u8"Stop using a skill.",
     u8"Stop using a skill.",
     (REQ_ALERT | REQ_UP | REQ_ACTION),
     COM_STOP},
    {u8"cast", u8"Cast a spell.", u8"Cast a spell.", (REQ_ALERT | REQ_UP | REQ_ACTION), COM_CAST},
    {u8"pray",
     u8"Use faith to enhance another command.",
     u8"Use faith to enhance another command.",
     (REQ_ALERT | REQ_UP | REQ_ACTION),
     COM_PRAY},

    {u8"shout",
     u8"Shout something to all nearby.",
     u8"Shout something to all nearby.",
     (REQ_AWAKE | CMD_FLAVORTEXT),
     COM_SHOUT},
    {u8"yell",
     u8"Shout something to all nearby.",
     u8"Shout something to all nearby.",
     (REQ_AWAKE | CMD_FLAVORTEXT),
     COM_YELL},
    {u8"call",
     u8"Shout something to all nearby.",
     u8"Shout something to all nearby.",
     (REQ_AWAKE | CMD_FLAVORTEXT),
     COM_CALL},
    {u8"say", u8"Say something.", u8"Say something.", (REQ_AWAKE | CMD_FLAVORTEXT), COM_SAY},
    {u8"emote",
     u8"Indicate to others that you are doing something.",
     u8"Indicate to others that you are doing something.",
     (REQ_AWAKE | CMD_FLAVORTEXT),
     COM_EMOTE},
    {u8"introduce",
     u8"Indroduce yourself, or someone or something else, to others.",
     u8"Indroduce yourself, or someone or something else, to others.",
     (REQ_AWAKE | CMD_FLAVORTEXT),
     COM_INTRODUCE},

    {u8"eat",
     u8"Eat an item you are carrying.",
     u8"Eat an item you are carrying.",
     (REQ_ALERT | REQ_ACTION),
     COM_EAT},
    {u8"drink",
     u8"Drink from an item you are carrying.",
     u8"Drink from an item you are carrying.",
     (REQ_ALERT | REQ_ACTION),
     COM_DRINK},
    {u8"fill",
     u8"Fill a held liquid container from another.",
     u8"Fill a held liquid container from another.",
     (REQ_ALERT | REQ_ACTION),
     COM_FILL},
    {u8"dump",
     u8"Dump all liquid out of a container you are carrying.",
     u8"Dump all liquid out of a container you are carrying.",
     (REQ_ALERT | REQ_ACTION),
     COM_DUMP},

    {u8"point",
     u8"Point at an object, or stop pointing.",
     u8"Point at an object, or stop pointing.",
     (REQ_ALERT),
     COM_POINT},
    {u8"follow",
     u8"Follow someone, or stop following someone.",
     u8"Follow someone, or stop following someone.",
     (REQ_ALERT | REQ_UP),
     COM_FOLLOW},
    {u8"attack",
     u8"Attack somebody, or an object.",
     u8"Attack somebody, or an object.",
     (REQ_ALERT | REQ_ACTION | REQ_STAND),
     COM_ATTACK},
    {u8"kill",
     u8"Try to kill somebody, or smash an object.",
     u8"Try to kill somebody, or smash an object.",
     (REQ_ALERT | REQ_ACTION | REQ_STAND),
     COM_KILL},
    {u8"punch",
     u8"Punch somebody, or an object.",
     u8"Punch somebody, or an object.",
     (REQ_ALERT | REQ_ACTION | REQ_STAND),
     COM_PUNCH},
    {u8"kick",
     u8"Kick somebody, or an object.",
     u8"Kick somebody, or an object.",
     (REQ_ALERT | REQ_ACTION | REQ_STAND),
     COM_KICK},
    {u8"flee",
     u8"Escape the area as fast as you can, in a random direction.",
     u8"Escape the area as fast as you can, in a random direction.",
     (REQ_ALERT | REQ_ACTION | REQ_UP),
     COM_FLEE},

    {u8"list",
     u8"List items available at a shop.",
     u8"List items available at a shop.",
     (REQ_ALERT | REQ_ACTION),
     COM_LIST},
    {u8"buy",
     u8"Buy an item at a shop.",
     u8"Buy an item at a shop.",
     (REQ_ALERT | REQ_ACTION),
     COM_BUY},
    {u8"value",
     u8"Find out how much a shop will give you for an item.",
     u8"Find out how much a shop will give you for an item.",
     (REQ_ALERT | REQ_ACTION),
     COM_VALUE},
    {u8"sell",
     u8"Sell an item at a shop.",
     u8"Sell an item at a shop.",
     (REQ_ALERT | REQ_ACTION),
     COM_SELL},

    {u8"toggle",
     u8"Turn options on and off, or show current options setting.",
     u8"Turn options on and off, or show current options setting.",
     (REQ_ANY),
     COM_TOGGLE},

    {u8"who",
     u8"Get a list of who is on the MUD right now.",
     u8"Get a list of who is on the MUD right now.",
     (REQ_ANY),
     COM_WHO},
    {u8"ooc",
     u8"Toggle or use ooc (Out-Of-Character) chat.",
     u8"Toggle or use ooc (Out-Of-Character) chat.",
     (REQ_ANY | CMD_FLAVORTEXT),
     COM_OOC},
    {u8"newbie",
     u8"Toggle or use newbie (new player) chat (use this to ask for help).",
     u8"Toggle or use newbie (new player) chat (use this to ask for help).",
     (REQ_ANY | CMD_FLAVORTEXT),
     COM_NEWBIE},

    {u8"newcharacter",
     u8"Create a new character.",
     u8"Create a new character.",
     (REQ_ETHEREAL | CMD_FLAVORTEXT),
     COM_NEWCHARACTER},
    {u8"raise",
     u8"Spend a skill or attribute point of current character.",
     u8"Spend a skill or attribute point of current character.",
     (REQ_ETHEREAL | REQ_CORPOREAL),
     COM_RAISE},
    {u8"lower",
     u8"Lower a skill or attribute point of current in-progress character.",
     u8"Lower a skill or attribute point of current in-progress character.",
     (REQ_ETHEREAL),
     COM_LOWER},
    {u8"resetcharacter",
     u8"Undo (erase!) *all* work on in-progress character.",
     u8"Undo (erase!) *all* work on in-progress character.",
     (REQ_ETHEREAL),
     COM_RESETCHARACTER},
    {u8"randomize",
     u8"Spend all remaining points of current character randomly.",
     u8"Spend all remaining points of current character randomly.",
     (REQ_ETHEREAL),
     COM_RANDOMIZE},
    {u8"archetype",
     u8"Replace (erase!) *all* work on in-progress character with preset.",
     u8"Replace (erase!) *all* work on in-progress character with preset.",
     (REQ_ETHEREAL),
     COM_ARCHETYPE},
    {u8"score",
     u8"Get your current character and/or player's stats and score.",
     u8"Get your current character and/or player's stats and score.",
     (REQ_ETHEREAL | REQ_CORPOREAL),
     COM_SCORE},

    {u8"time",
     u8"Get the current world's MUD time.",
     u8"Get the current world's MUD time.",
     (REQ_ETHEREAL | REQ_CORPOREAL),
     COM_TIME},
    {u8"world",
     u8"Get the name of the current world.",
     u8"Get the name of the current world.",
     (REQ_ETHEREAL | REQ_CORPOREAL),
     COM_WORLD},
    {u8"zone",
     u8"Get the name of the current zone.",
     u8"Get the name of the current zone.",
     (REQ_CORPOREAL),
     COM_ZONE},
    {u8"version",
     u8"Query the version information of running AcidMUD.",
     u8"Query the version information of running AcidMUD.",
     (REQ_ANY),
     COM_VERSION},

    {u8"skilllist",
     u8"List all available skill categories, or all skills in a category.",
     u8"List all available skill categories, or all skills in a category.",
     (REQ_ANY),
     COM_SKILLLIST},

    {u8"recall",
     u8"Teleport back to the start when you are uninjured.",
     u8"Teleport back to the start when you are uninjured.",
     (REQ_STAND),
     COM_RECALL},
    {u8"teleport",
     u8"Teleport to a named location (requires a power to enable).",
     u8"Teleport to a named location (requires a power to enable).",
     (REQ_STAND),
     COM_TELEPORT},
    {u8"resurrect",
     u8"Resurrect a long-dead character (one with no corpse left).",
     u8"Resurrect a long-dead character (one with no corpse left).",
     (REQ_STAND),
     COM_RESURRECT},

    {u8"ninjamode",
     u8"Ninja command: run a command in Ninja Mode[TM].",
     u8"Ninja command: run a command in Ninja Mode[TM].",
     (REQ_ETHEREAL | REQ_ANY | REQ_ACTION | REQ_NINJA | CMD_FLAVORTEXT),
     COM_NINJAMODE},
    {u8"makeninja",
     u8"Ninja command: make (or unmake) another player into a True Ninja[TM].",
     u8"Ninja command: make (or unmake) another player into a True Ninja[TM].",
     (REQ_ANY | REQ_SUPERNINJA | REQ_NINJAMODE),
     COM_MAKENINJA},
    {u8"makesuperninja",
     u8"Ninja command: make (or unmake) a True Ninja into a Super Ninja[TM].",
     u8"Ninja command: make (or unmake) a True Ninja into a Super Ninja[TM].",
     (REQ_ANY | REQ_SUPERNINJA | REQ_NINJAMODE),
     COM_MAKESUPERNINJA},

    {u8"create",
     u8"Ninja command.",
     u8"Ninja command - ninjas only!",
     (REQ_ALERT | REQ_NINJAMODE | CMD_FLAVORTEXT),
     COM_CREATE},
    {u8"dcreate",
     u8"Ninja command.",
     u8"Ninja command - ninjas only!",
     (REQ_ALERT | REQ_NINJAMODE),
     COM_DCREATE},
    {u8"ccreate",
     u8"Ninja command.",
     u8"Ninja command - ninjas only!",
     (REQ_ALERT | REQ_NINJAMODE),
     COM_CCREATE},
    {u8"wload",
     u8"Ninja command.",
     u8"Ninja command - ninjas only!",
     (REQ_ALERT | REQ_NINJAMODE),
     COM_WLOAD},
    {u8"anchor",
     u8"Ninja command.",
     u8"Ninja command - ninjas only!",
     (REQ_ALERT | REQ_NINJAMODE),
     COM_ANCHOR},
    {u8"link",
     u8"Ninja command.",
     u8"Ninja command - ninjas only!",
     (REQ_ALERT | REQ_NINJAMODE),
     COM_LINK},
    {u8"connect",
     u8"Ninja command.",
     u8"Ninja command - ninjas only!",
     (REQ_ALERT | REQ_NINJAMODE),
     COM_CONNECT},
    {u8"command",
     u8"Ninja command.",
     u8"Ninja command - ninjas only!",
     (REQ_CORPOREAL | REQ_NINJAMODE | CMD_FLAVORTEXT),
     COM_COMMAND},
    {u8"control",
     u8"Ninja command.",
     u8"Ninja command - ninjas only!",
     (REQ_CORPOREAL | REQ_NINJAMODE),
     COM_CONTROL},
    {u8"clone",
     u8"Ninja command.",
     u8"Ninja command - ninjas only!",
     (REQ_ALERT | REQ_NINJAMODE),
     COM_CLONE},
    {u8"mirror",
     u8"Ninja command.",
     u8"Ninja command - ninjas only!",
     (REQ_ALERT | REQ_NINJAMODE),
     COM_MIRROR},
    {u8"junk",
     u8"Ninja command.",
     u8"Ninja command - ninjas only!",
     (REQ_ALERT | REQ_NINJAMODE),
     COM_JUNK},
    {u8"prod",
     u8"Ninja command.",
     u8"Ninja command - ninjas only!",
     (REQ_ALERT | REQ_NINJAMODE),
     COM_PROD},
    {u8"reset",
     u8"Ninja command.",
     u8"Ninja command - ninjas only!",
     (REQ_ALERT | REQ_NINJAMODE),
     COM_RESET},

    {u8"players",
     u8"Ninja command.",
     u8"Ninja command - ninjas only!",
     (REQ_ANY | REQ_NINJAMODE),
     COM_PLAYERS},
    {u8"delplayer",
     u8"Ninja command.",
     u8"Ninja command - ninjas only!",
     (REQ_ANY | REQ_NINJAMODE),
     COM_DELPLAYER},
    {u8"characters",
     u8"Ninja command.",
     u8"Ninja command - ninjas only!",
     (REQ_ANY | REQ_NINJAMODE),
     COM_CHARACTERS},

    {u8"jack",
     u8"Ninja command.",
     u8"Ninja command - ninjas only!",
     (REQ_ALERT | REQ_NINJAMODE),
     COM_JACK},
    {u8"chump",
     u8"Ninja command.",
     u8"Ninja command - ninjas only!",
     (REQ_ALERT | REQ_NINJAMODE),
     COM_CHUMP},
    {u8"increment",
     u8"Ninja command.",
     u8"Ninja command - ninjas only!",
     (REQ_ALERT | REQ_NINJAMODE | CMD_FLAVORTEXT),
     COM_INCREMENT},
    {u8"decrement",
     u8"Ninja command.",
     u8"Ninja command - ninjas only!",
     (REQ_ALERT | REQ_NINJAMODE | CMD_FLAVORTEXT),
     COM_DECREMENT},
    {u8"double",
     u8"Ninja command.",
     u8"Ninja command - ninjas only!",
     (REQ_ALERT | REQ_NINJAMODE),
     COM_DOUBLE},

    {u8"setstats",
     u8"Ninja command.",
     u8"Ninja command - ninjas only!",
     (REQ_ALERT | REQ_NINJAMODE),
     COM_SETSTATS},
    {u8"brief",
     u8"Ninja command.",
     u8"Ninja command - ninjas only!",
     (REQ_ALERT | REQ_NINJAMODE | CMD_FLAVORTEXT),
     COM_BRIEF},
    {u8"name",
     u8"Ninja command.",
     u8"Ninja command - ninjas only!",
     (REQ_ALERT | REQ_NINJAMODE | CMD_FLAVORTEXT),
     COM_NAME},
    {u8"unname",
     u8"Ninja command.",
     u8"Ninja command - ninjas only!",
     (REQ_ALERT | REQ_NINJAMODE | CMD_FLAVORTEXT),
     COM_UNNAME},
    {u8"undescribe",
     u8"Ninja command.",
     u8"Ninja command - ninjas only!",
     (REQ_ALERT | REQ_NINJAMODE),
     COM_UNDESCRIBE},
    {u8"describe",
     u8"Ninja command.",
     u8"Ninja command - ninjas only!",
     (REQ_ALERT | REQ_NINJAMODE | CMD_FLAVORTEXT),
     COM_DESCRIBE},
    {u8"undefine",
     u8"Ninja command.",
     u8"Ninja command - ninjas only!",
     (REQ_ALERT | REQ_NINJAMODE),
     COM_UNDEFINE},
    {u8"define",
     u8"Ninja command.",
     u8"Ninja command - ninjas only!",
     (REQ_ALERT | REQ_NINJAMODE | CMD_FLAVORTEXT),
     COM_DEFINE},

    {u8"stats",
     u8"Ninja command: Get stats of a character, object or creature.",
     u8"Ninja command: Get stats of a character, object or creature.",
     (REQ_ALERT | REQ_NINJAMODE),
     COM_STATS},

    {u8"shutdown",
     u8"Ninja command: shutdown the entire MUD.",
     u8"Ninja command: shutdown the entire MUD.",
     (REQ_ANY | REQ_NINJAMODE),
     COM_SHUTDOWN},
    {u8"restart",
     u8"Ninja command: restart the entire MUD - preserving connections.",
     u8"Ninja command: restart the entire MUD - preserving connections.",
     (REQ_ANY | REQ_NINJAMODE),
     COM_RESTART},
    {u8"saveall",
     u8"Ninja command: save the entire MUD Universe - preserving connections.",
     u8"Ninja command: save the entire MUD Universe - preserving connections.",
     (REQ_ANY | REQ_NINJAMODE),
     COM_SAVEALL},

    {u8"makestart",
     u8"Ninja command.",
     u8"Ninja command - ninjas only!",
     (REQ_ALERT | REQ_NINJAMODE),
     COM_MAKESTART},

    {u8"tload",
     u8"Ninja command.",
     u8"Ninja command - ninjas only!",
     (REQ_ALERT | REQ_NINJAMODE | CMD_FLAVORTEXT),
     COM_TLOAD},
    {u8"tclean",
     u8"Ninja command.",
     u8"Ninja command - ninjas only!",
     (REQ_ALERT | REQ_NINJAMODE),
     COM_TCLEAN},
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
static_assert(static_comlist[COM_INTRODUCE].id == COM_INTRODUCE);
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
static_assert(static_comlist[COM_ZONE].id == COM_ZONE);
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
static_assert(static_comlist[COM_WLOAD].id == COM_WLOAD);
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
static_assert(static_comlist[COM_BRIEF].id == COM_BRIEF);
static_assert(static_comlist[COM_NAME].id == COM_NAME);
static_assert(static_comlist[COM_UNNAME].id == COM_UNNAME);
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

static std::u8string soc_com[1024] = {};
static std::u8string socials[1024][13] = {};

static void load_commands() {
  int cnum = 1;
  while (cnum < COM_MAX) {
    comlist[cnum] = static_comlist[cnum];
    ++cnum;
  }
  infile soc(u8"tba/socials.new");
  if (soc) {
    // loge(u8"There were {} commands!\n", cnum);
    while (soc.length() > 0 && nextchar(soc) == '~') {
      std::u8string_view com = getuntil(soc, ' ');
      skipspace(soc);
      getuntil(soc, ' '); // Skip abbreviation - automatically determined by AcidMUD
      skipspace(soc);
      nextnum(soc); // v1 - ignored in AcidMUD
      skipspace(soc);
      nextnum(soc); // v2 - ignored in AcidMUD
      skipspace(soc);
      nextnum(soc); // v3 - ignored in AcidMUD
      skipspace(soc);
      nextnum(soc); // v4 - ignored in AcidMUD
      skipspace(soc);

      soc_com[cnum] = com;
      for (int mnum = 0; mnum < 13; ++mnum) {
        std::u8string_view line = getuntil(soc, '\n');
        if (line.contains('#')) {
          socials[cnum][mnum] = u8"";
        } else {
          socials[cnum][mnum] = line;
        }
      }

      comlist[cnum].command = soc_com[cnum];
      comlist[cnum].shortdesc = u8"Social command.";
      comlist[cnum].longdesc = u8"Social command.";
      comlist[cnum].sit = (REQ_ALERT | CMD_FLAVORTEXT); // FIXME: Import This?
      comlist[cnum].id = COM_SOCIAL;
      ++cnum;

      skipspace(soc); // Eat blank line before next social command, or the terminating '$'
    }
    // loge(u8"There are now {} commands!\n", cnum);
  }
}

com_t identify_command(const std::u8string_view str, bool corporeal) {
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
    if (comlist[ctr].id == COM_DUMP &&
        str == std::u8string_view(u8"empty").substr(0, str.length())) {
      return comlist[ctr].id;
    }
    if (comlist[ctr].id == COM_GET && str == std::u8string_view(u8"take").substr(0, str.length())) {
      return comlist[ctr].id;
    }
  }
  return COM_NONE;
}

// Return values: -1: Player D/Ced
//                0: Command Understood
//                1: Command NOT Understood
//                2: Command Understood - No More Actions This Round
static int handle_single_command(Object* body, std::u8string line, std::shared_ptr<Mind> mind) {
  if (comlist[COM_MAX].id == COM_NONE) { // Haven't loaded commands yet
    load_commands();
  }

  std::u8string_view cmd = line;
  std::u8string_view args = line;
  std::u8string args_buf;

  auto c1 = cmd.find_first_not_of(u8" \t\n\r;");
  if (c1 == std::u8string::npos) {
    return 1;
  }

  auto c2 = cmd.find_first_of(u8" \t\n\r;", c1 + 1);
  if (c2 == std::u8string::npos) {
    cmd = cmd.substr(c1);
  } else {
    cmd = cmd.substr(c1, c2 - c1);
  }

  // Lowercase the command portion, and only that portion, for now.
  auto clen = cmd.find_first_not_of(u8"abcdefghijklmnopqrstuvwxyz");
  if (clen != std::u8string::npos && clen < cmd.length() && ascii_isupper(cmd[clen])) {
    if (c2 == std::u8string::npos) {
      std::transform(line.begin(), line.end(), line.begin(), ascii_tolower);
    } else {
      std::transform(line.begin(), line.begin() + c2 + 1, line.begin(), ascii_tolower);
    }
  }

  if (comlist[COM_MAX].id == COM_NONE) { // Haven't loaded commands yet
    load_commands();
  }

  if ((!body) && (!mind)) { // Nobody doing something?
    loge(u8"Warning: absolutely nobody tried to '{}'.\n", line);
    return 0;
  }

  if (cmd.empty() || cmd.front() == '#')
    return 0;

  if (mind && mind->Type() == mind_t::REMOTE && (!mind->Owner())) {
    if (mind->PName() != u8"")
      mind->SetPPass(std::u8string(cmd));
    else {
      int ctr;
      for (ctr = 0; ctr < int(cmd.length()); ++ctr) {
        if (!(ascii_isalnum(cmd[ctr]) || cmd[ctr] == ' ')) {
          mind->Send(
              u8"Name '{}' is invalid.\nNames can only have letters, numbers, and spaces.\n", cmd);
          loge(u8"Name '{}' is invalid.\nNames can only have letters, numbers, and spaces.\n", cmd);
          break;
        }
      }
      if (ctr == int(cmd.length()))
        mind->SetPName(std::u8string(cmd));
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
  if (body && body->Power(prhash(u8"Dark Vision"))) {
    vmode |= LOC_DARK;
  }
  if (body && body->Power(prhash(u8"Heat Vision"))) {
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
          cmd == std::u8string_view(u8"chars").substr(0, cmd.length())) {
        cnum = ctr;
        break;
      }
    }
  }

  // Lowercase the entire command, for non-flavortext commands.
  if (cnum != COM_NONE && (comlist[cnum].sit & CMD_FLAVORTEXT) == 0) {
    auto fclen = line.find_first_of(u8"ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    if (fclen != std::u8string::npos && fclen < line.length()) {
      std::transform(line.begin(), line.end(), line.begin(), ascii_tolower);
    }
  }

  if (c2 == std::u8string::npos) {
    args = u8"";
  } else {
    auto a1 = args.find_first_not_of(u8" \t\n\r;", c2 + 1);
    if (a1 == std::u8string::npos) {
      args = u8"";
    } else {
      args = args.substr(a1);
    }
  }

  if ((!nmode) && cnum != COM_RECALL && body && body->Parent()) {
    DArr64<Object*> items;
    Object* room = body->PickObject(u8"here", LOC_HERE);
    auto items2 = body->PickObjects(u8"everything", LOC_INTERNAL | LOC_NEARBY);
    auto items3 = body->PickObjects(u8"everyone", LOC_NEARBY);
    items.reserve(items2.size() + items3.size() + (room != nullptr));
    if (room != nullptr)
      items.push_back(room);
    items.insert(items.end(), items2.begin(), items2.end());
    items.insert(items.end(), items3.begin(), items3.end());

    for (auto obj : items) {
      auto trigs = obj->PickObjects(u8"all tbamud trigger script", LOC_NINJA | LOC_INTERNAL);
      for (auto trig : trigs) {
        if (trig->Skill(prhash(u8"TBAScriptType")) & 0x04) { //*-COMMAND trigs
          if ((cnum == COM_NONE &&
               cmd == std::u8string_view(trig->Desc()).substr(0, cmd.length())) ||
              (cnum && cnum == identify_command(trig->Desc(), true))) {
            if (trig->Skill(prhash(u8"TBAScriptType")) & 0x2000000) { // OBJ
              int narg = trig->Skill(prhash(u8"TBAScriptNArg"));
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
            std::u8string cmln;
            if (cnum != COM_NONE) {
              cmln = fmt::format(u8"{} {}", comlist[cnum].command, args);
            } else {
              cmln = fmt::format(u8"{} {}", trig->Desc(), args);
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
      mind->Send(u8"Command NOT understood - type 'help' for assistance.\n");
    return 1;
  }

  if ((!sninja) && (comlist[cnum].sit & SIT_SUPERNINJA)) {
    if (mind)
      mind->Send(u8"Sorry, that command is for Super Ninjas only!\n");
    return 0;
  }

  if ((!ninja) && (comlist[cnum].sit & SIT_NINJA)) {
    if (mind)
      mind->Send(u8"Sorry, that command is for True Ninjas only!\n");
    return 0;
  }

  if ((!nmode) && (comlist[cnum].sit & SIT_NINJAMODE)) {
    if (mind && ninja)
      mind->Send(u8"Sorry, you need to be in Ninja Mode[TM] to do that.\n");
    else if (mind)
      mind->Send(u8"Sorry, that command is for Ninjas only!\n");
    return 0;
  }

  if ((!(comlist[cnum].sit & SIT_ETHEREAL)) && (!body)) {
    if (mind)
      mind->Send(
          u8"You can't use that command until you join the game - with "
          u8"the 'enter' command.\n");
    return 0;
  }

  if ((!(comlist[cnum].sit & SIT_CORPOREAL)) && (body)) {
    if (mind)
      mind->Send(
          u8"You can't use that command until you quit the game - with "
          u8"the 'quit' command.\n");
    return 0;
  }

  if (body) {
    if (body->StillBusy() && (comlist[cnum].sit & SIT_ACTION)) {
      body->DoWhenFree(std::u8string(comlist[cnum].command) + u8" " + std::u8string(args));
      return 0;
    }
    if (comlist[cnum].sit & (SIT_ALIVE | SIT_AWAKE | SIT_ALERT)) {
      if (body->IsAct(act_t::DYING) || body->IsAct(act_t::DEAD)) {
        if (mind)
          mind->Send(u8"You must be alive to use that command.\n");
        return 0;
      }
    }
    if (body && (comlist[cnum].sit & SIT_CONSCIOUS)) {
      if (body->IsAct(act_t::UNCONSCIOUS)) {
        if (mind)
          mind->Send(u8"You can't do that, you are out cold.\n");
        return 0;
      }
    }
    if ((comlist[cnum].sit & (SIT_STAND | SIT_USE)) == (SIT_STAND | SIT_USE)) {
      if (body->Pos() != pos_t::STAND && body->Pos() != pos_t::USE) {
        if (mind)
          mind->Send(u8"You must stand up to use that command.\n");
        handle_single_command(body, u8"stand", mind);
        if (body->Pos() != pos_t::STAND && body->Pos() != pos_t::USE)
          return 0;
      }
    } else if ((comlist[cnum].sit & (SIT_STAND | SIT_SIT)) == (SIT_STAND | SIT_SIT)) {
      if (body->Pos() == pos_t::USE) {
        if (mind)
          mind->Send(u8"You must stop using this skill to do that.\n");
        handle_single_command(body, u8"stop", mind);
        if (body->Pos() != pos_t::STAND)
          return 0;
      } else if (body->Pos() != pos_t::SIT && body->Pos() != pos_t::STAND) {
        if (mind)
          mind->Send(u8"You must at least sit up to use that command.\n");
        handle_single_command(body, u8"sit", mind);
        if (body->Pos() != pos_t::SIT && body->Pos() != pos_t::STAND)
          return 0;
      }
    } else if (comlist[cnum].sit & SIT_STAND) {
      if (body->Pos() == pos_t::USE) {
        if (mind)
          mind->Send(u8"You must stop using this skill to do that.\n");
        handle_single_command(body, u8"stop", mind);
        if (body->Pos() != pos_t::STAND)
          return 0;
      } else if (body->Pos() != pos_t::STAND) {
        if (mind)
          mind->Send(u8"You must stand up to use that command.\n");
        handle_single_command(body, u8"stand", mind);
        if (body->Pos() != pos_t::STAND)
          return 0;
      }
    }
    if (comlist[cnum].sit & SIT_SIT) {
      if (body->Pos() != pos_t::SIT) {
        if (mind)
          mind->Send(u8"You must sit to use that command.\n");
        handle_single_command(body, u8"sit", mind);
        if (body->Pos() != pos_t::SIT)
          return 0;
      }
    }
    if (comlist[cnum].sit & SIT_ALERT) {
      if (body->IsAct(act_t::SLEEP)) {
        if (mind)
          mind->Send(u8"You must be awake to use that command.\n");
        handle_single_command(body, u8"wake", mind);
        if (body->IsAct(act_t::SLEEP))
          return 0;
      }
      if (body->IsAct(act_t::REST)) {
        if (mind)
          mind->Send(u8"You must be alert to use that command.\n");
        handle_single_command(body, u8"rest", mind);
        if (body->IsAct(act_t::REST))
          return 0;
      }
    }
    if (comlist[cnum].sit & SIT_AWAKE) {
      if (body->IsAct(act_t::SLEEP)) {
        if (mind)
          mind->Send(u8"You must be awake to use that command.\n");
        handle_single_command(body, u8"wake", mind);
        if (body->IsAct(act_t::SLEEP))
          return 0;
      }
    }
  }

  int stealth_t = 0, stealth_s = 0;

  if (body && body->IsUsing(prhash(u8"Stealth")) && body->Skill(prhash(u8"Stealth")) > 0) {
    stealth_t = body->Skill(prhash(u8"Stealth"));
    stealth_s = body->Roll(prhash(u8"Stealth"), 2);
  }

  if (cnum == COM_VERSION) {
    if (mind)
      mind->Send(
          u8"Version of this MUD is {}.{}.{}-{}-{}: {}.\n",
          CurrentVersion.acidmud_version[0],
          CurrentVersion.acidmud_version[1],
          CurrentVersion.acidmud_version[2],
          CurrentVersion.acidmud_git_revs,
          CurrentVersion.acidmud_git_hash,
          CurrentVersion.acidmud_datestamp);
    return 0;
  }

  if (cnum == COM_SHUTDOWN) {
    shutdn = 1;
    if (mind)
      mind->Send(u8"You instruct the system to shut down.\n");
    return 0;
  }

  if (cnum == COM_RESTART) {
    shutdn = 2;
    if (mind)
      mind->Send(u8"You instruct the system to restart.\n");
    return 0;
  }

  if (cnum == COM_SAVEALL) {
    shutdn = -1;
    if (mind)
      mind->Send(u8"You instruct the system to save all.\n");
    return 0;
  }

  if (cnum == COM_SELECT) {
    Object* sel =
        mind->Owner()->Room()->PickObject(std::u8string(args), vmode | LOC_INTERNAL | LOC_NINJA);
    if (!sel) {
      mind->Send(
          u8"Sorry, that character doesn't exist.\n"
          u8"Use the 'newcharacter' command to create a new character.\n");
      return 0;
    } else {
      mind->Send(u8"'{}' is now selected as your currect character to work on.\n", sel->Name());
      mind->Owner()->SetCreator(sel);
      return 0;
    }
  }

  if (cnum == COM_NORTH) {
    cnum = COM_ENTER;
    args = u8"north";
  }
  if (cnum == COM_SOUTH) {
    cnum = COM_ENTER;
    args = u8"south";
  }
  if (cnum == COM_EAST) {
    cnum = COM_ENTER;
    args = u8"east";
  }
  if (cnum == COM_WEST) {
    cnum = COM_ENTER;
    args = u8"west";
  }
  if (cnum == COM_UP) {
    cnum = COM_ENTER;
    args = u8"up";
  }
  if (cnum == COM_DOWN) {
    cnum = COM_ENTER;
    args = u8"down";
  }

  if (cnum == COM_FLEE) {
    auto dirs = body->PickObjects(u8"everywhere", vmode | LOC_NEARBY);
    dirs.erase(
        std::remove_if(
            dirs.begin(),
            dirs.end(),
            [](const Object* o) { return o->Skill(prhash(u8"Open")) < 1; }),
        dirs.end());
    if (dirs.size() < 1) {
      if (mind)
        mind->Send(u8"There is nowhere go, you can't flee!\n");
      return 0;
    }

    body->StartUsing(prhash(u8"Sprinting"));
    body->ClearSkill(prhash(u8"Hidden"));

    auto dir = dirs.begin();
    int sel = rand() % dirs.size();
    while (sel > 0) {
      ++dir;
      --sel;
    }
    if (mind)
      mind->Send(u8"You try to flee {}.\n", (*dir)->ShortDesc());

    cnum = COM_ENTER;
    args_buf = (*dir)->ShortDesc();
    args = args_buf;
  }

  if (cnum == COM_ENTER) {
    if (!body) { // Implies that there is a u8"mind"
      if (args.empty()) {
        mind->Send(u8"Enter which character?  Use 'enter <charname>'.\n");
        return 0;
      }
      if (!mind->Owner()) { // The Autoninja (Initial Startup)
        Object* god = new_body(Object::Universe());
        god->SetDescs(u8"a metaphysical being", std::u8string(args), u8"", u8"");
        god->Attach(mind);
        return 0;
      }

      body =
          mind->Owner()->Room()->PickObject(std::u8string(args), vmode | LOC_INTERNAL | LOC_NINJA);
      if (!body) {
        mind->Send(
            u8"Sorry, that character doesn't exist.\n"
            u8"Use the 'newcharacter' command to create a new character.\n");
        return 0;
      }
      if (body->Skill(prhash(u8"Attribute Points")) > 0 ||
          body->Skill(prhash(u8"Skill Points")) > 0) {
        mind->Send(u8"You need to finish that character before you can use it.\n");
        mind->Send(u8"'{}' is now selected as your currect character to work on.\n", body->Name());
        mind->Owner()->SetCreator(body);
        return 0;
      }
      if ((!nmode) && body->IsAct(act_t::DEAD)) { // Ninjas can autoheal
        mind->Send(
            u8"Sorry, that character is dead.\n"
            u8"Use the 'newcharacter' command to create a new character.\n");
        return 0;
      }

      // FIXME: Handle conversion of body->Skill(prhash(u8"Resources")).
      if (mind->Owner()->Creator() == body)
        mind->Owner()->SetCreator(nullptr);

      body->Attach(mind);

      if (!body->HasSkill(prhash(u8"Object ID"))) {
        if (body->World()) {
          body->ClearSkill(prhash(u8"Attribute Points"));
          body->ClearSkill(prhash(u8"Skill Points"));
          body->ClearSkill(prhash(u8"Invisible"));
          auto obj_id = body->World()->Skill(prhash(u8"Last Object ID")) + 1;
          body->World()->SetSkill(prhash(u8"Last Object ID"), obj_id);
          body->SetSkill(prhash(u8"Object ID"), obj_id);
        }
      }

      if (nmode) {
        // This is ninja-healing and bypasses all healing mechanisms.
        body->ClearSkill(prhash(u8"Poisoned"));
        body->ClearSkill(prhash(u8"Thirsty"));
        body->ClearSkill(prhash(u8"Hungry"));
        body->SetStun(0);
        body->SetPhys(0);
        body->SetStru(0);
        body->UpdateDamage();
        body->Parent()->SendOut(
            stealth_t,
            stealth_s,
            u8";s heals and repairs ;s with Ninja Powers[TM].\n",
            u8"You heal ;s.\n",
            body,
            body);
      }

      if (body->IsAct(act_t::DYING)) {
        mind->Send(u8"You can see nothing, you are too busy dying.\n");
      } else if (body->IsAct(act_t::UNCONSCIOUS)) {
        mind->Send(u8"You can see nothing, you are out cold.\n");
      } else if (body->IsAct(act_t::SLEEP)) {
        mind->Send(u8"You can see nothing since you are asleep.\n");
      } else {
        body->Parent()->SendDescSurround(mind, body);
      }
      mind->Send(CMAG u8"You have entered: {}\n" CNRM, body->World()->ShortDesc());
      return 0;
    }
    Object* dest = body->PickObject(std::u8string(args), vmode | LOC_NEARBY);
    Object* rdest = dest;
    Object* veh = body;

    if (!dest) {
      if (mind)
        mind->Send(u8"You want to go where?\n");
      return 0;
    }

    if (dest->ActTarg(act_t::SPECIAL_LINKED) && dest->ActTarg(act_t::SPECIAL_LINKED)->Parent()) {
      rdest = dest->ActTarg(act_t::SPECIAL_LINKED)->Parent();
    }

    if ((!dest->Skill(prhash(u8"Enterable"))) && (!ninja)) {
      if (mind)
        mind->Send(u8"It is not possible to enter that object!\n");
    } else if ((!dest->Skill(prhash(u8"Enterable"))) && (!nmode)) {
      if (mind)
        mind->Send(u8"You need to be in ninja mode to enter that object!\n");
    } else if (dest->Skill(prhash(u8"Open")) < 1 && (!nmode)) {
      if (mind)
        mind->Send(u8"Sorry, {} is closed!\n", dest->Noun());
    } else if (
        dest->Parent() != body->Parent() && dest->Parent() == body->Parent()->Parent() &&
        body->Parent()->Skill(prhash(u8"Vehicle")) == 0) {
      if (mind)
        mind->Send(u8"You can't get {} to go there!\n", body->Parent()->Noun(1));
    } else if (
        dest->Parent() != body->Parent() && dest->Parent() == body->Parent()->Parent() &&
        (!(body->Parent()->Skill(prhash(u8"Vehicle")) & 0xFFF0)) // No Land Travel!
        && body->Parent()->Parent()->Skill(prhash(u8"WaterDepth")) == 0 &&
        rdest->Skill(prhash(u8"WaterDepth")) == 0) {
      if (mind)
        mind->Send(u8"You can't get {} to go there!\n", body->Parent()->Noun(1));
    } else {
      if (nmode) {
        // Ninja-movement can't be followed!
        if (body->Parent())
          body->Parent()->NotifyGone(body);
      }
      if (dest->Parent() != body->Parent() && dest->Parent() == body->Parent()->Parent()) {
        if (body->Parent()->Skill(prhash(u8"Vehicle")) == 4 &&
            body->Skill(prhash(u8"Boat, Row")) == 0) {
          if (mind)
            mind->Send(u8"You don't know how to operate {}!\n", body->Parent()->Noun(1));
          return 0;
        }
        veh = body->Parent();
      }

      if (rdest->Skill(prhash(u8"WaterDepth")) == 1 && body->Skill(prhash(u8"Swimming")) == 0) {
        if (veh == body || (veh->Skill(prhash(u8"Vehicle")) & 4) == 0) { // Have boat?
          if (mind)
            mind->Send(u8"Sorry, but you can't swim!\n");
          return 0;
        }
      } else if (rdest->Skill(prhash(u8"WaterDepth")) > 1) {
        if (veh == body || (veh->Skill(prhash(u8"Vehicle")) & 4) == 0) { // Have boat?
          if (mind)
            mind->Send(u8"Sorry, you need a boat to go there!\n");
          return 0;
        }
      }

      int newworld = (body->World() != rdest->World());
      if (dest->ActTarg(act_t::SPECIAL_LINKED) && dest->ActTarg(act_t::SPECIAL_LINKED)->Parent()) {
        body->Parent()->SendOut(stealth_t, stealth_s, u8";s leaves ;s.\n", u8"", body, dest);
        dest = dest->ActTarg(act_t::SPECIAL_LINKED)->Parent();
      } else if (body->Parent()) {
        body->Parent()->SendOut(stealth_t, stealth_s, u8";s enters ;s.\n", u8"", body, dest);
      }

      int reas = 0;
      if ((!nmode) && (reas = veh->Travel(dest))) {
        if (reas < 0) { // If it's not a script-prevent (which handles alert)
          body->Parent()->SendOut(
              stealth_t,
              stealth_s,
              u8"...but ;s didn't seem to fit!\n",
              u8"You could not fit!\n",
              body,
              nullptr);
        }
      } else {
        if (nmode) {
          body->Parent()->RemoveLink(body);
          body->SetParent(dest);
        }
        body->Parent()->SendOut(stealth_t, stealth_s, u8";s arrives.\n", u8"", body, nullptr);
        if (mind && (vmode & (LOC_NINJA | LOC_DARK)) == 0 && body->Parent()->LightLevel() < 100) {
          mind->Send(u8"It's too dark, you can't see anything.\n");
        } else if (mind && mind->Type() == mind_t::REMOTE) {
          body->Parent()->SendDescSurround(body, body, vmode);
        } else if (mind && mind->Type() == mind_t::SYSTEM) {
          mind->Send(u8"You enter {}\n", std::u8string(args));
        }

        if (mind && newworld) {
          mind->Send(CMAG u8"You have entered: {}\n" CNRM, body->World()->ShortDesc());
        }
        if (stealth_t > 0) {
          body->SetSkill(prhash(u8"Hidden"), body->Roll(prhash(u8"Stealth"), 2) * 2);
        }
        if (body->Roll(prhash(u8"Running"), 2) < 1) { // FIXME: Terrain/Direction Mods?
          if (mind) {
            mind->Send(
                CRED u8"\nYou are winded, and have to catch your breath." CNRM u8"  Raise the " CMAG u8"Running" CNRM u8" skill.\n");
          }
          body->BusyFor(3000);
        }
      }
    }
    return 0;
  }

  if (cnum == COM_QUIT) {
    if (!body) {
      return -1; // Player Disconnected
    }
    // if(body) delete body;
    if (mind)
      body->Detach(mind);

    if (mind && mind->Owner() && mind->Owner()->Room()) {
      mind->Owner()->Room()->SendDesc(mind);
      mind->Owner()->Room()->SendContents(mind);
    } else if (mind)
      mind->Send(u8"Use 'Enter' to return to the game.\n");

    return 0;
  }

  if (cnum == COM_HELP) {
    if ((!mind) || (!mind->Owner()))
      return 0;
    if (args == u8"commands") {
      std::u8string mes = u8"";
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
        mes += u8" - ";
        mes += comlist[ctr].shortdesc;
        mes += '\n';
      }
      mind->Send(mes);
      return 0;
    } else if (args == u8"socials") {
      std::u8string mes = u8"";
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
        mes += u8" - ";
        mes += comlist[ctr].shortdesc;
        mes += '\n';
      }
      mind->Send(mes);
      return 0;
    }
    mind->Send(
        u8"Well, the help command's not really implemented yet :P\n"
        u8"Try 'help commands' for a list of general commands.\n"
        u8"Try 'help socials' for a list of social commands.\n");
    return 0;
  }

  if (cnum == COM_SAY) {
    if (args.empty()) {
      if (!mind->SpecialPrompt().starts_with(u8"say")) {
        mind->SetSpecialPrompt(u8"say");
        mind->Send(u8"Type what your character will say - exit by just hitting ENTER:");
      } else {
        mind->SetSpecialPrompt(u8"");
        mind->Send(u8"Exiting out of say mode.");
      }
      return 0;
    } else {
      bool shouting = (args.length() >= 4 && !std::any_of(args.begin(), args.end(), ascii_islower));
      if (!shouting) {
        body->Parent()->SendOut(ALL, 0, u8";s says '{}'\n", u8"You say '{}'\n", body, body, args);
        body->ClearSkill(prhash(u8"Hidden"));
        return 0;
      } else {
        cnum = COM_SHOUT;
      }
    }
  }

  if (cnum == COM_SHOUT || cnum == COM_YELL || cnum == COM_CALL) {
    if (mind && args.empty()) {
      if (!mind->SpecialPrompt().starts_with(u8"shout")) {
        mind->SetSpecialPrompt(u8"shout");
        mind->Send(
            u8"Type what your character will shout - exit by just hitting "
            u8"ENTER:");
      } else {
        mind->SetSpecialPrompt(u8"");
        mind->Send(u8"Exiting out of shout mode.");
      }
    } else {
      if (args.substr(0, 4) == u8"for ") {
        auto prefix = args.find_first_not_of(u8" \t\n\r", 4);
        if (prefix == std::u8string::npos) {
          args = u8"";
        } else {
          args = args.substr(prefix);
        }
      }

      std::u8string mes = std::u8string(args);
      std::transform(mes.begin(), mes.end(), mes.begin(), ascii_toupper);
      body->Parent()->SendOut(
          ALL, 0, u8";s shouts '{}'!!!\n", u8"You shout '{}'!!!\n", body, body, mes);
      body->Parent()->Loud(body->ModAttribute(2), u8"someone shout '{}'!!!", mes);
    }
    body->ClearSkill(prhash(u8"Hidden"));
    return 0;
  }

  if (cnum >= COM_SOCIAL) {
    Object* targ = nullptr;
    if (body && body->Parent()) {
      std::u8string youmes = socials[cnum][0];
      std::u8string outmes = socials[cnum][1];
      std::u8string targmes = u8"";
      if (!args.empty()) {
        targ = body->PickObject(std::u8string(args), vmode | LOC_NEARBY | LOC_SELF | LOC_INTERNAL);
        if (!targ) {
          youmes = socials[cnum][5];
          outmes = u8"";
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
        replace_all(outmes, u8"$e", body->Pron());
        replace_all(outmes, u8"$m", body->Obje());
        replace_all(outmes, u8"$s", body->Poss());
        replace_all(outmes, u8"$n", body->Noun(1));
        if (targ) {
          replace_all(outmes, u8"$E", targ->Pron());
          replace_all(outmes, u8"$M", targ->Obje());
          replace_all(outmes, u8"$S", targ->Poss());
          replace_all(outmes, u8"$N", targ->Noun(1));
          replace_all(outmes, u8"$t", targ->Noun(1));
          replace_all(outmes, u8"$p", targ->Noun(1));
        }
        outmes[0] = ascii_toupper(outmes[0]);
        outmes += u8"\n";
      }
      if (youmes[0]) {
        replace_all(youmes, u8"$e", body->Pron());
        replace_all(youmes, u8"$m", body->Obje());
        replace_all(youmes, u8"$s", body->Poss());
        replace_all(youmes, u8"$n", body->Noun(1));
        if (targ) {
          replace_all(youmes, u8"$E", targ->Pron());
          replace_all(youmes, u8"$M", targ->Obje());
          replace_all(youmes, u8"$S", targ->Poss());
          replace_all(youmes, u8"$N", targ->Noun(1));
          replace_all(youmes, u8"$t", targ->Noun(1));
          replace_all(youmes, u8"$p", targ->Noun(1));
        }
        youmes[0] = ascii_toupper(youmes[0]);
        youmes += u8"\n";
      }
      if (targ) {
        replace_all(targmes, u8"$e", body->Pron());
        replace_all(targmes, u8"$m", body->Obje());
        replace_all(targmes, u8"$s", body->Poss());
        replace_all(targmes, u8"$n", body->Noun(1));
        if (targ) {
          replace_all(targmes, u8"$E", targ->Pron());
          replace_all(targmes, u8"$M", targ->Obje());
          replace_all(targmes, u8"$S", targ->Poss());
          replace_all(targmes, u8"$N", targ->Noun(1));
          replace_all(targmes, u8"$t", targ->Noun(1));
          replace_all(targmes, u8"$p", targ->Noun(1));
        }
        targmes[0] = ascii_toupper(targmes[0]);
        targmes += u8"\n";
        targ->Send(0, 0, targmes);
        targ->Deafen(true);
      }
      body->Parent()->SendOut(0, 0, outmes, youmes, body, targ);
      if (targ)
        targ->Deafen(false);
    }
    return 0;
  }

  if (cnum == COM_EMOTE) {
    std::u8string dot = u8".";
    if (args.back() == '.' || args.back() == '?' || args.back() == '!') {
      dot = u8"";
    }
    body->Parent()->SendOut(
        ALL, 0, u8";s {}{}\n", u8"Your character {}{}\n", body, body, args, dot);
    body->ClearSkill(prhash(u8"Hidden"));
    return 0;
  }

  if (cnum == COM_INTRODUCE) {
    Object* targ = body;
    if (!args.empty()) {
      targ = body->PickObject(std::u8string(args), vmode | LOC_NEARBY | LOC_SELF | LOC_INTERNAL);
      if (!targ) {
        if (mind) {
          mind->Send(u8"You don't see them here.\n");
        }
        return 0;
      }
      if ((!targ->HasName()) || (!body->Knows(targ))) {
        if (mind) {
          mind->Send(u8"You don't know what name to introduce them as.\n");
        }
        return 0;
      }
    }
    body->Parent()->SendOut(
        ALL,
        0,
        u8";s introduces ;s as \"{}\"\n",
        u8"You introduce ;s as \"{}\".\n",
        body,
        targ,
        targ->Name());
    body->ClearSkill(prhash(u8"Hidden"));
    return 0;
  }

  if (cnum == COM_INVENTORY) {
    if (mind) {
      mind->Send(u8"You ({}) are carrying:\n", body->ShortDesc());
      body->SendExtendedActions(mind, LOC_TOUCH | vmode | 1);
    }
    return 0;
  }

  if (cnum == COM_EQUIPMENT) {
    if (mind) {
      mind->Send(u8"You ({}) are using:\n", body->ShortDesc());
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
        mind->Send(u8"It's too dark, you can't see anything.\n");
      return 0;
    }

    int within = 0;
    if (args.substr(0, 3) == u8"at ") {
      auto prefix = args.find_first_not_of(u8" \t\n\r", 3);
      if (prefix == std::u8string::npos) {
        args = u8"";
      } else {
        args = args.substr(prefix);
      }
    } else if (args.substr(0, 3) == u8"in ") {
      auto prefix = args.find_first_not_of(u8" \t\n\r", 3);
      if (prefix == std::u8string::npos) {
        args = u8"";
      } else {
        args = args.substr(prefix);
      }
      within = 1;
    }

    DArr64<Object*> targs;
    if (!args.empty()) {
      targs = body->PickObjects(
          std::u8string(args), vmode | LOC_NEARBY | LOC_ADJACENT | LOC_SELF | LOC_INTERNAL);
    } else {
      targs.push_back(body->Parent());
    }

    if (targs.size() < 1) {
      if (mind)
        mind->Send(u8"You don't see that here.\n");
      return 0;
    }

    for (auto targ : targs) {
      if (within && (!targ->Skill(prhash(u8"Container"))) &&
          (!targ->Skill(prhash(u8"Liquid Container")))) {
        if (mind)
          mind->Send(u8"You can't look inside {}, it is not a container.\n", targ->Noun());
      } else if (within && (targ->Skill(prhash(u8"Locked")))) {
        if (mind)
          mind->Send(u8"You can't look inside {}, it is locked.\n", targ->Noun());
      } else {
        int must_open = within;
        if (within && targ->Skill(prhash(u8"Open")))
          must_open = 0;

        if (must_open) {
          targ->SetSkill(prhash(u8"Open"), 1000);
          body->Parent()->SendOut(
              stealth_t, stealth_s, u8";s opens ;s.\n", u8"You open ;s.\n", body, targ);
        }

        if (args.empty()) {
          body->Parent()->SendOut(stealth_t, stealth_s, u8";s looks around.\n", u8"", body, targ);
          if (mind)
            targ->SendDescSurround(mind, body, vmode);
        } else if (
            args == u8"north" || args == u8"south" || args == u8"east" || args == u8"west" ||
            args == u8"up" || args == u8"down") {
          body->Parent()->SendOut(stealth_t, stealth_s, u8";s looks ;s.\n", u8"", body, targ);
          if (mind) {
            targ->SendDesc(mind, body);
            targ->SendExtendedActions(mind, vmode);
          }
        } else if (within) {
          body->Parent()->SendOut(
              stealth_t, stealth_s, u8";s looks inside ;s.\n", u8"", body, targ);
          if (mind) {
            targ->SendDesc(mind, body);
            targ->SendExtendedActions(mind, vmode);
            targ->SendContents(mind, nullptr, vmode);
          }
        } else {
          body->Parent()->SendOut(stealth_t, stealth_s, u8";s looks at ;s.\n", u8"", body, targ);
          if (mind) {
            targ->SendDesc(mind, body);
            targ->SendExtendedActions(mind, vmode);
          }
        }

        if (must_open) {
          targ->ClearSkill(prhash(u8"Open"));
          body->Parent()->SendOut(
              stealth_t, stealth_s, u8";s closes ;s.\n", u8"You close ;s.\n", body, targ);
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
        mind->Send(u8"It's too dark, you can't see anything.\n");
      return 0;
    }

    DArr64<Object*> targs;
    if (!args.empty()) {
      targs = body->PickObjects(
          std::u8string(args), vmode | LOC_NEARBY | LOC_ADJACENT | LOC_SELF | LOC_INTERNAL);
      if (targs.size() == 0) {
        if (mind)
          mind->Send(u8"You don't see that here.\n");
        return 0;
      }
    } else {
      targs.push_back(body->Parent());
    }

    stealth_t = 0;
    stealth_s = 0;
    if (body->Pos() == pos_t::USE && (!body->IsUsing(prhash(u8"Perception")))) {
      body->Parent()->SendOut(
          stealth_t,
          stealth_s,
          u8";s stops {}.\n",
          u8"You stop {}.\n",
          body,
          nullptr,
          body->UsingString());
    }
    body->StartUsing(prhash(u8"Perception"));
    body->ClearSkill(prhash(u8"Hidden"));
    for (auto targ : targs) {
      std::u8string denied = u8"";
      for (Object* own = targ; own; own = own->Parent()) {
        if (own->IsAnimate() && own != body && (!own->IsAct(act_t::SLEEP)) &&
            (!own->IsAct(act_t::DEAD)) && (!own->IsAct(act_t::DYING)) &&
            (!own->IsAct(act_t::UNCONSCIOUS))) {
          denied = u8"You would need ";
          denied += own->Noun(1);
          denied += u8"'s permission to search ";
          denied += targ->Noun(0, 0, nullptr, own);
          denied += u8".\n";
        } else if (
            own->Skill(prhash(u8"Container")) && (!own->Skill(prhash(u8"Open"))) &&
            own->Skill(prhash(u8"Locked"))) {
          denied = own->Noun(1);
          if (own == targ) {
            denied += u8" is closed and locked so you can't search it.\n";
          } else {
            denied += u8" is closed and locked so you can't get to ";
            denied += targ->Noun(1);
            denied += u8".\n";
          }
          denied[0] = ascii_toupper(denied[0]);
        }
      }
      if ((!nmode) && (!denied.empty())) {
        if (mind)
          mind->Send(denied);
        continue;
      }

      body->Parent()->SendOut(
          stealth_t, stealth_s, u8";s searches ;s.\n", u8"you search ;s.\n", body, targ);

      auto objs = targ->Contents(vmode);
      for (auto obj : objs) {
        if (obj->Skill(prhash(u8"Hidden"))) {
          if (body->Roll(prhash(u8"Perception"), obj->Skill(prhash(u8"Hidden")))) {
            obj->ClearSkill(prhash(u8"Hidden"));
            body->Parent()->SendOut(
                stealth_t, stealth_s, u8";s reveals ;s.\n", u8"you reveal ;s.\n", body, obj);
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
    DArr64<Object*> targs;
    if (args.empty()) {
      targs.push_back(body);
    } else {
      targs = body->PickObjects(std::u8string(args), vmode | LOC_INTERNAL | LOC_NEARBY | LOC_SELF);
    }
    if (targs.size() < 1) {
      if (mind)
        mind->Send(u8"You don't see that here.\n");
      return 0;
    }

    for (auto targ : targs) {
      std::u8string denied = u8"";
      for (Object* own = targ; own; own = own->Parent()) {
        if (own->IsAnimate() && own != body && (!own->IsAct(act_t::SLEEP)) &&
            (!own->IsAct(act_t::DEAD)) && (!own->IsAct(act_t::DYING)) &&
            (!own->IsAct(act_t::UNCONSCIOUS))) {
          denied = u8"You would need ";
          denied += own->Noun(1);
          denied += u8"'s permission to hide ";
          denied += targ->Noun(0, 0, nullptr, own);
          denied += u8".\n";
        } else if (
            own->Skill(prhash(u8"Container")) && (!own->Skill(prhash(u8"Open"))) &&
            own->Skill(prhash(u8"Locked"))) {
          if (own != targ) {
            denied = own->Noun(1);
            denied += u8" is closed and locked so you can't get to ";
            denied += targ->Noun(1);
            denied += u8".\n";
            denied[0] = ascii_toupper(denied[0]);
          }
        }
      }
      if ((!nmode) && (denied.empty())) {
        if (mind)
          mind->Send(denied);
        continue;
      }
      if ((!nmode) && targ->Skill(prhash(u8"Obvious"))) {
        if (mind)
          mind->Send(u8"You could never hide {}, it's too obvious.", targ->Noun(0, 0, body));
        continue;
      }
      if ((!nmode) && targ->Skill(prhash(u8"Open"))) {
        if (targ->Skill(prhash(u8"Closeable"))) {
          if (mind)
            mind->Send(u8"You can't hide {} while it's open.", targ->Noun(0, 0, body));
        } else {
          if (mind)
            mind->Send(u8"You can't hide {}.  It's wide open.", targ->Noun(0, 0, body));
        }
        continue;
      }

      body->Parent()->SendOut(
          stealth_t, stealth_s, u8";s hides ;s.\n", u8"you hide ;s.\n", body, targ);

      targ->SetSkill(prhash(u8"Hidden"), body->Roll(prhash(u8"Stealth"), 2) * 2);
    }
    return 0;
  }

  if (cnum == COM_EXAMINE) {
    Object* targ = nullptr;
    if (args.empty()) {
      if (mind)
        mind->Send(u8"You want to examine what?\n");
      return 0;
    }
    targ = body->PickObject(std::u8string(args), vmode | LOC_INTERNAL | LOC_NEARBY | LOC_SELF);
    if (!targ) {
      if (mind)
        mind->Send(u8"You don't see that here.\n");
    } else {
      body->Parent()->SendOut(stealth_t, stealth_s, u8";s examines ;s.\n", u8"", body, targ);
      if (mind)
        targ->SendLongDesc(mind, body);
    }
    return 0;
  }

  if (cnum == COM_CONSIDER) {
    Object* targ = nullptr;
    if (args.empty()) {
      if (mind)
        mind->Send(u8"You want to consider what?\n");
      return 0;
    }
    targ = body->PickObject(std::u8string(args), vmode | LOC_INTERNAL | LOC_NEARBY | LOC_SELF);
    if (!targ) {
      if (mind)
        mind->Send(u8"You don't see that here.\n");
    } else if (!targ->IsAnimate()) { // Inanimate Object (Consider Using)
      body->Parent()->SendOut(
          stealth_t,
          stealth_s,
          u8";s considers using ;s.\n",
          u8"You consider using ;s.\n",
          body,
          targ);
      if (!mind)
        return 0;

      int handled = 0;

      // Weapons
      if (targ->HasSkill(prhash(u8"WeaponType"))) {
        handled = 1;
        Object* base = nullptr;
        for (auto loc :
             {act_t::WIELD,
              act_t::WEAR_RHIP,
              act_t::WEAR_RSHOULDER,
              act_t::WEAR_LHIP,
              act_t::WEAR_LSHOULDER}) {
          if (!base) {
            if (body->ActTarg(loc)) {
              base = body->ActTarg(loc);
            }
          }
        }

        if (base == targ) {
          mind->Send(u8"{} is your current weapon!\n", base->Noun(0, 0, body));
          mind->Send(u8"Consider using something else for comparison.\n");
          return 0;
        }
        uint32_t sk = (get_weapon_skill(targ->Skill(prhash(u8"WeaponType"))));
        if (!body->HasSkill(sk)) {
          mind->Send(
              CYEL u8"You don't know much about weapons like {}.\n" CNRM, targ->Noun(1, 1, body));
          mind->Send(
              CYEL u8"You would need to learn the {} skill to know more.\n" CNRM, SkillName(sk));
        } else {
          int diff;
          mind->Send(u8"Use of this weapon would use your {} skill.\n", SkillName(sk));

          if (base) {
            mind->Send(u8"You would use this weapon instead of {}.\n", base->Noun(1, 1, body));
          } else {
            mind->Send(u8"You would use this weapon instead of your fists.\n");
          }

          diff = body->Skill(sk);
          if (base)
            diff -= body->Skill(get_weapon_skill(base->Skill(prhash(u8"WeaponType"))));
          else
            diff -= body->Skill(prhash(u8"Punching"));
          if (diff > 0)
            mind->Send(CGRN u8"   ...would be a weapon you are more skilled with.\n" CNRM);
          else if (diff < 0)
            mind->Send(CYEL u8"   ...would be a weapon you are less skilled with.\n" CNRM);
          else
            mind->Send(u8"   ...would be a weapon you are similarly skilled with.\n");

          diff = targ->Skill(prhash(u8"WeaponReach"));
          if (base)
            diff -= base->Skill(prhash(u8"WeaponReach"));
          if (diff > 0)
            mind->Send(CGRN u8"   ...would give you more reach.\n" CNRM);
          else if (diff < 0)
            mind->Send(CYEL u8"   ...would give you less reach.\n" CNRM);
          else
            mind->Send(u8"   ...would give you similar reach.\n");

          diff = targ->Skill(prhash(u8"WeaponForce"));
          if (base)
            diff -= base->Skill(prhash(u8"WeaponForce"));
          if (diff > 0)
            mind->Send(CGRN u8"   ...would be more likely to do damage.\n" CNRM);
          else if (diff < 0)
            mind->Send(CYEL u8"   ...would be less likely to do damage.\n" CNRM);
          else
            mind->Send(u8"   ...would be about as likely to do damage.\n");

          diff = targ->Skill(prhash(u8"WeaponSeverity"));
          if (base)
            diff -= base->Skill(prhash(u8"WeaponSeverity"));
          if (diff > 0)
            mind->Send(CGRN u8"   ...would do more damage.\n" CNRM);
          else if (diff < 0)
            mind->Send(CYEL u8"   ...would do less damage.\n" CNRM);
          else
            mind->Send(u8"   ...would do similar damage.\n");

          diff = two_handed(targ->Skill(prhash(u8"WeaponType")));
          if (base)
            diff -= two_handed(base->Skill(prhash(u8"WeaponType")));
          if (diff > 0)
            mind->Send(CYEL u8"   ...would require both hands to use.\n" CNRM);
          else if (diff < 0)
            mind->Send(CGRN u8"   ...would not reqire both hands to use.\n" CNRM);
        }
      }

      // Containers
      int wtlimit = 0;
      int szlimit = 0;
      if (targ->HasSkill(prhash(u8"Container"))) {
        mind->Send(u8"{} is a container\n", targ->Noun(1, 1, body));

        wtlimit = targ->Skill(prhash(u8"Container"));
        szlimit = targ->Skill(prhash(u8"Capacity"));
        if (targ->Contents(vmode).size() == 0) {
          mind->Send(u8"   ...it appears to be empty.\n");
        } else {
          if (targ->ContainedVolume() < szlimit / 10) {
            mind->Send(u8"   ...it is nearly empty, ");
          } else if (targ->ContainedVolume() < szlimit / 2) {
            mind->Send(u8"   ...it is less than half full, ");
          } else if (targ->ContainedVolume() < szlimit * 9 / 10) {
            mind->Send(u8"   ...it is more than half full, ");
          } else if (targ->ContainedVolume() < szlimit) {
            mind->Send(u8"   ...it is nearly full, ");
          } else {
            mind->Send(u8"   ...it is full, ");
          }

          if (targ->ContainedWeight() < wtlimit / 10) {
            mind->Send(u8"and is nearly unloaded.\n");
          } else if (targ->ContainedWeight() < wtlimit / 2) {
            mind->Send(u8"and is less than half loaded.\n");
          } else if (targ->ContainedWeight() < wtlimit * 9 / 10) {
            mind->Send(u8"and is more than half loaded.\n");
          } else if (targ->ContainedWeight() < wtlimit) {
            mind->Send(u8"and is heavily laden.\n");
          } else {
            mind->Send(u8"and can hold no more.\n");
          }
        }
      }

      // Liquid Containers
      int volume = 0;
      if (targ->HasSkill(prhash(u8"Liquid Container"))) {
        mind->Send(u8"{} is a liquid container\n", targ->Noun(1, 1, body));
        volume = targ->Skill(prhash(u8"Liquid Container"));
        if (targ->Contents(vmode).size() == 0) {
          mind->Send(u8"   ...it appears to be empty.\n");
        } else {
          if (targ->Contents(vmode).front()->Quantity() < volume / 10) {
            mind->Send(u8"   ...it is nearly empty.\n");
          } else if (targ->Contents(vmode).front()->Quantity() < volume / 2) {
            mind->Send(u8"   ...it is less than half full.\n");
          } else if (targ->Contents(vmode).front()->Quantity() < volume * 9 / 10) {
            mind->Send(u8"   ...it is more than half full.\n");
          } else if (targ->Contents(vmode).front()->Quantity() < volume) {
            mind->Send(u8"   ...it is nearly full.\n");
          } else {
            mind->Send(u8"   ...it is full.\n");
          }
        }
      }

      Object* other = body->ActTarg(act_t::HOLD);
      if ((volume || wtlimit || szlimit) && other && other != targ) {
        // Containers
        if (szlimit && other->HasSkill(prhash(u8"Capacity"))) {
          if (szlimit < other->Skill(prhash(u8"Capacity"))) {
            mind->Send(CYEL u8"   ...it can't fit as much, " CNRM);
          } else if (szlimit > other->Skill(prhash(u8"Capacity"))) {
            mind->Send(CGRN u8"   ...it can fit more, " CNRM);
          } else {
            mind->Send(u8"   ...it can fit the same, ");
          }
        }
        if (wtlimit && other->HasSkill(prhash(u8"Container"))) {
          if (wtlimit < other->Skill(prhash(u8"Container"))) {
            mind->Send(CYEL u8"and can't carry as much as {}.\n" CNRM, other->Noun(0, 0, body));
          } else if (wtlimit > other->Skill(prhash(u8"Container"))) {
            mind->Send(CGRN u8"and can carry more than {}.\n" CNRM, other->Noun(0, 0, body));
          } else {
            mind->Send(u8"and can carry the same as {}.\n", other->Noun(0, 0, body));
          }
        }

        // Liquid Containers
        if (volume && other->HasSkill(prhash(u8"Liquid Container"))) {
          if (volume < other->Skill(prhash(u8"Liquid Container"))) {
            mind->Send(CYEL u8"   ...it can't hold as much as {}.\n" CNRM, other->Noun(0, 0, body));
          } else if (volume > other->Skill(prhash(u8"Liquid Container"))) {
            mind->Send(CGRN u8"   ...it can hold more than {}.\n" CNRM, other->Noun(0, 0, body));
          } else {
            mind->Send(
                u8"   ...it can hold about the same amount as {}.\n", other->Noun(0, 0, body));
          }
        }
      } else if ((volume || wtlimit || szlimit) && other) {
        mind->Send(u8"      (hold another of your containers to compare it with)\n");
      } else if (volume || wtlimit || szlimit) {
        mind->Send(u8"      (hold one of your containers to compare it with)\n");
      }

      // Armor/Clothing
      if (!targ->HasSkill(
              prhash(u8"WeaponType"))) { // Exclude weapons which can be worn (sheathed).
        int all = targ->WearMask();
        int num = count_ones(all);
        if (num > 1) {
          mind->Send(u8"{} can be worn in {} different ways:\n", targ->Noun(0, 0, body), num);
        } else if (num == 1) {
          mind->Send(u8"{} can only be worn one way:\n", targ->Noun(0, 0, body));
        }
        for (int mask = 1; mask <= all; mask <<= 1) {
          std::set<act_t> locs = targ->WearSlots(mask);
          if (locs.size() > 0) {
            mind->Send(u8"It can be worn on {}.\n", targ->WearNames(locs));
            handled = 1;

            std::set<Object*> repls;
            for (const auto loc : locs) {
              if (body->ActTarg(loc))
                repls.insert(body->ActTarg(loc));
            }

            for (const auto repl : repls) {
              if (repl != targ) {
                mind->Send(u8"   ...it would replace {}.\n", repl->Noun(0, 0, body));

                int diff = targ->NormAttribute(0);
                diff -= repl->NormAttribute(0);
                if (diff > 0) {
                  mind->Send(CGRN u8"      ...and would provide better protection.\n" CNRM);
                } else if (diff < 0) {
                  mind->Send(CYEL u8"      ...and would not provide as much protection.\n" CNRM);
                } else {
                  mind->Send(u8"      ...and would provide similar protection.\n");
                }
              } else if (repls.size() == 1) {
                mind->Send(u8"   ...it is already being worn there.\n");
              }
            }
          }
        }
      }

      // Other
      if (!handled) {
        mind->Send(u8"You really don't know what you would do with {}.\n", targ->Noun(1, 1, body));
      }
    } else { // Animate Opponent (Consider Attacking)
      body->Parent()->SendOut(
          stealth_t,
          stealth_s,
          u8";s considers attacking ;s.\n",
          u8"You consider attacking ;s.\n",
          body,
          targ);
      if (mind) {
        int diff;
        std::u8string mes = targ->Noun() + u8"...\n";
        mes[0] = ascii_toupper(mes[0]);
        mind->Send(mes);

        if ((targ->ActTarg(act_t::WIELD)) && (body->ActTarg(act_t::WIELD))) {
          mind->Send(u8"   ...has a weapon out and ready, but so do you.\n");
        } else if (!targ->ActTarg(act_t::WIELD)) {
          mind->Send(CGRN u8"   ...doesn't have a weapon out.\n" CNRM);
        } else if (!body->ActTarg(act_t::WIELD)) {
          mind->Send(CYEL u8"   ...has a weapon out and ready, and you do not!\n" CNRM);
        }

        Object* myweap = nullptr;
        Object* enweap = nullptr;
        for (auto loc :
             {act_t::WIELD,
              act_t::WEAR_RHIP,
              act_t::WEAR_RSHOULDER,
              act_t::WEAR_LHIP,
              act_t::WEAR_LSHOULDER}) {
          if (!myweap) {
            if (body->ActTarg(loc)) {
              myweap = body->ActTarg(loc);
            }
          }
          if (!enweap) {
            if (targ->ActTarg(loc)) {
              enweap = targ->ActTarg(loc);
            }
          }
        }

        if ((enweap) && (myweap)) {
          mind->Send(u8"   ...is armed, but so are you.\n");
        } else if ((!enweap) && (!myweap)) {
          mind->Send(u8"   ...is unarmed, but so are you.\n");
        } else if (!enweap) {
          mind->Send(CGRN u8"   ...is unarmed, and you are armed.\n" CNRM);
        } else if (!myweap) {
          mind->Send(CYEL u8"   ...is armed, and you are unarmed!\n" CNRM);
        }

        if (targ->HasSkill(prhash(u8"NaturalWeapon")) &&
            body->HasSkill(prhash(u8"NaturalWeapon"))) {
          mind->Send(u8"   ...has natural weaponry, but so do you.\n");
        } else if (body->HasSkill(prhash(u8"NaturalWeapon"))) {
          mind->Send(CGRN u8"   ...has no natural weaponry, but you do.\n" CNRM);
        } else if (targ->HasSkill(prhash(u8"NaturalWeapon"))) {
          mind->Send(CYEL u8"   ...has natural weaponry, and you do not!\n" CNRM);
        }

        diff = 0;
        if (myweap) {
          diff = (myweap->Skill(prhash(u8"WeaponReach")) > 9);
        }
        if (enweap && enweap->Skill(prhash(u8"WeaponReach")) > 9) {
          if (diff) {
            mind->Send(u8"   ...has a ranged weapon, and so do you!\n");
          } else {
            mind->Send(CYEL u8"   ...has a ranged weapon!\n" CNRM);
          }
        } else if (diff) {
          mind->Send(CGRN u8"   ...doesn't have a ranged weapon, and you do!\n" CNRM);
        } else {
          diff = 0;
          if (myweap) {
            diff += myweap->Skill(prhash(u8"WeaponReach"));
          }
          if (enweap) {
            diff -= enweap->Skill(prhash(u8"WeaponReach"));
          }
          if (diff < -5) {
            mind->Send(CRED u8"   ...outreaches you by a mile.\n" CNRM);
          } else if (diff < -2) {
            mind->Send(CRED u8"   ...has much greater reach than you.\n" CNRM);
          } else if (diff < -1) {
            mind->Send(CYEL u8"   ...has greater reach than you.\n" CNRM);
          } else if (diff < 0) {
            mind->Send(CYEL u8"   ...has a bit greater reach than you.\n" CNRM);
          } else if (diff > 5) {
            mind->Send(CGRN u8"   ...has a mile less reach than you.\n" CNRM);
          } else if (diff > 2) {
            mind->Send(CGRN u8"   ...has much less reach than you.\n" CNRM);
          } else if (diff > 1) {
            mind->Send(CGRN u8"   ...has less reach than you.\n" CNRM);
          } else if (diff > 0) {
            mind->Send(CGRN u8"   ...has a bit less reach than you.\n" CNRM);
          } else {
            mind->Send(u8"   ...has about your reach.\n");
          }

          if (myweap && enweap) {
            uint32_t sk1 = (get_weapon_skill(myweap->Skill(prhash(u8"WeaponType"))));
            uint32_t sk2 = (get_weapon_skill(enweap->Skill(prhash(u8"WeaponType"))));
            if (body->HasSkill(sk1) && body->HasSkill(sk2)) {
              diff = myweap->Skill(prhash(u8"WeaponForce"));
              diff -= enweap->Skill(prhash(u8"WeaponForce"));
              if (diff < -5) {
                mind->Send(CRED u8"   ...has a far more effective weapon than yours.\n" CNRM);
              } else if (diff < -2) {
                mind->Send(CRED u8"   ...has a much more effective weapon than yours.\n" CNRM);
              } else if (diff < -1) {
                mind->Send(CYEL u8"   ...has a more effective weapon than yours.\n" CNRM);
              } else if (diff < 0) {
                mind->Send(CYEL u8"   ...has a bit more effective weapon than yours.\n" CNRM);
              } else if (diff > 5) {
                mind->Send(CGRN u8"   ...has a far less effective weapon than yours.\n" CNRM);
              } else if (diff > 2) {
                mind->Send(CGRN u8"   ...has a much less effective weapon than yours.\n" CNRM);
              } else if (diff > 1) {
                mind->Send(CGRN u8"   ...has a less effective weapon than yours.\n" CNRM);
              } else if (diff > 0) {
                mind->Send(CGRN u8"   ...has a bit less effective weapon than yours.\n" CNRM);
              } else {
                mind->Send(u8"   ...has a weapon about as effective as yours.\n");
              }

              diff = myweap->Skill(prhash(u8"WeaponSeverity"));
              diff -= enweap->Skill(prhash(u8"WeaponSeverity"));
              if (diff < -2) {
                mind->Send(CRED u8"   ...has a far more damaging weapon than yours.\n" CNRM);
              } else if (diff < -1) {
                mind->Send(CRED u8"   ...has a much more damaging weapon than yours.\n" CNRM);
              } else if (diff < 0) {
                mind->Send(CYEL u8"   ...has a more damaging weapon than yours.\n" CNRM);
              } else if (diff > 2) {
                mind->Send(CGRN u8"   ...has a far less damaging weapon than yours.\n" CNRM);
              } else if (diff > 1) {
                mind->Send(CGRN u8"   ...has a much less damaging weapon than yours.\n" CNRM);
              } else if (diff > 0) {
                mind->Send(CGRN u8"   ...has a less damaging weapon than yours.\n" CNRM);
              } else {
                mind->Send(u8"   ...has a weapon about as damaging as yours.\n");
              }
            } else if (!body->HasSkill(sk2)) {
              mind->Send(CYEL u8"   ...has a weapon you are not familiar with.\n" CNRM);
            } else {
              mind->Send(CYEL u8"   ...has a weapon you familiar with, unlike your own.\n" CNRM);
            }
          }
        }

        if ((!targ->ActTarg(act_t::WEAR_SHIELD)) && (!body->ActTarg(act_t::WEAR_SHIELD))) {
          mind->Send(u8"   ...has no shield, but neither do you.\n");
        } else if (!targ->ActTarg(act_t::WEAR_SHIELD)) {
          mind->Send(CGRN u8"   ...has no shield.\n" CNRM);
        } else if (!body->ActTarg(act_t::WEAR_SHIELD)) {
          mind->Send(CYEL u8"   ...has a shield, and you do not!\n" CNRM);
        }

        diff = 0;
        uint32_t sk = prhash(u8"Punching");
        if (myweap) {
          sk = get_weapon_skill(myweap->Skill(prhash(u8"WeaponType")));
        }
        if (body->HasSkill(sk)) {
          diff += body->Skill(sk);
        }
        sk = prhash(u8"Punching");
        if (enweap) {
          sk = get_weapon_skill(enweap->Skill(prhash(u8"WeaponType")));
        }
        if (targ->HasSkill(sk)) {
          diff -= targ->Skill(sk);
        }
        if (diff < -5) {
          mind->Send(CRED u8"   ...is far more skilled than you.\n" CNRM);
        } else if (diff < -2) {
          mind->Send(CRED u8"   ...is much more skilled than you.\n" CNRM);
        } else if (diff < -1) {
          mind->Send(CYEL u8"   ...is more skilled than you.\n" CNRM);
        } else if (diff < 0) {
          mind->Send(CYEL u8"   ...is a bit more skilled than you.\n" CNRM);
        } else if (diff > 5) {
          mind->Send(CGRN u8"   ...is far less skilled than you.\n" CNRM);
        } else if (diff > 2) {
          mind->Send(CGRN u8"   ...is much less skilled than you.\n" CNRM);
        } else if (diff > 1) {
          mind->Send(CGRN u8"   ...is less skilled than you.\n" CNRM);
        } else if (diff > 0) {
          mind->Send(CGRN u8"   ...is a bit less skilled than you.\n" CNRM);
        } else {
          mind->Send(u8"   ...is about as skilled as you.\n");
        }
        diff = body->NormAttribute(0) - targ->NormAttribute(0);
        if (diff < -10) {
          mind->Send(CRED u8"   ...is titanic.\n" CNRM);
        } else if (diff < -5) {
          mind->Send(CRED u8"   ...is gargantuan.\n" CNRM);
        } else if (diff < -2) {
          mind->Send(CRED u8"   ...is much larger than you.\n" CNRM);
        } else if (diff < -1) {
          mind->Send(CYEL u8"   ...is larger than you.\n" CNRM);
        } else if (diff < 0) {
          mind->Send(CYEL u8"   ...is a bit larger than you.\n" CNRM);
        } else if (diff > 10) {
          mind->Send(CGRN u8"   ...is an ant compared to you.\n" CNRM);
        } else if (diff > 5) {
          mind->Send(CGRN u8"   ...is tiny compared to you.\n" CNRM);
        } else if (diff > 2) {
          mind->Send(CGRN u8"   ...is much smaller than you.\n" CNRM);
        } else if (diff > 1) {
          mind->Send(CGRN u8"   ...is smaller than you.\n" CNRM);
        } else if (diff > 0) {
          mind->Send(CGRN u8"   ...is a bit smaller than you.\n" CNRM);
        } else {
          mind->Send(u8"   ...is about your size.\n");
        }
        diff = body->NormAttribute(1) - targ->NormAttribute(1);
        if (diff < -10) {
          mind->Send(CRED u8"   ...is a blur of speed.\n" CNRM);
        } else if (diff < -5) {
          mind->Send(CRED u8"   ...is lightning fast.\n" CNRM);
        } else if (diff < -2) {
          mind->Send(CRED u8"   ...is much faster than you.\n" CNRM);
        } else if (diff < -1) {
          mind->Send(CYEL u8"   ...is faster than you.\n" CNRM);
        } else if (diff < 0) {
          mind->Send(CYEL u8"   ...is a bit faster than you.\n" CNRM);
        } else if (diff > 10) {
          mind->Send(CGRN u8"   ...is a turtle on valium.\n" CNRM);
        } else if (diff > 5) {
          mind->Send(CGRN u8"   ...is slower than dial-up.\n" CNRM);
        } else if (diff > 2) {
          mind->Send(CGRN u8"   ...is much slower than you.\n" CNRM);
        } else if (diff > 1) {
          mind->Send(CGRN u8"   ...is slower than you.\n" CNRM);
        } else if (diff > 0) {
          mind->Send(CGRN u8"   ...is a bit slower than you.\n" CNRM);
        } else {
          mind->Send(u8"   ...is about your speed.\n");
        }
        diff = body->NormAttribute(2) - targ->NormAttribute(2);
        if (diff < -10) {
          mind->Send(CRED u8"   ...is the strongest thing you've ever seen.\n" CNRM);
        } else if (diff < -5) {
          mind->Send(CRED u8"   ...is super-strong.\n" CNRM);
        } else if (diff < -2) {
          mind->Send(CRED u8"   ...is much stronger than you.\n" CNRM);
        } else if (diff < -1) {
          mind->Send(CYEL u8"   ...is stronger than you.\n" CNRM);
        } else if (diff < 0) {
          mind->Send(CYEL u8"   ...is a bit stronger than you.\n" CNRM);
        } else if (diff > 10) {
          mind->Send(CGRN u8"   ...is a complete push-over.\n" CNRM);
        } else if (diff > 5) {
          mind->Send(CGRN u8"   ...is a wimp compared to you.\n" CNRM);
        } else if (diff > 2) {
          mind->Send(CGRN u8"   ...is much weaker than you.\n" CNRM);
        } else if (diff > 1) {
          mind->Send(CGRN u8"   ...is weaker than you.\n" CNRM);
        } else if (diff > 0) {
          mind->Send(CGRN u8"   ...is a bit weaker than you.\n" CNRM);
        } else {
          mind->Send(u8"   ...is about your strength.\n");
        }
        if (targ->HasSkill(prhash(u8"TBAAction"))) {
          if ((targ->Skill(prhash(u8"TBAAction")) & 4128) == 0) {
            mind->Send(CGRN u8"   ...does not seem threatening.\n" CNRM);
          } else if ((targ->Skill(prhash(u8"TBAAction")) & 160) == 32) {
            mind->Send(CRED u8"   ...is spoiling for a fight.\n" CNRM);
          } else if ((targ->Skill(prhash(u8"TBAAction")) & 160) == 160) {
            mind->Send(CRED u8"   ...seems to be trolling for victems.\n" CNRM);
          } else if (targ->Skill(prhash(u8"TBAAction")) & 4096) {
            mind->Send(CYEL u8"   ...seems to be on the look-out for trouble.\n" CNRM);
          } else {
            mind->Send(u8"   ...is impossible - tell the Ninjas[TM].\n");
          }
        }

        if (targ->Skill(prhash(u8"Accomplishment"))) {
          if (body->HasAccomplished(targ->Skill(prhash(u8"Accomplishment")))) {
            mind->Send(CYEL u8"   ...has been defeated by you already.\n" CNRM);
          } else {
            mind->Send(CGRN u8"   ...has never been defeated by you.\n" CNRM);
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
        mind->Send(CCYN u8"{} is in {}.\n" CNRM, body->Noun(), body->Parent()->ShortDesc());
        body->SendScore(mind, body);
      } else {
        mind->Send(u8"You need to select a character first.\n");
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
        if (world->HasSkill(prhash(u8"Day Time")) && world->HasSkill(prhash(u8"Day Length"))) {
          int curtime = world->Skill(prhash(u8"Day Time"));
          curtime *= 24 * 60;
          curtime /= world->Skill(prhash(u8"Day Length"));
          mind->Send(u8"The time is now {}:{:02} in this world.\n", curtime / 60, curtime % 60);
        } else {
          mind->Send(u8"This world has no concept of time....\n");
        }
      } else {
        mind->Send(u8"This character is not in any world.\n");
      }
    } else {
      mind->Send(u8"You need to enter a character before doing this.\n");
    }
    return 0;
  }

  if (cnum == COM_WORLD) {
    if (!mind)
      return 0;
    if (body) {
      mind->Send(u8"This world is called: {}\n", body->World()->ShortDesc());

    } else if (args.length() > 0) {
      bool selected = false;
      for (const auto& world : Object::Universe()->Contents()) {
        if (world->IsAct(act_t::SPECIAL_HOME)) {
          if (world->Matches(std::u8string(args))) {
            mind->Owner()->SetWorld(world);
            mind->Send(
                u8"Selected " CBLU u8"{}" CNRM u8" as your current world.\n", world->ShortDesc());
            selected = true;
            break;
          }
        }
      }
      if (!selected) {
        mind->Send(u8"\nCan't find any active world by that name.\n");
        mind->Send(u8"\nThe worlds that exist in this instance of AcidMUD are:\n");
        for (const auto& world : Object::Universe()->Contents()) {
          if (world->IsAct(act_t::SPECIAL_HOME)) {
            mind->Send(u8"  * " CBLU u8"{}" CNRM u8"\n", world->ShortDesc());
          } else if (nmode) {
            mind->Send(u8"  * {} (inactive)\n", world->ShortDesc());
          }
        }
      }

    } else {
      Object* chr = mind->Owner()->Creator();
      if (chr) {
        mind->Send(
            u8"{} is in the world called: " CBLU u8"{}" CNRM u8"\n",
            chr->Name(),
            chr->World()->ShortDesc());
      } else if (mind->Owner()->World()) {
        mind->Send(
            u8"You have selected the world called: " CBLU u8"{}" CNRM u8"\n",
            mind->Owner()->World()->ShortDesc());
      } else {
        mind->Send(u8"You are not currently in any world.\n");
      }

      mind->Send(u8"\nThe worlds that exist in this instance of AcidMUD are:\n");
      for (const auto& world : Object::Universe()->Contents()) {
        if (world->IsAct(act_t::SPECIAL_HOME)) {
          mind->Send(u8"  * " CBLU u8"{}" CNRM u8"\n", world->ShortDesc());
        } else if (nmode) {
          mind->Send(u8"  * {} (inactive)\n", world->ShortDesc());
        }
      }
    }
    return 0;
  }

  if (cnum == COM_ZONE) {
    if (!mind)
      return 0;
    if (body) {
      mind->Send(u8"This zone is called: {}\n", body->Zone()->ShortDesc());
    }
    return 0;
  }

  if (cnum == COM_STATS) {
    Object* targ = nullptr;
    if ((!body) && args.empty()) {
      targ = mind->Owner()->Creator();
    } else if (!body) {
      targ = mind->Owner()->Room()->PickObject(std::u8string(args), vmode | LOC_INTERNAL);
    } else if (args.empty()) {
      targ = body;
    } else {
      targ = body->PickObject(
          std::u8string(args), vmode | LOC_INTERNAL | LOC_NEARBY | LOC_SELF | LOC_HERE);
    }
    if (!targ) {
      if (mind)
        mind->Send(u8"You don't see that here.\n");
    } else {
      if (mind) {
        if (targ->Room() == targ) {
          mind->Send(
              CCYN u8"{} is in {}, in {}.\n" CNRM,
              targ->Noun(),
              targ->Zone()->ShortDesc(),
              targ->World()->ShortDesc());
          targ->SendScore(mind, body);
        } else if (targ->Room() == targ->Parent()) {
          mind->Send(
              CCYN u8"{} is in {}, in {}, in {}.\n" CNRM,
              targ->Noun(),
              targ->Parent()->ShortDesc(),
              targ->Zone()->ShortDesc(),
              targ->World()->ShortDesc());
          targ->SendScore(mind, body);
        } else {
          mind->Send(
              CCYN u8"{} is in {}, in {}, in {}, in {}.\n" CNRM,
              targ->Noun(),
              targ->Parent()->ShortDesc(),
              targ->Room()->ShortDesc(),
              targ->Zone()->ShortDesc(),
              targ->World()->ShortDesc());
          targ->SendScore(mind, body);
        }
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
        mind->Send(u8"You want to lock what?\n");
      return 0;
    }
    targ = body->PickObject(std::u8string(args), vmode | LOC_INTERNAL | LOC_NEARBY);
    if (!targ) {
      if (mind)
        mind->Send(u8"You don't see that here.\n");
    } else if (targ->Skill(prhash(u8"Locked"))) {
      if (mind)
        mind->Send(u8"It is already locked!\n");
    } else if (targ->Skill(prhash(u8"Lock")) <= 0 && (!nmode)) {
      if (mind)
        mind->Send(u8"It does not seem to have a keyhole!\n");
    } else {
      if ((!nmode) && (!body->HasKeyFor(targ, vmode))) {
        if (mind) {
          mind->Send(u8"You don't seem to have the right key.\n");
        }
        return 0;
      }
      targ->SetSkill(prhash(u8"Locked"), 1);
      body->Parent()->SendOut(
          stealth_t, stealth_s, u8";s locks ;s.\n", u8"You lock ;s.\n", body, targ);
      if (targ->ActTarg(act_t::SPECIAL_LINKED)) {
        Object* targ2 = targ->ActTarg(act_t::SPECIAL_LINKED);
        targ2->Parent()->SendOut(stealth_t, stealth_s, u8";s locks.\n", u8"", targ2, nullptr);
        targ2->SetSkill(prhash(u8"Locked"), 1);
      }
    }
    return 0;
  }

  if (cnum == COM_UNLOCK) {
    Object* targ = nullptr;
    if (args.empty()) {
      if (mind)
        mind->Send(u8"You want to unlock what?\n");
      return 0;
    }
    targ = body->PickObject(std::u8string(args), vmode | LOC_INTERNAL | LOC_NEARBY);
    if (!targ) {
      if (mind)
        mind->Send(u8"You don't see that here.\n");
    } else if (!targ->Skill(prhash(u8"Locked"))) {
      if (mind)
        mind->Send(u8"It is not locked!\n");
    } else if (targ->Skill(prhash(u8"Lock")) <= 0 && (!nmode)) {
      if (mind)
        mind->Send(u8"It does not seem to have a keyhole!\n");
    } else {
      if ((!nmode) && (!body->HasKeyFor(targ, vmode))) {
        if (mind) {
          mind->Send(u8"You don't seem to have the right key.\n");
        }
        return 0;
      }
      targ->ClearSkill(prhash(u8"Locked"));
      body->Parent()->SendOut(
          stealth_t, stealth_s, u8";s unlocks ;s.\n", u8"You unlock ;s.\n", body, targ);
      if (targ->ActTarg(act_t::SPECIAL_LINKED)) {
        Object* targ2 = targ->ActTarg(act_t::SPECIAL_LINKED);
        targ2->Parent()->SendOut(stealth_t, stealth_s, u8";s unlocks.\n", u8"", targ2, nullptr);
        targ2->ClearSkill(prhash(u8"Locked"));
      }
      if (targ->Skill(prhash(u8"Accomplishment"))) {
        body->Accomplish(targ->Skill(prhash(u8"Accomplishment")), u8"unlocking this");
        targ->ClearSkill(prhash(u8"Accomplishment"));
      }
    }
    return 0;
  }

  if (cnum == COM_OPEN) {
    Object* targ = nullptr;
    if (args.empty()) {
      if (mind)
        mind->Send(u8"You want to open what?\n");
      return 0;
    }
    targ = body->PickObject(std::u8string(args), vmode | LOC_INTERNAL | LOC_NEARBY);
    if (!targ) {
      if (mind)
        mind->Send(u8"You don't see that here.\n");
    } else if (!targ->Skill(prhash(u8"Closeable"))) {
      if (mind)
        mind->Send(u8"That can't be opened or closed.\n");
    } else if (targ->Skill(prhash(u8"Open"))) {
      if (mind)
        mind->Send(u8"It's already open!\n");
    } else if (targ->Skill(prhash(u8"Locked"))) {
      if (mind)
        mind->Send(u8"It is locked!\n");
    } else {
      targ->SetSkill(prhash(u8"Open"), 1000);
      body->Parent()->SendOut(
          stealth_t, stealth_s, u8";s opens ;s.\n", u8"You open ;s.\n", body, targ);
      if (targ->ActTarg(act_t::SPECIAL_LINKED)) {
        Object* targ2 = targ->ActTarg(act_t::SPECIAL_LINKED);
        targ2->Parent()->SendOut(stealth_t, stealth_s, u8";s opens.\n", u8"", targ2, nullptr);
        targ2->SetSkill(prhash(u8"Open"), 1000);
        targ2->ClearSkill(prhash(u8"Locked")); // FIXME: Do I want to do this?
      }
    }
    return 0;
  }

  if (cnum == COM_CLOSE) {
    Object* targ = nullptr;
    if (args.empty()) {
      if (mind)
        mind->Send(u8"You want to close what?\n");
      return 0;
    }
    targ = body->PickObject(std::u8string(args), vmode | LOC_INTERNAL | LOC_NEARBY);
    if (!targ) {
      if (mind)
        mind->Send(u8"You don't see that here.\n");
    } else if (!targ->Skill(prhash(u8"Closeable"))) {
      if (mind)
        mind->Send(u8"That can't be opened or closed.\n");
    } else if (!targ->Skill(prhash(u8"Open"))) {
      if (mind)
        mind->Send(u8"It's already closed!\n");
    } else if (targ->Skill(prhash(u8"Locked"))) {
      if (mind)
        mind->Send(u8"It is locked!\n");
    } else {
      targ->ClearSkill(prhash(u8"Open"));
      body->Parent()->SendOut(
          stealth_t, stealth_s, u8";s closes ;s.\n", u8"You close ;s.\n", body, targ);
      if (targ->ActTarg(act_t::SPECIAL_LINKED)) {
        Object* targ2 = targ->ActTarg(act_t::SPECIAL_LINKED);
        targ2->Parent()->SendOut(stealth_t, stealth_s, u8";s closes.\n", u8"", targ2, nullptr);
        targ2->ClearSkill(prhash(u8"Open"));
        targ2->ClearSkill(prhash(u8"Locked")); // FIXME: Do I want to do this?
      }
    }
    return 0;
  }

  if (cnum == COM_LIST) {
    if (!mind)
      return 0;

    auto objs = body->Parent()->Contents(vmode);
    DArr64<Object*> shpkps;
    std::u8string reason = u8"";
    for (auto shpkp : objs) {
      if (shpkp->Skill(prhash(u8"Sell Profit"))) {
        if (shpkp->IsAct(act_t::DEAD)) {
          reason = u8"Sorry, the shopkeeper is dead!\n";
        } else if (shpkp->IsAct(act_t::DYING)) {
          reason = u8"Sorry, the shopkeeper is dying!\n";
        } else if (shpkp->IsAct(act_t::UNCONSCIOUS)) {
          reason = u8"Sorry, the shopkeeper is unconscious!\n";
        } else if (shpkp->IsAct(act_t::SLEEP)) {
          reason = u8"Sorry, the shopkeeper is asleep!\n";
        } else {
          shpkps.push_back(shpkp);
        }
      }
    }
    if (shpkps.size() == 0) {
      if (mind) {
        mind->Send(reason);
        mind->Send(u8"You can only do that around a shopkeeper.\n");
      }
    } else {
      Object* shpkp = shpkps.front();
      if (shpkp->ActTarg(act_t::WEAR_RSHOULDER) &&
          shpkp->ActTarg(act_t::WEAR_RSHOULDER)->Skill(prhash(u8"Vortex"))) {
        Object* vortex = shpkp->ActTarg(act_t::WEAR_RSHOULDER);
        objs = vortex->Contents(vmode);
        auto oobj = objs.front();
        for (auto obj : objs) {
          if (obj != objs.front() && obj->IsSameAs(*oobj))
            continue;
          int price = obj->Value();
          if (obj->Skill(prhash(u8"Money")) != obj->Value()) { // Not 1-1 Money
            price *= shpkp->Skill(prhash(u8"Sell Profit"));
            price += 999;
            price /= 1000;
          }
          mind->Send(u8"{:>10} gp: {}\n", price, obj->ShortDesc());
          oobj = obj;
        }
      }
    }
    return 0;
  }

  if (cnum == COM_BUY) {
    if (args.empty()) {
      if (mind)
        mind->Send(u8"What do you want to buy?\n");
      return 0;
    }

    auto objs = body->Parent()->Contents(vmode);
    DArr64<Object*> shpkps;
    std::u8string reason = u8"";
    for (auto shpkp : objs) {
      if (shpkp->Skill(prhash(u8"Sell Profit"))) {
        if (shpkp->IsAct(act_t::DEAD)) {
          reason = u8"Sorry, the shopkeeper is dead!\n";
        } else if (shpkp->IsAct(act_t::DYING)) {
          reason = u8"Sorry, the shopkeeper is dying!\n";
        } else if (shpkp->IsAct(act_t::UNCONSCIOUS)) {
          reason = u8"Sorry, the shopkeeper is unconscious!\n";
        } else if (shpkp->IsAct(act_t::SLEEP)) {
          reason = u8"Sorry, the shopkeeper is asleep!\n";
        } else {
          shpkps.push_back(shpkp);
        }
      }
    }
    if (shpkps.size() == 0) {
      if (mind) {
        mind->Send(reason);
        mind->Send(u8"You can only do that around a shopkeeper.\n");
      }
    } else {
      Object* shpkp = shpkps.front();
      if (shpkp->ActTarg(act_t::WEAR_RSHOULDER) &&
          shpkp->ActTarg(act_t::WEAR_RSHOULDER)->Skill(prhash(u8"Vortex"))) {
        Object* vortex = shpkp->ActTarg(act_t::WEAR_RSHOULDER);

        auto targs = vortex->PickObjects(std::u8string(args), vmode | LOC_INTERNAL);
        if (!targs.size()) {
          if (mind)
            mind->Send(u8"The shopkeeper doesn't have that.\n");
          return 0;
        }

        for (auto targ : targs) {
          int price = targ->Value() * targ->Quantity();
          if (price < 0) {
            if (mind)
              mind->Send(u8"You can't buy {}.\n", targ->Noun(0, 0, body));
            continue;
          } else if (price == 0) {
            if (mind) {
              std::u8string mes = targ->Noun(0, 0, body);
              mes += u8" is worthless.\n";
              mes[0] = ascii_toupper(mes[0]);
              mind->Send(mes);
            }
            continue;
          }

          if (targ->Skill(prhash(u8"Money")) != targ->Value()) { // Not 1-1 Money
            price *= shpkp->Skill(prhash(u8"Sell Profit"));
            price += 999;
            price /= 1000;
          }
          mind->Send(u8"{} gp: {}\n", price, targ->ShortDesc());

          int togo = price, ord = -price;
          auto pay = body->PickObjects(u8"a gold piece", vmode | LOC_INTERNAL, &ord);
          for (auto coin : pay) {
            togo -= coin->Quantity();
          }

          if (togo > 0) {
            if (mind)
              mind->Send(u8"You can't afford the {} gold (short {}).\n", price, togo);
          } else if (body->Stash(targ, 0, 0)) {
            body->Parent()->SendOut(
                stealth_t,
                stealth_s,
                u8";s buys and stashes ;s.\n",
                u8"You buy and stash ;s.\n",
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
                  u8";s stops holding ;s.\n",
                  u8"You stop holding ;s.\n",
                  body,
                  body->ActTarg(act_t::HOLD));
              body->StopAct(act_t::HOLD);
            }
            body->AddAct(act_t::HOLD, targ);
            body->Parent()->SendOut(
                stealth_t,
                stealth_s,
                u8";s buys and holds ;s.\n",
                u8"You buy and hold ;s.\n",
                body,
                targ);
            for (auto coin : pay) {
              shpkp->Stash(coin, 0, 1);
            }
          } else {
            if (mind)
              mind->Send(u8"You can't stash or hold {}.\n", targ->Noun(1));
          }
        }
      }
    }
    return 0;
  }

  if (cnum == COM_VALUE || cnum == COM_SELL) {
    if (args.empty()) {
      if (mind)
        mind->Send(u8"What do you want to sell?\n");
      return 0;
    }

    Object* targ = body->PickObject(std::u8string(args), vmode | LOC_NOTWORN | LOC_INTERNAL);
    if ((!targ) && body->ActTarg(act_t::HOLD) &&
        body->ActTarg(act_t::HOLD)->Parent() != body // Dragging
        && body->ActTarg(act_t::HOLD)->Matches(std::u8string(args))) {
      targ = body->ActTarg(act_t::HOLD);
    }

    if (!targ) {
      if (mind)
        mind->Send(u8"You want to sell what?\n");
      return 0;
    }

    if (targ->Skill(prhash(u8"Container")) || targ->Skill(prhash(u8"Liquid Container"))) {
      if (mind) {
        std::u8string mes = targ->Noun(0, 0, body);
        mes += u8" is a container.";
        mes += u8"  You can't sell containers (yet).\n";
        mes[0] = ascii_toupper(mes[0]);
        mind->Send(mes);
      }
      return 0;
    }

    if (targ->Contents(LOC_TOUCH | LOC_NOTFIXED).size() > 0) {
      if (mind) {
        std::u8string mes = targ->Noun(0, 0, body);
        mes += u8" is not empty.";
        mes += u8"  You must empty it before you can sell it.\n";
        mes[0] = ascii_toupper(mes[0]);
        mind->Send(mes);
      }
      return 0;
    }

    int price = targ->Value() * targ->Quantity();
    if (price < 0 || targ->HasSkill(prhash(u8"Priceless")) || targ->HasSkill(prhash(u8"Cursed"))) {
      if (mind)
        mind->Send(u8"You can't sell {}.\n", targ->Noun(0, 0, body));
      return 0;
    }
    if (price == 0) {
      if (mind)
        mind->Send(u8"{} is worthless.\n", targ->Noun(0, 0, body));
      return 0;
    }

    int wearable = 0;
    if (targ->HasSkill(prhash(u8"Wearable on Back")))
      wearable = 1;
    if (targ->HasSkill(prhash(u8"Wearable on Chest")))
      wearable = 1;
    if (targ->HasSkill(prhash(u8"Wearable on Head")))
      wearable = 1;
    if (targ->HasSkill(prhash(u8"Wearable on Neck")))
      wearable = 1;
    if (targ->HasSkill(prhash(u8"Wearable on Collar")))
      wearable = 1;
    if (targ->HasSkill(prhash(u8"Wearable on Waist")))
      wearable = 1;
    if (targ->HasSkill(prhash(u8"Wearable on Shield")))
      wearable = 1;
    if (targ->HasSkill(prhash(u8"Wearable on Left Arm")))
      wearable = 1;
    if (targ->HasSkill(prhash(u8"Wearable on Right Arm")))
      wearable = 1;
    if (targ->HasSkill(prhash(u8"Wearable on Left Finger")))
      wearable = 1;
    if (targ->HasSkill(prhash(u8"Wearable on Right Finger")))
      wearable = 1;
    if (targ->HasSkill(prhash(u8"Wearable on Left Foot")))
      wearable = 1;
    if (targ->HasSkill(prhash(u8"Wearable on Right Foot")))
      wearable = 1;
    if (targ->HasSkill(prhash(u8"Wearable on Left Hand")))
      wearable = 1;
    if (targ->HasSkill(prhash(u8"Wearable on Right Hand")))
      wearable = 1;
    if (targ->HasSkill(prhash(u8"Wearable on Left Leg")))
      wearable = 1;
    if (targ->HasSkill(prhash(u8"Wearable on Right Leg")))
      wearable = 1;
    if (targ->HasSkill(prhash(u8"Wearable on Left Wrist")))
      wearable = 1;
    if (targ->HasSkill(prhash(u8"Wearable on Right Wrist")))
      wearable = 1;
    if (targ->HasSkill(prhash(u8"Wearable on Left Shoulder")))
      wearable = 1;
    if (targ->HasSkill(prhash(u8"Wearable on Right Shoulder")))
      wearable = 1;
    if (targ->HasSkill(prhash(u8"Wearable on Left Hip")))
      wearable = 1;
    if (targ->HasSkill(prhash(u8"Wearable on Right Hip")))
      wearable = 1;

    auto objs = body->Parent()->Contents();
    DArr64<Object*> shpkps;
    std::u8string reason = u8"Sorry, nobody is buying that sort of thing here.\n";
    uint32_t skill = prhash(u8"None");
    for (auto shpkp : objs) {
      if (shpkp->IsAct(act_t::DEAD)) {
        reason = u8"Sorry, the shopkeeper is dead!\n";
      } else if (shpkp->IsAct(act_t::DYING)) {
        reason = u8"Sorry, the shopkeeper is dying!\n";
      } else if (shpkp->IsAct(act_t::UNCONSCIOUS)) {
        reason = u8"Sorry, the shopkeeper is unconscious!\n";
      } else if (shpkp->IsAct(act_t::SLEEP)) {
        reason = u8"Sorry, the shopkeeper is asleep!\n";
      } else if (targ->Skill(prhash(u8"Money")) == targ->Value()) { // 1-1 Money
        for (auto skl : shpkp->GetSkills()) {
          if (SkillName(skl.first).starts_with(u8"Buy ")) {
            skill = prhash(u8"Money");
            break;
          }
        }
      } else if (wearable && targ->NormAttribute(0) > 0) {
        if (shpkp->HasSkill(prhash(u8"Buy Armor"))) {
          skill = prhash(u8"Buy Armor");
        }
      } else if (targ->Skill(prhash(u8"Vehicle")) == 4) {
        if (shpkp->HasSkill(prhash(u8"Buy Boat"))) {
          skill = prhash(u8"Buy Boat");
        }
      } else if (targ->HasSkill(prhash(u8"Container"))) {
        if (shpkp->HasSkill(prhash(u8"Buy Container"))) {
          skill = prhash(u8"Buy Container");
        }
      } else if (targ->HasSkill(prhash(u8"Food")) && (!targ->HasSkill(prhash(u8"Drink")))) {
        if (shpkp->HasSkill(prhash(u8"Buy Food"))) {
          skill = prhash(u8"Buy Food");
        }
      }
      //      else if(false) {				//FIXME: Implement
      //	if(shpkp->HasSkill(prhash(u8"Buy Light"))) {
      //	  skill = prhash(u8"Buy Light");
      //	  }
      //	}
      else if (targ->HasSkill(prhash(u8"Liquid Container"))) { // FIXME: Not Potions?
        if (shpkp->HasSkill(prhash(u8"Buy Liquid Container"))) {
          skill = prhash(u8"Buy Liquid Container");
        }
      } else if (targ->HasSkill(prhash(u8"Liquid Container"))) { // FIXME: Not Bottles?
        if (shpkp->HasSkill(prhash(u8"Buy Potion"))) {
          skill = prhash(u8"Buy Potion");
        }
      } else if (targ->HasSkill(prhash(u8"Magical Scroll"))) {
        if (shpkp->HasSkill(prhash(u8"Buy Scroll"))) {
          skill = prhash(u8"Buy Scroll");
        }
      } else if (targ->HasSkill(prhash(u8"Magical Staff"))) {
        if (shpkp->HasSkill(prhash(u8"Buy Staff"))) {
          skill = prhash(u8"Buy Staff");
        }
      } else if (targ->HasSkill(prhash(u8"Magical Wand"))) {
        if (shpkp->HasSkill(prhash(u8"Buy Wand"))) {
          skill = prhash(u8"Buy Wand");
        }
      }
      //      else if(false) {				//FIXME: Implement
      //	if(shpkp->HasSkill(prhash(u8"Buy Trash"))) {
      //	  skill = prhash(u8"Buy Trash");
      //	  }
      //	}
      //      else if(false) {				//FIXME: Implement
      //	if(shpkp->HasSkill(prhash(u8"Buy Treasure"))) {
      //	  skill = prhash(u8"Buy Treasure");
      //	  }
      //	}
      else if (targ->Skill(prhash(u8"WeaponType")) > 0) {
        if (shpkp->HasSkill(prhash(u8"Buy Weapon"))) {
          skill = prhash(u8"Buy Weapon");
        }
      } else if (wearable && targ->NormAttribute(0) == 0) {
        if (shpkp->HasSkill(prhash(u8"Buy Worn"))) {
          skill = prhash(u8"Buy Worn");
        }
      }
      //      else if(false) {					//FIXME:
      //      Implement
      //	if(shpkp->HasSkill(prhash(u8"Buy Other"))) {
      //	  skill = prhash(u8"Buy Other");
      //	  }
      //	}

      if (skill == prhash(u8"None") && shpkp->HasSkill(prhash(u8"Buy All"))) {
        skill = prhash(u8"Buy All");
      }

      if (skill != prhash(u8"None")) {
        shpkps.push_back(shpkp);
      }
    }
    if (shpkps.size() == 0) {
      if (mind) {
        mind->Send(reason);
      }
    } else {
      Object* shpkp = shpkps.front();
      if (shpkp->ActTarg(act_t::WEAR_RSHOULDER) &&
          shpkp->ActTarg(act_t::WEAR_RSHOULDER)->Skill(prhash(u8"Vortex"))) {
        Object* vortex = shpkp->ActTarg(act_t::WEAR_RSHOULDER);

        if (skill != prhash(u8"Money")) { // Not 1-1 Money
          price *= shpkp->Skill(skill);
          price += 0;
          price /= 1000;
          if (price <= 0)
            price = 1;
        }
        mind->Send(u8"I'll give you {}gp for {}\n", price, targ->ShortDesc());

        if (cnum == COM_SELL) {
          int togo = price, ord = -price;
          auto pay = shpkp->PickObjects(u8"a gold piece", vmode | LOC_INTERNAL, &ord);
          for (auto coin : pay) {
            togo -= std::max(1, coin->Skill(prhash(u8"Quantity")));
          }

          if (togo <= 0) {
            body->Parent()->SendOut(
                stealth_t, stealth_s, u8";s sells ;s.\n", u8"You sell ;s.\n", body, targ);
            Object* payment = new Object;
            for (auto coin : pay) {
              coin->Travel(payment);
            }
            if (body->Stash(payment->Contents().front())) {
              targ->Travel(vortex);
            } else { // Keeper gets it back
              shpkp->Stash(payment->Contents().front(), 0, 1);
              if (mind)
                mind->Send(u8"You couldn't stash {} gold!\n", price);
            }
            delete payment;
          } else {
            if (mind)
              mind->Send(u8"I can't afford the {} gold.\n", price);
          }
        }
      }
    }
    return 0;
  }

  if (cnum == COM_DRAG) {
    if (args.empty()) {
      if (mind)
        mind->Send(u8"What do you want to drag?\n");
      return 0;
    }

    Object* targ = body->PickObject(std::u8string(args), vmode | LOC_NEARBY);
    if (!targ) {
      if (mind)
        mind->Send(u8"You want to drag what?\n");
      return 0;
    } else if (body->ActTarg(act_t::HOLD) == targ) {
      if (mind) {
        mind->Send(u8"You are already holding {}!\n", targ->Noun(1, 1, body));
      }
    } else if (
        body->IsAct(act_t::HOLD) && body->ActTarg(act_t::HOLD) != body->ActTarg(act_t::WIELD) &&
        body->ActTarg(act_t::HOLD) != body->ActTarg(act_t::WEAR_SHIELD)) {
      if (mind) {
        mind->Send(u8"You are already holding something.  Drop or stash it first.\n");
      }
    }

    if (targ->Pos() == pos_t::NONE) {
      if (mind)
        mind->Send(u8"You can't drag {}, it is fixed in place!\n", targ->Noun());
    } else if (targ->IsAnimate()) {
      if (mind) {
        mind->Send(
            u8"You would need {}'s permission to drag {}.\n",
            targ->Noun(1),
            targ->Noun(0, 0, nullptr, targ));
      }
    } else if (targ->Weight() > body->ModAttribute(2) * 50000) {
      if (mind)
        mind->Send(u8"You could never lift {}, it is too heavy.\n", targ->Noun());
    } else {
      body->AddAct(act_t::HOLD, targ);
      body->Parent()->SendOut(
          stealth_t,
          stealth_s,
          u8";s starts dragging ;s.\n",
          u8"You start dragging ;s.\n",
          body,
          targ);
    }
    return 0;
  }

  if (cnum == COM_GET) {
    if (args.empty()) {
      if (mind)
        mind->Send(u8"What do you want to get?\n");
      return 0;
    }

    auto targs = body->PickObjects(std::u8string(args), vmode | LOC_NEARBY);
    if (targs.size() == 0) {
      if (mind)
        mind->Send(u8"You want to get what?\n");
      return 0;
    }

    for (auto targ : targs) {
      auto trigs = targ->PickObjects(u8"all tbamud trigger script", LOC_NINJA | LOC_INTERNAL);
      for (auto trig : trigs) {
        int ttype = trig->Skill(prhash(u8"TBAScriptType"));
        if ((ttype & 0x2000040) == 0x2000040) { // OBJ-GET trigs
          if ((rand() % 100) < trig->Skill(prhash(u8"TBAScriptNArg"))) { // % Chance
            if (new_trigger(0, trig, body))
              return 0; // Says fail!
            if (targ->Parent() == targ->TrashBin())
              return 0; // Purged it
          }
        }
      }

      if ((!nmode) && targ->Pos() == pos_t::NONE) {
        if (mind)
          mind->Send(u8"You can't get {}, it is fixed in place!\n", targ->Noun());
      } else if ((!nmode) && targ->IsAnimate()) {
        if (mind)
          mind->Send(u8"You can't get {}, it is not inanimate.\n", targ->Noun());
      } else if ((!nmode) && targ->Weight() > body->ModAttribute(2) * 50000) {
        if (mind)
          mind->Send(u8"You could never lift {}, it is too heavy.\n", targ->Noun());
      } else if ((!nmode) && targ->Weight() > body->ModAttribute(2) * 10000) {
        if (mind)
          mind->Send(u8"You can't carry {}, it is too heavy.  Try 'drag' instead.\n", targ->Noun());
      } else {
        std::u8string denied = u8"";
        for (Object* owner = targ->Parent(); owner; owner = owner->Parent()) {
          if (owner->IsAnimate() && owner != body && (!owner->IsAct(act_t::SLEEP)) &&
              (!owner->IsAct(act_t::DEAD)) && (!owner->IsAct(act_t::DYING)) &&
              (!owner->IsAct(act_t::UNCONSCIOUS)) &&
              (owner->ActTarg(act_t::OFFER) != body || owner->ActTarg(act_t::HOLD) != targ)) {
            denied = u8"You would need ";
            denied += owner->Noun(1);
            denied += u8"'s permission to get ";
            denied += targ->Noun(0, 0, nullptr, owner);
            denied += u8".\n";
          } else if (
              owner->Skill(prhash(u8"Container")) && (!owner->Skill(prhash(u8"Open"))) &&
              owner->Skill(prhash(u8"Locked"))) {
            denied = owner->Noun(1);
            denied += u8" is closed and locked so you can't get to ";
            denied += targ->Noun(1);
            denied += u8".\n";
            denied[0] = ascii_toupper(denied[0]);
          }
        }

        if ((!nmode) && denied != u8"") {
          if (mind)
            mind->Send(denied);
        } else if (body->Stash(targ, 0)) {
          body->Parent()->SendOut(
              stealth_t,
              stealth_s,
              u8";s gets and stashes ;s.\n",
              u8"You get and stash ;s.\n",
              body,
              targ);
          if (targ->HasSkill(prhash(u8"Perishable"))) {
            targ->Deactivate();
          }
        } else if (
            body->IsAct(act_t::HOLD) &&
            body->ActTarg(act_t::HOLD) != body->ActTarg(act_t::WEAR_SHIELD) &&
            body->ActTarg(act_t::HOLD) != body->ActTarg(act_t::WIELD)) {
          if (mind)
            mind->Send(u8"You have no place to stash {}.\n", targ->Noun());
        } else if (targ->Skill(prhash(u8"Quantity")) > 1) {
          if (mind)
            mind->Send(u8"You have no place to stash {}.\n", targ->Noun());
        } else {
          if (body->IsAct(act_t::HOLD)) {
            body->Parent()->SendOut(
                stealth_t,
                stealth_s,
                u8";s stops holding ;s.\n",
                u8"You stop holding ;s.\n",
                body,
                body->ActTarg(act_t::HOLD));
            body->StopAct(act_t::HOLD);
          }
          targ->Travel(body);
          body->AddAct(act_t::HOLD, targ);
          body->Parent()->SendOut(
              stealth_t,
              stealth_s,
              u8";s gets and holds ;s.\n",
              u8"You get and hold ;s.\n",
              body,
              targ);
          if (targ->HasSkill(prhash(u8"Perishable"))) {
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
        mind->Send(u8"You must first 'hold' the object you want label.\n");
      return 0;
    } else if (!body->ActTarg(act_t::HOLD)) {
      if (mind)
        mind->Send(u8"What!?!?!  You are holding nothing?\n");
      return 0;
    }

    std::u8string name(body->ActTarg(act_t::HOLD)->ShortDesc());
    size_t start = name.find_first_of('(');
    if (start != name.npos) {
      name = name.substr(0, start);
      trim_string(name);
      body->ActTarg(act_t::HOLD)->SetShortDesc(name);
      if (mind)
        mind->Send(u8"{} is now unlabeled.\n", body->ActTarg(act_t::HOLD)->Noun(1, 1, body));
    } else {
      if (mind)
        mind->Send(
            u8"{} does not have a label to remove.\n",
            body->ActTarg(act_t::HOLD)->Noun(1, 1, body));
    }
    return 0;
  }

  if (cnum == COM_LABEL) {
    if (!body->IsAct(act_t::HOLD)) {
      if (mind)
        mind->Send(u8"You must first 'hold' the object you want label.\n");
      return 0;
    } else if (!body->ActTarg(act_t::HOLD)) {
      if (mind)
        mind->Send(u8"What!?!?!  You are holding nothing?\n");
      return 0;
    }

    if (args.empty()) { // Just Checking Label
      std::u8string label(body->ActTarg(act_t::HOLD)->ShortDesc());
      size_t start = label.find_first_of('(');
      if (start == label.npos) {
        if (mind)
          mind->Send(u8"{} has no label.\n", body->ActTarg(act_t::HOLD)->Noun(1, 1, body));
      } else {
        label = label.substr(start + 1);
        trim_string(label);
        size_t end = label.find_last_of(')');
        if (end != label.npos) {
          label = label.substr(0, end);
          trim_string(label);
        }
        if (mind)
          mind->Send(
              u8"{} is labeled '{}'.\n", body->ActTarg(act_t::HOLD)->Noun(1, 1, body), label);
      }
    } else { // Adding to Label
      std::u8string name(body->ActTarg(act_t::HOLD)->ShortDesc());
      std::u8string label = name;
      size_t start = label.find_first_of('(');
      if (start == label.npos) {
        label = (std::u8string(args));
        trim_string(label);
      } else {
        name = label.substr(0, start);
        trim_string(name);
        label = label.substr(start + 1);
        size_t end = label.find_last_of(')');
        if (end != label.npos)
          label = label.substr(0, end);
        trim_string(label);
        if (matches(label, std::u8string(args))) {
          if (mind)
            mind->Send(
                u8"{} already has that on the label.\n",
                body->ActTarg(act_t::HOLD)->Noun(1, 1, body));
          return 0;
        } else {
          label += u8" ";
          label += (std::u8string(args));
          trim_string(label);
        }
      }
      body->ActTarg(act_t::HOLD)->SetShortDesc(name);
      if (mind)
        mind->Send(
            u8"{} is now labeled '{}'.\n", body->ActTarg(act_t::HOLD)->Noun(1, 1, body), label);
      body->ActTarg(act_t::HOLD)->SetShortDesc((name + u8" (" + label + u8")"));
    }

    return 0;
  }

  if (cnum == COM_PUT) {
    if (args.substr(0, 3) == u8"in ") {
      auto prefix = args.find_first_not_of(u8" \t\n\r", 3);
      if (prefix == std::u8string::npos) {
        args = u8"";
      } else {
        args = args.substr(prefix);
      }
    }

    if (!body->IsAct(act_t::HOLD)) {
      if (mind)
        mind->Send(u8"You must first 'hold' the object you want to 'put'.\n");
      return 0;
    } else if (!body->ActTarg(act_t::HOLD)) {
      if (mind)
        mind->Send(u8"What!?!?!  You are holding nothing?\n");
      return 0;
    }

    if (args.empty()) {
      if (mind)
        mind->Send(
            u8"What do you want to put {} in?\n", body->ActTarg(act_t::HOLD)->Noun(0, 0, body));
      return 0;
    }

    Object* targ = body->PickObject(std::u8string(args), vmode | LOC_NEARBY | LOC_INTERNAL);
    if (!targ) {
      if (mind)
        mind->Send(
            u8"I don't see '{}' to put '{}' in!\n",
            std::u8string(args),
            body->ActTarg(act_t::HOLD)->Noun(0, 0, body));
    } else if (targ->IsAnimate()) {
      if (mind)
        mind->Send(u8"You can only put things in inanimate objects!\n");
    } else if (!targ->Skill(prhash(u8"Container"))) {
      if (mind)
        mind->Send(u8"You can't put anything in that, it is not a container.\n");
    } else if (targ->Skill(prhash(u8"Locked"))) {
      if (mind)
        mind->Send(u8"You can't put anything in that, it is locked.\n");
    } else if (targ == body->ActTarg(act_t::HOLD)) {
      if (mind)
        mind->Send(
            u8"You can't put {} into itself.\n", body->ActTarg(act_t::HOLD)->Noun(0, 0, body));
    } else if (
        (!nmode) && body->ActTarg(act_t::HOLD)->SubHasSkill(prhash(u8"Cursed")) &&
        targ->Owner() != body) {
      if (mind)
        mind->Send(
            u8"You can't seem to part with {}.\n", body->ActTarg(act_t::HOLD)->Noun(0, 0, body));
    } else {
      int closed = 0, res = 0;
      Object* obj = body->ActTarg(act_t::HOLD);
      res = obj->Travel(targ);
      if (res == -2) {
        if (mind)
          mind->Send(u8"It won't fit in there.\n");
      } else if (res == -3) {
        if (mind)
          mind->Send(u8"It's too heavy to put in there.\n");
      } else if (res) {
        if (mind)
          mind->Send(u8"You can't put it in there.\n");
      } else {
        if (!targ->Skill(prhash(u8"Open")))
          closed = 1;
        if (closed)
          body->Parent()->SendOut(
              stealth_t,
              stealth_s, // FIXME: Really open/close stuff!
              u8";s opens ;s.\n",
              u8"You open ;s.\n",
              body,
              targ);
        std::u8string safety = obj->Noun(0, 0, body);
        body->Parent()->SendOut(
            stealth_t,
            stealth_s,
            u8";s puts {} into ;s.\n",
            u8"You put {} into ;s.\n",
            body,
            targ,
            safety);
        if (closed)
          body->Parent()->SendOut(
              stealth_t, stealth_s, u8";s close ;s.\n", u8"You close ;s.\n", body, targ);
      }
    }
    return 0;
  }

  if (cnum == COM_UNWIELD) {
    if (!body->IsAct(act_t::WIELD)) {
      if (mind) {
        mind->Send(u8"You aren't currently wielding anything.\n");
      }
      return 0; // Already not wielding anything.  So, done.
    }
    cnum = COM_WIELD;
    args = u8"";
  }

  if (cnum == COM_WIELD) {
    Object* targ = nullptr;
    if (args.empty()) {
      if (body->IsAct(act_t::WIELD)) {
        Object* wield = body->ActTarg(act_t::WIELD);
        if ((!nmode) && wield && wield->SubHasSkill(prhash(u8"Cursed"))) {
          if (mind)
            mind->Send(u8"You can't seem to stop wielding {}!\n", wield->Noun(0, 0, body));
        } else if (wield && body->Wear(wield, ~(0UL), false)) {
          body->StopAct(act_t::WIELD);
          body->Parent()->SendOut(
              stealth_t, stealth_s, u8";s puts ;s away.\n", u8"You put ;s away.\n", body, wield);
        } else if (wield && body->Stash(wield, 0)) {
          body->StopAct(act_t::WIELD);
          body->Parent()->SendOut(
              stealth_t,
              stealth_s,
              u8";s stops wielding and stashes ;s.\n",
              u8"You stop wielding and stash ;s.\n",
              body,
              wield);
        } else if (
            body->IsAct(act_t::HOLD) &&
            body->ActTarg(act_t::HOLD) != body->ActTarg(act_t::WEAR_SHIELD) &&
            body->ActTarg(act_t::HOLD) != wield) {
          if (mind)
            mind->Send(
                u8"You are holding {} and can't stash {}.\n"
                u8"Perhaps you want to 'drop' one of these items?",
                body->ActTarg(act_t::HOLD)->Noun(1, 1, body),
                wield->Noun(1, 1, body));
        } else {
          body->StopAct(act_t::WIELD);
          body->Parent()->SendOut(
              stealth_t,
              stealth_s,
              u8";s stops wielding ;s.\n",
              u8"You stop wielding ;s.\n",
              body,
              wield);
          if (!body->Stash(wield))
            body->AddAct(act_t::HOLD, wield);
        }
        return 0;
      } else if (
          body->ActTarg(act_t::WEAR_RHIP) &&
          body->ActTarg(act_t::WEAR_RHIP)->HasSkill(prhash(u8"WeaponType"))) {
        targ = body->ActTarg(act_t::WEAR_RHIP);
      } else if (
          body->ActTarg(act_t::WEAR_RSHOULDER) &&
          body->ActTarg(act_t::WEAR_RSHOULDER)->HasSkill(prhash(u8"WeaponType"))) {
        targ = body->ActTarg(act_t::WEAR_RSHOULDER);
      } else if (
          body->ActTarg(act_t::WEAR_LHIP) &&
          body->ActTarg(act_t::WEAR_LHIP)->HasSkill(prhash(u8"WeaponType"))) {
        targ = body->ActTarg(act_t::WEAR_LHIP);
      } else if (
          body->ActTarg(act_t::WEAR_LSHOULDER) &&
          body->ActTarg(act_t::WEAR_LSHOULDER)->HasSkill(prhash(u8"WeaponType"))) {
        targ = body->ActTarg(act_t::WEAR_LSHOULDER);
      } else {
        if (mind) {
          mind->Send(u8"You are not wielding anything, and have no weapon ready to draw.\n");
        }
        return 0;
      }
    }

    if (!targ) {
      targ = body->PickObject(std::u8string(args), vmode | LOC_INTERNAL);
    }
    if (!targ) {
      if (mind)
        mind->Send(u8"You want to wield what?\n");
    } else if (targ->Skill(prhash(u8"WeaponType")) <= 0) {
      if (mind)
        mind->Send(u8"You can't wield that - it's not a weapon!\n");
    } else {
      if (body->IsAct(act_t::WIELD) && body->IsAct(act_t::HOLD)) {
        if (body->ActTarg(act_t::HOLD) != targ) {
          if (mind)
            mind->Send(
                u8"You are both holding and wielding other things.\n"
                u8"Perhaps you want to drop one of them?\n");
          return 0;
        }
      }

      // Auto-unwield (trying to wield something else)
      Object* wield = body->ActTarg(act_t::WIELD);
      if ((!nmode) && wield && wield->SubHasSkill(prhash(u8"Cursed"))) {
        if (mind)
          mind->Send(u8"You can't seem to stop wielding {}!\n", wield->Noun(0, 0, body));
        return 0;
      }
      targ->Travel(body->Zone()); // Kills Holds and Wields on u8"targ"
      targ->Travel(body);
      if (wield) {
        body->StopAct(act_t::WIELD);
        body->Parent()->SendOut(
            stealth_t,
            stealth_s,
            u8";s stops wielding ;s.\n",
            u8"You stop wielding ;s.\n",
            body,
            wield);
        if (!body->Stash(wield)) { // Try to stash first
          body->AddAct(act_t::HOLD, wield); // If not, just hold it
        }
      }

      auto trigs = targ->PickObjects(u8"all tbamud trigger script", LOC_NINJA | LOC_INTERNAL);
      for (auto trig : trigs) {
        int ttype = trig->Skill(prhash(u8"TBAScriptType"));
        if ((ttype & 0x2000200) == 0x2000200) { // OBJ-WEAR trigs
          if (new_trigger(0, trig, body))
            return 0; // Says FAIL!
        }
      }

      body->AddAct(act_t::WIELD, targ);
      body->Parent()->SendOut(
          stealth_t, stealth_s, u8";s wields ;s.\n", u8"You wield ;s.\n", body, targ);
    }
    return 0;
  }

  if (cnum == COM_HOLD || cnum == COM_LIGHT) {
    if (args.empty()) {
      if (mind)
        mind->Send(u8"What do you want to hold?\n");
      return 0;
    }

    Object* targ = body->PickObject(std::u8string(args), vmode | LOC_INTERNAL);
    if (!targ) {
      if (mind) {
        mind->Send(u8"You want to hold what?\n");
      }
      // FIXME - Implement Str-based Holding Capacity
      //    else if(targ->Skill(prhash(u8"WeaponType")) <= 0) {
      //      if(mind) mind->Send(u8"You can't hold that - you are too weak!\n");
      //      }
    } else if (body->ActTarg(act_t::HOLD) == targ) {
      if (mind) {
        mind->Send(u8"You are already holding {}!\n", targ->Noun(1, 1, body));
      }
    } else if (
        body->IsAct(act_t::HOLD) && body->ActTarg(act_t::HOLD) != body->ActTarg(act_t::WIELD) &&
        body->ActTarg(act_t::HOLD) != body->ActTarg(act_t::WEAR_SHIELD)) {
      if (mind) {
        mind->Send(u8"You are already holding something.  Drop or stash it first.\n");
      }
    } else if (
        body->ActTarg(act_t::WIELD) == targ &&
        two_handed(body->ActTarg(act_t::WIELD)->Skill(prhash(u8"WeaponType")))) {
      body->AddAct(act_t::HOLD, targ);
      body->Parent()->SendOut(
          stealth_t,
          stealth_s,
          u8";s puts a second hand on ;s.\n",
          u8"You put a second hand on ;s.\n",
          body,
          targ);
    } else if (body->ActTarg(act_t::WEAR_SHIELD) == targ) {
      body->AddAct(act_t::HOLD, targ);
      body->Parent()->SendOut(
          stealth_t, stealth_s, u8";s holds ;s.\n", u8"You hold ;s.\n", body, targ);
    } else if (body->Wearing(targ) && targ->SubHasSkill(prhash(u8"Cursed"))) {
      if (mind) {
        if (body->ActTarg(act_t::WIELD) == targ) {
          mind->Send(u8"You can't seem to stop wielding {}!\n", targ->Noun(0, 0, body));
        } else {
          mind->Send(u8"You can't seem to remove {}.\n", targ->Noun(0, 0, body));
        }
      }
    } else {
      if (body->IsAct(act_t::HOLD)) { // Means it's a shield/2-h weapon due to above.
        body->Parent()->SendOut(
            stealth_t,
            stealth_s,
            u8";s stops holding ;s.\n",
            u8"You stop holding ;s.\n",
            body,
            body->ActTarg(act_t::HOLD));
        body->StopAct(act_t::HOLD);
      }
      targ->Travel(body); // Kills Holds, Wears and Wields on u8"targ"
      body->AddAct(act_t::HOLD, targ);
      if (cnum == COM_LIGHT) {
        if (targ->HasSkill(prhash(u8"Lightable"))) {
          body->Parent()->SendOut(
              stealth_t,
              stealth_s,
              u8";s holds and lights ;s.\n",
              u8"You hold and light ;s.\n",
              body,
              targ);
          targ->SetSkill(prhash(u8"Lightable"), targ->Skill(prhash(u8"Lightable")) - 1);
          targ->SetSkill(prhash(u8"Light Source"), targ->Skill(prhash(u8"Brightness")));
          targ->Activate();
        } else {
          body->Parent()->SendOut(
              stealth_t,
              stealth_s,
              u8";s holds ;s.\n",
              u8"You hold ;s, but it can't be lit.\n",
              body,
              targ);
        }
      } else {
        body->Parent()->SendOut(
            stealth_t, stealth_s, u8";s holds ;s.\n", u8"You hold ;s.\n", body, targ);
      }
    }
    return 0;
  }

  if (cnum == COM_REMOVE) {
    if (args.empty()) {
      if (mind)
        mind->Send(u8"You want to remove what?\n");
      return 0;
    }

    auto targs = body->PickObjects(std::u8string(args), vmode | LOC_INTERNAL);
    if (targs.size() == 0) {
      if (mind)
        mind->Send(u8"You want to remove what?\n");
      return 0;
    }

    for (auto targ : targs) {
      auto trigs = targ->PickObjects(u8"all tbamud trigger script", LOC_NINJA | LOC_INTERNAL);
      for (auto trig : trigs) {
        int ttype = trig->Skill(prhash(u8"TBAScriptType"));
        if ((ttype & 0x2000800) == 0x2000800) { // OBJ-REMOVE trigs
          if (new_trigger(0, trig, body))
            return 0; // Says FAIL!
        }
      }

      int removed = 0;
      if ((!nmode) && targ->HasSkill(prhash(u8"Cursed"))) {
        if (mind)
          mind->Send(u8"{} won't come off!\n", targ->Noun(0, 0, body));
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
          mind->Send(u8"You are not wearing {}!\n", targ->Noun(0, 0, body));
      } else if (body->Stash(targ, 0, 0)) {
        body->Parent()->SendOut(
            stealth_t,
            stealth_s,
            u8";s removes and stashes ;s.\n",
            u8"You remove and stash ;s.\n",
            body,
            targ);
      } else if (
          body->IsAct(act_t::HOLD) &&
          body->ActTarg(act_t::HOLD) != body->ActTarg(act_t::WEAR_SHIELD) &&
          body->ActTarg(act_t::HOLD) != body->ActTarg(act_t::WIELD)) {
        if (mind)
          mind->Send(
              u8"You are already holding something else and can't stash {}.\n",
              targ->Noun(0, 0, body));
      } else {
        if (body->IsAct(act_t::HOLD)) {
          body->Parent()->SendOut(
              stealth_t,
              stealth_s,
              u8";s stops holding ;s.\n",
              u8"You stop holding ;s.\n",
              body,
              body->ActTarg(act_t::HOLD));
          body->StopAct(act_t::HOLD);
        }
        targ->Travel(body);
        body->AddAct(act_t::HOLD, targ);
        body->Parent()->SendOut(
            stealth_t,
            stealth_s,
            u8";s removes and holds ;s.\n",
            u8"You remove and hold ;s.\n",
            body,
            targ);
      }
    }
    return 0;
  }

  if (cnum == COM_WEAR) {
    DArr64<Object*> targs;

    if (args.empty()) {
      if (body->ActTarg(act_t::HOLD)) {
        targs.push_back(body->ActTarg(act_t::HOLD));
      } else {
        if (mind) {
          mind->Send(u8"What do you want to wear?  ");
          mind->Send(u8"Use 'wear <item>' or hold the item first.\n");
        }
        return 0;
      }
    }

    if (targs.size() < 1) {
      targs = body->PickObjects(std::u8string(args), vmode | LOC_NOTWORN | LOC_INTERNAL);
      if (!targs.size()) {
        if (mind)
          mind->Send(u8"You want to wear what?\n");
        return 0;
      }
    }

    int did_something = 0;
    for (auto targ : targs) {
      // loge(u8"You try to wear {}!\n", targ->Noun(0, 0, body));
      // if(mind) mind->Send(u8"You try to wear {}!\n", targ->Noun(0, 0, body));
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
          mind->Send(u8"You are already wearing {}!\n", targ->Noun(0, 0, body));
      } else {
        auto trigs = targ->PickObjects(u8"all tbamud trigger script", LOC_NINJA | LOC_INTERNAL);
        for (auto trig : trigs) {
          int ttype = trig->Skill(prhash(u8"TBAScriptType"));
          if ((ttype & 0x2000200) == 0x2000200) { // OBJ-WEAR trigs
            if (new_trigger(0, trig, body))
              return 0; // Says FAIL!
          }
        }

        if (body->Wear(targ)) {
          did_something = 1;
        }
      }
    }
    if (!did_something)
      if (mind)
        mind->Send(u8"You don't seem to have anything (else) to wear.\n");
    return 0;
  }

  if (cnum == COM_EAT) {
    if (args.empty()) {
      if (mind)
        mind->Send(u8"What do you want to eat?\n");
      return 0;
    }
    if (!body->HasSkill(prhash(u8"Hungry"))) {
      if (mind)
        mind->Send(u8"You are not hungry, you can't eat any more.\n");
      return 0;
    }
    auto targs = body->PickObjects(std::u8string(args), vmode | LOC_NOTWORN | LOC_INTERNAL);
    if (body->ActTarg(act_t::HOLD) && body->ActTarg(act_t::HOLD)->Parent() != body // Dragging
        && body->ActTarg(act_t::HOLD)->Matches(std::u8string(args))) {
      targs.push_back(body->ActTarg(act_t::HOLD));
    }
    if (!targs.size()) {
      if (mind)
        mind->Send(u8"You want to eat what?\n");
    } else {
      for (auto targ : targs) {
        if (!(targ->HasSkill(prhash(u8"Ingestible")))) {
          if (mind)
            mind->Send(u8"You don't want to eat {}.\n", targ->Noun(0, 0, body));
        } else {
          body->Parent()->SendOut(
              stealth_t, stealth_s, u8";s eats ;s.\n", u8"You eat ;s.\n", body, targ);

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
          mind->Send(u8"You have no place to stash {}.\n", targ->Noun(0, 0, body));
      }
    } else {
      if (mind)
        mind->Send(u8"You are not holding anything to stash.\n");
    }
    return 0;
  }

  if (cnum == COM_DROP) {
    if (args.empty()) {
      if (mind)
        mind->Send(u8"What do you want to drop?\n");
      return 0;
    }
    auto targs = body->PickObjects(std::u8string(args), vmode | LOC_NOTWORN | LOC_INTERNAL);
    if (body->ActTarg(act_t::HOLD) && body->ActTarg(act_t::HOLD)->Parent() != body // Dragging
        && body->ActTarg(act_t::HOLD)->Matches(std::u8string(args))) {
      targs.push_back(body->ActTarg(act_t::HOLD));
    }
    if (!targs.size()) {
      if (mind)
        mind->Send(u8"You want to drop what?\n");
    } else {
      for (auto targ : targs) {
        auto trigs = targ->PickObjects(u8"all tbamud trigger script", LOC_NINJA | LOC_INTERNAL);
        for (auto trig : trigs) {
          int ttype = trig->Skill(prhash(u8"TBAScriptType"));
          if ((ttype & 0x2000080) == 0x2000080) { // OBJ-DROP trigs
            if ((rand() % 100) < trig->Skill(prhash(u8"TBAScriptNArg"))) { // % Chance
              if (new_trigger(0, trig, body))
                return 0; // Says FAIL!
              if (targ->Parent() == targ->TrashBin())
                return 0; // Purged it
            }
          }
        }

        Object* room = body->PickObject(u8"here", LOC_HERE);
        trigs.clear();
        if (room)
          trigs = room->PickObjects(u8"all tbamud trigger script", LOC_NINJA | LOC_INTERNAL);
        for (auto trig : trigs) {
          int ttype = trig->Skill(prhash(u8"TBAScriptType"));
          if ((ttype & 0x4000080) == 0x4000080) { // ROOM-DROP trigs
            if ((rand() % 100) < trig->Skill(prhash(u8"TBAScriptNArg"))) { // % Chance
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
            mind->Send(u8"You can't drop {} here.\n", targ->Noun(0, 0, body));
        } else if (ret == -2) { // Exceeds Capacity
          if (mind)
            mind->Send(u8"You can't drop {}, there isn't room.\n", targ->Noun(0, 0, body));
        } else if (ret == -3) { // Exceeds Weight Limit
          if (mind)
            mind->Send(u8"You can't drop {}, it's too heavy.\n", targ->Noun(0, 0, body));
        } else if (ret == -4) { // Cursed
          if (mind)
            mind->Send(u8"You don't seem to be able to drop {}!\n", targ->Noun(0, 0, body));
        } else if (ret != 0) { //?
          if (mind)
            mind->Send(u8"You can't seem to drop {}!\n", targ->Noun(0, 0, body));
        }
      }
    }
    return 0;
  }

  if (cnum == COM_DRINK) {
    if (args.empty()) {
      if (mind)
        mind->Send(u8"What do you want to drink from?\n");
      return 0;
    }
    if (body->Skill(prhash(u8"Thirsty")) < 100) {
      if (mind)
        mind->Send(u8"You are not thirsty, you can't drink any more.\n");
      return 0;
    }
    Object* targ = body->PickObject(std::u8string(args), vmode | LOC_NOTWORN | LOC_INTERNAL);
    if ((!targ) && body->ActTarg(act_t::HOLD) &&
        body->ActTarg(act_t::HOLD)->Parent() != body // Dragging
        && body->ActTarg(act_t::HOLD)->Matches(std::u8string(args))) {
      targ = body->ActTarg(act_t::HOLD);
    }
    if (!targ) {
      targ = body->PickObject(std::u8string(args), vmode | LOC_NEARBY);
    }
    if (!targ) {
      if (mind)
        mind->Send(u8"You want to drink from what?\n");
    } else {
      std::u8string denied = u8"";
      for (Object* own = targ; own; own = own->Parent()) {
        if (own->IsAnimate() && own != body && (!own->IsAct(act_t::SLEEP)) &&
            (!own->IsAct(act_t::DEAD)) && (!own->IsAct(act_t::DYING)) &&
            (!own->IsAct(act_t::UNCONSCIOUS))) {
          denied = u8"You would need ";
          denied += own->Noun(1);
          denied += u8"'s permission to drink from ";
          denied += targ->Noun(0, 0, nullptr, own);
          denied += u8".\n";
        } else if (
            own->Skill(prhash(u8"Container")) && (!own->Skill(prhash(u8"Open"))) &&
            own->Skill(prhash(u8"Locked"))) {
          denied = own->Noun(1);
          if (own == targ) {
            denied += u8" is closed and locked so you can't drink from it.\n";
          } else {
            denied += u8" is closed and locked so you can't get to ";
            denied += targ->Noun(1);
            denied += u8".\n";
          }
          denied[0] = ascii_toupper(denied[0]);
        }
      }
      if ((!nmode) && (!denied.empty())) {
        if (mind)
          mind->Send(denied);
        return 0;
      }
      if (!(targ->HasSkill(prhash(u8"Liquid Container")))) {
        if (mind)
          mind->Send(
              u8"{} is not a liquid container.  You can't drink from it.\n",
              targ->Noun(0, 0, body));
        return 0;
      }
      if (targ->Contents(vmode).size() < 1) {
        if (mind)
          mind->Send(u8"{} is empty.  There is nothing to drink\n", targ->Noun(0, 0, body));
        return 0;
      }
      Object* obj = targ->Contents(vmode).front();
      if (targ->HasSkill(prhash(u8"Liquid Source")) && obj->Skill(prhash(u8"Quantity")) < 2) {
        if (mind)
          mind->Send(u8"{} is almost empty.  There is nothing to drink\n", targ->Noun(0, 0, body));
        return 0;
      }
      if ((!(obj->HasSkill(prhash(u8"Ingestible"))))) {
        if (mind)
          mind->Send(u8"You don't want to drink what's in {}.\n", targ->Noun(0, 0, body));
        return 0;
      }

      body->Parent()->SendOut(
          stealth_t,
          stealth_s,
          u8";s drinks some liquid out of ;s.\n",
          u8"You drink some liquid out of ;s.\n",
          body,
          targ);

      // Hunger/Thirst/Posion/Potion Effects
      body->Consume(obj);

      if (obj->Skill(prhash(u8"Quantity")) < 2) {
        obj->Recycle();
        if (targ->HasSkill(prhash(u8"Perishable"))) { // One-Use Vials
          targ->Recycle();
        }
      } else {
        obj->SetSkill(prhash(u8"Quantity"), obj->Skill(prhash(u8"Quantity")) - 1);
      }
    }
    return 0;
  }

  if (cnum == COM_DUMP) {
    if (args.empty()) {
      if (mind)
        mind->Send(u8"What do you want to dump?\n");
      return 0;
    }
    auto targs = body->PickObjects(std::u8string(args), vmode | LOC_NOTWORN | LOC_INTERNAL);
    if (body->ActTarg(act_t::HOLD) && body->ActTarg(act_t::HOLD)->Parent() != body // Dragging
        && body->ActTarg(act_t::HOLD)->Matches(std::u8string(args))) {
      targs.push_back(body->ActTarg(act_t::HOLD));
    }
    if (!targs.size()) {
      if (mind)
        mind->Send(u8"You want to dump what?\n");
    } else {
      for (auto targ : targs) {
        if (!(targ->HasSkill(prhash(u8"Liquid Container")))) {
          if (mind)
            mind->Send(
                u8"{} is not a liquid container.  It can't be dumped.\n", targ->Noun(0, 0, body));
          continue;
        }
        if (targ->Contents(LOC_TOUCH).size() < 1) {
          if (mind)
            mind->Send(u8"{} is empty.  There is nothing to dump\n", targ->Noun(0, 0, body));
          continue;
        }
        body->Parent()->SendOut(
            stealth_t,
            stealth_s,
            u8";s dumps all the liquid out of ;s.\n",
            u8"You dump all the liquid out of ;s.\n",
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
    if (args.substr(0, 5) == u8"from ") {
      auto prefix = args.find_first_not_of(u8" \t\n\r", 5);
      if (prefix == std::u8string::npos) {
        args = u8"";
      } else {
        args = args.substr(prefix);
      }
    }

    if (!body->IsAct(act_t::HOLD)) {
      if (mind)
        mind->Send(u8"You must first 'hold' the object you want to 'fill'.\n");
      return 0;
    } else if (!body->ActTarg(act_t::HOLD)) {
      if (mind)
        mind->Send(u8"What!?!?!  You are holding nothing?\n");
      return 0;
    }

    if (args.empty()) {
      if (mind)
        mind->Send(
            u8"Where do you want to fill {} from?\n", body->ActTarg(act_t::HOLD)->Noun(0, 0, body));
      return 0;
    }

    Object* src = body->PickObject(std::u8string(args), vmode | LOC_NEARBY | LOC_INTERNAL);
    Object* dst = body->ActTarg(act_t::HOLD);
    if (!src) {
      if (mind)
        mind->Send(
            u8"I don't see '{}' to fill {} from!\n", std::u8string(args), dst->Noun(0, 0, body));
    } else if (!dst->HasSkill(prhash(u8"Liquid Container"))) {
      if (mind)
        mind->Send(u8"You can not fill {}, it is not a liquid container.\n", dst->Noun(0, 0, body));
    } else if (src->IsAnimate()) {
      if (mind)
        mind->Send(u8"You can only fill things from inanimate objects!\n");
    } else if (!src->HasSkill(prhash(u8"Liquid Container"))) {
      if (mind)
        mind->Send(
            u8"You can't fill anything from that, it's not a liquid "
            u8"container.\n");
    } else if (dst->Skill(prhash(u8"Locked"))) {
      if (mind)
        mind->Send(u8"You can't fill {}, it is locked.\n", dst->Noun(0, 0, body));
    } else if (src->Skill(prhash(u8"Locked"))) {
      if (mind)
        mind->Send(u8"You can't fill anything from that, it is locked.\n");
    } else if (src == dst) {
      if (mind)
        mind->Send(u8"You can't fill {} from itself.\n", dst->Noun(0, 0, body));
    } else if (src->Contents(vmode).size() < 1) {
      if (mind)
        mind->Send(u8"You can't fill anything from that, it is empty.\n");
    } else if (
        src->HasSkill(prhash(u8"Liquid Source")) &&
        src->Contents(vmode).front()->Skill(prhash(u8"Quantity")) < 2) {
      if (mind)
        mind->Send(u8"You can't fill anything from that, it is almost empty.\n");
    } else {
      int myclosed = 0, itclosed = 0;

      int sqty = 1;
      int dqty = dst->Skill(prhash(u8"Capacity"));
      if (src->Contents(vmode).front()->Skill(prhash(u8"Quantity")) > 0) {
        sqty = src->Contents(vmode).front()->Skill(prhash(u8"Quantity"));
      }

      if (src->HasSkill(prhash(u8"Liquid Source"))) {
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
      liq->SetSkill(prhash(u8"Quantity"), dqty);
      if (sqty > 0) {
        src->Contents(vmode).front()->SetSkill(prhash(u8"Quantity"), sqty);
      } else {
        src->Contents(vmode).front()->Recycle();
      }

      if (!src->Skill(prhash(u8"Open")))
        itclosed = 1;
      if (!dst->Skill(prhash(u8"Open")))
        myclosed = 1;

      // FIXME: Really open/close stuff!
      if (itclosed)
        body->Parent()->SendOut(
            stealth_t, stealth_s, u8";s opens ;s.\n", u8"You open ;s.\n", body, src);
      if (myclosed)
        body->Parent()->SendOut(
            stealth_t, stealth_s, u8";s opens ;s.\n", u8"You open ;s.\n", body, dst);

      std::u8string safety = dst->Noun(0, 0, body);
      body->Parent()->SendOut(
          stealth_t,
          stealth_s,
          u8";s dumps out and fills {} from ;s.\n",
          u8"You dump out and fill {} from ;s.\n",
          body,
          src,
          safety);

      // FIXME: Really open/close stuff!
      if (myclosed)
        body->Parent()->SendOut(
            stealth_t, stealth_s, u8";s close ;s.\n", u8"You close ;s.\n", body, dst);
      if (itclosed)
        body->Parent()->SendOut(
            stealth_t, stealth_s, u8";s close ;s.\n", u8"You close ;s.\n", body, src);
    }
    return 0;
  }

  if (cnum == COM_LEAVE) {
    Object* oldp = body->Parent();
    if (!body->Parent()->Parent()) {
      if (mind)
        mind->Send(u8"It is not possible to leave this object!\n");
    } else if ((!body->Parent()->Skill(prhash(u8"Enterable"))) && (!ninja)) {
      if (mind)
        mind->Send(u8"It is not possible to leave this object!\n");
    } else if ((!body->Parent()->Skill(prhash(u8"Enterable"))) && (!nmode)) {
      if (mind)
        mind->Send(u8"You need to be in ninja mode to leave this object!\n");
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
        oldp->SendOut(stealth_t, stealth_s, u8";s leaves.\n", u8"", body, nullptr);
      body->Parent()->SendDescSurround(body, body);
      body->Parent()->SendOut(stealth_t, stealth_s, u8";s arrives.\n", u8"", body, nullptr);
    }
    return 0;
  }

  if (cnum == COM_SLEEP) {
    if (body->IsAct(act_t::SLEEP)) {
      if (mind)
        mind->Send(u8"You are already sleeping!\n");
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
          u8";s stops holding ;s.\n",
          u8"You stop holding ;s.\n",
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
          u8";s lies down and goes to sleep.\n",
          u8"You lie down and go to sleep.\n",
          body,
          nullptr);
    } else {
      body->Parent()->SendOut(
          stealth_t, stealth_s, u8";s goes to sleep.\n", u8"You go to sleep.\n", body, nullptr);
    }
    return 0;
  }

  if (cnum == COM_WAKE) {
    if (!body->IsAct(act_t::SLEEP)) {
      if (mind)
        mind->Send(u8"But you aren't asleep!\n");
    } else {
      body->StopAct(act_t::SLEEP);
      body->Parent()->SendOut(
          stealth_t, stealth_s, u8";s wakes up.\n", u8"You wake up.\n", body, nullptr);
    }
    return 0;
  }

  if (cnum == COM_REST) {
    if (body->IsAct(act_t::REST)) {
      body->StopAct(act_t::REST);
      body->Parent()->SendOut(
          stealth_t, stealth_s, u8";s stops resting.\n", u8"You stop resting.\n", body, nullptr);
      return 0;
    } else if (body->IsAct(act_t::SLEEP)) {
      body->StopAct(act_t::REST);
      body->AddAct(act_t::REST);
      body->Parent()->SendOut(
          stealth_t,
          stealth_s,
          u8";s wakes up and starts resting.\n",
          u8"You wake up and start resting.\n",
          body,
          nullptr);
    } else if (body->Pos() == pos_t::LIE || body->Pos() == pos_t::SIT) {
      body->AddAct(act_t::REST);
      body->Parent()->SendOut(
          stealth_t, stealth_s, u8";s starts resting.\n", u8"You start resting.\n", body, nullptr);
    } else {
      body->AddAct(act_t::REST);
      if (body->Pos() != pos_t::LIE)
        body->SetPos(pos_t::SIT);
      body->Parent()->SendOut(
          stealth_t,
          stealth_s,
          u8";s sits down and rests.\n",
          u8"You sit down and rest.\n",
          body,
          nullptr);
    }
    if (body->IsAct(act_t::FOLLOW)) {
      if (body->ActTarg(act_t::FOLLOW) && mind)
        body->Parent()->SendOut(
            stealth_t,
            stealth_s,
            u8";s stop following ;s.\n",
            u8"You stop following ;s.\n",
            body,
            body->ActTarg(act_t::FOLLOW));
      body->StopAct(act_t::FOLLOW);
    }
    return 0;
  }

  if (cnum == COM_STAND) {
    if (body->Pos() == pos_t::STAND || body->Pos() == pos_t::USE) {
      if (mind)
        mind->Send(u8"But you are already standing!\n");
    } else if (body->IsAct(act_t::SLEEP)) {
      body->SetPos(pos_t::STAND);
      body->StopAct(act_t::SLEEP);
      body->Parent()->SendOut(
          stealth_t,
          stealth_s,
          u8";s wakes up and stands.\n",
          u8"You wake up and stand.\n",
          body,
          nullptr);
    } else if (body->IsAct(act_t::REST)) {
      body->StopAct(act_t::REST);
      body->SetPos(pos_t::STAND);
      body->Parent()->SendOut(
          stealth_t,
          stealth_s,
          u8";s stops resting and stands up.\n",
          u8"You stop resting and stand up.\n",
          body,
          nullptr);
    } else {
      body->SetPos(pos_t::STAND);
      body->Parent()->SendOut(
          stealth_t, stealth_s, u8";s stands up.\n", u8"You stand up.\n", body, nullptr);
    }
    return 0;
  }

  if (cnum == COM_SIT) {
    if (body->Pos() == pos_t::SIT) {
      if (mind)
        mind->Send(u8"But you are already sitting!\n");
    } else if (body->IsAct(act_t::SLEEP)) {
      body->StopAct(act_t::SLEEP);
      body->SetPos(pos_t::SIT);
      body->Parent()->SendOut(
          stealth_t,
          stealth_s,
          u8";s awaken and sit up.\n",
          u8"You awaken and sit up.\n",
          body,
          nullptr);
    } else if (body->Pos() == pos_t::LIE) {
      body->SetPos(pos_t::SIT);
      body->Parent()->SendOut(
          stealth_t, stealth_s, u8";s sits up.\n", u8"You sit up.\n", body, nullptr);
    } else {
      body->SetPos(pos_t::SIT);
      body->Parent()->SendOut(
          stealth_t, stealth_s, u8";s sits down.\n", u8"You sit down.\n", body, nullptr);
    }
    if (body->IsAct(act_t::FOLLOW)) {
      if (body->ActTarg(act_t::FOLLOW) && mind)
        body->Parent()->SendOut(
            stealth_t,
            stealth_s,
            u8";s stop following ;s.\n",
            u8"You stop following ;s.\n",
            body,
            body->ActTarg(act_t::FOLLOW));
      body->StopAct(act_t::FOLLOW);
    }
    return 0;
  }

  if (cnum == COM_LIE) {
    if (body->Pos() == pos_t::LIE) {
      if (mind)
        mind->Send(u8"But you are already lying down!\n");
    } else {
      body->SetPos(pos_t::LIE);
      body->Parent()->SendOut(
          stealth_t, stealth_s, u8";s lies down.\n", u8"You lie down.\n", body, nullptr);
    }
    if (body->IsAct(act_t::FOLLOW)) {
      if (body->ActTarg(act_t::FOLLOW) && mind)
        body->Parent()->SendOut(
            stealth_t,
            stealth_s,
            u8";s stop following ;s.\n",
            u8"You stop following ;s.\n",
            body,
            body->ActTarg(act_t::FOLLOW));
      body->StopAct(act_t::FOLLOW);
    }
    return 0;
  }

  if (cnum == COM_CAST) {
    if (args.empty()) {
      if (mind)
        mind->Send(u8"What spell do you want to cast?\n");
      return 0;
    }

    int defself = 0;
    int special = 0;
    int freehand = 0;
    uint32_t spname = prhash(u8"None");
    if (args == std::u8string_view(u8"identify").substr(0, args.length())) {
      special = 1;
      spname = prhash(u8"Identify");
    } else if (args == std::u8string_view(u8"create food").substr(0, args.length())) {
      defself = -1;
      special = 2;
      freehand = 1;
      spname = prhash(u8"Create Food");
    } else if (args == std::u8string_view(u8"force sword").substr(0, args.length())) {
      defself = -1;
      special = 2;
      freehand = 1;
      spname = prhash(u8"Force Sword");
    } else if (args == std::u8string_view(u8"heat vision").substr(0, args.length())) {
      defself = 1;
      spname = prhash(u8"Heat Vision");
    } else if (args == std::u8string_view(u8"dark vision").substr(0, args.length())) {
      defself = 1;
      spname = prhash(u8"Dark Vision");
    } else if (args == std::u8string_view(u8"recall").substr(0, args.length())) {
      defself = 1;
      spname = prhash(u8"Recall");
    } else if (args == std::u8string_view(u8"teleport").substr(0, args.length())) {
      defself = 1;
      spname = prhash(u8"Teleport");
    } else if (args == std::u8string_view(u8"resurrect").substr(0, args.length())) {
      defself = 1;
      spname = prhash(u8"Resurrect");
    } else if (args == std::u8string_view(u8"remove curse").substr(0, args.length())) {
      defself = 1;
      spname = prhash(u8"Remove Curse");
    } else if (args == std::u8string_view(u8"cure poison").substr(0, args.length())) {
      defself = 1;
      spname = prhash(u8"Cure Poison");
    } else if (args == std::u8string_view(u8"sleep other").substr(0, args.length())) {
      spname = prhash(u8"Sleep Other");
    } else {
      if (mind)
        mind->Send(u8"Never heard of that spell.\n");
      return 0;
    }

    Object* src = nullptr;
    if (!nmode)
      src = body->NextHasSkill(crc32c(SkillName(spname) + u8" Spell"));
    if ((!nmode) && (!src)) {
      if (mind)
        mind->Send(
            u8"You don't know the {} Spell and have no items enchanted with it.\n",
            SkillName(spname));
      return 0;
    }

    if ((!defself) && (!body->ActTarg(act_t::POINT))) {
      if (mind)
        mind->Send(u8"That spell requires you to first point at your target.\n");
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
      std::u8string youmes = u8"You use ;s to cast " + SkillName(spname) + u8".\n";
      body->Parent()->SendOut(
          stealth_t, stealth_s, u8";s uses ;s to cast a spell.\n", youmes, body, src);
    }
    if (defself >= 0) { // Targeted
      std::u8string youmes = u8"You cast " + SkillName(spname) + u8" on ;s.\n";
      body->Parent()->SendOut(
          stealth_t, stealth_s, u8";s casts a spell on ;s.\n", youmes, body, targ);
    } else { // Not Targeted
      std::u8string youmes = u8"You cast " + SkillName(spname) + u8".\n";
      body->Parent()->SendOut(stealth_t, stealth_s, u8";s casts a spell.\n", youmes, body, nullptr);
    }

    int force = 1000; // FIXME: Magic Force!
    if (src)
      force = src->Skill(crc32c(SkillName(spname) + u8" Spell"));
    if (!special) { // Effect Person/Creature Spells
      Object* spell = new Object();
      spell->SetSkill(SkillName(spname) + u8" Spell", force);
      targ->Consume(spell);
      delete (spell);
    } else if (special == 2) { // Temporary Object Creation Spells
      Object* obj = new Object(body);
      if (spname == prhash(u8"Create Food")) {
        obj->SetShortDesc(u8"a piece of magical food");
        obj->SetSkill(prhash(u8"Food"), force * 100);
        obj->SetSkill(prhash(u8"Ingestible"), force);
      } else if (spname == prhash(u8"Force Sword")) {
        obj->SetShortDesc(u8"a sword of force");
        obj->SetSkill(prhash(u8"WeaponType"), 13);
        obj->SetSkill(prhash(u8"WeaponReach"), 1);
        obj->SetSkill(prhash(u8"WeaponSeverity"), 2);
        obj->SetSkill(prhash(u8"WeaponForce"), std::min(100, force));
      }
      obj->SetWeight(1);
      obj->SetVolume(1);
      obj->SetSize(1);
      obj->SetSkill(prhash(u8"Magical"), force);
      obj->SetSkill(prhash(u8"Light Source"), 10);
      obj->SetSkill(prhash(u8"Temporary"), force);
      obj->Activate();
      obj->SetPos(pos_t::LIE);
      body->AddAct(act_t::HOLD, obj);
      body->Parent()->SendOut(
          0,
          0,
          u8"{} appears in ;s's hand.\n",
          u8"{} appears in your hand.\n",
          body,
          nullptr,
          obj->Noun());
    } else if (spname == prhash(u8"Identify")) { // Other kinds of spells
      if (mind) {
        if (targ->Room() == targ->Parent()) {
          mind->Send(
              CCYN u8"{} is in {}, in {}, in {}.\n" CNRM,
              targ->Noun(),
              targ->Parent()->ShortDesc(),
              targ->Zone()->ShortDesc(),
              targ->World()->ShortDesc());
          targ->SendScore(mind, body);
        } else {
          mind->Send(
              CCYN u8"{} is in {}, in {}, in {}, in {}.\n" CNRM,
              targ->Noun(),
              targ->Parent()->ShortDesc(),
              targ->Room()->ShortDesc(),
              targ->Zone()->ShortDesc(),
              targ->World()->ShortDesc());
          targ->SendScore(mind, body);
        }
      }
    }

    if (src) {
      if (src->Skill(prhash(u8"Quantity")) > 1) {
        src->Split(src->Skill(prhash(u8"Quantity")) - 1); // Split off the rest
      }
      if (src->HasSkill(prhash(u8"Magical Charges"))) {
        if (src->Skill(prhash(u8"Magical Charges")) > 1) {
          src->SetSkill(prhash(u8"Magical Charges"), src->Skill(prhash(u8"Magical Charges")) - 1);
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
      mind->Send(u8"What command do you want to pray for?\n");
      mind->Send(u8"You need to include the command, like 'pray kill Joe'.\n");
    } else {
      // FIXME: Tell sub-command you're praying!
      body->Parent()->SendOut(
          stealth_t, stealth_s, u8";s prays.\n", u8"You pray.\n", body, nullptr);
      handle_single_command(body, std::u8string(args), mind);
    }
    return 0;
  }

  if (cnum == COM_STOP) { // Alias u8"stop" to u8"use"
    cnum = COM_USE;
    args = u8"";
  }
  if (cnum == COM_USE) {
    if (args.empty()) {
      if (body->Pos() != pos_t::USE) {
        mind->Send(u8"You're not using a skill.  Try 'use <skillname>' to start.\n");
      } else {
        body->Parent()->SendOut(
            stealth_t,
            stealth_s,
            u8";s stops {}.\n",
            u8"You stop {}.\n",
            body,
            nullptr,
            body->UsingString());
        body->SetPos(pos_t::STAND);
        return 2;
      }
      return 0;
    }

    int longterm = 0; // Long-running skills for results
    auto skill = get_skill(std::u8string(args));
    if (skill == prhash(u8"None")) {
      mind->Send(u8"Don't know what skill you're trying to use.\n");
      return 0;
    }

    if (skill == prhash(u8"Lumberjack")) {
      if (!body->HasSkill(skill)) {
        mind->Send(CYEL u8"You don't know how to do that.\n" CNRM);
        return 0;
      }
      if (!body->Parent()) { // You're nowhere?!?
        mind->Send(CYEL u8"There are no trees here.\n" CNRM);
        return 0;
      }
      if (body->Parent()->ShortDesc().contains(u8"orest") &&
          body->Parent()->HasSkill(prhash(u8"TBAZone")) &&
          (!body->Parent()->HasSkill(prhash(u8"Mature Trees")))) {
        body->Parent()->SetSkill(prhash(u8"Mature Trees"), 100);
        body->Parent()->Activate();
      }
      if (!body->Parent()->HasSkill(prhash(u8"Mature Trees"))) {
        mind->Send(CYEL u8"There are no trees here.\n" CNRM);
        return 0;
      } else if (body->Parent()->Skill(prhash(u8"Mature Trees")) < 10) {
        mind->Send(CYEL u8"There are too few trees to harvest here.\n" CNRM);
        return 0;
      } else {
        longterm = 3000; // FIXME: Temporary - should take longer!
      }
      if (body->IsUsing(prhash(u8"Lumberjack"))) { // Already been doing it
        if (body->Roll(skill, 10) > 0) { // Succeeded!
          body->Parent()->SendOut(
              ALL, 0, u8";s shouts 'TIMBER'!!!\n", u8"You shout 'TIMBER'!!!\n", body, body);
          body->Parent()->Loud(body->ModAttribute(2), u8"someone shout 'TIMBER'!!!");
          body->Parent()->SetSkill(
              prhash(u8"Mature Trees"), body->Parent()->Skill(prhash(u8"Mature Trees")) - 1);
          body->ClearSkill(prhash(u8"Hidden"));

          Object* log = new Object(body->Parent());
          log->SetShortDesc(u8"a log");
          log->SetDesc(u8"a fallen tree.");
          log->SetLongDesc(u8"This is a tree that has recently been cut down.");
          log->SetPos(pos_t::LIE);
          log->SetValue(10);
          log->SetVolume(1000);
          log->SetWeight(220000);
          log->SetSize(8000);
          log->SetSkill(prhash(u8"Made of Wood"), 200000);
        }
        body->Parent()->SendOut(
            ALL,
            0,
            u8";s continues chopping down trees.\n",
            u8"You continue chopping down trees.\n",
            body,
            body);
        body->Parent()->Loud(body->ModAttribute(2) / 2, u8"loud chopping sounds.");
        body->ClearSkill(prhash(u8"Hidden"));
      }
    }

    if (!body->IsUsing(skill)) { // Only if really STARTING to use skill.
      body->StartUsing(skill);

      // In case Stealth was started, re-calc (to hide going into stealth).
      stealth_t = 0;
      stealth_s = 0;
      if (body->IsUsing(prhash(u8"Stealth")) && body->Skill(prhash(u8"Stealth")) > 0) {
        stealth_t = body->Skill(prhash(u8"Stealth"));
        stealth_s = body->Roll(prhash(u8"Stealth"), 2);
      }

      if (body->Pos() != pos_t::STAND && body->Pos() != pos_t::USE) { // FIXME: Unused
        body->Parent()->SendOut(
            stealth_t,
            stealth_s,
            u8";s stands and starts {}.\n",
            u8"You stand up and start {}.\n",
            body,
            nullptr,
            body->UsingString());
      } else {
        body->Parent()->SendOut(
            stealth_t,
            stealth_s,
            u8";s starts {}.\n",
            u8"You start {}.\n",
            body,
            nullptr,
            body->UsingString());
      }
      if (!body->HasSkill(skill)) {
        mind->Send(
            CYEL u8"...you don't have the '{}' skill, so you're bad at this.\n" CNRM,
            SkillName(skill));
      }
    } else if (longterm == 0) {
      mind->Send(u8"You are already using {}\n", SkillName(skill));
    }

    if (longterm > 0) { // Long-running skills for results
      body->BusyFor(longterm, u8"use Lumberjack"); // FIXME: Temporary!
      return 2; // Full-round (and more) action!
    }
    return 0;
  }

  if (cnum == COM_POINT) {
    if (!args.empty()) {
      Object* targ =
          body->PickObject(std::u8string(args), vmode | LOC_NEARBY | LOC_SELF | LOC_INTERNAL);
      if (!targ) {
        if (mind)
          mind->Send(u8"You don't see that here.\n");
      } else {
        body->AddAct(act_t::POINT, targ);
        body->Parent()->SendOut(
            stealth_t,
            stealth_s,
            u8";s starts pointing at ;s.\n",
            u8"You start pointing at ;s.\n",
            body,
            targ);
      }
    } else if (body->IsAct(act_t::POINT)) {
      Object* targ = body->ActTarg(act_t::POINT);
      body->StopAct(act_t::POINT);
      body->Parent()->SendOut(
          stealth_t,
          stealth_s,
          u8";s stops pointing at ;s.\n",
          u8"You stop pointing at ;s.\n",
          body,
          targ);
    } else {
      if (mind)
        mind->Send(u8"But, you aren't pointing at anyting!\n");
    }
    return 0;
  }

  if (cnum == COM_OFFER) {
    if (!args.empty()) {
      Object* targ = body->PickObject(std::u8string(args), vmode | LOC_NEARBY);
      if (!targ) {
        if (mind)
          mind->Send(u8"You don't see that person here.\n");
      } else if (!body->ActTarg(act_t::HOLD)) {
        if (mind)
          mind->Send(u8"You aren't holding anything to offer.\n");
      } else if (!targ->IsAnimate()) {
        if (mind)
          mind->Send(u8"You can't offer anything to that.\n");
      } else {
        body->AddAct(act_t::OFFER, targ);
        body->Parent()->SendOut(
            stealth_t,
            stealth_s,
            u8";s offers something to ;s.\n",
            u8"You offer something to ;s.\n",
            body,
            targ);

        auto trigs = targ->PickObjects(u8"all tbamud trigger script", LOC_NINJA | LOC_INTERNAL);
        for (auto trig : trigs) {
          if ((trig->Skill(prhash(u8"TBAScriptType")) & 0x1000200) ==
              0x1000200) { // MOB-RECEIVE trigs
            if (!new_trigger(0, trig, body, body->ActTarg(act_t::HOLD))) {
              body->ActTarg(act_t::OFFER)->Travel(targ);
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
          u8";s stops offering something to ;s.\n",
          u8"You stop offering something to ;s.\n",
          body,
          targ);
    } else {
      if (mind)
        mind->Send(u8"But, you aren't offering anything to anyone!\n");
    }
    return 0;
  }

  if (cnum == COM_FOLLOW) {
    if (!args.empty()) {
      Object* targ = body->PickObject(std::u8string(args), vmode | LOC_NEARBY | LOC_SELF);
      if (!targ) {
        if (mind)
          mind->Send(u8"You don't see that here.\n");
      } else {
        body->AddAct(act_t::FOLLOW, targ);
        body->Parent()->SendOut(
            stealth_t,
            stealth_s,
            u8";s starts following ;s.\n",
            u8"You start following ;s.\n",
            body,
            targ);
      }
    } else if (body->IsAct(act_t::FOLLOW)) {
      Object* targ = body->ActTarg(act_t::FOLLOW);
      body->StopAct(act_t::FOLLOW);
      body->Parent()->SendOut(
          stealth_t,
          stealth_s,
          u8";s stops following ;s.\n",
          u8"You stop following ;s.\n",
          body,
          targ);
    } else {
      if (mind)
        mind->Send(u8"But, you aren't following anyone!\n");
    }
    return 0;
  }

  if (cnum == COM_ATTACK || cnum == COM_KILL || cnum == COM_PUNCH || cnum == COM_KICK) {
    // logemm(u8"Handling attack command from {} of '{}'\n", body->Name(), args);

    int attacknow = 1;
    if (!body->IsAct(act_t::FIGHT))
      attacknow = 0;

    Object* targ = nullptr;
    if (!args.empty()) {
      targ = body->PickObject(std::u8string(args), vmode | LOC_NEARBY | LOC_CONSCIOUS);
      if (!targ)
        targ = body->PickObject(std::u8string(args), vmode | LOC_NEARBY | LOC_ALIVE);
      if (!targ)
        targ = body->PickObject(std::u8string(args), vmode | LOC_NEARBY);
      if (!targ) {
        if (mind)
          mind->Send(u8"You don't see that here.\n");
        return 0;
      }
    } else {
      if (body->IsAct(act_t::FIGHT)) {
        targ = body->ActTarg(act_t::FIGHT);
        if (!body->IsNearBy(targ)) {
          if (mind)
            mind->Send(u8"Your target is gone!\n");
          body->StopAct(act_t::FIGHT);
          return 0;
        }
      } else {
        if (mind)
          mind->Send(u8"Who did you want to hit?\n");
        return 0;
      }
    }

    if (cnum == COM_ATTACK &&
        (!targ->IsAnimate() || targ->IsAct(act_t::DEAD) || targ->IsAct(act_t::DYING) ||
         targ->IsAct(act_t::UNCONSCIOUS))) {
      if (mind)
        mind->Send(u8"No need, target is down!\n");
      body->StopAct(act_t::FIGHT);
      return 0;
    }

    if (is_pc(body) && is_pc(targ)) {
      if (mind) {
        mind->Send(
            u8"You can't attack {}, {} is a PC (no PvP)!\n",
            targ->Noun(0, 0, body),
            targ->Noun(0, 0, body));
      }
      return 0;
    }

    if ((!body->Parent()) || body->Parent()->HasSkill(prhash(u8"Peaceful"))) {
      if (mind) {
        mind->Send(u8"You can't fight here.  This is a place of peace.\n");
      }
      return 0;
    }

    body->BusyFor(3000); // Overridden below if is alive/animate

    if (!(!targ->IsAnimate() || targ->IsAct(act_t::DEAD) || targ->IsAct(act_t::DYING) ||
          targ->IsAct(act_t::UNCONSCIOUS))) {
      body->AddAct(act_t::FIGHT, targ);
      body->BusyFor(3000, body->Tactics());
      if (!targ->IsAct(act_t::FIGHT)) {
        targ->BusyFor(3000, body->Tactics());
        targ->AddAct(act_t::FIGHT, body);
      } else if (targ->StillBusy()) {
        body->BusyWith(targ, body->Tactics());
      }
    } else {
      attacknow = 1; // Uncontested.
    }

    // Draw your weapon, if it's sheathed on your body.
    if (!body->ActTarg(act_t::WIELD)) {
      // Yes, I know, and I agree.  But, this is Fantasy, so you can draw swords from your back.
      for (auto loc :
           {act_t::WEAR_RHIP, act_t::WEAR_RSHOULDER, act_t::WEAR_LHIP, act_t::WEAR_LSHOULDER}) {
        Object* weap = body->ActTarg(loc);
        if (weap && weap->HasSkill(prhash(u8"WeaponType"))) {
          weap->Travel(body->Zone()); // Kills wear actions on "weap"
          weap->Travel(body);
          body->AddAct(act_t::WIELD, weap);
          body->Parent()->SendOut(
              stealth_t, stealth_s, u8";s wields ;s.\n", u8"You wield ;s.\n", body, weap);
          break;
        }
      }
    }

    // Free your off-hand if needed (if it's not a shield or weapon)
    if (body->ActTarg(act_t::HOLD) // FIXME: Don't drop offhand weapons?!?
        && body->ActTarg(act_t::HOLD) != body->ActTarg(act_t::WEAR_SHIELD) &&
        body->ActTarg(act_t::HOLD) != body->ActTarg(act_t::WIELD) &&
        (body->ActTarg(act_t::WEAR_SHIELD) // Need Off-Hand for shield
         || (body->ActTarg(act_t::WIELD) //...or for two-hander
             && two_handed(body->ActTarg(act_t::WIELD)->Skill(prhash(u8"WeaponType")))))) {
      if (body->DropOrStash(body->ActTarg(act_t::HOLD))) {
        if (mind)
          mind->Send(
              u8"Oh, no!  You can't drop or stash {}, but you need your off-hand!",
              body->ActTarg(act_t::HOLD)->Noun(0, 0, body));
      }
    }

    // Hold your 2-hander, even if you have to let go of your shield
    if (body->ActTarg(act_t::WIELD) // Half-Wielding a 2-Hander
        && body->ActTarg(act_t::HOLD) != body->ActTarg(act_t::WIELD) &&
        two_handed(body->ActTarg(act_t::WIELD)->Skill(prhash(u8"WeaponType")))) {
      if (body->ActTarg(act_t::HOLD) // Some non-shield stuck in other hand!
          && body->ActTarg(act_t::HOLD) != body->ActTarg(act_t::WEAR_SHIELD)) {
        if (mind)
          mind->Send(
              u8"Oh, no!  You can't use {} - it's two-handed!\n",
              body->ActTarg(act_t::WIELD)->Noun(0, 0, body));
        if (body->DropOrStash(body->ActTarg(act_t::WIELD))) {
          if (mind)
            mind->Send(
                u8"Oh, no!  You can't drop or stash {}!\n",
                body->ActTarg(act_t::WIELD)->Noun(0, 0, body));
        }
      } else {
        if (body->ActTarg(act_t::HOLD)) { // Unhold your shield
          body->Parent()->SendOut(
              stealth_t,
              stealth_s,
              u8";s stops holding ;s.\n",
              u8"You stop holding ;s.\n",
              body,
              body->ActTarg(act_t::HOLD));
          body->StopAct(act_t::HOLD);
        }
        Object* weap = body->ActTarg(act_t::WIELD); // Hold your 2-hander
        body->AddAct(act_t::HOLD, weap);
        body->Parent()->SendOut(
            stealth_t, stealth_s, u8";s holds ;s.\n", u8"You hold ;s.\n", body, weap);
      }
    }

    // If you're hand's now free, and you have a shield, use it.
    if (body->ActTarg(act_t::WEAR_SHIELD) && (!body->IsAct(act_t::HOLD))) {
      Object* shield = body->ActTarg(act_t::WEAR_SHIELD);

      body->AddAct(act_t::HOLD, shield);
      body->Parent()->SendOut(
          stealth_t, stealth_s, u8";s holds ;s.\n", u8"You hold ;s.\n", body, shield);
    } else if (
        mind && body->ActTarg(act_t::WEAR_SHIELD) &&
        body->ActTarg(act_t::WEAR_SHIELD) != body->ActTarg(act_t::HOLD)) {
      mind->Send(
          u8"Oh, no!  You can't use {} - your off-hand is not free!\n",
          body->ActTarg(act_t::WEAR_SHIELD)->Noun(0, 0, body));
    }

    if (!attacknow) {
      body->Parent()->SendOut(
          stealth_t,
          stealth_s,
          u8";s moves to attack ;s.\n",
          u8"You move to attack ;s.\n",
          body,
          targ);

      // HACK!  Make this command used first rnd!
      body->BusyWith(body, std::u8string(comlist[cnum].command) + u8" " + std::u8string(args));

      return 2; // No more actions until next round!
    }

    // Attacking, or being attacked removes hidden-ness.
    body->ClearSkill(prhash(u8"Hidden"));
    targ->ClearSkill(prhash(u8"Hidden"));

    int reachmod = 0;
    auto sk1 = prhash(u8"Punching");
    auto sk2 = prhash(u8"Punching");

    if (cnum == COM_KICK) {
      sk1 = prhash(u8"Kicking");
      sk2 = prhash(u8"Kicking");
    }

    else if (
        body->ActTarg(act_t::WIELD) // Not Holding your 2-Hander
        && two_handed(body->ActTarg(act_t::WIELD)->Skill(prhash(u8"WeaponType"))) &&
        body->ActTarg(act_t::WIELD) != body->ActTarg(act_t::HOLD)) {
      sk1 = prhash(u8"Kicking");
      sk2 = prhash(u8"Kicking");
    }

    else {
      if (body->IsAct(act_t::WIELD)) {
        sk1 = get_weapon_skill(body->ActTarg(act_t::WIELD)->Skill(prhash(u8"WeaponType")));
        reachmod += std::max(0, body->ActTarg(act_t::WIELD)->Skill(prhash(u8"WeaponReach")));
        if (reachmod > 9)
          reachmod = 0;
      }
      if (body->ActTarg(act_t::HOLD) == body->ActTarg(act_t::WEAR_SHIELD) &&
          body->ActTarg(act_t::HOLD)) {
        sk2 = prhash(u8"None"); // Occupy opponent's primary weapon, so they can't use it to defend.
      }
      if (targ->ActTarg(act_t::HOLD) == targ->ActTarg(act_t::WEAR_SHIELD) &&
          targ->ActTarg(act_t::HOLD)) {
        sk2 = prhash(u8"Shields");
        reachmod = 0; // Shield neutralizes reach
      } else if (
          targ->ActTarg(act_t::WIELD) // Not Holding their 2-Hander
          && two_handed(targ->ActTarg(act_t::WIELD)->Skill(prhash(u8"WeaponType"))) &&
          targ->ActTarg(act_t::WIELD) != targ->ActTarg(act_t::HOLD)) {
        sk2 = prhash(u8"None"); // ...so they can't defend with it
      } else if (targ->ActTarg(act_t::WIELD)) {
        if (sk2 == prhash(u8"Punching"))
          sk2 = get_weapon_skill(targ->ActTarg(act_t::WIELD)->Skill(prhash(u8"WeaponType")));
        reachmod -= std::max(0, targ->ActTarg(act_t::WIELD)->Skill(prhash(u8"WeaponReach")));
        if (reachmod < -9)
          reachmod = 0;
      }
    }

    if (!targ->HasSkill(sk2)) { // Not equipped to defend with a weapon skill, dodge instead
      if (targ->HasSkill(prhash(u8"Dodge"))) {
        sk2 = prhash(u8"Dodge");
        reachmod = 0; // Skilled dodge neutralizes reach
      } else {
        sk2 = prhash(u8"Acrobatics");
      }
    }

    std::u8string rolls_offense = u8"";
    std::u8string rolls_defense = u8"";
    int offense = body->Roll(
        sk1,
        targ->SkillTarget(sk2) - reachmod + body->Modifier(u8"Accuracy") -
            targ->Modifier(u8"Evasion"),
        (is_pc(body) || is_pc(targ)) ? &rolls_offense : nullptr);
    int defense = targ->Roll(
        sk2,
        body->SkillTarget(sk1) + reachmod,
        (is_pc(body) || is_pc(targ)) ? &rolls_defense : nullptr);
    int succ = offense - defense;

    int loc = rand() % 100;
    act_t loca = act_t::WEAR_CHEST;
    std::u8string locm = u8"";
    int stage = 0;
    if (loc < 50) {
      loca = act_t::WEAR_CHEST;
      locm = u8" in the chest";
    } else if (loc < 56) {
      loca = act_t::WEAR_BACK;
      locm = u8" in the back";
    } else if (loc < 59) {
      loca = act_t::WEAR_HEAD;
      locm = u8" in the head";
      stage = 1;
    } else if (loc < 60) {
      loca = act_t::WEAR_FACE;
      locm = u8" in the face";
      stage = 2;
    } else if (loc < 61) {
      loca = act_t::WEAR_NECK;
      locm = u8" in the neck";
      stage = 2;
    } else if (loc < 62) {
      loca = act_t::WEAR_COLLAR;
      locm = u8" in the throat";
      stage = 2;
    } else if (loc < 72) {
      loca = act_t::WEAR_LARM;
      locm = u8" in the left arm";
      stage = -1;
    } else if (loc < 82) {
      loca = act_t::WEAR_RARM;
      locm = u8" in the right arm";
      stage = -1;
    } else if (loc < 86) {
      loca = act_t::WEAR_LLEG;
      locm = u8" in the left leg";
      stage = -1;
    } else if (loc < 90) {
      loca = act_t::WEAR_RLEG;
      locm = u8" in the right leg";
      stage = -1;
    } else if (loc < 93) {
      loca = act_t::WEAR_LHAND;
      locm = u8" in the left hand";
      stage = -2;
    } else if (loc < 96) {
      loca = act_t::WEAR_RHAND;
      locm = u8" in the right hand";
      stage = -2;
    } else if (loc < 98) {
      loca = act_t::WEAR_LFOOT;
      locm = u8" in the left foot";
      stage = -2;
    } else {
      loca = act_t::WEAR_RFOOT;
      locm = u8" in the right foot";
      stage = -2;
    }

    std::u8string verb = u8"punch"; // Non-weapon verb
    std::u8string verb3 = u8"punches"; // 3rd Person
    if (body->Skill(prhash(u8"NaturalWeapon")) == 14) { // Natural Weapon: stab
      verb = u8"stab";
      verb3 = u8"stabs";
    } else if (body->Skill(prhash(u8"NaturalWeapon")) == 13) { // Natural Weapon: hit
      verb = u8"hit";
      verb3 = u8"hits";
    } else if (body->Skill(prhash(u8"NaturalWeapon")) == 12) { // Natural Weapon: blast
      verb = u8"blast";
      verb3 = u8"blasts";
    } else if (body->Skill(prhash(u8"NaturalWeapon")) == 11) { // Natural Weapon: pierce
      verb = u8"pierce";
      verb3 = u8"pierces";
    } else if (body->Skill(prhash(u8"NaturalWeapon")) == 10) { // Natural Weapon: thrash
      verb = u8"thrash";
      verb3 = u8"thrashes";
    } else if (body->Skill(prhash(u8"NaturalWeapon")) == 9) { // Natural Weapon: maul
      verb = u8"maul";
      verb3 = u8"mauls";
    } else if (body->Skill(prhash(u8"NaturalWeapon")) == 8) { // Natural Weapon: claw
      verb = u8"claw";
      verb3 = u8"claws";
    } else if (body->Skill(prhash(u8"NaturalWeapon")) == 7) { // Natural Weapon: pound
      verb = u8"pound";
      verb3 = u8"pounds";
    } else if (body->Skill(prhash(u8"NaturalWeapon")) == 6) { // Natural Weapon: crush
      verb = u8"crush";
      verb3 = u8"crushes";
    } else if (body->Skill(prhash(u8"NaturalWeapon")) == 5) { // Natural Weapon: bludgeon
      verb = u8"bludgeon";
      verb3 = u8"bludgeons";
    } else if (body->Skill(prhash(u8"NaturalWeapon")) == 4) { // Natural Weapon: bite
      verb = u8"bite";
      verb3 = u8"bites";
    } else if (body->Skill(prhash(u8"NaturalWeapon")) == 3) { // Natural Weapon: slash
      verb = u8"slash";
      verb3 = u8"slashes";
    } else if (body->Skill(prhash(u8"NaturalWeapon")) == 2) { // Natural Weapon: whip
      verb = u8"whip";
      verb3 = u8"whips";
    } else if (body->Skill(prhash(u8"NaturalWeapon")) == 1) { // Natural Weapon: sting
      verb = u8"sting";
      verb3 = u8"stings";
    }

    int defense_armor = 0;
    std::u8string rolls_armor = u8"";
    if (succ > 0) {
      int stun = 0;
      if (sk1 == prhash(u8"Kicking")) { // Kicking Action
        stun = 1;
        body->Parent()->SendOut(
            ALL, -1, u8"*;s kicks ;s{}.\n", u8"*You kick ;s{}.\n", body, targ, locm);
      } else if (
          body->IsAct(act_t::WIELD) // Ranged Weapon
          && body->ActTarg(act_t::WIELD)->Skill(prhash(u8"WeaponReach")) > 9) {
        body->Parent()->SendOut(
            ALL,
            -1,
            u8"*;s throws {} and hits ;s{}.\n",
            u8"*You throw {} and hit ;s{}.\n",
            body,
            targ,
            body->ActTarg(act_t::WIELD)->ShortDesc(),
            locm);
        body->ActTarg(act_t::WIELD)->Travel(body->Parent()); // FIXME: Get Another
        body->StopAct(act_t::WIELD); // FIXME: Bows/Guns!
      } else if (body->IsAct(act_t::WIELD)) { // Melee Weapon
        body->Parent()->SendOut(
            ALL,
            -1,
            u8"*;s hits ;s{} with {}.\n",
            u8"*You hit ;s{} with {}.\n",
            body,
            targ,
            locm,
            body->ActTarg(act_t::WIELD)->ShortDesc());
      } else { // No Weapon or Natural Weapon
        if (!body->HasSkill(prhash(u8"NaturalWeapon")))
          stun = 1;
        body->Parent()->SendOut(
            ALL, -1, u8"*;s {2} ;s{0}.\n", u8"*You {1} ;s{0}.\n", body, targ, locm, verb, verb3);
      }

      int sev = 0;
      int force = body->ModAttribute(2) + body->Modifier(u8"Damage");

      if (cnum == COM_KICK) {
        force -= 2;
        stage += 2;
      }
      if (body->IsAct(act_t::WIELD)) {
        force += body->ActTarg(act_t::WIELD)->Skill(prhash(u8"WeaponForce"));
        if (two_handed(body->ActTarg(act_t::WIELD)->Skill(prhash(u8"WeaponType")))) {
          force += body->ModAttribute(2);
        }
        stage += body->ActTarg(act_t::WIELD)->Skill(prhash(u8"WeaponSeverity"));
      } else {
        force -= 1;
        stage += 1;
      }

      if (targ->ActTarg(loca)) {
        // FIXME: Implement the rest of the Armor Effect types
        defense_armor = targ->ActTarg(loca)->Roll(prhash(u8"Body"), force, &rolls_armor);
        succ -= defense_armor;
      }

      if (stun) {
        sev = targ->HitStun(force, stage, succ);
      } else {
        sev = targ->HitPhys(force, stage, succ);
      }
      if (body->Skill(prhash(u8"Poisonous")) > 0) { // Injects poison!
        targ->SetSkill(
            prhash(u8"Poisoned"),
            targ->Skill(prhash(u8"Poisoned")) + body->Skill(prhash(u8"Poisonous")));
      }

      if (sev <= 0) {
        if (mind)
          mind->Send(u8"You hit - but didn't do much.\n"); // FIXME - Real
        // Messages
      }
    } else {
      if (cnum == COM_KICK) { // Kicking Action
        body->Parent()->SendOut(
            ALL,
            -1,
            u8";s tries to kick ;s, but misses.\n",
            u8"You try to kick ;s, but miss.\n",
            body,
            targ);
      } else if (
          body->IsAct(act_t::WIELD) // Ranged Weapon
          && body->ActTarg(act_t::WIELD)->Skill(prhash(u8"WeaponReach")) > 9) {
        body->Parent()->SendOut(
            ALL,
            -1,
            u8"*;s throws {} at ;s, but misses.\n",
            u8"*You throw {} at ;s, but miss.\n",
            body,
            targ,
            body->ActTarg(act_t::WIELD)->ShortDesc());
        body->ActTarg(act_t::WIELD)->Travel(body->Parent()); // FIXME: Get Another
        body->StopAct(act_t::WIELD); // FIXME: Bows/Guns!
      } else if (body->IsAct(act_t::WIELD)) { // Melee Weapon
        body->Parent()->SendOut(
            ALL,
            -1,
            u8";s tries to attack ;s, but misses.\n",
            u8"You try to attack ;s, but miss.\n",
            body,
            targ);
      } else { // Unarmed
        body->Parent()->SendOut(
            ALL,
            -1,
            u8";s tries to {} ;s, but misses.\n",
            u8"You try to {} ;s, but miss.\n",
            body,
            targ,
            verb);
      }
    }

    if (!targ->IsAnimate() || targ->IsAct(act_t::DEAD) || targ->IsAct(act_t::DYING) ||
        targ->IsAct(act_t::UNCONSCIOUS)) {
      body->StopAct(act_t::FIGHT);
      body->BusyFor(3000);
      if (targ->Skill(prhash(u8"Accomplishment"))) {
        body->Accomplish(targ->Skill(prhash(u8"Accomplishment")), u8"this victory");
        targ->ClearSkill(prhash(u8"Accomplishment"));
      }
    }

    if (rolls_armor.empty()) {
      body->Send(CHANNEL_ROLLS, CMAG u8"[{}] = {} - {}\n" CNRM, succ, rolls_offense, rolls_defense);
      targ->Send(CHANNEL_ROLLS, CMAG u8"[{}] = {} - {}\n" CNRM, succ, rolls_offense, rolls_defense);
    } else {
      body->Send(
          CHANNEL_ROLLS,
          CMAG u8"[{}] = {} - {} - {}\n" CNRM,
          succ,
          rolls_offense,
          rolls_defense,
          rolls_armor);
      targ->Send(
          CHANNEL_ROLLS,
          CMAG u8"[{}] = {} - {} - {}\n" CNRM,
          succ,
          rolls_offense,
          rolls_defense,
          rolls_armor);
    }

    return 0;
  }

  static const std::u8string statnames[] = {
      u8"Body", u8"Quickness", u8"Strength", u8"Charisma", u8"Intelligence", u8"Willpower"};

  if (cnum == COM_RESETCHARACTER) {
    Object* chr = mind->Owner()->Creator();
    if (!chr) {
      mind->Send(
          u8"You need to be working on a character first (use 'select "
          u8"<character>'.\n");
      return 0;
    } else if (!args.empty()) {
      mind->Send(
          u8"Just type 'reset' to undo all your work and start over on {}\n", chr->ShortDesc());
      return 0;
    }

    if (body->HasSkill(prhash(u8"Object ID"))) {
      mind->Send(u8"This is not a new character, you can't modify the chargen steps anymore.\n");
      return 0;
    }

    body = new_body(chr->World());
    body->SetDescs(u8"a human adventurer", chr->Name(), u8"", u8"");
    mind->Owner()->AddChar(body);
    delete chr;

    mind->Send(u8"You reset all chargen work for '{}'.\n", body->ShortDesc());
    return 0;
  }

  if (cnum == COM_RANDOMIZE) {
    Object* chr = mind->Owner()->Creator();
    if (!chr) {
      mind->Send(
          u8"You need to be working on a character first (use 'select "
          u8"<character>'.\n");
      return 0;
    } else if (!args.empty()) {
      mind->Send(u8"Just type 'randomize' to randomly spend all points for {}\n", chr->ShortDesc());
      return 0;
    }

    if (body->HasSkill(prhash(u8"Object ID"))) {
      mind->Send(u8"This is not a new character, you can't modify the chargen steps anymore.\n");
      return 0;
    }

    while (chr->Skill(prhash(u8"Attribute Points")) > 0) {
      int which = (rand() % 6);
      if (chr->NormAttribute(which) < 6) {
        chr->SetSkill(prhash(u8"Attribute Points"), chr->Skill(prhash(u8"Attribute Points")) - 1);
        chr->SetAttribute(which, chr->NormAttribute(which) + 1);
      }
    }

    auto skills = get_skills(u8"all");
    while (chr->Skill(prhash(u8"Skill Points"))) {
      int which = (rand() % skills.size());
      auto skl = skills.begin();
      while (which) {
        ++skl;
        --which;
      }
      if (chr->Skill(*skl) < (chr->NormAttribute(get_linked(*skl)) + 1) / 2 &&
          chr->Skill(prhash(u8"Skill Points")) > chr->Skill(*skl)) {
        chr->SetSkill(*skl, chr->Skill(*skl) + 1);
        chr->SetSkill(
            prhash(u8"Skill Points"), chr->Skill(prhash(u8"Skill Points")) - chr->Skill(*skl));
      }
    }
    mind->Send(u8"You randomly spend all remaining points for '{}'.\n", chr->ShortDesc());
    return 0;
  }

  if (cnum == COM_ARCHETYPE) {
    Object* chr = mind->Owner()->Creator();
    if (!chr) {
      mind->Send(
          u8"You need to be working on a character first (use 'select "
          u8"<character>'.\n");
      return 0;
    } else if (args.empty()) {
      mind->Send(u8"You need to select an archetype to apply to {}.\n", chr->ShortDesc());
      mind->Send(u8"Supported archetypes are:\n");
      mind->Send(u8"  1. Fighter\n");
      return 0;
    }

    if (chr->HasSkill(prhash(u8"Object ID"))) {
      mind->Send(u8"This is not a new character, you can't modify the chargen steps anymore.\n");
      return 0;
    }

    if (args == std::u8string_view(u8"fighter").substr(0, args.length())) {
      body = new_body(chr->World());
      body->SetDescs(u8"a human fighter", chr->Name(), u8"", u8"");
      mind->Owner()->AddChar(body);
      delete chr;

      body->SetAttribute(0, 5);
      body->SetAttribute(1, 6);
      body->SetAttribute(2, 6);
      body->SetAttribute(3, 2);
      body->SetAttribute(4, 6);
      body->SetAttribute(5, 5);
      body->ClearSkill(prhash(u8"Attribute Points"));

      body->SetSkill(prhash(u8"Long Blades"), 3);
      body->SetSkill(prhash(u8"Two-Handed Blades"), 3);
      body->SetSkill(prhash(u8"Short Piercing"), 3);
      body->SetSkill(prhash(u8"Shields"), 3);
      body->SetSkill(prhash(u8"Running"), 3);
      body->SetSkill(prhash(u8"Climbing"), 2);
      body->SetSkill(prhash(u8"Sprinting"), 2);
      body->SetSkill(prhash(u8"Swimming"), 2);
      body->SetSkill(prhash(u8"Lifting"), 2);
      body->SetSkill(prhash(u8"Acrobatics"), 2);
      body->SetSkill(prhash(u8"Punching"), 3);
      body->SetSkill(prhash(u8"Kicking"), 3);
      body->SetSkill(prhash(u8"Grappling"), 3);
      body->SetSkill(prhash(u8"Intimidation"), 1);
      body->SetSkill(prhash(u8"Skill Points"), 0);

      auto weap = new Object(body);
      weap->SetShortDesc(u8"a dull arming sword");
      weap->SetDesc(
          u8"This sword really isn't that great.  Is even metal?  It's sure not steel.  "
          u8"At least it includes its own custom scabbard.");
      weap->SetSkill(prhash(u8"WeaponType"), get_weapon_type(u8"Long Blades"));
      weap->SetSkill(prhash(u8"WeaponForce"), -2);
      weap->SetSkill(prhash(u8"WeaponSeverity"), 1);
      weap->SetSkill(prhash(u8"WeaponReach"), 1);
      weap->SetSkill(prhash(u8"Wearable on Right Hip"), 1);
      weap->SetSkill(prhash(u8"Wearable on Left Hip"), 2);
      weap->SetPos(pos_t::LIE);
      body->AddAct(act_t::WIELD, weap);

      auto shi = new Object(body);
      shi->SetShortDesc(u8"a cracked leather shield");
      shi->SetDesc(u8"This shield has seen better days... but, it was pretty bad back then too.");
      shi->SetSkill(prhash(u8"Wearable on Shield"), 1);
      shi->SetAttribute(0, 1);
      shi->SetPos(pos_t::LIE);
      body->AddAct(act_t::WEAR_SHIELD, shi);

      auto arm = new Object(body);
      arm->SetShortDesc(u8"a full suit of old padded armor");
      arm->SetDesc(u8"This armor smells pretty bad, but it's better than nothing... maybe.");
      arm->SetSkill(prhash(u8"Wearable on Back"), 1);
      arm->SetSkill(prhash(u8"Wearable on Chest"), 1);
      arm->SetSkill(prhash(u8"Wearable on Left Arm"), 1);
      arm->SetSkill(prhash(u8"Wearable on Right Arm"), 1);
      arm->SetSkill(prhash(u8"Wearable on Left Leg"), 1);
      arm->SetSkill(prhash(u8"Wearable on Right Leg"), 1);
      arm->SetAttribute(0, 1);
      arm->SetPos(pos_t::LIE);
      body->AddAct(act_t::WEAR_BACK, arm);
      body->AddAct(act_t::WEAR_CHEST, arm);
      body->AddAct(act_t::WEAR_LARM, arm);
      body->AddAct(act_t::WEAR_RARM, arm);
      body->AddAct(act_t::WEAR_LLEG, arm);
      body->AddAct(act_t::WEAR_RLEG, arm);

      auto helm = new Object(body);
      helm->SetShortDesc(u8"a soft leather cap");
      helm->SetDesc(u8"This is... armor... probably.");
      helm->SetSkill(prhash(u8"Wearable on Head"), 1);
      helm->SetAttribute(0, 1);
      helm->SetPos(pos_t::LIE);
      body->AddAct(act_t::WEAR_HEAD, helm);

      body->ClearSkill(prhash(u8"Status Points"));

      mind->Send(u8"You reform '{}' into a {}.\n", body->Name(), u8"Fighter");
      mind->Send(u8"You can now adjust things from here, or just enter the game.\n");

    } else {
      mind->Send(
          u8"You need to select a *supported* archetype to apply to {}.\n", chr->ShortDesc());
      mind->Send(u8"Supported archetypes are:\n");
      mind->Send(u8"  1. Fighter\n");
    }

    return 0;
  }

  if (cnum == COM_LOWER) {
    if ((!mind) || (!mind->Owner()))
      return 0;

    Object* chr = mind->Owner()->Creator();
    if (!chr) {
      mind->Send(u8"You need to be working on a character first (use 'select <character>').\n");
      return 0;
    }

    if (chr->HasSkill(prhash(u8"Object ID"))) {
      mind->Send(u8"This is not a new character, you can't modify the chargen steps anymore.\n");
      return 0;
    }

    if (args == std::u8string_view(u8"body").substr(0, args.length()) ||
        args == std::u8string_view(u8"quickness").substr(0, args.length()) ||
        args == std::u8string_view(u8"strength").substr(0, args.length()) ||
        args == std::u8string_view(u8"charisma").substr(0, args.length()) ||
        args == std::u8string_view(u8"intelligence").substr(0, args.length()) ||
        args == std::u8string_view(u8"willpower").substr(0, args.length())) {
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
        mind->Send(u8"Your {} is already at the minimum.\n", statnames[attr]);
        return 0;
      } else {
        chr->SetAttribute(attr, chr->NormAttribute(attr) - 1);
        chr->SetSkill(prhash(u8"Attribute Points"), chr->Skill(prhash(u8"Attribute Points")) + 1);
        mind->Send(u8"You lower your {}.\n", statnames[attr]);
      }
    } else {
      auto skill = get_skill(std::u8string(args));
      if (skill != prhash(u8"None")) {
        if (chr->Skill(skill) < 1) {
          mind->Send(u8"You don't have {}.\n", SkillName(skill));
          return 0;
        }
        chr->SetSkill(skill, chr->Skill(skill) - 1);
        chr->SetSkill(
            prhash(u8"Skill Points"), chr->Skill(prhash(u8"Skill Points")) + chr->Skill(skill) + 1);
        mind->Send(u8"You lower your {} skill.\n", SkillName(skill));
      } else {
        mind->Send(u8"I'm not sure what you are trying to lower.\n");
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
        mind->Send(u8"You need to be working on a character first (use 'select <character>').\n");
        return 0;
      } else if (chr->HasSkill(prhash(u8"Object ID"))) {
        mind->Send(u8"This is not a new character, you can't modify the chargen steps anymore.\n");
        return 0;
      }
    }
    if (args.empty()) {
      mind->Send(u8"What do you want to buy?\n");
      return 0;
    }

    if (args == std::u8string_view(u8"body").substr(0, args.length()) ||
        args == std::u8string_view(u8"quickness").substr(0, args.length()) ||
        args == std::u8string_view(u8"strength").substr(0, args.length()) ||
        args == std::u8string_view(u8"charisma").substr(0, args.length()) ||
        args == std::u8string_view(u8"intelligence").substr(0, args.length()) ||
        args == std::u8string_view(u8"willpower").substr(0, args.length())) {
      if ((!body) && (chr->Skill(prhash(u8"Attribute Points")) < 1)) {
        mind->Send(u8"You have no free attribute points left.\n");
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
        mind->Send(
            u8"You don't have enough experience to raise your {}.\n"
            u8"You need 20, but you only have {}\n",
            statnames[attr],
            chr->TotalExp());
        return 0;
      }

      uint32_t maxask[6] = {
          prhash(u8"MaxBody"),
          prhash(u8"MaxQuickness"),
          prhash(u8"MaxStrength"),
          prhash(u8"MaxCharisma"),
          prhash(u8"MaxIntelligence"),
          prhash(u8"MaxWillpower")};
      if ((body) && (!body->Skill(maxask[attr]))) {
        body->SetSkill(maxask[attr], (body->NormAttribute(attr) * 3) / 2);
      }

      if ((!body) && chr->NormAttribute(attr) >= 6) {
        mind->Send(u8"Your {} is already at the maximum.\n", statnames[attr]);
      } else if (body && chr->NormAttribute(attr) >= body->Skill(maxask[attr])) {
        mind->Send(u8"Your {} is already at the maximum.\n", statnames[attr]);
      } else {
        if (!body)
          chr->SetSkill(prhash(u8"Attribute Points"), chr->Skill(prhash(u8"Attribute Points")) - 1);
        else
          chr->SpendExp(20);
        chr->SetAttribute(attr, chr->NormAttribute(attr) + 1);
        mind->Send(u8"You raise your {}.\n", statnames[attr]);
      }
    } else {
      auto skill = get_skill(std::u8string(args));
      if (skill != prhash(u8"None")) {
        if (body && (chr->Skill(skill) >= (chr->NormAttribute(get_linked(skill)) * 3 + 1) / 2)) {
          mind->Send(u8"Your {} is already at the maximum.\n", SkillName(skill));
          return 0;
        } else if (
            (!body) && (chr->Skill(skill) >= (chr->NormAttribute(get_linked(skill)) + 1) / 2)) {
          mind->Send(u8"Your {} is already at the maximum.\n", SkillName(skill));
          return 0;
        }
        int cost = (chr->Skill(skill) + 1);
        if (body) {
          if (cost > chr->NormAttribute(get_linked(skill)))
            cost *= 2;
          if (chr->TotalExp() < (cost * 2)) {
            mind->Send(
                u8"You don't have enough experience to raise your {}.\n"
                u8"You need {}, but you only have {}\n",
                SkillName(skill),
                cost * 2,
                chr->TotalExp());
            return 0;
          }
        } else if (!chr->Skill(prhash(u8"Skill Points"))) {
          mind->Send(u8"You have no free skill points left.\n");
          return 0;
        } else if (chr->Skill(prhash(u8"Skill Points")) < cost) {
          mind->Send(u8"You don't have enough free skill points left.\n");
          return 0;
        }
        if (body)
          chr->SpendExp(cost * 2);
        else
          chr->SetSkill(prhash(u8"Skill Points"), chr->Skill(prhash(u8"Skill Points")) - cost);
        chr->SetSkill(skill, chr->Skill(skill) + 1);
        mind->Send(u8"You raise your {} skill.\n", SkillName(skill));
      } else {
        mind->Send(u8"I'm not sure what you are trying to raise.\n");
      }
    }
    return 0;
  }

  if (cnum == COM_NEWCHARACTER) {
    if (!mind)
      return 0; // FIXME: Should never happen!
    if (args.empty()) {
      mind->Send(u8"What's the character's name?  Use 'newcharacter <charname>'.\n");
      return 0;
    }

    if (!std::all_of(args.begin(), args.end(), ascii_isalpha)) {
      mind->Send(
          u8"Sorry, character names can only contain letters.\n"
          u8"Please pick another name.\n");
      return 0;
    }

    if (args.length() > 32) { // Max reasonable lenght?  Arbitrary.
      mind->Send(
          u8"Sorry, character names can only be up to 32 letters long.\n"
          u8"Please pick another name.\n");
      return 0;
    }

    body = mind->Owner()->Room()->PickObject(std::u8string(args), vmode | LOC_INTERNAL);
    if (body) {
      mind->Send(
          u8"Sorry, you already have a character with that name.\n"
          u8"Please pick another name.\n");
      return 0;
    }

    if (!mind->Owner()->World()) {
      if (Object::Universe()->IsAct(act_t::SPECIAL_HOME)) {
        mind->Owner()->SetWorld(Object::Universe()->ActTarg(act_t::SPECIAL_HOME));
        mind->Send(
            u8"Automatically selecting the default world first.\n"
            u8"Try the 'world' command to select a different one.\n");
      }
    }

    if (!mind->Owner()->World()) {
      mind->Send(
          u8"Sorry, you need to select a world first.\n"
          u8"Try the 'world' command.\n");
      return 0;
    }

    body = new_body(mind->Owner()->World());
    body->SetDescs(u8"a human adventurer", std::u8string(args), u8"", u8"");
    mind->Owner()->AddChar(body);
    mind->Send(u8"You created {}.\n", std::u8string(args));
    return 0;
  }

  if (cnum == COM_RECALL) {
    if (body->Phys() || body->Stun()) {
      if (mind)
        mind->Send(u8"You must be uninjured to use that command!\n");
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
          u8"BAMF! ;s teleports away.\n",
          u8"BAMF! You teleport home.\n",
          body,
          nullptr);
      body->Travel(dest);
      body->Parent()->SendOut(
          0,
          0, // Not Stealthy!
          u8"BAMF! ;s teleports here.\n",
          u8"",
          body,
          nullptr);
      if (mind && mind->Type() == mind_t::REMOTE)
        body->Parent()->SendDescSurround(body, body);
    }
    return 0;
  }

  if (cnum == COM_TELEPORT) {
    if (args.empty()) {
      mind->Send(u8"Where do you want to teleport to?.\n");
      return 0;
    }

    Object* src = nullptr;
    if (!nmode)
      src = body->NextHasSkill(prhash(u8"Restricted Item"));
    while (src) {
      if (!src->HasSkill(prhash(u8"Teleport"))) {
        src = body->NextHasSkill(prhash(u8"Restricted Item"), src);
        continue;
      }
      std::u8string comline = u8"teleport ";
      comline += (std::u8string(args));
      comline += u8"\n";
      if (src->LongDesc().contains(comline)) {
        src = body->NextHasSkill(prhash(u8"Restricted Item"), src);
        continue;
      }
      break;
    }

    if ((!nmode) && (!src) && body->Skill(prhash(u8"Teleport")) < 1) {
      if (mind)
        mind->Send(u8"You don't have the power to teleport!\n");
      return 0;
    }
    if ((!body->Parent()) || (!body->Parent()->Parent())) {
      if (mind)
        mind->Send(u8"You can't teleport from here!\n");
      return 0;
    }

    Object* dest = body->Parent();
    if (nmode && args == u8"universe") {
      dest = dest->Universe();
    } else if (nmode && args == u8"trashbin") {
      dest = dest->TrashBin();
    } else { // Only Ninjas can teleport to u8"Universe"/"TrashBin"
      while (dest->Parent()->Parent()) {
        dest = dest->Parent();
      }
      dest = dest->PickObject(std::u8string(args), vmode | LOC_INTERNAL);
    }

    if (!dest) {
      if (mind)
        mind->Send(u8"No such teleportation destination found.\n");
    } else {
      body->ClearSkill(prhash(u8"Teleport")); // Use it up
      body->Parent()->SendOut(
          0,
          0, // Not Stealthy!
          u8"BAMF! ;s teleports away.\n",
          u8"BAMF! You teleport.\n",
          body,
          nullptr);
      body->Travel(dest);
      body->Parent()->SendOut(
          0,
          0, // Not Stealthy!
          u8"BAMF! ;s teleports here.\n",
          u8"",
          body,
          nullptr);
      if (mind && mind->Type() == mind_t::REMOTE)
        body->Parent()->SendDescSurround(body, body);
    }
    return 0;
  }

  if (cnum == COM_RESURRECT) {
    if ((!nmode) && body->Skill(prhash(u8"Resurrect")) < 1) {
      if (mind)
        mind->Send(u8"You don't have the power to resurrect!\n");
      return 0;
    }
    if (args.empty()) {
      mind->Send(u8"Who do you want to resurrect?.\n");
      return 0;
    }

    std::vector<Player*> pls = get_all_players();
    for (auto pl : pls) {
      auto chs = pl->Room()->Contents();
      for (auto ch : chs) {
        if (ch->Matches(std::u8string(args))) {
          if (ch->IsActive()) {
            if (mind)
              mind->Send(u8"{} is not long dead (yet).\n", ch->Noun());
          } else {
            body->ClearSkill(prhash(u8"Resurrect")); // Use it up
            ch->ClearSkill(prhash(u8"Poisoned"));
            ch->ClearSkill(prhash(u8"Thirsty"));
            ch->ClearSkill(prhash(u8"Hungry"));
            ch->SetPhys(0);
            ch->SetStun(0);
            ch->SetStru(0);
            ch->UpdateDamage();
            ch->Activate();
            ch->Parent()->SendOut(
                stealth_t, stealth_s, u8";s has been resurrected!.\n", u8"", ch, nullptr);
            if (mind)
              mind->Send(u8"{} has been resurrected!\n", ch->Noun());
          }
          return 0;
        }
      }
    }
    if (mind)
      mind->Send(u8"{} isn't a character on this MUD\n", std::u8string(args));
    return 0;
  }

  if (cnum == COM_SKILLLIST) {
    if (!mind)
      return 0;

    std::u8string skills;
    std::vector<uint32_t> skls;
    if (args.empty()) {
      skills = u8"Here are all the skill categories (use 'skill <Category>' to see the skills):\n";
      skls = get_skills();
    } else {
      std::u8string cat = get_skill_cat(std::u8string(args));
      if (args != u8"all") {
        if (cat == u8"") {
          mind->Send(u8"There is no skill category called '{}'.\n", std::u8string(args));
          return 0;
        }
        skills = u8"Total " + cat + u8" in play on this MUD:\n";
      } else {
        skills = u8"Total skills in play on this MUD:\n";
        cat = u8"all";
      }
      skls = get_skills(cat);
    }

    std::vector<std::u8string> sknms;
    for (auto skl : skls) {
      sknms.push_back(SkillName(skl));
    }
    std::sort(sknms.begin(), sknms.end());
    for (auto skn : sknms) {
      skills += skn;
      skills += u8"\n";
    }
    mind->Send(skills);

    return 0;
  }

  if (cnum == COM_TOGGLE) {
    if (!mind)
      return 0;
    Player* pl = mind->Owner();
    if (!pl)
      return 0;

    if (!args.empty() && args == std::u8string_view(u8"restore").substr(0, args.length())) {
      mind->SetSVars(pl->Vars());
      mind->Send(u8"Your settings have been reset to your defaults.\n");
      args = u8""; // Show current settings too
    }

    if (args.empty()) {
      mind->Send(u8"Your current settings:\n");

      if (mind->IsSVar(u8"combatinfo")) {
        mind->Send(u8"  CombatInfo is " CYEL u8"on" CNRM u8".\n");
      } else {
        mind->Send(u8"  CombatInfo is " CYEL u8"off" CNRM u8".\n");
      }

    } else if (args == std::u8string_view(u8"combatinfo").substr(0, args.length())) {
      if (mind->IsSVar(u8"combatinfo")) {
        mind->ClearSVar(u8"combatinfo");
        mind->Send(u8"CombatInfo is now " CYEL u8"off" CNRM u8".\n");
      } else {
        mind->SetSVar(u8"combatinfo", u8"1");
        mind->Send(u8"CombatInfo is now " CYEL u8"on" CNRM u8".\n");
      }
    } else if (args == std::u8string_view(u8"save").substr(0, args.length())) {
      pl->SetVars(mind->SVars());
      mind->Send(u8"Your current settings have been saved as your defaults.\n");
    } else {
      mind->Send(u8"Don't know what setting you want to change.\n");
      mind->Send(u8"Just type " CYEL u8"toggle" CNRM u8" to see a full list.\n");
      return 0;
    }
    if (mind->SVars() != pl->Vars()) {
      mind->Send(CYEL u8"\nYour current settings are not all saved:\n" CNRM);
      mind->Send(u8"  Type '" CMAG u8"toggle save" CNRM u8"' to make this change permanent.\n");
      mind->Send(
          u8"  Type '" CMAG u8"toggle restore" CNRM u8"' to restore your default settings.\n");
    }
    return 0;
  }

  if (cnum == COM_WHO) {
    if (!mind)
      return 0;
    std::u8string users = u8"Currently on this MUD:\n";
    std::vector<std::shared_ptr<Mind>> mns = get_human_minds();

    for (auto mn : mns) {
      users += mn->Owner()->Name();
      if (mn->Body())
        users = fmt::format(u8"{} as {}.\n", users, mn->Body()->ShortDesc());
      else
        users += u8" in character room.\n";
    }
    mind->Send(users);

    return 0;
  }

  if (cnum == COM_OOC) {
    if (!mind)
      return 0;
    if (args.empty()) {
      if (!mind->SpecialPrompt().starts_with(u8"ooc")) {
        mind->SetSpecialPrompt(u8"ooc");
        mind->Send(u8"Type your out-of-character text - exit by just hitting ENTER:");
      } else {
        mind->SetSpecialPrompt(u8"");
        mind->Send(u8"Exiting out of out-of-character mode.");
      }
    } else {
      std::u8string name = u8"Unknown";
      if (mind->Owner())
        name = mind->Owner()->Name();
      std::u8string mes =
          std::u8string(u8"OOC: <") + name + u8"> " + (std::u8string(args)) + u8"\n";
      std::vector<std::shared_ptr<Mind>> mns = get_human_minds();
      for (auto mn : mns) {
        mn->Send(mes);
      }
    }
    return 0;
  }

  if (cnum == COM_NEWBIE) {
    if (!mind)
      return 0;
    if (args.empty()) {
      if (!mind->SpecialPrompt().starts_with(u8"newbie")) {
        mind->SetSpecialPrompt(u8"newbie");
        mind->Send(u8"Type your newbie-chat text - exit by just hitting ENTER:");
      } else {
        mind->SetSpecialPrompt(u8"");
        mind->Send(u8"Exiting out of newbie-chat mode.");
      }
    } else {
      std::u8string name = u8"Unknown";
      if (mind->Owner())
        name = mind->Owner()->Name();
      std::u8string mes =
          std::u8string(u8"NEWBIE: <") + name + u8"> " + (std::u8string(args)) + u8"\n";
      std::vector<std::shared_ptr<Mind>> mns = get_human_minds();
      for (auto mn : mns) {
        mn->Send(mes);
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
      mind->Send(u8"What command do you want to run in Ninja Mode[TM]?\n");
      mind->Send(u8"You need to include the command, like 'ninja junk boat'.\n");
    } else if (!pl) {
      mind->Send(u8"Sorry, you don't seem to be a player!\n");
    } else {
      pl->Set(PLAYER_NINJAMODE);
      mind->Send(u8"Ninja mode activated.\n");
      handle_single_command(body, std::u8string(args), mind);
      pl->UnSet(PLAYER_NINJAMODE);
      mind->Send(u8"Ninja mode deactivated.\n");
    }
    return 0;
  }

  if (cnum == COM_MAKENINJA) {
    if (!mind)
      return 0;

    Player* pl = get_player(std::u8string(args));
    if (args.empty()) {
      mind->Send(u8"You can only make/unmake other True Ninjas[TM]\n");
      return 0;
    }
    if (!pl) {
      mind->Send(u8"There is no PLAYER named '{}'\n", std::u8string(args));
      return 0;
    }
    if (pl == mind->Owner()) {
      mind->Send(u8"You can only make/unmake other True Ninjas[TM]\n");
      return 0;
    }

    if (pl->Is(PLAYER_SUPERNINJA)) {
      mind->Send(u8"'{}' is already a Super Ninja[TM] - this is irrevocable.\n", args);
    } else if (pl->Is(PLAYER_NINJA)) {
      pl->UnSet(PLAYER_NINJA);
      pl->UnSet(PLAYER_NINJAMODE);
      mind->Send(u8"Now '{}' is no longer a True Ninja[TM].\n", std::u8string(args));
    } else {
      pl->Set(PLAYER_NINJA);
      mind->Send(u8"You made '{}' into a True Ninja[TM].\n", std::u8string(args));
    }

    return 0;
  }

  if (cnum == COM_MAKESUPERNINJA) {
    if (!mind)
      return 0;

    Player* pl = get_player(std::u8string(args));
    if (args.empty()) {
      mind->Send(u8"You can only make other Super Ninjas[TM]\n");
      return 0;
    }
    if (!pl) {
      mind->Send(u8"There is no PLAYER named '{}'\n", std::u8string(args));
      return 0;
    }
    if (pl == mind->Owner()) {
      mind->Send(u8"You can only make other Super Ninjas[TM]\n");
      return 0;
    }

    if (pl->Is(PLAYER_SUPERNINJA)) {
      mind->Send(u8"'{}' is already a Super Ninja[TM] - this is irrevocable.\n", args);
    } else if (!pl->Is(PLAYER_NINJA)) {
      mind->Send(
          u8"'{}' isn't even a True Ninja[TM] yet!\n"
          u8"Be careful - Super Ninja[TM] status is irrevocable.\n",
          args);
    } else {
      pl->Set(PLAYER_SUPERNINJA);
      mind->Send(u8"You made '{}' into a Super Ninja[TM].\n", std::u8string(args));
    }

    return 0;
  }

  if (cnum == COM_MAKESTART) {
    if (!mind)
      return 0;
    Object* world = body->World();
    world->AddAct(act_t::SPECIAL_HOME, body->Parent());
    world->Parent()->AddAct(act_t::SPECIAL_HOME, body->Parent());
    mind->Send(u8"You make this the default starting room for players.\n");
    return 0;
  }

  if (cnum == COM_SETSTATS) {
    if (!mind)
      return 0;
    Object* targ = body->ActTarg(act_t::POINT);
    if (!targ) {
      mind->Send(u8"You need to be pointing at your target.\n");
      return 0;
    }
    if (!args.empty()) {
      char8_t gchar = 'N';
      int weight = 0, size = 0, volume = 0, value = 0;
      weight = nextnum(args);
      skipspace(args);
      size = nextnum(args);
      skipspace(args);
      volume = nextnum(args);
      skipspace(args);
      value = nextnum(args);
      skipspace(args);
      if (args.length() != 1) {
        mind->Send(u8"You need to specify 5 stats (weight, size, volume, value, gender)\n");
        mind->Send(u8"Example: " CCYN u8"setstats 100000 1800 2000 0 F\n" CNRM);
        return 0;
      }
      gchar = nextchar(args);
      targ->SetWeight(weight);
      targ->SetSize(size);
      targ->SetVolume(volume);
      targ->SetValue(value);
      if (ascii_toupper(gchar) == 'N') {
        targ->SetGender(gender_t::NONE);
      } else if (ascii_toupper(gchar) == 'F') {
        targ->SetGender(gender_t::FEMALE);
      } else if (ascii_toupper(gchar) == 'M') {
        targ->SetGender(gender_t::MALE);
      } else {
        targ->SetGender(gender_t::NEITHER);
      }
    } else {
      mind->Send(u8"Set stats to what?\n");
    }
    return 0;
  }

  if (cnum == COM_BRIEF) {
    if (!mind)
      return 0;
    Object* targ = body->ActTarg(act_t::POINT);
    if (!targ) {
      mind->Send(u8"You need to be pointing at your target.\n");
      return 0;
    }
    if (!args.empty()) {
      std::u8string oldn(targ->ShortDesc());
      targ->SetShortDesc(std::u8string(args));
      mind->Send(u8"You re-brief '{}' to '{}'\n", oldn,
                 targ->ShortDesc()); // FIXME - Real Message
    } else {
      mind->Send(u8"Re-brief it to what?\n");
    }
    return 0;
  }

  if (cnum == COM_NAME) {
    if (!mind)
      return 0;
    Object* targ = body->ActTarg(act_t::POINT);
    if (!targ) {
      mind->Send(u8"You need to be pointing at your target.\n");
      return 0;
    }
    if (!args.empty()) {
      std::u8string oldn(targ->Name());
      targ->SetName(std::u8string(args));
      mind->Send(u8"You rename '{}' to '{}'\n", oldn,
                 targ->Name()); // FIXME - Real Message
    } else {
      mind->Send(u8"Rename it to what?\n");
    }
    return 0;
  }

  if (cnum == COM_UNNAME) {
    if (!mind)
      return 0;
    Object* targ = body->ActTarg(act_t::POINT);
    if (!targ) {
      mind->Send(u8"You need to be pointing at your target.\n");
      return 0;
    }
    targ->SetName(u8"");
    mind->Send(u8"You remove the name from '{}'\n", targ->Noun(0, 0, body));
    return 0;
  }

  if (cnum == COM_UNDESCRIBE) {
    if (!mind)
      return 0;
    Object* targ = body->ActTarg(act_t::POINT);
    if (!targ) {
      mind->Send(u8"You need to be pointing at your target.\n");
      return 0;
    }
    targ->SetDesc(u8"");
    mind->Send(u8"You remove the description from '{}'\n", targ->Noun(0, 0, body));
    return 0;
  }

  if (cnum == COM_DESCRIBE) {
    if (!mind)
      return 0;
    Object* targ = body->ActTarg(act_t::POINT);
    if (!targ) {
      mind->Send(u8"You need to be pointing at your target.\n");
      return 0;
    }
    if (!args.empty()) {
      if (targ->Desc() == targ->ShortDesc()) {
        targ->SetDesc(std::u8string(args));
        mind->Send(u8"You add a description to '{}'\n", targ->Noun(0, 0, body));
      } else {
        targ->SetDesc(fmt::format(u8"{}\n{}", targ->Desc(), args));
        mind->Send(u8"You add to the description of '{}'\n", targ->Noun(0, 0, body));
      }
    } else {
      if (!mind->SpecialPrompt().starts_with(u8"nin des")) {
        mind->SetSpecialPrompt(u8"nin des");
        mind->Send(u8"Type the description - exit by just hitting ENTER:");
      } else {
        mind->SetSpecialPrompt(u8"");
        mind->Send(u8"Exiting out of describe mode.");
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
      mind->Send(u8"You need to be pointing at your target.\n");
      return 0;
    }
    targ->SetLongDesc(u8"");
    mind->Send(u8"You remove the definition from '{}'\n", targ->Noun(0, 0, body));
    return 0;
  }

  if (cnum == COM_DEFINE) {
    if (!mind)
      return 0;
    Object* targ = body->ActTarg(act_t::POINT);
    if (!targ) {
      mind->Send(u8"You need to be pointing at your target.\n");
      return 0;
    }
    if (!args.empty()) {
      if (targ->LongDesc() == targ->Desc()) {
        targ->SetLongDesc(std::u8string(args));
        mind->Send(u8"You add a definition to '{}'\n", targ->Noun(0, 0, body));
      } else {
        targ->SetLongDesc(fmt::format(u8"{}\n{}", targ->LongDesc(), args));
        mind->Send(u8"You add to the definition of '{}'\n", targ->Noun(0, 0, body));
      }
    } else {
      if (!mind->SpecialPrompt().starts_with(u8"nin def")) {
        mind->SetSpecialPrompt(u8"nin def");
        mind->Send(u8"Type the definition - exit by just hitting ENTER:");
      } else {
        mind->SetSpecialPrompt(u8"");
        mind->Send(u8"Exiting out of define mode.");
      }
      return 0;
    }
    return 0;
  }

  if (cnum == COM_CONTROL) {
    if (!mind)
      return 0;
    Object* targ = body->PickObject(std::u8string(args), vmode | LOC_NEARBY);
    if (!targ) {
      mind->Send(u8"You want to control who?\n");
    } else if (!targ->IsAnimate()) {
      mind->Send(u8"You can't control inanimate objects!\n");
    } else {
      body->Parent()->SendOut(
          stealth_t,
          stealth_s,
          u8";s controls ;s with Ninja Powers[TM].\n",
          u8"You control ;s.\n",
          body,
          targ);
      body->Detach(mind);
      targ->Attach(mind);
    }
    return 0;
  }

  if (cnum == COM_COMMAND) {
    if (!mind)
      return 0;
    Object* targ = body->ActTarg(act_t::POINT);
    if (!targ) {
      mind->Send(u8"You need to be pointing at your target.\n");
    } else if (args.empty()) {
      mind->Send(u8"Command {} to do what?\n", targ->ShortDesc());
    } else if (targ->NormAttribute(5) <= 0) {
      mind->Send(u8"You can't command an object that has no will of its own.\n");
    } else {
      body->Parent()->SendOut(
          stealth_t,
          stealth_s,
          u8";s commands ;s to '{}' with Ninja Powers[TM].\n",
          u8"You command ;s to '{}'.\n",
          body,
          targ,
          args);

      if (handle_command(targ, std::u8string(args)) > 0)
        body->Parent()->SendOut(
            stealth_t,
            stealth_s,
            u8";s did not understand the command.\n",
            u8";s did not understand the command.\n",
            targ,
            body);
    }
    return 0;
  }

  if (cnum == COM_CONNECT) {
    Object *src = body->ActTarg(act_t::POINT), *dest = nullptr;
    if (!src) {
      mind->Send(u8"You need to be pointing at your source.\n");
      return 0;
    }
    dest = body->PickObject(
        std::u8string(args), vmode | LOC_INTERNAL | LOC_NEARBY | LOC_SELF | LOC_HERE);
    if (!dest) {
      if (mind)
        mind->Send(u8"I can't find that destination.\n");
    } else {
      Object* exit = new Object(dest);
      exit->SetShortDesc(u8"a passage exit");
      exit->SetDesc(u8"A passage exit.");
      exit->SetSkill(prhash(u8"Invisible"), 1000);
      src->SetSkill(prhash(u8"Open"), 1000);
      src->SetSkill(prhash(u8"Enterable"), 1);
      src->AddAct(act_t::SPECIAL_LINKED, exit);
      if (mind) {
        mind->Send(u8"You link {} to {}.\n", src->Noun(0, 0, body), dest->Noun(0, 0, body));
      }
    }
    return 0;
  }

  if (cnum == COM_CREATE) {
    if (!mind)
      return 0;
    Object* obj = new Object(body->Parent());
    if (!args.empty()) {
      obj->SetShortDesc(std::u8string(args));
    }
    body->Parent()->SendOut(
        stealth_t,
        stealth_s,
        u8";s creates ;s with Ninja Powers[TM].\n",
        u8"You create ;s.\n",
        body,
        obj);
    return 0;
  }

  if (cnum == COM_DCREATE) {
    if (!mind)
      return 0;
    if (args.empty()) {
      mind->Send(u8"You need to specify in what direction!\n");
    } else {
      Object* box = new Object(body->World()); // It's in its own zone.
      Object* next = new Object(box);
      std::u8string dirb = u8"south";
      std::u8string dir = u8"north";
      if (args == u8"north") {
      } else if (args == u8"south") {
        dirb = u8"north";
        dir = u8"south";
      } else if (args == u8"west") {
        dirb = u8"east";
        dir = u8"west";
      } else if (args == u8"east") {
        dirb = u8"west";
        dir = u8"east";
      } else if (args == u8"up") {
        dirb = u8"down";
        dir = u8"up";
      } else if (args == u8"down") {
        dirb = u8"up";
        dir = u8"down";
      } else {
        mind->Send(u8"Direction '{}' not meaningful!\n", std::u8string(args));
        return 0;
      }

      box->SetShortDesc(u8"a dynamic dungeon");
      next->SetShortDesc(u8"An Entrance to a Large Mining Tunnel");
      next->SetDesc(
          u8"This tunnel looks to have been carved centuries ago.  It is so well crafted\n"
          u8"that you think it will stand as-is for another millenium.\n");
      next->SetSkill(prhash(u8"DynamicInit"), 1);
      next->ClearSkill(prhash(u8"DynamicPhase")); // Entrance
      next->SetSkill(prhash(u8"DynamicMojo"), 1000000);

      body->Parent()->Link(
          next,
          dir,
          std::u8string(u8"You see a solid passage leading ") + dir + u8".\n",
          dirb,
          std::u8string(u8"You see a solid passage leading ") + dirb + u8".\n");

      body->World()->LoadTagsFrom( // Load the special tags for this scenario into this world.
          u8"tag:npc:dungeon_cage_key\n"
          u8"type:none\n"
          u8"itag:dungeon_cage_key\n"
          u8"tag:item:dungeon_cage_key\n"
          u8"short:a key\n"
          u8"desc:A heavy steel key.  It looks dwarven, and very old.\n"
          u8"prop:Key:1510003\n"
          u8"weight:100\n"
          u8"size:10\n"
          u8"volume:20\n");

      body->Parent()->SendOut(
          stealth_t,
          stealth_s,
          u8";s creates a new dynamic dungeon '{}' with Ninja Powers[TM].\n",
          u8"You create a new dynamic dungeon '{}'.\n",
          body,
          nullptr,
          dir);
    }
    return 0;
  }

  static Object* anchor = nullptr;

  if (cnum == COM_ANCHOR) {
    if (!mind)
      return 0;
    anchor = new Object(body->Parent());
    anchor->SetShortDesc(u8"a shimmering portal");
    anchor->SetDesc(
        u8"This portal could only have been created by a True "
        u8"Ninja[TM].  You wonder where it leads.");
    body->Parent()->SendOut(
        stealth_t,
        stealth_s,
        u8";s creates a shimmering portal with Ninja Powers[TM].\n",
        u8"You create a shimmering portal.\n",
        body,
        nullptr);
    return 0;
  }

  if (cnum == COM_LINK) {
    if (!mind)
      return 0;
    if (anchor == nullptr) {
      mind->Send(u8"You need to make an anchor before you can link to it!\n");
    } else if (args.empty()) {
      mind->Send(u8"You need to specify what the portal will be named!\n");
    } else {
      Object* link;

      body->Parent()->SendOut(
          stealth_t,
          stealth_s,
          u8";s creates a shimmering portal '{}' with Ninja Powers[TM].\n",
          u8"You create a shimmering portal '{}'.\n",
          body,
          nullptr,
          args);

      link = new Object(body->Parent());
      link->SetShortDesc(std::u8string(args));
      link->AddAct(act_t::SPECIAL_LINKED, anchor);
      link->SetSkill(prhash(u8"Open"), 1000);
      link->SetSkill(prhash(u8"Enterable"), 1);
      anchor->AddAct(act_t::SPECIAL_LINKED, link);
      anchor->SetSkill(prhash(u8"Open"), 1000);
      anchor->SetSkill(prhash(u8"Enterable"), 1);
      std::u8string other = std::u8string(args);
      if (args == u8"east") {
        other = u8"west";
        other += args.substr(0, 4);
      } else if (args == u8"west") {
        other = u8"east";
        other += args.substr(0, 4);
      } else if (args == u8"north") {
        other = u8"south";
        other += args.substr(0, 5);
      } else if (args == u8"south") {
        other = u8"north";
        other += args.substr(0, 5);
      } else if (args == u8"up") {
        other = u8"down";
        other += args.substr(0, 2);
      } else if (args == u8"down") {
        other = u8"up";
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
        mind->Send(u8"You want to delete which player?\n");
    } else {
      Player* pl = get_player(std::u8string(args));
      if (!pl) {
        if (mind)
          mind->Send(u8"That player doesn't seem to exist.\n");
      } else {
        if (mind)
          mind->Send(u8"You delete the player '{}'.\n", pl->Name());
        delete pl;
      }
    }
    return 0;
  }

  if (cnum == COM_PLAYERS) {
    if (!mind)
      return 0;
    std::u8string users = u8"Current accounts on this MUD:\n";
    std::vector<Player*> pls = get_all_players();

    for (auto pl : pls) {
      users += pl->Name();
      users += u8"\n";
    }
    mind->Send(users);

    return 0;
  }

  if (cnum == COM_CHARACTERS) {
    if (!mind)
      return 0;
    std::u8string chars = u8"Current characters on this MUD:\n";
    std::vector<Player*> pls = get_all_players();

    for (auto pl : pls) {
      auto chs = pl->Room()->Contents();
      for (auto ch : chs) {
        chars += pl->Name();
        chars += u8": ";
        chars += ch->ShortDesc();
        chars += u8" (";
        Object* top = ch;
        while (top->Parent() && top->Parent()->Parent())
          top = top->Parent();
        chars += top->ShortDesc();
        chars += u8")\n";
      }
    }
    mind->Send(chars);

    return 0;
  }

  if (cnum == COM_RESET) {
    if (!mind) {
      return 0;
    }
    if (args.empty()) {
      mind->Send(u8"You want to reset what?\n");
      return 0;
    }
    Object* targ =
        body->PickObject(std::u8string(args), vmode | LOC_NEARBY | LOC_INTERNAL | LOC_SELF);
    if (!targ) {
      mind->Send(u8"You want to reset what?\n");
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
      targ->Travel(dest);

      body->Parent()->SendOut(
          stealth_t,
          stealth_s,
          u8";s resets ;s with Ninja Powers[TM].\n",
          u8"You reset ;s.\n",
          body,
          targ);
    }
    return 0;
  }

  if (cnum == COM_MIRROR) {
    if (!mind) {
      return 0;
    }
    if (args.empty()) {
      mind->Send(u8"You want to mirror what?\n");
      return 0;
    }
    Object* targ = body->PickObject(std::u8string(args), vmode | LOC_NEARBY | LOC_INTERNAL);
    if (!targ) {
      mind->Send(u8"You want to mirror what?\n");
    } else {
      Object* nobj = new Object(*targ);
      nobj->SetParent(targ->Parent());

      nobj->SetSkill(
          prhash(u8"Wearable on Left Arm"), targ->Skill(prhash(u8"Wearable on Right Arm")));
      nobj->SetSkill(
          prhash(u8"Wearable on Left Finger"), targ->Skill(prhash(u8"Wearable on Right Finger")));
      nobj->SetSkill(
          prhash(u8"Wearable on Left Foot"), targ->Skill(prhash(u8"Wearable on Right Foot")));
      nobj->SetSkill(
          prhash(u8"Wearable on Left Hand"), targ->Skill(prhash(u8"Wearable on Right Hand")));
      nobj->SetSkill(
          prhash(u8"Wearable on Left Leg"), targ->Skill(prhash(u8"Wearable on Right Leg")));
      nobj->SetSkill(
          prhash(u8"Wearable on Left Wrist"), targ->Skill(prhash(u8"Wearable on Right Wrist")));
      nobj->SetSkill(
          prhash(u8"Wearable on Left Shoulder"),
          targ->Skill(prhash(u8"Wearable on Right Shoulder")));
      nobj->SetSkill(
          prhash(u8"Wearable on Left Hip"), targ->Skill(prhash(u8"Wearable on Right Hip")));

      nobj->SetSkill(
          prhash(u8"Wearable on Right Arm"), targ->Skill(prhash(u8"Wearable on Left Arm")));
      nobj->SetSkill(
          prhash(u8"Wearable on Right Finger"), targ->Skill(prhash(u8"Wearable on Left Finger")));
      nobj->SetSkill(
          prhash(u8"Wearable on Right Foot"), targ->Skill(prhash(u8"Wearable on Left Foot")));
      nobj->SetSkill(
          prhash(u8"Wearable on Right Hand"), targ->Skill(prhash(u8"Wearable on Left Hand")));
      nobj->SetSkill(
          prhash(u8"Wearable on Right Leg"), targ->Skill(prhash(u8"Wearable on Left Leg")));
      nobj->SetSkill(
          prhash(u8"Wearable on Right Wrist"), targ->Skill(prhash(u8"Wearable on Left Wrist")));
      nobj->SetSkill(
          prhash(u8"Wearable on Right Shoulder"),
          targ->Skill(prhash(u8"Wearable on Left Shoulder")));
      nobj->SetSkill(
          prhash(u8"Wearable on Right Hip"), targ->Skill(prhash(u8"Wearable on Left Hip")));

      int start;
      std::u8string name(nobj->ShortDesc());
      start = name.find(std::u8string(u8"(left)"));
      if (start < int(name.length()) && start >= 0) {
        name = name.substr(0, start) + u8"(right)" + name.substr(start + 6);
      } else {
        start = name.find(std::u8string(u8"(right)"));
        if (start < int(name.length()) && start >= 0) {
          name = name.substr(0, start) + u8"(left)" + name.substr(start + 7);
        }
      }
      nobj->SetShortDesc(name);

      body->Parent()->SendOut(
          stealth_t,
          stealth_s,
          u8";s mirrors ;s with Ninja Powers[TM].\n",
          u8"You mirror ;s.\n",
          body,
          targ);
    }
    return 0;
  }

  if (cnum == COM_CLONE) {
    if (!mind) {
      return 0;
    }
    if (args.empty()) {
      mind->Send(u8"You want to clone what?\n");
      return 0;
    }
    Object* targ =
        body->PickObject(std::u8string(args), vmode | LOC_NEARBY | LOC_INTERNAL | LOC_SELF);
    if (!targ) {
      mind->Send(u8"You want to clone what?\n");
    } else {
      Object* nobj = new Object(*targ);
      nobj->SetParent(targ->Parent());

      body->Parent()->SendOut(
          stealth_t,
          stealth_s,
          u8";s clones ;s with Ninja Powers[TM].\n",
          u8"You clone ;s.\n",
          body,
          targ);
    }
    return 0;
  }

  if (cnum == COM_PROD) {
    if (!mind) {
      return 0;
    }
    if (args.empty()) {
      mind->Send(u8"You want to prod what?\n");
      return 0;
    }
    auto targs = body->PickObjects(std::u8string(args), vmode | LOC_NEARBY);
    if (targs.size() == 0) {
      mind->Send(u8"You want to prod what?\n");
      return 0;
    }
    for (auto targ : targs) {
      body->Parent()->SendOut(
          stealth_t,
          stealth_s,
          u8";s prods ;s with Ninja Powers[TM].\n",
          u8"You prod ;s.\n",
          body,
          targ);
      targ->Activate();
    }
    return 0;
  }

  if (cnum == COM_JUNK) {
    if (!mind) {
      return 0;
    }
    if (args.empty()) {
      mind->Send(u8"You want to destroy what?\n");
      return 0;
    }
    auto targs = body->PickObjects(std::u8string(args), vmode | LOC_NEARBY);
    if (targs.size() == 0) {
      mind->Send(u8"You want to destroy what?\n");
      return 0;
    }
    for (auto targ : targs) {
      body->Parent()->SendOut(
          stealth_t,
          stealth_s,
          u8";s destroys ;s with Ninja Powers[TM].\n",
          u8"You destroy ;s.\n",
          body,
          targ);
      targ->Recycle();
    }
    return 0;
  }

  if (cnum == COM_HEAL) {
    if (!mind) {
      return 0;
    }
    if (args.empty()) {
      mind->Send(u8"You want to heal what?\n");
      return 0;
    }
    Object* targ =
        body->PickObject(std::u8string(args), vmode | LOC_NEARBY | LOC_INTERNAL | LOC_SELF);

    int finished = 0;
    if (body->IsAct(act_t::HEAL) // Finish Previous Healing
        || body->IsUsing(prhash(u8"Healing")) || body->IsUsing(prhash(u8"First Aid")) ||
        body->IsUsing(prhash(u8"Treatment"))) {
      finished = 1;
      if (body->IsAct(act_t::HEAL)) {
        if (body->IsUsing(prhash(u8"Healing"))) {
          mind->Send(u8"You complete your healing efforts.\n");
          int phys = body->ActTarg(act_t::HEAL)->Phys();
          phys -= body->Roll(prhash(u8"Healing"), phys + 2);
          if (phys < 0)
            phys = 0;
          body->ActTarg(act_t::HEAL)->SetPhys(phys);
          int pois = body->ActTarg(act_t::HEAL)->Skill(prhash(u8"Poisoned"));
          pois -= body->Roll(prhash(u8"Healing"), pois + 2);
          if (pois < 0)
            pois = 0;
          body->ActTarg(act_t::HEAL)->SetSkill(prhash(u8"Poisoned"), pois);
        } else if (body->IsUsing(prhash(u8"First Aid"))) {
          mind->Send(u8"You complete your first-aid efforts.\n");
          int phys = body->ActTarg(act_t::HEAL)->Phys();
          phys -= body->Roll(prhash(u8"First Aid"), phys);
          if (phys < 0)
            phys = 0;
          body->ActTarg(act_t::HEAL)->SetPhys(phys);
        } else if (body->IsUsing(prhash(u8"Treatment"))) {
          mind->Send(u8"You complete your treatment efforts.\n");
          int pois = body->ActTarg(act_t::HEAL)->Skill(prhash(u8"Poisoned"));
          pois -= body->Roll(prhash(u8"Treatment"), pois);
          if (pois < 0)
            pois = 0;
          body->ActTarg(act_t::HEAL)->SetSkill(prhash(u8"Poisoned"), pois);
        }
        body->StopUsing();
        body->StopAct(act_t::HEAL);
      }
    }
    if (!targ) {
      if (!finished)
        mind->Send(u8"You want to heal what?\n");
    } else if (targ->NormAttribute(2) < 1) {
      mind->Send(u8"You can't heal {}, it is not alive.\n", targ->Noun(0, 0, body));
    } else if (nmode) {
      // This is ninja-healing and bypasses all healing mechanisms.
      targ->ClearSkill(prhash(u8"Poisoned"));
      targ->ClearSkill(prhash(u8"Thirsty"));
      targ->ClearSkill(prhash(u8"Hungry"));
      targ->SetPhys(0);
      targ->SetStun(0);
      targ->SetStru(0);
      targ->UpdateDamage();

      body->Parent()->SendOut(
          stealth_t,
          stealth_s,
          u8";s heals and repairs ;s with Ninja Powers[TM].\n",
          u8"You heal ;s.\n",
          body,
          targ);
    } else if (
        (!body->HasSkill(prhash(u8"Healing"))) && (!body->HasSkill(prhash(u8"First Aid"))) &&
        (!body->HasSkill(prhash(u8"Treatment")))) {
      if (mind) {
        mind->Send(u8"You don't know how to help {}.\n", targ->Noun(0, 0, body));
      }
    } else {
      int duration = 0;
      auto skill = prhash(u8"None");
      body->Parent()->SendOut(
          stealth_t, stealth_s, u8";s tries to heal ;s.\n", u8"You try to heal ;s.\n", body, targ);
      if (body->HasSkill(prhash(u8"First Aid"))) {
        if (targ->Phys() < 1) {
          mind->Send(u8"{} is not injured.\n", targ->Noun());
        } else {
          mind->Send(u8"{} is injured.\n", targ->Noun());
          skill = prhash(u8"First Aid");
          duration = 3000;
        }
      } else if (body->HasSkill(prhash(u8"Healing"))) {
        if (targ->Phys() < 1) {
          mind->Send(u8"{} is not injured.\n", targ->Noun());
        } else {
          mind->Send(u8"{} is injured.\n", targ->Noun());
          skill = prhash(u8"Healing");
          duration = 3000;
        }
      }
      if (body->HasSkill(prhash(u8"Treatment"))) {
        if (targ->Skill(prhash(u8"Poisoned")) < 1) {
          mind->Send(u8"{} does not need other help.\n", targ->Noun());
        } else {
          mind->Send(u8"{} is poisoned.\n", targ->Noun());
          skill = prhash(u8"Treatment");
          duration = 3000;
        }
      } else if (body->HasSkill(prhash(u8"Healing"))) {
        if (targ->Skill(prhash(u8"Poisoned")) < 1) {
          mind->Send(u8"{} does not need other help.\n", targ->Noun());
        } else {
          mind->Send(u8"{} is poisoned.\n", targ->Noun());
          skill = prhash(u8"Healing");
          duration = 3000;
        }
      }
      if (skill != prhash(u8"None")) {
        body->AddAct(act_t::HEAL, targ);
        body->StartUsing(skill);
      }
      if (duration > 0) {
        body->BusyFor(duration, u8"heal");
      }
    }
    return 0;
  }

  if (cnum == COM_JACK) {
    if (!mind)
      return 0;
    Object* targ = body->ActTarg(act_t::POINT);
    if (!targ) {
      mind->Send(u8"You need to be pointing at your target.\n");
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
      mind->Send(u8"This object doesn't have that stat.\n");
      return 0;
    }

    targ->SetAttribute(stat, targ->NormAttribute(stat) + 1);

    body->Parent()->SendOut(
        stealth_t,
        stealth_s,
        u8";s jacks the {} of ;s with Ninja Powers[TM].\n",
        u8"You jack the {} of ;s.\n",
        body,
        targ,
        statnames[stat]);

    return 0;
  }

  if (cnum == COM_CHUMP) {
    if (!mind)
      return 0;
    Object* targ = body->ActTarg(act_t::POINT);
    if (!targ) {
      mind->Send(u8"You need to be pointing at your target.\n");
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
      mind->Send(u8"This object doesn't have that stat.\n");
      return 0;
    }
    if (targ->NormAttribute(stat) == 1) {
      mind->Send(u8"It is already a 1 (the minimum!).\n");
      return 0;
    }

    targ->SetAttribute(stat, targ->NormAttribute(stat) - 1);

    body->Parent()->SendOut(
        stealth_t,
        stealth_s,
        u8";s chumps the {} of ;s with Ninja Powers[TM].\n",
        u8"You chump the {} of ;s.\n",
        body,
        targ,
        statnames[stat]);

    return 0;
  }

  if (cnum == COM_INCREMENT) {
    if (!mind)
      return 0;
    Object* targ = body->ActTarg(act_t::POINT);
    if (!targ) {
      mind->Send(u8"You need to be pointing at your target.\n");
      return 0;
    }

    int amt = 1;
    if (isdigit(args[0])) {
      amt = nextnum(args);
      trim_string(args);
    }

    if (!is_skill(crc32c(args))) {
      mind->Send(u8"Warning, '{}' is not a real skill name!\n", std::u8string(args));
    }

    targ->SetSkill(args, targ->Skill(crc32c(args)) + amt);

    body->Parent()->SendOut(
        stealth_t,
        stealth_s,
        u8";s increments the {} of ;s with Ninja Powers[TM].\n",
        u8"You increment the {} of ;s.\n",
        body,
        targ,
        args);

    return 0;
  }

  if (cnum == COM_DECREMENT) {
    if (!mind)
      return 0;
    Object* targ = body->ActTarg(act_t::POINT);
    if (!targ) {
      mind->Send(u8"You need to be pointing at your target.\n");
      return 0;
    }

    int amt = 1;
    if (isdigit(args[0])) {
      amt = nextnum(args);
      trim_string(args);
    }

    targ->SetSkill(args, targ->Skill(crc32c(args)) - amt);

    body->Parent()->SendOut(
        stealth_t,
        stealth_s,
        u8";s decrements the {} of ;s with Ninja Powers[TM].\n",
        u8"You decrement the {} of ;s.\n",
        body,
        targ,
        args);

    return 0;
  }

  if (cnum == COM_DOUBLE) {
    if (!mind)
      return 0;
    auto targs = body->PickObjects(std::u8string(args), vmode | LOC_NEARBY | LOC_INTERNAL);
    if (targs.size() == 0) {
      mind->Send(u8"You want to double what?\n");
      return 0;
    }
    for (auto targ : targs) {
      body->Parent()->SendOut(
          stealth_t,
          stealth_s,
          u8";s doubles ;s with Ninja Powers[TM].\n",
          u8"You double ;s.\n",
          body,
          targ);
      if (targ->Skill(prhash(u8"Quantity")) > 1) {
        targ->SetSkill(prhash(u8"Quantity"), targ->Skill(prhash(u8"Quantity")) * 2);
      } else {
        targ->SetSkill(prhash(u8"Quantity"), 2);
      }
    }
    return 0;
  }

  if (cnum == COM_CCREATE) {
    if (!mind)
      return 0;
    return handle_command_ccreate(body, mind, args, stealth_t, stealth_s);
  }

  if (cnum == COM_WLOAD) {
    if (!mind)
      return 0;
    return handle_command_wload(body, mind, args, stealth_t, stealth_s);
  }

  if (cnum == COM_TLOAD) {
    if (!mind)
      return 0;
    if (args.empty()) {
      Object* world = new Object(body->Parent());
      world->SetShortDesc(u8"The tbaMUD World");
      world->SetSkill(prhash(u8"Light Source"), 1000);
      world->SetSkill(prhash(u8"Day Length"), 240);
      world->SetSkill(prhash(u8"Day Time"), 120);
      world->TBALoadAll();
      body->Parent()->SendOut(
          stealth_t,
          stealth_s,
          u8";s loads the entire TBA world with Ninja Powers[TM].\n",
          u8"You load the entire TBA world.\n",
          body,
          nullptr);
      world->Activate();
    } else {
      body->Parent()->TBALoadWLD(fmt::format(u8"tba/wld/{}.wld", args));
      body->Parent()->TBALoadOBJ(fmt::format(u8"tba/obj/{}.obj", args));
      body->Parent()->SendOut(
          stealth_t,
          stealth_s,
          u8";s loads a TBA world with Ninja Powers[TM].\n",
          u8"You load a TBA world.\n",
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
        u8";s cleans up after loading TBA worlds.\n",
        u8"You clean up after loading TBA worlds.\n",
        body,
        nullptr);
    return 0;
  }

  if (mind)
    mind->Send(u8"Sorry, that command's not yet implemented.\n");
  return 0;
}

int handle_command(Object* body, const std::u8string_view& cl, std::shared_ptr<Mind> mind) {
  int ret = 0;

  if (mind && !mind->SpecialPrompt().empty()) {
    std::u8string cmd = fmt::format(u8"{} {}", mind->SpecialPrompt(), cl);
    ret = handle_single_command(body, cmd, mind);
    return ret;
  }

  auto start = cl.find_first_not_of(u8"; \t\r\n");
  while (start != std::u8string::npos) {
    auto end = cl.find_first_of(u8";\r\n", start + 1);

    std::u8string_view single(cl);
    if (end != std::u8string::npos) {
      single = single.substr(start, end - start);
    } else {
      single = single.substr(start);
    }

    int stat = handle_single_command(body, std::u8string(single), mind);
    if (stat < 0)
      return stat;
    else if (ret == 0)
      ret = stat;
    if (end != std::u8string::npos) {
      start = cl.find_first_not_of(u8"; \t\r\n", end + 1);
    } else {
      start = std::u8string::npos;
    }
  }
  return ret;
}
