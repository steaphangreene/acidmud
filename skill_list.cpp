#include <map>

#include "stats.hpp"
#include "utils.hpp"

std::map<uint32_t, int32_t> defaults = {
    {crc32c("Acrobatics"), 1},
    {crc32c("Acting"), 3},
    {crc32c("Activate Juju"), 7},
    {crc32c("Air Pistols"), 1},
    {crc32c("Air Rifles"), 1},
    {crc32c("Air Weaving"), 4},
    {crc32c("Alchemy"), 4},
    {crc32c("Amphibious Combat"), 4},
    {crc32c("Archery"), 1},
    {crc32c("Arctic Waraxe Martial Art"), 2},
    {crc32c("Armoring"), 4},
    {crc32c("Art"), 3},
    {crc32c("Artificing"), 4},
    {crc32c("Artificing, Star"), 4},
    {crc32c("Assault Cannons"), 2},
    {crc32c("Assault Rifles"), 1},
    {crc32c("Ballista"), 6},
    {crc32c("Bike"), 6},
    {crc32c("Blacksmithing"), 4},
    {crc32c("Blowgun"), 1},
    {crc32c("Boat, Powered"), 4},
    {crc32c("Boat, Row"), 2},
    {crc32c("Boat, Sail"), 6},
    {crc32c("Body Checking"), 0},
    {crc32c("Bowyer"), 4},
    {crc32c("Breath Weapon"), 1},
    {crc32c("Buggy"), 6},
    {crc32c("Cannon"), 4},
    {crc32c("Car"), 6},
    {crc32c("Carpentry"), 4},
    {crc32c("Carromeleg - Tier I"), 0},
    {crc32c("Carromeleg - Tier II"), 1},
    {crc32c("Carromeleg - Tier III"), 2},
    {crc32c("Carromeleg - Tier IV"), 3},
    {crc32c("Carromeleg - Tier V"), 4},
    {crc32c("Carromeleg - Tier VI"), 5},
    {crc32c("Cart"), 6},
    {crc32c("Catching"), 1},
    {crc32c("Centering"), 5},
    {crc32c("Climbing"), 2},
    {crc32c("Clothier"), 4},
    {crc32c("Communications"), 4},
    {crc32c("Computer"), 4},
    {crc32c("Computer Build/Repair"), 4},
    {crc32c("Computer Targeting"), 4},
    {crc32c("Conjuring"), 3},
    {crc32c("Crossbow"), 1},
    {crc32c("Cyber Data Manipulation"), 4},
    {crc32c("Cyber Vehicle Operation"), 4},
    {crc32c("Cycling"), 6},
    {crc32c("Damage Control"), 4},
    {crc32c("Damage Resistance"), 0},
    {crc32c("Demolitions"), 4},
    {crc32c("Diplomacy"), 3},
    {crc32c("Diving"), 1},
    {crc32c("Dodge"), 1},
    {crc32c("Earth Weaving"), 2},
    {crc32c("Electrical Build/Repair"), 4},
    {crc32c("Electronics"), 4},
    {crc32c("Enchanting"), 4},
    {crc32c("Endurance"), 2},
    {crc32c("Engineering"), 4},
    {crc32c("Etiquette"), 3},
    {crc32c("Evasion"), 1},
    {crc32c("Finance"), 4},
    {crc32c("Find Juju"), 5},
    {crc32c("Fire Weaving"), 3},
    {crc32c("First Aid"), 4},
    {crc32c("Fixed-Wing Craft"), 6},
    {crc32c("Fixed-Wing Drone"), 4},
    {crc32c("Fletcher"), 4},
    {crc32c("Flight"), 1},
    {crc32c("Focusing"), 2},
    {crc32c("Grappling"), 2},
    {crc32c("Gunnery"), 4},
    {crc32c("Half-Track"), 6},
    {crc32c("Healing"), 4},
    {crc32c("Heavy Lasers"), 2},
    {crc32c("Heavy Projectors"), 2},
    {crc32c("Heavy Rifles"), 2},
    {crc32c("Helmsman, Star"), 4},
    {crc32c("High-G Combat"), 4},
    {crc32c("High-G Ops"), 4},
    {crc32c("Hovercraft"), 6},
    {crc32c("Hover Drone"), 4},
    {crc32c("Hurling"), 2},
    {crc32c("Identify Juju"), 4},
    {crc32c("Interrogation"), 4},
    {crc32c("Intimidation"), 5},
    {crc32c("Jumping"), 2},
    {crc32c("Kicking"), 1},
    {crc32c("Knowledge"), 4},
    {crc32c("Laser Pistols"), 1},
    {crc32c("Laser Rifles"), 1},
    {crc32c("Lasso"), 1},
    {crc32c("Launch Weapons"), 4},
    {crc32c("Leadership"), 3},
    {crc32c("Leatherworking"), 4},
    {crc32c("Lifting"), 2},
    {crc32c("Long Blades"), 1},
    {crc32c("Long Cleaves"), 1},
    {crc32c("Long Crushing"), 1},
    {crc32c("Long Flails"), 1},
    {crc32c("Long Piercing"), 1},
    {crc32c("Long Staves"), 1},
    {crc32c("Low-G Combat"), 4},
    {crc32c("Low-G Ops"), 4},
    {crc32c("Lumberjack"), 2},
    {crc32c("Machine Guns"), 2},
    {crc32c("Machine Pistols"), 1},
    {crc32c("Magic Manipulation"), 3},
    {crc32c("Martial Arts, Elven"), 5},
    {crc32c("Masonry"), 4},
    {crc32c("Mechanical Build/Repair"), 4},
    {crc32c("Mechanics"), 4},
    {crc32c("Metalworking"), 4},
    {crc32c("Mindcasting"), 5},
    {crc32c("Mounted Air Pistols"), 1},
    {crc32c("Mounted Archery"), 1},
    {crc32c("Mounted Blades"), 1},
    {crc32c("Mounted Blowgun"), 1},
    {crc32c("Mounted Cleaves"), 1},
    {crc32c("Mounted Crossbow"), 1},
    {crc32c("Mounted Crushing"), 1},
    {crc32c("Mounted Flails"), 1},
    {crc32c("Mounted Hurling"), 1},
    {crc32c("Mounted Laser Pistols"), 1},
    {crc32c("Mounted Machine Pistols"), 1},
    {crc32c("Mounted Nets"), 1},
    {crc32c("Mounted Piercing"), 1},
    {crc32c("Mounted Pistols"), 1},
    {crc32c("Mounted Plasma Pistols"), 1},
    {crc32c("Mounted Shot Pistols"), 1},
    {crc32c("Mounted Slings"), 1},
    {crc32c("Mounted SMGs"), 1},
    {crc32c("Mounted Throwing, Aero"), 1},
    {crc32c("Mounted Throwing, Non-Aero"), 1},
    {crc32c("Mounted Whips"), 1},
    {crc32c("Navigation"), 4},
    {crc32c("Negotiation"), 3},
    {crc32c("Nets"), 1},
    {crc32c("Neural Interface"), 5},
    {crc32c("Offhand Air Pistols"), 1},
    {crc32c("Offhand Blades"), 1},
    {crc32c("Offhand Cleaves"), 1},
    {crc32c("Offhand Crossbow"), 1},
    {crc32c("Offhand Crushing"), 1},
    {crc32c("Offhand Flails"), 1},
    {crc32c("Offhand Hurling"), 1},
    {crc32c("Offhand Laser Pistols"), 1},
    {crc32c("Offhand Machine Pistols"), 1},
    {crc32c("Offhand Piercing"), 1},
    {crc32c("Offhand Pistols"), 1},
    {crc32c("Offhand Plasma Pistols"), 1},
    {crc32c("Offhand Punching"), 1},
    {crc32c("Offhand Shot Pistols"), 1},
    {crc32c("Offhand SMGs"), 1},
    {crc32c("Offhand Staves"), 1},
    {crc32c("Offhand Throwing, Aero"), 1},
    {crc32c("Offhand Throwing, Non-Aero"), 1},
    {crc32c("Offhand Whips"), 1},
    {crc32c("Perception"), 4},
    {crc32c("Performance"), 3},
    {crc32c("Pistols"), 1},
    {crc32c("Plasma Cannons"), 2},
    {crc32c("Plasma Pistols"), 1},
    {crc32c("Plasma Rifles"), 1},
    {crc32c("Power Distribution"), 4},
    {crc32c("Power Suit"), 6},
    {crc32c("Prepared Simple"), 5},
    {crc32c("Punching"), 1},
    {crc32c("Quickdraw"), 1},
    {crc32c("Race Car"), 6},
    {crc32c("Refine Juju"), 3},
    {crc32c("Research"), 4},
    {crc32c("Riding"), 1},
    {crc32c("Rifles"), 1},
    {crc32c("Ritual Conjuring"), 5},
    {crc32c("Ritual Spellcasting"), 5},
    {crc32c("Rod Targeting"), 1},
    {crc32c("Rotorcraft"), 6},
    {crc32c("Rotor Drone"), 4},
    {crc32c("Running"), 2},
    {crc32c("Saurian Line Attacking"), 2},
    {crc32c("Security"), 4},
    {crc32c("Seduction"), 3},
    {crc32c("Self Artificing"), 3},
    {crc32c("Sensors"), 4},
    {crc32c("Shields"), 2},
    {crc32c("Ship"), 1},
    {crc32c("Ship, Powered"), 4},
    {crc32c("Shipwright"), 4},
    {crc32c("Short Blades"), 1},
    {crc32c("Short Cleaves"), 1},
    {crc32c("Short Crushing"), 1},
    {crc32c("Short Flails"), 1},
    {crc32c("Short Piercing"), 1},
    {crc32c("Short Staves"), 1},
    {crc32c("Shotguns"), 1},
    {crc32c("Shot Pistols"), 1},
    {crc32c("Skiing"), 1},
    {crc32c("Sled"), 1},
    {crc32c("Slings"), 1},
    {crc32c("Smelting"), 4},
    {crc32c("SMGs"), 1},
    {crc32c("Snowmobile"), 6},
    {crc32c("Spacecraft"), 4},
    {crc32c("Spellcasting"), 3},
    {crc32c("Spellcraft"), 4},
    {crc32c("Spellcraft, Star"), 4},
    {crc32c("Spell Preparation"), 4},
    {crc32c("Spell Targeting"), 1},
    {crc32c("Sprinting"), 1},
    {crc32c("Staff Slings"), 1},
    {crc32c("Staff Targeting"), 1},
    {crc32c("Stealth"), 1},
    {crc32c("Style"), 3},
    {crc32c("Surgery"), 1},
    {crc32c("Survival"), 4},
    {crc32c("Swimming"), 1},
    {crc32c("Swordsmithing"), 4},
    {crc32c("Tactics"), 4},
    {crc32c("Talismongery"), 5},
    {crc32c("Teamster"), 3},
    {crc32c("Throwing, Aero"), 1},
    {crc32c("Throwing, Non-Aero"), 1},
    {crc32c("Tracked Drone"), 4},
    {crc32c("Tracked Vehicle"), 6},
    {crc32c("Tracking"), 4},
    {crc32c("Tractor"), 6},
    {crc32c("Treatment"), 4},
    {crc32c("Trike"), 6},
    {crc32c("Two-Handed Blades"), 2},
    {crc32c("Two-Handed Cleaves"), 2},
    {crc32c("Two-Handed Crushing"), 2},
    {crc32c("Two-Handed Flails"), 2},
    {crc32c("Two-Handed Piercing"), 1},
    {crc32c("Two-Handed Staves"), 1},
    {crc32c("Underwater Combat"), 4},
    {crc32c("Vectored Thrust"), 6},
    {crc32c("Vectored Thrust Drone"), 4},
    {crc32c("Verbal Interface"), 4},
    {crc32c("Wand Targeting"), 1},
    {crc32c("Water Weaving"), 1},
    {crc32c("Wheeled Drone"), 4},
    {crc32c("Whips"), 1},
    {crc32c("Woodworking"), 4},
    {crc32c("Zero-G Combat"), 4},
    {crc32c("Zero-G Ops"), 4},
};

std::map<int32_t, uint32_t> weaponskills;
std::map<uint32_t, int32_t> weapontypes;
std::map<std::string, std::vector<uint32_t>> skcat;

static int last_wtype = 0;
static void add_wts(uint32_t sk) {
  if (defaults.count(sk) == 0) {
    fprintf(
        stderr,
        "Warning: Tried to link weapon type %d to '%s' which isn't a skill.\n",
        last_wtype + 1,
        SkillName(sk).c_str());
    return;
  }
  ++last_wtype;
  weaponskills[last_wtype] = sk;
  weapontypes[sk] = last_wtype;
}

void init_skill_list() {
  // Skill Definition: Acrobatics
  skcat["Quickness-Based Skills"].push_back(crc32c("Acrobatics"));
  skcat["Athletic Skills"].push_back(crc32c("Acrobatics"));
  skcat["Medieval Skills"].push_back(crc32c("Acrobatics"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Acrobatics"));
  skcat["Modern Skills"].push_back(crc32c("Acrobatics"));
  skcat["Shadowrun Skills"].push_back(crc32c("Acrobatics"));
  skcat["Skills"].push_back(crc32c("Acrobatics"));

  // Skill Definition: Acting
  skcat["Charisma-Based Skills"].push_back(crc32c("Acting"));
  skcat["Social Skills"].push_back(crc32c("Acting"));
  skcat["Medieval Skills"].push_back(crc32c("Acting"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Acting"));
  skcat["Modern Skills"].push_back(crc32c("Acting"));
  skcat["Shadowrun Skills"].push_back(crc32c("Acting"));
  skcat["Skills"].push_back(crc32c("Acting"));

  // Skill Definition: Activate Juju
  skcat["Skill-Based Skills"].push_back(crc32c("Activate Juju"));
  skcat["Shamanistic Skills"].push_back(crc32c("Activate Juju"));
  skcat["Medieval Skills"].push_back(crc32c("Activate Juju"));
  skcat["Skills"].push_back(crc32c("Activate Juju"));

  // Skill Definition: Air Pistols
  skcat["Quickness-Based Skills"].push_back(crc32c("Air Pistols"));
  skcat["Pistol Skills"].push_back(crc32c("Air Pistols"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Air Pistols"));
  skcat["Modern Skills"].push_back(crc32c("Air Pistols"));
  skcat["Shadowrun Skills"].push_back(crc32c("Air Pistols"));
  skcat["Skills"].push_back(crc32c("Air Pistols"));
  add_wts(crc32c("Air Pistols"));

  // Skill Definition: Air Rifles
  skcat["Quickness-Based Skills"].push_back(crc32c("Air Rifles"));
  skcat["Rifle Skills"].push_back(crc32c("Air Rifles"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Air Rifles"));
  skcat["Modern Skills"].push_back(crc32c("Air Rifles"));
  skcat["Shadowrun Skills"].push_back(crc32c("Air Rifles"));
  skcat["Skills"].push_back(crc32c("Air Rifles"));
  add_wts(crc32c("Air Rifles"));

  // Skill Definition: Air Weaving
  skcat["Intelligence-Based Skills"].push_back(crc32c("Air Weaving"));
  skcat["Magical Skills"].push_back(crc32c("Air Weaving"));
  skcat["Medieval Skills"].push_back(crc32c("Air Weaving"));
  skcat["FP Skills"].push_back(crc32c("Air Weaving"));
  skcat["Skills"].push_back(crc32c("Air Weaving"));

  // Skill Definition: Alchemy
  skcat["Intelligence-Based Skills"].push_back(crc32c("Alchemy"));
  skcat["Magical Skills"].push_back(crc32c("Alchemy"));
  skcat["Medieval Skills"].push_back(crc32c("Alchemy"));
  skcat["Shadowrun Skills"].push_back(crc32c("Alchemy"));
  skcat["Skills"].push_back(crc32c("Alchemy"));

  // Skill Definition: Amphibious Combat
  skcat["Intelligence-Based Skills"].push_back(crc32c("Amphibious Combat"));
  skcat["Combat Environment Skills"].push_back(crc32c("Amphibious Combat"));
  skcat["Medieval Skills"].push_back(crc32c("Amphibious Combat"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Amphibious Combat"));
  skcat["Modern Skills"].push_back(crc32c("Amphibious Combat"));
  skcat["Shadowrun Skills"].push_back(crc32c("Amphibious Combat"));
  skcat["Skills"].push_back(crc32c("Amphibious Combat"));

  // Skill Definition: Archery
  skcat["Quickness-Based Skills"].push_back(crc32c("Archery"));
  skcat["Ranged-Combat Skills"].push_back(crc32c("Archery"));
  skcat["Medieval Skills"].push_back(crc32c("Archery"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Archery"));
  skcat["Modern Skills"].push_back(crc32c("Archery"));
  skcat["Shadowrun Skills"].push_back(crc32c("Archery"));
  skcat["Skills"].push_back(crc32c("Archery"));
  add_wts(crc32c("Archery"));

  // Skill Definition: Arctic Waraxe Martial Art
  skcat["Strength-Based Skills"].push_back(crc32c("Arctic Waraxe Martial Art"));
  skcat["Lost Martial Arts"].push_back(crc32c("Arctic Waraxe Martial Art"));
  skcat["Medieval Skills"].push_back(crc32c("Arctic Waraxe Martial Art"));
  skcat["Skills"].push_back(crc32c("Arctic Waraxe Martial Art"));

  // Skill Definition: Armoring
  skcat["Intelligence-Based Skills"].push_back(crc32c("Armoring"));
  skcat["Creation Skills"].push_back(crc32c("Armoring"));
  skcat["Medieval Skills"].push_back(crc32c("Armoring"));
  skcat["Skills"].push_back(crc32c("Armoring"));

  // Skill Definition: Art
  skcat["Charisma-Based Skills"].push_back(crc32c("Art"));
  skcat["Social Skills"].push_back(crc32c("Art"));
  skcat["Medieval Skills"].push_back(crc32c("Art"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Art"));
  skcat["Modern Skills"].push_back(crc32c("Art"));
  skcat["Shadowrun Skills"].push_back(crc32c("Art"));
  skcat["Skills"].push_back(crc32c("Art"));

  // Skill Definition: Artificing
  skcat["Intelligence-Based Skills"].push_back(crc32c("Artificing"));
  skcat["Magical Skills"].push_back(crc32c("Artificing"));
  skcat["Medieval Skills"].push_back(crc32c("Artificing"));
  skcat["Skills"].push_back(crc32c("Artificing"));

  // Skill Definition: Artificing, Star
  skcat["Intelligence-Based Skills"].push_back(crc32c("Artificing, Star"));
  skcat["Star Magical Skills"].push_back(crc32c("Artificing, Star"));
  skcat["Medieval Skills"].push_back(crc32c("Artificing, Star"));
  skcat["Skills"].push_back(crc32c("Artificing, Star"));

  // Skill Definition: Assault Cannons
  skcat["Strength-Based Skills"].push_back(crc32c("Assault Cannons"));
  skcat["Heavy Firearm Skills"].push_back(crc32c("Assault Cannons"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Assault Cannons"));
  skcat["Modern Skills"].push_back(crc32c("Assault Cannons"));
  skcat["Shadowrun Skills"].push_back(crc32c("Assault Cannons"));
  skcat["Skills"].push_back(crc32c("Assault Cannons"));

  // Skill Definition: Assault Rifles
  skcat["Quickness-Based Skills"].push_back(crc32c("Assault Rifles"));
  skcat["Rifle Skills"].push_back(crc32c("Assault Rifles"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Assault Rifles"));
  skcat["Modern Skills"].push_back(crc32c("Assault Rifles"));
  skcat["Shadowrun Skills"].push_back(crc32c("Assault Rifles"));
  skcat["Skills"].push_back(crc32c("Assault Rifles"));
  add_wts(crc32c("Assault Rifles"));

  // Skill Definition: Ballista
  skcat["Reaction-Based Skills"].push_back(crc32c("Ballista"));
  skcat["Medieval Skills"].push_back(crc32c("Ballista"));
  skcat["Skills"].push_back(crc32c("Ballista"));

  // Skill Definition: Bike
  skcat["Reaction-Based Skills"].push_back(crc32c("Bike"));
  skcat["Piloting Skills"].push_back(crc32c("Bike"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Bike"));
  skcat["Modern Skills"].push_back(crc32c("Bike"));
  skcat["Shadowrun Skills"].push_back(crc32c("Bike"));
  skcat["Skills"].push_back(crc32c("Bike"));

  // Skill Definition: Blacksmithing
  skcat["Intelligence-Based Skills"].push_back(crc32c("Blacksmithing"));
  skcat["Creation Skills"].push_back(crc32c("Blacksmithing"));
  skcat["Medieval Skills"].push_back(crc32c("Blacksmithing"));
  skcat["Skills"].push_back(crc32c("Blacksmithing"));

  // Skill Definition: Blowgun
  skcat["Quickness-Based Skills"].push_back(crc32c("Blowgun"));
  skcat["Ranged-Combat Skills"].push_back(crc32c("Blowgun"));
  skcat["Medieval Skills"].push_back(crc32c("Blowgun"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Blowgun"));
  skcat["Modern Skills"].push_back(crc32c("Blowgun"));
  skcat["Shadowrun Skills"].push_back(crc32c("Blowgun"));
  skcat["Skills"].push_back(crc32c("Blowgun"));
  add_wts(crc32c("Blowgun"));

  // Skill Definition: Boat, Powered
  skcat["Intelligence-Based Skills"].push_back(crc32c("Boat, Powered"));
  skcat["Piloting Skills"].push_back(crc32c("Boat, Powered"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Boat, Powered"));
  skcat["Modern Skills"].push_back(crc32c("Boat, Powered"));
  skcat["Shadowrun Skills"].push_back(crc32c("Boat, Powered"));
  skcat["Skills"].push_back(crc32c("Boat, Powered"));

  // Skill Definition: Boat, Row
  skcat["Strength-Based Skills"].push_back(crc32c("Boat, Row"));
  skcat["Piloting Skills"].push_back(crc32c("Boat, Row"));
  skcat["Medieval Skills"].push_back(crc32c("Boat, Row"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Boat, Row"));
  skcat["Modern Skills"].push_back(crc32c("Boat, Row"));
  skcat["Shadowrun Skills"].push_back(crc32c("Boat, Row"));
  skcat["Skills"].push_back(crc32c("Boat, Row"));

  // Skill Definition: Boat, Sail
  skcat["Reaction-Based Skills"].push_back(crc32c("Boat, Sail"));
  skcat["Piloting Skills"].push_back(crc32c("Boat, Sail"));
  skcat["Medieval Skills"].push_back(crc32c("Boat, Sail"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Boat, Sail"));
  skcat["Modern Skills"].push_back(crc32c("Boat, Sail"));
  skcat["Shadowrun Skills"].push_back(crc32c("Boat, Sail"));
  skcat["Skills"].push_back(crc32c("Boat, Sail"));

  // Skill Definition: Body Checking
  skcat["Body-Based Skills"].push_back(crc32c("Body Checking"));
  skcat["Hand-to-Hand Combat Skills"].push_back(crc32c("Body Checking"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Body Checking"));
  skcat["Modern Skills"].push_back(crc32c("Body Checking"));
  skcat["Shadowrun Skills"].push_back(crc32c("Body Checking"));
  skcat["Medieval Skills"].push_back(crc32c("Body Checking"));
  skcat["Skills"].push_back(crc32c("Body Checking"));
  add_wts(crc32c("Body Checking"));

  // Skill Definition: Bowyer
  skcat["Intelligence-Based Skills"].push_back(crc32c("Bowyer"));
  skcat["Creation Skills"].push_back(crc32c("Bowyer"));
  skcat["Medieval Skills"].push_back(crc32c("Bowyer"));
  skcat["Skills"].push_back(crc32c("Bowyer"));

  // Skill Definition: Breath Weapon
  skcat["Quickness-Based Skills"].push_back(crc32c("Breath Weapon"));
  skcat["Medieval Skills"].push_back(crc32c("Breath Weapon"));
  skcat["Shadowrun Skills"].push_back(crc32c("Breath Weapon"));
  skcat["Skills"].push_back(crc32c("Breath Weapon"));

  // Skill Definition: Buggy
  skcat["Reaction-Based Skills"].push_back(crc32c("Buggy"));
  skcat["Piloting Skills"].push_back(crc32c("Buggy"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Buggy"));
  skcat["Modern Skills"].push_back(crc32c("Buggy"));
  skcat["Shadowrun Skills"].push_back(crc32c("Buggy"));
  skcat["Skills"].push_back(crc32c("Buggy"));

  // Skill Definition: Cannon
  skcat["Intelligence-Based Skills"].push_back(crc32c("Cannon"));
  skcat["Heavy Firearm Skills"].push_back(crc32c("Cannon"));
  skcat["Medieval Skills"].push_back(crc32c("Cannon"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Cannon"));
  skcat["Modern Skills"].push_back(crc32c("Cannon"));
  skcat["Shadowrun Skills"].push_back(crc32c("Cannon"));
  skcat["Skills"].push_back(crc32c("Cannon"));

  // Skill Definition: Car
  skcat["Reaction-Based Skills"].push_back(crc32c("Car"));
  skcat["Piloting Skills"].push_back(crc32c("Car"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Car"));
  skcat["Modern Skills"].push_back(crc32c("Car"));
  skcat["Shadowrun Skills"].push_back(crc32c("Car"));
  skcat["Skills"].push_back(crc32c("Car"));

  // Skill Definition: Carpentry
  skcat["Intelligence-Based Skills"].push_back(crc32c("Carpentry"));
  skcat["Creation Skills"].push_back(crc32c("Carpentry"));
  skcat["Medieval Skills"].push_back(crc32c("Carpentry"));
  skcat["Modern Skills"].push_back(crc32c("Carpentry"));
  skcat["Skills"].push_back(crc32c("Carpentry"));

  // Skill Definition: Carromeleg - Tier I
  skcat["Body-Based Skills"].push_back(crc32c("Carromeleg - Tier I"));
  skcat["Elven Martial Arts Skills"].push_back(crc32c("Carromeleg - Tier I"));
  skcat["Medieval Skills"].push_back(crc32c("Carromeleg - Tier I"));
  skcat["Shadowrun Skills"].push_back(crc32c("Carromeleg - Tier I"));
  skcat["Skills"].push_back(crc32c("Carromeleg - Tier I"));

  // Skill Definition: Carromeleg - Tier II
  skcat["Quickness-Based Skills"].push_back(crc32c("Carromeleg - Tier II"));
  skcat["Elven Martial Arts Skills"].push_back(crc32c("Carromeleg - Tier II"));
  skcat["Medieval Skills"].push_back(crc32c("Carromeleg - Tier II"));
  skcat["Shadowrun Skills"].push_back(crc32c("Carromeleg - Tier II"));
  skcat["Skills"].push_back(crc32c("Carromeleg - Tier II"));

  // Skill Definition: Carromeleg - Tier III
  skcat["Strength-Based Skills"].push_back(crc32c("Carromeleg - Tier III"));
  skcat["Elven Martial Arts Skills"].push_back(crc32c("Carromeleg - Tier III"));
  skcat["Medieval Skills"].push_back(crc32c("Carromeleg - Tier III"));
  skcat["Shadowrun Skills"].push_back(crc32c("Carromeleg - Tier III"));
  skcat["Skills"].push_back(crc32c("Carromeleg - Tier III"));

  // Skill Definition: Carromeleg - Tier IV
  skcat["Charisma-Based Skills"].push_back(crc32c("Carromeleg - Tier IV"));
  skcat["Elven Martial Arts Skills"].push_back(crc32c("Carromeleg - Tier IV"));
  skcat["Medieval Skills"].push_back(crc32c("Carromeleg - Tier IV"));
  skcat["Shadowrun Skills"].push_back(crc32c("Carromeleg - Tier IV"));
  skcat["Skills"].push_back(crc32c("Carromeleg - Tier IV"));

  // Skill Definition: Carromeleg - Tier V
  skcat["Intelligence-Based Skills"].push_back(crc32c("Carromeleg - Tier V"));
  skcat["Elven Martial Arts Skills"].push_back(crc32c("Carromeleg - Tier V"));
  skcat["Medieval Skills"].push_back(crc32c("Carromeleg - Tier V"));
  skcat["Shadowrun Skills"].push_back(crc32c("Carromeleg - Tier V"));
  skcat["Skills"].push_back(crc32c("Carromeleg - Tier V"));

  // Skill Definition: Carromeleg - Tier VI
  skcat["Willpower-Based Skills"].push_back(crc32c("Carromeleg - Tier VI"));
  skcat["Elven Martial Arts Skills"].push_back(crc32c("Carromeleg - Tier VI"));
  skcat["Medieval Skills"].push_back(crc32c("Carromeleg - Tier VI"));
  skcat["Shadowrun Skills"].push_back(crc32c("Carromeleg - Tier VI"));
  skcat["Skills"].push_back(crc32c("Carromeleg - Tier VI"));

  // Skill Definition: Cart
  skcat["Reaction-Based Skills"].push_back(crc32c("Cart"));
  skcat["Piloting Skills"].push_back(crc32c("Cart"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Cart"));
  skcat["Modern Skills"].push_back(crc32c("Cart"));
  skcat["Shadowrun Skills"].push_back(crc32c("Cart"));
  skcat["Skills"].push_back(crc32c("Cart"));

  // Skill Definition: Catching
  skcat["Quickness-Based Skills"].push_back(crc32c("Catching"));
  skcat["Athletic Skills"].push_back(crc32c("Catching"));
  skcat["Medieval Skills"].push_back(crc32c("Catching"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Catching"));
  skcat["Modern Skills"].push_back(crc32c("Catching"));
  skcat["Shadowrun Skills"].push_back(crc32c("Catching"));
  skcat["Skills"].push_back(crc32c("Catching"));

  // Skill Definition: Centering
  skcat["Willpower-Based Skills"].push_back(crc32c("Centering"));
  skcat["Star Magical Skills"].push_back(crc32c("Centering"));
  skcat["Medieval Skills"].push_back(crc32c("Centering"));
  skcat["Skills"].push_back(crc32c("Centering"));

  // Skill Definition: Climbing
  skcat["Strength-Based Skills"].push_back(crc32c("Climbing"));
  skcat["Athletic Skills"].push_back(crc32c("Climbing"));
  skcat["Medieval Skills"].push_back(crc32c("Climbing"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Climbing"));
  skcat["Modern Skills"].push_back(crc32c("Climbing"));
  skcat["Shadowrun Skills"].push_back(crc32c("Climbing"));
  skcat["Skills"].push_back(crc32c("Climbing"));

  // Skill Definition: Clothier
  skcat["Intelligence-Based Skills"].push_back(crc32c("Clothier"));
  skcat["Creation Skills"].push_back(crc32c("Clothier"));
  skcat["Medieval Skills"].push_back(crc32c("Clothier"));
  skcat["Skills"].push_back(crc32c("Clothier"));

  // Skill Definition: Communications
  skcat["Intelligence-Based Skills"].push_back(crc32c("Communications"));
  skcat["Technical Skills"].push_back(crc32c("Communications"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Communications"));
  skcat["Modern Skills"].push_back(crc32c("Communications"));
  skcat["Shadowrun Skills"].push_back(crc32c("Communications"));
  skcat["Skills"].push_back(crc32c("Communications"));

  // Skill Definition: Computer
  skcat["Intelligence-Based Skills"].push_back(crc32c("Computer"));
  skcat["Technical Skills"].push_back(crc32c("Computer"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Computer"));
  skcat["Modern Skills"].push_back(crc32c("Computer"));
  skcat["Shadowrun Skills"].push_back(crc32c("Computer"));
  skcat["Skills"].push_back(crc32c("Computer"));

  // Skill Definition: Computer Build/Repair
  skcat["Intelligence-Based Skills"].push_back(crc32c("Computer Build/Repair"));
  skcat["Build/Repair Skills"].push_back(crc32c("Computer Build/Repair"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Computer Build/Repair"));
  skcat["Modern Skills"].push_back(crc32c("Computer Build/Repair"));
  skcat["Shadowrun Skills"].push_back(crc32c("Computer Build/Repair"));
  skcat["Skills"].push_back(crc32c("Computer Build/Repair"));

  // Skill Definition: Computer Targeting
  skcat["Intelligence-Based Skills"].push_back(crc32c("Computer Targeting"));
  skcat["Weapon System Skills"].push_back(crc32c("Computer Targeting"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Computer Targeting"));
  skcat["Modern Skills"].push_back(crc32c("Computer Targeting"));
  skcat["Shadowrun Skills"].push_back(crc32c("Computer Targeting"));
  skcat["Skills"].push_back(crc32c("Computer Targeting"));

  // Skill Definition: Conjuring
  skcat["Charisma-Based Skills"].push_back(crc32c("Conjuring"));
  skcat["Magical Skills"].push_back(crc32c("Conjuring"));
  skcat["Medieval Skills"].push_back(crc32c("Conjuring"));
  skcat["Shadowrun Skills"].push_back(crc32c("Conjuring"));
  skcat["Skills"].push_back(crc32c("Conjuring"));

  // Skill Definition: Crossbow
  skcat["Quickness-Based Skills"].push_back(crc32c("Crossbow"));
  skcat["Ranged-Combat Skills"].push_back(crc32c("Crossbow"));
  skcat["Medieval Skills"].push_back(crc32c("Crossbow"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Crossbow"));
  skcat["Modern Skills"].push_back(crc32c("Crossbow"));
  skcat["Shadowrun Skills"].push_back(crc32c("Crossbow"));
  skcat["Skills"].push_back(crc32c("Crossbow"));
  add_wts(crc32c("Crossbow"));

  // Skill Definition: Cyber Data Manipulation
  skcat["Intelligence-Based Skills"].push_back(crc32c("Cyber Data Manipulation"));
  skcat["Cyber Skills"].push_back(crc32c("Cyber Data Manipulation"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Cyber Data Manipulation"));
  skcat["Shadowrun Skills"].push_back(crc32c("Cyber Data Manipulation"));
  skcat["Specialty Skills"].push_back(crc32c("Cyber Data Manipulation"));
  skcat["Skills"].push_back(crc32c("Cyber Data Manipulation"));

  // Skill Definition: Cyber Vehicle Operation
  skcat["Intelligence-Based Skills"].push_back(crc32c("Cyber Vehicle Operation"));
  skcat["Cyber Skills"].push_back(crc32c("Cyber Vehicle Operation"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Cyber Vehicle Operation"));
  skcat["Shadowrun Skills"].push_back(crc32c("Cyber Vehicle Operation"));
  skcat["Specialty Skills"].push_back(crc32c("Cyber Vehicle Operation"));
  skcat["Skills"].push_back(crc32c("Cyber Vehicle Operation"));

  // Skill Definition: Cycling
  skcat["Reaction-Based Skills"].push_back(crc32c("Cycling"));
  skcat["Piloting Skills"].push_back(crc32c("Cycling"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Cycling"));
  skcat["Modern Skills"].push_back(crc32c("Cycling"));
  skcat["Shadowrun Skills"].push_back(crc32c("Cycling"));
  skcat["Skills"].push_back(crc32c("Cycling"));

  // Skill Definition: Damage Control
  skcat["Intelligence-Based Skills"].push_back(crc32c("Damage Control"));
  skcat["Technical Skills"].push_back(crc32c("Damage Control"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Damage Control"));
  skcat["Modern Skills"].push_back(crc32c("Damage Control"));
  skcat["Shadowrun Skills"].push_back(crc32c("Damage Control"));
  skcat["Skills"].push_back(crc32c("Damage Control"));

  // Skill Definition: Damage Resistance
  skcat["Body-Based Skills"].push_back(crc32c("Damage Resistance"));
  skcat["Medieval Skills"].push_back(crc32c("Damage Resistance"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Damage Resistance"));
  skcat["Modern Skills"].push_back(crc32c("Damage Resistance"));
  skcat["Shadowrun Skills"].push_back(crc32c("Damage Resistance"));
  skcat["Skills"].push_back(crc32c("Damage Resistance"));

  // Skill Definition: Demolitions
  skcat["Intelligence-Based Skills"].push_back(crc32c("Demolitions"));
  skcat["Technical Skills"].push_back(crc32c("Demolitions"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Demolitions"));
  skcat["Modern Skills"].push_back(crc32c("Demolitions"));
  skcat["Shadowrun Skills"].push_back(crc32c("Demolitions"));
  skcat["Skills"].push_back(crc32c("Demolitions"));

  // Skill Definition: Diplomacy
  skcat["Charisma-Based Skills"].push_back(crc32c("Diplomacy"));
  skcat["Social Skills"].push_back(crc32c("Diplomacy"));
  skcat["Medieval Skills"].push_back(crc32c("Diplomacy"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Diplomacy"));
  skcat["Modern Skills"].push_back(crc32c("Diplomacy"));
  skcat["Shadowrun Skills"].push_back(crc32c("Diplomacy"));
  skcat["Skills"].push_back(crc32c("Diplomacy"));

  // Skill Definition: Diving
  skcat["Quickness-Based Skills"].push_back(crc32c("Diving"));
  skcat["Athletic Skills"].push_back(crc32c("Diving"));
  skcat["Medieval Skills"].push_back(crc32c("Diving"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Diving"));
  skcat["Modern Skills"].push_back(crc32c("Diving"));
  skcat["Shadowrun Skills"].push_back(crc32c("Diving"));
  skcat["Skills"].push_back(crc32c("Diving"));

  // Skill Definition: Dodge
  skcat["Quickness-Based Skills"].push_back(crc32c("Dodge"));
  skcat["Athletic Skills"].push_back(crc32c("Dodge"));
  skcat["Medieval Skills"].push_back(crc32c("Dodge"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Dodge"));
  skcat["Modern Skills"].push_back(crc32c("Dodge"));
  skcat["Shadowrun Skills"].push_back(crc32c("Dodge"));
  skcat["Skills"].push_back(crc32c("Dodge"));

  // Skill Definition: Earth Weaving
  skcat["Strength-Based Skills"].push_back(crc32c("Earth Weaving"));
  skcat["Magical Skills"].push_back(crc32c("Earth Weaving"));
  skcat["Medieval Skills"].push_back(crc32c("Earth Weaving"));
  skcat["FP Skills"].push_back(crc32c("Earth Weaving"));
  skcat["Skills"].push_back(crc32c("Earth Weaving"));

  // Skill Definition: Electrical Build/Repair
  skcat["Intelligence-Based Skills"].push_back(crc32c("Electrical Build/Repair"));
  skcat["Build/Repair Skills"].push_back(crc32c("Electrical Build/Repair"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Electrical Build/Repair"));
  skcat["Modern Skills"].push_back(crc32c("Electrical Build/Repair"));
  skcat["Shadowrun Skills"].push_back(crc32c("Electrical Build/Repair"));
  skcat["Skills"].push_back(crc32c("Electrical Build/Repair"));

  // Skill Definition: Electronics
  skcat["Intelligence-Based Skills"].push_back(crc32c("Electronics"));
  skcat["Technical Skills"].push_back(crc32c("Electronics"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Electronics"));
  skcat["Modern Skills"].push_back(crc32c("Electronics"));
  skcat["Shadowrun Skills"].push_back(crc32c("Electronics"));
  skcat["Skills"].push_back(crc32c("Electronics"));

  // Skill Definition: Enchanting
  skcat["Intelligence-Based Skills"].push_back(crc32c("Enchanting"));
  skcat["Magical Skills"].push_back(crc32c("Enchanting"));
  skcat["Medieval Skills"].push_back(crc32c("Enchanting"));
  skcat["Shadowrun Skills"].push_back(crc32c("Enchanting"));
  skcat["Skills"].push_back(crc32c("Enchanting"));

  // Skill Definition: Endurance
  skcat["Strength-Based Skills"].push_back(crc32c("Endurance"));
  skcat["Medieval Skills"].push_back(crc32c("Endurance"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Endurance"));
  skcat["Modern Skills"].push_back(crc32c("Endurance"));
  skcat["Shadowrun Skills"].push_back(crc32c("Endurance"));
  skcat["Skills"].push_back(crc32c("Endurance"));

  // Skill Definition: Engineering
  skcat["Intelligence-Based Skills"].push_back(crc32c("Engineering"));
  skcat["Technical Skills"].push_back(crc32c("Engineering"));
  skcat["Medieval Skills"].push_back(crc32c("Engineering"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Engineering"));
  skcat["Modern Skills"].push_back(crc32c("Engineering"));
  skcat["Shadowrun Skills"].push_back(crc32c("Engineering"));
  skcat["Skills"].push_back(crc32c("Engineering"));

  // Skill Definition: Prepared Intermediate
  skcat["Skill-Based Skills"].push_back(crc32c("Prepared Intermediate"));
  skcat["Star Magical Skills"].push_back(crc32c("Prepared Intermediate"));
  skcat["Medieval Skills"].push_back(crc32c("Prepared Intermediate"));
  skcat["Skills"].push_back(crc32c("Prepared Intermediate"));

  // Skill Definition: Prepared Primary
  skcat["Skill-Based Skills"].push_back(crc32c("Prepared Primary"));
  skcat["Star Magical Skills"].push_back(crc32c("Prepared Primary"));
  skcat["Medieval Skills"].push_back(crc32c("Prepared Primary"));
  skcat["Skills"].push_back(crc32c("Prepared Primary"));

  // Skill Definition: Prepared Basic
  skcat["Skill-Based Skills"].push_back(crc32c("Prepared Basic"));
  skcat["Star Magical Skills"].push_back(crc32c("Prepared Basic"));
  skcat["Medieval Skills"].push_back(crc32c("Prepared Basic"));
  skcat["Skills"].push_back(crc32c("Prepared Basic"));

  // Skill Definition: Etiquette
  skcat["Charisma-Based Skills"].push_back(crc32c("Etiquette"));
  skcat["Social Skills"].push_back(crc32c("Etiquette"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Etiquette"));
  skcat["Medieval Skills"].push_back(crc32c("Etiquette"));
  skcat["Modern Skills"].push_back(crc32c("Etiquette"));
  skcat["Shadowrun Skills"].push_back(crc32c("Etiquette"));
  skcat["Skills"].push_back(crc32c("Etiquette"));

  // Skill Definition: Evasion
  skcat["Quickness-Based Skills"].push_back(crc32c("Evasion"));
  skcat["Athletic Skills"].push_back(crc32c("Evasion"));
  skcat["Medieval Skills"].push_back(crc32c("Evasion"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Evasion"));
  skcat["Modern Skills"].push_back(crc32c("Evasion"));
  skcat["Shadowrun Skills"].push_back(crc32c("Evasion"));
  skcat["Skills"].push_back(crc32c("Evasion"));

  // Skill Definition: Finance
  skcat["Intelligence-Based Skills"].push_back(crc32c("Finance"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Finance"));
  skcat["Modern Skills"].push_back(crc32c("Finance"));
  skcat["Shadowrun Skills"].push_back(crc32c("Finance"));
  skcat["Skills"].push_back(crc32c("Finance"));

  // Skill Definition: Find Juju
  skcat["Willpower-Based Skills"].push_back(crc32c("Find Juju"));
  skcat["Shamanistic Skills"].push_back(crc32c("Find Juju"));
  skcat["Medieval Skills"].push_back(crc32c("Find Juju"));
  skcat["Skills"].push_back(crc32c("Find Juju"));

  // Skill Definition: Fire Weaving
  skcat["Charisma-Based Skills"].push_back(crc32c("Fire Weaving"));
  skcat["Magical Skills"].push_back(crc32c("Fire Weaving"));
  skcat["Medieval Skills"].push_back(crc32c("Fire Weaving"));
  skcat["FP Skills"].push_back(crc32c("Fire Weaving"));
  skcat["Skills"].push_back(crc32c("Fire Weaving"));

  // Skill Definition: First Aid
  skcat["Intelligence-Based Skills"].push_back(crc32c("First Aid"));
  skcat["Technical Skills"].push_back(crc32c("First Aid"));
  skcat["Cyberpunk Skills"].push_back(crc32c("First Aid"));
  skcat["Modern Skills"].push_back(crc32c("First Aid"));
  skcat["Shadowrun Skills"].push_back(crc32c("First Aid"));
  skcat["Skills"].push_back(crc32c("First Aid"));

  // Skill Definition: Fixed-Wing Craft
  skcat["Reaction-Based Skills"].push_back(crc32c("Fixed-Wing Craft"));
  skcat["Piloting Skills"].push_back(crc32c("Fixed-Wing Craft"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Fixed-Wing Craft"));
  skcat["Modern Skills"].push_back(crc32c("Fixed-Wing Craft"));
  skcat["Shadowrun Skills"].push_back(crc32c("Fixed-Wing Craft"));
  skcat["Skills"].push_back(crc32c("Fixed-Wing Craft"));

  // Skill Definition: Fixed-Wing Drone
  skcat["Intelligence-Based Skills"].push_back(crc32c("Fixed-Wing Drone"));
  skcat["Drone Piloting Skills"].push_back(crc32c("Fixed-Wing Drone"));
  skcat["Shadowrun Skills"].push_back(crc32c("Fixed-Wing Drone"));
  skcat["Skills"].push_back(crc32c("Fixed-Wing Drone"));

  // Skill Definition: Fletcher
  skcat["Intelligence-Based Skills"].push_back(crc32c("Fletcher"));
  skcat["Creation Skills"].push_back(crc32c("Fletcher"));
  skcat["Medieval Skills"].push_back(crc32c("Fletcher"));
  skcat["Skills"].push_back(crc32c("Fletcher"));

  // Skill Definition: Flight
  skcat["Quickness-Based Skills"].push_back(crc32c("Flight"));
  skcat["Athletic Skills"].push_back(crc32c("Flight"));
  skcat["Medieval Skills"].push_back(crc32c("Flight"));
  skcat["Shadowrun Skills"].push_back(crc32c("Flight"));
  skcat["Skills"].push_back(crc32c("Flight"));

  // Skill Definition: Focusing
  skcat["Strength-Based Skills"].push_back(crc32c("Focusing"));
  skcat["Star Magical Skills"].push_back(crc32c("Focusing"));
  skcat["Medieval Skills"].push_back(crc32c("Focusing"));
  skcat["Skills"].push_back(crc32c("Focusing"));

  // Skill Definition: Grappling
  skcat["Strength-Based Skills"].push_back(crc32c("Grappling"));
  skcat["Hand-to-Hand Combat Skills"].push_back(crc32c("Grappling"));
  skcat["Medieval Skills"].push_back(crc32c("Grappling"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Grappling"));
  skcat["Modern Skills"].push_back(crc32c("Grappling"));
  skcat["Shadowrun Skills"].push_back(crc32c("Grappling"));
  skcat["Skills"].push_back(crc32c("Grappling"));
  add_wts(crc32c("Grappling"));

  // Skill Definition: Gunnery
  skcat["Intelligence-Based Skills"].push_back(crc32c("Gunnery"));
  skcat["Weapon System Skills"].push_back(crc32c("Gunnery"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Gunnery"));
  skcat["Modern Skills"].push_back(crc32c("Gunnery"));
  skcat["Shadowrun Skills"].push_back(crc32c("Gunnery"));
  skcat["Skills"].push_back(crc32c("Gunnery"));

  // Skill Definition: Half-Track
  skcat["Reaction-Based Skills"].push_back(crc32c("Half-Track"));
  skcat["Piloting Skills"].push_back(crc32c("Half-Track"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Half-Track"));
  skcat["Modern Skills"].push_back(crc32c("Half-Track"));
  skcat["Shadowrun Skills"].push_back(crc32c("Half-Track"));
  skcat["Skills"].push_back(crc32c("Half-Track"));

  // Skill Definition: Healing
  skcat["Intelligence-Based Skills"].push_back(crc32c("Healing"));
  skcat["Nature Skills"].push_back(crc32c("Healing"));
  skcat["Medieval Skills"].push_back(crc32c("Healing"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Healing"));
  skcat["Modern Skills"].push_back(crc32c("Healing"));
  skcat["Shadowrun Skills"].push_back(crc32c("Healing"));
  skcat["Skills"].push_back(crc32c("Healing"));

  // Skill Definition: Heavy Lasers
  skcat["Strength-Based Skills"].push_back(crc32c("Heavy Lasers"));
  skcat["Heavy Firearm Skills"].push_back(crc32c("Heavy Lasers"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Heavy Lasers"));
  skcat["Modern Skills"].push_back(crc32c("Heavy Lasers"));
  skcat["Shadowrun Skills"].push_back(crc32c("Heavy Lasers"));
  skcat["Skills"].push_back(crc32c("Heavy Lasers"));

  // Skill Definition: Heavy Projectors
  skcat["Strength-Based Skills"].push_back(crc32c("Heavy Projectors"));
  skcat["Heavy Firearm Skills"].push_back(crc32c("Heavy Projectors"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Heavy Projectors"));
  skcat["Modern Skills"].push_back(crc32c("Heavy Projectors"));
  skcat["Shadowrun Skills"].push_back(crc32c("Heavy Projectors"));
  skcat["Skills"].push_back(crc32c("Heavy Projectors"));

  // Skill Definition: Heavy Rifles
  skcat["Strength-Based Skills"].push_back(crc32c("Heavy Rifles"));
  skcat["Heavy Firearm Skills"].push_back(crc32c("Heavy Rifles"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Heavy Rifles"));
  skcat["Modern Skills"].push_back(crc32c("Heavy Rifles"));
  skcat["Shadowrun Skills"].push_back(crc32c("Heavy Rifles"));
  skcat["Skills"].push_back(crc32c("Heavy Rifles"));

  // Skill Definition: Helmsman, Star
  skcat["Intelligence-Based Skills"].push_back(crc32c("Helmsman, Star"));
  skcat["Magical Skills"].push_back(crc32c("Helmsman, Star"));
  skcat["Medieval Skills"].push_back(crc32c("Helmsman, Star"));
  skcat["Skills"].push_back(crc32c("Helmsman, Star"));

  // Skill Definition: High-G Combat
  skcat["Intelligence-Based Skills"].push_back(crc32c("High-G Combat"));
  skcat["Combat Environment Skills"].push_back(crc32c("High-G Combat"));
  skcat["Medieval Skills"].push_back(crc32c("High-G Combat"));
  skcat["Cyberpunk Skills"].push_back(crc32c("High-G Combat"));
  skcat["Modern Skills"].push_back(crc32c("High-G Combat"));
  skcat["Shadowrun Skills"].push_back(crc32c("High-G Combat"));
  skcat["Skills"].push_back(crc32c("High-G Combat"));

  // Skill Definition: High-G Ops
  skcat["Intelligence-Based Skills"].push_back(crc32c("High-G Ops"));
  skcat["Specialized Skills"].push_back(crc32c("High-G Ops"));
  skcat["Cyberpunk Skills"].push_back(crc32c("High-G Ops"));
  skcat["Shadowrun Skills"].push_back(crc32c("High-G Ops"));
  skcat["Skills"].push_back(crc32c("High-G Ops"));

  // Skill Definition: Hovercraft
  skcat["Reaction-Based Skills"].push_back(crc32c("Hovercraft"));
  skcat["Piloting Skills"].push_back(crc32c("Hovercraft"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Hovercraft"));
  skcat["Modern Skills"].push_back(crc32c("Hovercraft"));
  skcat["Shadowrun Skills"].push_back(crc32c("Hovercraft"));
  skcat["Skills"].push_back(crc32c("Hovercraft"));

  // Skill Definition: Hover Drone
  skcat["Intelligence-Based Skills"].push_back(crc32c("Hover Drone"));
  skcat["Drone Piloting Skills"].push_back(crc32c("Hover Drone"));
  skcat["Shadowrun Skills"].push_back(crc32c("Hover Drone"));
  skcat["Skills"].push_back(crc32c("Hover Drone"));

  // Skill Definition: Hurling
  skcat["Strength-Based Skills"].push_back(crc32c("Hurling"));
  skcat["Ranged-Combat Skills"].push_back(crc32c("Hurling"));
  skcat["Medieval Skills"].push_back(crc32c("Hurling"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Hurling"));
  skcat["Modern Skills"].push_back(crc32c("Hurling"));
  skcat["Shadowrun Skills"].push_back(crc32c("Hurling"));
  skcat["Skills"].push_back(crc32c("Hurling"));
  add_wts(crc32c("Hurling"));

  // Skill Definition: Identify Juju
  skcat["Intelligence-Based Skills"].push_back(crc32c("Identify Juju"));
  skcat["Shamanistic Skills"].push_back(crc32c("Identify Juju"));
  skcat["Medieval Skills"].push_back(crc32c("Identify Juju"));
  skcat["Skills"].push_back(crc32c("Identify Juju"));

  // Skill Definition: Interrogation
  skcat["Intelligence-Based Skills"].push_back(crc32c("Interrogation"));
  skcat["Social Skills"].push_back(crc32c("Interrogation"));
  skcat["Medieval Skills"].push_back(crc32c("Interrogation"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Interrogation"));
  skcat["Modern Skills"].push_back(crc32c("Interrogation"));
  skcat["Shadowrun Skills"].push_back(crc32c("Interrogation"));
  skcat["Skills"].push_back(crc32c("Interrogation"));

  // Skill Definition: Intimidation
  skcat["Willpower-Based Skills"].push_back(crc32c("Intimidation"));
  skcat["Social Skills"].push_back(crc32c("Intimidation"));
  skcat["Medieval Skills"].push_back(crc32c("Intimidation"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Intimidation"));
  skcat["Modern Skills"].push_back(crc32c("Intimidation"));
  skcat["Shadowrun Skills"].push_back(crc32c("Intimidation"));
  skcat["Skills"].push_back(crc32c("Intimidation"));

  // Skill Definition: Jumping
  skcat["Strength-Based Skills"].push_back(crc32c("Jumping"));
  skcat["Athletic Skills"].push_back(crc32c("Jumping"));
  skcat["Medieval Skills"].push_back(crc32c("Jumping"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Jumping"));
  skcat["Modern Skills"].push_back(crc32c("Jumping"));
  skcat["Shadowrun Skills"].push_back(crc32c("Jumping"));
  skcat["Skills"].push_back(crc32c("Jumping"));

  // Skill Definition: Kicking
  skcat["Quickness-Based Skills"].push_back(crc32c("Kicking"));
  skcat["Hand-to-Hand Combat Skills"].push_back(crc32c("Kicking"));
  skcat["Medieval Skills"].push_back(crc32c("Kicking"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Kicking"));
  skcat["Modern Skills"].push_back(crc32c("Kicking"));
  skcat["Shadowrun Skills"].push_back(crc32c("Kicking"));
  skcat["Skills"].push_back(crc32c("Kicking"));
  add_wts(crc32c("Kicking"));

  // Skill Definition: Knowledge
  skcat["Intelligence-Based Skills"].push_back(crc32c("Knowledge"));
  skcat["Specialized Skills"].push_back(crc32c("Knowledge"));
  skcat["Medieval Skills"].push_back(crc32c("Knowledge"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Knowledge"));
  skcat["Modern Skills"].push_back(crc32c("Knowledge"));
  skcat["Shadowrun Skills"].push_back(crc32c("Knowledge"));
  skcat["Skills"].push_back(crc32c("Knowledge"));

  // Skill Definition: Laser Pistols
  skcat["Quickness-Based Skills"].push_back(crc32c("Laser Pistols"));
  skcat["Pistol Skills"].push_back(crc32c("Laser Pistols"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Laser Pistols"));
  skcat["Modern Skills"].push_back(crc32c("Laser Pistols"));
  skcat["Shadowrun Skills"].push_back(crc32c("Laser Pistols"));
  skcat["Skills"].push_back(crc32c("Laser Pistols"));
  add_wts(crc32c("Laser Pistols"));

  // Skill Definition: Laser Rifles
  skcat["Quickness-Based Skills"].push_back(crc32c("Laser Rifles"));
  skcat["Rifle Skills"].push_back(crc32c("Laser Rifles"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Laser Rifles"));
  skcat["Modern Skills"].push_back(crc32c("Laser Rifles"));
  skcat["Shadowrun Skills"].push_back(crc32c("Laser Rifles"));
  skcat["Skills"].push_back(crc32c("Laser Rifles"));
  add_wts(crc32c("Laser Rifles"));

  // Skill Definition: Lasso
  skcat["Quickness-Based Skills"].push_back(crc32c("Lasso"));
  skcat["Ranged-Combat Skills"].push_back(crc32c("Lasso"));
  skcat["Medieval Skills"].push_back(crc32c("Lasso"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Lasso"));
  skcat["Modern Skills"].push_back(crc32c("Lasso"));
  skcat["Shadowrun Skills"].push_back(crc32c("Lasso"));
  skcat["Skills"].push_back(crc32c("Lasso"));
  add_wts(crc32c("Lasso"));

  // Skill Definition: Launch Weapons
  skcat["Intelligence-Based Skills"].push_back(crc32c("Launch Weapons"));
  skcat["Weapon System Skills"].push_back(crc32c("Launch Weapons"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Launch Weapons"));
  skcat["Modern Skills"].push_back(crc32c("Launch Weapons"));
  skcat["Shadowrun Skills"].push_back(crc32c("Launch Weapons"));
  skcat["Skills"].push_back(crc32c("Launch Weapons"));

  // Skill Definition: Leadership
  skcat["Charisma-Based Skills"].push_back(crc32c("Leadership"));
  skcat["Social Skills"].push_back(crc32c("Leadership"));
  skcat["Medieval Skills"].push_back(crc32c("Leadership"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Leadership"));
  skcat["Modern Skills"].push_back(crc32c("Leadership"));
  skcat["Shadowrun Skills"].push_back(crc32c("Leadership"));
  skcat["Skills"].push_back(crc32c("Leadership"));

  // Skill Definition: Leatherworking
  skcat["Intelligence-Based Skills"].push_back(crc32c("Leatherworking"));
  skcat["Creation Skills"].push_back(crc32c("Leatherworking"));
  skcat["Medieval Skills"].push_back(crc32c("Leatherworking"));
  skcat["Skills"].push_back(crc32c("Leatherworking"));

  // Skill Definition: Lifting
  skcat["Strength-Based Skills"].push_back(crc32c("Lifting"));
  skcat["Athletic Skills"].push_back(crc32c("Lifting"));
  skcat["Medieval Skills"].push_back(crc32c("Lifting"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Lifting"));
  skcat["Modern Skills"].push_back(crc32c("Lifting"));
  skcat["Shadowrun Skills"].push_back(crc32c("Lifting"));
  skcat["Skills"].push_back(crc32c("Lifting"));

  // Skill Definition: Long Blades
  skcat["Quickness-Based Skills"].push_back(crc32c("Long Blades"));
  skcat["Melee-Combat Skills"].push_back(crc32c("Long Blades"));
  skcat["Medieval Skills"].push_back(crc32c("Long Blades"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Long Blades"));
  skcat["Modern Skills"].push_back(crc32c("Long Blades"));
  skcat["Shadowrun Skills"].push_back(crc32c("Long Blades"));
  skcat["Skills"].push_back(crc32c("Long Blades"));
  add_wts(crc32c("Long Blades"));

  // Skill Definition: Long Cleaves
  skcat["Quickness-Based Skills"].push_back(crc32c("Long Cleaves"));
  skcat["Melee-Combat Skills"].push_back(crc32c("Long Cleaves"));
  skcat["Medieval Skills"].push_back(crc32c("Long Cleaves"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Long Cleaves"));
  skcat["Modern Skills"].push_back(crc32c("Long Cleaves"));
  skcat["Shadowrun Skills"].push_back(crc32c("Long Cleaves"));
  skcat["Skills"].push_back(crc32c("Long Cleaves"));
  add_wts(crc32c("Long Cleaves"));

  // Skill Definition: Long Crushing
  skcat["Quickness-Based Skills"].push_back(crc32c("Long Crushing"));
  skcat["Melee-Combat Skills"].push_back(crc32c("Long Crushing"));
  skcat["Medieval Skills"].push_back(crc32c("Long Crushing"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Long Crushing"));
  skcat["Modern Skills"].push_back(crc32c("Long Crushing"));
  skcat["Shadowrun Skills"].push_back(crc32c("Long Crushing"));
  skcat["Skills"].push_back(crc32c("Long Crushing"));
  add_wts(crc32c("Long Crushing"));

  // Skill Definition: Long Flails
  skcat["Quickness-Based Skills"].push_back(crc32c("Long Flails"));
  skcat["Melee-Combat Skills"].push_back(crc32c("Long Flails"));
  skcat["Medieval Skills"].push_back(crc32c("Long Flails"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Long Flails"));
  skcat["Modern Skills"].push_back(crc32c("Long Flails"));
  skcat["Shadowrun Skills"].push_back(crc32c("Long Flails"));
  skcat["Skills"].push_back(crc32c("Long Flails"));
  add_wts(crc32c("Long Flails"));

  // Skill Definition: Long Piercing
  skcat["Quickness-Based Skills"].push_back(crc32c("Long Piercing"));
  skcat["Melee-Combat Skills"].push_back(crc32c("Long Piercing"));
  skcat["Medieval Skills"].push_back(crc32c("Long Piercing"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Long Piercing"));
  skcat["Modern Skills"].push_back(crc32c("Long Piercing"));
  skcat["Shadowrun Skills"].push_back(crc32c("Long Piercing"));
  skcat["Skills"].push_back(crc32c("Long Piercing"));
  add_wts(crc32c("Long Piercing"));

  // Skill Definition: Long Staves
  skcat["Quickness-Based Skills"].push_back(crc32c("Long Staves"));
  skcat["Melee-Combat Skills"].push_back(crc32c("Long Staves"));
  skcat["Medieval Skills"].push_back(crc32c("Long Staves"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Long Staves"));
  skcat["Modern Skills"].push_back(crc32c("Long Staves"));
  skcat["Shadowrun Skills"].push_back(crc32c("Long Staves"));
  skcat["Skills"].push_back(crc32c("Long Staves"));
  add_wts(crc32c("Long Staves"));

  // Skill Definition: Low-G Combat
  skcat["Intelligence-Based Skills"].push_back(crc32c("Low-G Combat"));
  skcat["Combat Environment Skills"].push_back(crc32c("Low-G Combat"));
  skcat["Medieval Skills"].push_back(crc32c("Low-G Combat"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Low-G Combat"));
  skcat["Modern Skills"].push_back(crc32c("Low-G Combat"));
  skcat["Shadowrun Skills"].push_back(crc32c("Low-G Combat"));
  skcat["Skills"].push_back(crc32c("Low-G Combat"));

  // Skill Definition: Low-G Ops
  skcat["Intelligence-Based Skills"].push_back(crc32c("Low-G Ops"));
  skcat["Specialized Skills"].push_back(crc32c("Low-G Ops"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Low-G Ops"));
  skcat["Shadowrun Skills"].push_back(crc32c("Low-G Ops"));
  skcat["Skills"].push_back(crc32c("Low-G Ops"));

  // Skill Definition: Lumberjack
  skcat["Strength-Based Skills"].push_back(crc32c("Lumberjack"));
  skcat["Creation Skills"].push_back(crc32c("Lumberjack"));
  skcat["Medieval Skills"].push_back(crc32c("Lumberjack"));
  skcat["Modern Skills"].push_back(crc32c("Lumberjack"));
  skcat["Skills"].push_back(crc32c("Lumberjack"));

  // Skill Definition: Machine Guns
  skcat["Strength-Based Skills"].push_back(crc32c("Machine Guns"));
  skcat["Heavy Firearm Skills"].push_back(crc32c("Machine Guns"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Machine Guns"));
  skcat["Modern Skills"].push_back(crc32c("Machine Guns"));
  skcat["Shadowrun Skills"].push_back(crc32c("Machine Guns"));
  skcat["Skills"].push_back(crc32c("Machine Guns"));

  // Skill Definition: Machine Pistols
  skcat["Quickness-Based Skills"].push_back(crc32c("Machine Pistols"));
  skcat["Pistol Skills"].push_back(crc32c("Machine Pistols"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Machine Pistols"));
  skcat["Modern Skills"].push_back(crc32c("Machine Pistols"));
  skcat["Shadowrun Skills"].push_back(crc32c("Machine Pistols"));
  skcat["Skills"].push_back(crc32c("Machine Pistols"));
  add_wts(crc32c("Machine Pistols"));

  // Skill Definition: Magic Manipulation
  skcat["Charisma-Based Skills"].push_back(crc32c("Magic Manipulation"));
  skcat["Magical Skills"].push_back(crc32c("Magic Manipulation"));
  skcat["Medieval Skills"].push_back(crc32c("Magic Manipulation"));
  skcat["Skills"].push_back(crc32c("Magic Manipulation"));

  // Skill Definition: Martial Arts, Elven
  skcat["Willpower-Based Skills"].push_back(crc32c("Martial Arts, Elven"));
  skcat["Martial Arts Skills"].push_back(crc32c("Martial Arts, Elven"));
  skcat["Medieval Skills"].push_back(crc32c("Martial Arts, Elven"));
  skcat["Skills"].push_back(crc32c("Martial Arts, Elven"));

  // Skill Definition: Masonry
  skcat["Intelligence-Based Skills"].push_back(crc32c("Masonry"));
  skcat["Creation Skills"].push_back(crc32c("Masonry"));
  skcat["Medieval Skills"].push_back(crc32c("Masonry"));
  skcat["Skills"].push_back(crc32c("Masonry"));

  // Skill Definition: Mechanical Build/Repair
  skcat["Intelligence-Based Skills"].push_back(crc32c("Mechanical Build/Repair"));
  skcat["Build/Repair Skills"].push_back(crc32c("Mechanical Build/Repair"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Mechanical Build/Repair"));
  skcat["Modern Skills"].push_back(crc32c("Mechanical Build/Repair"));
  skcat["Shadowrun Skills"].push_back(crc32c("Mechanical Build/Repair"));
  skcat["Skills"].push_back(crc32c("Mechanical Build/Repair"));

  // Skill Definition: Mechanics
  skcat["Intelligence-Based Skills"].push_back(crc32c("Mechanics"));
  skcat["Technical Skills"].push_back(crc32c("Mechanics"));
  skcat["Medieval Skills"].push_back(crc32c("Mechanics"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Mechanics"));
  skcat["Modern Skills"].push_back(crc32c("Mechanics"));
  skcat["Shadowrun Skills"].push_back(crc32c("Mechanics"));
  skcat["Skills"].push_back(crc32c("Mechanics"));

  // Skill Definition: Metalworking
  skcat["Intelligence-Based Skills"].push_back(crc32c("Metalworking"));
  skcat["Creation Skills"].push_back(crc32c("Metalworking"));
  skcat["Medieval Skills"].push_back(crc32c("Metalworking"));
  skcat["Skills"].push_back(crc32c("Metalworking"));

  // Skill Definition: Mindcasting
  skcat["Willpower-Based Skills"].push_back(crc32c("Mindcasting"));
  skcat["Magical Skills"].push_back(crc32c("Mindcasting"));
  skcat["Medieval Skills"].push_back(crc32c("Mindcasting"));
  skcat["Shadowrun Skills"].push_back(crc32c("Mindcasting"));
  skcat["Skills"].push_back(crc32c("Mindcasting"));

  // Skill Definition: Mounted Air Pistols
  skcat["Quickness-Based Skills"].push_back(crc32c("Mounted Air Pistols"));
  skcat["Mounted Pistol Skills"].push_back(crc32c("Mounted Air Pistols"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Mounted Air Pistols"));
  skcat["Modern Skills"].push_back(crc32c("Mounted Air Pistols"));
  skcat["Shadowrun Skills"].push_back(crc32c("Mounted Air Pistols"));
  skcat["Skills"].push_back(crc32c("Mounted Air Pistols"));
  add_wts(crc32c("Mounted Air Pistols"));

  // Skill Definition: Mounted Archery
  skcat["Quickness-Based Skills"].push_back(crc32c("Mounted Archery"));
  skcat["Mounted Ranged-Combat Skills"].push_back(crc32c("Mounted Archery"));
  skcat["Medieval Skills"].push_back(crc32c("Mounted Archery"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Mounted Archery"));
  skcat["Modern Skills"].push_back(crc32c("Mounted Archery"));
  skcat["Shadowrun Skills"].push_back(crc32c("Mounted Archery"));
  skcat["Skills"].push_back(crc32c("Mounted Archery"));
  add_wts(crc32c("Mounted Archery"));

  // Skill Definition: Mounted Blades
  skcat["Quickness-Based Skills"].push_back(crc32c("Mounted Blades"));
  skcat["Mounted Melee-Combat Skills"].push_back(crc32c("Mounted Blades"));
  skcat["Medieval Skills"].push_back(crc32c("Mounted Blades"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Mounted Blades"));
  skcat["Modern Skills"].push_back(crc32c("Mounted Blades"));
  skcat["Shadowrun Skills"].push_back(crc32c("Mounted Blades"));
  skcat["Skills"].push_back(crc32c("Mounted Blades"));
  add_wts(crc32c("Mounted Blades"));

  // Skill Definition: Mounted Blowgun
  skcat["Quickness-Based Skills"].push_back(crc32c("Mounted Blowgun"));
  skcat["Mounted Ranged-Combat Skills"].push_back(crc32c("Mounted Blowgun"));
  skcat["Medieval Skills"].push_back(crc32c("Mounted Blowgun"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Mounted Blowgun"));
  skcat["Modern Skills"].push_back(crc32c("Mounted Blowgun"));
  skcat["Shadowrun Skills"].push_back(crc32c("Mounted Blowgun"));
  skcat["Skills"].push_back(crc32c("Mounted Blowgun"));
  add_wts(crc32c("Mounted Blowgun"));

  // Skill Definition: Mounted Cleaves
  skcat["Quickness-Based Skills"].push_back(crc32c("Mounted Cleaves"));
  skcat["Mounted Melee-Combat Skills"].push_back(crc32c("Mounted Cleaves"));
  skcat["Medieval Skills"].push_back(crc32c("Mounted Cleaves"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Mounted Cleaves"));
  skcat["Modern Skills"].push_back(crc32c("Mounted Cleaves"));
  skcat["Shadowrun Skills"].push_back(crc32c("Mounted Cleaves"));
  skcat["Skills"].push_back(crc32c("Mounted Cleaves"));
  add_wts(crc32c("Mounted Cleaves"));

  // Skill Definition: Mounted Crossbow
  skcat["Quickness-Based Skills"].push_back(crc32c("Mounted Crossbow"));
  skcat["Mounted Ranged-Combat Skills"].push_back(crc32c("Mounted Crossbow"));
  skcat["Medieval Skills"].push_back(crc32c("Mounted Crossbow"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Mounted Crossbow"));
  skcat["Modern Skills"].push_back(crc32c("Mounted Crossbow"));
  skcat["Shadowrun Skills"].push_back(crc32c("Mounted Crossbow"));
  skcat["Skills"].push_back(crc32c("Mounted Crossbow"));
  add_wts(crc32c("Mounted Crossbow"));

  // Skill Definition: Mounted Crushing
  skcat["Quickness-Based Skills"].push_back(crc32c("Mounted Crushing"));
  skcat["Mounted Melee-Combat Skills"].push_back(crc32c("Mounted Crushing"));
  skcat["Medieval Skills"].push_back(crc32c("Mounted Crushing"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Mounted Crushing"));
  skcat["Modern Skills"].push_back(crc32c("Mounted Crushing"));
  skcat["Shadowrun Skills"].push_back(crc32c("Mounted Crushing"));
  skcat["Skills"].push_back(crc32c("Mounted Crushing"));
  add_wts(crc32c("Mounted Crushing"));

  // Skill Definition: Mounted Flails
  skcat["Quickness-Based Skills"].push_back(crc32c("Mounted Flails"));
  skcat["Mounted Melee-Combat Skills"].push_back(crc32c("Mounted Flails"));
  skcat["Medieval Skills"].push_back(crc32c("Mounted Flails"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Mounted Flails"));
  skcat["Modern Skills"].push_back(crc32c("Mounted Flails"));
  skcat["Shadowrun Skills"].push_back(crc32c("Mounted Flails"));
  skcat["Skills"].push_back(crc32c("Mounted Flails"));
  add_wts(crc32c("Mounted Flails"));

  // Skill Definition: Mounted Hurling
  skcat["Quickness-Based Skills"].push_back(crc32c("Mounted Hurling"));
  skcat["Mounted Ranged-Combat Skills"].push_back(crc32c("Mounted Hurling"));
  skcat["Medieval Skills"].push_back(crc32c("Mounted Hurling"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Mounted Hurling"));
  skcat["Modern Skills"].push_back(crc32c("Mounted Hurling"));
  skcat["Shadowrun Skills"].push_back(crc32c("Mounted Hurling"));
  skcat["Skills"].push_back(crc32c("Mounted Hurling"));
  add_wts(crc32c("Mounted Hurling"));

  // Skill Definition: Mounted Laser Pistols
  skcat["Quickness-Based Skills"].push_back(crc32c("Mounted Laser Pistols"));
  skcat["Mounted Pistol Skills"].push_back(crc32c("Mounted Laser Pistols"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Mounted Laser Pistols"));
  skcat["Modern Skills"].push_back(crc32c("Mounted Laser Pistols"));
  skcat["Shadowrun Skills"].push_back(crc32c("Mounted Laser Pistols"));
  skcat["Skills"].push_back(crc32c("Mounted Laser Pistols"));
  add_wts(crc32c("Mounted Laser Pistols"));

  // Skill Definition: Mounted Machine Pistols
  skcat["Quickness-Based Skills"].push_back(crc32c("Mounted Machine Pistols"));
  skcat["Mounted Pistol Skills"].push_back(crc32c("Mounted Machine Pistols"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Mounted Machine Pistols"));
  skcat["Modern Skills"].push_back(crc32c("Mounted Machine Pistols"));
  skcat["Shadowrun Skills"].push_back(crc32c("Mounted Machine Pistols"));
  skcat["Skills"].push_back(crc32c("Mounted Machine Pistols"));
  add_wts(crc32c("Mounted Machine Pistols"));

  // Skill Definition: Mounted Nets
  skcat["Quickness-Based Skills"].push_back(crc32c("Mounted Nets"));
  skcat["Mounted Ranged-Combat Skills"].push_back(crc32c("Mounted Nets"));
  skcat["Medieval Skills"].push_back(crc32c("Mounted Nets"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Mounted Nets"));
  skcat["Modern Skills"].push_back(crc32c("Mounted Nets"));
  skcat["Shadowrun Skills"].push_back(crc32c("Mounted Nets"));
  skcat["Skills"].push_back(crc32c("Mounted Nets"));
  add_wts(crc32c("Mounted Nets"));

  // Skill Definition: Mounted Piercing
  skcat["Quickness-Based Skills"].push_back(crc32c("Mounted Piercing"));
  skcat["Mounted Melee-Combat Skills"].push_back(crc32c("Mounted Piercing"));
  skcat["Medieval Skills"].push_back(crc32c("Mounted Piercing"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Mounted Piercing"));
  skcat["Modern Skills"].push_back(crc32c("Mounted Piercing"));
  skcat["Shadowrun Skills"].push_back(crc32c("Mounted Piercing"));
  skcat["Skills"].push_back(crc32c("Mounted Piercing"));
  add_wts(crc32c("Mounted Piercing"));

  // Skill Definition: Mounted Pistols
  skcat["Quickness-Based Skills"].push_back(crc32c("Mounted Pistols"));
  skcat["Mounted Pistol Skills"].push_back(crc32c("Mounted Pistols"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Mounted Pistols"));
  skcat["Modern Skills"].push_back(crc32c("Mounted Pistols"));
  skcat["Shadowrun Skills"].push_back(crc32c("Mounted Pistols"));
  skcat["Skills"].push_back(crc32c("Mounted Pistols"));
  add_wts(crc32c("Mounted Pistols"));

  // Skill Definition: Mounted Plasma Pistols
  skcat["Quickness-Based Skills"].push_back(crc32c("Mounted Plasma Pistols"));
  skcat["Mounted Pistol Skills"].push_back(crc32c("Mounted Plasma Pistols"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Mounted Plasma Pistols"));
  skcat["Modern Skills"].push_back(crc32c("Mounted Plasma Pistols"));
  skcat["Shadowrun Skills"].push_back(crc32c("Mounted Plasma Pistols"));
  skcat["Skills"].push_back(crc32c("Mounted Plasma Pistols"));
  add_wts(crc32c("Mounted Plasma Pistols"));

  // Skill Definition: Mounted Shot Pistols
  skcat["Quickness-Based Skills"].push_back(crc32c("Mounted Shot Pistols"));
  skcat["Mounted Pistol Skills"].push_back(crc32c("Mounted Shot Pistols"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Mounted Shot Pistols"));
  skcat["Modern Skills"].push_back(crc32c("Mounted Shot Pistols"));
  skcat["Shadowrun Skills"].push_back(crc32c("Mounted Shot Pistols"));
  skcat["Skills"].push_back(crc32c("Mounted Shot Pistols"));
  add_wts(crc32c("Mounted Shot Pistols"));

  // Skill Definition: Mounted Slings
  skcat["Quickness-Based Skills"].push_back(crc32c("Mounted Slings"));
  skcat["Mounted Ranged-Combat Skills"].push_back(crc32c("Mounted Slings"));
  skcat["Medieval Skills"].push_back(crc32c("Mounted Slings"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Mounted Slings"));
  skcat["Modern Skills"].push_back(crc32c("Mounted Slings"));
  skcat["Shadowrun Skills"].push_back(crc32c("Mounted Slings"));
  skcat["Skills"].push_back(crc32c("Mounted Slings"));
  add_wts(crc32c("Mounted Slings"));

  // Skill Definition: Mounted SMGs
  skcat["Quickness-Based Skills"].push_back(crc32c("Mounted SMGs"));
  skcat["Mounted Pistol Skills"].push_back(crc32c("Mounted SMGs"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Mounted SMGs"));
  skcat["Modern Skills"].push_back(crc32c("Mounted SMGs"));
  skcat["Shadowrun Skills"].push_back(crc32c("Mounted SMGs"));
  skcat["Skills"].push_back(crc32c("Mounted SMGs"));
  add_wts(crc32c("Mounted SMGs"));

  // Skill Definition: Mounted Throwing, Aero
  skcat["Quickness-Based Skills"].push_back(crc32c("Mounted Throwing, Aero"));
  skcat["Mounted Ranged-Combat Skills"].push_back(crc32c("Mounted Throwing, Aero"));
  skcat["Medieval Skills"].push_back(crc32c("Mounted Throwing, Aero"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Mounted Throwing, Aero"));
  skcat["Modern Skills"].push_back(crc32c("Mounted Throwing, Aero"));
  skcat["Shadowrun Skills"].push_back(crc32c("Mounted Throwing, Aero"));
  skcat["Skills"].push_back(crc32c("Mounted Throwing, Aero"));
  add_wts(crc32c("Mounted Throwing, Aero"));

  // Skill Definition: Mounted Throwing, Non-Aero
  skcat["Quickness-Based Skills"].push_back(crc32c("Mounted Throwing, Non-Aero"));
  skcat["Mounted Ranged-Combat Skills"].push_back(crc32c("Mounted Throwing, Non-Aero"));
  skcat["Medieval Skills"].push_back(crc32c("Mounted Throwing, Non-Aero"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Mounted Throwing, Non-Aero"));
  skcat["Modern Skills"].push_back(crc32c("Mounted Throwing, Non-Aero"));
  skcat["Shadowrun Skills"].push_back(crc32c("Mounted Throwing, Non-Aero"));
  skcat["Skills"].push_back(crc32c("Mounted Throwing, Non-Aero"));
  add_wts(crc32c("Mounted Throwing, Non-Aero"));

  // Skill Definition: Mounted Whips
  skcat["Quickness-Based Skills"].push_back(crc32c("Mounted Whips"));
  skcat["Mounted Ranged-Combat Skills"].push_back(crc32c("Mounted Whips"));
  skcat["Medieval Skills"].push_back(crc32c("Mounted Whips"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Mounted Whips"));
  skcat["Modern Skills"].push_back(crc32c("Mounted Whips"));
  skcat["Shadowrun Skills"].push_back(crc32c("Mounted Whips"));
  skcat["Skills"].push_back(crc32c("Mounted Whips"));
  add_wts(crc32c("Mounted Whips"));

  // Skill Definition: Navigation
  skcat["Intelligence-Based Skills"].push_back(crc32c("Navigation"));
  skcat["Nature Skills"].push_back(crc32c("Navigation"));
  skcat["Medieval Skills"].push_back(crc32c("Navigation"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Navigation"));
  skcat["Modern Skills"].push_back(crc32c("Navigation"));
  skcat["Shadowrun Skills"].push_back(crc32c("Navigation"));
  skcat["Skills"].push_back(crc32c("Navigation"));

  // Skill Definition: Negotiation
  skcat["Charisma-Based Skills"].push_back(crc32c("Negotiation"));
  skcat["Social Skills"].push_back(crc32c("Negotiation"));
  skcat["Medieval Skills"].push_back(crc32c("Negotiation"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Negotiation"));
  skcat["Modern Skills"].push_back(crc32c("Negotiation"));
  skcat["Shadowrun Skills"].push_back(crc32c("Negotiation"));
  skcat["Skills"].push_back(crc32c("Negotiation"));

  // Skill Definition: Nets
  skcat["Quickness-Based Skills"].push_back(crc32c("Nets"));
  skcat["Ranged-Combat Skills"].push_back(crc32c("Nets"));
  skcat["Medieval Skills"].push_back(crc32c("Nets"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Nets"));
  skcat["Modern Skills"].push_back(crc32c("Nets"));
  skcat["Shadowrun Skills"].push_back(crc32c("Nets"));
  skcat["Skills"].push_back(crc32c("Nets"));
  add_wts(crc32c("Nets"));

  // Skill Definition: Neural Interface
  skcat["Willpower-Based Skills"].push_back(crc32c("Neural Interface"));
  skcat["Technical Skills"].push_back(crc32c("Neural Interface"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Neural Interface"));
  skcat["Shadowrun Skills"].push_back(crc32c("Neural Interface"));
  skcat["Skills"].push_back(crc32c("Neural Interface"));

  // Skill Definition: Offhand Air Pistols
  skcat["Quickness-Based Skills"].push_back(crc32c("Offhand Air Pistols"));
  skcat["Pistol Skills"].push_back(crc32c("Offhand Air Pistols"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Offhand Air Pistols"));
  skcat["Modern Skills"].push_back(crc32c("Offhand Air Pistols"));
  skcat["Shadowrun Skills"].push_back(crc32c("Offhand Air Pistols"));
  skcat["Skills"].push_back(crc32c("Offhand Air Pistols"));
  add_wts(crc32c("Offhand Air Pistols"));

  // Skill Definition: Offhand Blades
  skcat["Quickness-Based Skills"].push_back(crc32c("Offhand Blades"));
  skcat["Melee-Combat Skills"].push_back(crc32c("Offhand Blades"));
  skcat["Medieval Skills"].push_back(crc32c("Offhand Blades"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Offhand Blades"));
  skcat["Modern Skills"].push_back(crc32c("Offhand Blades"));
  skcat["Shadowrun Skills"].push_back(crc32c("Offhand Blades"));
  skcat["Skills"].push_back(crc32c("Offhand Blades"));
  add_wts(crc32c("Offhand Blades"));

  // Skill Definition: Offhand Cleaves
  skcat["Quickness-Based Skills"].push_back(crc32c("Offhand Cleaves"));
  skcat["Melee-Combat Skills"].push_back(crc32c("Offhand Cleaves"));
  skcat["Medieval Skills"].push_back(crc32c("Offhand Cleaves"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Offhand Cleaves"));
  skcat["Modern Skills"].push_back(crc32c("Offhand Cleaves"));
  skcat["Shadowrun Skills"].push_back(crc32c("Offhand Cleaves"));
  skcat["Skills"].push_back(crc32c("Offhand Cleaves"));
  add_wts(crc32c("Offhand Cleaves"));

  // Skill Definition: Offhand Crossbow
  skcat["Quickness-Based Skills"].push_back(crc32c("Offhand Crossbow"));
  skcat["Ranged-Combat Skills"].push_back(crc32c("Offhand Crossbow"));
  skcat["Medieval Skills"].push_back(crc32c("Offhand Crossbow"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Offhand Crossbow"));
  skcat["Modern Skills"].push_back(crc32c("Offhand Crossbow"));
  skcat["Shadowrun Skills"].push_back(crc32c("Offhand Crossbow"));
  skcat["Skills"].push_back(crc32c("Offhand Crossbow"));
  add_wts(crc32c("Offhand Crossbow"));

  // Skill Definition: Offhand Crushing
  skcat["Quickness-Based Skills"].push_back(crc32c("Offhand Crushing"));
  skcat["Melee-Combat Skills"].push_back(crc32c("Offhand Crushing"));
  skcat["Medieval Skills"].push_back(crc32c("Offhand Crushing"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Offhand Crushing"));
  skcat["Modern Skills"].push_back(crc32c("Offhand Crushing"));
  skcat["Shadowrun Skills"].push_back(crc32c("Offhand Crushing"));
  skcat["Skills"].push_back(crc32c("Offhand Crushing"));
  add_wts(crc32c("Offhand Crushing"));

  // Skill Definition: Offhand Flails
  skcat["Quickness-Based Skills"].push_back(crc32c("Offhand Flails"));
  skcat["Melee-Combat Skills"].push_back(crc32c("Offhand Flails"));
  skcat["Medieval Skills"].push_back(crc32c("Offhand Flails"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Offhand Flails"));
  skcat["Modern Skills"].push_back(crc32c("Offhand Flails"));
  skcat["Shadowrun Skills"].push_back(crc32c("Offhand Flails"));
  skcat["Skills"].push_back(crc32c("Offhand Flails"));
  add_wts(crc32c("Offhand Flails"));

  // Skill Definition: Offhand Hurling
  skcat["Quickness-Based Skills"].push_back(crc32c("Offhand Hurling"));
  skcat["Ranged-Combat Skills"].push_back(crc32c("Offhand Hurling"));
  skcat["Medieval Skills"].push_back(crc32c("Offhand Hurling"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Offhand Hurling"));
  skcat["Modern Skills"].push_back(crc32c("Offhand Hurling"));
  skcat["Shadowrun Skills"].push_back(crc32c("Offhand Hurling"));
  skcat["Skills"].push_back(crc32c("Offhand Hurling"));
  add_wts(crc32c("Offhand Hurling"));

  // Skill Definition: Offhand Laser Pistols
  skcat["Quickness-Based Skills"].push_back(crc32c("Offhand Laser Pistols"));
  skcat["Pistol Skills"].push_back(crc32c("Offhand Laser Pistols"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Offhand Laser Pistols"));
  skcat["Modern Skills"].push_back(crc32c("Offhand Laser Pistols"));
  skcat["Shadowrun Skills"].push_back(crc32c("Offhand Laser Pistols"));
  skcat["Skills"].push_back(crc32c("Offhand Laser Pistols"));
  add_wts(crc32c("Offhand Laser Pistols"));

  // Skill Definition: Offhand Machine Pistols
  skcat["Quickness-Based Skills"].push_back(crc32c("Offhand Machine Pistols"));
  skcat["Pistol Skills"].push_back(crc32c("Offhand Machine Pistols"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Offhand Machine Pistols"));
  skcat["Modern Skills"].push_back(crc32c("Offhand Machine Pistols"));
  skcat["Shadowrun Skills"].push_back(crc32c("Offhand Machine Pistols"));
  skcat["Skills"].push_back(crc32c("Offhand Machine Pistols"));
  add_wts(crc32c("Offhand Machine Pistols"));

  // Skill Definition: Offhand Piercing
  skcat["Quickness-Based Skills"].push_back(crc32c("Offhand Piercing"));
  skcat["Melee-Combat Skills"].push_back(crc32c("Offhand Piercing"));
  skcat["Medieval Skills"].push_back(crc32c("Offhand Piercing"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Offhand Piercing"));
  skcat["Modern Skills"].push_back(crc32c("Offhand Piercing"));
  skcat["Shadowrun Skills"].push_back(crc32c("Offhand Piercing"));
  skcat["Skills"].push_back(crc32c("Offhand Piercing"));
  add_wts(crc32c("Offhand Piercing"));

  // Skill Definition: Offhand Pistols
  skcat["Quickness-Based Skills"].push_back(crc32c("Offhand Pistols"));
  skcat["Pistol Skills"].push_back(crc32c("Offhand Pistols"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Offhand Pistols"));
  skcat["Modern Skills"].push_back(crc32c("Offhand Pistols"));
  skcat["Shadowrun Skills"].push_back(crc32c("Offhand Pistols"));
  skcat["Skills"].push_back(crc32c("Offhand Pistols"));
  add_wts(crc32c("Offhand Pistols"));

  // Skill Definition: Offhand Plasma Pistols
  skcat["Quickness-Based Skills"].push_back(crc32c("Offhand Plasma Pistols"));
  skcat["Pistol Skills"].push_back(crc32c("Offhand Plasma Pistols"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Offhand Plasma Pistols"));
  skcat["Modern Skills"].push_back(crc32c("Offhand Plasma Pistols"));
  skcat["Shadowrun Skills"].push_back(crc32c("Offhand Plasma Pistols"));
  skcat["Skills"].push_back(crc32c("Offhand Plasma Pistols"));
  add_wts(crc32c("Offhand Plasma Pistols"));

  // Skill Definition: Offhand Punching
  skcat["Quickness-Based Skills"].push_back(crc32c("Offhand Punching"));
  skcat["Medieval Skills"].push_back(crc32c("Offhand Punching"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Offhand Punching"));
  skcat["Modern Skills"].push_back(crc32c("Offhand Punching"));
  skcat["Shadowrun Skills"].push_back(crc32c("Offhand Punching"));
  skcat["Skills"].push_back(crc32c("Offhand Punching"));

  // Skill Definition: Offhand Shot Pistols
  skcat["Quickness-Based Skills"].push_back(crc32c("Offhand Shot Pistols"));
  skcat["Pistol Skills"].push_back(crc32c("Offhand Shot Pistols"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Offhand Shot Pistols"));
  skcat["Modern Skills"].push_back(crc32c("Offhand Shot Pistols"));
  skcat["Shadowrun Skills"].push_back(crc32c("Offhand Shot Pistols"));
  skcat["Skills"].push_back(crc32c("Offhand Shot Pistols"));
  add_wts(crc32c("Offhand Shot Pistols"));

  // Skill Definition: Offhand SMGs
  skcat["Quickness-Based Skills"].push_back(crc32c("Offhand SMGs"));
  skcat["Pistol Skills"].push_back(crc32c("Offhand SMGs"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Offhand SMGs"));
  skcat["Modern Skills"].push_back(crc32c("Offhand SMGs"));
  skcat["Shadowrun Skills"].push_back(crc32c("Offhand SMGs"));
  skcat["Skills"].push_back(crc32c("Offhand SMGs"));
  add_wts(crc32c("Offhand SMGs"));

  // Skill Definition: Offhand Staves
  skcat["Quickness-Based Skills"].push_back(crc32c("Offhand Staves"));
  skcat["Melee-Combat Skills"].push_back(crc32c("Offhand Staves"));
  skcat["Medieval Skills"].push_back(crc32c("Offhand Staves"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Offhand Staves"));
  skcat["Modern Skills"].push_back(crc32c("Offhand Staves"));
  skcat["Shadowrun Skills"].push_back(crc32c("Offhand Staves"));
  skcat["Skills"].push_back(crc32c("Offhand Staves"));
  add_wts(crc32c("Offhand Staves"));

  // Skill Definition: Offhand Throwing, Aero
  skcat["Quickness-Based Skills"].push_back(crc32c("Offhand Throwing, Aero"));
  skcat["Ranged-Combat Skills"].push_back(crc32c("Offhand Throwing, Aero"));
  skcat["Medieval Skills"].push_back(crc32c("Offhand Throwing, Aero"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Offhand Throwing, Aero"));
  skcat["Modern Skills"].push_back(crc32c("Offhand Throwing, Aero"));
  skcat["Shadowrun Skills"].push_back(crc32c("Offhand Throwing, Aero"));
  skcat["Skills"].push_back(crc32c("Offhand Throwing, Aero"));
  add_wts(crc32c("Offhand Throwing, Aero"));

  // Skill Definition: Offhand Throwing, Non-Aero
  skcat["Quickness-Based Skills"].push_back(crc32c("Offhand Throwing, Non-Aero"));
  skcat["Ranged-Combat Skills"].push_back(crc32c("Offhand Throwing, Non-Aero"));
  skcat["Medieval Skills"].push_back(crc32c("Offhand Throwing, Non-Aero"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Offhand Throwing, Non-Aero"));
  skcat["Modern Skills"].push_back(crc32c("Offhand Throwing, Non-Aero"));
  skcat["Shadowrun Skills"].push_back(crc32c("Offhand Throwing, Non-Aero"));
  skcat["Skills"].push_back(crc32c("Offhand Throwing, Non-Aero"));
  add_wts(crc32c("Offhand Throwing, Non-Aero"));

  // Skill Definition: Offhand Whips
  skcat["Quickness-Based Skills"].push_back(crc32c("Offhand Whips"));
  skcat["Ranged-Combat Skills"].push_back(crc32c("Offhand Whips"));
  skcat["Medieval Skills"].push_back(crc32c("Offhand Whips"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Offhand Whips"));
  skcat["Modern Skills"].push_back(crc32c("Offhand Whips"));
  skcat["Shadowrun Skills"].push_back(crc32c("Offhand Whips"));
  skcat["Skills"].push_back(crc32c("Offhand Whips"));
  add_wts(crc32c("Offhand Whips"));

  // Skill Definition: Perception
  skcat["Intelligence-Based Skills"].push_back(crc32c("Perception"));
  skcat["Specialized Skills"].push_back(crc32c("Perception"));
  skcat["Medieval Skills"].push_back(crc32c("Perception"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Perception"));
  skcat["Modern Skills"].push_back(crc32c("Perception"));
  skcat["Shadowrun Skills"].push_back(crc32c("Perception"));
  skcat["Expert Skills"].push_back(crc32c("Perception"));
  skcat["Skills"].push_back(crc32c("Perception"));

  // Skill Definition: Performance
  skcat["Charisma-Based Skills"].push_back(crc32c("Performance"));
  skcat["Social Skills"].push_back(crc32c("Performance"));
  skcat["Medieval Skills"].push_back(crc32c("Performance"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Performance"));
  skcat["Modern Skills"].push_back(crc32c("Performance"));
  skcat["Shadowrun Skills"].push_back(crc32c("Performance"));
  skcat["Skills"].push_back(crc32c("Performance"));

  // Skill Definition: Pistols
  skcat["Quickness-Based Skills"].push_back(crc32c("Pistols"));
  skcat["Pistol Skills"].push_back(crc32c("Pistols"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Pistols"));
  skcat["Modern Skills"].push_back(crc32c("Pistols"));
  skcat["Shadowrun Skills"].push_back(crc32c("Pistols"));
  skcat["Skills"].push_back(crc32c("Pistols"));
  add_wts(crc32c("Pistols"));

  // Skill Definition: Plasma Cannons
  skcat["Strength-Based Skills"].push_back(crc32c("Plasma Cannons"));
  skcat["Heavy Firearm Skills"].push_back(crc32c("Plasma Cannons"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Plasma Cannons"));
  skcat["Modern Skills"].push_back(crc32c("Plasma Cannons"));
  skcat["Shadowrun Skills"].push_back(crc32c("Plasma Cannons"));
  skcat["Skills"].push_back(crc32c("Plasma Cannons"));

  // Skill Definition: Plasma Pistols
  skcat["Quickness-Based Skills"].push_back(crc32c("Plasma Pistols"));
  skcat["Pistol Skills"].push_back(crc32c("Plasma Pistols"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Plasma Pistols"));
  skcat["Modern Skills"].push_back(crc32c("Plasma Pistols"));
  skcat["Shadowrun Skills"].push_back(crc32c("Plasma Pistols"));
  skcat["Skills"].push_back(crc32c("Plasma Pistols"));
  add_wts(crc32c("Plasma Pistols"));

  // Skill Definition: Plasma Rifles
  skcat["Quickness-Based Skills"].push_back(crc32c("Plasma Rifles"));
  skcat["Rifle Skills"].push_back(crc32c("Plasma Rifles"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Plasma Rifles"));
  skcat["Modern Skills"].push_back(crc32c("Plasma Rifles"));
  skcat["Shadowrun Skills"].push_back(crc32c("Plasma Rifles"));
  skcat["Skills"].push_back(crc32c("Plasma Rifles"));
  add_wts(crc32c("Plasma Rifles"));

  // Skill Definition: Power Distribution
  skcat["Intelligence-Based Skills"].push_back(crc32c("Power Distribution"));
  skcat["Technical Skills"].push_back(crc32c("Power Distribution"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Power Distribution"));
  skcat["Modern Skills"].push_back(crc32c("Power Distribution"));
  skcat["Shadowrun Skills"].push_back(crc32c("Power Distribution"));
  skcat["Skills"].push_back(crc32c("Power Distribution"));

  // Skill Definition: Power Suit
  skcat["Reaction-Based Skills"].push_back(crc32c("Power Suit"));
  skcat["Piloting Skills"].push_back(crc32c("Power Suit"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Power Suit"));
  skcat["Shadowrun Skills"].push_back(crc32c("Power Suit"));
  skcat["Skills"].push_back(crc32c("Power Suit"));

  // Skill Definition: Prepared Simple
  skcat["Willpower-Based Skills"].push_back(crc32c("Prepared Simple"));
  skcat["Star Magical Skills"].push_back(crc32c("Prepared Simple"));
  skcat["Medieval Skills"].push_back(crc32c("Prepared Simple"));
  skcat["Skills"].push_back(crc32c("Prepared Simple"));

  // Skill Definition: Punching
  skcat["Quickness-Based Skills"].push_back(crc32c("Punching"));
  skcat["Hand-to-Hand Combat Skills"].push_back(crc32c("Punching"));
  skcat["Medieval Skills"].push_back(crc32c("Punching"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Punching"));
  skcat["Modern Skills"].push_back(crc32c("Punching"));
  skcat["Shadowrun Skills"].push_back(crc32c("Punching"));
  skcat["Skills"].push_back(crc32c("Punching"));
  add_wts(crc32c("Punching"));

  // Skill Definition: Quickdraw
  skcat["Quickness-Based Skills"].push_back(crc32c("Quickdraw"));
  skcat["Specialized Skills"].push_back(crc32c("Quickdraw"));
  skcat["Medieval Skills"].push_back(crc32c("Quickdraw"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Quickdraw"));
  skcat["Modern Skills"].push_back(crc32c("Quickdraw"));
  skcat["Shadowrun Skills"].push_back(crc32c("Quickdraw"));
  skcat["Skills"].push_back(crc32c("Quickdraw"));

  // Skill Definition: Race Car
  skcat["Reaction-Based Skills"].push_back(crc32c("Race Car"));
  skcat["Piloting Skills"].push_back(crc32c("Race Car"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Race Car"));
  skcat["Modern Skills"].push_back(crc32c("Race Car"));
  skcat["Shadowrun Skills"].push_back(crc32c("Race Car"));
  skcat["Skills"].push_back(crc32c("Race Car"));

  // Skill Definition: Refine Juju
  skcat["Charisma-Based Skills"].push_back(crc32c("Refine Juju"));
  skcat["Shamanistic Skills"].push_back(crc32c("Refine Juju"));
  skcat["Medieval Skills"].push_back(crc32c("Refine Juju"));
  skcat["Skills"].push_back(crc32c("Refine Juju"));

  // Skill Definition: Research
  skcat["Intelligence-Based Skills"].push_back(crc32c("Research"));
  skcat["Specialized Skills"].push_back(crc32c("Research"));
  skcat["Medieval Skills"].push_back(crc32c("Research"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Research"));
  skcat["Modern Skills"].push_back(crc32c("Research"));
  skcat["Shadowrun Skills"].push_back(crc32c("Research"));
  skcat["Specialty Skills"].push_back(crc32c("Research"));
  skcat["Skills"].push_back(crc32c("Research"));

  // Skill Definition: Riding
  skcat["Quickness-Based Skills"].push_back(crc32c("Riding"));
  skcat["Athletic Skills"].push_back(crc32c("Riding"));
  skcat["Medieval Skills"].push_back(crc32c("Riding"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Riding"));
  skcat["Modern Skills"].push_back(crc32c("Riding"));
  skcat["Shadowrun Skills"].push_back(crc32c("Riding"));
  skcat["Skills"].push_back(crc32c("Riding"));

  // Skill Definition: Rifles
  skcat["Quickness-Based Skills"].push_back(crc32c("Rifles"));
  skcat["Rifle Skills"].push_back(crc32c("Rifles"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Rifles"));
  skcat["Modern Skills"].push_back(crc32c("Rifles"));
  skcat["Shadowrun Skills"].push_back(crc32c("Rifles"));
  skcat["Skills"].push_back(crc32c("Rifles"));
  add_wts(crc32c("Rifles"));

  // Skill Definition: Ritual Conjuring
  skcat["Willpower-Based Skills"].push_back(crc32c("Ritual Conjuring"));
  skcat["Magical Skills"].push_back(crc32c("Ritual Conjuring"));
  skcat["Medieval Skills"].push_back(crc32c("Ritual Conjuring"));
  skcat["Shadowrun Skills"].push_back(crc32c("Ritual Conjuring"));
  skcat["Skills"].push_back(crc32c("Ritual Conjuring"));

  // Skill Definition: Ritual Spellcasting
  skcat["Willpower-Based Skills"].push_back(crc32c("Ritual Spellcasting"));
  skcat["Magical Skills"].push_back(crc32c("Ritual Spellcasting"));
  skcat["Medieval Skills"].push_back(crc32c("Ritual Spellcasting"));
  skcat["Shadowrun Skills"].push_back(crc32c("Ritual Spellcasting"));
  skcat["Skills"].push_back(crc32c("Ritual Spellcasting"));

  // Skill Definition: Rod Targeting
  skcat["Quickness-Based Skills"].push_back(crc32c("Rod Targeting"));
  skcat["Magical Skills"].push_back(crc32c("Rod Targeting"));
  skcat["Medieval Skills"].push_back(crc32c("Rod Targeting"));
  skcat["Skills"].push_back(crc32c("Rod Targeting"));

  // Skill Definition: Rotorcraft
  skcat["Reaction-Based Skills"].push_back(crc32c("Rotorcraft"));
  skcat["Piloting Skills"].push_back(crc32c("Rotorcraft"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Rotorcraft"));
  skcat["Modern Skills"].push_back(crc32c("Rotorcraft"));
  skcat["Shadowrun Skills"].push_back(crc32c("Rotorcraft"));
  skcat["Skills"].push_back(crc32c("Rotorcraft"));

  // Skill Definition: Rotor Drone
  skcat["Intelligence-Based Skills"].push_back(crc32c("Rotor Drone"));
  skcat["Drone Piloting Skills"].push_back(crc32c("Rotor Drone"));
  skcat["Shadowrun Skills"].push_back(crc32c("Rotor Drone"));
  skcat["Skills"].push_back(crc32c("Rotor Drone"));

  // Skill Definition: Running
  skcat["Strength-Based Skills"].push_back(crc32c("Running"));
  skcat["Athletic Skills"].push_back(crc32c("Running"));
  skcat["Medieval Skills"].push_back(crc32c("Running"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Running"));
  skcat["Modern Skills"].push_back(crc32c("Running"));
  skcat["Shadowrun Skills"].push_back(crc32c("Running"));
  skcat["Skills"].push_back(crc32c("Running"));

  // Skill Definition: Saurian Line Attacking
  skcat["Strength-Based Skills"].push_back(crc32c("Saurian Line Attacking"));
  skcat["Star Martial Arts"].push_back(crc32c("Saurian Line Attacking"));
  skcat["Medieval Skills"].push_back(crc32c("Saurian Line Attacking"));
  skcat["Skills"].push_back(crc32c("Saurian Line Attacking"));

  // Skill Definition: Security
  skcat["Intelligence-Based Skills"].push_back(crc32c("Security"));
  skcat["Technical Skills"].push_back(crc32c("Security"));
  skcat["Medieval Skills"].push_back(crc32c("Security"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Security"));
  skcat["Modern Skills"].push_back(crc32c("Security"));
  skcat["Shadowrun Skills"].push_back(crc32c("Security"));
  skcat["Skills"].push_back(crc32c("Security"));

  // Skill Definition: Seduction
  skcat["Charisma-Based Skills"].push_back(crc32c("Seduction"));
  skcat["Social Skills"].push_back(crc32c("Seduction"));
  skcat["Medieval Skills"].push_back(crc32c("Seduction"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Seduction"));
  skcat["Modern Skills"].push_back(crc32c("Seduction"));
  skcat["Shadowrun Skills"].push_back(crc32c("Seduction"));
  skcat["Skills"].push_back(crc32c("Seduction"));

  // Skill Definition: Self Artificing
  skcat["Charisma-Based Skills"].push_back(crc32c("Self Artificing"));
  skcat["Magical Skills"].push_back(crc32c("Self Artificing"));
  skcat["Medieval Skills"].push_back(crc32c("Self Artificing"));
  skcat["Skills"].push_back(crc32c("Self Artificing"));

  // Skill Definition: Sensors
  skcat["Intelligence-Based Skills"].push_back(crc32c("Sensors"));
  skcat["Technical Skills"].push_back(crc32c("Sensors"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Sensors"));
  skcat["Modern Skills"].push_back(crc32c("Sensors"));
  skcat["Shadowrun Skills"].push_back(crc32c("Sensors"));
  skcat["Skills"].push_back(crc32c("Sensors"));

  // Skill Definition: Shields
  skcat["Strength-Based Skills"].push_back(crc32c("Shields"));
  skcat["Melee-Combat Skills"].push_back(crc32c("Shields"));
  skcat["Medieval Skills"].push_back(crc32c("Shields"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Shields"));
  skcat["Modern Skills"].push_back(crc32c("Shields"));
  skcat["Shadowrun Skills"].push_back(crc32c("Shields"));
  skcat["Skills"].push_back(crc32c("Shields"));
  add_wts(crc32c("Shields"));

  // Skill Definition: Ship
  skcat["Quickness-Based Skills"].push_back(crc32c("Ship"));
  skcat["Piloting Skills"].push_back(crc32c("Ship"));
  skcat["Medieval Skills"].push_back(crc32c("Ship"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Ship"));
  skcat["Modern Skills"].push_back(crc32c("Ship"));
  skcat["Shadowrun Skills"].push_back(crc32c("Ship"));
  skcat["Skills"].push_back(crc32c("Ship"));

  // Skill Definition: Ship, Powered
  skcat["Intelligence-Based Skills"].push_back(crc32c("Ship, Powered"));
  skcat["Piloting Skills"].push_back(crc32c("Ship, Powered"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Ship, Powered"));
  skcat["Modern Skills"].push_back(crc32c("Ship, Powered"));
  skcat["Shadowrun Skills"].push_back(crc32c("Ship, Powered"));
  skcat["Skills"].push_back(crc32c("Ship, Powered"));

  // Skill Definition: Shipwright
  skcat["Intelligence-Based Skills"].push_back(crc32c("Shipwright"));
  skcat["Creation Skills"].push_back(crc32c("Shipwright"));
  skcat["Medieval Skills"].push_back(crc32c("Shipwright"));
  skcat["Skills"].push_back(crc32c("Shipwright"));

  // Skill Definition: Short Blades
  skcat["Quickness-Based Skills"].push_back(crc32c("Short Blades"));
  skcat["Melee-Combat Skills"].push_back(crc32c("Short Blades"));
  skcat["Medieval Skills"].push_back(crc32c("Short Blades"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Short Blades"));
  skcat["Modern Skills"].push_back(crc32c("Short Blades"));
  skcat["Shadowrun Skills"].push_back(crc32c("Short Blades"));
  skcat["Skills"].push_back(crc32c("Short Blades"));
  add_wts(crc32c("Short Blades"));

  // Skill Definition: Short Cleaves
  skcat["Quickness-Based Skills"].push_back(crc32c("Short Cleaves"));
  skcat["Melee-Combat Skills"].push_back(crc32c("Short Cleaves"));
  skcat["Medieval Skills"].push_back(crc32c("Short Cleaves"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Short Cleaves"));
  skcat["Modern Skills"].push_back(crc32c("Short Cleaves"));
  skcat["Shadowrun Skills"].push_back(crc32c("Short Cleaves"));
  skcat["Skills"].push_back(crc32c("Short Cleaves"));
  add_wts(crc32c("Short Cleaves"));

  // Skill Definition: Short Crushing
  skcat["Quickness-Based Skills"].push_back(crc32c("Short Crushing"));
  skcat["Melee-Combat Skills"].push_back(crc32c("Short Crushing"));
  skcat["Medieval Skills"].push_back(crc32c("Short Crushing"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Short Crushing"));
  skcat["Modern Skills"].push_back(crc32c("Short Crushing"));
  skcat["Shadowrun Skills"].push_back(crc32c("Short Crushing"));
  skcat["Skills"].push_back(crc32c("Short Crushing"));
  add_wts(crc32c("Short Crushing"));

  // Skill Definition: Short Flails
  skcat["Quickness-Based Skills"].push_back(crc32c("Short Flails"));
  skcat["Melee-Combat Skills"].push_back(crc32c("Short Flails"));
  skcat["Medieval Skills"].push_back(crc32c("Short Flails"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Short Flails"));
  skcat["Modern Skills"].push_back(crc32c("Short Flails"));
  skcat["Shadowrun Skills"].push_back(crc32c("Short Flails"));
  skcat["Skills"].push_back(crc32c("Short Flails"));
  add_wts(crc32c("Short Flails"));

  // Skill Definition: Short Piercing
  skcat["Quickness-Based Skills"].push_back(crc32c("Short Piercing"));
  skcat["Melee-Combat Skills"].push_back(crc32c("Short Piercing"));
  skcat["Medieval Skills"].push_back(crc32c("Short Piercing"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Short Piercing"));
  skcat["Modern Skills"].push_back(crc32c("Short Piercing"));
  skcat["Shadowrun Skills"].push_back(crc32c("Short Piercing"));
  skcat["Skills"].push_back(crc32c("Short Piercing"));
  add_wts(crc32c("Short Piercing"));

  // Skill Definition: Short Staves
  skcat["Quickness-Based Skills"].push_back(crc32c("Short Staves"));
  skcat["Melee-Combat Skills"].push_back(crc32c("Short Staves"));
  skcat["Medieval Skills"].push_back(crc32c("Short Staves"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Short Staves"));
  skcat["Modern Skills"].push_back(crc32c("Short Staves"));
  skcat["Shadowrun Skills"].push_back(crc32c("Short Staves"));
  skcat["Skills"].push_back(crc32c("Short Staves"));
  add_wts(crc32c("Short Staves"));

  // Skill Definition: Shotguns
  skcat["Quickness-Based Skills"].push_back(crc32c("Shotguns"));
  skcat["Rifle Skills"].push_back(crc32c("Shotguns"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Shotguns"));
  skcat["Modern Skills"].push_back(crc32c("Shotguns"));
  skcat["Shadowrun Skills"].push_back(crc32c("Shotguns"));
  skcat["Skills"].push_back(crc32c("Shotguns"));
  add_wts(crc32c("Shotguns"));

  // Skill Definition: Shot Pistols
  skcat["Quickness-Based Skills"].push_back(crc32c("Shot Pistols"));
  skcat["Pistol Skills"].push_back(crc32c("Shot Pistols"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Shot Pistols"));
  skcat["Modern Skills"].push_back(crc32c("Shot Pistols"));
  skcat["Shadowrun Skills"].push_back(crc32c("Shot Pistols"));
  skcat["Skills"].push_back(crc32c("Shot Pistols"));
  add_wts(crc32c("Shot Pistols"));

  // Skill Definition: Skiing
  skcat["Quickness-Based Skills"].push_back(crc32c("Skiing"));
  skcat["Athletic Skills"].push_back(crc32c("Skiing"));
  skcat["Medieval Skills"].push_back(crc32c("Skiing"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Skiing"));
  skcat["Modern Skills"].push_back(crc32c("Skiing"));
  skcat["Shadowrun Skills"].push_back(crc32c("Skiing"));
  skcat["Skills"].push_back(crc32c("Skiing"));

  // Skill Definition: Sled
  skcat["Quickness-Based Skills"].push_back(crc32c("Sled"));
  skcat["Piloting Skills"].push_back(crc32c("Sled"));
  skcat["Medieval Skills"].push_back(crc32c("Sled"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Sled"));
  skcat["Modern Skills"].push_back(crc32c("Sled"));
  skcat["Shadowrun Skills"].push_back(crc32c("Sled"));
  skcat["Skills"].push_back(crc32c("Sled"));

  // Skill Definition: Slings
  skcat["Quickness-Based Skills"].push_back(crc32c("Slings"));
  skcat["Ranged-Combat Skills"].push_back(crc32c("Slings"));
  skcat["Medieval Skills"].push_back(crc32c("Slings"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Slings"));
  skcat["Modern Skills"].push_back(crc32c("Slings"));
  skcat["Shadowrun Skills"].push_back(crc32c("Slings"));
  skcat["Skills"].push_back(crc32c("Slings"));
  add_wts(crc32c("Slings"));

  // Skill Definition: Smelting
  skcat["Intelligence-Based Skills"].push_back(crc32c("Smelting"));
  skcat["Creation Skills"].push_back(crc32c("Smelting"));
  skcat["Medieval Skills"].push_back(crc32c("Smelting"));
  skcat["Skills"].push_back(crc32c("Smelting"));

  // Skill Definition: SMGs
  skcat["Quickness-Based Skills"].push_back(crc32c("SMGs"));
  skcat["Pistol Skills"].push_back(crc32c("SMGs"));
  skcat["Cyberpunk Skills"].push_back(crc32c("SMGs"));
  skcat["Modern Skills"].push_back(crc32c("SMGs"));
  skcat["Shadowrun Skills"].push_back(crc32c("SMGs"));
  skcat["Skills"].push_back(crc32c("SMGs"));
  add_wts(crc32c("SMGs"));

  // Skill Definition: Snowmobile
  skcat["Reaction-Based Skills"].push_back(crc32c("Snowmobile"));
  skcat["Piloting Skills"].push_back(crc32c("Snowmobile"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Snowmobile"));
  skcat["Modern Skills"].push_back(crc32c("Snowmobile"));
  skcat["Shadowrun Skills"].push_back(crc32c("Snowmobile"));
  skcat["Skills"].push_back(crc32c("Snowmobile"));

  // Skill Definition: Spacecraft
  skcat["Intelligence-Based Skills"].push_back(crc32c("Spacecraft"));
  skcat["Piloting Skills"].push_back(crc32c("Spacecraft"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Spacecraft"));
  skcat["Modern Skills"].push_back(crc32c("Spacecraft"));
  skcat["Shadowrun Skills"].push_back(crc32c("Spacecraft"));
  skcat["Skills"].push_back(crc32c("Spacecraft"));

  // Skill Definition: Spellcasting
  skcat["Charisma-Based Skills"].push_back(crc32c("Spellcasting"));
  skcat["Magical Skills"].push_back(crc32c("Spellcasting"));
  skcat["Medieval Skills"].push_back(crc32c("Spellcasting"));
  skcat["Shadowrun Skills"].push_back(crc32c("Spellcasting"));
  skcat["Skills"].push_back(crc32c("Spellcasting"));

  // Skill Definition: Spellcraft
  skcat["Intelligence-Based Skills"].push_back(crc32c("Spellcraft"));
  skcat["Magical Skills"].push_back(crc32c("Spellcraft"));
  skcat["Medieval Skills"].push_back(crc32c("Spellcraft"));
  skcat["Shadowrun Skills"].push_back(crc32c("Spellcraft"));
  skcat["Skills"].push_back(crc32c("Spellcraft"));

  // Skill Definition: Spellcraft, Star
  skcat["Intelligence-Based Skills"].push_back(crc32c("Spellcraft, Star"));
  skcat["Star Magical Skills"].push_back(crc32c("Spellcraft, Star"));
  skcat["Medieval Skills"].push_back(crc32c("Spellcraft, Star"));
  skcat["Skills"].push_back(crc32c("Spellcraft, Star"));

  // Skill Definition: Spell Preparation
  skcat["Intelligence-Based Skills"].push_back(crc32c("Spell Preparation"));
  skcat["Star Magical Skills"].push_back(crc32c("Spell Preparation"));
  skcat["Medieval Skills"].push_back(crc32c("Spell Preparation"));
  skcat["Skills"].push_back(crc32c("Spell Preparation"));

  // Skill Definition: Spell Targeting
  skcat["Quickness-Based Skills"].push_back(crc32c("Spell Targeting"));
  skcat["Magical Skills"].push_back(crc32c("Spell Targeting"));
  skcat["Medieval Skills"].push_back(crc32c("Spell Targeting"));
  skcat["Shadowrun Skills"].push_back(crc32c("Spell Targeting"));
  skcat["Skills"].push_back(crc32c("Spell Targeting"));

  // Skill Definition: Sprinting
  skcat["Quickness-Based Skills"].push_back(crc32c("Sprinting"));
  skcat["Athletic Skills"].push_back(crc32c("Sprinting"));
  skcat["Medieval Skills"].push_back(crc32c("Sprinting"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Sprinting"));
  skcat["Modern Skills"].push_back(crc32c("Sprinting"));
  skcat["Shadowrun Skills"].push_back(crc32c("Sprinting"));
  skcat["Skills"].push_back(crc32c("Sprinting"));

  // Skill Definition: Staff Slings
  skcat["Quickness-Based Skills"].push_back(crc32c("Staff Slings"));
  skcat["Ranged-Combat Skills"].push_back(crc32c("Staff Slings"));
  skcat["Medieval Skills"].push_back(crc32c("Staff Slings"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Staff Slings"));
  skcat["Modern Skills"].push_back(crc32c("Staff Slings"));
  skcat["Shadowrun Skills"].push_back(crc32c("Staff Slings"));
  skcat["Skills"].push_back(crc32c("Staff Slings"));
  add_wts(crc32c("Staff Slings"));

  // Skill Definition: Staff Targeting
  skcat["Quickness-Based Skills"].push_back(crc32c("Staff Targeting"));
  skcat["Magical Skills"].push_back(crc32c("Staff Targeting"));
  skcat["Medieval Skills"].push_back(crc32c("Staff Targeting"));
  skcat["Skills"].push_back(crc32c("Staff Targeting"));

  // Skill Definition: Stealth
  skcat["Quickness-Based Skills"].push_back(crc32c("Stealth"));
  skcat["Athletic Skills"].push_back(crc32c("Stealth"));
  skcat["Medieval Skills"].push_back(crc32c("Stealth"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Stealth"));
  skcat["Modern Skills"].push_back(crc32c("Stealth"));
  skcat["Shadowrun Skills"].push_back(crc32c("Stealth"));
  skcat["Skills"].push_back(crc32c("Stealth"));

  // Skill Definition: Style
  skcat["Charisma-Based Skills"].push_back(crc32c("Style"));
  skcat["Social Skills"].push_back(crc32c("Style"));
  skcat["Medieval Skills"].push_back(crc32c("Style"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Style"));
  skcat["Modern Skills"].push_back(crc32c("Style"));
  skcat["Shadowrun Skills"].push_back(crc32c("Style"));
  skcat["Skills"].push_back(crc32c("Style"));

  // Skill Definition: Surgery
  skcat["Quickness-Based Skills"].push_back(crc32c("Surgery"));
  skcat["Technical Skills"].push_back(crc32c("Surgery"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Surgery"));
  skcat["Modern Skills"].push_back(crc32c("Surgery"));
  skcat["Shadowrun Skills"].push_back(crc32c("Surgery"));
  skcat["Skills"].push_back(crc32c("Surgery"));

  // Skill Definition: Survival
  skcat["Intelligence-Based Skills"].push_back(crc32c("Survival"));
  skcat["Nature Skills"].push_back(crc32c("Survival"));
  skcat["Medieval Skills"].push_back(crc32c("Survival"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Survival"));
  skcat["Modern Skills"].push_back(crc32c("Survival"));
  skcat["Shadowrun Skills"].push_back(crc32c("Survival"));
  skcat["Skills"].push_back(crc32c("Survival"));

  // Skill Definition: Swimming
  skcat["Quickness-Based Skills"].push_back(crc32c("Swimming"));
  skcat["Athletic Skills"].push_back(crc32c("Swimming"));
  skcat["Medieval Skills"].push_back(crc32c("Swimming"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Swimming"));
  skcat["Modern Skills"].push_back(crc32c("Swimming"));
  skcat["Shadowrun Skills"].push_back(crc32c("Swimming"));
  skcat["Skills"].push_back(crc32c("Swimming"));

  // Skill Definition: Swordsmithing
  skcat["Intelligence-Based Skills"].push_back(crc32c("Swordsmithing"));
  skcat["Creation Skills"].push_back(crc32c("Swordsmithing"));
  skcat["Medieval Skills"].push_back(crc32c("Swordsmithing"));
  skcat["Skills"].push_back(crc32c("Swordsmithing"));

  // Skill Definition: Tactics
  skcat["Intelligence-Based Skills"].push_back(crc32c("Tactics"));
  skcat["Specialized Skills"].push_back(crc32c("Tactics"));
  skcat["Medieval Skills"].push_back(crc32c("Tactics"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Tactics"));
  skcat["Modern Skills"].push_back(crc32c("Tactics"));
  skcat["Shadowrun Skills"].push_back(crc32c("Tactics"));
  skcat["Skills"].push_back(crc32c("Tactics"));

  // Skill Definition: Talismongery
  skcat["Willpower-Based Skills"].push_back(crc32c("Talismongery"));
  skcat["Magical Skills"].push_back(crc32c("Talismongery"));
  skcat["Medieval Skills"].push_back(crc32c("Talismongery"));
  skcat["Shadowrun Skills"].push_back(crc32c("Talismongery"));
  skcat["Skills"].push_back(crc32c("Talismongery"));

  // Skill Definition: Teamster
  skcat["Charisma-Based Skills"].push_back(crc32c("Teamster"));
  skcat["Piloting Skills"].push_back(crc32c("Teamster"));
  skcat["Medieval Skills"].push_back(crc32c("Teamster"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Teamster"));
  skcat["Modern Skills"].push_back(crc32c("Teamster"));
  skcat["Shadowrun Skills"].push_back(crc32c("Teamster"));
  skcat["Skills"].push_back(crc32c("Teamster"));

  // Skill Definition: Throwing, Aero
  skcat["Quickness-Based Skills"].push_back(crc32c("Throwing, Aero"));
  skcat["Ranged-Combat Skills"].push_back(crc32c("Throwing, Aero"));
  skcat["Medieval Skills"].push_back(crc32c("Throwing, Aero"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Throwing, Aero"));
  skcat["Modern Skills"].push_back(crc32c("Throwing, Aero"));
  skcat["Shadowrun Skills"].push_back(crc32c("Throwing, Aero"));
  skcat["Skills"].push_back(crc32c("Throwing, Aero"));
  add_wts(crc32c("Throwing, Aero"));

  // Skill Definition: Throwing, Non-Aero
  skcat["Quickness-Based Skills"].push_back(crc32c("Throwing, Non-Aero"));
  skcat["Ranged-Combat Skills"].push_back(crc32c("Throwing, Non-Aero"));
  skcat["Medieval Skills"].push_back(crc32c("Throwing, Non-Aero"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Throwing, Non-Aero"));
  skcat["Modern Skills"].push_back(crc32c("Throwing, Non-Aero"));
  skcat["Shadowrun Skills"].push_back(crc32c("Throwing, Non-Aero"));
  skcat["Skills"].push_back(crc32c("Throwing, Non-Aero"));
  add_wts(crc32c("Throwing, Non-Aero"));

  // Skill Definition: Tracked Drone
  skcat["Intelligence-Based Skills"].push_back(crc32c("Tracked Drone"));
  skcat["Drone Piloting Skills"].push_back(crc32c("Tracked Drone"));
  skcat["Shadowrun Skills"].push_back(crc32c("Tracked Drone"));
  skcat["Skills"].push_back(crc32c("Tracked Drone"));

  // Skill Definition: Tracked Vehicle
  skcat["Reaction-Based Skills"].push_back(crc32c("Tracked Vehicle"));
  skcat["Piloting Skills"].push_back(crc32c("Tracked Vehicle"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Tracked Vehicle"));
  skcat["Modern Skills"].push_back(crc32c("Tracked Vehicle"));
  skcat["Shadowrun Skills"].push_back(crc32c("Tracked Vehicle"));
  skcat["Skills"].push_back(crc32c("Tracked Vehicle"));

  // Skill Definition: Tracking
  skcat["Intelligence-Based Skills"].push_back(crc32c("Tracking"));
  skcat["Specialized Skills"].push_back(crc32c("Tracking"));
  skcat["Medieval Skills"].push_back(crc32c("Tracking"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Tracking"));
  skcat["Modern Skills"].push_back(crc32c("Tracking"));
  skcat["Shadowrun Skills"].push_back(crc32c("Tracking"));
  skcat["Skills"].push_back(crc32c("Tracking"));

  // Skill Definition: Tractor
  skcat["Reaction-Based Skills"].push_back(crc32c("Tractor"));
  skcat["Piloting Skills"].push_back(crc32c("Tractor"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Tractor"));
  skcat["Modern Skills"].push_back(crc32c("Tractor"));
  skcat["Shadowrun Skills"].push_back(crc32c("Tractor"));
  skcat["Skills"].push_back(crc32c("Tractor"));

  // Skill Definition: Treatment
  skcat["Intelligence-Based Skills"].push_back(crc32c("Treatment"));
  skcat["Technical Skills"].push_back(crc32c("Treatment"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Treatment"));
  skcat["Modern Skills"].push_back(crc32c("Treatment"));
  skcat["Shadowrun Skills"].push_back(crc32c("Treatment"));
  skcat["Skills"].push_back(crc32c("Treatment"));

  // Skill Definition: Trike
  skcat["Reaction-Based Skills"].push_back(crc32c("Trike"));
  skcat["Piloting Skills"].push_back(crc32c("Trike"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Trike"));
  skcat["Modern Skills"].push_back(crc32c("Trike"));
  skcat["Shadowrun Skills"].push_back(crc32c("Trike"));
  skcat["Skills"].push_back(crc32c("Trike"));

  // Skill Definition: Two-Handed Blades
  skcat["Strength-Based Skills"].push_back(crc32c("Two-Handed Blades"));
  skcat["Melee-Combat Skills"].push_back(crc32c("Two-Handed Blades"));
  skcat["Medieval Skills"].push_back(crc32c("Two-Handed Blades"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Two-Handed Blades"));
  skcat["Modern Skills"].push_back(crc32c("Two-Handed Blades"));
  skcat["Shadowrun Skills"].push_back(crc32c("Two-Handed Blades"));
  skcat["Skills"].push_back(crc32c("Two-Handed Blades"));
  add_wts(crc32c("Two-Handed Blades"));

  // Skill Definition: Two-Handed Cleaves
  skcat["Strength-Based Skills"].push_back(crc32c("Two-Handed Cleaves"));
  skcat["Melee-Combat Skills"].push_back(crc32c("Two-Handed Cleaves"));
  skcat["Medieval Skills"].push_back(crc32c("Two-Handed Cleaves"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Two-Handed Cleaves"));
  skcat["Modern Skills"].push_back(crc32c("Two-Handed Cleaves"));
  skcat["Shadowrun Skills"].push_back(crc32c("Two-Handed Cleaves"));
  skcat["Skills"].push_back(crc32c("Two-Handed Cleaves"));
  add_wts(crc32c("Two-Handed Cleaves"));

  // Skill Definition: Two-Handed Crushing
  skcat["Strength-Based Skills"].push_back(crc32c("Two-Handed Crushing"));
  skcat["Melee-Combat Skills"].push_back(crc32c("Two-Handed Crushing"));
  skcat["Medieval Skills"].push_back(crc32c("Two-Handed Crushing"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Two-Handed Crushing"));
  skcat["Modern Skills"].push_back(crc32c("Two-Handed Crushing"));
  skcat["Shadowrun Skills"].push_back(crc32c("Two-Handed Crushing"));
  skcat["Skills"].push_back(crc32c("Two-Handed Crushing"));
  add_wts(crc32c("Two-Handed Crushing"));

  // Skill Definition: Two-Handed Flails
  skcat["Strength-Based Skills"].push_back(crc32c("Two-Handed Flails"));
  skcat["Melee-Combat Skills"].push_back(crc32c("Two-Handed Flails"));
  skcat["Medieval Skills"].push_back(crc32c("Two-Handed Flails"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Two-Handed Flails"));
  skcat["Modern Skills"].push_back(crc32c("Two-Handed Flails"));
  skcat["Shadowrun Skills"].push_back(crc32c("Two-Handed Flails"));
  skcat["Skills"].push_back(crc32c("Two-Handed Flails"));
  add_wts(crc32c("Two-Handed Flails"));

  // Skill Definition: Two-Handed Piercing
  skcat["Quickness-Based Skills"].push_back(crc32c("Two-Handed Piercing"));
  skcat["Melee-Combat Skills"].push_back(crc32c("Two-Handed Piercing"));
  skcat["Medieval Skills"].push_back(crc32c("Two-Handed Piercing"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Two-Handed Piercing"));
  skcat["Modern Skills"].push_back(crc32c("Two-Handed Piercing"));
  skcat["Shadowrun Skills"].push_back(crc32c("Two-Handed Piercing"));
  skcat["Skills"].push_back(crc32c("Two-Handed Piercing"));
  add_wts(crc32c("Two-Handed Piercing"));

  // Skill Definition: Two-Handed Staves
  skcat["Quickness-Based Skills"].push_back(crc32c("Two-Handed Staves"));
  skcat["Melee-Combat Skills"].push_back(crc32c("Two-Handed Staves"));
  skcat["Medieval Skills"].push_back(crc32c("Two-Handed Staves"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Two-Handed Staves"));
  skcat["Modern Skills"].push_back(crc32c("Two-Handed Staves"));
  skcat["Shadowrun Skills"].push_back(crc32c("Two-Handed Staves"));
  skcat["Skills"].push_back(crc32c("Two-Handed Staves"));
  add_wts(crc32c("Two-Handed Staves"));

  // Skill Definition: Underwater Combat
  skcat["Intelligence-Based Skills"].push_back(crc32c("Underwater Combat"));
  skcat["Combat Environment Skills"].push_back(crc32c("Underwater Combat"));
  skcat["Medieval Skills"].push_back(crc32c("Underwater Combat"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Underwater Combat"));
  skcat["Modern Skills"].push_back(crc32c("Underwater Combat"));
  skcat["Shadowrun Skills"].push_back(crc32c("Underwater Combat"));
  skcat["Skills"].push_back(crc32c("Underwater Combat"));

  // Skill Definition: Vectored Thrust
  skcat["Reaction-Based Skills"].push_back(crc32c("Vectored Thrust"));
  skcat["Piloting Skills"].push_back(crc32c("Vectored Thrust"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Vectored Thrust"));
  skcat["Modern Skills"].push_back(crc32c("Vectored Thrust"));
  skcat["Shadowrun Skills"].push_back(crc32c("Vectored Thrust"));
  skcat["Skills"].push_back(crc32c("Vectored Thrust"));

  // Skill Definition: Vectored Thrust Drone
  skcat["Intelligence-Based Skills"].push_back(crc32c("Vectored Thrust Drone"));
  skcat["Drone Piloting Skills"].push_back(crc32c("Vectored Thrust Drone"));
  skcat["Shadowrun Skills"].push_back(crc32c("Vectored Thrust Drone"));
  skcat["Skills"].push_back(crc32c("Vectored Thrust Drone"));

  // Skill Definition: Verbal Interface
  skcat["Intelligence-Based Skills"].push_back(crc32c("Verbal Interface"));
  skcat["Technical Skills"].push_back(crc32c("Verbal Interface"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Verbal Interface"));
  skcat["Shadowrun Skills"].push_back(crc32c("Verbal Interface"));
  skcat["Sci-Fi Skills"].push_back(crc32c("Verbal Interface"));
  skcat["Skills"].push_back(crc32c("Verbal Interface"));

  // Skill Definition: Wand Targeting
  skcat["Quickness-Based Skills"].push_back(crc32c("Wand Targeting"));
  skcat["Magical Skills"].push_back(crc32c("Wand Targeting"));
  skcat["Medieval Skills"].push_back(crc32c("Wand Targeting"));
  skcat["Skills"].push_back(crc32c("Wand Targeting"));

  // Skill Definition: Water Weaving
  skcat["Quickness-Based Skills"].push_back(crc32c("Water Weaving"));
  skcat["Magical Skills"].push_back(crc32c("Water Weaving"));
  skcat["Medieval Skills"].push_back(crc32c("Water Weaving"));
  skcat["FP Skills"].push_back(crc32c("Water Weaving"));
  skcat["Skills"].push_back(crc32c("Water Weaving"));

  // Skill Definition: Wheeled Drone
  skcat["Intelligence-Based Skills"].push_back(crc32c("Wheeled Drone"));
  skcat["Drone Piloting Skills"].push_back(crc32c("Wheeled Drone"));
  skcat["Shadowrun Skills"].push_back(crc32c("Wheeled Drone"));
  skcat["Skills"].push_back(crc32c("Wheeled Drone"));

  // Skill Definition: Whips
  skcat["Quickness-Based Skills"].push_back(crc32c("Whips"));
  skcat["Ranged-Combat Skills"].push_back(crc32c("Whips"));
  skcat["Medieval Skills"].push_back(crc32c("Whips"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Whips"));
  skcat["Modern Skills"].push_back(crc32c("Whips"));
  skcat["Shadowrun Skills"].push_back(crc32c("Whips"));
  skcat["Skills"].push_back(crc32c("Whips"));
  add_wts(crc32c("Whips"));

  // Skill Definition: Woodworking
  skcat["Intelligence-Based Skills"].push_back(crc32c("Woodworking"));
  skcat["Creation Skills"].push_back(crc32c("Woodworking"));
  skcat["Medieval Skills"].push_back(crc32c("Woodworking"));
  skcat["Skills"].push_back(crc32c("Woodworking"));

  // Skill Definition: Zero-G Combat
  skcat["Intelligence-Based Skills"].push_back(crc32c("Zero-G Combat"));
  skcat["Combat Environment Skills"].push_back(crc32c("Zero-G Combat"));
  skcat["Medieval Skills"].push_back(crc32c("Zero-G Combat"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Zero-G Combat"));
  skcat["Modern Skills"].push_back(crc32c("Zero-G Combat"));
  skcat["Shadowrun Skills"].push_back(crc32c("Zero-G Combat"));
  skcat["Skills"].push_back(crc32c("Zero-G Combat"));

  // Skill Definition: Zero-G Ops
  skcat["Intelligence-Based Skills"].push_back(crc32c("Zero-G Ops"));
  skcat["Specialized Skills"].push_back(crc32c("Zero-G Ops"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Zero-G Ops"));
  skcat["Modern Skills"].push_back(crc32c("Zero-G Ops"));
  skcat["Shadowrun Skills"].push_back(crc32c("Zero-G Ops"));
  skcat["Skills"].push_back(crc32c("Zero-G Ops"));
}
