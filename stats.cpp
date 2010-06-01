#include <cstring>
#include <cstdlib>

#include "object.h"

static map<AtomString,int> defaults;
static map<int,AtomString> weaponskills;
static map<AtomString,int> weapontypes;
static map<AtomString,list<AtomString> > skcat;

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

  //Skill Definition: Acrobatics
  defaults["Acrobatics"] = 1;
  skcat["Athletic Skills"].push_back("Acrobatics");
  skcat["Medieval Skills"].push_back("Acrobatics");
  skcat["Cyberpunk Skills"].push_back("Acrobatics");
  skcat["Modern Skills"].push_back("Acrobatics");
  skcat["Shadowrun Skills"].push_back("Acrobatics");
  skcat["Skills"].push_back("Acrobatics");

  //Skill Definition: Acting
  defaults["Acting"] = 3;
  skcat["Social Skills"].push_back("Acting");
  skcat["Medieval Skills"].push_back("Acting");
  skcat["Cyberpunk Skills"].push_back("Acting");
  skcat["Modern Skills"].push_back("Acting");
  skcat["Shadowrun Skills"].push_back("Acting");
  skcat["Skills"].push_back("Acting");

  //Skill Definition: Activist_Etiquette
  defaults["Activist Etiquette"] = 3;
  skcat["Etiquette Skills"].push_back("Activist Etiquette");
  skcat["Cyberpunk Skills"].push_back("Activist Etiquette");
  skcat["Modern Skills"].push_back("Activist Etiquette");
  skcat["Shadowrun Skills"].push_back("Activist Etiquette");
  skcat["Skills"].push_back("Activist Etiquette");

  //Skill Definition: Aikido
  defaults["Aikido"] = 4;
  skcat["Martial Arts Skills"].push_back("Aikido");
  skcat["Cyberpunk Skills"].push_back("Aikido");
  skcat["Modern Skills"].push_back("Aikido");
  skcat["Shadowrun Skills"].push_back("Aikido");
  skcat["Skills"].push_back("Aikido");

  //Skill Definition: Air_Pistols
  defaults["Air Pistols"] = 1;
  skcat["Pistol Skills"].push_back("Air Pistols");
  skcat["Cyberpunk Skills"].push_back("Air Pistols");
  skcat["Modern Skills"].push_back("Air Pistols");
  skcat["Shadowrun Skills"].push_back("Air Pistols");
  skcat["Skills"].push_back("Air Pistols");
  add_wts("Air Pistols");

  //Skill Definition: Air_Rifles
  defaults["Air Rifles"] = 1;
  skcat["Rifle Skills"].push_back("Air Rifles");
  skcat["Cyberpunk Skills"].push_back("Air Rifles");
  skcat["Modern Skills"].push_back("Air Rifles");
  skcat["Shadowrun Skills"].push_back("Air Rifles");
  skcat["Skills"].push_back("Air Rifles");
  add_wts("Air Rifles");

  //Skill Definition: Alchemy
  defaults["Alchemy"] = 4;
  skcat["Magical Skills"].push_back("Alchemy");
  skcat["Medieval Skills"].push_back("Alchemy");
  skcat["Shadowrun Skills"].push_back("Alchemy");
  skcat["Skills"].push_back("Alchemy");

  //Skill Definition: Amphibious_Combat
  defaults["Amphibious Combat"] = 4;
  skcat["Combat Environment Skills"].push_back("Amphibious Combat");
  skcat["Medieval Skills"].push_back("Amphibious Combat");
  skcat["Cyberpunk Skills"].push_back("Amphibious Combat");
  skcat["Modern Skills"].push_back("Amphibious Combat");
  skcat["Shadowrun Skills"].push_back("Amphibious Combat");
  skcat["Skills"].push_back("Amphibious Combat");

  //Skill Definition: Archery
  defaults["Archery"] = 1;
  skcat["Ranged-Combat Skills"].push_back("Archery");
  skcat["Medieval Skills"].push_back("Archery");
  skcat["Cyberpunk Skills"].push_back("Archery");
  skcat["Modern Skills"].push_back("Archery");
  skcat["Shadowrun Skills"].push_back("Archery");
  skcat["Skills"].push_back("Archery");
  add_wts("Archery");

  //Skill Definition: Armoring
  defaults["Armoring"] = 4;
  skcat["Creation Skills"].push_back("Armoring");
  skcat["Medieval Skills"].push_back("Armoring");
  skcat["Skills"].push_back("Armoring");

  //Skill Definition: Art,_(Type)
  defaults["Art, (Type)"] = 3;
  skcat["Social Skills"].push_back("Art, (Type)");
  skcat["Medieval Skills"].push_back("Art, (Type)");
  skcat["Cyberpunk Skills"].push_back("Art, (Type)");
  skcat["Modern Skills"].push_back("Art, (Type)");
  skcat["Shadowrun Skills"].push_back("Art, (Type)");
  skcat["Skills"].push_back("Art, (Type)");

  //Skill Definition: Assault_Rifles
  defaults["Assault Rifles"] = 1;
  skcat["Rifle Skills"].push_back("Assault Rifles");
  skcat["Cyberpunk Skills"].push_back("Assault Rifles");
  skcat["Modern Skills"].push_back("Assault Rifles");
  skcat["Shadowrun Skills"].push_back("Assault Rifles");
  skcat["Skills"].push_back("Assault Rifles");
  add_wts("Assault Rifles");

  //Skill Definition: Bike
  defaults["Bike"] = 4;
  skcat["Piloting Skills"].push_back("Bike");
  skcat["Cyberpunk Skills"].push_back("Bike");
  skcat["Modern Skills"].push_back("Bike");
  skcat["Shadowrun Skills"].push_back("Bike");
  skcat["Skills"].push_back("Bike");

  //Skill Definition: Blacksmithing
  defaults["Blacksmithing"] = 4;
  skcat["Creation Skills"].push_back("Blacksmithing");
  skcat["Medieval Skills"].push_back("Blacksmithing");
  skcat["Skills"].push_back("Blacksmithing");

  //Skill Definition: Blowgun
  defaults["Blowgun"] = 1;
  skcat["Ranged-Combat Skills"].push_back("Blowgun");
  skcat["Medieval Skills"].push_back("Blowgun");
  skcat["Cyberpunk Skills"].push_back("Blowgun");
  skcat["Modern Skills"].push_back("Blowgun");
  skcat["Shadowrun Skills"].push_back("Blowgun");
  skcat["Skills"].push_back("Blowgun");
  add_wts("Blowgun");

  //Skill Definition: Boat
  defaults["Boat"] = 2;
  skcat["Piloting Skills"].push_back("Boat");
  skcat["Medieval Skills"].push_back("Boat");
  skcat["Cyberpunk Skills"].push_back("Boat");
  skcat["Modern Skills"].push_back("Boat");
  skcat["Shadowrun Skills"].push_back("Boat");
  skcat["Skills"].push_back("Boat");

  //Skill Definition: Boat,_Powered
  defaults["Boat, Powered"] = 4;
  skcat["Piloting Skills"].push_back("Boat, Powered");
  skcat["Cyberpunk Skills"].push_back("Boat, Powered");
  skcat["Modern Skills"].push_back("Boat, Powered");
  skcat["Shadowrun Skills"].push_back("Boat, Powered");
  skcat["Skills"].push_back("Boat, Powered");

  //Skill Definition: Bowyer
  defaults["Bowyer"] = 4;
  skcat["Creation Skills"].push_back("Bowyer");
  skcat["Medieval Skills"].push_back("Bowyer");
  skcat["Skills"].push_back("Bowyer");

  //Skill Definition: Boxing
  defaults["Boxing"] = 2;
  skcat["Sport Martial Arts Skills"].push_back("Boxing");
  skcat["Cyberpunk Skills"].push_back("Boxing");
  skcat["Modern Skills"].push_back("Boxing");
  skcat["Shadowrun Skills"].push_back("Boxing");
  skcat["Skills"].push_back("Boxing");

  //Skill Definition: Brawling
  defaults["Brawling"] = 5;
  skcat["Specialized Hand-to-Hand Skills"].push_back("Brawling");
  skcat["Medieval Skills"].push_back("Brawling");
  skcat["Cyberpunk Skills"].push_back("Brawling");
  skcat["Modern Skills"].push_back("Brawling");
  skcat["Shadowrun Skills"].push_back("Brawling");
  skcat["Skills"].push_back("Brawling");

  //Skill Definition: Car
  defaults["Car"] = 6;
  skcat["Piloting Skills"].push_back("Car");
  skcat["Cyberpunk Skills"].push_back("Car");
  skcat["Modern Skills"].push_back("Car");
  skcat["Shadowrun Skills"].push_back("Car");
  skcat["Skills"].push_back("Car");

  //Skill Definition: Carpentry
  defaults["Carpentry"] = 4;
  skcat["Creation Skills"].push_back("Carpentry");
  skcat["Medieval Skills"].push_back("Carpentry");
  skcat["Skills"].push_back("Carpentry");

  //Skill Definition: Carromeleg_-_Tier_I
  defaults["Carromeleg - Tier I"] = 0;
  skcat["Elven Martial Arts Skills"].push_back("Carromeleg - Tier I");
  skcat["Medieval Skills"].push_back("Carromeleg - Tier I");
  skcat["Shadowrun Skills"].push_back("Carromeleg - Tier I");
  skcat["Skills"].push_back("Carromeleg - Tier I");

  //Skill Definition: Carromeleg_-_Tier_II
  defaults["Carromeleg - Tier II"] = 1;
  skcat["Elven Martial Arts Skills"].push_back("Carromeleg - Tier II");
  skcat["Medieval Skills"].push_back("Carromeleg - Tier II");
  skcat["Shadowrun Skills"].push_back("Carromeleg - Tier II");
  skcat["Skills"].push_back("Carromeleg - Tier II");

  //Skill Definition: Carromeleg_-_Tier_III
  defaults["Carromeleg - Tier III"] = 2;
  skcat["Elven Martial Arts Skills"].push_back("Carromeleg - Tier III");
  skcat["Medieval Skills"].push_back("Carromeleg - Tier III");
  skcat["Shadowrun Skills"].push_back("Carromeleg - Tier III");
  skcat["Skills"].push_back("Carromeleg - Tier III");

  //Skill Definition: Carromeleg_-_Tier_IV
  defaults["Carromeleg - Tier IV"] = 3;
  skcat["Elven Martial Arts Skills"].push_back("Carromeleg - Tier IV");
  skcat["Medieval Skills"].push_back("Carromeleg - Tier IV");
  skcat["Shadowrun Skills"].push_back("Carromeleg - Tier IV");
  skcat["Skills"].push_back("Carromeleg - Tier IV");

  //Skill Definition: Carromeleg_-_Tier_V
  defaults["Carromeleg - Tier V"] = 4;
  skcat["Elven Martial Arts Skills"].push_back("Carromeleg - Tier V");
  skcat["Medieval Skills"].push_back("Carromeleg - Tier V");
  skcat["Shadowrun Skills"].push_back("Carromeleg - Tier V");
  skcat["Skills"].push_back("Carromeleg - Tier V");

  //Skill Definition: Carromeleg_-_Tier_VI
  defaults["Carromeleg - Tier VI"] = 5;
  skcat["Elven Martial Arts Skills"].push_back("Carromeleg - Tier VI");
  skcat["Medieval Skills"].push_back("Carromeleg - Tier VI");
  skcat["Shadowrun Skills"].push_back("Carromeleg - Tier VI");
  skcat["Skills"].push_back("Carromeleg - Tier VI");

  //Skill Definition: Cart
  defaults["Cart"] = 6;
  skcat["Piloting Skills"].push_back("Cart");
  skcat["Cyberpunk Skills"].push_back("Cart");
  skcat["Modern Skills"].push_back("Cart");
  skcat["Shadowrun Skills"].push_back("Cart");
  skcat["Skills"].push_back("Cart");

  //Skill Definition: Catching
  defaults["Catching"] = 1;
  skcat["Athletic Skills"].push_back("Catching");
  skcat["Medieval Skills"].push_back("Catching");
  skcat["Cyberpunk Skills"].push_back("Catching");
  skcat["Modern Skills"].push_back("Catching");
  skcat["Shadowrun Skills"].push_back("Catching");
  skcat["Skills"].push_back("Catching");

  //Skill Definition: Climbing
  defaults["Climbing"] = 2;
  skcat["Athletic Skills"].push_back("Climbing");
  skcat["Medieval Skills"].push_back("Climbing");
  skcat["Cyberpunk Skills"].push_back("Climbing");
  skcat["Modern Skills"].push_back("Climbing");
  skcat["Shadowrun Skills"].push_back("Climbing");
  skcat["Skills"].push_back("Climbing");

  //Skill Definition: Clothier
  defaults["Clothier"] = 4;
  skcat["Creation Skills"].push_back("Clothier");
  skcat["Medieval Skills"].push_back("Clothier");
  skcat["Skills"].push_back("Clothier");

  //Skill Definition: Communications
  defaults["Communications"] = 4;
  skcat["Technical Skills"].push_back("Communications");
  skcat["Cyberpunk Skills"].push_back("Communications");
  skcat["Modern Skills"].push_back("Communications");
  skcat["Shadowrun Skills"].push_back("Communications");
  skcat["Skills"].push_back("Communications");

  //Skill Definition: Computer
  defaults["Computer"] = 4;
  skcat["Technical Skills"].push_back("Computer");
  skcat["Cyberpunk Skills"].push_back("Computer");
  skcat["Modern Skills"].push_back("Computer");
  skcat["Shadowrun Skills"].push_back("Computer");
  skcat["Skills"].push_back("Computer");

  //Skill Definition: Computer_Build/Repair
  defaults["Computer Build/Repair"] = 4;
  skcat["Build/Repair Skills"].push_back("Computer Build/Repair");
  skcat["Cyberpunk Skills"].push_back("Computer Build/Repair");
  skcat["Modern Skills"].push_back("Computer Build/Repair");
  skcat["Shadowrun Skills"].push_back("Computer Build/Repair");
  skcat["Skills"].push_back("Computer Build/Repair");

  //Skill Definition: Conjuring
  defaults["Conjuring"] = 3;
  skcat["Magical Skills"].push_back("Conjuring");
  skcat["Medieval Skills"].push_back("Conjuring");
  skcat["Shadowrun Skills"].push_back("Conjuring");
  skcat["Skills"].push_back("Conjuring");

  //Skill Definition: Corporate_Etiquette
  defaults["Corporate Etiquette"] = 3;
  skcat["Etiquette Skills"].push_back("Corporate Etiquette");
  skcat["Cyberpunk Skills"].push_back("Corporate Etiquette");
  skcat["Modern Skills"].push_back("Corporate Etiquette");
  skcat["Shadowrun Skills"].push_back("Corporate Etiquette");
  skcat["Skills"].push_back("Corporate Etiquette");

  //Skill Definition: Craftsman_Etiquette
  defaults["Craftsman Etiquette"] = 3;
  skcat["Etiquette Skills"].push_back("Craftsman Etiquette");
  skcat["Medieval Skills"].push_back("Craftsman Etiquette");
  skcat["Cyberpunk Skills"].push_back("Craftsman Etiquette");
  skcat["Modern Skills"].push_back("Craftsman Etiquette");
  skcat["Shadowrun Skills"].push_back("Craftsman Etiquette");
  skcat["Skills"].push_back("Craftsman Etiquette");

  //Skill Definition: Crossbow
  defaults["Crossbow"] = 1;
  skcat["Ranged-Combat Skills"].push_back("Crossbow");
  skcat["Medieval Skills"].push_back("Crossbow");
  skcat["Cyberpunk Skills"].push_back("Crossbow");
  skcat["Modern Skills"].push_back("Crossbow");
  skcat["Shadowrun Skills"].push_back("Crossbow");
  skcat["Skills"].push_back("Crossbow");
  add_wts("Crossbow");

  //Skill Definition: Cycling
  defaults["Cycling"] = 6;
  skcat["Piloting Skills"].push_back("Cycling");
  skcat["Cyberpunk Skills"].push_back("Cycling");
  skcat["Modern Skills"].push_back("Cycling");
  skcat["Shadowrun Skills"].push_back("Cycling");
  skcat["Skills"].push_back("Cycling");

  //Skill Definition: Damage_Control
  defaults["Damage Control"] = 4;
  skcat["Technical Skills"].push_back("Damage Control");
  skcat["Cyberpunk Skills"].push_back("Damage Control");
  skcat["Modern Skills"].push_back("Damage Control");
  skcat["Shadowrun Skills"].push_back("Damage Control");
  skcat["Skills"].push_back("Damage Control");

  //Skill Definition: Dark_Elf_Etiquette
  defaults["Dark Elf Etiquette"] = 3;
  skcat["Etiquette Skills"].push_back("Dark Elf Etiquette");
  skcat["Medieval Skills"].push_back("Dark Elf Etiquette");
  skcat["Skills"].push_back("Dark Elf Etiquette");

  //Skill Definition: Decker_Etiquette
  defaults["Decker Etiquette"] = 3;
  skcat["Etiquette Skills"].push_back("Decker Etiquette");
  skcat["Shadowrun Skills"].push_back("Decker Etiquette");
  skcat["Skills"].push_back("Decker Etiquette");

  //Skill Definition: Demolitions
  defaults["Demolitions"] = 4;
  skcat["Technical Skills"].push_back("Demolitions");
  skcat["Cyberpunk Skills"].push_back("Demolitions");
  skcat["Modern Skills"].push_back("Demolitions");
  skcat["Shadowrun Skills"].push_back("Demolitions");
  skcat["Skills"].push_back("Demolitions");

  //Skill Definition: Diplomacy
  defaults["Diplomacy"] = 3;
  skcat["Social Skills"].push_back("Diplomacy");
  skcat["Medieval Skills"].push_back("Diplomacy");
  skcat["Cyberpunk Skills"].push_back("Diplomacy");
  skcat["Modern Skills"].push_back("Diplomacy");
  skcat["Shadowrun Skills"].push_back("Diplomacy");
  skcat["Skills"].push_back("Diplomacy");

  //Skill Definition: Diving
  defaults["Diving"] = 1;
  skcat["Athletic Skills"].push_back("Diving");
  skcat["Medieval Skills"].push_back("Diving");
  skcat["Cyberpunk Skills"].push_back("Diving");
  skcat["Modern Skills"].push_back("Diving");
  skcat["Shadowrun Skills"].push_back("Diving");
  skcat["Skills"].push_back("Diving");

  //Skill Definition: Dueling
  defaults["Dueling"] = 1;
  skcat["Martial Arts Skills"].push_back("Dueling");
  skcat["Cyberpunk Skills"].push_back("Dueling");
  skcat["Modern Skills"].push_back("Dueling");
  skcat["Shadowrun Skills"].push_back("Dueling");
  skcat["Skills"].push_back("Dueling");

  //Skill Definition: Dwarf_Etiquette
  defaults["Dwarf Etiquette"] = 3;
  skcat["Etiquette Skills"].push_back("Dwarf Etiquette");
  skcat["Medieval Skills"].push_back("Dwarf Etiquette");
  skcat["Skills"].push_back("Dwarf Etiquette");

  //Skill Definition: Eastern_Etiquette
  defaults["Eastern Etiquette"] = 3;
  skcat["Etiquette Skills"].push_back("Eastern Etiquette");
  skcat["Medieval Skills"].push_back("Eastern Etiquette");
  skcat["Skills"].push_back("Eastern Etiquette");

  //Skill Definition: Electrical_Build/Repair
  defaults["Electrical Build/Repair"] = 4;
  skcat["Build/Repair Skills"].push_back("Electrical Build/Repair");
  skcat["Cyberpunk Skills"].push_back("Electrical Build/Repair");
  skcat["Modern Skills"].push_back("Electrical Build/Repair");
  skcat["Shadowrun Skills"].push_back("Electrical Build/Repair");
  skcat["Skills"].push_back("Electrical Build/Repair");

  //Skill Definition: Electronics
  defaults["Electronics"] = 4;
  skcat["Technical Skills"].push_back("Electronics");
  skcat["Cyberpunk Skills"].push_back("Electronics");
  skcat["Modern Skills"].push_back("Electronics");
  skcat["Shadowrun Skills"].push_back("Electronics");
  skcat["Skills"].push_back("Electronics");

  //Skill Definition: Enchanting
  defaults["Enchanting"] = 4;
  skcat["Magical Skills"].push_back("Enchanting");
  skcat["Medieval Skills"].push_back("Enchanting");
  skcat["Shadowrun Skills"].push_back("Enchanting");
  skcat["Skills"].push_back("Enchanting");

  //Skill Definition: Engineering
  defaults["Engineering"] = 4;
  skcat["Technical Skills"].push_back("Engineering");
  skcat["Medieval Skills"].push_back("Engineering");
  skcat["Cyberpunk Skills"].push_back("Engineering");
  skcat["Modern Skills"].push_back("Engineering");
  skcat["Shadowrun Skills"].push_back("Engineering");
  skcat["Skills"].push_back("Engineering");

  //Skill Definition: Farmer_Etiquette
  defaults["Farmer Etiquette"] = 3;
  skcat["Etiquette Skills"].push_back("Farmer Etiquette");
  skcat["Cyberpunk Skills"].push_back("Farmer Etiquette");
  skcat["Modern Skills"].push_back("Farmer Etiquette");
  skcat["Shadowrun Skills"].push_back("Farmer Etiquette");
  skcat["Skills"].push_back("Farmer Etiquette");

  //Skill Definition: Fencing
  defaults["Fencing"] = 1;
  skcat["Sport Martial Arts Skills"].push_back("Fencing");
  skcat["Cyberpunk Skills"].push_back("Fencing");
  skcat["Modern Skills"].push_back("Fencing");
  skcat["Shadowrun Skills"].push_back("Fencing");
  skcat["Skills"].push_back("Fencing");

  //Skill Definition: Find_Juju
  defaults["Find Juju"] = 5;
  skcat["Shamanistic Skills"].push_back("Find Juju");
  skcat["Medieval Skills"].push_back("Find Juju");
  skcat["Skills"].push_back("Find Juju");

  //Skill Definition: First_Aid
  defaults["First Aid"] = 4;
  skcat["Technical Skills"].push_back("First Aid");
  skcat["Cyberpunk Skills"].push_back("First Aid");
  skcat["Modern Skills"].push_back("First Aid");
  skcat["Shadowrun Skills"].push_back("First Aid");
  skcat["Skills"].push_back("First Aid");

  //Skill Definition: Fixed-Wing_Drone
  defaults["Fixed-Wing Drone"] = 4;
  skcat["Drone Piloting Skills"].push_back("Fixed-Wing Drone");
  skcat["Shadowrun Skills"].push_back("Fixed-Wing Drone");
  skcat["Skills"].push_back("Fixed-Wing Drone");

  //Skill Definition: Fixed-wing_Craft
  defaults["Fixed-wing Craft"] = 6;
  skcat["Piloting Skills"].push_back("Fixed-wing Craft");
  skcat["Cyberpunk Skills"].push_back("Fixed-wing Craft");
  skcat["Modern Skills"].push_back("Fixed-wing Craft");
  skcat["Shadowrun Skills"].push_back("Fixed-wing Craft");
  skcat["Skills"].push_back("Fixed-wing Craft");

  //Skill Definition: Fletcher
  defaults["Fletcher"] = 4;
  skcat["Creation Skills"].push_back("Fletcher");
  skcat["Medieval Skills"].push_back("Fletcher");
  skcat["Skills"].push_back("Fletcher");

  //Skill Definition: Flight
  defaults["Flight"] = 1;
  skcat["Athletic Skills"].push_back("Flight");
  skcat["Medieval Skills"].push_back("Flight");
  skcat["Cyberpunk Skills"].push_back("Flight");
  skcat["Modern Skills"].push_back("Flight");
  skcat["Shadowrun Skills"].push_back("Flight");
  skcat["Skills"].push_back("Flight");

  //Skill Definition: Gang_Etiquette
  defaults["Gang Etiquette"] = 3;
  skcat["Etiquette Skills"].push_back("Gang Etiquette");
  skcat["Cyberpunk Skills"].push_back("Gang Etiquette");
  skcat["Modern Skills"].push_back("Gang Etiquette");
  skcat["Shadowrun Skills"].push_back("Gang Etiquette");
  skcat["Skills"].push_back("Gang Etiquette");

  //Skill Definition: Goblin_Etiquette
  defaults["Goblin Etiquette"] = 3;
  skcat["Etiquette Skills"].push_back("Goblin Etiquette");
  skcat["Medieval Skills"].push_back("Goblin Etiquette");
  skcat["Skills"].push_back("Goblin Etiquette");

  //Skill Definition: Government_Etiquette
  defaults["Government Etiquette"] = 3;
  skcat["Etiquette Skills"].push_back("Government Etiquette");
  skcat["Cyberpunk Skills"].push_back("Government Etiquette");
  skcat["Modern Skills"].push_back("Government Etiquette");
  skcat["Shadowrun Skills"].push_back("Government Etiquette");
  skcat["Skills"].push_back("Government Etiquette");

  //Skill Definition: Grappling
  defaults["Grappling"] = 2;
  skcat["Hand-to-Hand Combat Skills"].push_back("Grappling");
  skcat["Medieval Skills"].push_back("Grappling");
  skcat["Cyberpunk Skills"].push_back("Grappling");
  skcat["Modern Skills"].push_back("Grappling");
  skcat["Shadowrun Skills"].push_back("Grappling");
  skcat["Skills"].push_back("Grappling");
  add_wts("Grappling");

  //Skill Definition: Grey_Elf_Etiquette
  defaults["Grey Elf Etiquette"] = 3;
  skcat["Etiquette Skills"].push_back("Grey Elf Etiquette");
  skcat["Medieval Skills"].push_back("Grey Elf Etiquette");
  skcat["Skills"].push_back("Grey Elf Etiquette");

  //Skill Definition: Gunnery
  defaults["Gunnery"] = 4;
  skcat["Heavy Firearm Skills"].push_back("Gunnery");
  skcat["Cyberpunk Skills"].push_back("Gunnery");
  skcat["Modern Skills"].push_back("Gunnery");
  skcat["Shadowrun Skills"].push_back("Gunnery");
  skcat["Skills"].push_back("Gunnery");

  //Skill Definition: Half-Track
  defaults["Half-Track"] = 6;
  skcat["Piloting Skills"].push_back("Half-Track");
  skcat["Cyberpunk Skills"].push_back("Half-Track");
  skcat["Modern Skills"].push_back("Half-Track");
  skcat["Shadowrun Skills"].push_back("Half-Track");
  skcat["Skills"].push_back("Half-Track");

  //Skill Definition: Hapkido
  defaults["Hapkido"] = 1;
  skcat["Martial Arts Skills"].push_back("Hapkido");
  skcat["Cyberpunk Skills"].push_back("Hapkido");
  skcat["Modern Skills"].push_back("Hapkido");
  skcat["Shadowrun Skills"].push_back("Hapkido");
  skcat["Skills"].push_back("Hapkido");

  //Skill Definition: Healing
  defaults["Healing"] = 4;
  skcat["Nature Skills"].push_back("Healing");
  skcat["Medieval Skills"].push_back("Healing");
  skcat["Cyberpunk Skills"].push_back("Healing");
  skcat["Modern Skills"].push_back("Healing");
  skcat["Shadowrun Skills"].push_back("Healing");
  skcat["Skills"].push_back("Healing");

  //Skill Definition: Heavy_Firearms
  defaults["Heavy Firearms"] = 2;
  skcat["Heavy Firearm Skills"].push_back("Heavy Firearms");
  skcat["Cyberpunk Skills"].push_back("Heavy Firearms");
  skcat["Modern Skills"].push_back("Heavy Firearms");
  skcat["Shadowrun Skills"].push_back("Heavy Firearms");
  skcat["Skills"].push_back("Heavy Firearms");

  //Skill Definition: Heavy_Lasers
  defaults["Heavy Lasers"] = 2;
  skcat["Heavy Firearm Skills"].push_back("Heavy Lasers");
  skcat["Cyberpunk Skills"].push_back("Heavy Lasers");
  skcat["Modern Skills"].push_back("Heavy Lasers");
  skcat["Shadowrun Skills"].push_back("Heavy Lasers");
  skcat["Skills"].push_back("Heavy Lasers");

  //Skill Definition: Helmsman,_Star
  defaults["Helmsman, Star"] = 4;
  skcat["Magical Skills"].push_back("Helmsman, Star");
  skcat["Medieval Skills"].push_back("Helmsman, Star");
  skcat["Skills"].push_back("Helmsman, Star");

  //Skill Definition: High_Elf_Etiquette
  defaults["High Elf Etiquette"] = 3;
  skcat["Etiquette Skills"].push_back("High Elf Etiquette");
  skcat["Medieval Skills"].push_back("High Elf Etiquette");
  skcat["Skills"].push_back("High Elf Etiquette");

  //Skill Definition: High_Society_Etiquette
  defaults["High Society Etiquette"] = 3;
  skcat["Etiquette Skills"].push_back("High Society Etiquette");
  skcat["Cyberpunk Skills"].push_back("High Society Etiquette");
  skcat["Modern Skills"].push_back("High Society Etiquette");
  skcat["Shadowrun Skills"].push_back("High Society Etiquette");
  skcat["Skills"].push_back("High Society Etiquette");

  //Skill Definition: High-G_Combat
  defaults["High-G Combat"] = 4;
  skcat["Combat Environment Skills"].push_back("High-G Combat");
  skcat["Medieval Skills"].push_back("High-G Combat");
  skcat["Cyberpunk Skills"].push_back("High-G Combat");
  skcat["Modern Skills"].push_back("High-G Combat");
  skcat["Shadowrun Skills"].push_back("High-G Combat");
  skcat["Skills"].push_back("High-G Combat");

  //Skill Definition: High-G_Ops
  defaults["High-G Ops"] = 4;
  skcat["Specialized Skills"].push_back("High-G Ops");
  skcat["Medieval Skills"].push_back("High-G Ops");
  skcat["Cyberpunk Skills"].push_back("High-G Ops");
  skcat["Modern Skills"].push_back("High-G Ops");
  skcat["Shadowrun Skills"].push_back("High-G Ops");
  skcat["Skills"].push_back("High-G Ops");

  //Skill Definition: Hover_Drone
  defaults["Hover Drone"] = 4;
  skcat["Drone Piloting Skills"].push_back("Hover Drone");
  skcat["Shadowrun Skills"].push_back("Hover Drone");
  skcat["Skills"].push_back("Hover Drone");

  //Skill Definition: Hovercraft
  defaults["Hovercraft"] = 6;
  skcat["Piloting Skills"].push_back("Hovercraft");
  skcat["Cyberpunk Skills"].push_back("Hovercraft");
  skcat["Modern Skills"].push_back("Hovercraft");
  skcat["Shadowrun Skills"].push_back("Hovercraft");
  skcat["Skills"].push_back("Hovercraft");

  //Skill Definition: Human_Etiquette
  defaults["Human Etiquette"] = 3;
  skcat["Etiquette Skills"].push_back("Human Etiquette");
  skcat["Medieval Skills"].push_back("Human Etiquette");
  skcat["Skills"].push_back("Human Etiquette");

  //Skill Definition: Hurling
  defaults["Hurling"] = 2;
  skcat["Ranged-Combat Skills"].push_back("Hurling");
  skcat["Medieval Skills"].push_back("Hurling");
  skcat["Cyberpunk Skills"].push_back("Hurling");
  skcat["Modern Skills"].push_back("Hurling");
  skcat["Shadowrun Skills"].push_back("Hurling");
  skcat["Skills"].push_back("Hurling");
  add_wts("Hurling");

  //Skill Definition: Identify_Juju
  defaults["Identify Juju"] = 4;
  skcat["Shamanistic Skills"].push_back("Identify Juju");
  skcat["Medieval Skills"].push_back("Identify Juju");
  skcat["Skills"].push_back("Identify Juju");

  //Skill Definition: Instruction
  defaults["Instruction"] = 3;
  skcat["Social Skills"].push_back("Instruction");
  skcat["Medieval Skills"].push_back("Instruction");
  skcat["Cyberpunk Skills"].push_back("Instruction");
  skcat["Modern Skills"].push_back("Instruction");
  skcat["Shadowrun Skills"].push_back("Instruction");
  skcat["Skills"].push_back("Instruction");

  //Skill Definition: Interrogation
  defaults["Interrogation"] = 4;
  skcat["Social Skills"].push_back("Interrogation");
  skcat["Medieval Skills"].push_back("Interrogation");
  skcat["Cyberpunk Skills"].push_back("Interrogation");
  skcat["Modern Skills"].push_back("Interrogation");
  skcat["Shadowrun Skills"].push_back("Interrogation");
  skcat["Skills"].push_back("Interrogation");

  //Skill Definition: Intimidation
  defaults["Intimidation"] = 5;
  skcat["Social Skills"].push_back("Intimidation");
  skcat["Medieval Skills"].push_back("Intimidation");
  skcat["Cyberpunk Skills"].push_back("Intimidation");
  skcat["Modern Skills"].push_back("Intimidation");
  skcat["Shadowrun Skills"].push_back("Intimidation");
  skcat["Skills"].push_back("Intimidation");

  //Skill Definition: Judo
  defaults["Judo"] = 2;
  skcat["Sport Martial Arts Skills"].push_back("Judo");
  skcat["Cyberpunk Skills"].push_back("Judo");
  skcat["Modern Skills"].push_back("Judo");
  skcat["Shadowrun Skills"].push_back("Judo");
  skcat["Skills"].push_back("Judo");

  //Skill Definition: Jujitsu
  defaults["Jujitsu"] = 1;
  skcat["Martial Arts Skills"].push_back("Jujitsu");
  skcat["Cyberpunk Skills"].push_back("Jujitsu");
  skcat["Modern Skills"].push_back("Jujitsu");
  skcat["Shadowrun Skills"].push_back("Jujitsu");
  skcat["Skills"].push_back("Jujitsu");

  //Skill Definition: Jumping
  defaults["Jumping"] = 2;
  skcat["Athletic Skills"].push_back("Jumping");
  skcat["Medieval Skills"].push_back("Jumping");
  skcat["Cyberpunk Skills"].push_back("Jumping");
  skcat["Modern Skills"].push_back("Jumping");
  skcat["Shadowrun Skills"].push_back("Jumping");
  skcat["Skills"].push_back("Jumping");

  //Skill Definition: Karate
  defaults["Karate"] = 1;
  skcat["Martial Arts Skills"].push_back("Karate");
  skcat["Cyberpunk Skills"].push_back("Karate");
  skcat["Modern Skills"].push_back("Karate");
  skcat["Shadowrun Skills"].push_back("Karate");
  skcat["Skills"].push_back("Karate");

  //Skill Definition: Kicking
  defaults["Kicking"] = 1;
  skcat["Hand-to-Hand Combat Skills"].push_back("Kicking");
  skcat["Medieval Skills"].push_back("Kicking");
  skcat["Cyberpunk Skills"].push_back("Kicking");
  skcat["Modern Skills"].push_back("Kicking");
  skcat["Shadowrun Skills"].push_back("Kicking");
  skcat["Skills"].push_back("Kicking");
  add_wts("Kicking");

  //Skill Definition: Knowledge
  defaults["Knowledge"] = 4;
  skcat["Specialized Skills"].push_back("Knowledge");
  skcat["Medieval Skills"].push_back("Knowledge");
  skcat["Cyberpunk Skills"].push_back("Knowledge");
  skcat["Modern Skills"].push_back("Knowledge");
  skcat["Shadowrun Skills"].push_back("Knowledge");
  skcat["Skills"].push_back("Knowledge");

  //Skill Definition: Kung_Fu
  defaults["Kung Fu"] = 1;
  skcat["Sport Martial Arts Skills"].push_back("Kung Fu");
  skcat["Cyberpunk Skills"].push_back("Kung Fu");
  skcat["Modern Skills"].push_back("Kung Fu");
  skcat["Shadowrun Skills"].push_back("Kung Fu");
  skcat["Skills"].push_back("Kung Fu");

  //Skill Definition: Laser_Pistols
  defaults["Laser Pistols"] = 1;
  skcat["Pistol Skills"].push_back("Laser Pistols");
  skcat["Cyberpunk Skills"].push_back("Laser Pistols");
  skcat["Modern Skills"].push_back("Laser Pistols");
  skcat["Shadowrun Skills"].push_back("Laser Pistols");
  skcat["Skills"].push_back("Laser Pistols");
  add_wts("Laser Pistols");

  //Skill Definition: Laser_Rifles
  defaults["Laser Rifles"] = 1;
  skcat["Rifle Skills"].push_back("Laser Rifles");
  skcat["Cyberpunk Skills"].push_back("Laser Rifles");
  skcat["Modern Skills"].push_back("Laser Rifles");
  skcat["Shadowrun Skills"].push_back("Laser Rifles");
  skcat["Skills"].push_back("Laser Rifles");
  add_wts("Laser Rifles");

  //Skill Definition: Lasso
  defaults["Lasso"] = 1;
  skcat["Ranged-Combat Skills"].push_back("Lasso");
  skcat["Medieval Skills"].push_back("Lasso");
  skcat["Cyberpunk Skills"].push_back("Lasso");
  skcat["Modern Skills"].push_back("Lasso");
  skcat["Shadowrun Skills"].push_back("Lasso");
  skcat["Skills"].push_back("Lasso");
  add_wts("Lasso");

  //Skill Definition: Launch_Weapons
  defaults["Launch Weapons"] = 4;
  skcat["Heavy Firearm Skills"].push_back("Launch Weapons");
  skcat["Cyberpunk Skills"].push_back("Launch Weapons");
  skcat["Modern Skills"].push_back("Launch Weapons");
  skcat["Shadowrun Skills"].push_back("Launch Weapons");
  skcat["Skills"].push_back("Launch Weapons");

  //Skill Definition: Leadership
  defaults["Leadership"] = 3;
  skcat["Social Skills"].push_back("Leadership");
  skcat["Medieval Skills"].push_back("Leadership");
  skcat["Cyberpunk Skills"].push_back("Leadership");
  skcat["Modern Skills"].push_back("Leadership");
  skcat["Shadowrun Skills"].push_back("Leadership");
  skcat["Skills"].push_back("Leadership");

  //Skill Definition: Leatherworking
  defaults["Leatherworking"] = 4;
  skcat["Creation Skills"].push_back("Leatherworking");
  skcat["Medieval Skills"].push_back("Leatherworking");
  skcat["Skills"].push_back("Leatherworking");

  //Skill Definition: Lifting
  defaults["Lifting"] = 2;
  skcat["Athletic Skills"].push_back("Lifting");
  skcat["Medieval Skills"].push_back("Lifting");
  skcat["Cyberpunk Skills"].push_back("Lifting");
  skcat["Modern Skills"].push_back("Lifting");
  skcat["Shadowrun Skills"].push_back("Lifting");
  skcat["Skills"].push_back("Lifting");

  //Skill Definition: Lone_Star_Etiquette
  defaults["Lone Star Etiquette"] = 3;
  skcat["Etiquette Skills"].push_back("Lone Star Etiquette");
  skcat["Cyberpunk Skills"].push_back("Lone Star Etiquette");
  skcat["Shadowrun Skills"].push_back("Lone Star Etiquette");
  skcat["Skills"].push_back("Lone Star Etiquette");

  //Skill Definition: Long_Blades
  defaults["Long Blades"] = 1;
  skcat["Melee-Combat Skills"].push_back("Long Blades");
  skcat["Medieval Skills"].push_back("Long Blades");
  skcat["Cyberpunk Skills"].push_back("Long Blades");
  skcat["Modern Skills"].push_back("Long Blades");
  skcat["Shadowrun Skills"].push_back("Long Blades");
  skcat["Skills"].push_back("Long Blades");
  add_wts("Long Blades");

  //Skill Definition: Long_Cleaves
  defaults["Long Cleaves"] = 1;
  skcat["Melee-Combat Skills"].push_back("Long Cleaves");
  skcat["Medieval Skills"].push_back("Long Cleaves");
  skcat["Cyberpunk Skills"].push_back("Long Cleaves");
  skcat["Modern Skills"].push_back("Long Cleaves");
  skcat["Shadowrun Skills"].push_back("Long Cleaves");
  skcat["Skills"].push_back("Long Cleaves");
  add_wts("Long Cleaves");

  //Skill Definition: Long_Crushing
  defaults["Long Crushing"] = 1;
  skcat["Melee-Combat Skills"].push_back("Long Crushing");
  skcat["Medieval Skills"].push_back("Long Crushing");
  skcat["Cyberpunk Skills"].push_back("Long Crushing");
  skcat["Modern Skills"].push_back("Long Crushing");
  skcat["Shadowrun Skills"].push_back("Long Crushing");
  skcat["Skills"].push_back("Long Crushing");
  add_wts("Long Crushing");

  //Skill Definition: Long_Flails
  defaults["Long Flails"] = 1;
  skcat["Melee-Combat Skills"].push_back("Long Flails");
  skcat["Medieval Skills"].push_back("Long Flails");
  skcat["Cyberpunk Skills"].push_back("Long Flails");
  skcat["Modern Skills"].push_back("Long Flails");
  skcat["Shadowrun Skills"].push_back("Long Flails");
  skcat["Skills"].push_back("Long Flails");
  add_wts("Long Flails");

  //Skill Definition: Long_Piercing
  defaults["Long Piercing"] = 1;
  skcat["Melee-Combat Skills"].push_back("Long Piercing");
  skcat["Medieval Skills"].push_back("Long Piercing");
  skcat["Cyberpunk Skills"].push_back("Long Piercing");
  skcat["Modern Skills"].push_back("Long Piercing");
  skcat["Shadowrun Skills"].push_back("Long Piercing");
  skcat["Skills"].push_back("Long Piercing");
  add_wts("Long Piercing");

  //Skill Definition: Low-G_Combat
  defaults["Low-G Combat"] = 4;
  skcat["Combat Environment Skills"].push_back("Low-G Combat");
  skcat["Medieval Skills"].push_back("Low-G Combat");
  skcat["Cyberpunk Skills"].push_back("Low-G Combat");
  skcat["Modern Skills"].push_back("Low-G Combat");
  skcat["Shadowrun Skills"].push_back("Low-G Combat");
  skcat["Skills"].push_back("Low-G Combat");

  //Skill Definition: Low-G_Ops
  defaults["Low-G Ops"] = 4;
  skcat["Specialized Skills"].push_back("Low-G Ops");
  skcat["Medieval Skills"].push_back("Low-G Ops");
  skcat["Cyberpunk Skills"].push_back("Low-G Ops");
  skcat["Modern Skills"].push_back("Low-G Ops");
  skcat["Shadowrun Skills"].push_back("Low-G Ops");
  skcat["Skills"].push_back("Low-G Ops");

  //Skill Definition: Machine_Pistols
  defaults["Machine Pistols"] = 1;
  skcat["Pistol Skills"].push_back("Machine Pistols");
  skcat["Cyberpunk Skills"].push_back("Machine Pistols");
  skcat["Modern Skills"].push_back("Machine Pistols");
  skcat["Shadowrun Skills"].push_back("Machine Pistols");
  skcat["Skills"].push_back("Machine Pistols");
  add_wts("Machine Pistols");

  //Skill Definition: Mage_Etiquette
  defaults["Mage Etiquette"] = 3;
  skcat["Etiquette Skills"].push_back("Mage Etiquette");
  skcat["Medieval Skills"].push_back("Mage Etiquette");
  skcat["Shadowrun Skills"].push_back("Mage Etiquette");
  skcat["Skills"].push_back("Mage Etiquette");

  //Skill Definition: Masonry
  defaults["Masonry"] = 4;
  skcat["Creation Skills"].push_back("Masonry");
  skcat["Medieval Skills"].push_back("Masonry");
  skcat["Skills"].push_back("Masonry");

  //Skill Definition: Matrix_Etiquette
  defaults["Matrix Etiquette"] = 3;
  skcat["Etiquette Skills"].push_back("Matrix Etiquette");
  skcat["Shadowrun Skills"].push_back("Matrix Etiquette");
  skcat["Skills"].push_back("Matrix Etiquette");

  //Skill Definition: Mechanical_Build/Repair
  defaults["Mechanical Build/Repair"] = 4;
  skcat["Build/Repair Skills"].push_back("Mechanical Build/Repair");
  skcat["Cyberpunk Skills"].push_back("Mechanical Build/Repair");
  skcat["Modern Skills"].push_back("Mechanical Build/Repair");
  skcat["Shadowrun Skills"].push_back("Mechanical Build/Repair");
  skcat["Skills"].push_back("Mechanical Build/Repair");

  //Skill Definition: Mechanics
  defaults["Mechanics"] = 4;
  skcat["Technical Skills"].push_back("Mechanics");
  skcat["Medieval Skills"].push_back("Mechanics");
  skcat["Cyberpunk Skills"].push_back("Mechanics");
  skcat["Modern Skills"].push_back("Mechanics");
  skcat["Shadowrun Skills"].push_back("Mechanics");
  skcat["Skills"].push_back("Mechanics");

  //Skill Definition: Mercenary_Etiquette
  defaults["Mercenary Etiquette"] = 3;
  skcat["Etiquette Skills"].push_back("Mercenary Etiquette");
  skcat["Medieval Skills"].push_back("Mercenary Etiquette");
  skcat["Cyberpunk Skills"].push_back("Mercenary Etiquette");
  skcat["Modern Skills"].push_back("Mercenary Etiquette");
  skcat["Shadowrun Skills"].push_back("Mercenary Etiquette");
  skcat["Skills"].push_back("Mercenary Etiquette");

  //Skill Definition: Merchant_Etiquette
  defaults["Merchant Etiquette"] = 3;
  skcat["Etiquette Skills"].push_back("Merchant Etiquette");
  skcat["Medieval Skills"].push_back("Merchant Etiquette");
  skcat["Cyberpunk Skills"].push_back("Merchant Etiquette");
  skcat["Modern Skills"].push_back("Merchant Etiquette");
  skcat["Shadowrun Skills"].push_back("Merchant Etiquette");
  skcat["Skills"].push_back("Merchant Etiquette");

  //Skill Definition: Metalworking
  defaults["Metalworking"] = 4;
  skcat["Creation Skills"].push_back("Metalworking");
  skcat["Medieval Skills"].push_back("Metalworking");
  skcat["Skills"].push_back("Metalworking");

  //Skill Definition: Military_Etiquette
  defaults["Military Etiquette"] = 3;
  skcat["Etiquette Skills"].push_back("Military Etiquette");
  skcat["Medieval Skills"].push_back("Military Etiquette");
  skcat["Cyberpunk Skills"].push_back("Military Etiquette");
  skcat["Modern Skills"].push_back("Military Etiquette");
  skcat["Shadowrun Skills"].push_back("Military Etiquette");
  skcat["Skills"].push_back("Military Etiquette");

  //Skill Definition: Mounted_Air_Pistols
  defaults["Mounted Air Pistols"] = 1;
  skcat["Mounted Pistol Skills"].push_back("Mounted Air Pistols");
  skcat["Cyberpunk Skills"].push_back("Mounted Air Pistols");
  skcat["Modern Skills"].push_back("Mounted Air Pistols");
  skcat["Shadowrun Skills"].push_back("Mounted Air Pistols");
  skcat["Skills"].push_back("Mounted Air Pistols");
  add_wts("Mounted Air Pistols");

  //Skill Definition: Mounted_Archery
  defaults["Mounted Archery"] = 1;
  skcat["Mounted Ranged-Combat Skills"].push_back("Mounted Archery");
  skcat["Medieval Skills"].push_back("Mounted Archery");
  skcat["Cyberpunk Skills"].push_back("Mounted Archery");
  skcat["Modern Skills"].push_back("Mounted Archery");
  skcat["Shadowrun Skills"].push_back("Mounted Archery");
  skcat["Skills"].push_back("Mounted Archery");
  add_wts("Mounted Archery");

  //Skill Definition: Mounted_Blades
  defaults["Mounted Blades"] = 1;
  skcat["Mounted Melee-Combat Skills"].push_back("Mounted Blades");
  skcat["Medieval Skills"].push_back("Mounted Blades");
  skcat["Cyberpunk Skills"].push_back("Mounted Blades");
  skcat["Modern Skills"].push_back("Mounted Blades");
  skcat["Shadowrun Skills"].push_back("Mounted Blades");
  skcat["Skills"].push_back("Mounted Blades");
  add_wts("Mounted Blades");

  //Skill Definition: Mounted_Blowgun
  defaults["Mounted Blowgun"] = 1;
  skcat["Mounted Ranged-Combat Skills"].push_back("Mounted Blowgun");
  skcat["Medieval Skills"].push_back("Mounted Blowgun");
  skcat["Cyberpunk Skills"].push_back("Mounted Blowgun");
  skcat["Modern Skills"].push_back("Mounted Blowgun");
  skcat["Shadowrun Skills"].push_back("Mounted Blowgun");
  skcat["Skills"].push_back("Mounted Blowgun");
  add_wts("Mounted Blowgun");

  //Skill Definition: Mounted_Cleaves
  defaults["Mounted Cleaves"] = 1;
  skcat["Mounted Melee-Combat Skills"].push_back("Mounted Cleaves");
  skcat["Medieval Skills"].push_back("Mounted Cleaves");
  skcat["Cyberpunk Skills"].push_back("Mounted Cleaves");
  skcat["Modern Skills"].push_back("Mounted Cleaves");
  skcat["Shadowrun Skills"].push_back("Mounted Cleaves");
  skcat["Skills"].push_back("Mounted Cleaves");
  add_wts("Mounted Cleaves");

  //Skill Definition: Mounted_Crossbow
  defaults["Mounted Crossbow"] = 1;
  skcat["Mounted Ranged-Combat Skills"].push_back("Mounted Crossbow");
  skcat["Medieval Skills"].push_back("Mounted Crossbow");
  skcat["Cyberpunk Skills"].push_back("Mounted Crossbow");
  skcat["Modern Skills"].push_back("Mounted Crossbow");
  skcat["Shadowrun Skills"].push_back("Mounted Crossbow");
  skcat["Skills"].push_back("Mounted Crossbow");
  add_wts("Mounted Crossbow");

  //Skill Definition: Mounted_Crushing
  defaults["Mounted Crushing"] = 1;
  skcat["Mounted Melee-Combat Skills"].push_back("Mounted Crushing");
  skcat["Medieval Skills"].push_back("Mounted Crushing");
  skcat["Cyberpunk Skills"].push_back("Mounted Crushing");
  skcat["Modern Skills"].push_back("Mounted Crushing");
  skcat["Shadowrun Skills"].push_back("Mounted Crushing");
  skcat["Skills"].push_back("Mounted Crushing");
  add_wts("Mounted Crushing");

  //Skill Definition: Mounted_Flails
  defaults["Mounted Flails"] = 1;
  skcat["Mounted Melee-Combat Skills"].push_back("Mounted Flails");
  skcat["Medieval Skills"].push_back("Mounted Flails");
  skcat["Cyberpunk Skills"].push_back("Mounted Flails");
  skcat["Modern Skills"].push_back("Mounted Flails");
  skcat["Shadowrun Skills"].push_back("Mounted Flails");
  skcat["Skills"].push_back("Mounted Flails");
  add_wts("Mounted Flails");

  //Skill Definition: Mounted_Hurling
  defaults["Mounted Hurling"] = 1;
  skcat["Mounted Ranged-Combat Skills"].push_back("Mounted Hurling");
  skcat["Medieval Skills"].push_back("Mounted Hurling");
  skcat["Cyberpunk Skills"].push_back("Mounted Hurling");
  skcat["Modern Skills"].push_back("Mounted Hurling");
  skcat["Shadowrun Skills"].push_back("Mounted Hurling");
  skcat["Skills"].push_back("Mounted Hurling");
  add_wts("Mounted Hurling");

  //Skill Definition: Mounted_Laser_Pistols
  defaults["Mounted Laser Pistols"] = 1;
  skcat["Mounted Pistol Skills"].push_back("Mounted Laser Pistols");
  skcat["Cyberpunk Skills"].push_back("Mounted Laser Pistols");
  skcat["Modern Skills"].push_back("Mounted Laser Pistols");
  skcat["Shadowrun Skills"].push_back("Mounted Laser Pistols");
  skcat["Skills"].push_back("Mounted Laser Pistols");
  add_wts("Mounted Laser Pistols");

  //Skill Definition: Mounted_Machine_Pistols
  defaults["Mounted Machine Pistols"] = 1;
  skcat["Mounted Pistol Skills"].push_back("Mounted Machine Pistols");
  skcat["Cyberpunk Skills"].push_back("Mounted Machine Pistols");
  skcat["Modern Skills"].push_back("Mounted Machine Pistols");
  skcat["Shadowrun Skills"].push_back("Mounted Machine Pistols");
  skcat["Skills"].push_back("Mounted Machine Pistols");
  add_wts("Mounted Machine Pistols");

  //Skill Definition: Mounted_Nets
  defaults["Mounted Nets"] = 1;
  skcat["Mounted Ranged-Combat Skills"].push_back("Mounted Nets");
  skcat["Medieval Skills"].push_back("Mounted Nets");
  skcat["Cyberpunk Skills"].push_back("Mounted Nets");
  skcat["Modern Skills"].push_back("Mounted Nets");
  skcat["Shadowrun Skills"].push_back("Mounted Nets");
  skcat["Skills"].push_back("Mounted Nets");
  add_wts("Mounted Nets");

  //Skill Definition: Mounted_Piercing
  defaults["Mounted Piercing"] = 1;
  skcat["Mounted Melee-Combat Skills"].push_back("Mounted Piercing");
  skcat["Medieval Skills"].push_back("Mounted Piercing");
  skcat["Cyberpunk Skills"].push_back("Mounted Piercing");
  skcat["Modern Skills"].push_back("Mounted Piercing");
  skcat["Shadowrun Skills"].push_back("Mounted Piercing");
  skcat["Skills"].push_back("Mounted Piercing");
  add_wts("Mounted Piercing");

  //Skill Definition: Mounted_Pistols
  defaults["Mounted Pistols"] = 1;
  skcat["Mounted Pistol Skills"].push_back("Mounted Pistols");
  skcat["Cyberpunk Skills"].push_back("Mounted Pistols");
  skcat["Modern Skills"].push_back("Mounted Pistols");
  skcat["Shadowrun Skills"].push_back("Mounted Pistols");
  skcat["Skills"].push_back("Mounted Pistols");
  add_wts("Mounted Pistols");

  //Skill Definition: Mounted_Plasma_Pistols
  defaults["Mounted Plasma Pistols"] = 1;
  skcat["Mounted Pistol Skills"].push_back("Mounted Plasma Pistols");
  skcat["Cyberpunk Skills"].push_back("Mounted Plasma Pistols");
  skcat["Modern Skills"].push_back("Mounted Plasma Pistols");
  skcat["Shadowrun Skills"].push_back("Mounted Plasma Pistols");
  skcat["Skills"].push_back("Mounted Plasma Pistols");
  add_wts("Mounted Plasma Pistols");

  //Skill Definition: Mounted_SMGs
  defaults["Mounted SMGs"] = 1;
  skcat["Mounted Pistol Skills"].push_back("Mounted SMGs");
  skcat["Cyberpunk Skills"].push_back("Mounted SMGs");
  skcat["Modern Skills"].push_back("Mounted SMGs");
  skcat["Shadowrun Skills"].push_back("Mounted SMGs");
  skcat["Skills"].push_back("Mounted SMGs");
  add_wts("Mounted SMGs");

  //Skill Definition: Mounted_Shot_Pistols
  defaults["Mounted Shot Pistols"] = 1;
  skcat["Mounted Pistol Skills"].push_back("Mounted Shot Pistols");
  skcat["Cyberpunk Skills"].push_back("Mounted Shot Pistols");
  skcat["Modern Skills"].push_back("Mounted Shot Pistols");
  skcat["Shadowrun Skills"].push_back("Mounted Shot Pistols");
  skcat["Skills"].push_back("Mounted Shot Pistols");
  add_wts("Mounted Shot Pistols");

  //Skill Definition: Mounted_Slings
  defaults["Mounted Slings"] = 1;
  skcat["Mounted Ranged-Combat Skills"].push_back("Mounted Slings");
  skcat["Medieval Skills"].push_back("Mounted Slings");
  skcat["Cyberpunk Skills"].push_back("Mounted Slings");
  skcat["Modern Skills"].push_back("Mounted Slings");
  skcat["Shadowrun Skills"].push_back("Mounted Slings");
  skcat["Skills"].push_back("Mounted Slings");
  add_wts("Mounted Slings");

  //Skill Definition: Mounted_Throwing,_Aero
  defaults["Mounted Throwing, Aero"] = 1;
  skcat["Mounted Ranged-Combat Skills"].push_back("Mounted Throwing, Aero");
  skcat["Medieval Skills"].push_back("Mounted Throwing, Aero");
  skcat["Cyberpunk Skills"].push_back("Mounted Throwing, Aero");
  skcat["Modern Skills"].push_back("Mounted Throwing, Aero");
  skcat["Shadowrun Skills"].push_back("Mounted Throwing, Aero");
  skcat["Skills"].push_back("Mounted Throwing, Aero");
  add_wts("Mounted Throwing, Aero");

  //Skill Definition: Mounted_Throwing,_Non-Aero
  defaults["Mounted Throwing, Non-Aero"] = 1;
  skcat["Mounted Ranged-Combat Skills"].push_back("Mounted Throwing, Non-Aero");
  skcat["Medieval Skills"].push_back("Mounted Throwing, Non-Aero");
  skcat["Cyberpunk Skills"].push_back("Mounted Throwing, Non-Aero");
  skcat["Modern Skills"].push_back("Mounted Throwing, Non-Aero");
  skcat["Shadowrun Skills"].push_back("Mounted Throwing, Non-Aero");
  skcat["Skills"].push_back("Mounted Throwing, Non-Aero");
  add_wts("Mounted Throwing, Non-Aero");

  //Skill Definition: Mounted_Whips
  defaults["Mounted Whips"] = 1;
  skcat["Mounted Ranged-Combat Skills"].push_back("Mounted Whips");
  skcat["Medieval Skills"].push_back("Mounted Whips");
  skcat["Cyberpunk Skills"].push_back("Mounted Whips");
  skcat["Modern Skills"].push_back("Mounted Whips");
  skcat["Shadowrun Skills"].push_back("Mounted Whips");
  skcat["Skills"].push_back("Mounted Whips");
  add_wts("Mounted Whips");

  //Skill Definition: Muay_Thai
  defaults["Muay Thai"] = 1;
  skcat["Sport Martial Arts Skills"].push_back("Muay Thai");
  skcat["Cyberpunk Skills"].push_back("Muay Thai");
  skcat["Modern Skills"].push_back("Muay Thai");
  skcat["Shadowrun Skills"].push_back("Muay Thai");
  skcat["Skills"].push_back("Muay Thai");

  //Skill Definition: Navigation
  defaults["Navigation"] = 4;
  skcat["Nature Skills"].push_back("Navigation");
  skcat["Medieval Skills"].push_back("Navigation");
  skcat["Cyberpunk Skills"].push_back("Navigation");
  skcat["Modern Skills"].push_back("Navigation");
  skcat["Shadowrun Skills"].push_back("Navigation");
  skcat["Skills"].push_back("Navigation");

  //Skill Definition: Negotiation
  defaults["Negotiation"] = 3;
  skcat["Social Skills"].push_back("Negotiation");
  skcat["Medieval Skills"].push_back("Negotiation");
  skcat["Cyberpunk Skills"].push_back("Negotiation");
  skcat["Modern Skills"].push_back("Negotiation");
  skcat["Shadowrun Skills"].push_back("Negotiation");
  skcat["Skills"].push_back("Negotiation");

  //Skill Definition: Netiquette
  defaults["Netiquette"] = 4;
  skcat["Etiquette Skills"].push_back("Netiquette");
  skcat["Cyberpunk Skills"].push_back("Netiquette");
  skcat["Modern Skills"].push_back("Netiquette");
  skcat["Skills"].push_back("Netiquette");

  //Skill Definition: Nets
  defaults["Nets"] = 1;
  skcat["Ranged-Combat Skills"].push_back("Nets");
  skcat["Medieval Skills"].push_back("Nets");
  skcat["Cyberpunk Skills"].push_back("Nets");
  skcat["Modern Skills"].push_back("Nets");
  skcat["Shadowrun Skills"].push_back("Nets");
  skcat["Skills"].push_back("Nets");
  add_wts("Nets");

  //Skill Definition: Noble_Etiquette
  defaults["Noble Etiquette"] = 3;
  skcat["Etiquette Skills"].push_back("Noble Etiquette");
  skcat["Medieval Skills"].push_back("Noble Etiquette");
  skcat["Skills"].push_back("Noble Etiquette");

  //Skill Definition: Oceanic_Etiquette
  defaults["Oceanic Etiquette"] = 3;
  skcat["Etiquette Skills"].push_back("Oceanic Etiquette");
  skcat["Medieval Skills"].push_back("Oceanic Etiquette");
  skcat["Skills"].push_back("Oceanic Etiquette");

  //Skill Definition: Offhand_Air_Pistols
  defaults["Offhand Air Pistols"] = 1;
  skcat["Pistol Skills"].push_back("Offhand Air Pistols");
  skcat["Cyberpunk Skills"].push_back("Offhand Air Pistols");
  skcat["Modern Skills"].push_back("Offhand Air Pistols");
  skcat["Shadowrun Skills"].push_back("Offhand Air Pistols");
  skcat["Skills"].push_back("Offhand Air Pistols");
  add_wts("Offhand Air Pistols");

  //Skill Definition: Offhand_Blades
  defaults["Offhand Blades"] = 1;
  skcat["Melee-Combat Skills"].push_back("Offhand Blades");
  skcat["Medieval Skills"].push_back("Offhand Blades");
  skcat["Cyberpunk Skills"].push_back("Offhand Blades");
  skcat["Modern Skills"].push_back("Offhand Blades");
  skcat["Shadowrun Skills"].push_back("Offhand Blades");
  skcat["Skills"].push_back("Offhand Blades");
  add_wts("Offhand Blades");

  //Skill Definition: Offhand_Cleaves
  defaults["Offhand Cleaves"] = 1;
  skcat["Melee-Combat Skills"].push_back("Offhand Cleaves");
  skcat["Medieval Skills"].push_back("Offhand Cleaves");
  skcat["Cyberpunk Skills"].push_back("Offhand Cleaves");
  skcat["Modern Skills"].push_back("Offhand Cleaves");
  skcat["Shadowrun Skills"].push_back("Offhand Cleaves");
  skcat["Skills"].push_back("Offhand Cleaves");
  add_wts("Offhand Cleaves");

  //Skill Definition: Offhand_Crossbow
  defaults["Offhand Crossbow"] = 1;
  skcat["Ranged-Combat Skills"].push_back("Offhand Crossbow");
  skcat["Medieval Skills"].push_back("Offhand Crossbow");
  skcat["Cyberpunk Skills"].push_back("Offhand Crossbow");
  skcat["Modern Skills"].push_back("Offhand Crossbow");
  skcat["Shadowrun Skills"].push_back("Offhand Crossbow");
  skcat["Skills"].push_back("Offhand Crossbow");
  add_wts("Offhand Crossbow");

  //Skill Definition: Offhand_Crushing
  defaults["Offhand Crushing"] = 1;
  skcat["Melee-Combat Skills"].push_back("Offhand Crushing");
  skcat["Medieval Skills"].push_back("Offhand Crushing");
  skcat["Cyberpunk Skills"].push_back("Offhand Crushing");
  skcat["Modern Skills"].push_back("Offhand Crushing");
  skcat["Shadowrun Skills"].push_back("Offhand Crushing");
  skcat["Skills"].push_back("Offhand Crushing");
  add_wts("Offhand Crushing");

  //Skill Definition: Offhand_Flails
  defaults["Offhand Flails"] = 1;
  skcat["Melee-Combat Skills"].push_back("Offhand Flails");
  skcat["Medieval Skills"].push_back("Offhand Flails");
  skcat["Cyberpunk Skills"].push_back("Offhand Flails");
  skcat["Modern Skills"].push_back("Offhand Flails");
  skcat["Shadowrun Skills"].push_back("Offhand Flails");
  skcat["Skills"].push_back("Offhand Flails");
  add_wts("Offhand Flails");

  //Skill Definition: Offhand_Hurling
  defaults["Offhand Hurling"] = 1;
  skcat["Ranged-Combat Skills"].push_back("Offhand Hurling");
  skcat["Medieval Skills"].push_back("Offhand Hurling");
  skcat["Cyberpunk Skills"].push_back("Offhand Hurling");
  skcat["Modern Skills"].push_back("Offhand Hurling");
  skcat["Shadowrun Skills"].push_back("Offhand Hurling");
  skcat["Skills"].push_back("Offhand Hurling");
  add_wts("Offhand Hurling");

  //Skill Definition: Offhand_Laser_Pistols
  defaults["Offhand Laser Pistols"] = 1;
  skcat["Pistol Skills"].push_back("Offhand Laser Pistols");
  skcat["Cyberpunk Skills"].push_back("Offhand Laser Pistols");
  skcat["Modern Skills"].push_back("Offhand Laser Pistols");
  skcat["Shadowrun Skills"].push_back("Offhand Laser Pistols");
  skcat["Skills"].push_back("Offhand Laser Pistols");
  add_wts("Offhand Laser Pistols");

  //Skill Definition: Offhand_Machine_Pistols
  defaults["Offhand Machine Pistols"] = 1;
  skcat["Pistol Skills"].push_back("Offhand Machine Pistols");
  skcat["Cyberpunk Skills"].push_back("Offhand Machine Pistols");
  skcat["Modern Skills"].push_back("Offhand Machine Pistols");
  skcat["Shadowrun Skills"].push_back("Offhand Machine Pistols");
  skcat["Skills"].push_back("Offhand Machine Pistols");
  add_wts("Offhand Machine Pistols");

  //Skill Definition: Offhand_Piercing
  defaults["Offhand Piercing"] = 1;
  skcat["Melee-Combat Skills"].push_back("Offhand Piercing");
  skcat["Medieval Skills"].push_back("Offhand Piercing");
  skcat["Cyberpunk Skills"].push_back("Offhand Piercing");
  skcat["Modern Skills"].push_back("Offhand Piercing");
  skcat["Shadowrun Skills"].push_back("Offhand Piercing");
  skcat["Skills"].push_back("Offhand Piercing");
  add_wts("Offhand Piercing");

  //Skill Definition: Offhand_Pistols
  defaults["Offhand Pistols"] = 1;
  skcat["Pistol Skills"].push_back("Offhand Pistols");
  skcat["Cyberpunk Skills"].push_back("Offhand Pistols");
  skcat["Modern Skills"].push_back("Offhand Pistols");
  skcat["Shadowrun Skills"].push_back("Offhand Pistols");
  skcat["Skills"].push_back("Offhand Pistols");
  add_wts("Offhand Pistols");

  //Skill Definition: Offhand_Plasma_Pistols
  defaults["Offhand Plasma Pistols"] = 1;
  skcat["Pistol Skills"].push_back("Offhand Plasma Pistols");
  skcat["Cyberpunk Skills"].push_back("Offhand Plasma Pistols");
  skcat["Modern Skills"].push_back("Offhand Plasma Pistols");
  skcat["Shadowrun Skills"].push_back("Offhand Plasma Pistols");
  skcat["Skills"].push_back("Offhand Plasma Pistols");
  add_wts("Offhand Plasma Pistols");

  //Skill Definition: Offhand_SMGs
  defaults["Offhand SMGs"] = 1;
  skcat["Pistol Skills"].push_back("Offhand SMGs");
  skcat["Cyberpunk Skills"].push_back("Offhand SMGs");
  skcat["Modern Skills"].push_back("Offhand SMGs");
  skcat["Shadowrun Skills"].push_back("Offhand SMGs");
  skcat["Skills"].push_back("Offhand SMGs");
  add_wts("Offhand SMGs");

  //Skill Definition: Offhand_Shot_Pistols
  defaults["Offhand Shot Pistols"] = 1;
  skcat["Pistol Skills"].push_back("Offhand Shot Pistols");
  skcat["Cyberpunk Skills"].push_back("Offhand Shot Pistols");
  skcat["Modern Skills"].push_back("Offhand Shot Pistols");
  skcat["Shadowrun Skills"].push_back("Offhand Shot Pistols");
  skcat["Skills"].push_back("Offhand Shot Pistols");
  add_wts("Offhand Shot Pistols");

  //Skill Definition: Offhand_Staves
  defaults["Offhand Staves"] = 1;
  skcat["Melee-Combat Skills"].push_back("Offhand Staves");
  skcat["Medieval Skills"].push_back("Offhand Staves");
  skcat["Cyberpunk Skills"].push_back("Offhand Staves");
  skcat["Modern Skills"].push_back("Offhand Staves");
  skcat["Shadowrun Skills"].push_back("Offhand Staves");
  skcat["Skills"].push_back("Offhand Staves");
  add_wts("Offhand Staves");

  //Skill Definition: Offhand_Throwing,_Aero
  defaults["Offhand Throwing, Aero"] = 1;
  skcat["Ranged-Combat Skills"].push_back("Offhand Throwing, Aero");
  skcat["Medieval Skills"].push_back("Offhand Throwing, Aero");
  skcat["Cyberpunk Skills"].push_back("Offhand Throwing, Aero");
  skcat["Modern Skills"].push_back("Offhand Throwing, Aero");
  skcat["Shadowrun Skills"].push_back("Offhand Throwing, Aero");
  skcat["Skills"].push_back("Offhand Throwing, Aero");
  add_wts("Offhand Throwing, Aero");

  //Skill Definition: Offhand_Throwing,_Non-Aero
  defaults["Offhand Throwing, Non-Aero"] = 1;
  skcat["Ranged-Combat Skills"].push_back("Offhand Throwing, Non-Aero");
  skcat["Medieval Skills"].push_back("Offhand Throwing, Non-Aero");
  skcat["Cyberpunk Skills"].push_back("Offhand Throwing, Non-Aero");
  skcat["Modern Skills"].push_back("Offhand Throwing, Non-Aero");
  skcat["Shadowrun Skills"].push_back("Offhand Throwing, Non-Aero");
  skcat["Skills"].push_back("Offhand Throwing, Non-Aero");
  add_wts("Offhand Throwing, Non-Aero");

  //Skill Definition: Offhand_Whips
  defaults["Offhand Whips"] = 1;
  skcat["Ranged-Combat Skills"].push_back("Offhand Whips");
  skcat["Medieval Skills"].push_back("Offhand Whips");
  skcat["Cyberpunk Skills"].push_back("Offhand Whips");
  skcat["Modern Skills"].push_back("Offhand Whips");
  skcat["Shadowrun Skills"].push_back("Offhand Whips");
  skcat["Skills"].push_back("Offhand Whips");
  add_wts("Offhand Whips");

  //Skill Definition: Organized_Crime_Etiquette
  defaults["Organized Crime Etiquette"] = 3;
  skcat["Etiquette Skills"].push_back("Organized Crime Etiquette");
  skcat["Cyberpunk Skills"].push_back("Organized Crime Etiquette");
  skcat["Modern Skills"].push_back("Organized Crime Etiquette");
  skcat["Shadowrun Skills"].push_back("Organized Crime Etiquette");
  skcat["Skills"].push_back("Organized Crime Etiquette");

  //Skill Definition: Ork_Etiquette
  defaults["Ork Etiquette"] = 3;
  skcat["Etiquette Skills"].push_back("Ork Etiquette");
  skcat["Medieval Skills"].push_back("Ork Etiquette");
  skcat["Skills"].push_back("Ork Etiquette");

  //Skill Definition: Peasant_Etiquette
  defaults["Peasant Etiquette"] = 3;
  skcat["Etiquette Skills"].push_back("Peasant Etiquette");
  skcat["Medieval Skills"].push_back("Peasant Etiquette");
  skcat["Skills"].push_back("Peasant Etiquette");

  //Skill Definition: Perception
  defaults["Perception"] = 4;
  skcat["Specialized Skills"].push_back("Perception");
  skcat["Medieval Skills"].push_back("Perception");
  skcat["Cyberpunk Skills"].push_back("Perception");
  skcat["Modern Skills"].push_back("Perception");
  skcat["Shadowrun Skills"].push_back("Perception");
  skcat["Expert Skills"].push_back("Perception");
  skcat["Skills"].push_back("Perception");

  //Skill Definition: Performance,_(Type)
  defaults["Performance, (Type)"] = 3;
  skcat["Social Skills"].push_back("Performance, (Type)");
  skcat["Medieval Skills"].push_back("Performance, (Type)");
  skcat["Cyberpunk Skills"].push_back("Performance, (Type)");
  skcat["Modern Skills"].push_back("Performance, (Type)");
  skcat["Shadowrun Skills"].push_back("Performance, (Type)");
  skcat["Skills"].push_back("Performance, (Type)");

  //Skill Definition: Pistols
  defaults["Pistols"] = 1;
  skcat["Pistol Skills"].push_back("Pistols");
  skcat["Cyberpunk Skills"].push_back("Pistols");
  skcat["Modern Skills"].push_back("Pistols");
  skcat["Shadowrun Skills"].push_back("Pistols");
  skcat["Skills"].push_back("Pistols");
  add_wts("Pistols");

  //Skill Definition: Plasma_Pistols
  defaults["Plasma Pistols"] = 1;
  skcat["Pistol Skills"].push_back("Plasma Pistols");
  skcat["Cyberpunk Skills"].push_back("Plasma Pistols");
  skcat["Modern Skills"].push_back("Plasma Pistols");
  skcat["Shadowrun Skills"].push_back("Plasma Pistols");
  skcat["Skills"].push_back("Plasma Pistols");
  add_wts("Plasma Pistols");

  //Skill Definition: Plasma_Rifles
  defaults["Plasma Rifles"] = 1;
  skcat["Rifle Skills"].push_back("Plasma Rifles");
  skcat["Cyberpunk Skills"].push_back("Plasma Rifles");
  skcat["Modern Skills"].push_back("Plasma Rifles");
  skcat["Shadowrun Skills"].push_back("Plasma Rifles");
  skcat["Skills"].push_back("Plasma Rifles");
  add_wts("Plasma Rifles");

  //Skill Definition: Police_Etiquette
  defaults["Police Etiquette"] = 3;
  skcat["Etiquette Skills"].push_back("Police Etiquette");
  skcat["Cyberpunk Skills"].push_back("Police Etiquette");
  skcat["Modern Skills"].push_back("Police Etiquette");
  skcat["Shadowrun Skills"].push_back("Police Etiquette");
  skcat["Skills"].push_back("Police Etiquette");

  //Skill Definition: Pop_Culture_Etiquette
  defaults["Pop Culture Etiquette"] = 3;
  skcat["Etiquette Skills"].push_back("Pop Culture Etiquette");
  skcat["Cyberpunk Skills"].push_back("Pop Culture Etiquette");
  skcat["Modern Skills"].push_back("Pop Culture Etiquette");
  skcat["Shadowrun Skills"].push_back("Pop Culture Etiquette");
  skcat["Skills"].push_back("Pop Culture Etiquette");

  //Skill Definition: Punching
  defaults["Punching"] = 1;
  skcat["Hand-to-Hand Combat Skills"].push_back("Punching");
  skcat["Medieval Skills"].push_back("Punching");
  skcat["Cyberpunk Skills"].push_back("Punching");
  skcat["Modern Skills"].push_back("Punching");
  skcat["Shadowrun Skills"].push_back("Punching");
  skcat["Skills"].push_back("Punching");
  add_wts("Punching");

  //Skill Definition: Quick-draw
  defaults["Quick-draw"] = 1;
  skcat["Specialized Skills"].push_back("Quick-draw");
  skcat["Medieval Skills"].push_back("Quick-draw");
  skcat["Cyberpunk Skills"].push_back("Quick-draw");
  skcat["Modern Skills"].push_back("Quick-draw");
  skcat["Shadowrun Skills"].push_back("Quick-draw");
  skcat["Skills"].push_back("Quick-draw");

  //Skill Definition: Refine_Juju
  defaults["Refine Juju"] = 3;
  skcat["Shamanistic Skills"].push_back("Refine Juju");
  skcat["Medieval Skills"].push_back("Refine Juju");
  skcat["Skills"].push_back("Refine Juju");

  //Skill Definition: Religion_Etiquette
  defaults["Religion Etiquette"] = 3;
  skcat["Etiquette Skills"].push_back("Religion Etiquette");
  skcat["Cyberpunk Skills"].push_back("Religion Etiquette");
  skcat["Modern Skills"].push_back("Religion Etiquette");
  skcat["Shadowrun Skills"].push_back("Religion Etiquette");
  skcat["Skills"].push_back("Religion Etiquette");

  //Skill Definition: Research
  defaults["Research"] = 4;
  skcat["Specialized Skills"].push_back("Research");
  skcat["Medieval Skills"].push_back("Research");
  skcat["Cyberpunk Skills"].push_back("Research");
  skcat["Modern Skills"].push_back("Research");
  skcat["Shadowrun Skills"].push_back("Research");
  skcat["Specialty Skills"].push_back("Research");
  skcat["Skills"].push_back("Research");

  //Skill Definition: Riding
  defaults["Riding"] = 1;
  skcat["Athletic Skills"].push_back("Riding");
  skcat["Medieval Skills"].push_back("Riding");
  skcat["Cyberpunk Skills"].push_back("Riding");
  skcat["Modern Skills"].push_back("Riding");
  skcat["Shadowrun Skills"].push_back("Riding");
  skcat["Skills"].push_back("Riding");

  //Skill Definition: Rifles
  defaults["Rifles"] = 1;
  skcat["Rifle Skills"].push_back("Rifles");
  skcat["Cyberpunk Skills"].push_back("Rifles");
  skcat["Modern Skills"].push_back("Rifles");
  skcat["Shadowrun Skills"].push_back("Rifles");
  skcat["Skills"].push_back("Rifles");
  add_wts("Rifles");

  //Skill Definition: Rigger_Etiquette
  defaults["Rigger Etiquette"] = 3;
  skcat["Etiquette Skills"].push_back("Rigger Etiquette");
  skcat["Shadowrun Skills"].push_back("Rigger Etiquette");
  skcat["Skills"].push_back("Rigger Etiquette");

  //Skill Definition: Ritual_Conjuring
  defaults["Ritual Conjuring"] = 5;
  skcat["Magical Skills"].push_back("Ritual Conjuring");
  skcat["Medieval Skills"].push_back("Ritual Conjuring");
  skcat["Shadowrun Skills"].push_back("Ritual Conjuring");
  skcat["Skills"].push_back("Ritual Conjuring");

  //Skill Definition: Ritual_Spellcasting
  defaults["Ritual Spellcasting"] = 5;
  skcat["Magical Skills"].push_back("Ritual Spellcasting");
  skcat["Medieval Skills"].push_back("Ritual Spellcasting");
  skcat["Shadowrun Skills"].push_back("Ritual Spellcasting");
  skcat["Skills"].push_back("Ritual Spellcasting");

  //Skill Definition: Rod_Targeting
  defaults["Rod Targeting"] = 1;
  skcat["Magical Skills"].push_back("Rod Targeting");
  skcat["Medieval Skills"].push_back("Rod Targeting");
  skcat["Skills"].push_back("Rod Targeting");

  //Skill Definition: Rotor_Drone
  defaults["Rotor Drone"] = 4;
  skcat["Drone Piloting Skills"].push_back("Rotor Drone");
  skcat["Shadowrun Skills"].push_back("Rotor Drone");
  skcat["Skills"].push_back("Rotor Drone");

  //Skill Definition: Rotorcraft
  defaults["Rotorcraft"] = 6;
  skcat["Piloting Skills"].push_back("Rotorcraft");
  skcat["Cyberpunk Skills"].push_back("Rotorcraft");
  skcat["Modern Skills"].push_back("Rotorcraft");
  skcat["Shadowrun Skills"].push_back("Rotorcraft");
  skcat["Skills"].push_back("Rotorcraft");

  //Skill Definition: Running
  defaults["Running"] = 2;
  skcat["Athletic Skills"].push_back("Running");
  skcat["Medieval Skills"].push_back("Running");
  skcat["Cyberpunk Skills"].push_back("Running");
  skcat["Modern Skills"].push_back("Running");
  skcat["Shadowrun Skills"].push_back("Running");
  skcat["Skills"].push_back("Running");

  //Skill Definition: SMGs
  defaults["SMGs"] = 1;
  skcat["Pistol Skills"].push_back("SMGs");
  skcat["Cyberpunk Skills"].push_back("SMGs");
  skcat["Modern Skills"].push_back("SMGs");
  skcat["Shadowrun Skills"].push_back("SMGs");
  skcat["Skills"].push_back("SMGs");
  add_wts("SMGs");

  //Skill Definition: Science_Etiquette
  defaults["Science Etiquette"] = 3;
  skcat["Etiquette Skills"].push_back("Science Etiquette");
  skcat["Cyberpunk Skills"].push_back("Science Etiquette");
  skcat["Modern Skills"].push_back("Science Etiquette");
  skcat["Shadowrun Skills"].push_back("Science Etiquette");
  skcat["Skills"].push_back("Science Etiquette");

  //Skill Definition: Security
  defaults["Security"] = 4;
  skcat["Technical Skills"].push_back("Security");
  skcat["Medieval Skills"].push_back("Security");
  skcat["Cyberpunk Skills"].push_back("Security");
  skcat["Modern Skills"].push_back("Security");
  skcat["Shadowrun Skills"].push_back("Security");
  skcat["Skills"].push_back("Security");

  //Skill Definition: Seduction
  defaults["Seduction"] = 3;
  skcat["Social Skills"].push_back("Seduction");
  skcat["Medieval Skills"].push_back("Seduction");
  skcat["Cyberpunk Skills"].push_back("Seduction");
  skcat["Modern Skills"].push_back("Seduction");
  skcat["Shadowrun Skills"].push_back("Seduction");
  skcat["Skills"].push_back("Seduction");

  //Skill Definition: Separatist_Etiquette
  defaults["Separatist Etiquette"] = 3;
  skcat["Etiquette Skills"].push_back("Separatist Etiquette");
  skcat["Cyberpunk Skills"].push_back("Separatist Etiquette");
  skcat["Modern Skills"].push_back("Separatist Etiquette");
  skcat["Shadowrun Skills"].push_back("Separatist Etiquette");
  skcat["Skills"].push_back("Separatist Etiquette");

  //Skill Definition: Shaman_Etiquette
  defaults["Shaman Etiquette"] = 3;
  skcat["Etiquette Skills"].push_back("Shaman Etiquette");
  skcat["Medieval Skills"].push_back("Shaman Etiquette");
  skcat["Shadowrun Skills"].push_back("Shaman Etiquette");
  skcat["Skills"].push_back("Shaman Etiquette");

  //Skill Definition: Shields
  defaults["Shields"] = 2;
  skcat["Melee-Combat Skills"].push_back("Shields");
  skcat["Medieval Skills"].push_back("Shields");
  skcat["Cyberpunk Skills"].push_back("Shields");
  skcat["Modern Skills"].push_back("Shields");
  skcat["Shadowrun Skills"].push_back("Shields");
  skcat["Skills"].push_back("Shields");
  add_wts("Shields");

  //Skill Definition: Ship
  defaults["Ship"] = 1;
  skcat["Piloting Skills"].push_back("Ship");
  skcat["Medieval Skills"].push_back("Ship");
  skcat["Cyberpunk Skills"].push_back("Ship");
  skcat["Modern Skills"].push_back("Ship");
  skcat["Shadowrun Skills"].push_back("Ship");
  skcat["Skills"].push_back("Ship");

  //Skill Definition: Ship,_Powered
  defaults["Ship, Powered"] = 6;
  skcat["Piloting Skills"].push_back("Ship, Powered");
  skcat["Cyberpunk Skills"].push_back("Ship, Powered");
  skcat["Modern Skills"].push_back("Ship, Powered");
  skcat["Shadowrun Skills"].push_back("Ship, Powered");
  skcat["Skills"].push_back("Ship, Powered");

  //Skill Definition: Shipwright
  defaults["Shipwright"] = 4;
  skcat["Creation Skills"].push_back("Shipwright");
  skcat["Medieval Skills"].push_back("Shipwright");
  skcat["Skills"].push_back("Shipwright");

  //Skill Definition: Short_Blades
  defaults["Short Blades"] = 1;
  skcat["Melee-Combat Skills"].push_back("Short Blades");
  skcat["Medieval Skills"].push_back("Short Blades");
  skcat["Cyberpunk Skills"].push_back("Short Blades");
  skcat["Modern Skills"].push_back("Short Blades");
  skcat["Shadowrun Skills"].push_back("Short Blades");
  skcat["Skills"].push_back("Short Blades");
  add_wts("Short Blades");

  //Skill Definition: Short_Cleaves
  defaults["Short Cleaves"] = 1;
  skcat["Melee-Combat Skills"].push_back("Short Cleaves");
  skcat["Medieval Skills"].push_back("Short Cleaves");
  skcat["Cyberpunk Skills"].push_back("Short Cleaves");
  skcat["Modern Skills"].push_back("Short Cleaves");
  skcat["Shadowrun Skills"].push_back("Short Cleaves");
  skcat["Skills"].push_back("Short Cleaves");
  add_wts("Short Cleaves");

  //Skill Definition: Short_Crushing
  defaults["Short Crushing"] = 1;
  skcat["Melee-Combat Skills"].push_back("Short Crushing");
  skcat["Medieval Skills"].push_back("Short Crushing");
  skcat["Cyberpunk Skills"].push_back("Short Crushing");
  skcat["Modern Skills"].push_back("Short Crushing");
  skcat["Shadowrun Skills"].push_back("Short Crushing");
  skcat["Skills"].push_back("Short Crushing");
  add_wts("Short Crushing");

  //Skill Definition: Short_Flails
  defaults["Short Flails"] = 1;
  skcat["Melee-Combat Skills"].push_back("Short Flails");
  skcat["Medieval Skills"].push_back("Short Flails");
  skcat["Cyberpunk Skills"].push_back("Short Flails");
  skcat["Modern Skills"].push_back("Short Flails");
  skcat["Shadowrun Skills"].push_back("Short Flails");
  skcat["Skills"].push_back("Short Flails");
  add_wts("Short Flails");

  //Skill Definition: Short_Piercing
  defaults["Short Piercing"] = 1;
  skcat["Melee-Combat Skills"].push_back("Short Piercing");
  skcat["Medieval Skills"].push_back("Short Piercing");
  skcat["Cyberpunk Skills"].push_back("Short Piercing");
  skcat["Modern Skills"].push_back("Short Piercing");
  skcat["Shadowrun Skills"].push_back("Short Piercing");
  skcat["Skills"].push_back("Short Piercing");
  add_wts("Short Piercing");

  //Skill Definition: Shot_Pistols
  defaults["Shot Pistols"] = 1;
  skcat["Pistol Skills"].push_back("Shot Pistols");
  skcat["Cyberpunk Skills"].push_back("Shot Pistols");
  skcat["Modern Skills"].push_back("Shot Pistols");
  skcat["Shadowrun Skills"].push_back("Shot Pistols");
  skcat["Skills"].push_back("Shot Pistols");
  add_wts("Shot Pistols");

  //Skill Definition: Shotguns
  defaults["Shotguns"] = 1;
  skcat["Rifle Skills"].push_back("Shotguns");
  skcat["Cyberpunk Skills"].push_back("Shotguns");
  skcat["Modern Skills"].push_back("Shotguns");
  skcat["Shadowrun Skills"].push_back("Shotguns");
  skcat["Skills"].push_back("Shotguns");
  add_wts("Shotguns");

  //Skill Definition: Skiing
  defaults["Skiing"] = 1;
  skcat["Athletic Skills"].push_back("Skiing");
  skcat["Medieval Skills"].push_back("Skiing");
  skcat["Cyberpunk Skills"].push_back("Skiing");
  skcat["Modern Skills"].push_back("Skiing");
  skcat["Shadowrun Skills"].push_back("Skiing");
  skcat["Skills"].push_back("Skiing");

  //Skill Definition: Sled
  defaults["Sled"] = 1;
  skcat["Piloting Skills"].push_back("Sled");
  skcat["Medieval Skills"].push_back("Sled");
  skcat["Cyberpunk Skills"].push_back("Sled");
  skcat["Modern Skills"].push_back("Sled");
  skcat["Shadowrun Skills"].push_back("Sled");
  skcat["Skills"].push_back("Sled");

  //Skill Definition: Slings
  defaults["Slings"] = 1;
  skcat["Ranged-Combat Skills"].push_back("Slings");
  skcat["Medieval Skills"].push_back("Slings");
  skcat["Cyberpunk Skills"].push_back("Slings");
  skcat["Modern Skills"].push_back("Slings");
  skcat["Shadowrun Skills"].push_back("Slings");
  skcat["Skills"].push_back("Slings");
  add_wts("Slings");

  //Skill Definition: Smelting
  defaults["Smelting"] = 4;
  skcat["Creation Skills"].push_back("Smelting");
  skcat["Medieval Skills"].push_back("Smelting");
  skcat["Skills"].push_back("Smelting");

  //Skill Definition: Snowmobile
  defaults["Snowmobile"] = 6;
  skcat["Piloting Skills"].push_back("Snowmobile");
  skcat["Cyberpunk Skills"].push_back("Snowmobile");
  skcat["Modern Skills"].push_back("Snowmobile");
  skcat["Shadowrun Skills"].push_back("Snowmobile");
  skcat["Skills"].push_back("Snowmobile");

  //Skill Definition: Spacecraft
  defaults["Spacecraft"] = 4;
  skcat["Piloting Skills"].push_back("Spacecraft");
  skcat["Cyberpunk Skills"].push_back("Spacecraft");
  skcat["Modern Skills"].push_back("Spacecraft");
  skcat["Shadowrun Skills"].push_back("Spacecraft");
  skcat["Skills"].push_back("Spacecraft");

  //Skill Definition: Spell_Targeting
  defaults["Spell Targeting"] = 1;
  skcat["Magical Skills"].push_back("Spell Targeting");
  skcat["Medieval Skills"].push_back("Spell Targeting");
  skcat["Shadowrun Skills"].push_back("Spell Targeting");
  skcat["Skills"].push_back("Spell Targeting");

  //Skill Definition: Spellcasting
  defaults["Spellcasting"] = 4;
  skcat["Magical Skills"].push_back("Spellcasting");
  skcat["Medieval Skills"].push_back("Spellcasting");
  skcat["Shadowrun Skills"].push_back("Spellcasting");
  skcat["Skills"].push_back("Spellcasting");

  //Skill Definition: Spellcraft
  defaults["Spellcraft"] = 4;
  skcat["Magical Skills"].push_back("Spellcraft");
  skcat["Medieval Skills"].push_back("Spellcraft");
  skcat["Shadowrun Skills"].push_back("Spellcraft");
  skcat["Skills"].push_back("Spellcraft");

  //Skill Definition: Sport_Wrestling
  defaults["Sport Wrestling"] = 2;
  skcat["Sport Martial Arts Skills"].push_back("Sport Wrestling");
  skcat["Cyberpunk Skills"].push_back("Sport Wrestling");
  skcat["Modern Skills"].push_back("Sport Wrestling");
  skcat["Shadowrun Skills"].push_back("Sport Wrestling");
  skcat["Skills"].push_back("Sport Wrestling");

  //Skill Definition: Sprinting
  defaults["Sprinting"] = 1;
  skcat["Athletic Skills"].push_back("Sprinting");
  skcat["Medieval Skills"].push_back("Sprinting");
  skcat["Cyberpunk Skills"].push_back("Sprinting");
  skcat["Modern Skills"].push_back("Sprinting");
  skcat["Shadowrun Skills"].push_back("Sprinting");
  skcat["Skills"].push_back("Sprinting");

  //Skill Definition: Staff_Slings
  defaults["Staff Slings"] = 1;
  skcat["Ranged-Combat Skills"].push_back("Staff Slings");
  skcat["Medieval Skills"].push_back("Staff Slings");
  skcat["Cyberpunk Skills"].push_back("Staff Slings");
  skcat["Modern Skills"].push_back("Staff Slings");
  skcat["Shadowrun Skills"].push_back("Staff Slings");
  skcat["Skills"].push_back("Staff Slings");
  add_wts("Staff Slings");

  //Skill Definition: Staff_Targeting
  defaults["Staff Targeting"] = 1;
  skcat["Magical Skills"].push_back("Staff Targeting");
  skcat["Medieval Skills"].push_back("Staff Targeting");
  skcat["Skills"].push_back("Staff Targeting");

  //Skill Definition: Staves
  defaults["Staves"] = 1;
  skcat["Melee-Combat Skills"].push_back("Staves");
  skcat["Medieval Skills"].push_back("Staves");
  skcat["Cyberpunk Skills"].push_back("Staves");
  skcat["Modern Skills"].push_back("Staves");
  skcat["Shadowrun Skills"].push_back("Staves");
  skcat["Skills"].push_back("Staves");
  add_wts("Staves");

  //Skill Definition: Stealth
  defaults["Stealth"] = 1;
  skcat["Athletic Skills"].push_back("Stealth");
  skcat["Medieval Skills"].push_back("Stealth");
  skcat["Cyberpunk Skills"].push_back("Stealth");
  skcat["Modern Skills"].push_back("Stealth");
  skcat["Shadowrun Skills"].push_back("Stealth");
  skcat["Skills"].push_back("Stealth");

  //Skill Definition: Street_Etiquette
  defaults["Street Etiquette"] = 3;
  skcat["Etiquette Skills"].push_back("Street Etiquette");
  skcat["Cyberpunk Skills"].push_back("Street Etiquette");
  skcat["Modern Skills"].push_back("Street Etiquette");
  skcat["Shadowrun Skills"].push_back("Street Etiquette");
  skcat["Skills"].push_back("Street Etiquette");

  //Skill Definition: Style
  defaults["Style"] = 3;
  skcat["Social Skills"].push_back("Style");
  skcat["Medieval Skills"].push_back("Style");
  skcat["Cyberpunk Skills"].push_back("Style");
  skcat["Modern Skills"].push_back("Style");
  skcat["Shadowrun Skills"].push_back("Style");
  skcat["Skills"].push_back("Style");

  //Skill Definition: Submission_Wrestling
  defaults["Submission Wrestling"] = 4;
  skcat["Specialized Hand-to-Hand Skills"].push_back("Submission Wrestling");
  skcat["Medieval Skills"].push_back("Submission Wrestling");
  skcat["Cyberpunk Skills"].push_back("Submission Wrestling");
  skcat["Modern Skills"].push_back("Submission Wrestling");
  skcat["Shadowrun Skills"].push_back("Submission Wrestling");
  skcat["Skills"].push_back("Submission Wrestling");

  //Skill Definition: Sumo-Wrestling
  defaults["Sumo-Wrestling"] = 0;
  skcat["Sport Martial Arts Skills"].push_back("Sumo-Wrestling");
  skcat["Cyberpunk Skills"].push_back("Sumo-Wrestling");
  skcat["Modern Skills"].push_back("Sumo-Wrestling");
  skcat["Shadowrun Skills"].push_back("Sumo-Wrestling");
  skcat["Skills"].push_back("Sumo-Wrestling");

  //Skill Definition: Surgery
  defaults["Surgery"] = 4;
  skcat["Technical Skills"].push_back("Surgery");
  skcat["Cyberpunk Skills"].push_back("Surgery");
  skcat["Modern Skills"].push_back("Surgery");
  skcat["Shadowrun Skills"].push_back("Surgery");
  skcat["Skills"].push_back("Surgery");

  //Skill Definition: Survival
  defaults["Survival"] = 4;
  skcat["Nature Skills"].push_back("Survival");
  skcat["Medieval Skills"].push_back("Survival");
  skcat["Cyberpunk Skills"].push_back("Survival");
  skcat["Modern Skills"].push_back("Survival");
  skcat["Shadowrun Skills"].push_back("Survival");
  skcat["Skills"].push_back("Survival");

  //Skill Definition: Swimming
  defaults["Swimming"] = 1;
  skcat["Athletic Skills"].push_back("Swimming");
  skcat["Medieval Skills"].push_back("Swimming");
  skcat["Cyberpunk Skills"].push_back("Swimming");
  skcat["Modern Skills"].push_back("Swimming");
  skcat["Shadowrun Skills"].push_back("Swimming");
  skcat["Skills"].push_back("Swimming");

  //Skill Definition: Swordsmithing
  defaults["Swordsmithing"] = 4;
  skcat["Creation Skills"].push_back("Swordsmithing");
  skcat["Medieval Skills"].push_back("Swordsmithing");
  skcat["Skills"].push_back("Swordsmithing");

  //Skill Definition: Tactics
  defaults["Tactics"] = 4;
  skcat["Specialized Skills"].push_back("Tactics");
  skcat["Medieval Skills"].push_back("Tactics");
  skcat["Cyberpunk Skills"].push_back("Tactics");
  skcat["Modern Skills"].push_back("Tactics");
  skcat["Shadowrun Skills"].push_back("Tactics");
  skcat["Skills"].push_back("Tactics");

  //Skill Definition: Tae_Kwon_Do
  defaults["Tae Kwon Do"] = 1;
  skcat["Martial Arts Skills"].push_back("Tae Kwon Do");
  skcat["Cyberpunk Skills"].push_back("Tae Kwon Do");
  skcat["Modern Skills"].push_back("Tae Kwon Do");
  skcat["Shadowrun Skills"].push_back("Tae Kwon Do");
  skcat["Skills"].push_back("Tae Kwon Do");

  //Skill Definition: Talismongery
  defaults["Talismongery"] = 5;
  skcat["Magical Skills"].push_back("Talismongery");
  skcat["Medieval Skills"].push_back("Talismongery");
  skcat["Shadowrun Skills"].push_back("Talismongery");
  skcat["Skills"].push_back("Talismongery");

  //Skill Definition: Teamster
  defaults["Teamster"] = 3;
  skcat["Piloting Skills"].push_back("Teamster");
  skcat["Medieval Skills"].push_back("Teamster");
  skcat["Cyberpunk Skills"].push_back("Teamster");
  skcat["Modern Skills"].push_back("Teamster");
  skcat["Shadowrun Skills"].push_back("Teamster");
  skcat["Skills"].push_back("Teamster");

  //Skill Definition: Thief_Etiquette
  defaults["Thief Etiquette"] = 3;
  skcat["Etiquette Skills"].push_back("Thief Etiquette");
  skcat["Medieval Skills"].push_back("Thief Etiquette");
  skcat["Skills"].push_back("Thief Etiquette");

  //Skill Definition: Throwing,_Aero
  defaults["Throwing, Aero"] = 1;
  skcat["Ranged-Combat Skills"].push_back("Throwing, Aero");
  skcat["Medieval Skills"].push_back("Throwing, Aero");
  skcat["Cyberpunk Skills"].push_back("Throwing, Aero");
  skcat["Modern Skills"].push_back("Throwing, Aero");
  skcat["Shadowrun Skills"].push_back("Throwing, Aero");
  skcat["Skills"].push_back("Throwing, Aero");
  add_wts("Throwing, Aero");

  //Skill Definition: Throwing,_Non-Aero
  defaults["Throwing, Non-Aero"] = 1;
  skcat["Ranged-Combat Skills"].push_back("Throwing, Non-Aero");
  skcat["Medieval Skills"].push_back("Throwing, Non-Aero");
  skcat["Cyberpunk Skills"].push_back("Throwing, Non-Aero");
  skcat["Modern Skills"].push_back("Throwing, Non-Aero");
  skcat["Shadowrun Skills"].push_back("Throwing, Non-Aero");
  skcat["Skills"].push_back("Throwing, Non-Aero");
  add_wts("Throwing, Non-Aero");

  //Skill Definition: Tracked_Drone
  defaults["Tracked Drone"] = 4;
  skcat["Drone Piloting Skills"].push_back("Tracked Drone");
  skcat["Shadowrun Skills"].push_back("Tracked Drone");
  skcat["Skills"].push_back("Tracked Drone");

  //Skill Definition: Tracked_Vehicle
  defaults["Tracked Vehicle"] = 6;
  skcat["Piloting Skills"].push_back("Tracked Vehicle");
  skcat["Cyberpunk Skills"].push_back("Tracked Vehicle");
  skcat["Modern Skills"].push_back("Tracked Vehicle");
  skcat["Shadowrun Skills"].push_back("Tracked Vehicle");
  skcat["Skills"].push_back("Tracked Vehicle");

  //Skill Definition: Tracking
  defaults["Tracking"] = 4;
  skcat["Specialized Skills"].push_back("Tracking");
  skcat["Medieval Skills"].push_back("Tracking");
  skcat["Cyberpunk Skills"].push_back("Tracking");
  skcat["Modern Skills"].push_back("Tracking");
  skcat["Shadowrun Skills"].push_back("Tracking");
  skcat["Skills"].push_back("Tracking");

  //Skill Definition: Tractor
  defaults["Tractor"] = 6;
  skcat["Piloting Skills"].push_back("Tractor");
  skcat["Cyberpunk Skills"].push_back("Tractor");
  skcat["Modern Skills"].push_back("Tractor");
  skcat["Shadowrun Skills"].push_back("Tractor");
  skcat["Skills"].push_back("Tractor");

  //Skill Definition: Treatment
  defaults["Treatment"] = 4;
  skcat["Technical Skills"].push_back("Treatment");
  skcat["Cyberpunk Skills"].push_back("Treatment");
  skcat["Modern Skills"].push_back("Treatment");
  skcat["Shadowrun Skills"].push_back("Treatment");
  skcat["Skills"].push_back("Treatment");

  //Skill Definition: Trike
  defaults["Trike"] = 6;
  skcat["Piloting Skills"].push_back("Trike");
  skcat["Cyberpunk Skills"].push_back("Trike");
  skcat["Modern Skills"].push_back("Trike");
  skcat["Shadowrun Skills"].push_back("Trike");
  skcat["Skills"].push_back("Trike");

  //Skill Definition: Trucker_Etiquette
  defaults["Trucker Etiquette"] = 3;
  skcat["Etiquette Skills"].push_back("Trucker Etiquette");
  skcat["Cyberpunk Skills"].push_back("Trucker Etiquette");
  skcat["Modern Skills"].push_back("Trucker Etiquette");
  skcat["Skills"].push_back("Trucker Etiquette");

  //Skill Definition: Two-Handed_Blades
  defaults["Two-Handed Blades"] = 2;
  skcat["Melee-Combat Skills"].push_back("Two-Handed Blades");
  skcat["Medieval Skills"].push_back("Two-Handed Blades");
  skcat["Cyberpunk Skills"].push_back("Two-Handed Blades");
  skcat["Modern Skills"].push_back("Two-Handed Blades");
  skcat["Shadowrun Skills"].push_back("Two-Handed Blades");
  skcat["Skills"].push_back("Two-Handed Blades");
  add_wts("Two-Handed Blades");

  //Skill Definition: Two-Handed_Cleaves
  defaults["Two-Handed Cleaves"] = 2;
  skcat["Melee-Combat Skills"].push_back("Two-Handed Cleaves");
  skcat["Medieval Skills"].push_back("Two-Handed Cleaves");
  skcat["Cyberpunk Skills"].push_back("Two-Handed Cleaves");
  skcat["Modern Skills"].push_back("Two-Handed Cleaves");
  skcat["Shadowrun Skills"].push_back("Two-Handed Cleaves");
  skcat["Skills"].push_back("Two-Handed Cleaves");
  add_wts("Two-Handed Cleaves");

  //Skill Definition: Two-Handed_Crushing
  defaults["Two-Handed Crushing"] = 2;
  skcat["Melee-Combat Skills"].push_back("Two-Handed Crushing");
  skcat["Medieval Skills"].push_back("Two-Handed Crushing");
  skcat["Cyberpunk Skills"].push_back("Two-Handed Crushing");
  skcat["Modern Skills"].push_back("Two-Handed Crushing");
  skcat["Shadowrun Skills"].push_back("Two-Handed Crushing");
  skcat["Skills"].push_back("Two-Handed Crushing");
  add_wts("Two-Handed Crushing");

  //Skill Definition: Two-Handed_Flails
  defaults["Two-Handed Flails"] = 2;
  skcat["Melee-Combat Skills"].push_back("Two-Handed Flails");
  skcat["Medieval Skills"].push_back("Two-Handed Flails");
  skcat["Cyberpunk Skills"].push_back("Two-Handed Flails");
  skcat["Modern Skills"].push_back("Two-Handed Flails");
  skcat["Shadowrun Skills"].push_back("Two-Handed Flails");
  skcat["Skills"].push_back("Two-Handed Flails");
  add_wts("Two-Handed Flails");

  //Skill Definition: Two-Handed_Piercing
  defaults["Two-Handed Piercing"] = 1;
  skcat["Melee-Combat Skills"].push_back("Two-Handed Piercing");
  skcat["Medieval Skills"].push_back("Two-Handed Piercing");
  skcat["Cyberpunk Skills"].push_back("Two-Handed Piercing");
  skcat["Modern Skills"].push_back("Two-Handed Piercing");
  skcat["Shadowrun Skills"].push_back("Two-Handed Piercing");
  skcat["Skills"].push_back("Two-Handed Piercing");
  add_wts("Two-Handed Piercing");

  //Skill Definition: Two-Handed_Staves
  defaults["Two-Handed Staves"] = 1;
  skcat["Melee-Combat Skills"].push_back("Two-Handed Staves");
  skcat["Medieval Skills"].push_back("Two-Handed Staves");
  skcat["Cyberpunk Skills"].push_back("Two-Handed Staves");
  skcat["Modern Skills"].push_back("Two-Handed Staves");
  skcat["Shadowrun Skills"].push_back("Two-Handed Staves");
  skcat["Skills"].push_back("Two-Handed Staves");
  add_wts("Two-Handed Staves");

  //Skill Definition: Underwater_Combat
  defaults["Underwater Combat"] = 4;
  skcat["Combat Environment Skills"].push_back("Underwater Combat");
  skcat["Medieval Skills"].push_back("Underwater Combat");
  skcat["Cyberpunk Skills"].push_back("Underwater Combat");
  skcat["Modern Skills"].push_back("Underwater Combat");
  skcat["Shadowrun Skills"].push_back("Underwater Combat");
  skcat["Skills"].push_back("Underwater Combat");

  //Skill Definition: Underworld_Etiquette
  defaults["Underworld Etiquette"] = 3;
  skcat["Etiquette Skills"].push_back("Underworld Etiquette");
  skcat["Medieval Skills"].push_back("Underworld Etiquette");
  skcat["Skills"].push_back("Underworld Etiquette");

  //Skill Definition: University_Etiquette
  defaults["University Etiquette"] = 3;
  skcat["Etiquette Skills"].push_back("University Etiquette");
  skcat["Cyberpunk Skills"].push_back("University Etiquette");
  skcat["Modern Skills"].push_back("University Etiquette");
  skcat["Shadowrun Skills"].push_back("University Etiquette");
  skcat["Skills"].push_back("University Etiquette");

  //Skill Definition: Vectored_Thrust
  defaults["Vectored Thrust"] = 6;
  skcat["Piloting Skills"].push_back("Vectored Thrust");
  skcat["Cyberpunk Skills"].push_back("Vectored Thrust");
  skcat["Modern Skills"].push_back("Vectored Thrust");
  skcat["Shadowrun Skills"].push_back("Vectored Thrust");
  skcat["Skills"].push_back("Vectored Thrust");

  //Skill Definition: Vectored_Thrust_Drone
  defaults["Vectored Thrust Drone"] = 4;
  skcat["Drone Piloting Skills"].push_back("Vectored Thrust Drone");
  skcat["Shadowrun Skills"].push_back("Vectored Thrust Drone");
  skcat["Skills"].push_back("Vectored Thrust Drone");

  //Skill Definition: Wand_Targeting
  defaults["Wand Targeting"] = 1;
  skcat["Magical Skills"].push_back("Wand Targeting");
  skcat["Medieval Skills"].push_back("Wand Targeting");
  skcat["Skills"].push_back("Wand Targeting");

  //Skill Definition: Wheeled_Drone
  defaults["Wheeled Drone"] = 4;
  skcat["Drone Piloting Skills"].push_back("Wheeled Drone");
  skcat["Shadowrun Skills"].push_back("Wheeled Drone");
  skcat["Skills"].push_back("Wheeled Drone");

  //Skill Definition: Whips
  defaults["Whips"] = 1;
  skcat["Ranged-Combat Skills"].push_back("Whips");
  skcat["Medieval Skills"].push_back("Whips");
  skcat["Cyberpunk Skills"].push_back("Whips");
  skcat["Modern Skills"].push_back("Whips");
  skcat["Shadowrun Skills"].push_back("Whips");
  skcat["Skills"].push_back("Whips");
  add_wts("Whips");

  //Skill Definition: Wildcat
  defaults["Wildcat"] = 1;
  skcat["Martial Arts Skills"].push_back("Wildcat");
  skcat["Cyberpunk Skills"].push_back("Wildcat");
  skcat["Modern Skills"].push_back("Wildcat");
  skcat["Shadowrun Skills"].push_back("Wildcat");
  skcat["Skills"].push_back("Wildcat");

  //Skill Definition: Woodworking
  defaults["Woodworking"] = 4;
  skcat["Creation Skills"].push_back("Woodworking");
  skcat["Medieval Skills"].push_back("Woodworking");
  skcat["Skills"].push_back("Woodworking");

  //Skill Definition: Zero-G_Combat
  defaults["Zero-G Combat"] = 4;
  skcat["Combat Environment Skills"].push_back("Zero-G Combat");
  skcat["Medieval Skills"].push_back("Zero-G Combat");
  skcat["Cyberpunk Skills"].push_back("Zero-G Combat");
  skcat["Modern Skills"].push_back("Zero-G Combat");
  skcat["Shadowrun Skills"].push_back("Zero-G Combat");
  skcat["Skills"].push_back("Zero-G Combat");

  //Skill Definition: Zero-G_Ops
  defaults["Zero-G Ops"] = 4;
  skcat["Specialized Skills"].push_back("Zero-G Ops");
  skcat["Medieval Skills"].push_back("Zero-G Ops");
  skcat["Cyberpunk Skills"].push_back("Zero-G Ops");
  skcat["Modern Skills"].push_back("Zero-G Ops");
  skcat["Shadowrun Skills"].push_back("Zero-G Ops");
  skcat["Skills"].push_back("Zero-G Ops");
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

int roll(int ndice, int targ, list<int> *wraps) {
  int ret = 0, ctr;
  for(ctr=0; ctr<ndice; ++ctr) {
    int val = 1+(rand()%6);
    while((val%6) == 0) val += (1+rand()%6);
    if(val > 1 && val >= targ) {
      unsigned int numwrap = (unsigned int)((val - targ) / 12);
      ret += (1 + int(numwrap));
      if(wraps && numwrap > 0) {
	if(wraps->size() < numwrap) wraps->resize(numwrap, 0);
	list<int>::iterator wit = wraps->begin();
	for(unsigned int i=0; i < numwrap; ++i, ++wit) {
	  (*wit) += numwrap - i;
	  }
	}
      }
    }
  return ret;
  }

int Object::Attribute(int a) const {
  return att[a];
  }

void Object::SetAttribute(int a, int v) {
  if(v > 1000000000) v = 1000000000;
  else if(v < -1000000000) v = -1000000000;
  att[a] = v;
  }

void Object::SetSkill(const string &s, int v) {
  if(v > 1000000000) v = 1000000000;
  else if(v < -1000000000) v = -1000000000;
  if(v <= 0) skills.erase(s);
  else skills[s] = v;
  }

int Object::Skill(const string &s, int *tnum) const {
  if(strlen(s.c_str()) == 0) return 0;
  if(!strncasecmp(s.c_str(), "Body", s.length())) return att[0];
  if(!strncasecmp(s.c_str(), "Quickness", s.length())) return att[1];
  if(!strncasecmp(s.c_str(), "Strength", s.length())) return att[2];
  if(!strncasecmp(s.c_str(), "Charisma", s.length())) return att[3];
  if(!strncasecmp(s.c_str(), "Intelligence", s.length())) return att[4];
  if(!strncasecmp(s.c_str(), "Willpower", s.length())) return att[5];
  if(!strncasecmp(s.c_str(), "Reaction", s.length())) return (att[1]+att[4])/2;
  if(!defaults_init) init_defaults();
  if(skills.count(s)) return (skills.find(s))->second;  //const for 'skills[s]'
  if(tnum) {
    (*tnum) += 4;
    return att[defaults[s]];
    }
  return 0;
  }

int Object::Roll(const string &s1, const Object *p2, const string &s2, int bias, string *res) const {
  return Roll(s1, p2, s2, bias, NULL, res);
  }

int Object::Roll(const string &s1, const Object *p2, const string &s2, int bias, list<int> *wraps, string *res) const {
  int succ = 0;

  int t1 = p2->Skill(s2) - bias;
  int t2 = Skill(s1) + bias;

  if(res) (*res) += "(";
  succ = Roll(s1, t1, wraps, res);
  if(s2 != "") {
    if(res) (*res) += " - ";
    succ -= p2->Roll(s2, t2, res);	//FIXME: Contested Wraps!
    }
  if(res) (*res) += ")";
  return succ;
  }

int Object::Roll(const string &s1, int targ, string *res) const {
  return Roll(s1, targ, NULL, res);
  }

int Object::Roll(const string &s1, int targ, list<int> *wraps, string *res) const {
  if(phys>=10 || stun>=10 || (!(att[0]*att[1]*att[2]*att[3]*att[4]*att[5]))) {
    if(res) (*res) += "N/A";
    return 0;
    }
  targ += WoundPenalty();
  return RollNoWounds(s1, targ, wraps, res);
  }

int Object::RollNoWounds(const string &s1, int targ, string *res) const {
  return RollNoWounds(s1, targ, NULL, res);
  }

int Object::RollNoWounds(const string &s1, int targ, list<int> *wraps, string *res) const {
  int succ = 0;
  int d1 = Skill(s1, &targ);
  succ = roll(abs(d1), targ, wraps);

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

list<int> Object::RollInitiative() const {
  list<int> ret;
  int start = Roll("Reaction", 6 - att[5], &ret);
  ret.push_front(start);

  /* Begin Debug Output */
//  if(IsAct(ACT_FIGHT)) {
//    fprintf(stderr, "Initiative: [");
//    for(list<int>::iterator it = ret.begin(); it != ret.end();) {
//      fprintf(stderr, "%d", *it);
//      ++it;
//      if(it != ret.end()) fprintf(stderr, ", ");
//      }
//    fprintf(stderr, "] (%s)\n", Name());
//    }
  /* End Debug Output */

  return ret;
  }
