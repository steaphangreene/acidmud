#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>
#include <vector>

#include "color.hpp"
#include "object.hpp"

static std::map<std::string, int> defaults;
static std::map<int, std::string> weaponskills;
static std::map<std::string, int> weapontypes;
static std::map<std::string, std::vector<std::string>> skcat;

static int last_wtype = 0;
static void add_wts(const std::string& sk) {
  if (defaults.count(sk) == 0) {
    fprintf(
        stderr,
        "Warning: Tried to link weapon type %d to '%s' which isn't a skill.\n",
        last_wtype + 1,
        sk.c_str());
    return;
  }
  ++last_wtype;
  weaponskills[last_wtype] = sk;
  weapontypes[sk] = last_wtype;
}

static int defaults_init = 0;
static void init_defaults() {
  if (defaults_init)
    return;
  defaults_init = 1;

  // Skill Definition: Acrobatics
  defaults["Acrobatics"] = 1;
  skcat["Quickness-Based Skills"].push_back("Acrobatics");
  skcat["Athletic Skills"].push_back("Acrobatics");
  skcat["Medieval Skills"].push_back("Acrobatics");
  skcat["Cyberpunk Skills"].push_back("Acrobatics");
  skcat["Modern Skills"].push_back("Acrobatics");
  skcat["Shadowrun Skills"].push_back("Acrobatics");
  skcat["Skills"].push_back("Acrobatics");

  // Skill Definition: Acting
  defaults["Acting"] = 3;
  skcat["Charisma-Based Skills"].push_back("Acting");
  skcat["Social Skills"].push_back("Acting");
  skcat["Medieval Skills"].push_back("Acting");
  skcat["Cyberpunk Skills"].push_back("Acting");
  skcat["Modern Skills"].push_back("Acting");
  skcat["Shadowrun Skills"].push_back("Acting");
  skcat["Skills"].push_back("Acting");

  // Skill Definition: Activate Juju
  defaults["Activate Juju"] = 7;
  skcat["Skill-Based Skills"].push_back("Activate Juju");
  skcat["Shamanistic Skills"].push_back("Activate Juju");
  skcat["Medieval Skills"].push_back("Activate Juju");
  skcat["Skills"].push_back("Activate Juju");

  // Skill Definition: Air Pistols
  defaults["Air Pistols"] = 1;
  skcat["Quickness-Based Skills"].push_back("Air Pistols");
  skcat["Pistol Skills"].push_back("Air Pistols");
  skcat["Cyberpunk Skills"].push_back("Air Pistols");
  skcat["Modern Skills"].push_back("Air Pistols");
  skcat["Shadowrun Skills"].push_back("Air Pistols");
  skcat["Skills"].push_back("Air Pistols");
  add_wts("Air Pistols");

  // Skill Definition: Air Rifles
  defaults["Air Rifles"] = 1;
  skcat["Quickness-Based Skills"].push_back("Air Rifles");
  skcat["Rifle Skills"].push_back("Air Rifles");
  skcat["Cyberpunk Skills"].push_back("Air Rifles");
  skcat["Modern Skills"].push_back("Air Rifles");
  skcat["Shadowrun Skills"].push_back("Air Rifles");
  skcat["Skills"].push_back("Air Rifles");
  add_wts("Air Rifles");

  // Skill Definition: Air Weaving
  defaults["Air Weaving"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Air Weaving");
  skcat["Magical Skills"].push_back("Air Weaving");
  skcat["Medieval Skills"].push_back("Air Weaving");
  skcat["FP Skills"].push_back("Air Weaving");
  skcat["Skills"].push_back("Air Weaving");

  // Skill Definition: Alchemy
  defaults["Alchemy"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Alchemy");
  skcat["Magical Skills"].push_back("Alchemy");
  skcat["Medieval Skills"].push_back("Alchemy");
  skcat["Shadowrun Skills"].push_back("Alchemy");
  skcat["Skills"].push_back("Alchemy");

  // Skill Definition: Amphibious Combat
  defaults["Amphibious Combat"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Amphibious Combat");
  skcat["Combat Environment Skills"].push_back("Amphibious Combat");
  skcat["Medieval Skills"].push_back("Amphibious Combat");
  skcat["Cyberpunk Skills"].push_back("Amphibious Combat");
  skcat["Modern Skills"].push_back("Amphibious Combat");
  skcat["Shadowrun Skills"].push_back("Amphibious Combat");
  skcat["Skills"].push_back("Amphibious Combat");

  // Skill Definition: Archery
  defaults["Archery"] = 1;
  skcat["Quickness-Based Skills"].push_back("Archery");
  skcat["Ranged-Combat Skills"].push_back("Archery");
  skcat["Medieval Skills"].push_back("Archery");
  skcat["Cyberpunk Skills"].push_back("Archery");
  skcat["Modern Skills"].push_back("Archery");
  skcat["Shadowrun Skills"].push_back("Archery");
  skcat["Skills"].push_back("Archery");
  add_wts("Archery");

  // Skill Definition: Arctic Waraxe Martial Art
  defaults["Arctic Waraxe Martial Art"] = 2;
  skcat["Strength-Based Skills"].push_back("Arctic Waraxe Martial Art");
  skcat["Lost Martial Arts"].push_back("Arctic Waraxe Martial Art");
  skcat["Medieval Skills"].push_back("Arctic Waraxe Martial Art");
  skcat["Skills"].push_back("Arctic Waraxe Martial Art");

  // Skill Definition: Armoring
  defaults["Armoring"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Armoring");
  skcat["Creation Skills"].push_back("Armoring");
  skcat["Medieval Skills"].push_back("Armoring");
  skcat["Skills"].push_back("Armoring");

  // Skill Definition: Art
  defaults["Art"] = 3;
  skcat["Charisma-Based Skills"].push_back("Art");
  skcat["Social Skills"].push_back("Art");
  skcat["Medieval Skills"].push_back("Art");
  skcat["Cyberpunk Skills"].push_back("Art");
  skcat["Modern Skills"].push_back("Art");
  skcat["Shadowrun Skills"].push_back("Art");
  skcat["Skills"].push_back("Art");

  // Skill Definition: Artificing
  defaults["Artificing"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Artificing");
  skcat["Magical Skills"].push_back("Artificing");
  skcat["Medieval Skills"].push_back("Artificing");
  skcat["Skills"].push_back("Artificing");

  // Skill Definition: Artificing, Star
  defaults["Artificing, Star"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Artificing, Star");
  skcat["Star Magical Skills"].push_back("Artificing, Star");
  skcat["Medieval Skills"].push_back("Artificing, Star");
  skcat["Skills"].push_back("Artificing, Star");

  // Skill Definition: Assault Cannons
  defaults["Assault Cannons"] = 2;
  skcat["Strength-Based Skills"].push_back("Assault Cannons");
  skcat["Heavy Firearm Skills"].push_back("Assault Cannons");
  skcat["Cyberpunk Skills"].push_back("Assault Cannons");
  skcat["Modern Skills"].push_back("Assault Cannons");
  skcat["Shadowrun Skills"].push_back("Assault Cannons");
  skcat["Skills"].push_back("Assault Cannons");

  // Skill Definition: Assault Rifles
  defaults["Assault Rifles"] = 1;
  skcat["Quickness-Based Skills"].push_back("Assault Rifles");
  skcat["Rifle Skills"].push_back("Assault Rifles");
  skcat["Cyberpunk Skills"].push_back("Assault Rifles");
  skcat["Modern Skills"].push_back("Assault Rifles");
  skcat["Shadowrun Skills"].push_back("Assault Rifles");
  skcat["Skills"].push_back("Assault Rifles");
  add_wts("Assault Rifles");

  // Skill Definition: Ballista
  defaults["Ballista"] = 6;
  skcat["Reaction-Based Skills"].push_back("Ballista");
  skcat["Medieval Skills"].push_back("Ballista");
  skcat["Skills"].push_back("Ballista");

  // Skill Definition: Bike
  defaults["Bike"] = 6;
  skcat["Reaction-Based Skills"].push_back("Bike");
  skcat["Piloting Skills"].push_back("Bike");
  skcat["Cyberpunk Skills"].push_back("Bike");
  skcat["Modern Skills"].push_back("Bike");
  skcat["Shadowrun Skills"].push_back("Bike");
  skcat["Skills"].push_back("Bike");

  // Skill Definition: Blacksmithing
  defaults["Blacksmithing"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Blacksmithing");
  skcat["Creation Skills"].push_back("Blacksmithing");
  skcat["Medieval Skills"].push_back("Blacksmithing");
  skcat["Skills"].push_back("Blacksmithing");

  // Skill Definition: Blowgun
  defaults["Blowgun"] = 1;
  skcat["Quickness-Based Skills"].push_back("Blowgun");
  skcat["Ranged-Combat Skills"].push_back("Blowgun");
  skcat["Medieval Skills"].push_back("Blowgun");
  skcat["Cyberpunk Skills"].push_back("Blowgun");
  skcat["Modern Skills"].push_back("Blowgun");
  skcat["Shadowrun Skills"].push_back("Blowgun");
  skcat["Skills"].push_back("Blowgun");
  add_wts("Blowgun");

  // Skill Definition: Boat, Powered
  defaults["Boat, Powered"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Boat, Powered");
  skcat["Piloting Skills"].push_back("Boat, Powered");
  skcat["Cyberpunk Skills"].push_back("Boat, Powered");
  skcat["Modern Skills"].push_back("Boat, Powered");
  skcat["Shadowrun Skills"].push_back("Boat, Powered");
  skcat["Skills"].push_back("Boat, Powered");

  // Skill Definition: Boat, Row
  defaults["Boat, Row"] = 2;
  skcat["Strength-Based Skills"].push_back("Boat, Row");
  skcat["Piloting Skills"].push_back("Boat, Row");
  skcat["Medieval Skills"].push_back("Boat, Row");
  skcat["Cyberpunk Skills"].push_back("Boat, Row");
  skcat["Modern Skills"].push_back("Boat, Row");
  skcat["Shadowrun Skills"].push_back("Boat, Row");
  skcat["Skills"].push_back("Boat, Row");

  // Skill Definition: Boat, Sail
  defaults["Boat, Sail"] = 6;
  skcat["Reaction-Based Skills"].push_back("Boat, Sail");
  skcat["Piloting Skills"].push_back("Boat, Sail");
  skcat["Medieval Skills"].push_back("Boat, Sail");
  skcat["Cyberpunk Skills"].push_back("Boat, Sail");
  skcat["Modern Skills"].push_back("Boat, Sail");
  skcat["Shadowrun Skills"].push_back("Boat, Sail");
  skcat["Skills"].push_back("Boat, Sail");

  // Skill Definition: Body Checking
  defaults["Body Checking"] = 0;
  skcat["Body-Based Skills"].push_back("Body Checking");
  skcat["Hand-to-Hand Combat Skills"].push_back("Body Checking");
  skcat["Cyberpunk Skills"].push_back("Body Checking");
  skcat["Modern Skills"].push_back("Body Checking");
  skcat["Shadowrun Skills"].push_back("Body Checking");
  skcat["Medieval Skills"].push_back("Body Checking");
  skcat["Skills"].push_back("Body Checking");
  add_wts("Body Checking");

  // Skill Definition: Bowyer
  defaults["Bowyer"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Bowyer");
  skcat["Creation Skills"].push_back("Bowyer");
  skcat["Medieval Skills"].push_back("Bowyer");
  skcat["Skills"].push_back("Bowyer");

  // Skill Definition: Breath Weapon
  defaults["Breath Weapon"] = 1;
  skcat["Quickness-Based Skills"].push_back("Breath Weapon");
  skcat["Medieval Skills"].push_back("Breath Weapon");
  skcat["Shadowrun Skills"].push_back("Breath Weapon");
  skcat["Skills"].push_back("Breath Weapon");

  // Skill Definition: Buggy
  defaults["Buggy"] = 6;
  skcat["Reaction-Based Skills"].push_back("Buggy");
  skcat["Piloting Skills"].push_back("Buggy");
  skcat["Cyberpunk Skills"].push_back("Buggy");
  skcat["Modern Skills"].push_back("Buggy");
  skcat["Shadowrun Skills"].push_back("Buggy");
  skcat["Skills"].push_back("Buggy");

  // Skill Definition: Cannon
  defaults["Cannon"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Cannon");
  skcat["Heavy Firearm Skills"].push_back("Cannon");
  skcat["Medieval Skills"].push_back("Cannon");
  skcat["Cyberpunk Skills"].push_back("Cannon");
  skcat["Modern Skills"].push_back("Cannon");
  skcat["Shadowrun Skills"].push_back("Cannon");
  skcat["Skills"].push_back("Cannon");

  // Skill Definition: Car
  defaults["Car"] = 6;
  skcat["Reaction-Based Skills"].push_back("Car");
  skcat["Piloting Skills"].push_back("Car");
  skcat["Cyberpunk Skills"].push_back("Car");
  skcat["Modern Skills"].push_back("Car");
  skcat["Shadowrun Skills"].push_back("Car");
  skcat["Skills"].push_back("Car");

  // Skill Definition: Carpentry
  defaults["Carpentry"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Carpentry");
  skcat["Creation Skills"].push_back("Carpentry");
  skcat["Medieval Skills"].push_back("Carpentry");
  skcat["Modern Skills"].push_back("Carpentry");
  skcat["Skills"].push_back("Carpentry");

  // Skill Definition: Carromeleg - Tier I
  defaults["Carromeleg - Tier I"] = 0;
  skcat["Body-Based Skills"].push_back("Carromeleg - Tier I");
  skcat["Elven Martial Arts Skills"].push_back("Carromeleg - Tier I");
  skcat["Medieval Skills"].push_back("Carromeleg - Tier I");
  skcat["Shadowrun Skills"].push_back("Carromeleg - Tier I");
  skcat["Skills"].push_back("Carromeleg - Tier I");

  // Skill Definition: Carromeleg - Tier II
  defaults["Carromeleg - Tier II"] = 1;
  skcat["Quickness-Based Skills"].push_back("Carromeleg - Tier II");
  skcat["Elven Martial Arts Skills"].push_back("Carromeleg - Tier II");
  skcat["Medieval Skills"].push_back("Carromeleg - Tier II");
  skcat["Shadowrun Skills"].push_back("Carromeleg - Tier II");
  skcat["Skills"].push_back("Carromeleg - Tier II");

  // Skill Definition: Carromeleg - Tier III
  defaults["Carromeleg - Tier III"] = 2;
  skcat["Strength-Based Skills"].push_back("Carromeleg - Tier III");
  skcat["Elven Martial Arts Skills"].push_back("Carromeleg - Tier III");
  skcat["Medieval Skills"].push_back("Carromeleg - Tier III");
  skcat["Shadowrun Skills"].push_back("Carromeleg - Tier III");
  skcat["Skills"].push_back("Carromeleg - Tier III");

  // Skill Definition: Carromeleg - Tier IV
  defaults["Carromeleg - Tier IV"] = 3;
  skcat["Charisma-Based Skills"].push_back("Carromeleg - Tier IV");
  skcat["Elven Martial Arts Skills"].push_back("Carromeleg - Tier IV");
  skcat["Medieval Skills"].push_back("Carromeleg - Tier IV");
  skcat["Shadowrun Skills"].push_back("Carromeleg - Tier IV");
  skcat["Skills"].push_back("Carromeleg - Tier IV");

  // Skill Definition: Carromeleg - Tier V
  defaults["Carromeleg - Tier V"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Carromeleg - Tier V");
  skcat["Elven Martial Arts Skills"].push_back("Carromeleg - Tier V");
  skcat["Medieval Skills"].push_back("Carromeleg - Tier V");
  skcat["Shadowrun Skills"].push_back("Carromeleg - Tier V");
  skcat["Skills"].push_back("Carromeleg - Tier V");

  // Skill Definition: Carromeleg - Tier VI
  defaults["Carromeleg - Tier VI"] = 5;
  skcat["Willpower-Based Skills"].push_back("Carromeleg - Tier VI");
  skcat["Elven Martial Arts Skills"].push_back("Carromeleg - Tier VI");
  skcat["Medieval Skills"].push_back("Carromeleg - Tier VI");
  skcat["Shadowrun Skills"].push_back("Carromeleg - Tier VI");
  skcat["Skills"].push_back("Carromeleg - Tier VI");

  // Skill Definition: Cart
  defaults["Cart"] = 6;
  skcat["Reaction-Based Skills"].push_back("Cart");
  skcat["Piloting Skills"].push_back("Cart");
  skcat["Cyberpunk Skills"].push_back("Cart");
  skcat["Modern Skills"].push_back("Cart");
  skcat["Shadowrun Skills"].push_back("Cart");
  skcat["Skills"].push_back("Cart");

  // Skill Definition: Catching
  defaults["Catching"] = 1;
  skcat["Quickness-Based Skills"].push_back("Catching");
  skcat["Athletic Skills"].push_back("Catching");
  skcat["Medieval Skills"].push_back("Catching");
  skcat["Cyberpunk Skills"].push_back("Catching");
  skcat["Modern Skills"].push_back("Catching");
  skcat["Shadowrun Skills"].push_back("Catching");
  skcat["Skills"].push_back("Catching");

  // Skill Definition: Centering
  defaults["Centering"] = 5;
  skcat["Willpower-Based Skills"].push_back("Centering");
  skcat["Star Magical Skills"].push_back("Centering");
  skcat["Medieval Skills"].push_back("Centering");
  skcat["Skills"].push_back("Centering");

  // Skill Definition: Climbing
  defaults["Climbing"] = 2;
  skcat["Strength-Based Skills"].push_back("Climbing");
  skcat["Athletic Skills"].push_back("Climbing");
  skcat["Medieval Skills"].push_back("Climbing");
  skcat["Cyberpunk Skills"].push_back("Climbing");
  skcat["Modern Skills"].push_back("Climbing");
  skcat["Shadowrun Skills"].push_back("Climbing");
  skcat["Skills"].push_back("Climbing");

  // Skill Definition: Clothier
  defaults["Clothier"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Clothier");
  skcat["Creation Skills"].push_back("Clothier");
  skcat["Medieval Skills"].push_back("Clothier");
  skcat["Skills"].push_back("Clothier");

  // Skill Definition: Communications
  defaults["Communications"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Communications");
  skcat["Technical Skills"].push_back("Communications");
  skcat["Cyberpunk Skills"].push_back("Communications");
  skcat["Modern Skills"].push_back("Communications");
  skcat["Shadowrun Skills"].push_back("Communications");
  skcat["Skills"].push_back("Communications");

  // Skill Definition: Computer
  defaults["Computer"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Computer");
  skcat["Technical Skills"].push_back("Computer");
  skcat["Cyberpunk Skills"].push_back("Computer");
  skcat["Modern Skills"].push_back("Computer");
  skcat["Shadowrun Skills"].push_back("Computer");
  skcat["Skills"].push_back("Computer");

  // Skill Definition: Computer Build/Repair
  defaults["Computer Build/Repair"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Computer Build/Repair");
  skcat["Build/Repair Skills"].push_back("Computer Build/Repair");
  skcat["Cyberpunk Skills"].push_back("Computer Build/Repair");
  skcat["Modern Skills"].push_back("Computer Build/Repair");
  skcat["Shadowrun Skills"].push_back("Computer Build/Repair");
  skcat["Skills"].push_back("Computer Build/Repair");

  // Skill Definition: Computer Targeting
  defaults["Computer Targeting"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Computer Targeting");
  skcat["Weapon System Skills"].push_back("Computer Targeting");
  skcat["Cyberpunk Skills"].push_back("Computer Targeting");
  skcat["Modern Skills"].push_back("Computer Targeting");
  skcat["Shadowrun Skills"].push_back("Computer Targeting");
  skcat["Skills"].push_back("Computer Targeting");

  // Skill Definition: Conjuring
  defaults["Conjuring"] = 3;
  skcat["Charisma-Based Skills"].push_back("Conjuring");
  skcat["Magical Skills"].push_back("Conjuring");
  skcat["Medieval Skills"].push_back("Conjuring");
  skcat["Shadowrun Skills"].push_back("Conjuring");
  skcat["Skills"].push_back("Conjuring");

  // Skill Definition: Crossbow
  defaults["Crossbow"] = 1;
  skcat["Quickness-Based Skills"].push_back("Crossbow");
  skcat["Ranged-Combat Skills"].push_back("Crossbow");
  skcat["Medieval Skills"].push_back("Crossbow");
  skcat["Cyberpunk Skills"].push_back("Crossbow");
  skcat["Modern Skills"].push_back("Crossbow");
  skcat["Shadowrun Skills"].push_back("Crossbow");
  skcat["Skills"].push_back("Crossbow");
  add_wts("Crossbow");

  // Skill Definition: Cyber Data Manipulation
  defaults["Cyber Data Manipulation"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Cyber Data Manipulation");
  skcat["Cyber Skills"].push_back("Cyber Data Manipulation");
  skcat["Cyberpunk Skills"].push_back("Cyber Data Manipulation");
  skcat["Shadowrun Skills"].push_back("Cyber Data Manipulation");
  skcat["Specialty Skills"].push_back("Cyber Data Manipulation");
  skcat["Skills"].push_back("Cyber Data Manipulation");

  // Skill Definition: Cyber Vehicle Operation
  defaults["Cyber Vehicle Operation"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Cyber Vehicle Operation");
  skcat["Cyber Skills"].push_back("Cyber Vehicle Operation");
  skcat["Cyberpunk Skills"].push_back("Cyber Vehicle Operation");
  skcat["Shadowrun Skills"].push_back("Cyber Vehicle Operation");
  skcat["Specialty Skills"].push_back("Cyber Vehicle Operation");
  skcat["Skills"].push_back("Cyber Vehicle Operation");

  // Skill Definition: Cycling
  defaults["Cycling"] = 6;
  skcat["Reaction-Based Skills"].push_back("Cycling");
  skcat["Piloting Skills"].push_back("Cycling");
  skcat["Cyberpunk Skills"].push_back("Cycling");
  skcat["Modern Skills"].push_back("Cycling");
  skcat["Shadowrun Skills"].push_back("Cycling");
  skcat["Skills"].push_back("Cycling");

  // Skill Definition: Damage Control
  defaults["Damage Control"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Damage Control");
  skcat["Technical Skills"].push_back("Damage Control");
  skcat["Cyberpunk Skills"].push_back("Damage Control");
  skcat["Modern Skills"].push_back("Damage Control");
  skcat["Shadowrun Skills"].push_back("Damage Control");
  skcat["Skills"].push_back("Damage Control");

  // Skill Definition: Damage Resistance
  defaults["Damage Resistance"] = 0;
  skcat["Body-Based Skills"].push_back("Damage Resistance");
  skcat["Medieval Skills"].push_back("Damage Resistance");
  skcat["Cyberpunk Skills"].push_back("Damage Resistance");
  skcat["Modern Skills"].push_back("Damage Resistance");
  skcat["Shadowrun Skills"].push_back("Damage Resistance");
  skcat["Skills"].push_back("Damage Resistance");

  // Skill Definition: Demolitions
  defaults["Demolitions"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Demolitions");
  skcat["Technical Skills"].push_back("Demolitions");
  skcat["Cyberpunk Skills"].push_back("Demolitions");
  skcat["Modern Skills"].push_back("Demolitions");
  skcat["Shadowrun Skills"].push_back("Demolitions");
  skcat["Skills"].push_back("Demolitions");

  // Skill Definition: Diplomacy
  defaults["Diplomacy"] = 3;
  skcat["Charisma-Based Skills"].push_back("Diplomacy");
  skcat["Social Skills"].push_back("Diplomacy");
  skcat["Medieval Skills"].push_back("Diplomacy");
  skcat["Cyberpunk Skills"].push_back("Diplomacy");
  skcat["Modern Skills"].push_back("Diplomacy");
  skcat["Shadowrun Skills"].push_back("Diplomacy");
  skcat["Skills"].push_back("Diplomacy");

  // Skill Definition: Diving
  defaults["Diving"] = 1;
  skcat["Quickness-Based Skills"].push_back("Diving");
  skcat["Athletic Skills"].push_back("Diving");
  skcat["Medieval Skills"].push_back("Diving");
  skcat["Cyberpunk Skills"].push_back("Diving");
  skcat["Modern Skills"].push_back("Diving");
  skcat["Shadowrun Skills"].push_back("Diving");
  skcat["Skills"].push_back("Diving");

  // Skill Definition: Dodge
  defaults["Dodge"] = 1;
  skcat["Quickness-Based Skills"].push_back("Dodge");
  skcat["Athletic Skills"].push_back("Dodge");
  skcat["Medieval Skills"].push_back("Dodge");
  skcat["Cyberpunk Skills"].push_back("Dodge");
  skcat["Modern Skills"].push_back("Dodge");
  skcat["Shadowrun Skills"].push_back("Dodge");
  skcat["Skills"].push_back("Dodge");

  // Skill Definition: Earth Weaving
  defaults["Earth Weaving"] = 2;
  skcat["Strength-Based Skills"].push_back("Earth Weaving");
  skcat["Magical Skills"].push_back("Earth Weaving");
  skcat["Medieval Skills"].push_back("Earth Weaving");
  skcat["FP Skills"].push_back("Earth Weaving");
  skcat["Skills"].push_back("Earth Weaving");

  // Skill Definition: Electrical Build/Repair
  defaults["Electrical Build/Repair"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Electrical Build/Repair");
  skcat["Build/Repair Skills"].push_back("Electrical Build/Repair");
  skcat["Cyberpunk Skills"].push_back("Electrical Build/Repair");
  skcat["Modern Skills"].push_back("Electrical Build/Repair");
  skcat["Shadowrun Skills"].push_back("Electrical Build/Repair");
  skcat["Skills"].push_back("Electrical Build/Repair");

  // Skill Definition: Electronics
  defaults["Electronics"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Electronics");
  skcat["Technical Skills"].push_back("Electronics");
  skcat["Cyberpunk Skills"].push_back("Electronics");
  skcat["Modern Skills"].push_back("Electronics");
  skcat["Shadowrun Skills"].push_back("Electronics");
  skcat["Skills"].push_back("Electronics");

  // Skill Definition: Enchanting
  defaults["Enchanting"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Enchanting");
  skcat["Magical Skills"].push_back("Enchanting");
  skcat["Medieval Skills"].push_back("Enchanting");
  skcat["Shadowrun Skills"].push_back("Enchanting");
  skcat["Skills"].push_back("Enchanting");

  // Skill Definition: Endurance
  defaults["Endurance"] = 2;
  skcat["Strength-Based Skills"].push_back("Endurance");
  skcat["Medieval Skills"].push_back("Endurance");
  skcat["Cyberpunk Skills"].push_back("Endurance");
  skcat["Modern Skills"].push_back("Endurance");
  skcat["Shadowrun Skills"].push_back("Endurance");
  skcat["Skills"].push_back("Endurance");

  // Skill Definition: Engineering
  defaults["Engineering"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Engineering");
  skcat["Technical Skills"].push_back("Engineering");
  skcat["Medieval Skills"].push_back("Engineering");
  skcat["Cyberpunk Skills"].push_back("Engineering");
  skcat["Modern Skills"].push_back("Engineering");
  skcat["Shadowrun Skills"].push_back("Engineering");
  skcat["Skills"].push_back("Engineering");

  // Skill Definition: Prepared Intermediate
  skcat["Skill-Based Skills"].push_back("Prepared Intermediate");
  skcat["Star Magical Skills"].push_back("Prepared Intermediate");
  skcat["Medieval Skills"].push_back("Prepared Intermediate");
  skcat["Skills"].push_back("Prepared Intermediate");

  // Skill Definition: Prepared Primary
  skcat["Skill-Based Skills"].push_back("Prepared Primary");
  skcat["Star Magical Skills"].push_back("Prepared Primary");
  skcat["Medieval Skills"].push_back("Prepared Primary");
  skcat["Skills"].push_back("Prepared Primary");

  // Skill Definition: Prepared Basic
  skcat["Skill-Based Skills"].push_back("Prepared Basic");
  skcat["Star Magical Skills"].push_back("Prepared Basic");
  skcat["Medieval Skills"].push_back("Prepared Basic");
  skcat["Skills"].push_back("Prepared Basic");

  // Skill Definition: Etiquette
  defaults["Etiquette"] = 3;
  skcat["Charisma-Based Skills"].push_back("Etiquette");
  skcat["Social Skills"].push_back("Etiquette");
  skcat["Cyberpunk Skills"].push_back("Etiquette");
  skcat["Medieval Skills"].push_back("Etiquette");
  skcat["Modern Skills"].push_back("Etiquette");
  skcat["Shadowrun Skills"].push_back("Etiquette");
  skcat["Skills"].push_back("Etiquette");

  // Skill Definition: Evasion
  defaults["Evasion"] = 1;
  skcat["Quickness-Based Skills"].push_back("Evasion");
  skcat["Athletic Skills"].push_back("Evasion");
  skcat["Medieval Skills"].push_back("Evasion");
  skcat["Cyberpunk Skills"].push_back("Evasion");
  skcat["Modern Skills"].push_back("Evasion");
  skcat["Shadowrun Skills"].push_back("Evasion");
  skcat["Skills"].push_back("Evasion");

  // Skill Definition: Finance
  defaults["Finance"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Finance");
  skcat["Cyberpunk Skills"].push_back("Finance");
  skcat["Modern Skills"].push_back("Finance");
  skcat["Shadowrun Skills"].push_back("Finance");
  skcat["Skills"].push_back("Finance");

  // Skill Definition: Find Juju
  defaults["Find Juju"] = 5;
  skcat["Willpower-Based Skills"].push_back("Find Juju");
  skcat["Shamanistic Skills"].push_back("Find Juju");
  skcat["Medieval Skills"].push_back("Find Juju");
  skcat["Skills"].push_back("Find Juju");

  // Skill Definition: Fire Weaving
  defaults["Fire Weaving"] = 3;
  skcat["Charisma-Based Skills"].push_back("Fire Weaving");
  skcat["Magical Skills"].push_back("Fire Weaving");
  skcat["Medieval Skills"].push_back("Fire Weaving");
  skcat["FP Skills"].push_back("Fire Weaving");
  skcat["Skills"].push_back("Fire Weaving");

  // Skill Definition: First Aid
  defaults["First Aid"] = 4;
  skcat["Intelligence-Based Skills"].push_back("First Aid");
  skcat["Technical Skills"].push_back("First Aid");
  skcat["Cyberpunk Skills"].push_back("First Aid");
  skcat["Modern Skills"].push_back("First Aid");
  skcat["Shadowrun Skills"].push_back("First Aid");
  skcat["Skills"].push_back("First Aid");

  // Skill Definition: Fixed-Wing Craft
  defaults["Fixed-Wing Craft"] = 6;
  skcat["Reaction-Based Skills"].push_back("Fixed-Wing Craft");
  skcat["Piloting Skills"].push_back("Fixed-Wing Craft");
  skcat["Cyberpunk Skills"].push_back("Fixed-Wing Craft");
  skcat["Modern Skills"].push_back("Fixed-Wing Craft");
  skcat["Shadowrun Skills"].push_back("Fixed-Wing Craft");
  skcat["Skills"].push_back("Fixed-Wing Craft");

  // Skill Definition: Fixed-Wing Drone
  defaults["Fixed-Wing Drone"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Fixed-Wing Drone");
  skcat["Drone Piloting Skills"].push_back("Fixed-Wing Drone");
  skcat["Shadowrun Skills"].push_back("Fixed-Wing Drone");
  skcat["Skills"].push_back("Fixed-Wing Drone");

  // Skill Definition: Fletcher
  defaults["Fletcher"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Fletcher");
  skcat["Creation Skills"].push_back("Fletcher");
  skcat["Medieval Skills"].push_back("Fletcher");
  skcat["Skills"].push_back("Fletcher");

  // Skill Definition: Flight
  defaults["Flight"] = 1;
  skcat["Quickness-Based Skills"].push_back("Flight");
  skcat["Athletic Skills"].push_back("Flight");
  skcat["Medieval Skills"].push_back("Flight");
  skcat["Shadowrun Skills"].push_back("Flight");
  skcat["Skills"].push_back("Flight");

  // Skill Definition: Focusing
  defaults["Focusing"] = 2;
  skcat["Strength-Based Skills"].push_back("Focusing");
  skcat["Star Magical Skills"].push_back("Focusing");
  skcat["Medieval Skills"].push_back("Focusing");
  skcat["Skills"].push_back("Focusing");

  // Skill Definition: Grappling
  defaults["Grappling"] = 2;
  skcat["Strength-Based Skills"].push_back("Grappling");
  skcat["Hand-to-Hand Combat Skills"].push_back("Grappling");
  skcat["Medieval Skills"].push_back("Grappling");
  skcat["Cyberpunk Skills"].push_back("Grappling");
  skcat["Modern Skills"].push_back("Grappling");
  skcat["Shadowrun Skills"].push_back("Grappling");
  skcat["Skills"].push_back("Grappling");
  add_wts("Grappling");

  // Skill Definition: Gunnery
  defaults["Gunnery"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Gunnery");
  skcat["Weapon System Skills"].push_back("Gunnery");
  skcat["Cyberpunk Skills"].push_back("Gunnery");
  skcat["Modern Skills"].push_back("Gunnery");
  skcat["Shadowrun Skills"].push_back("Gunnery");
  skcat["Skills"].push_back("Gunnery");

  // Skill Definition: Half-Track
  defaults["Half-Track"] = 6;
  skcat["Reaction-Based Skills"].push_back("Half-Track");
  skcat["Piloting Skills"].push_back("Half-Track");
  skcat["Cyberpunk Skills"].push_back("Half-Track");
  skcat["Modern Skills"].push_back("Half-Track");
  skcat["Shadowrun Skills"].push_back("Half-Track");
  skcat["Skills"].push_back("Half-Track");

  // Skill Definition: Healing
  defaults["Healing"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Healing");
  skcat["Nature Skills"].push_back("Healing");
  skcat["Medieval Skills"].push_back("Healing");
  skcat["Cyberpunk Skills"].push_back("Healing");
  skcat["Modern Skills"].push_back("Healing");
  skcat["Shadowrun Skills"].push_back("Healing");
  skcat["Skills"].push_back("Healing");

  // Skill Definition: Heavy Lasers
  defaults["Heavy Lasers"] = 2;
  skcat["Strength-Based Skills"].push_back("Heavy Lasers");
  skcat["Heavy Firearm Skills"].push_back("Heavy Lasers");
  skcat["Cyberpunk Skills"].push_back("Heavy Lasers");
  skcat["Modern Skills"].push_back("Heavy Lasers");
  skcat["Shadowrun Skills"].push_back("Heavy Lasers");
  skcat["Skills"].push_back("Heavy Lasers");

  // Skill Definition: Heavy Projectors
  defaults["Heavy Projectors"] = 2;
  skcat["Strength-Based Skills"].push_back("Heavy Projectors");
  skcat["Heavy Firearm Skills"].push_back("Heavy Projectors");
  skcat["Cyberpunk Skills"].push_back("Heavy Projectors");
  skcat["Modern Skills"].push_back("Heavy Projectors");
  skcat["Shadowrun Skills"].push_back("Heavy Projectors");
  skcat["Skills"].push_back("Heavy Projectors");

  // Skill Definition: Heavy Rifles
  defaults["Heavy Rifles"] = 2;
  skcat["Strength-Based Skills"].push_back("Heavy Rifles");
  skcat["Heavy Firearm Skills"].push_back("Heavy Rifles");
  skcat["Cyberpunk Skills"].push_back("Heavy Rifles");
  skcat["Modern Skills"].push_back("Heavy Rifles");
  skcat["Shadowrun Skills"].push_back("Heavy Rifles");
  skcat["Skills"].push_back("Heavy Rifles");

  // Skill Definition: Helmsman, Star
  defaults["Helmsman, Star"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Helmsman, Star");
  skcat["Magical Skills"].push_back("Helmsman, Star");
  skcat["Medieval Skills"].push_back("Helmsman, Star");
  skcat["Skills"].push_back("Helmsman, Star");

  // Skill Definition: High-G Combat
  defaults["High-G Combat"] = 4;
  skcat["Intelligence-Based Skills"].push_back("High-G Combat");
  skcat["Combat Environment Skills"].push_back("High-G Combat");
  skcat["Medieval Skills"].push_back("High-G Combat");
  skcat["Cyberpunk Skills"].push_back("High-G Combat");
  skcat["Modern Skills"].push_back("High-G Combat");
  skcat["Shadowrun Skills"].push_back("High-G Combat");
  skcat["Skills"].push_back("High-G Combat");

  // Skill Definition: High-G Ops
  defaults["High-G Ops"] = 4;
  skcat["Intelligence-Based Skills"].push_back("High-G Ops");
  skcat["Specialized Skills"].push_back("High-G Ops");
  skcat["Cyberpunk Skills"].push_back("High-G Ops");
  skcat["Shadowrun Skills"].push_back("High-G Ops");
  skcat["Skills"].push_back("High-G Ops");

  // Skill Definition: Hovercraft
  defaults["Hovercraft"] = 6;
  skcat["Reaction-Based Skills"].push_back("Hovercraft");
  skcat["Piloting Skills"].push_back("Hovercraft");
  skcat["Cyberpunk Skills"].push_back("Hovercraft");
  skcat["Modern Skills"].push_back("Hovercraft");
  skcat["Shadowrun Skills"].push_back("Hovercraft");
  skcat["Skills"].push_back("Hovercraft");

  // Skill Definition: Hover Drone
  defaults["Hover Drone"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Hover Drone");
  skcat["Drone Piloting Skills"].push_back("Hover Drone");
  skcat["Shadowrun Skills"].push_back("Hover Drone");
  skcat["Skills"].push_back("Hover Drone");

  // Skill Definition: Hurling
  defaults["Hurling"] = 2;
  skcat["Strength-Based Skills"].push_back("Hurling");
  skcat["Ranged-Combat Skills"].push_back("Hurling");
  skcat["Medieval Skills"].push_back("Hurling");
  skcat["Cyberpunk Skills"].push_back("Hurling");
  skcat["Modern Skills"].push_back("Hurling");
  skcat["Shadowrun Skills"].push_back("Hurling");
  skcat["Skills"].push_back("Hurling");
  add_wts("Hurling");

  // Skill Definition: Identify Juju
  defaults["Identify Juju"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Identify Juju");
  skcat["Shamanistic Skills"].push_back("Identify Juju");
  skcat["Medieval Skills"].push_back("Identify Juju");
  skcat["Skills"].push_back("Identify Juju");

  // Skill Definition: Interrogation
  defaults["Interrogation"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Interrogation");
  skcat["Social Skills"].push_back("Interrogation");
  skcat["Medieval Skills"].push_back("Interrogation");
  skcat["Cyberpunk Skills"].push_back("Interrogation");
  skcat["Modern Skills"].push_back("Interrogation");
  skcat["Shadowrun Skills"].push_back("Interrogation");
  skcat["Skills"].push_back("Interrogation");

  // Skill Definition: Intimidation
  defaults["Intimidation"] = 5;
  skcat["Willpower-Based Skills"].push_back("Intimidation");
  skcat["Social Skills"].push_back("Intimidation");
  skcat["Medieval Skills"].push_back("Intimidation");
  skcat["Cyberpunk Skills"].push_back("Intimidation");
  skcat["Modern Skills"].push_back("Intimidation");
  skcat["Shadowrun Skills"].push_back("Intimidation");
  skcat["Skills"].push_back("Intimidation");

  // Skill Definition: Jumping
  defaults["Jumping"] = 2;
  skcat["Strength-Based Skills"].push_back("Jumping");
  skcat["Athletic Skills"].push_back("Jumping");
  skcat["Medieval Skills"].push_back("Jumping");
  skcat["Cyberpunk Skills"].push_back("Jumping");
  skcat["Modern Skills"].push_back("Jumping");
  skcat["Shadowrun Skills"].push_back("Jumping");
  skcat["Skills"].push_back("Jumping");

  // Skill Definition: Kicking
  defaults["Kicking"] = 1;
  skcat["Quickness-Based Skills"].push_back("Kicking");
  skcat["Hand-to-Hand Combat Skills"].push_back("Kicking");
  skcat["Medieval Skills"].push_back("Kicking");
  skcat["Cyberpunk Skills"].push_back("Kicking");
  skcat["Modern Skills"].push_back("Kicking");
  skcat["Shadowrun Skills"].push_back("Kicking");
  skcat["Skills"].push_back("Kicking");
  add_wts("Kicking");

  // Skill Definition: Knowledge
  defaults["Knowledge"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Knowledge");
  skcat["Specialized Skills"].push_back("Knowledge");
  skcat["Medieval Skills"].push_back("Knowledge");
  skcat["Cyberpunk Skills"].push_back("Knowledge");
  skcat["Modern Skills"].push_back("Knowledge");
  skcat["Shadowrun Skills"].push_back("Knowledge");
  skcat["Skills"].push_back("Knowledge");

  // Skill Definition: Laser Pistols
  defaults["Laser Pistols"] = 1;
  skcat["Quickness-Based Skills"].push_back("Laser Pistols");
  skcat["Pistol Skills"].push_back("Laser Pistols");
  skcat["Cyberpunk Skills"].push_back("Laser Pistols");
  skcat["Modern Skills"].push_back("Laser Pistols");
  skcat["Shadowrun Skills"].push_back("Laser Pistols");
  skcat["Skills"].push_back("Laser Pistols");
  add_wts("Laser Pistols");

  // Skill Definition: Laser Rifles
  defaults["Laser Rifles"] = 1;
  skcat["Quickness-Based Skills"].push_back("Laser Rifles");
  skcat["Rifle Skills"].push_back("Laser Rifles");
  skcat["Cyberpunk Skills"].push_back("Laser Rifles");
  skcat["Modern Skills"].push_back("Laser Rifles");
  skcat["Shadowrun Skills"].push_back("Laser Rifles");
  skcat["Skills"].push_back("Laser Rifles");
  add_wts("Laser Rifles");

  // Skill Definition: Lasso
  defaults["Lasso"] = 1;
  skcat["Quickness-Based Skills"].push_back("Lasso");
  skcat["Ranged-Combat Skills"].push_back("Lasso");
  skcat["Medieval Skills"].push_back("Lasso");
  skcat["Cyberpunk Skills"].push_back("Lasso");
  skcat["Modern Skills"].push_back("Lasso");
  skcat["Shadowrun Skills"].push_back("Lasso");
  skcat["Skills"].push_back("Lasso");
  add_wts("Lasso");

  // Skill Definition: Launch Weapons
  defaults["Launch Weapons"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Launch Weapons");
  skcat["Weapon System Skills"].push_back("Launch Weapons");
  skcat["Cyberpunk Skills"].push_back("Launch Weapons");
  skcat["Modern Skills"].push_back("Launch Weapons");
  skcat["Shadowrun Skills"].push_back("Launch Weapons");
  skcat["Skills"].push_back("Launch Weapons");

  // Skill Definition: Leadership
  defaults["Leadership"] = 3;
  skcat["Charisma-Based Skills"].push_back("Leadership");
  skcat["Social Skills"].push_back("Leadership");
  skcat["Medieval Skills"].push_back("Leadership");
  skcat["Cyberpunk Skills"].push_back("Leadership");
  skcat["Modern Skills"].push_back("Leadership");
  skcat["Shadowrun Skills"].push_back("Leadership");
  skcat["Skills"].push_back("Leadership");

  // Skill Definition: Leatherworking
  defaults["Leatherworking"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Leatherworking");
  skcat["Creation Skills"].push_back("Leatherworking");
  skcat["Medieval Skills"].push_back("Leatherworking");
  skcat["Skills"].push_back("Leatherworking");

  // Skill Definition: Lifting
  defaults["Lifting"] = 2;
  skcat["Strength-Based Skills"].push_back("Lifting");
  skcat["Athletic Skills"].push_back("Lifting");
  skcat["Medieval Skills"].push_back("Lifting");
  skcat["Cyberpunk Skills"].push_back("Lifting");
  skcat["Modern Skills"].push_back("Lifting");
  skcat["Shadowrun Skills"].push_back("Lifting");
  skcat["Skills"].push_back("Lifting");

  // Skill Definition: Long Blades
  defaults["Long Blades"] = 1;
  skcat["Quickness-Based Skills"].push_back("Long Blades");
  skcat["Melee-Combat Skills"].push_back("Long Blades");
  skcat["Medieval Skills"].push_back("Long Blades");
  skcat["Cyberpunk Skills"].push_back("Long Blades");
  skcat["Modern Skills"].push_back("Long Blades");
  skcat["Shadowrun Skills"].push_back("Long Blades");
  skcat["Skills"].push_back("Long Blades");
  add_wts("Long Blades");

  // Skill Definition: Long Cleaves
  defaults["Long Cleaves"] = 1;
  skcat["Quickness-Based Skills"].push_back("Long Cleaves");
  skcat["Melee-Combat Skills"].push_back("Long Cleaves");
  skcat["Medieval Skills"].push_back("Long Cleaves");
  skcat["Cyberpunk Skills"].push_back("Long Cleaves");
  skcat["Modern Skills"].push_back("Long Cleaves");
  skcat["Shadowrun Skills"].push_back("Long Cleaves");
  skcat["Skills"].push_back("Long Cleaves");
  add_wts("Long Cleaves");

  // Skill Definition: Long Crushing
  defaults["Long Crushing"] = 1;
  skcat["Quickness-Based Skills"].push_back("Long Crushing");
  skcat["Melee-Combat Skills"].push_back("Long Crushing");
  skcat["Medieval Skills"].push_back("Long Crushing");
  skcat["Cyberpunk Skills"].push_back("Long Crushing");
  skcat["Modern Skills"].push_back("Long Crushing");
  skcat["Shadowrun Skills"].push_back("Long Crushing");
  skcat["Skills"].push_back("Long Crushing");
  add_wts("Long Crushing");

  // Skill Definition: Long Flails
  defaults["Long Flails"] = 1;
  skcat["Quickness-Based Skills"].push_back("Long Flails");
  skcat["Melee-Combat Skills"].push_back("Long Flails");
  skcat["Medieval Skills"].push_back("Long Flails");
  skcat["Cyberpunk Skills"].push_back("Long Flails");
  skcat["Modern Skills"].push_back("Long Flails");
  skcat["Shadowrun Skills"].push_back("Long Flails");
  skcat["Skills"].push_back("Long Flails");
  add_wts("Long Flails");

  // Skill Definition: Long Piercing
  defaults["Long Piercing"] = 1;
  skcat["Quickness-Based Skills"].push_back("Long Piercing");
  skcat["Melee-Combat Skills"].push_back("Long Piercing");
  skcat["Medieval Skills"].push_back("Long Piercing");
  skcat["Cyberpunk Skills"].push_back("Long Piercing");
  skcat["Modern Skills"].push_back("Long Piercing");
  skcat["Shadowrun Skills"].push_back("Long Piercing");
  skcat["Skills"].push_back("Long Piercing");
  add_wts("Long Piercing");

  // Skill Definition: Long Staves
  defaults["Long Staves"] = 1;
  skcat["Quickness-Based Skills"].push_back("Long Staves");
  skcat["Melee-Combat Skills"].push_back("Long Staves");
  skcat["Medieval Skills"].push_back("Long Staves");
  skcat["Cyberpunk Skills"].push_back("Long Staves");
  skcat["Modern Skills"].push_back("Long Staves");
  skcat["Shadowrun Skills"].push_back("Long Staves");
  skcat["Skills"].push_back("Long Staves");
  add_wts("Long Staves");

  // Skill Definition: Low-G Combat
  defaults["Low-G Combat"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Low-G Combat");
  skcat["Combat Environment Skills"].push_back("Low-G Combat");
  skcat["Medieval Skills"].push_back("Low-G Combat");
  skcat["Cyberpunk Skills"].push_back("Low-G Combat");
  skcat["Modern Skills"].push_back("Low-G Combat");
  skcat["Shadowrun Skills"].push_back("Low-G Combat");
  skcat["Skills"].push_back("Low-G Combat");

  // Skill Definition: Low-G Ops
  defaults["Low-G Ops"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Low-G Ops");
  skcat["Specialized Skills"].push_back("Low-G Ops");
  skcat["Cyberpunk Skills"].push_back("Low-G Ops");
  skcat["Shadowrun Skills"].push_back("Low-G Ops");
  skcat["Skills"].push_back("Low-G Ops");

  // Skill Definition: Lumberjack
  defaults["Lumberjack"] = 2;
  skcat["Strength-Based Skills"].push_back("Lumberjack");
  skcat["Creation Skills"].push_back("Lumberjack");
  skcat["Medieval Skills"].push_back("Lumberjack");
  skcat["Modern Skills"].push_back("Lumberjack");
  skcat["Skills"].push_back("Lumberjack");

  // Skill Definition: Machine Guns
  defaults["Machine Guns"] = 2;
  skcat["Strength-Based Skills"].push_back("Machine Guns");
  skcat["Heavy Firearm Skills"].push_back("Machine Guns");
  skcat["Cyberpunk Skills"].push_back("Machine Guns");
  skcat["Modern Skills"].push_back("Machine Guns");
  skcat["Shadowrun Skills"].push_back("Machine Guns");
  skcat["Skills"].push_back("Machine Guns");

  // Skill Definition: Machine Pistols
  defaults["Machine Pistols"] = 1;
  skcat["Quickness-Based Skills"].push_back("Machine Pistols");
  skcat["Pistol Skills"].push_back("Machine Pistols");
  skcat["Cyberpunk Skills"].push_back("Machine Pistols");
  skcat["Modern Skills"].push_back("Machine Pistols");
  skcat["Shadowrun Skills"].push_back("Machine Pistols");
  skcat["Skills"].push_back("Machine Pistols");
  add_wts("Machine Pistols");

  // Skill Definition: Magic Manipulation
  defaults["Magic Manipulation"] = 3;
  skcat["Charisma-Based Skills"].push_back("Magic Manipulation");
  skcat["Magical Skills"].push_back("Magic Manipulation");
  skcat["Medieval Skills"].push_back("Magic Manipulation");
  skcat["Skills"].push_back("Magic Manipulation");

  // Skill Definition: Martial Arts, Elven
  defaults["Martial Arts, Elven"] = 5;
  skcat["Willpower-Based Skills"].push_back("Martial Arts, Elven");
  skcat["Martial Arts Skills"].push_back("Martial Arts, Elven");
  skcat["Medieval Skills"].push_back("Martial Arts, Elven");
  skcat["Skills"].push_back("Martial Arts, Elven");

  // Skill Definition: Masonry
  defaults["Masonry"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Masonry");
  skcat["Creation Skills"].push_back("Masonry");
  skcat["Medieval Skills"].push_back("Masonry");
  skcat["Skills"].push_back("Masonry");

  // Skill Definition: Mechanical Build/Repair
  defaults["Mechanical Build/Repair"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Mechanical Build/Repair");
  skcat["Build/Repair Skills"].push_back("Mechanical Build/Repair");
  skcat["Cyberpunk Skills"].push_back("Mechanical Build/Repair");
  skcat["Modern Skills"].push_back("Mechanical Build/Repair");
  skcat["Shadowrun Skills"].push_back("Mechanical Build/Repair");
  skcat["Skills"].push_back("Mechanical Build/Repair");

  // Skill Definition: Mechanics
  defaults["Mechanics"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Mechanics");
  skcat["Technical Skills"].push_back("Mechanics");
  skcat["Medieval Skills"].push_back("Mechanics");
  skcat["Cyberpunk Skills"].push_back("Mechanics");
  skcat["Modern Skills"].push_back("Mechanics");
  skcat["Shadowrun Skills"].push_back("Mechanics");
  skcat["Skills"].push_back("Mechanics");

  // Skill Definition: Metalworking
  defaults["Metalworking"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Metalworking");
  skcat["Creation Skills"].push_back("Metalworking");
  skcat["Medieval Skills"].push_back("Metalworking");
  skcat["Skills"].push_back("Metalworking");

  // Skill Definition: Mindcasting
  defaults["Mindcasting"] = 5;
  skcat["Willpower-Based Skills"].push_back("Mindcasting");
  skcat["Magical Skills"].push_back("Mindcasting");
  skcat["Medieval Skills"].push_back("Mindcasting");
  skcat["Shadowrun Skills"].push_back("Mindcasting");
  skcat["Skills"].push_back("Mindcasting");

  // Skill Definition: Mounted Air Pistols
  defaults["Mounted Air Pistols"] = 1;
  skcat["Quickness-Based Skills"].push_back("Mounted Air Pistols");
  skcat["Mounted Pistol Skills"].push_back("Mounted Air Pistols");
  skcat["Cyberpunk Skills"].push_back("Mounted Air Pistols");
  skcat["Modern Skills"].push_back("Mounted Air Pistols");
  skcat["Shadowrun Skills"].push_back("Mounted Air Pistols");
  skcat["Skills"].push_back("Mounted Air Pistols");
  add_wts("Mounted Air Pistols");

  // Skill Definition: Mounted Archery
  defaults["Mounted Archery"] = 1;
  skcat["Quickness-Based Skills"].push_back("Mounted Archery");
  skcat["Mounted Ranged-Combat Skills"].push_back("Mounted Archery");
  skcat["Medieval Skills"].push_back("Mounted Archery");
  skcat["Cyberpunk Skills"].push_back("Mounted Archery");
  skcat["Modern Skills"].push_back("Mounted Archery");
  skcat["Shadowrun Skills"].push_back("Mounted Archery");
  skcat["Skills"].push_back("Mounted Archery");
  add_wts("Mounted Archery");

  // Skill Definition: Mounted Blades
  defaults["Mounted Blades"] = 1;
  skcat["Quickness-Based Skills"].push_back("Mounted Blades");
  skcat["Mounted Melee-Combat Skills"].push_back("Mounted Blades");
  skcat["Medieval Skills"].push_back("Mounted Blades");
  skcat["Cyberpunk Skills"].push_back("Mounted Blades");
  skcat["Modern Skills"].push_back("Mounted Blades");
  skcat["Shadowrun Skills"].push_back("Mounted Blades");
  skcat["Skills"].push_back("Mounted Blades");
  add_wts("Mounted Blades");

  // Skill Definition: Mounted Blowgun
  defaults["Mounted Blowgun"] = 1;
  skcat["Quickness-Based Skills"].push_back("Mounted Blowgun");
  skcat["Mounted Ranged-Combat Skills"].push_back("Mounted Blowgun");
  skcat["Medieval Skills"].push_back("Mounted Blowgun");
  skcat["Cyberpunk Skills"].push_back("Mounted Blowgun");
  skcat["Modern Skills"].push_back("Mounted Blowgun");
  skcat["Shadowrun Skills"].push_back("Mounted Blowgun");
  skcat["Skills"].push_back("Mounted Blowgun");
  add_wts("Mounted Blowgun");

  // Skill Definition: Mounted Cleaves
  defaults["Mounted Cleaves"] = 1;
  skcat["Quickness-Based Skills"].push_back("Mounted Cleaves");
  skcat["Mounted Melee-Combat Skills"].push_back("Mounted Cleaves");
  skcat["Medieval Skills"].push_back("Mounted Cleaves");
  skcat["Cyberpunk Skills"].push_back("Mounted Cleaves");
  skcat["Modern Skills"].push_back("Mounted Cleaves");
  skcat["Shadowrun Skills"].push_back("Mounted Cleaves");
  skcat["Skills"].push_back("Mounted Cleaves");
  add_wts("Mounted Cleaves");

  // Skill Definition: Mounted Crossbow
  defaults["Mounted Crossbow"] = 1;
  skcat["Quickness-Based Skills"].push_back("Mounted Crossbow");
  skcat["Mounted Ranged-Combat Skills"].push_back("Mounted Crossbow");
  skcat["Medieval Skills"].push_back("Mounted Crossbow");
  skcat["Cyberpunk Skills"].push_back("Mounted Crossbow");
  skcat["Modern Skills"].push_back("Mounted Crossbow");
  skcat["Shadowrun Skills"].push_back("Mounted Crossbow");
  skcat["Skills"].push_back("Mounted Crossbow");
  add_wts("Mounted Crossbow");

  // Skill Definition: Mounted Crushing
  defaults["Mounted Crushing"] = 1;
  skcat["Quickness-Based Skills"].push_back("Mounted Crushing");
  skcat["Mounted Melee-Combat Skills"].push_back("Mounted Crushing");
  skcat["Medieval Skills"].push_back("Mounted Crushing");
  skcat["Cyberpunk Skills"].push_back("Mounted Crushing");
  skcat["Modern Skills"].push_back("Mounted Crushing");
  skcat["Shadowrun Skills"].push_back("Mounted Crushing");
  skcat["Skills"].push_back("Mounted Crushing");
  add_wts("Mounted Crushing");

  // Skill Definition: Mounted Flails
  defaults["Mounted Flails"] = 1;
  skcat["Quickness-Based Skills"].push_back("Mounted Flails");
  skcat["Mounted Melee-Combat Skills"].push_back("Mounted Flails");
  skcat["Medieval Skills"].push_back("Mounted Flails");
  skcat["Cyberpunk Skills"].push_back("Mounted Flails");
  skcat["Modern Skills"].push_back("Mounted Flails");
  skcat["Shadowrun Skills"].push_back("Mounted Flails");
  skcat["Skills"].push_back("Mounted Flails");
  add_wts("Mounted Flails");

  // Skill Definition: Mounted Hurling
  defaults["Mounted Hurling"] = 1;
  skcat["Quickness-Based Skills"].push_back("Mounted Hurling");
  skcat["Mounted Ranged-Combat Skills"].push_back("Mounted Hurling");
  skcat["Medieval Skills"].push_back("Mounted Hurling");
  skcat["Cyberpunk Skills"].push_back("Mounted Hurling");
  skcat["Modern Skills"].push_back("Mounted Hurling");
  skcat["Shadowrun Skills"].push_back("Mounted Hurling");
  skcat["Skills"].push_back("Mounted Hurling");
  add_wts("Mounted Hurling");

  // Skill Definition: Mounted Laser Pistols
  defaults["Mounted Laser Pistols"] = 1;
  skcat["Quickness-Based Skills"].push_back("Mounted Laser Pistols");
  skcat["Mounted Pistol Skills"].push_back("Mounted Laser Pistols");
  skcat["Cyberpunk Skills"].push_back("Mounted Laser Pistols");
  skcat["Modern Skills"].push_back("Mounted Laser Pistols");
  skcat["Shadowrun Skills"].push_back("Mounted Laser Pistols");
  skcat["Skills"].push_back("Mounted Laser Pistols");
  add_wts("Mounted Laser Pistols");

  // Skill Definition: Mounted Machine Pistols
  defaults["Mounted Machine Pistols"] = 1;
  skcat["Quickness-Based Skills"].push_back("Mounted Machine Pistols");
  skcat["Mounted Pistol Skills"].push_back("Mounted Machine Pistols");
  skcat["Cyberpunk Skills"].push_back("Mounted Machine Pistols");
  skcat["Modern Skills"].push_back("Mounted Machine Pistols");
  skcat["Shadowrun Skills"].push_back("Mounted Machine Pistols");
  skcat["Skills"].push_back("Mounted Machine Pistols");
  add_wts("Mounted Machine Pistols");

  // Skill Definition: Mounted Nets
  defaults["Mounted Nets"] = 1;
  skcat["Quickness-Based Skills"].push_back("Mounted Nets");
  skcat["Mounted Ranged-Combat Skills"].push_back("Mounted Nets");
  skcat["Medieval Skills"].push_back("Mounted Nets");
  skcat["Cyberpunk Skills"].push_back("Mounted Nets");
  skcat["Modern Skills"].push_back("Mounted Nets");
  skcat["Shadowrun Skills"].push_back("Mounted Nets");
  skcat["Skills"].push_back("Mounted Nets");
  add_wts("Mounted Nets");

  // Skill Definition: Mounted Piercing
  defaults["Mounted Piercing"] = 1;
  skcat["Quickness-Based Skills"].push_back("Mounted Piercing");
  skcat["Mounted Melee-Combat Skills"].push_back("Mounted Piercing");
  skcat["Medieval Skills"].push_back("Mounted Piercing");
  skcat["Cyberpunk Skills"].push_back("Mounted Piercing");
  skcat["Modern Skills"].push_back("Mounted Piercing");
  skcat["Shadowrun Skills"].push_back("Mounted Piercing");
  skcat["Skills"].push_back("Mounted Piercing");
  add_wts("Mounted Piercing");

  // Skill Definition: Mounted Pistols
  defaults["Mounted Pistols"] = 1;
  skcat["Quickness-Based Skills"].push_back("Mounted Pistols");
  skcat["Mounted Pistol Skills"].push_back("Mounted Pistols");
  skcat["Cyberpunk Skills"].push_back("Mounted Pistols");
  skcat["Modern Skills"].push_back("Mounted Pistols");
  skcat["Shadowrun Skills"].push_back("Mounted Pistols");
  skcat["Skills"].push_back("Mounted Pistols");
  add_wts("Mounted Pistols");

  // Skill Definition: Mounted Plasma Pistols
  defaults["Mounted Plasma Pistols"] = 1;
  skcat["Quickness-Based Skills"].push_back("Mounted Plasma Pistols");
  skcat["Mounted Pistol Skills"].push_back("Mounted Plasma Pistols");
  skcat["Cyberpunk Skills"].push_back("Mounted Plasma Pistols");
  skcat["Modern Skills"].push_back("Mounted Plasma Pistols");
  skcat["Shadowrun Skills"].push_back("Mounted Plasma Pistols");
  skcat["Skills"].push_back("Mounted Plasma Pistols");
  add_wts("Mounted Plasma Pistols");

  // Skill Definition: Mounted Shot Pistols
  defaults["Mounted Shot Pistols"] = 1;
  skcat["Quickness-Based Skills"].push_back("Mounted Shot Pistols");
  skcat["Mounted Pistol Skills"].push_back("Mounted Shot Pistols");
  skcat["Cyberpunk Skills"].push_back("Mounted Shot Pistols");
  skcat["Modern Skills"].push_back("Mounted Shot Pistols");
  skcat["Shadowrun Skills"].push_back("Mounted Shot Pistols");
  skcat["Skills"].push_back("Mounted Shot Pistols");
  add_wts("Mounted Shot Pistols");

  // Skill Definition: Mounted Slings
  defaults["Mounted Slings"] = 1;
  skcat["Quickness-Based Skills"].push_back("Mounted Slings");
  skcat["Mounted Ranged-Combat Skills"].push_back("Mounted Slings");
  skcat["Medieval Skills"].push_back("Mounted Slings");
  skcat["Cyberpunk Skills"].push_back("Mounted Slings");
  skcat["Modern Skills"].push_back("Mounted Slings");
  skcat["Shadowrun Skills"].push_back("Mounted Slings");
  skcat["Skills"].push_back("Mounted Slings");
  add_wts("Mounted Slings");

  // Skill Definition: Mounted SMGs
  defaults["Mounted SMGs"] = 1;
  skcat["Quickness-Based Skills"].push_back("Mounted SMGs");
  skcat["Mounted Pistol Skills"].push_back("Mounted SMGs");
  skcat["Cyberpunk Skills"].push_back("Mounted SMGs");
  skcat["Modern Skills"].push_back("Mounted SMGs");
  skcat["Shadowrun Skills"].push_back("Mounted SMGs");
  skcat["Skills"].push_back("Mounted SMGs");
  add_wts("Mounted SMGs");

  // Skill Definition: Mounted Throwing, Aero
  defaults["Mounted Throwing, Aero"] = 1;
  skcat["Quickness-Based Skills"].push_back("Mounted Throwing, Aero");
  skcat["Mounted Ranged-Combat Skills"].push_back("Mounted Throwing, Aero");
  skcat["Medieval Skills"].push_back("Mounted Throwing, Aero");
  skcat["Cyberpunk Skills"].push_back("Mounted Throwing, Aero");
  skcat["Modern Skills"].push_back("Mounted Throwing, Aero");
  skcat["Shadowrun Skills"].push_back("Mounted Throwing, Aero");
  skcat["Skills"].push_back("Mounted Throwing, Aero");
  add_wts("Mounted Throwing, Aero");

  // Skill Definition: Mounted Throwing, Non-Aero
  defaults["Mounted Throwing, Non-Aero"] = 1;
  skcat["Quickness-Based Skills"].push_back("Mounted Throwing, Non-Aero");
  skcat["Mounted Ranged-Combat Skills"].push_back("Mounted Throwing, Non-Aero");
  skcat["Medieval Skills"].push_back("Mounted Throwing, Non-Aero");
  skcat["Cyberpunk Skills"].push_back("Mounted Throwing, Non-Aero");
  skcat["Modern Skills"].push_back("Mounted Throwing, Non-Aero");
  skcat["Shadowrun Skills"].push_back("Mounted Throwing, Non-Aero");
  skcat["Skills"].push_back("Mounted Throwing, Non-Aero");
  add_wts("Mounted Throwing, Non-Aero");

  // Skill Definition: Mounted Whips
  defaults["Mounted Whips"] = 1;
  skcat["Quickness-Based Skills"].push_back("Mounted Whips");
  skcat["Mounted Ranged-Combat Skills"].push_back("Mounted Whips");
  skcat["Medieval Skills"].push_back("Mounted Whips");
  skcat["Cyberpunk Skills"].push_back("Mounted Whips");
  skcat["Modern Skills"].push_back("Mounted Whips");
  skcat["Shadowrun Skills"].push_back("Mounted Whips");
  skcat["Skills"].push_back("Mounted Whips");
  add_wts("Mounted Whips");

  // Skill Definition: Navigation
  defaults["Navigation"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Navigation");
  skcat["Nature Skills"].push_back("Navigation");
  skcat["Medieval Skills"].push_back("Navigation");
  skcat["Cyberpunk Skills"].push_back("Navigation");
  skcat["Modern Skills"].push_back("Navigation");
  skcat["Shadowrun Skills"].push_back("Navigation");
  skcat["Skills"].push_back("Navigation");

  // Skill Definition: Negotiation
  defaults["Negotiation"] = 3;
  skcat["Charisma-Based Skills"].push_back("Negotiation");
  skcat["Social Skills"].push_back("Negotiation");
  skcat["Medieval Skills"].push_back("Negotiation");
  skcat["Cyberpunk Skills"].push_back("Negotiation");
  skcat["Modern Skills"].push_back("Negotiation");
  skcat["Shadowrun Skills"].push_back("Negotiation");
  skcat["Skills"].push_back("Negotiation");

  // Skill Definition: Nets
  defaults["Nets"] = 1;
  skcat["Quickness-Based Skills"].push_back("Nets");
  skcat["Ranged-Combat Skills"].push_back("Nets");
  skcat["Medieval Skills"].push_back("Nets");
  skcat["Cyberpunk Skills"].push_back("Nets");
  skcat["Modern Skills"].push_back("Nets");
  skcat["Shadowrun Skills"].push_back("Nets");
  skcat["Skills"].push_back("Nets");
  add_wts("Nets");

  // Skill Definition: Neural Interface
  defaults["Neural Interface"] = 5;
  skcat["Willpower-Based Skills"].push_back("Neural Interface");
  skcat["Technical Skills"].push_back("Neural Interface");
  skcat["Cyberpunk Skills"].push_back("Neural Interface");
  skcat["Shadowrun Skills"].push_back("Neural Interface");
  skcat["Skills"].push_back("Neural Interface");

  // Skill Definition: Offhand Air Pistols
  defaults["Offhand Air Pistols"] = 1;
  skcat["Quickness-Based Skills"].push_back("Offhand Air Pistols");
  skcat["Pistol Skills"].push_back("Offhand Air Pistols");
  skcat["Cyberpunk Skills"].push_back("Offhand Air Pistols");
  skcat["Modern Skills"].push_back("Offhand Air Pistols");
  skcat["Shadowrun Skills"].push_back("Offhand Air Pistols");
  skcat["Skills"].push_back("Offhand Air Pistols");
  add_wts("Offhand Air Pistols");

  // Skill Definition: Offhand Blades
  defaults["Offhand Blades"] = 1;
  skcat["Quickness-Based Skills"].push_back("Offhand Blades");
  skcat["Melee-Combat Skills"].push_back("Offhand Blades");
  skcat["Medieval Skills"].push_back("Offhand Blades");
  skcat["Cyberpunk Skills"].push_back("Offhand Blades");
  skcat["Modern Skills"].push_back("Offhand Blades");
  skcat["Shadowrun Skills"].push_back("Offhand Blades");
  skcat["Skills"].push_back("Offhand Blades");
  add_wts("Offhand Blades");

  // Skill Definition: Offhand Cleaves
  defaults["Offhand Cleaves"] = 1;
  skcat["Quickness-Based Skills"].push_back("Offhand Cleaves");
  skcat["Melee-Combat Skills"].push_back("Offhand Cleaves");
  skcat["Medieval Skills"].push_back("Offhand Cleaves");
  skcat["Cyberpunk Skills"].push_back("Offhand Cleaves");
  skcat["Modern Skills"].push_back("Offhand Cleaves");
  skcat["Shadowrun Skills"].push_back("Offhand Cleaves");
  skcat["Skills"].push_back("Offhand Cleaves");
  add_wts("Offhand Cleaves");

  // Skill Definition: Offhand Crossbow
  defaults["Offhand Crossbow"] = 1;
  skcat["Quickness-Based Skills"].push_back("Offhand Crossbow");
  skcat["Ranged-Combat Skills"].push_back("Offhand Crossbow");
  skcat["Medieval Skills"].push_back("Offhand Crossbow");
  skcat["Cyberpunk Skills"].push_back("Offhand Crossbow");
  skcat["Modern Skills"].push_back("Offhand Crossbow");
  skcat["Shadowrun Skills"].push_back("Offhand Crossbow");
  skcat["Skills"].push_back("Offhand Crossbow");
  add_wts("Offhand Crossbow");

  // Skill Definition: Offhand Crushing
  defaults["Offhand Crushing"] = 1;
  skcat["Quickness-Based Skills"].push_back("Offhand Crushing");
  skcat["Melee-Combat Skills"].push_back("Offhand Crushing");
  skcat["Medieval Skills"].push_back("Offhand Crushing");
  skcat["Cyberpunk Skills"].push_back("Offhand Crushing");
  skcat["Modern Skills"].push_back("Offhand Crushing");
  skcat["Shadowrun Skills"].push_back("Offhand Crushing");
  skcat["Skills"].push_back("Offhand Crushing");
  add_wts("Offhand Crushing");

  // Skill Definition: Offhand Flails
  defaults["Offhand Flails"] = 1;
  skcat["Quickness-Based Skills"].push_back("Offhand Flails");
  skcat["Melee-Combat Skills"].push_back("Offhand Flails");
  skcat["Medieval Skills"].push_back("Offhand Flails");
  skcat["Cyberpunk Skills"].push_back("Offhand Flails");
  skcat["Modern Skills"].push_back("Offhand Flails");
  skcat["Shadowrun Skills"].push_back("Offhand Flails");
  skcat["Skills"].push_back("Offhand Flails");
  add_wts("Offhand Flails");

  // Skill Definition: Offhand Hurling
  defaults["Offhand Hurling"] = 1;
  skcat["Quickness-Based Skills"].push_back("Offhand Hurling");
  skcat["Ranged-Combat Skills"].push_back("Offhand Hurling");
  skcat["Medieval Skills"].push_back("Offhand Hurling");
  skcat["Cyberpunk Skills"].push_back("Offhand Hurling");
  skcat["Modern Skills"].push_back("Offhand Hurling");
  skcat["Shadowrun Skills"].push_back("Offhand Hurling");
  skcat["Skills"].push_back("Offhand Hurling");
  add_wts("Offhand Hurling");

  // Skill Definition: Offhand Laser Pistols
  defaults["Offhand Laser Pistols"] = 1;
  skcat["Quickness-Based Skills"].push_back("Offhand Laser Pistols");
  skcat["Pistol Skills"].push_back("Offhand Laser Pistols");
  skcat["Cyberpunk Skills"].push_back("Offhand Laser Pistols");
  skcat["Modern Skills"].push_back("Offhand Laser Pistols");
  skcat["Shadowrun Skills"].push_back("Offhand Laser Pistols");
  skcat["Skills"].push_back("Offhand Laser Pistols");
  add_wts("Offhand Laser Pistols");

  // Skill Definition: Offhand Machine Pistols
  defaults["Offhand Machine Pistols"] = 1;
  skcat["Quickness-Based Skills"].push_back("Offhand Machine Pistols");
  skcat["Pistol Skills"].push_back("Offhand Machine Pistols");
  skcat["Cyberpunk Skills"].push_back("Offhand Machine Pistols");
  skcat["Modern Skills"].push_back("Offhand Machine Pistols");
  skcat["Shadowrun Skills"].push_back("Offhand Machine Pistols");
  skcat["Skills"].push_back("Offhand Machine Pistols");
  add_wts("Offhand Machine Pistols");

  // Skill Definition: Offhand Piercing
  defaults["Offhand Piercing"] = 1;
  skcat["Quickness-Based Skills"].push_back("Offhand Piercing");
  skcat["Melee-Combat Skills"].push_back("Offhand Piercing");
  skcat["Medieval Skills"].push_back("Offhand Piercing");
  skcat["Cyberpunk Skills"].push_back("Offhand Piercing");
  skcat["Modern Skills"].push_back("Offhand Piercing");
  skcat["Shadowrun Skills"].push_back("Offhand Piercing");
  skcat["Skills"].push_back("Offhand Piercing");
  add_wts("Offhand Piercing");

  // Skill Definition: Offhand Pistols
  defaults["Offhand Pistols"] = 1;
  skcat["Quickness-Based Skills"].push_back("Offhand Pistols");
  skcat["Pistol Skills"].push_back("Offhand Pistols");
  skcat["Cyberpunk Skills"].push_back("Offhand Pistols");
  skcat["Modern Skills"].push_back("Offhand Pistols");
  skcat["Shadowrun Skills"].push_back("Offhand Pistols");
  skcat["Skills"].push_back("Offhand Pistols");
  add_wts("Offhand Pistols");

  // Skill Definition: Offhand Plasma Pistols
  defaults["Offhand Plasma Pistols"] = 1;
  skcat["Quickness-Based Skills"].push_back("Offhand Plasma Pistols");
  skcat["Pistol Skills"].push_back("Offhand Plasma Pistols");
  skcat["Cyberpunk Skills"].push_back("Offhand Plasma Pistols");
  skcat["Modern Skills"].push_back("Offhand Plasma Pistols");
  skcat["Shadowrun Skills"].push_back("Offhand Plasma Pistols");
  skcat["Skills"].push_back("Offhand Plasma Pistols");
  add_wts("Offhand Plasma Pistols");

  // Skill Definition: Offhand Punching
  defaults["Offhand Punching"] = 1;
  skcat["Quickness-Based Skills"].push_back("Offhand Punching");
  skcat["Medieval Skills"].push_back("Offhand Punching");
  skcat["Cyberpunk Skills"].push_back("Offhand Punching");
  skcat["Modern Skills"].push_back("Offhand Punching");
  skcat["Shadowrun Skills"].push_back("Offhand Punching");
  skcat["Skills"].push_back("Offhand Punching");

  // Skill Definition: Offhand Shot Pistols
  defaults["Offhand Shot Pistols"] = 1;
  skcat["Quickness-Based Skills"].push_back("Offhand Shot Pistols");
  skcat["Pistol Skills"].push_back("Offhand Shot Pistols");
  skcat["Cyberpunk Skills"].push_back("Offhand Shot Pistols");
  skcat["Modern Skills"].push_back("Offhand Shot Pistols");
  skcat["Shadowrun Skills"].push_back("Offhand Shot Pistols");
  skcat["Skills"].push_back("Offhand Shot Pistols");
  add_wts("Offhand Shot Pistols");

  // Skill Definition: Offhand SMGs
  defaults["Offhand SMGs"] = 1;
  skcat["Quickness-Based Skills"].push_back("Offhand SMGs");
  skcat["Pistol Skills"].push_back("Offhand SMGs");
  skcat["Cyberpunk Skills"].push_back("Offhand SMGs");
  skcat["Modern Skills"].push_back("Offhand SMGs");
  skcat["Shadowrun Skills"].push_back("Offhand SMGs");
  skcat["Skills"].push_back("Offhand SMGs");
  add_wts("Offhand SMGs");

  // Skill Definition: Offhand Staves
  defaults["Offhand Staves"] = 1;
  skcat["Quickness-Based Skills"].push_back("Offhand Staves");
  skcat["Melee-Combat Skills"].push_back("Offhand Staves");
  skcat["Medieval Skills"].push_back("Offhand Staves");
  skcat["Cyberpunk Skills"].push_back("Offhand Staves");
  skcat["Modern Skills"].push_back("Offhand Staves");
  skcat["Shadowrun Skills"].push_back("Offhand Staves");
  skcat["Skills"].push_back("Offhand Staves");
  add_wts("Offhand Staves");

  // Skill Definition: Offhand Throwing, Aero
  defaults["Offhand Throwing, Aero"] = 1;
  skcat["Quickness-Based Skills"].push_back("Offhand Throwing, Aero");
  skcat["Ranged-Combat Skills"].push_back("Offhand Throwing, Aero");
  skcat["Medieval Skills"].push_back("Offhand Throwing, Aero");
  skcat["Cyberpunk Skills"].push_back("Offhand Throwing, Aero");
  skcat["Modern Skills"].push_back("Offhand Throwing, Aero");
  skcat["Shadowrun Skills"].push_back("Offhand Throwing, Aero");
  skcat["Skills"].push_back("Offhand Throwing, Aero");
  add_wts("Offhand Throwing, Aero");

  // Skill Definition: Offhand Throwing, Non-Aero
  defaults["Offhand Throwing, Non-Aero"] = 1;
  skcat["Quickness-Based Skills"].push_back("Offhand Throwing, Non-Aero");
  skcat["Ranged-Combat Skills"].push_back("Offhand Throwing, Non-Aero");
  skcat["Medieval Skills"].push_back("Offhand Throwing, Non-Aero");
  skcat["Cyberpunk Skills"].push_back("Offhand Throwing, Non-Aero");
  skcat["Modern Skills"].push_back("Offhand Throwing, Non-Aero");
  skcat["Shadowrun Skills"].push_back("Offhand Throwing, Non-Aero");
  skcat["Skills"].push_back("Offhand Throwing, Non-Aero");
  add_wts("Offhand Throwing, Non-Aero");

  // Skill Definition: Offhand Whips
  defaults["Offhand Whips"] = 1;
  skcat["Quickness-Based Skills"].push_back("Offhand Whips");
  skcat["Ranged-Combat Skills"].push_back("Offhand Whips");
  skcat["Medieval Skills"].push_back("Offhand Whips");
  skcat["Cyberpunk Skills"].push_back("Offhand Whips");
  skcat["Modern Skills"].push_back("Offhand Whips");
  skcat["Shadowrun Skills"].push_back("Offhand Whips");
  skcat["Skills"].push_back("Offhand Whips");
  add_wts("Offhand Whips");

  // Skill Definition: Perception
  defaults["Perception"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Perception");
  skcat["Specialized Skills"].push_back("Perception");
  skcat["Medieval Skills"].push_back("Perception");
  skcat["Cyberpunk Skills"].push_back("Perception");
  skcat["Modern Skills"].push_back("Perception");
  skcat["Shadowrun Skills"].push_back("Perception");
  skcat["Expert Skills"].push_back("Perception");
  skcat["Skills"].push_back("Perception");

  // Skill Definition: Performance
  defaults["Performance"] = 3;
  skcat["Charisma-Based Skills"].push_back("Performance");
  skcat["Social Skills"].push_back("Performance");
  skcat["Medieval Skills"].push_back("Performance");
  skcat["Cyberpunk Skills"].push_back("Performance");
  skcat["Modern Skills"].push_back("Performance");
  skcat["Shadowrun Skills"].push_back("Performance");
  skcat["Skills"].push_back("Performance");

  // Skill Definition: Pistols
  defaults["Pistols"] = 1;
  skcat["Quickness-Based Skills"].push_back("Pistols");
  skcat["Pistol Skills"].push_back("Pistols");
  skcat["Cyberpunk Skills"].push_back("Pistols");
  skcat["Modern Skills"].push_back("Pistols");
  skcat["Shadowrun Skills"].push_back("Pistols");
  skcat["Skills"].push_back("Pistols");
  add_wts("Pistols");

  // Skill Definition: Plasma Cannons
  defaults["Plasma Cannons"] = 2;
  skcat["Strength-Based Skills"].push_back("Plasma Cannons");
  skcat["Heavy Firearm Skills"].push_back("Plasma Cannons");
  skcat["Cyberpunk Skills"].push_back("Plasma Cannons");
  skcat["Modern Skills"].push_back("Plasma Cannons");
  skcat["Shadowrun Skills"].push_back("Plasma Cannons");
  skcat["Skills"].push_back("Plasma Cannons");

  // Skill Definition: Plasma Pistols
  defaults["Plasma Pistols"] = 1;
  skcat["Quickness-Based Skills"].push_back("Plasma Pistols");
  skcat["Pistol Skills"].push_back("Plasma Pistols");
  skcat["Cyberpunk Skills"].push_back("Plasma Pistols");
  skcat["Modern Skills"].push_back("Plasma Pistols");
  skcat["Shadowrun Skills"].push_back("Plasma Pistols");
  skcat["Skills"].push_back("Plasma Pistols");
  add_wts("Plasma Pistols");

  // Skill Definition: Plasma Rifles
  defaults["Plasma Rifles"] = 1;
  skcat["Quickness-Based Skills"].push_back("Plasma Rifles");
  skcat["Rifle Skills"].push_back("Plasma Rifles");
  skcat["Cyberpunk Skills"].push_back("Plasma Rifles");
  skcat["Modern Skills"].push_back("Plasma Rifles");
  skcat["Shadowrun Skills"].push_back("Plasma Rifles");
  skcat["Skills"].push_back("Plasma Rifles");
  add_wts("Plasma Rifles");

  // Skill Definition: Power Distribution
  defaults["Power Distribution"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Power Distribution");
  skcat["Technical Skills"].push_back("Power Distribution");
  skcat["Cyberpunk Skills"].push_back("Power Distribution");
  skcat["Modern Skills"].push_back("Power Distribution");
  skcat["Shadowrun Skills"].push_back("Power Distribution");
  skcat["Skills"].push_back("Power Distribution");

  // Skill Definition: Power Suit
  defaults["Power Suit"] = 6;
  skcat["Reaction-Based Skills"].push_back("Power Suit");
  skcat["Piloting Skills"].push_back("Power Suit");
  skcat["Cyberpunk Skills"].push_back("Power Suit");
  skcat["Shadowrun Skills"].push_back("Power Suit");
  skcat["Skills"].push_back("Power Suit");

  // Skill Definition: Prepared Simple
  defaults["Prepared Simple"] = 5;
  skcat["Willpower-Based Skills"].push_back("Prepared Simple");
  skcat["Star Magical Skills"].push_back("Prepared Simple");
  skcat["Medieval Skills"].push_back("Prepared Simple");
  skcat["Skills"].push_back("Prepared Simple");

  // Skill Definition: Punching
  defaults["Punching"] = 1;
  skcat["Quickness-Based Skills"].push_back("Punching");
  skcat["Hand-to-Hand Combat Skills"].push_back("Punching");
  skcat["Medieval Skills"].push_back("Punching");
  skcat["Cyberpunk Skills"].push_back("Punching");
  skcat["Modern Skills"].push_back("Punching");
  skcat["Shadowrun Skills"].push_back("Punching");
  skcat["Skills"].push_back("Punching");
  add_wts("Punching");

  // Skill Definition: Quickdraw
  defaults["Quickdraw"] = 1;
  skcat["Quickness-Based Skills"].push_back("Quickdraw");
  skcat["Specialized Skills"].push_back("Quickdraw");
  skcat["Medieval Skills"].push_back("Quickdraw");
  skcat["Cyberpunk Skills"].push_back("Quickdraw");
  skcat["Modern Skills"].push_back("Quickdraw");
  skcat["Shadowrun Skills"].push_back("Quickdraw");
  skcat["Skills"].push_back("Quickdraw");

  // Skill Definition: Race Car
  defaults["Race Car"] = 6;
  skcat["Reaction-Based Skills"].push_back("Race Car");
  skcat["Piloting Skills"].push_back("Race Car");
  skcat["Cyberpunk Skills"].push_back("Race Car");
  skcat["Modern Skills"].push_back("Race Car");
  skcat["Shadowrun Skills"].push_back("Race Car");
  skcat["Skills"].push_back("Race Car");

  // Skill Definition: Refine Juju
  defaults["Refine Juju"] = 3;
  skcat["Charisma-Based Skills"].push_back("Refine Juju");
  skcat["Shamanistic Skills"].push_back("Refine Juju");
  skcat["Medieval Skills"].push_back("Refine Juju");
  skcat["Skills"].push_back("Refine Juju");

  // Skill Definition: Research
  defaults["Research"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Research");
  skcat["Specialized Skills"].push_back("Research");
  skcat["Medieval Skills"].push_back("Research");
  skcat["Cyberpunk Skills"].push_back("Research");
  skcat["Modern Skills"].push_back("Research");
  skcat["Shadowrun Skills"].push_back("Research");
  skcat["Specialty Skills"].push_back("Research");
  skcat["Skills"].push_back("Research");

  // Skill Definition: Riding
  defaults["Riding"] = 1;
  skcat["Quickness-Based Skills"].push_back("Riding");
  skcat["Athletic Skills"].push_back("Riding");
  skcat["Medieval Skills"].push_back("Riding");
  skcat["Cyberpunk Skills"].push_back("Riding");
  skcat["Modern Skills"].push_back("Riding");
  skcat["Shadowrun Skills"].push_back("Riding");
  skcat["Skills"].push_back("Riding");

  // Skill Definition: Rifles
  defaults["Rifles"] = 1;
  skcat["Quickness-Based Skills"].push_back("Rifles");
  skcat["Rifle Skills"].push_back("Rifles");
  skcat["Cyberpunk Skills"].push_back("Rifles");
  skcat["Modern Skills"].push_back("Rifles");
  skcat["Shadowrun Skills"].push_back("Rifles");
  skcat["Skills"].push_back("Rifles");
  add_wts("Rifles");

  // Skill Definition: Ritual Conjuring
  defaults["Ritual Conjuring"] = 5;
  skcat["Willpower-Based Skills"].push_back("Ritual Conjuring");
  skcat["Magical Skills"].push_back("Ritual Conjuring");
  skcat["Medieval Skills"].push_back("Ritual Conjuring");
  skcat["Shadowrun Skills"].push_back("Ritual Conjuring");
  skcat["Skills"].push_back("Ritual Conjuring");

  // Skill Definition: Ritual Spellcasting
  defaults["Ritual Spellcasting"] = 5;
  skcat["Willpower-Based Skills"].push_back("Ritual Spellcasting");
  skcat["Magical Skills"].push_back("Ritual Spellcasting");
  skcat["Medieval Skills"].push_back("Ritual Spellcasting");
  skcat["Shadowrun Skills"].push_back("Ritual Spellcasting");
  skcat["Skills"].push_back("Ritual Spellcasting");

  // Skill Definition: Rod Targeting
  defaults["Rod Targeting"] = 1;
  skcat["Quickness-Based Skills"].push_back("Rod Targeting");
  skcat["Magical Skills"].push_back("Rod Targeting");
  skcat["Medieval Skills"].push_back("Rod Targeting");
  skcat["Skills"].push_back("Rod Targeting");

  // Skill Definition: Rotorcraft
  defaults["Rotorcraft"] = 6;
  skcat["Reaction-Based Skills"].push_back("Rotorcraft");
  skcat["Piloting Skills"].push_back("Rotorcraft");
  skcat["Cyberpunk Skills"].push_back("Rotorcraft");
  skcat["Modern Skills"].push_back("Rotorcraft");
  skcat["Shadowrun Skills"].push_back("Rotorcraft");
  skcat["Skills"].push_back("Rotorcraft");

  // Skill Definition: Rotor Drone
  defaults["Rotor Drone"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Rotor Drone");
  skcat["Drone Piloting Skills"].push_back("Rotor Drone");
  skcat["Shadowrun Skills"].push_back("Rotor Drone");
  skcat["Skills"].push_back("Rotor Drone");

  // Skill Definition: Running
  defaults["Running"] = 2;
  skcat["Strength-Based Skills"].push_back("Running");
  skcat["Athletic Skills"].push_back("Running");
  skcat["Medieval Skills"].push_back("Running");
  skcat["Cyberpunk Skills"].push_back("Running");
  skcat["Modern Skills"].push_back("Running");
  skcat["Shadowrun Skills"].push_back("Running");
  skcat["Skills"].push_back("Running");

  // Skill Definition: Saurian Line Attacking
  defaults["Saurian Line Attacking"] = 2;
  skcat["Strength-Based Skills"].push_back("Saurian Line Attacking");
  skcat["Star Martial Arts"].push_back("Saurian Line Attacking");
  skcat["Medieval Skills"].push_back("Saurian Line Attacking");
  skcat["Skills"].push_back("Saurian Line Attacking");

  // Skill Definition: Security
  defaults["Security"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Security");
  skcat["Technical Skills"].push_back("Security");
  skcat["Medieval Skills"].push_back("Security");
  skcat["Cyberpunk Skills"].push_back("Security");
  skcat["Modern Skills"].push_back("Security");
  skcat["Shadowrun Skills"].push_back("Security");
  skcat["Skills"].push_back("Security");

  // Skill Definition: Seduction
  defaults["Seduction"] = 3;
  skcat["Charisma-Based Skills"].push_back("Seduction");
  skcat["Social Skills"].push_back("Seduction");
  skcat["Medieval Skills"].push_back("Seduction");
  skcat["Cyberpunk Skills"].push_back("Seduction");
  skcat["Modern Skills"].push_back("Seduction");
  skcat["Shadowrun Skills"].push_back("Seduction");
  skcat["Skills"].push_back("Seduction");

  // Skill Definition: Self Artificing
  defaults["Self Artificing"] = 3;
  skcat["Charisma-Based Skills"].push_back("Self Artificing");
  skcat["Magical Skills"].push_back("Self Artificing");
  skcat["Medieval Skills"].push_back("Self Artificing");
  skcat["Skills"].push_back("Self Artificing");

  // Skill Definition: Sensors
  defaults["Sensors"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Sensors");
  skcat["Technical Skills"].push_back("Sensors");
  skcat["Cyberpunk Skills"].push_back("Sensors");
  skcat["Modern Skills"].push_back("Sensors");
  skcat["Shadowrun Skills"].push_back("Sensors");
  skcat["Skills"].push_back("Sensors");

  // Skill Definition: Shields
  defaults["Shields"] = 2;
  skcat["Strength-Based Skills"].push_back("Shields");
  skcat["Melee-Combat Skills"].push_back("Shields");
  skcat["Medieval Skills"].push_back("Shields");
  skcat["Cyberpunk Skills"].push_back("Shields");
  skcat["Modern Skills"].push_back("Shields");
  skcat["Shadowrun Skills"].push_back("Shields");
  skcat["Skills"].push_back("Shields");
  add_wts("Shields");

  // Skill Definition: Ship
  defaults["Ship"] = 1;
  skcat["Quickness-Based Skills"].push_back("Ship");
  skcat["Piloting Skills"].push_back("Ship");
  skcat["Medieval Skills"].push_back("Ship");
  skcat["Cyberpunk Skills"].push_back("Ship");
  skcat["Modern Skills"].push_back("Ship");
  skcat["Shadowrun Skills"].push_back("Ship");
  skcat["Skills"].push_back("Ship");

  // Skill Definition: Ship, Powered
  defaults["Ship, Powered"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Ship, Powered");
  skcat["Piloting Skills"].push_back("Ship, Powered");
  skcat["Cyberpunk Skills"].push_back("Ship, Powered");
  skcat["Modern Skills"].push_back("Ship, Powered");
  skcat["Shadowrun Skills"].push_back("Ship, Powered");
  skcat["Skills"].push_back("Ship, Powered");

  // Skill Definition: Shipwright
  defaults["Shipwright"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Shipwright");
  skcat["Creation Skills"].push_back("Shipwright");
  skcat["Medieval Skills"].push_back("Shipwright");
  skcat["Skills"].push_back("Shipwright");

  // Skill Definition: Short Blades
  defaults["Short Blades"] = 1;
  skcat["Quickness-Based Skills"].push_back("Short Blades");
  skcat["Melee-Combat Skills"].push_back("Short Blades");
  skcat["Medieval Skills"].push_back("Short Blades");
  skcat["Cyberpunk Skills"].push_back("Short Blades");
  skcat["Modern Skills"].push_back("Short Blades");
  skcat["Shadowrun Skills"].push_back("Short Blades");
  skcat["Skills"].push_back("Short Blades");
  add_wts("Short Blades");

  // Skill Definition: Short Cleaves
  defaults["Short Cleaves"] = 1;
  skcat["Quickness-Based Skills"].push_back("Short Cleaves");
  skcat["Melee-Combat Skills"].push_back("Short Cleaves");
  skcat["Medieval Skills"].push_back("Short Cleaves");
  skcat["Cyberpunk Skills"].push_back("Short Cleaves");
  skcat["Modern Skills"].push_back("Short Cleaves");
  skcat["Shadowrun Skills"].push_back("Short Cleaves");
  skcat["Skills"].push_back("Short Cleaves");
  add_wts("Short Cleaves");

  // Skill Definition: Short Crushing
  defaults["Short Crushing"] = 1;
  skcat["Quickness-Based Skills"].push_back("Short Crushing");
  skcat["Melee-Combat Skills"].push_back("Short Crushing");
  skcat["Medieval Skills"].push_back("Short Crushing");
  skcat["Cyberpunk Skills"].push_back("Short Crushing");
  skcat["Modern Skills"].push_back("Short Crushing");
  skcat["Shadowrun Skills"].push_back("Short Crushing");
  skcat["Skills"].push_back("Short Crushing");
  add_wts("Short Crushing");

  // Skill Definition: Short Flails
  defaults["Short Flails"] = 1;
  skcat["Quickness-Based Skills"].push_back("Short Flails");
  skcat["Melee-Combat Skills"].push_back("Short Flails");
  skcat["Medieval Skills"].push_back("Short Flails");
  skcat["Cyberpunk Skills"].push_back("Short Flails");
  skcat["Modern Skills"].push_back("Short Flails");
  skcat["Shadowrun Skills"].push_back("Short Flails");
  skcat["Skills"].push_back("Short Flails");
  add_wts("Short Flails");

  // Skill Definition: Short Piercing
  defaults["Short Piercing"] = 1;
  skcat["Quickness-Based Skills"].push_back("Short Piercing");
  skcat["Melee-Combat Skills"].push_back("Short Piercing");
  skcat["Medieval Skills"].push_back("Short Piercing");
  skcat["Cyberpunk Skills"].push_back("Short Piercing");
  skcat["Modern Skills"].push_back("Short Piercing");
  skcat["Shadowrun Skills"].push_back("Short Piercing");
  skcat["Skills"].push_back("Short Piercing");
  add_wts("Short Piercing");

  // Skill Definition: Short Staves
  defaults["Short Staves"] = 1;
  skcat["Quickness-Based Skills"].push_back("Short Staves");
  skcat["Melee-Combat Skills"].push_back("Short Staves");
  skcat["Medieval Skills"].push_back("Short Staves");
  skcat["Cyberpunk Skills"].push_back("Short Staves");
  skcat["Modern Skills"].push_back("Short Staves");
  skcat["Shadowrun Skills"].push_back("Short Staves");
  skcat["Skills"].push_back("Short Staves");
  add_wts("Short Staves");

  // Skill Definition: Shotguns
  defaults["Shotguns"] = 1;
  skcat["Quickness-Based Skills"].push_back("Shotguns");
  skcat["Rifle Skills"].push_back("Shotguns");
  skcat["Cyberpunk Skills"].push_back("Shotguns");
  skcat["Modern Skills"].push_back("Shotguns");
  skcat["Shadowrun Skills"].push_back("Shotguns");
  skcat["Skills"].push_back("Shotguns");
  add_wts("Shotguns");

  // Skill Definition: Shot Pistols
  defaults["Shot Pistols"] = 1;
  skcat["Quickness-Based Skills"].push_back("Shot Pistols");
  skcat["Pistol Skills"].push_back("Shot Pistols");
  skcat["Cyberpunk Skills"].push_back("Shot Pistols");
  skcat["Modern Skills"].push_back("Shot Pistols");
  skcat["Shadowrun Skills"].push_back("Shot Pistols");
  skcat["Skills"].push_back("Shot Pistols");
  add_wts("Shot Pistols");

  // Skill Definition: Skiing
  defaults["Skiing"] = 1;
  skcat["Quickness-Based Skills"].push_back("Skiing");
  skcat["Athletic Skills"].push_back("Skiing");
  skcat["Medieval Skills"].push_back("Skiing");
  skcat["Cyberpunk Skills"].push_back("Skiing");
  skcat["Modern Skills"].push_back("Skiing");
  skcat["Shadowrun Skills"].push_back("Skiing");
  skcat["Skills"].push_back("Skiing");

  // Skill Definition: Sled
  defaults["Sled"] = 1;
  skcat["Quickness-Based Skills"].push_back("Sled");
  skcat["Piloting Skills"].push_back("Sled");
  skcat["Medieval Skills"].push_back("Sled");
  skcat["Cyberpunk Skills"].push_back("Sled");
  skcat["Modern Skills"].push_back("Sled");
  skcat["Shadowrun Skills"].push_back("Sled");
  skcat["Skills"].push_back("Sled");

  // Skill Definition: Slings
  defaults["Slings"] = 1;
  skcat["Quickness-Based Skills"].push_back("Slings");
  skcat["Ranged-Combat Skills"].push_back("Slings");
  skcat["Medieval Skills"].push_back("Slings");
  skcat["Cyberpunk Skills"].push_back("Slings");
  skcat["Modern Skills"].push_back("Slings");
  skcat["Shadowrun Skills"].push_back("Slings");
  skcat["Skills"].push_back("Slings");
  add_wts("Slings");

  // Skill Definition: Smelting
  defaults["Smelting"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Smelting");
  skcat["Creation Skills"].push_back("Smelting");
  skcat["Medieval Skills"].push_back("Smelting");
  skcat["Skills"].push_back("Smelting");

  // Skill Definition: SMGs
  defaults["SMGs"] = 1;
  skcat["Quickness-Based Skills"].push_back("SMGs");
  skcat["Pistol Skills"].push_back("SMGs");
  skcat["Cyberpunk Skills"].push_back("SMGs");
  skcat["Modern Skills"].push_back("SMGs");
  skcat["Shadowrun Skills"].push_back("SMGs");
  skcat["Skills"].push_back("SMGs");
  add_wts("SMGs");

  // Skill Definition: Snowmobile
  defaults["Snowmobile"] = 6;
  skcat["Reaction-Based Skills"].push_back("Snowmobile");
  skcat["Piloting Skills"].push_back("Snowmobile");
  skcat["Cyberpunk Skills"].push_back("Snowmobile");
  skcat["Modern Skills"].push_back("Snowmobile");
  skcat["Shadowrun Skills"].push_back("Snowmobile");
  skcat["Skills"].push_back("Snowmobile");

  // Skill Definition: Spacecraft
  defaults["Spacecraft"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Spacecraft");
  skcat["Piloting Skills"].push_back("Spacecraft");
  skcat["Cyberpunk Skills"].push_back("Spacecraft");
  skcat["Modern Skills"].push_back("Spacecraft");
  skcat["Shadowrun Skills"].push_back("Spacecraft");
  skcat["Skills"].push_back("Spacecraft");

  // Skill Definition: Spellcasting
  defaults["Spellcasting"] = 3;
  skcat["Charisma-Based Skills"].push_back("Spellcasting");
  skcat["Magical Skills"].push_back("Spellcasting");
  skcat["Medieval Skills"].push_back("Spellcasting");
  skcat["Shadowrun Skills"].push_back("Spellcasting");
  skcat["Skills"].push_back("Spellcasting");

  // Skill Definition: Spellcraft
  defaults["Spellcraft"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Spellcraft");
  skcat["Magical Skills"].push_back("Spellcraft");
  skcat["Medieval Skills"].push_back("Spellcraft");
  skcat["Shadowrun Skills"].push_back("Spellcraft");
  skcat["Skills"].push_back("Spellcraft");

  // Skill Definition: Spellcraft, Star
  defaults["Spellcraft, Star"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Spellcraft, Star");
  skcat["Star Magical Skills"].push_back("Spellcraft, Star");
  skcat["Medieval Skills"].push_back("Spellcraft, Star");
  skcat["Skills"].push_back("Spellcraft, Star");

  // Skill Definition: Spell Preparation
  defaults["Spell Preparation"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Spell Preparation");
  skcat["Star Magical Skills"].push_back("Spell Preparation");
  skcat["Medieval Skills"].push_back("Spell Preparation");
  skcat["Skills"].push_back("Spell Preparation");

  // Skill Definition: Spell Targeting
  defaults["Spell Targeting"] = 1;
  skcat["Quickness-Based Skills"].push_back("Spell Targeting");
  skcat["Magical Skills"].push_back("Spell Targeting");
  skcat["Medieval Skills"].push_back("Spell Targeting");
  skcat["Shadowrun Skills"].push_back("Spell Targeting");
  skcat["Skills"].push_back("Spell Targeting");

  // Skill Definition: Sprinting
  defaults["Sprinting"] = 1;
  skcat["Quickness-Based Skills"].push_back("Sprinting");
  skcat["Athletic Skills"].push_back("Sprinting");
  skcat["Medieval Skills"].push_back("Sprinting");
  skcat["Cyberpunk Skills"].push_back("Sprinting");
  skcat["Modern Skills"].push_back("Sprinting");
  skcat["Shadowrun Skills"].push_back("Sprinting");
  skcat["Skills"].push_back("Sprinting");

  // Skill Definition: Staff Slings
  defaults["Staff Slings"] = 1;
  skcat["Quickness-Based Skills"].push_back("Staff Slings");
  skcat["Ranged-Combat Skills"].push_back("Staff Slings");
  skcat["Medieval Skills"].push_back("Staff Slings");
  skcat["Cyberpunk Skills"].push_back("Staff Slings");
  skcat["Modern Skills"].push_back("Staff Slings");
  skcat["Shadowrun Skills"].push_back("Staff Slings");
  skcat["Skills"].push_back("Staff Slings");
  add_wts("Staff Slings");

  // Skill Definition: Staff Targeting
  defaults["Staff Targeting"] = 1;
  skcat["Quickness-Based Skills"].push_back("Staff Targeting");
  skcat["Magical Skills"].push_back("Staff Targeting");
  skcat["Medieval Skills"].push_back("Staff Targeting");
  skcat["Skills"].push_back("Staff Targeting");

  // Skill Definition: Stealth
  defaults["Stealth"] = 1;
  skcat["Quickness-Based Skills"].push_back("Stealth");
  skcat["Athletic Skills"].push_back("Stealth");
  skcat["Medieval Skills"].push_back("Stealth");
  skcat["Cyberpunk Skills"].push_back("Stealth");
  skcat["Modern Skills"].push_back("Stealth");
  skcat["Shadowrun Skills"].push_back("Stealth");
  skcat["Skills"].push_back("Stealth");

  // Skill Definition: Style
  defaults["Style"] = 3;
  skcat["Charisma-Based Skills"].push_back("Style");
  skcat["Social Skills"].push_back("Style");
  skcat["Medieval Skills"].push_back("Style");
  skcat["Cyberpunk Skills"].push_back("Style");
  skcat["Modern Skills"].push_back("Style");
  skcat["Shadowrun Skills"].push_back("Style");
  skcat["Skills"].push_back("Style");

  // Skill Definition: Surgery
  defaults["Surgery"] = 1;
  skcat["Quickness-Based Skills"].push_back("Surgery");
  skcat["Technical Skills"].push_back("Surgery");
  skcat["Cyberpunk Skills"].push_back("Surgery");
  skcat["Modern Skills"].push_back("Surgery");
  skcat["Shadowrun Skills"].push_back("Surgery");
  skcat["Skills"].push_back("Surgery");

  // Skill Definition: Survival
  defaults["Survival"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Survival");
  skcat["Nature Skills"].push_back("Survival");
  skcat["Medieval Skills"].push_back("Survival");
  skcat["Cyberpunk Skills"].push_back("Survival");
  skcat["Modern Skills"].push_back("Survival");
  skcat["Shadowrun Skills"].push_back("Survival");
  skcat["Skills"].push_back("Survival");

  // Skill Definition: Swimming
  defaults["Swimming"] = 1;
  skcat["Quickness-Based Skills"].push_back("Swimming");
  skcat["Athletic Skills"].push_back("Swimming");
  skcat["Medieval Skills"].push_back("Swimming");
  skcat["Cyberpunk Skills"].push_back("Swimming");
  skcat["Modern Skills"].push_back("Swimming");
  skcat["Shadowrun Skills"].push_back("Swimming");
  skcat["Skills"].push_back("Swimming");

  // Skill Definition: Swordsmithing
  defaults["Swordsmithing"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Swordsmithing");
  skcat["Creation Skills"].push_back("Swordsmithing");
  skcat["Medieval Skills"].push_back("Swordsmithing");
  skcat["Skills"].push_back("Swordsmithing");

  // Skill Definition: Tactics
  defaults["Tactics"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Tactics");
  skcat["Specialized Skills"].push_back("Tactics");
  skcat["Medieval Skills"].push_back("Tactics");
  skcat["Cyberpunk Skills"].push_back("Tactics");
  skcat["Modern Skills"].push_back("Tactics");
  skcat["Shadowrun Skills"].push_back("Tactics");
  skcat["Skills"].push_back("Tactics");

  // Skill Definition: Talismongery
  defaults["Talismongery"] = 5;
  skcat["Willpower-Based Skills"].push_back("Talismongery");
  skcat["Magical Skills"].push_back("Talismongery");
  skcat["Medieval Skills"].push_back("Talismongery");
  skcat["Shadowrun Skills"].push_back("Talismongery");
  skcat["Skills"].push_back("Talismongery");

  // Skill Definition: Teamster
  defaults["Teamster"] = 3;
  skcat["Charisma-Based Skills"].push_back("Teamster");
  skcat["Piloting Skills"].push_back("Teamster");
  skcat["Medieval Skills"].push_back("Teamster");
  skcat["Cyberpunk Skills"].push_back("Teamster");
  skcat["Modern Skills"].push_back("Teamster");
  skcat["Shadowrun Skills"].push_back("Teamster");
  skcat["Skills"].push_back("Teamster");

  // Skill Definition: Throwing, Aero
  defaults["Throwing, Aero"] = 1;
  skcat["Quickness-Based Skills"].push_back("Throwing, Aero");
  skcat["Ranged-Combat Skills"].push_back("Throwing, Aero");
  skcat["Medieval Skills"].push_back("Throwing, Aero");
  skcat["Cyberpunk Skills"].push_back("Throwing, Aero");
  skcat["Modern Skills"].push_back("Throwing, Aero");
  skcat["Shadowrun Skills"].push_back("Throwing, Aero");
  skcat["Skills"].push_back("Throwing, Aero");
  add_wts("Throwing, Aero");

  // Skill Definition: Throwing, Non-Aero
  defaults["Throwing, Non-Aero"] = 1;
  skcat["Quickness-Based Skills"].push_back("Throwing, Non-Aero");
  skcat["Ranged-Combat Skills"].push_back("Throwing, Non-Aero");
  skcat["Medieval Skills"].push_back("Throwing, Non-Aero");
  skcat["Cyberpunk Skills"].push_back("Throwing, Non-Aero");
  skcat["Modern Skills"].push_back("Throwing, Non-Aero");
  skcat["Shadowrun Skills"].push_back("Throwing, Non-Aero");
  skcat["Skills"].push_back("Throwing, Non-Aero");
  add_wts("Throwing, Non-Aero");

  // Skill Definition: Tracked Drone
  defaults["Tracked Drone"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Tracked Drone");
  skcat["Drone Piloting Skills"].push_back("Tracked Drone");
  skcat["Shadowrun Skills"].push_back("Tracked Drone");
  skcat["Skills"].push_back("Tracked Drone");

  // Skill Definition: Tracked Vehicle
  defaults["Tracked Vehicle"] = 6;
  skcat["Reaction-Based Skills"].push_back("Tracked Vehicle");
  skcat["Piloting Skills"].push_back("Tracked Vehicle");
  skcat["Cyberpunk Skills"].push_back("Tracked Vehicle");
  skcat["Modern Skills"].push_back("Tracked Vehicle");
  skcat["Shadowrun Skills"].push_back("Tracked Vehicle");
  skcat["Skills"].push_back("Tracked Vehicle");

  // Skill Definition: Tracking
  defaults["Tracking"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Tracking");
  skcat["Specialized Skills"].push_back("Tracking");
  skcat["Medieval Skills"].push_back("Tracking");
  skcat["Cyberpunk Skills"].push_back("Tracking");
  skcat["Modern Skills"].push_back("Tracking");
  skcat["Shadowrun Skills"].push_back("Tracking");
  skcat["Skills"].push_back("Tracking");

  // Skill Definition: Tractor
  defaults["Tractor"] = 6;
  skcat["Reaction-Based Skills"].push_back("Tractor");
  skcat["Piloting Skills"].push_back("Tractor");
  skcat["Cyberpunk Skills"].push_back("Tractor");
  skcat["Modern Skills"].push_back("Tractor");
  skcat["Shadowrun Skills"].push_back("Tractor");
  skcat["Skills"].push_back("Tractor");

  // Skill Definition: Treatment
  defaults["Treatment"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Treatment");
  skcat["Technical Skills"].push_back("Treatment");
  skcat["Cyberpunk Skills"].push_back("Treatment");
  skcat["Modern Skills"].push_back("Treatment");
  skcat["Shadowrun Skills"].push_back("Treatment");
  skcat["Skills"].push_back("Treatment");

  // Skill Definition: Trike
  defaults["Trike"] = 6;
  skcat["Reaction-Based Skills"].push_back("Trike");
  skcat["Piloting Skills"].push_back("Trike");
  skcat["Cyberpunk Skills"].push_back("Trike");
  skcat["Modern Skills"].push_back("Trike");
  skcat["Shadowrun Skills"].push_back("Trike");
  skcat["Skills"].push_back("Trike");

  // Skill Definition: Two-Handed Blades
  defaults["Two-Handed Blades"] = 2;
  skcat["Strength-Based Skills"].push_back("Two-Handed Blades");
  skcat["Melee-Combat Skills"].push_back("Two-Handed Blades");
  skcat["Medieval Skills"].push_back("Two-Handed Blades");
  skcat["Cyberpunk Skills"].push_back("Two-Handed Blades");
  skcat["Modern Skills"].push_back("Two-Handed Blades");
  skcat["Shadowrun Skills"].push_back("Two-Handed Blades");
  skcat["Skills"].push_back("Two-Handed Blades");
  add_wts("Two-Handed Blades");

  // Skill Definition: Two-Handed Cleaves
  defaults["Two-Handed Cleaves"] = 2;
  skcat["Strength-Based Skills"].push_back("Two-Handed Cleaves");
  skcat["Melee-Combat Skills"].push_back("Two-Handed Cleaves");
  skcat["Medieval Skills"].push_back("Two-Handed Cleaves");
  skcat["Cyberpunk Skills"].push_back("Two-Handed Cleaves");
  skcat["Modern Skills"].push_back("Two-Handed Cleaves");
  skcat["Shadowrun Skills"].push_back("Two-Handed Cleaves");
  skcat["Skills"].push_back("Two-Handed Cleaves");
  add_wts("Two-Handed Cleaves");

  // Skill Definition: Two-Handed Crushing
  defaults["Two-Handed Crushing"] = 2;
  skcat["Strength-Based Skills"].push_back("Two-Handed Crushing");
  skcat["Melee-Combat Skills"].push_back("Two-Handed Crushing");
  skcat["Medieval Skills"].push_back("Two-Handed Crushing");
  skcat["Cyberpunk Skills"].push_back("Two-Handed Crushing");
  skcat["Modern Skills"].push_back("Two-Handed Crushing");
  skcat["Shadowrun Skills"].push_back("Two-Handed Crushing");
  skcat["Skills"].push_back("Two-Handed Crushing");
  add_wts("Two-Handed Crushing");

  // Skill Definition: Two-Handed Flails
  defaults["Two-Handed Flails"] = 2;
  skcat["Strength-Based Skills"].push_back("Two-Handed Flails");
  skcat["Melee-Combat Skills"].push_back("Two-Handed Flails");
  skcat["Medieval Skills"].push_back("Two-Handed Flails");
  skcat["Cyberpunk Skills"].push_back("Two-Handed Flails");
  skcat["Modern Skills"].push_back("Two-Handed Flails");
  skcat["Shadowrun Skills"].push_back("Two-Handed Flails");
  skcat["Skills"].push_back("Two-Handed Flails");
  add_wts("Two-Handed Flails");

  // Skill Definition: Two-Handed Piercing
  defaults["Two-Handed Piercing"] = 1;
  skcat["Quickness-Based Skills"].push_back("Two-Handed Piercing");
  skcat["Melee-Combat Skills"].push_back("Two-Handed Piercing");
  skcat["Medieval Skills"].push_back("Two-Handed Piercing");
  skcat["Cyberpunk Skills"].push_back("Two-Handed Piercing");
  skcat["Modern Skills"].push_back("Two-Handed Piercing");
  skcat["Shadowrun Skills"].push_back("Two-Handed Piercing");
  skcat["Skills"].push_back("Two-Handed Piercing");
  add_wts("Two-Handed Piercing");

  // Skill Definition: Two-Handed Staves
  defaults["Two-Handed Staves"] = 1;
  skcat["Quickness-Based Skills"].push_back("Two-Handed Staves");
  skcat["Melee-Combat Skills"].push_back("Two-Handed Staves");
  skcat["Medieval Skills"].push_back("Two-Handed Staves");
  skcat["Cyberpunk Skills"].push_back("Two-Handed Staves");
  skcat["Modern Skills"].push_back("Two-Handed Staves");
  skcat["Shadowrun Skills"].push_back("Two-Handed Staves");
  skcat["Skills"].push_back("Two-Handed Staves");
  add_wts("Two-Handed Staves");

  // Skill Definition: Underwater Combat
  defaults["Underwater Combat"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Underwater Combat");
  skcat["Combat Environment Skills"].push_back("Underwater Combat");
  skcat["Medieval Skills"].push_back("Underwater Combat");
  skcat["Cyberpunk Skills"].push_back("Underwater Combat");
  skcat["Modern Skills"].push_back("Underwater Combat");
  skcat["Shadowrun Skills"].push_back("Underwater Combat");
  skcat["Skills"].push_back("Underwater Combat");

  // Skill Definition: Vectored Thrust
  defaults["Vectored Thrust"] = 6;
  skcat["Reaction-Based Skills"].push_back("Vectored Thrust");
  skcat["Piloting Skills"].push_back("Vectored Thrust");
  skcat["Cyberpunk Skills"].push_back("Vectored Thrust");
  skcat["Modern Skills"].push_back("Vectored Thrust");
  skcat["Shadowrun Skills"].push_back("Vectored Thrust");
  skcat["Skills"].push_back("Vectored Thrust");

  // Skill Definition: Vectored Thrust Drone
  defaults["Vectored Thrust Drone"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Vectored Thrust Drone");
  skcat["Drone Piloting Skills"].push_back("Vectored Thrust Drone");
  skcat["Shadowrun Skills"].push_back("Vectored Thrust Drone");
  skcat["Skills"].push_back("Vectored Thrust Drone");

  // Skill Definition: Verbal Interface
  defaults["Verbal Interface"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Verbal Interface");
  skcat["Technical Skills"].push_back("Verbal Interface");
  skcat["Cyberpunk Skills"].push_back("Verbal Interface");
  skcat["Shadowrun Skills"].push_back("Verbal Interface");
  skcat["Sci-Fi Skills"].push_back("Verbal Interface");
  skcat["Skills"].push_back("Verbal Interface");

  // Skill Definition: Wand Targeting
  defaults["Wand Targeting"] = 1;
  skcat["Quickness-Based Skills"].push_back("Wand Targeting");
  skcat["Magical Skills"].push_back("Wand Targeting");
  skcat["Medieval Skills"].push_back("Wand Targeting");
  skcat["Skills"].push_back("Wand Targeting");

  // Skill Definition: Water Weaving
  defaults["Water Weaving"] = 1;
  skcat["Quickness-Based Skills"].push_back("Water Weaving");
  skcat["Magical Skills"].push_back("Water Weaving");
  skcat["Medieval Skills"].push_back("Water Weaving");
  skcat["FP Skills"].push_back("Water Weaving");
  skcat["Skills"].push_back("Water Weaving");

  // Skill Definition: Wheeled Drone
  defaults["Wheeled Drone"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Wheeled Drone");
  skcat["Drone Piloting Skills"].push_back("Wheeled Drone");
  skcat["Shadowrun Skills"].push_back("Wheeled Drone");
  skcat["Skills"].push_back("Wheeled Drone");

  // Skill Definition: Whips
  defaults["Whips"] = 1;
  skcat["Quickness-Based Skills"].push_back("Whips");
  skcat["Ranged-Combat Skills"].push_back("Whips");
  skcat["Medieval Skills"].push_back("Whips");
  skcat["Cyberpunk Skills"].push_back("Whips");
  skcat["Modern Skills"].push_back("Whips");
  skcat["Shadowrun Skills"].push_back("Whips");
  skcat["Skills"].push_back("Whips");
  add_wts("Whips");

  // Skill Definition: Woodworking
  defaults["Woodworking"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Woodworking");
  skcat["Creation Skills"].push_back("Woodworking");
  skcat["Medieval Skills"].push_back("Woodworking");
  skcat["Skills"].push_back("Woodworking");

  // Skill Definition: Zero-G Combat
  defaults["Zero-G Combat"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Zero-G Combat");
  skcat["Combat Environment Skills"].push_back("Zero-G Combat");
  skcat["Medieval Skills"].push_back("Zero-G Combat");
  skcat["Cyberpunk Skills"].push_back("Zero-G Combat");
  skcat["Modern Skills"].push_back("Zero-G Combat");
  skcat["Shadowrun Skills"].push_back("Zero-G Combat");
  skcat["Skills"].push_back("Zero-G Combat");

  // Skill Definition: Zero-G Ops
  defaults["Zero-G Ops"] = 4;
  skcat["Intelligence-Based Skills"].push_back("Zero-G Ops");
  skcat["Specialized Skills"].push_back("Zero-G Ops");
  skcat["Cyberpunk Skills"].push_back("Zero-G Ops");
  skcat["Modern Skills"].push_back("Zero-G Ops");
  skcat["Shadowrun Skills"].push_back("Zero-G Ops");
  skcat["Skills"].push_back("Zero-G Ops");
}

int is_skill(std::string sk) {
  if (!defaults_init)
    init_defaults();
  return (defaults.count(sk) != 0);
}

std::string get_weapon_skill(int wtype) {
  if (!defaults_init)
    init_defaults();
  if (!weaponskills.count(wtype)) {
    fprintf(stderr, "Warning: No Skill Type %d!\n", wtype);
    return "None";
  }
  return weaponskills[wtype];
}

int get_weapon_type(std::string wskill) {
  if (!defaults_init)
    init_defaults();
  if (!weapontypes.count(wskill)) {
    fprintf(stderr, "Warning: No Skill Named '%s'!\n", wskill.c_str());
    return 0;
  }
  return weapontypes[wskill];
}

std::string get_skill(std::string sk) {
  while (sk.length() > 0 && isspace(sk.back()))
    sk.pop_back();

  if (defaults.count(sk))
    return sk;
  if (sk.length() < 2)
    return "";

  for (auto itr : defaults) {
    std::string lc = itr.first;
    std::transform(lc.begin(), lc.end(), lc.begin(), ::tolower);
    if (!strncmp(sk.c_str(), lc.c_str(), sk.length())) {
      return itr.first;
    }
  }
  return "";
}

std::string get_skill_cat(std::string cat) {
  while (cat.length() > 0 && isspace(cat.back()))
    cat.pop_back();
  if (skcat.count(cat))
    return cat;
  if (cat.length() < 2)
    return "";

  for (auto itr : skcat) {
    if (!strncmp(cat.c_str(), itr.first.c_str(), cat.length())) {
      return itr.first;
    }
  }
  return "";
}

int get_linked(std::string sk) {
  while (sk.length() > 0 && isspace(sk.back()))
    sk.pop_back();
  if (defaults.count(sk))
    return defaults[sk];
  return 4; // Default to Int for knowledges
}

std::vector<std::string> get_skills(std::string cat) {
  std::vector<std::string> ret;

  if (!defaults_init)
    init_defaults();

  while (cat.length() > 0 && isspace(cat.back()))
    cat.pop_back();

  if (cat == "Categories") {
    for (auto ind : skcat) {
      ret.push_back(ind.first);
    }
  } else if (cat == "all") {
    for (auto ind : defaults) {
      ret.push_back(ind.first);
    }
  } else if (skcat.count(cat)) {
    for (auto ind : skcat[cat]) {
      ret.push_back(ind);
    }
  }

  return ret;
}

int roll(int ndice, int targ) {
  int ret = 0, ctr;
  for (ctr = 0; ctr < ndice; ++ctr) {
    int val = 1 + (rand() % 6);
    while ((val % 6) == 0)
      val += (1 + rand() % 6);
    if (val > 1 && val >= targ) {
      unsigned int numwrap = (unsigned int)((val - targ) / 12);
      ret += (1 + int(numwrap));
    }
  }
  return ret;
}

void Object::SetAttribute(int a, int v) {
  if (v > 100)
    v = 100;
  else if (v < 0)
    v = 0;
  att[a].cur = v;
}

void Object::SetModifier(int a, int v) {
  if (v > 10000)
    v = 10000;
  else if (v < -10000)
    v = -10000;
  att[a].mod = v;
}

void Object::SetSkill(const std::string& s, int v) {
  if (v > 1000000000)
    v = 1000000000;
  else if (v < -1000000000)
    v = -1000000000;
  if (v <= 0)
    skills.erase(s);
  else
    skills[s] = v;
}

int Object::HasSkill(const std::string& s) const {
  if (strlen(s.c_str()) == 0)
    return 0;
  if (skills.count(s))
    return 1;
  return 0;
}

int Object::Skill(const std::string& s, int* tnum) const {
  if (strlen(s.c_str()) == 0)
    return 0;
  if (!strncmp(s.c_str(), "Body", s.length()))
    return ModAttribute(0);
  if (!strncmp(s.c_str(), "Quickness", s.length()))
    return ModAttribute(1);
  if (!strncmp(s.c_str(), "Strength", s.length()))
    return ModAttribute(2);
  if (!strncmp(s.c_str(), "Charisma", s.length()))
    return ModAttribute(3);
  if (!strncmp(s.c_str(), "Intelligence", s.length()))
    return ModAttribute(4);
  if (!strncmp(s.c_str(), "Willpower", s.length()))
    return ModAttribute(5);
  if (!strncmp(s.c_str(), "Reaction", s.length()))
    return ModAttribute(6);
  if (!defaults_init)
    init_defaults();
  if (skills.count(s))
    return (skills.find(s))->second; // const for 'skills[s]'
  if (tnum) {
    (*tnum) += 4;
    return ModAttribute(defaults[s]);
  }
  return 0;
}

int Object::Roll(
    const std::string& s1,
    const Object* p2,
    const std::string& s2,
    int bias,
    std::string* res) const {
  int succ = 0;

  int t1 = p2->Skill(s2) - bias;
  int t2 = Skill(s1) + bias;

  if (res)
    (*res) += "(";
  succ = Roll(s1, t1, res);
  if (s2 != "") {
    if (res)
      (*res) += " - ";
    succ -= p2->Roll(s2, t2, res);
  }
  if (res)
    (*res) += ")";
  return succ;
}

int Object::Roll(const std::string& s1, int targ, std::string* res) const {
  if (phys >= 10 || stun >= 10 || att[0].cur == 0 || att[1].cur == 0 || att[2].cur == 0 ||
      att[3].cur == 0 || att[4].cur == 0 || att[5].cur == 0) {
    if (res)
      (*res) += "N/A";
    return 0;
  }
  targ += WoundPenalty();
  return RollNoWounds(s1, targ, res);
}

int Object::RollNoWounds(const std::string& s1, int targ, std::string* res) const {
  int succ = 0;
  int d1 = Skill(s1, &targ);
  succ = roll(abs(d1), targ);

  if (res) {
    char buffer[256];
    sprintf(buffer, "%d%c", abs(d1), 0);
    (*res) += buffer;
    (*res) += "(";
    sprintf(buffer, "%d%c", targ, 0);
    (*res) += buffer;
    (*res) += "):";
    sprintf(buffer, "%d%c", succ, 0);
    (*res) += buffer;
  }

  return succ;
}

int Object::WoundPenalty() const {
  int ret = 0;
  if (stun >= 10)
    ret += 4;
  else if (stun >= 6)
    ret += 3;
  else if (stun >= 3)
    ret += 2;
  else if (stun >= 1)
    ret += 1;
  if (phys >= 10)
    ret += 4;
  else if (phys >= 6)
    ret += 3;
  else if (phys >= 3)
    ret += 2;
  else if (phys >= 1)
    ret += 1;

  return ret;
}

int Object::RollInitiative() const {
  return Roll("Reaction", 6 - ModAttribute(5));
}
