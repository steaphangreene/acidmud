#include <string>

#include "mind.hpp"
#include "object.hpp"

typedef enum {
  COM_NONE = 0,
  COM_HELP,
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
  COM_HOLD,
  COM_OFFER,

  COM_DRAG,
  COM_PUT,
  COM_DROP,
  COM_STASH,
  COM_WIELD,
  COM_UNWIELD,
  COM_LIGHT,
  COM_WEAR,
  COM_REMOVE,
  COM_LABEL,
  COM_UNLABEL,
  COM_HEAL,

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
  COM_CAST,
  COM_PRAY,

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
  COM_FLEE,

  COM_LIST,
  COM_BUY,
  COM_VALUE,
  COM_SELL,

  COM_TOGGLE,

  COM_WHO,
  COM_OOC,
  COM_NEWBIE,

  COM_NEWCHARACTER,
  COM_RAISE,
  COM_LOWER,
  COM_RESETCHARACTER,
  COM_RANDOMIZE,
  COM_ARCHETYPE,

  COM_SCORE,
  COM_TIME,
  COM_WORLD,
  COM_VERSION,

  COM_SKILLLIST,

  COM_RECALL,
  COM_TELEPORT,
  COM_RESURRECT,

  COM_NINJAMODE,
  COM_MAKENINJA,
  COM_MAKESUPERNINJA,

  COM_CREATE,
  COM_DCREATE,
  COM_CCREATE,
  COM_ANCHOR,
  COM_LINK,
  COM_CONNECT,
  COM_COMMAND,
  COM_CONTROL,
  COM_CLONE,
  COM_MIRROR,
  COM_JUNK,
  COM_PROD,

  COM_RESET,
  COM_PLAYERS,
  COM_DELPLAYER,
  COM_CHARACTERS,

  COM_JACK,
  COM_CHUMP,
  COM_INCREMENT,
  COM_DECREMENT,
  COM_DOUBLE,
  COM_SETSTATS,
  COM_NAME,
  COM_UNDESCRIBE,
  COM_DESCRIBE,
  COM_UNDEFINE,
  COM_DEFINE,

  COM_STATS,
  COM_SHUTDOWN,
  COM_RESTART,
  COM_SAVEALL,

  COM_MAKESTART,

  COM_TLOAD,
  COM_TCLEAN,

  COM_MAX,
  COM_SOCIAL = COM_MAX, // Many commands, which all have no real effect.
} com_t;

int handle_command(Object*, const std::string&, Mind* mind = nullptr);
com_t identify_command(const std::string& line, bool corporeal);
