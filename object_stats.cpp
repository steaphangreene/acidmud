#include <cstdlib>

#include "object.h"

const int SAVEFILE_VERSION=104;

static map<string,int> defaults;
static map<int,string> weaponskills;
static map<string,int> weapontypes;
static map<string,list<string> > skcat;

static int last_wtype = 0;
static void add_wts(const string &sk) {
  if(defaults.count(sk) == 0) {
    fprintf(stderr,
	"Warning: Tried to link weapon type %d to '%s' which isn't a skill.\n",
	last_wtype+1, sk.c_str());
    return;
    }
  ++last_wtype;
  weaponskills[last_wtype] = sk;
  weapontypes[sk] = last_wtype;
  }

static int defaults_init = 0;
static void init_defaults() {
  if(defaults_init) return;
  defaults_init = 1;
  string cat;

  //The below was generated via a script from the web version
  defaults["Acrobatics"] = 1;
  defaults["Acting"] = 3;
  defaults["Activist Etiquette"] = 3;
  defaults["Aikido"] = 4;
  defaults["Alchemy"] = 4;
  defaults["Amphibious Combat"] = 4;
  defaults["Archery"] = 1;
  defaults["Armoring"] = 4;
  defaults["Assault Rifles"] = 1;
  defaults["Bike"] = 4;
  defaults["Blacksmithing"] = 4;
  defaults["Blowgun"] = 1;
  defaults["Boat"] = 2;
  defaults["Boat, Powered"] = 4;
  defaults["Bowyer"] = 4;
  defaults["Boxing"] = 2;
  defaults["Brawling"] = 2;
  defaults["Car"] = 4;
  defaults["Carpentry"] = 4;
  defaults["Carromeleg - Tier I"] = 0;
  defaults["Carromeleg - Tier II"] = 1;
  defaults["Carromeleg - Tier III"] = 2;
  defaults["Carromeleg - Tier IV"] = 3;
  defaults["Carromeleg - Tier V"] = 4;
  defaults["Carromeleg - Tier VI"] = 5;
  defaults["Catching"] = 1;
  defaults["Climbing"] = 2;
  defaults["Clothier"] = 4;
  defaults["Communications"] = 4;
  defaults["Computer"] = 4;
  defaults["Computer B/R"] = 4;
  defaults["Conjuring"] = 3;
  defaults["Corporate Etiquette"] = 3;
  defaults["Crossbow"] = 1;
  defaults["Dark Elf Etiquette"] = 3;
  defaults["Decker Etiquette"] = 3;
  defaults["Demolitions"] = 4;
  defaults["Diplomacy"] = 3;
  defaults["Diving"] = 1;
  defaults["Dwarf Etiquette"] = 3;
  defaults["Electrical B/R"] = 4;
  defaults["Electronics"] = 4;
  defaults["Enchanting"] = 4;
  defaults["Engineering"] = 4;
  defaults["Farmer Etiquette"] = 3;
  defaults["First Aid"] = 4;
  defaults["Fixed-Wing Drone"] = 4;
  defaults["Fixed-wing Craft"] = 4;
  defaults["Flails"] = 1;
  defaults["Fletcher"] = 4;
  defaults["Gang Etiquette"] = 3;
  defaults["Goblin Etiquette"] = 3;
  defaults["Government Etiquette"] = 3;
  defaults["Grappling"] = 2;
  defaults["Grey Elf Etiquette"] = 3;
  defaults["Gunnery"] = 4;
  defaults["Half-Track"] = 4;
  defaults["Hapkido"] = 1;
  defaults["Healing"] = 4;
  defaults["Heavy Firearms"] = 2;
  defaults["Heavy Lasers"] = 2;
  defaults["Helmsman (Star)"] = 4;
  defaults["High Elf Etiquette"] = 3;
  defaults["High Society Etiquette"] = 3;
  defaults["High-G Combat"] = 4;
  defaults["High-G Ops"] = 4;
  defaults["Hover Drone"] = 4;
  defaults["Hovercraft"] = 4;
  defaults["Human Etiquette"] = 3;
  defaults["Hurling"] = 2;
  defaults["Instruction"] = 3;
  defaults["Interrogation"] = 4;
  defaults["Intimidation"] = 5;
  defaults["Judo"] = 2;
  defaults["Jujitsu"] = 1;
  defaults["Jumping"] = 2;
  defaults["Karate"] = 1;
  defaults["Kicking"] = 1;
  defaults["Knowledge"] = 4;
  defaults["Kung Fu"] = 1;
  defaults["Laser Pistols"] = 1;
  defaults["Laser Rifles"] = 1;
  defaults["Launch Weapons"] = 4;
  defaults["Leadership"] = 3;
  defaults["Leatherworking"] = 4;
  defaults["Lifting"] = 2;
  defaults["Lone Star Etiquette"] = 3;
  defaults["Long Blades"] = 1;
  defaults["Long Cleaves"] = 1;
  defaults["Long Crushing"] = 1;
  defaults["Long Piercing"] = 1;
  defaults["Low-G Combat"] = 4;
  defaults["Low-G Ops"] = 4;
  defaults["Machine Pistols"] = 1;
  defaults["Mage Etiquette"] = 3;
  defaults["Masonry"] = 4;
  defaults["Matrix Etiquette"] = 3;
  defaults["Mechanical B/R"] = 4;
  defaults["Mechanics"] = 4;
  defaults["Mercenary Etiquette"] = 3;
  defaults["Metalworking"] = 4;
  defaults["Military Etiquette"] = 3;
  defaults["Mounted Archery"] = 1;
  defaults["Mounted Blades"] = 1;
  defaults["Mounted Blowgun"] = 1;
  defaults["Mounted Cleaves"] = 1;
  defaults["Mounted Crossbow"] = 1;
  defaults["Mounted Crushing"] = 1;
  defaults["Mounted Flails"] = 1;
  defaults["Mounted Laser Pistols"] = 1;
  defaults["Mounted Machine Pistols"] = 1;
  defaults["Mounted Nets"] = 1;
  defaults["Mounted Piercing"] = 1;
  defaults["Mounted Pistols"] = 1;
  defaults["Mounted SMGs"] = 1;
  defaults["Mounted Shot Pistols"] = 1;
  defaults["Mounted Slings"] = 1;
  defaults["Mounted Throwing"] = 1;
  defaults["Mounted Whips"] = 1;
  defaults["Muay Thai"] = 1;
  defaults["Navigation"] = 4;
  defaults["Negotiation"] = 3;
  defaults["Netiquette"] = 3;
  defaults["Nets"] = 1;
  defaults["Noble Etiquette"] = 3;
  defaults["Offhand Blades"] = 1;
  defaults["Offhand Cleaves"] = 1;
  defaults["Offhand Crushing"] = 1;
  defaults["Offhand Flails"] = 1;
  defaults["Offhand Piercing"] = 1;
  defaults["Offhand Staves"] = 1;
  defaults["Organized Crime Etiquette"] = 3;
  defaults["Ork Etiquette"] = 3;
  defaults["Perception"] = 4;
  defaults["Performance"] = 3;
  defaults["Pistols"] = 1;
  defaults["Police Etiquette"] = 3;
  defaults["Pop Culture Etiquette"] = 3;
  defaults["Punching"] = 1;
  defaults["Quick-draw"] = 1;
  defaults["Religion Etiquette"] = 3;
  defaults["Research"] = 4;
  defaults["Riding"] = 1;
  defaults["Rifles"] = 1;
  defaults["Rigger Etiquette"] = 3;
  defaults["Ritual Conjuring"] = 5;
  defaults["Ritual Spellcasting"] = 5;
  defaults["Rotor Drone"] = 4;
  defaults["Rotorcraft"] = 4;
  defaults["Running"] = 0;
  defaults["SMGs"] = 1;
  defaults["Science Etiquette"] = 3;
  defaults["Security"] = 4;
  defaults["Seduction"] = 3;
  defaults["Sepratist Etiquette"] = 3;
  defaults["Shaman Etiquette"] = 3;
  defaults["Shields"] = 2;
  defaults["Ship"] = 1;
  defaults["Ship, Powered"] = 4;
  defaults["Shipwright"] = 4;
  defaults["Short Blades"] = 1;
  defaults["Short Cleaves"] = 1;
  defaults["Short Crushing"] = 1;
  defaults["Short Piercing"] = 1;
  defaults["Shot Pistols"] = 1;
  defaults["Shotguns"] = 1;
  defaults["Skiing"] = 1;
  defaults["Sled"] = 1;
  defaults["Slings"] = 1;
  defaults["Smelting"] = 4;
  defaults["Snowmobile"] = 4;
  defaults["Spell Targeting"] = 1;
  defaults["Spellcasting"] = 4;
  defaults["Spellcraft"] = 4;
  defaults["Sport Wrestling"] = 2;
  defaults["Sprinting"] = 1;
  defaults["Staff Slings"] = 1;
  defaults["Staff Targeting"] = 1;
  defaults["Staves"] = 1;
  defaults["Stealth"] = 1;
  defaults["Street Etiquette"] = 3;
  defaults["Style"] = 3;
  defaults["Submission Wrestling"] = 2;
  defaults["Sumo-Wrestling"] = 0;
  defaults["Surgery"] = 4;
  defaults["Survival"] = 4;
  defaults["Swimming"] = 1;
  defaults["Tactics"] = 4;
  defaults["Tae Kwon Do"] = 1;
  defaults["Talismongery"] = 5;
  defaults["Teamster"] = 4;
  defaults["Thief Etiquette"] = 3;
  defaults["Throwing (Aero)"] = 1;
  defaults["Throwing (Non-Aero)"] = 1;
  defaults["Tracked Drone"] = 4;
  defaults["Tracked Vehicle"] = 4;
  defaults["Tracking"] = 4;
  defaults["Tractor"] = 4;
  defaults["Treatment"] = 4;
  defaults["Trike"] = 4;
  defaults["Trucker Etiquette"] = 3;
  defaults["Two-Handed Blades"] = 2;
  defaults["Two-Handed Cleaves"] = 2;
  defaults["Two-Handed Crushing"] = 2;
  defaults["Two-Handed Flails"] = 2;
  defaults["Two-Handed Piercing"] = 1;
  defaults["Two-Handed Staves"] = 1;
  defaults["Underwater Combat"] = 4;
  defaults["Underworld Etiquette"] = 3;
  defaults["University Etiquette"] = 3;
  defaults["Vectored Thrust"] = 4;
  defaults["Vectored Thrust Drone"] = 4;
  defaults["Wand Targeting"] = 1;
  defaults["Weaponsmithing"] = 4;
  defaults["Wheeled Drone"] = 4;
  defaults["Whips"] = 1;
  defaults["Wildcat"] = 1;
  defaults["Woodworking"] = 4;
  defaults["Zero-G Combat"] = 4;
  defaults["Zero-G Ops"] = 4;

  cat = "Ranged-Combat Skills";
  skcat[cat].push_back("Archery");
  skcat[cat].push_back("Blowgun");
  skcat[cat].push_back("Crossbow");
  skcat[cat].push_back("Hurling");
  skcat[cat].push_back("Nets");
  skcat[cat].push_back("Slings");
  skcat[cat].push_back("Staff Slings");
  skcat[cat].push_back("Throwing (Aero)");
  skcat[cat].push_back("Throwing (Non-Aero)");
  skcat[cat].push_back("Whips");
  cat = "Pistol Skills";
  skcat[cat].push_back("Laser Pistols");
  skcat[cat].push_back("Machine Pistols");
  skcat[cat].push_back("Pistols");
  skcat[cat].push_back("Shot Pistols");
  skcat[cat].push_back("SMGs");
  cat = "Rifle Skills";
  skcat[cat].push_back("Assault Rifles");
  skcat[cat].push_back("Laser Rifles");
  skcat[cat].push_back("Rifles");
  skcat[cat].push_back("Shotguns");
  cat = "Heavy Firearm Skills";
  skcat[cat].push_back("Gunnery");
  skcat[cat].push_back("Heavy Firearms");
  skcat[cat].push_back("Heavy Lasers");
  skcat[cat].push_back("Launch Weapons");
  cat = "Mounted Ranged-Combat Skills";
  skcat[cat].push_back("Mounted Archery");
  skcat[cat].push_back("Mounted Blowgun");
  skcat[cat].push_back("Mounted Crossbow");
  skcat[cat].push_back("Mounted Nets");
  skcat[cat].push_back("Mounted Slings");
  skcat[cat].push_back("Mounted Throwing");
  skcat[cat].push_back("Mounted Whips");
  cat = "Mounted Pistol Skills";
  skcat[cat].push_back("Mounted Laser Pistols");
  skcat[cat].push_back("Mounted Machine Pistols");
  skcat[cat].push_back("Mounted Pistols");
  skcat[cat].push_back("Mounted Shot Pistols");
  skcat[cat].push_back("Mounted SMGs");
  cat = "Melee-Combat Skills";
  skcat[cat].push_back("Offhand Blades");
  skcat[cat].push_back("Offhand Crushing");
  skcat[cat].push_back("Offhand Cleaves");
  skcat[cat].push_back("Offhand Flails");
  skcat[cat].push_back("Offhand Piercing");
  skcat[cat].push_back("Offhand Staves");
  skcat[cat].push_back("Short Blades");
  skcat[cat].push_back("Short Cleaves");
  skcat[cat].push_back("Short Crushing");
  skcat[cat].push_back("Short Piercing");
  skcat[cat].push_back("Flails");
  skcat[cat].push_back("Staves");
  skcat[cat].push_back("Long Blades");
  skcat[cat].push_back("Long Cleaves");
  skcat[cat].push_back("Long Crushing");
  skcat[cat].push_back("Long Piercing");
  skcat[cat].push_back("Two-Handed Blades");
  skcat[cat].push_back("Two-Handed Crushing");
  skcat[cat].push_back("Two-Handed Cleaves");
  skcat[cat].push_back("Two-Handed Flails");
  skcat[cat].push_back("Two-Handed Piercing");
  skcat[cat].push_back("Two-Handed Staves");
  skcat[cat].push_back("Shields");
  cat = "Mounted Melee-Combat Skills";
  skcat[cat].push_back("Mounted Blades");
  skcat[cat].push_back("Mounted Crushing");
  skcat[cat].push_back("Mounted Cleaves");
  skcat[cat].push_back("Mounted Flails");
  skcat[cat].push_back("Mounted Piercing");
  cat = "Hand-to-Hand Combat Skills";
  skcat[cat].push_back("Grappling");
  skcat[cat].push_back("Kicking");
  skcat[cat].push_back("Punching");
  cat = "Specialized Hand-to-Hand Skills";
  skcat[cat].push_back("Brawling");
  skcat[cat].push_back("Submission Wrestling");
  cat = "Elven Martial Arts";
  skcat[cat].push_back("Carromeleg - Tier I");
  skcat[cat].push_back("Carromeleg - Tier II");
  skcat[cat].push_back("Carromeleg - Tier III");
  skcat[cat].push_back("Carromeleg - Tier IV");
  skcat[cat].push_back("Carromeleg - Tier V");
  skcat[cat].push_back("Carromeleg - Tier VI");
  cat = "Martial Arts";
  skcat[cat].push_back("Aikido");
  skcat[cat].push_back("Boxing");
  skcat[cat].push_back("Hapkido");
  skcat[cat].push_back("Judo");
  skcat[cat].push_back("Jujitsu");
  skcat[cat].push_back("Karate");
  skcat[cat].push_back("Kung Fu");
  skcat[cat].push_back("Muay Thai");
  skcat[cat].push_back("Sport Wrestling");
  skcat[cat].push_back("Sumo-Wrestling");
  skcat[cat].push_back("Tae Kwon Do");
  skcat[cat].push_back("Wildcat");
  cat = "Combat Environment Skills";
  skcat[cat].push_back("Amphibious Combat");
  skcat[cat].push_back("High-G Combat");
  skcat[cat].push_back("Low-G Combat");
  skcat[cat].push_back("Underwater Combat");
  skcat[cat].push_back("Zero-G Combat");
  cat = "Athletic Skills";
  skcat[cat].push_back("Acrobatics");
  skcat[cat].push_back("Catching");
  skcat[cat].push_back("Climbing");
  skcat[cat].push_back("Jumping");
  skcat[cat].push_back("Lifting");
  skcat[cat].push_back("Riding");
  skcat[cat].push_back("Running");
  skcat[cat].push_back("Skiing");
  skcat[cat].push_back("Sprinting");
  skcat[cat].push_back("Stealth");
  skcat[cat].push_back("Swimming");
  skcat[cat].push_back("Diving");
  cat = "Nature Skills";
  skcat[cat].push_back("Healing");
  skcat[cat].push_back("Navigation");
  skcat[cat].push_back("Survival");
  cat = "Specialized Skills";
  skcat[cat].push_back("Knowledge");
  skcat[cat].push_back("Perception");
  skcat[cat].push_back("Mechanics");
  skcat[cat].push_back("Quick-draw");
  skcat[cat].push_back("Research");
  skcat[cat].push_back("Tactics");
  skcat[cat].push_back("Tracking");
  skcat[cat].push_back("High-G Ops");
  skcat[cat].push_back("Low-G Ops");
  skcat[cat].push_back("Zero-G Ops");
  cat = "Social Skills";
  skcat[cat].push_back("Acting");
  skcat[cat].push_back("Diplomacy");
  skcat[cat].push_back("Instruction");
  skcat[cat].push_back("Interrogation");
  skcat[cat].push_back("Intimidation");
  skcat[cat].push_back("Leadership");
  skcat[cat].push_back("Negotiation");
  skcat[cat].push_back("Performance");
  skcat[cat].push_back("Seduction");
  skcat[cat].push_back("Style");
  cat = "Etiquette Skills";
  skcat[cat].push_back("Farmer Etiquette");
  skcat[cat].push_back("Human Etiquette");
  skcat[cat].push_back("Dark Elf Etiquette");
  skcat[cat].push_back("Dwarf Etiquette");
  skcat[cat].push_back("Goblin Etiquette");
  skcat[cat].push_back("Grey Elf Etiquette");
  skcat[cat].push_back("High Elf Etiquette");
  skcat[cat].push_back("Mercenary Etiquette");
  skcat[cat].push_back("Noble Etiquette");
  skcat[cat].push_back("Ork Etiquette");
  skcat[cat].push_back("Thief Etiquette");
  skcat[cat].push_back("Underworld Etiquette");
  cat = "Creation Skills";
  skcat[cat].push_back("Armoring");
  skcat[cat].push_back("Bowyer");
  skcat[cat].push_back("Blacksmithing");
  skcat[cat].push_back("Carpentry");
  skcat[cat].push_back("Clothier");
  skcat[cat].push_back("Fletcher");
  skcat[cat].push_back("Leatherworking");
  skcat[cat].push_back("Masonry");
  skcat[cat].push_back("Metalworking");
  skcat[cat].push_back("Shipwright");
  skcat[cat].push_back("Smelting");
  skcat[cat].push_back("Weaponsmithing");
  skcat[cat].push_back("Woodworking");
  cat = "Magical Skills";
  skcat[cat].push_back("Alchemy");
  skcat[cat].push_back("Conjuring");
  skcat[cat].push_back("Enchanting");
  skcat[cat].push_back("Helmsman (Star)");
  skcat[cat].push_back("Ritual Conjuring");
  skcat[cat].push_back("Ritual Spellcasting");
  skcat[cat].push_back("Spellcasting");
  skcat[cat].push_back("Spellcraft");
  skcat[cat].push_back("Spell Targeting");
  skcat[cat].push_back("Staff Targeting");
  skcat[cat].push_back("Talismongery");
  skcat[cat].push_back("Wand Targeting");
  cat = "Etiquette Skills";
  skcat[cat].push_back("Activist Etiquette");
  skcat[cat].push_back("Corporate Etiquette");
  skcat[cat].push_back("Farmer Etiquette");
  skcat[cat].push_back("Gang Etiquette");
  skcat[cat].push_back("Government Etiquette");
  skcat[cat].push_back("High Society Etiquette");
  skcat[cat].push_back("Mercenary Etiquette");
  skcat[cat].push_back("Military Etiquette");
  skcat[cat].push_back("Netiquette");
  skcat[cat].push_back("Organized Crime Etiquette");
  skcat[cat].push_back("Police Etiquette");
  skcat[cat].push_back("Pop Culture Etiquette");
  skcat[cat].push_back("Religion Etiquette");
  skcat[cat].push_back("Trucker Etiquette");
  skcat[cat].push_back("Science Etiquette");
  skcat[cat].push_back("Sepratist Etiquette");
  skcat[cat].push_back("Street Etiquette");
  skcat[cat].push_back("University Etiquette");
  cat = "Technical Skills";
  skcat[cat].push_back("Computer");
  skcat[cat].push_back("Demolitions");
  skcat[cat].push_back("Electronics");
  skcat[cat].push_back("Mechanics");
  skcat[cat].push_back("Engineering");
  skcat[cat].push_back("First Aid");
  skcat[cat].push_back("Treatment");
  skcat[cat].push_back("Surgery");
  skcat[cat].push_back("Security");
  skcat[cat].push_back("Communications");
  cat = "Build/Repair Skills";
  skcat[cat].push_back("Computer B/R");
  skcat[cat].push_back("Electrical B/R");
  skcat[cat].push_back("Mechanical B/R");
  cat = "Piloting Skills";
  skcat[cat].push_back("Boat");
  skcat[cat].push_back("Ship");
  skcat[cat].push_back("Sled");
  skcat[cat].push_back("Teamster");
  cat = "Piloting Skills";
  skcat[cat].push_back("Bike");
  skcat[cat].push_back("Boat");
  skcat[cat].push_back("Boat, Powered");
  skcat[cat].push_back("Car");
  skcat[cat].push_back("Fixed-wing Craft");
  skcat[cat].push_back("Half-Track");
  skcat[cat].push_back("Hovercraft");
  skcat[cat].push_back("Rotorcraft");
  skcat[cat].push_back("Ship");
  skcat[cat].push_back("Ship, Powered");
  skcat[cat].push_back("Sled");
  skcat[cat].push_back("Snowmobile");
  skcat[cat].push_back("Teamster");
  skcat[cat].push_back("Tracked Vehicle");
  skcat[cat].push_back("Tractor");
  skcat[cat].push_back("Trike");
  skcat[cat].push_back("Vectored Thrust");
  cat = "Drone Piloting Skills";
  skcat[cat].push_back("Fixed-Wing Drone");
  skcat[cat].push_back("Hover Drone");
  skcat[cat].push_back("Rotor Drone");
  skcat[cat].push_back("Tracked Drone");
  skcat[cat].push_back("Vectored Thrust Drone");
  skcat[cat].push_back("Wheeled Drone");
  cat = "Etiquette Skills";
  skcat[cat].push_back("Activist Etiquette");
  skcat[cat].push_back("Corporate Etiquette");
  skcat[cat].push_back("Decker Etiquette");
  skcat[cat].push_back("Gang Etiquette");
  skcat[cat].push_back("Government Etiquette");
  skcat[cat].push_back("High Society Etiquette");
  skcat[cat].push_back("Lone Star Etiquette");
  skcat[cat].push_back("Mage Etiquette");
  skcat[cat].push_back("Matrix Etiquette");
  skcat[cat].push_back("Mercenary Etiquette");
  skcat[cat].push_back("Military Etiquette");
  skcat[cat].push_back("Organized Crime Etiquette");
  skcat[cat].push_back("Police Etiquette");
  skcat[cat].push_back("Pop Culture Etiquette");
  skcat[cat].push_back("Religion Etiquette");
  skcat[cat].push_back("Rigger Etiquette");
  skcat[cat].push_back("Science Etiquette");
  skcat[cat].push_back("Shaman Etiquette");
  skcat[cat].push_back("Street Etiquette");
  skcat[cat].push_back("University Etiquette");
  cat = "Magical Skills";
  skcat[cat].push_back("Alchemy");
  skcat[cat].push_back("Conjuring");
  skcat[cat].push_back("Enchanting");
  skcat[cat].push_back("Ritual Conjuring");
  skcat[cat].push_back("Ritual Spellcasting");
  skcat[cat].push_back("Spellcasting");
  skcat[cat].push_back("Spellcraft");
  skcat[cat].push_back("Talismongery");

  //Now back to non-automatically generated code
  add_wts("Grappling");
  add_wts("Kicking");
  add_wts("Punching");
  add_wts("Short Blades");
  add_wts("Short Cleaves");
  add_wts("Short Crushing");
  add_wts("Short Piercing");
  add_wts("Flails");
  add_wts("Staves");
  add_wts("Long Blades");
  add_wts("Long Cleaves");
  add_wts("Long Crushing");
  add_wts("Long Piercing");
  add_wts("Two-Handed Blades");
  add_wts("Two-Handed Cleaves");
  add_wts("Two-Handed Crushing");
  add_wts("Two-Handed Flails");
  add_wts("Two-Handed Piercing");
  add_wts("Two-Handed Staves");

  add_wts("Offhand Blades");
  add_wts("Offhand Cleaves");
  add_wts("Offhand Crushing");
  add_wts("Offhand Flails");
  add_wts("Offhand Piercing");
  add_wts("Offhand Staves");

  add_wts("Mounted Blades");
  add_wts("Mounted Cleaves");
  add_wts("Mounted Crushing");
  add_wts("Mounted Flails");
  add_wts("Mounted Piercing");

  add_wts("Archery");
  add_wts("Blowgun");
  add_wts("Crossbow");
  add_wts("Hurling");
  add_wts("Nets");
  add_wts("Slings");
  add_wts("Staff Slings");
  add_wts("Throwing (Aero)");
  add_wts("Throwing (Non-Aero)");
  add_wts("Whips");

  add_wts("Mounted Archery");
  add_wts("Mounted Blowgun");
  add_wts("Mounted Crossbow");
  add_wts("Mounted Nets");
  add_wts("Mounted Slings");
  add_wts("Mounted Throwing");
  add_wts("Mounted Whips");

  add_wts("Shields");
  }

int is_skill(string sk) {
  if(!defaults_init) init_defaults();
  return (defaults.count(sk) != 0);
  }

string get_weapon_skill(int wtype) {
  if(!defaults_init) init_defaults();
  if(!weaponskills.count(wtype)) {
    fprintf(stderr, "Warning: No Skill Type %d!\n", wtype);
    return "None";
    }
  return weaponskills[wtype];
  }

int get_weapon_type(string wskill) {
  if(!defaults_init) init_defaults();
  if(!weapontypes.count(wskill)) {
    fprintf(stderr, "Warning: No Skill Named '%s'!\n", wskill.c_str());
    return 0;
    }
  return weapontypes[wskill];
  }

string get_skill(string sk) {
  if(defaults.count(sk)) return sk;
  return "";
  }

string get_skill_cat(string cat) {
  if(skcat.count(cat)) return cat;
  return "";
  }

int get_linked(string sk) {
  if(defaults.count(sk)) return defaults[sk];
  return 4; // Default to Int for knowledges
  }

list<string> get_skills(string cat) {
  if(!defaults_init) init_defaults();
  list<string> ret;

  if(cat == "Categories") {
    typeof(skcat.begin()) ind;
    for(ind = skcat.begin(); ind != skcat.end(); ++ind) {
      ret.push_back(ind->first);
      }
    }
  else if(cat == "all") {
    typeof(defaults.begin()) ind;
    for(ind = defaults.begin(); ind != defaults.end(); ++ind) {
      ret.push_back(ind->first);
      }
    }
  else if(skcat.count(cat)) {
    typeof(skcat[cat].begin()) ind;
    for(ind = skcat[cat].begin(); ind != skcat[cat].end(); ++ind) {
      ret.push_back(*ind);
      }
    }

  return ret;
  }

int roll(int ndice, int targ) {
  int ret = 0, ctr;
  for(ctr=0; ctr<ndice; ++ctr) {
    int val = 1+(rand()%6);
    while((val%6) == 0) val += (1+rand()%6);
    while(val > 1 && val >= targ) { ++ret; val -= 12; }
    }
  return ret;
  }

int Object::Attribute(int a) const {
  return att[a];
  }

void Object::SetAttribute(int a, int v) {
  att[a] = v;
  }

void Object::SetSkill(const string &s, int v) {
  skills[s] = v;
  if(v <= 0) skills.erase(s);
  }

int Object::Skill(const string &s, int *tnum) const {
  if(strlen(s.c_str()) == 0) return 0;
  if(!strncasecmp(s.c_str(), "Body", s.length())) return att[0];
  if(!strncasecmp(s.c_str(), "Quickness", s.length())) return att[1];
  if(!strncasecmp(s.c_str(), "Strength", s.length())) return att[2];
  if(!strncasecmp(s.c_str(), "Charisma", s.length())) return att[3];
  if(!strncasecmp(s.c_str(), "Intelligence", s.length())) return att[4];
  if(!strncasecmp(s.c_str(), "Willpower", s.length())) return att[5];
  if(!defaults_init) init_defaults();
  if(skills.count(s)) return (skills.find(s))->second;  //const for 'skills[s]'
  if(tnum) {
    (*tnum) += 4;
    return att[defaults[s]];
    }
  return 0;
  }

int Object::Roll(const string &s1, const Object *p2, const string &s2, int bias, string *res) const {
  int succ = 0;

  int t1 = p2->Skill(s2) - bias;
  int t2 = Skill(s1) + bias;

  if(res) (*res) += "(";
  succ = Roll(s1, t1, res);
  if(s2 != "") {
    if(res) (*res) += " - ";
    succ -= p2->Roll(s2, t2, res);
    }
  if(res) (*res) += ")";
  return succ;
  }

int Object::Roll(const string &s1, int targ, string *res) const {
  if(phys>=10 || stun>=10 || (!(att[0]*att[1]*att[2]*att[3]*att[4]*att[5]))) {
    if(res) (*res) += "N/A";
    return 0;
    }
  targ += WoundPenalty();
  return RollNoWounds(s1, targ, res);
  }

int Object::RollNoWounds(const string &s1, int targ, string *res) const {
  int succ = 0;
  int d1 = Skill(s1, &targ);
  succ = roll(abs(d1), targ);

  if(res) {
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
  if(stun >= 10) ret += 4;
  else if(stun >= 6) ret += 3;
  else if(stun >= 3) ret += 2;
  else if(stun >= 1) ret += 1;
  if(phys >= 10) ret += 4;
  else if(phys >= 6) ret += 3;
  else if(phys >= 3) ret += 2;
  else if(phys >= 1) ret += 1;

  return ret;
  }

int Object::RollInitiative() const {
  int ret = 1+(rand()%6);
  ret += 1+(rand()%6);
  ret += att[1];
  ret += att[4];
  ret -= WoundPenalty();

  return ret;
  }
