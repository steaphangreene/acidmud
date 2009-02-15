struct skill_entry {
  const char *skcatname;
  const char *skname;
  int linked_attr;
  };

#define CUR_CAT "None"
static skill_entry skill_data[] = {
  #undef CUR_CAT
  #define CUR_CAT "Athletic Skills"
  { CUR_CAT, "Acrobatics", 1 },
  { CUR_CAT, "Catching", 1 },
  { CUR_CAT, "Climbing", 2 },
  { CUR_CAT, "Flight", 1 },
  { CUR_CAT, "Jumping", 2 },
  { CUR_CAT, "Lifting", 2 },
  { CUR_CAT, "Riding", 1 },
  { CUR_CAT, "Running", 2 },
  { CUR_CAT, "Skiing", 1 },
  { CUR_CAT, "Sprinting", 1 },
  { CUR_CAT, "Stealth", 1 },
  { CUR_CAT, "Swimming", 1 },
  { CUR_CAT, "Diving", 1 },
  #undef CUR_CAT
  #define CUR_CAT "Ranged-Combat Skills"
  { CUR_CAT, "Archery", 1 },
  { CUR_CAT, "Blowgun", 1 },
  { CUR_CAT, "Crossbow", 1 },
  { CUR_CAT, "Hurling", 2 },
  { CUR_CAT, "Nets", 1 },
  { CUR_CAT, "Slings", 1 },
  { CUR_CAT, "Staff Slings", 1 },
  { CUR_CAT, "Throwing, Aero", 1 },
  { CUR_CAT, "Throwing, Non-Aero", 1 },
  { CUR_CAT, "Whips", 1 },
  #undef CUR_CAT
  #define CUR_CAT "Pistol Skills"
  { CUR_CAT, "Air Pistols", 1 },
  { CUR_CAT, "Laser Pistols", 1 },
  { CUR_CAT, "Machine Pistols", 1 },
  { CUR_CAT, "Pistols", 1 },
  { CUR_CAT, "Plasma Pistols", 1 },
  { CUR_CAT, "Shot Pistols", 1 },
  { CUR_CAT, "SMGs", 1 },
  #undef CUR_CAT
  #define CUR_CAT "Rifle Skills"
  { CUR_CAT, "Air Rifles", 1 },
  { CUR_CAT, "Assault Rifles", 1 },
  { CUR_CAT, "Laser Rifles", 1 },
  { CUR_CAT, "Plasma Rifles", 1 },
  { CUR_CAT, "Rifles", 1 },
  { CUR_CAT, "Shotguns", 1 },
  #undef CUR_CAT
  #define CUR_CAT "Heavy Firearm Skills"
  { CUR_CAT, "Gunnery", 4 },
  { CUR_CAT, "Heavy Firearms", 2 },
  { CUR_CAT, "Heavy Lasers", 2 },
  { CUR_CAT, "Launch Weapons", 4 },
  #undef CUR_CAT
  #define CUR_CAT "Mounted Ranged-Combat Skills"
  { CUR_CAT, "Mounted Archery", 1 },
  { CUR_CAT, "Mounted Blowgun", 1 },
  { CUR_CAT, "Mounted Crossbow", 1 },
  { CUR_CAT, "Mounted Nets", 1 },
  { CUR_CAT, "Mounted Slings", 1 },
  { CUR_CAT, "Mounted Throwing", 1 },
  { CUR_CAT, "Mounted Whips", 1 },
  #undef CUR_CAT
  #define CUR_CAT "Mounted Pistol Skills"
  { CUR_CAT, "Mounted Air Pistols", 1 },
  { CUR_CAT, "Mounted Laser Pistols", 1 },
  { CUR_CAT, "Mounted Machine Pistols", 1 },
  { CUR_CAT, "Mounted Pistols", 1 },
  { CUR_CAT, "Mounted Plasma Pistols", 1 },
  { CUR_CAT, "Mounted Shot Pistols", 1 },
  { CUR_CAT, "Mounted SMGs", 1 },
  #undef CUR_CAT
  #define CUR_CAT "Hand-to-Hand Combat Skills"
  { CUR_CAT, "Grappling", 2 },
  { CUR_CAT, "Kicking", 1 },
  { CUR_CAT, "Punching", 1 },
  #undef CUR_CAT
  #define CUR_CAT "Specialized Hand-to-Hand Skills"
  { CUR_CAT, "Brawling", 5 },
  { CUR_CAT, "Submission Wrestling", 4 },
  #undef CUR_CAT
  #define CUR_CAT "Melee-Combat Skills"
  { CUR_CAT, "Offhand Blades", 1 },
  { CUR_CAT, "Offhand Crushing", 1 },
  { CUR_CAT, "Offhand Cleaves", 1 },
  { CUR_CAT, "Offhand Flails", 1 },
  { CUR_CAT, "Offhand Piercing", 1 },
  { CUR_CAT, "Offhand Staves", 1 },
  { CUR_CAT, "Short Blades", 1 },
  { CUR_CAT, "Short Cleaves", 1 },
  { CUR_CAT, "Short Crushing", 1 },
  { CUR_CAT, "Short Piercing", 1 },
  { CUR_CAT, "Flails", 1 },
  { CUR_CAT, "Staves", 1 },
  { CUR_CAT, "Long Blades", 1 },
  { CUR_CAT, "Long Cleaves", 1 },
  { CUR_CAT, "Long Crushing", 1 },
  { CUR_CAT, "Long Piercing", 1 },
  { CUR_CAT, "Two-Handed Blades", 2 },
  { CUR_CAT, "Two-Handed Crushing", 2 },
  { CUR_CAT, "Two-Handed Cleaves", 2 },
  { CUR_CAT, "Two-Handed Flails", 2 },
  { CUR_CAT, "Two-Handed Piercing", 1 },
  { CUR_CAT, "Two-Handed Staves", 1 },
  { CUR_CAT, "Shields", 2 },
  #undef CUR_CAT
  #define CUR_CAT "Mounted Melee-Combat Skills"
  { CUR_CAT, "Mounted Blades", 1 },
  { CUR_CAT, "Mounted Crushing", 1 },
  { CUR_CAT, "Mounted Cleaves", 1 },
  { CUR_CAT, "Mounted Flails", 1 },
  { CUR_CAT, "Mounted Piercing", 1 },
  #undef CUR_CAT
  #define CUR_CAT "Piloting Skills"
  { CUR_CAT, "Bike", 4 },
  { CUR_CAT, "Boat", 2 },
  { CUR_CAT, "Boat, Powered", 4 },
  { CUR_CAT, "Car", 4 },
  { CUR_CAT, "Fixed-wing Craft", 4 },
  { CUR_CAT, "Half-Track", 4 },
  { CUR_CAT, "Hovercraft", 4 },
  { CUR_CAT, "Rotorcraft", 4 },
  { CUR_CAT, "Ship", 1 },
  { CUR_CAT, "Ship, Powered", 4 },
  { CUR_CAT, "Sled", 1 },
  { CUR_CAT, "Snowmobile", 4 },
  { CUR_CAT, "Spacecraft", 4 },
  { CUR_CAT, "Teamster", 3 },
  { CUR_CAT, "Tracked Vehicle", 4 },
  { CUR_CAT, "Tractor", 4 },
  { CUR_CAT, "Trike", 4 },
  { CUR_CAT, "Vectored Thrust", 4 },
  #undef CUR_CAT
  #define CUR_CAT "Drone Piloting Skills"
  { CUR_CAT, "Fixed-Wing Drone", 4 },
  { CUR_CAT, "Hover Drone", 4 },
  { CUR_CAT, "Rotor Drone", 4 },
  { CUR_CAT, "Tracked Drone", 4 },
  { CUR_CAT, "Vectored Thrust Drone", 4 },
  { CUR_CAT, "Wheeled Drone", 4 },
  #undef CUR_CAT
  #define CUR_CAT "Elven Martial Arts"
  { CUR_CAT, "Carromeleg - Tier I", 0 },
  { CUR_CAT, "Carromeleg - Tier II", 1 },
  { CUR_CAT, "Carromeleg - Tier III", 2 },
  { CUR_CAT, "Carromeleg - Tier IV", 3 },
  { CUR_CAT, "Carromeleg - Tier V", 4 },
  { CUR_CAT, "Carromeleg - Tier VI", 5 },
  #undef CUR_CAT
  #define CUR_CAT "Martial Arts"
  { CUR_CAT, "Aikido", 4 },
  { CUR_CAT, "Dueling", 1 },
  { CUR_CAT, "Hapkido", 1 },
  { CUR_CAT, "Jujitsu", 1 },
  { CUR_CAT, "Karate", 1 },
  { CUR_CAT, "Tae Kwon Do", 1 },
  { CUR_CAT, "Wildcat", 1 },
  #undef CUR_CAT
  #define CUR_CAT "Sport Martial Arts"
  { CUR_CAT, "Boxing", 2 },
  { CUR_CAT, "Fencing", 1 },
  { CUR_CAT, "Judo", 2 },
  { CUR_CAT, "Kung Fu", 1 },
  { CUR_CAT, "Muay Thai", 1 },
  { CUR_CAT, "Sport Wrestling", 2 },
  { CUR_CAT, "Sumo-Wrestling", 0 },
  #undef CUR_CAT
  #define CUR_CAT "Combat Environment Skills"
  { CUR_CAT, "Amphibious Combat", 4 },
  { CUR_CAT, "High-G Combat", 4 },
  { CUR_CAT, "Low-G Combat", 4 },
  { CUR_CAT, "Underwater Combat", 4 },
  { CUR_CAT, "Zero-G Combat", 4 },
  #undef CUR_CAT
  #define CUR_CAT "Nature Skills"
  { CUR_CAT, "Healing", 4 },
  { CUR_CAT, "Navigation", 4 },
  { CUR_CAT, "Survival", 4 },
  #undef CUR_CAT
  #define CUR_CAT "Specialized Skills"
  { CUR_CAT, "Knowledge", 4 },
  { CUR_CAT, "Perception", 4 },
  { CUR_CAT, "Quick-draw", 1 },
  { CUR_CAT, "Research", 4 },
  { CUR_CAT, "Tactics", 4 },
  { CUR_CAT, "Tracking", 4 },
  { CUR_CAT, "High-G Ops", 4 },
  { CUR_CAT, "Low-G Ops", 4 },
  { CUR_CAT, "Zero-G Ops", 4 },
  #undef CUR_CAT
  #define CUR_CAT "Social Skills"
  { CUR_CAT, "Acting", 3 },
  { CUR_CAT, "Art, (Type)", 3 },
  { CUR_CAT, "Diplomacy", 3 },
  { CUR_CAT, "Instruction", 3 },
  { CUR_CAT, "Interrogation", 4 },
  { CUR_CAT, "Intimidation", 5 },
  { CUR_CAT, "Leadership", 3 },
  { CUR_CAT, "Negotiation", 3 },
  { CUR_CAT, "Performance, (Type)", 3 },
  { CUR_CAT, "Seduction", 3 },
  { CUR_CAT, "Style", 3 },
  #undef CUR_CAT
  #define CUR_CAT "Etiquette Skills"
  { CUR_CAT, "Activist Etiquette", 3 },
  { CUR_CAT, "Corporate Etiquette", 3 },
  { CUR_CAT, "Craftsman Etiquette", 3 },
  { CUR_CAT, "Dark Elf Etiquette", 3 },
  { CUR_CAT, "Decker Etiquette", 3 },
  { CUR_CAT, "Dwarf Etiquette", 3 },
  { CUR_CAT, "Eastern Etiquette", 3 },
  { CUR_CAT, "Farmer Etiquette", 3 },
  { CUR_CAT, "Gang Etiquette", 3 },
  { CUR_CAT, "Goblin Etiquette", 3 },
  { CUR_CAT, "Government Etiquette", 3 },
  { CUR_CAT, "Grey Elf Etiquette", 3 },
  { CUR_CAT, "High Elf Etiquette", 3 },
  { CUR_CAT, "High Society Etiquette", 3 },
  { CUR_CAT, "Human Etiquette", 3 },
  { CUR_CAT, "Lone Star Etiquette", 3 },
  { CUR_CAT, "Mage Etiquette", 3 },
  { CUR_CAT, "Matrix Etiquette", 3 },
  { CUR_CAT, "Mercenary Etiquette", 3 },
  { CUR_CAT, "Merchant Etiquette", 3 },
  { CUR_CAT, "Military Etiquette", 3 },
  { CUR_CAT, "Netiquette", 4 },
  { CUR_CAT, "Noble Etiquette", 3 },
  { CUR_CAT, "Oceanic Etiquette", 3 },
  { CUR_CAT, "Organized Crime Etiquette", 3 },
  { CUR_CAT, "Ork Etiquette", 3 },
  { CUR_CAT, "Peasant Etiquette", 3 },
  { CUR_CAT, "Police Etiquette", 3 },
  { CUR_CAT, "Pop Culture Etiquette", 3 },
  { CUR_CAT, "Religion Etiquette", 3 },
  { CUR_CAT, "Rigger Etiquette", 3 },
  { CUR_CAT, "Thief Etiquette", 3 },
  { CUR_CAT, "Trucker Etiquette", 3 },
  { CUR_CAT, "Science Etiquette", 3 },
  { CUR_CAT, "Separatist Etiquette", 3 },
  { CUR_CAT, "Shaman Etiquette", 3 },
  { CUR_CAT, "Street Etiquette", 3 },
  { CUR_CAT, "Underworld Etiquette", 3 },
  { CUR_CAT, "University Etiquette", 3 },
  #undef CUR_CAT
  #define CUR_CAT "Creation Skills"
  { CUR_CAT, "Armoring", 4 },
  { CUR_CAT, "Bowyer", 4 },
  { CUR_CAT, "Blacksmithing", 4 },
  { CUR_CAT, "Carpentry", 4 },
  { CUR_CAT, "Clothier", 4 },
  { CUR_CAT, "Fletcher", 4 },
  { CUR_CAT, "Leatherworking", 4 },
  { CUR_CAT, "Masonry", 4 },
  { CUR_CAT, "Metalworking", 4 },
  { CUR_CAT, "Shipwright", 4 },
  { CUR_CAT, "Smelting", 4 },
  { CUR_CAT, "Swordsmithing", 4 },
  { CUR_CAT, "Woodworking", 4 },
  #undef CUR_CAT
  #define CUR_CAT "Magical Skills"
  { CUR_CAT, "Alchemy", 4 },
  { CUR_CAT, "Conjuring", 3 },
  { CUR_CAT, "Enchanting", 4 },
  { CUR_CAT, "Helmsman, Star", 4 },
  { CUR_CAT, "Ritual Conjuring", 5 },
  { CUR_CAT, "Ritual Spellcasting", 5 },
  { CUR_CAT, "Rod Targeting", 1 },
  { CUR_CAT, "Spellcasting", 4 },
  { CUR_CAT, "Spellcraft", 4 },
  { CUR_CAT, "Spell Targeting", 1 },
  { CUR_CAT, "Staff Targeting", 1 },
  { CUR_CAT, "Talismongery", 5 },
  { CUR_CAT, "Wand Targeting", 1 },
  #undef CUR_CAT
  #define CUR_CAT "Shamanistic Skills"
  { CUR_CAT, "Find Juju", 5 },
  { CUR_CAT, "Identify Juju", 4 },
  { CUR_CAT, "Refine Juju", 3 },
  #undef CUR_CAT
  #define CUR_CAT "Build/Repair Skills"
  { CUR_CAT, "Computer B/R", 4 },
  { CUR_CAT, "Electrical B/R", 4 },
  { CUR_CAT, "Mechanical B/R", 4 },
  #undef CUR_CAT
  #define CUR_CAT "Technical Skills"
  { CUR_CAT, "Computer", 4 },
  { CUR_CAT, "Demolitions", 4 },
  { CUR_CAT, "Electronics", 4 },
  { CUR_CAT, "Mechanics", 4 },
  { CUR_CAT, "Engineering", 4 },
  { CUR_CAT, "Damage Control", 4 },
  { CUR_CAT, "First Aid", 4 },
  { CUR_CAT, "Treatment", 4 },
  { CUR_CAT, "Surgery", 4 },
  { CUR_CAT, "Security", 4 },
  { CUR_CAT, "Communications", 4 },
  { NULL, NULL, 0 }
  };
