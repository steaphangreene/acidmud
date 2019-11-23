#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>
#include <vector>

#include "color.hpp"
#include "object.hpp"
#include "utils.hpp"

static std::map<uint32_t, int32_t> defaults;
static std::map<int32_t, uint32_t> weaponskills;
static std::map<uint32_t, int32_t> weapontypes;
static std::map<std::string, std::vector<uint32_t>> skcat;

template <typename T>
typename std::vector<std::pair<uint32_t, T>>::iterator hash_guess(
    std::vector<std::pair<uint32_t, T>>& data,
    uint32_t hash) {
  if (data.empty())
    return data.end();

  int64_t off = (int64_t(data.size()) * int64_t(hash)) / int64_t(0x100000000);
  return data.begin() + off;
}

template <typename T>
typename std::vector<std::pair<uint32_t, T>>::iterator hash_locate(
    std::vector<std::pair<uint32_t, T>>& data,
    uint32_t hash) {
  if (data.empty())
    return data.end();

  auto itr = hash_guess(data, hash);
  if (itr == data.end())
    --itr;
  if (itr->first > hash) {
    while (itr != data.begin() && itr->first > hash)
      --itr;
  }
  if (itr != data.end() && itr->first < hash) {
    while (itr != data.end() && itr->first < hash)
      ++itr;
  }
  return itr;
}

template <typename T>
typename std::vector<std::pair<uint32_t, T>>::iterator hash_find(
    std::vector<std::pair<uint32_t, T>>& data,
    uint32_t hash) {
  if (data.empty())
    return data.end();

  auto itr = hash_locate(data, hash);
  if (itr != data.end() && itr->first != hash)
    itr = data.end();
  return itr;
}

template <typename T>
typename std::vector<std::pair<uint32_t, T>>::const_iterator hash_guess(
    const std::vector<std::pair<uint32_t, T>>& data,
    uint32_t hash) {
  if (data.empty())
    return data.end();

  int64_t off = (int64_t(data.size()) * int64_t(hash)) / int64_t(0x100000000);
  return data.cbegin() + off;
}

template <typename T>
typename std::vector<std::pair<uint32_t, T>>::const_iterator hash_locate(
    const std::vector<std::pair<uint32_t, T>>& data,
    uint32_t hash) {
  if (data.empty())
    return data.end();

  auto itr = hash_guess(data, hash);
  if (itr == data.cend())
    --itr;
  if (itr->first > hash) {
    while (itr != data.cbegin() && itr->first > hash)
      --itr;
  }
  if (itr != data.cend() && itr->first < hash) {
    while (itr != data.cend() && itr->first < hash)
      ++itr;
  }
  return itr;
}

template <typename T>
typename std::vector<std::pair<uint32_t, T>>::const_iterator hash_find(
    const std::vector<std::pair<uint32_t, T>>& data,
    uint32_t hash) {
  if (data.empty())
    return data.end();

  auto itr = hash_locate(data, hash);
  if (itr != data.cend() && itr->first != hash)
    itr = data.cend();
  return itr;
}

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

static std::vector<std::pair<uint32_t, std::string>> skill_names = {
    // Boostrap Strings List
    {crc32c("Accomplishment"), "Accomplishment"},
    {crc32c("Accuracy Bonus"), "Accuracy Bonus"},
    {crc32c("Accuracy Penalty"), "Accuracy Penalty"},
    {crc32c("Acrobatics"), "Acrobatics"},
    {crc32c("Acting"), "Acting"},
    {crc32c("Activate Juju"), "Activate Juju"},
    {crc32c("Air Pistols"), "Air Pistols"},
    {crc32c("Air Rifles"), "Air Rifles"},
    {crc32c("Air Weaving"), "Air Weaving"},
    {crc32c("Alchemy"), "Alchemy"},
    {crc32c("Alcohol"), "Alcohol"},
    {crc32c("Amphibious Combat"), "Amphibious Combat"},
    {crc32c("Archery"), "Archery"},
    {crc32c("Arctic Waraxe Martial Art"), "Arctic Waraxe Martial Art"},
    {crc32c("Armoring"), "Armoring"},
    {crc32c("Art"), "Art"},
    {crc32c("Artificing"), "Artificing"},
    {crc32c("Artificing, Star"), "Artificing, Star"},
    {crc32c("Assault Cannons"), "Assault Cannons"},
    {crc32c("Assault Rifles"), "Assault Rifles"},
    {crc32c("Athletic Skills"), "Athletic Skills"},
    {crc32c("Attribute Points"), "Attribute Points"},
    {crc32c("Ballista"), "Ballista"},
    {crc32c("Ballistic"), "Ballistic"},
    {crc32c("Bang"), "Bang"},
    {crc32c("Bike"), "Bike"},
    {crc32c("Blacksmithing"), "Blacksmithing"},
    {crc32c("Blessed"), "Blessed"},
    {crc32c("Blind Spell"), "Blind Spell"},
    {crc32c("Blowgun"), "Blowgun"},
    {crc32c("Boat, Powered"), "Boat, Powered"},
    {crc32c("Boat, Row"), "Boat, Row"},
    {crc32c("Boat, Sail"), "Boat, Sail"},
    {crc32c("Body-Based Skills"), "Body-Based Skills"},
    {crc32c("Body"), "Body"},
    {crc32c("Body Checking"), "Body Checking"},
    {crc32c("Bored"), "Bored"},
    {crc32c("Bowyer"), "Bowyer"},
    {crc32c("Breath Weapon"), "Breath Weapon"},
    {crc32c("Brightness"), "Brightness"},
    {crc32c("Buggy"), "Buggy"},
    {crc32c("Build/Repair Skills"), "Build/Repair Skills"},
    {crc32c("Bulk"), "Bulk"},
    {crc32c("Burn"), "Burn"},
    {crc32c("Cannon"), "Cannon"},
    {crc32c("Capacity"), "Capacity"},
    {crc32c("Car"), "Car"},
    {crc32c("Carpentry"), "Carpentry"},
    {crc32c("Carromeleg - Tier I"), "Carromeleg - Tier I"},
    {crc32c("Carromeleg - Tier II"), "Carromeleg - Tier II"},
    {crc32c("Carromeleg - Tier III"), "Carromeleg - Tier III"},
    {crc32c("Carromeleg - Tier IV"), "Carromeleg - Tier IV"},
    {crc32c("Carromeleg - Tier V"), "Carromeleg - Tier V"},
    {crc32c("Carromeleg - Tier VI"), "Carromeleg - Tier VI"},
    {crc32c("Cart"), "Cart"},
    {crc32c("Catching"), "Catching"},
    {crc32c("Centering"), "Centering"},
    {crc32c("Charisma-Based Skills"), "Charisma-Based Skills"},
    {crc32c("Charisma"), "Charisma"},
    {crc32c("Chill"), "Chill"},
    {crc32c("Clear Weather Spell"), "Clear Weather Spell"},
    {crc32c("Climbing"), "Climbing"},
    {crc32c("Closeable"), "Closeable"},
    {crc32c("Clothier"), "Clothier"},
    {crc32c("Combat Environment Skills"), "Combat Environment Skills"},
    {crc32c("Communications"), "Communications"},
    {crc32c("Computer Build/Repair"), "Computer Build/Repair"},
    {crc32c("Computer"), "Computer"},
    {crc32c("Computer Targeting"), "Computer Targeting"},
    {crc32c("Concuss"), "Concuss"},
    {crc32c("Conjuring"), "Conjuring"},
    {crc32c("Container"), "Container"},
    {crc32c("Copy Book Spell"), "Copy Book Spell"},
    {crc32c("Coverage"), "Coverage"},
    {crc32c("Create Food"), "Create Food"},
    {crc32c("Create Food Spell"), "Create Food Spell"},
    {crc32c("Create Water Spell"), "Create Water Spell"},
    {crc32c("Create Zombie Spell"), "Create Zombie Spell"},
    {crc32c("Creation Skills"), "Creation Skills"},
    {crc32c("Crossbow"), "Crossbow"},
    {crc32c("Cure Blindness Spell"), "Cure Blindness Spell"},
    {crc32c("Cure Poison"), "Cure Poison"},
    {crc32c("Cure Poison Spell"), "Cure Poison Spell"},
    {crc32c("Cursed"), "Cursed"},
    {crc32c("Cyber Data Manipulation"), "Cyber Data Manipulation"},
    {crc32c("Cyberpunk Skills"), "Cyberpunk Skills"},
    {crc32c("Cyber Skills"), "Cyber Skills"},
    {crc32c("Cyber Vehicle Operation"), "Cyber Vehicle Operation"},
    {crc32c("Cycling"), "Cycling"},
    {crc32c("Damage Bonus"), "Damage Bonus"},
    {crc32c("Damage Control"), "Damage Control"},
    {crc32c("Damage Penalty"), "Damage Penalty"},
    {crc32c("Damage Resistance"), "Damage Resistance"},
    {crc32c("Darkness Spell"), "Darkness Spell"},
    {crc32c("Dark Vision"), "Dark Vision"},
    {crc32c("Dark Vision Spell"), "Dark Vision Spell"},
    {crc32c("Day Length"), "Day Length"},
    {crc32c("Day Time"), "Day Time"},
    {crc32c("Defense Range"), "Defense Range"},
    {crc32c("Def Range"), "Def Range"},
    {crc32c("Dehydrate Effect"), "Dehydrate Effect"},
    {crc32c("Demolitions"), "Demolitions"},
    {crc32c("Detect Cursed Items Spell"), "Detect Cursed Items Spell"},
    {crc32c("Detect Poison Spell"), "Detect Poison Spell"},
    {crc32c("Diplomacy"), "Diplomacy"},
    {crc32c("Dishonor"), "Dishonor"},
    {crc32c("Distance"), "Distance"},
    {crc32c("Distract Spell"), "Distract Spell"},
    {crc32c("Diving"), "Diving"},
    {crc32c("Dodge"), "Dodge"},
    {crc32c("Drink"), "Drink"},
    {crc32c("Drone Piloting Skills"), "Drone Piloting Skills"},
    {crc32c("Durability"), "Durability"},
    {crc32c("DynamicInit"), "DynamicInit"},
    {crc32c("Earthquake Spell"), "Earthquake Spell"},
    {crc32c("Earth Weaving"), "Earth Weaving"},
    {crc32c("Electrical Build/Repair"), "Electrical Build/Repair"},
    {crc32c("Electronics"), "Electronics"},
    {crc32c("Elven Martial Arts Skills"), "Elven Martial Arts Skills"},
    {crc32c("Enchanting"), "Enchanting"},
    {crc32c("Encumbrance Bonus"), "Encumbrance Bonus"},
    {crc32c("Encumbrance Penalty"), "Encumbrance Penalty"},
    {crc32c("Endurance"), "Endurance"},
    {crc32c("Energize Spell"), "Energize Spell"},
    {crc32c("Engineering"), "Engineering"},
    {crc32c("Enterable"), "Enterable"},
    {crc32c("Etiquette"), "Etiquette"},
    {crc32c("Evasion Bonus"), "Evasion Bonus"},
    {crc32c("Evasion"), "Evasion"},
    {crc32c("Evasion Penalty"), "Evasion Penalty"},
    {crc32c("Expert Skills"), "Expert Skills"},
    {crc32c("Finance"), "Finance"},
    {crc32c("Find Juju"), "Find Juju"},
    {crc32c("Fireball Spell"), "Fireball Spell"},
    {crc32c("Fire Burst Spell"), "Fire Burst Spell"},
    {crc32c("Fire Dart Spell"), "Fire Dart Spell"},
    {crc32c("Fire Weaving"), "Fire Weaving"},
    {crc32c("First Aid"), "First Aid"},
    {crc32c("Fixed-Wing Craft"), "Fixed-Wing Craft"},
    {crc32c("Fixed-Wing Drone"), "Fixed-Wing Drone"},
    {crc32c("Flash"), "Flash"},
    {crc32c("Fletcher"), "Fletcher"},
    {crc32c("Flexibility"), "Flexibility"},
    {crc32c("Flight"), "Flight"},
    {crc32c("Float Spell"), "Float Spell"},
    {crc32c("Fly Spell"), "Fly Spell"},
    {crc32c("Focusing"), "Focusing"},
    {crc32c("Food"), "Food"},
    {crc32c("Force Arrow Spell"), "Force Arrow Spell"},
    {crc32c("Force Sword"), "Force Sword"},
    {crc32c("Force Sword Spell"), "Force Sword Spell"},
    {crc32c("FP Skills"), "FP Skills"},
    {crc32c("Grappling"), "Grappling"},
    {crc32c("Group Resilience Spell"), "Group Resilience Spell"},
    {crc32c("Gunnery"), "Gunnery"},
    {crc32c("Half-Track"), "Half-Track"},
    {crc32c("Hand-to-Hand Combat Skills"), "Hand-to-Hand Combat Skills"},
    {crc32c("Hardness"), "Hardness"},
    {crc32c("Harm Spell"), "Harm Spell"},
    {crc32c("Hazardous"), "Hazardous"},
    {crc32c("Heal Effect"), "Heal Effect"},
    {crc32c("Heal Group Spell"), "Heal Group Spell"},
    {crc32c("Healing"), "Healing"},
    {crc32c("Heal Spell"), "Heal Spell"},
    {crc32c("Heat Vision"), "Heat Vision"},
    {crc32c("Heat Vision Spell"), "Heat Vision Spell"},
    {crc32c("Heavy Firearm Skills"), "Heavy Firearm Skills"},
    {crc32c("Heavy Lasers"), "Heavy Lasers"},
    {crc32c("Heavy Projectors"), "Heavy Projectors"},
    {crc32c("Heavy Rifles"), "Heavy Rifles"},
    {crc32c("Helmsman, Star"), "Helmsman, Star"},
    {crc32c("Hidden"), "Hidden"},
    {crc32c("High-G Combat"), "High-G Combat"},
    {crc32c("High-G Ops"), "High-G Ops"},
    {crc32c("Hits"), "Hits"},
    {crc32c("Hit Weight"), "Hit Weight"},
    {crc32c("Honor"), "Honor"},
    {crc32c("Hovercraft"), "Hovercraft"},
    {crc32c("Hover Drone"), "Hover Drone"},
    {crc32c("Hungry"), "Hungry"},
    {crc32c("Hurling"), "Hurling"},
    {crc32c("Identify Character Spell"), "Identify Character Spell"},
    {crc32c("Identify"), "Identify"},
    {crc32c("Identify Juju"), "Identify Juju"},
    {crc32c("Identify Person Spell"), "Identify Person Spell"},
    {crc32c("Identify Spell"), "Identify Spell"},
    {crc32c("Influence Spell"), "Influence Spell"},
    {crc32c("Ingestible"), "Ingestible"},
    {crc32c("Injure Spell"), "Injure Spell"},
    {crc32c("Insulate"), "Insulate"},
    {crc32c("Intelligence-Based Skills"), "Intelligence-Based Skills"},
    {crc32c("Intelligence"), "Intelligence"},
    {crc32c("Interrogation"), "Interrogation"},
    {crc32c("Intimidation"), "Intimidation"},
    {crc32c("Invisibility Spell"), "Invisibility Spell"},
    {crc32c("Invisible"), "Invisible"},
    {crc32c("Irradiate"), "Irradiate"},
    {crc32c("Jumping"), "Jumping"},
    {crc32c("Key"), "Key"},
    {crc32c("Kicking"), "Kicking"},
    {crc32c("Knowledge"), "Knowledge"},
    {crc32c("Laser Pistols"), "Laser Pistols"},
    {crc32c("Laser Rifles"), "Laser Rifles"},
    {crc32c("Lasso"), "Lasso"},
    {crc32c("Launch Weapons"), "Launch Weapons"},
    {crc32c("Leadership"), "Leadership"},
    {crc32c("Leatherworking"), "Leatherworking"},
    {crc32c("Leverage"), "Leverage"},
    {crc32c("Lifting"), "Lifting"},
    {crc32c("Lightable"), "Lightable"},
    {crc32c("Lightning Bolt Spell"), "Lightning Bolt Spell"},
    {crc32c("Light Source"), "Light Source"},
    {crc32c("Light Spell"), "Light Spell"},
    {crc32c("Liquid Container"), "Liquid Container"},
    {crc32c("Liquid"), "Liquid"},
    {crc32c("Liquid Source"), "Liquid Source"},
    {crc32c("Locate Object Spell"), "Locate Object Spell"},
    {crc32c("Lockable"), "Lockable"},
    {crc32c("Locked"), "Locked"},
    {crc32c("Lock"), "Lock"},
    {crc32c("Long Blades"), "Long Blades"},
    {crc32c("Long Cleaves"), "Long Cleaves"},
    {crc32c("Long Crushing"), "Long Crushing"},
    {crc32c("Long Flails"), "Long Flails"},
    {crc32c("Long Piercing"), "Long Piercing"},
    {crc32c("Long Staves"), "Long Staves"},
    {crc32c("Lost Martial Arts"), "Lost Martial Arts"},
    {crc32c("Low-G Combat"), "Low-G Combat"},
    {crc32c("Low-G Ops"), "Low-G Ops"},
    {crc32c("Luck Spell"), "Luck Spell"},
    {crc32c("Lumberjack"), "Lumberjack"},
    {crc32c("Machine Guns"), "Machine Guns"},
    {crc32c("Machine Pistols"), "Machine Pistols"},
    {crc32c("Magical Charges"), "Magical Charges"},
    {crc32c("Magical"), "Magical"},
    {crc32c("Magical Scroll"), "Magical Scroll"},
    {crc32c("Magical Skills"), "Magical Skills"},
    {crc32c("Magical Spell"), "Magical Spell"},
    {crc32c("Magical Staff"), "Magical Staff"},
    {crc32c("Magical Wand"), "Magical Wand"},
    {crc32c("Magic Dead"), "Magic Dead"},
    {crc32c("Magic Force Bonus"), "Magic Force Bonus"},
    {crc32c("Magic Force Penalty"), "Magic Force Penalty"},
    {crc32c("Magic Manipulation"), "Magic Manipulation"},
    {crc32c("Magic Resistance"), "Magic Resistance"},
    {crc32c("Magic Vulnerability"), "Magic Vulnerability"},
    {crc32c("Martial Arts, Elven"), "Martial Arts, Elven"},
    {crc32c("Martial Arts Skills"), "Martial Arts Skills"},
    {crc32c("Masonry"), "Masonry"},
    {crc32c("Mature Trees"), "Mature Trees"},
    {crc32c("Max Gap"), "Max Gap"},
    {crc32c("Mechanical Build/Repair"), "Mechanical Build/Repair"},
    {crc32c("Mechanics"), "Mechanics"},
    {crc32c("Medieval Skills"), "Medieval Skills"},
    {crc32c("Melee-Combat Skills"), "Melee-Combat Skills"},
    {crc32c("Metalworking"), "Metalworking"},
    {crc32c("Mindcasting"), "Mindcasting"},
    {crc32c("Min Gap"), "Min Gap"},
    {crc32c("Misfortune Spell"), "Misfortune Spell"},
    {crc32c("Modern Skills"), "Modern Skills"},
    {crc32c("Money"), "Money"},
    {crc32c("Mounted Air Pistols"), "Mounted Air Pistols"},
    {crc32c("Mounted Archery"), "Mounted Archery"},
    {crc32c("Mounted Blades"), "Mounted Blades"},
    {crc32c("Mounted Blowgun"), "Mounted Blowgun"},
    {crc32c("Mounted Cleaves"), "Mounted Cleaves"},
    {crc32c("Mounted Crossbow"), "Mounted Crossbow"},
    {crc32c("Mounted Crushing"), "Mounted Crushing"},
    {crc32c("Mounted Flails"), "Mounted Flails"},
    {crc32c("Mounted Hurling"), "Mounted Hurling"},
    {crc32c("Mounted Laser Pistols"), "Mounted Laser Pistols"},
    {crc32c("Mounted Machine Pistols"), "Mounted Machine Pistols"},
    {crc32c("Mounted Melee-Combat Skills"), "Mounted Melee-Combat Skills"},
    {crc32c("Mounted Nets"), "Mounted Nets"},
    {crc32c("Mounted Piercing"), "Mounted Piercing"},
    {crc32c("Mounted Pistol Skills"), "Mounted Pistol Skills"},
    {crc32c("Mounted Pistols"), "Mounted Pistols"},
    {crc32c("Mounted Plasma Pistols"), "Mounted Plasma Pistols"},
    {crc32c("Mounted Ranged-Combat Skills"), "Mounted Ranged-Combat Skills"},
    {crc32c("Mounted Shot Pistols"), "Mounted Shot Pistols"},
    {crc32c("Mounted Slings"), "Mounted Slings"},
    {crc32c("Mounted SMGs"), "Mounted SMGs"},
    {crc32c("Mounted Throwing, Aero"), "Mounted Throwing, Aero"},
    {crc32c("Mounted Throwing, Non-Aero"), "Mounted Throwing, Non-Aero"},
    {crc32c("Mounted Whips"), "Mounted Whips"},
    {crc32c("Muffle"), "Muffle"},
    {crc32c("Multiple"), "Multiple"},
    {crc32c("NaturalWeapon"), "NaturalWeapon"},
    {crc32c("Nature Skills"), "Nature Skills"},
    {crc32c("Navigation"), "Navigation"},
    {crc32c("Needy"), "Needy"},
    {crc32c("Negotiation"), "Negotiation"},
    {crc32c("Nets"), "Nets"},
    {crc32c("Neural Interface"), "Neural Interface"},
    {crc32c("Noise Source"), "Noise Source"},
    {crc32c("None"), "None"},
    {crc32c("Obvious"), "Obvious"},
    {crc32c("Offhand Air Pistols"), "Offhand Air Pistols"},
    {crc32c("Offhand Blades"), "Offhand Blades"},
    {crc32c("Offhand Cleaves"), "Offhand Cleaves"},
    {crc32c("Offhand Crossbow"), "Offhand Crossbow"},
    {crc32c("Offhand Crushing"), "Offhand Crushing"},
    {crc32c("Offhand Flails"), "Offhand Flails"},
    {crc32c("Offhand Hurling"), "Offhand Hurling"},
    {crc32c("Offhand Laser Pistols"), "Offhand Laser Pistols"},
    {crc32c("Offhand Machine Pistols"), "Offhand Machine Pistols"},
    {crc32c("Offhand Piercing"), "Offhand Piercing"},
    {crc32c("Offhand Pistols"), "Offhand Pistols"},
    {crc32c("Offhand Plasma Pistols"), "Offhand Plasma Pistols"},
    {crc32c("Offhand Punching"), "Offhand Punching"},
    {crc32c("Offhand Shot Pistols"), "Offhand Shot Pistols"},
    {crc32c("Offhand SMGs"), "Offhand SMGs"},
    {crc32c("Offhand Staves"), "Offhand Staves"},
    {crc32c("Offhand Throwing, Aero"), "Offhand Throwing, Aero"},
    {crc32c("Offhand Throwing, Non-Aero"), "Offhand Throwing, Non-Aero"},
    {crc32c("Offhand Whips"), "Offhand Whips"},
    {crc32c("Open"), "Open"},
    {crc32c("Padding"), "Padding"},
    {crc32c("Peaceful"), "Peaceful"},
    {crc32c("Pen. Dist"), "Pen. Dist"},
    {crc32c("Pen. Ratio"), "Pen. Ratio"},
    {crc32c("Pen. Width"), "Pen. Width"},
    {crc32c("Perception"), "Perception"},
    {crc32c("Performance"), "Performance"},
    {crc32c("Perishable"), "Perishable"},
    {crc32c("Personality"), "Personality"},
    {crc32c("Personal Shield Spell"), "Personal Shield Spell"},
    {crc32c("Pickable"), "Pickable"},
    {crc32c("Piloting Skills"), "Piloting Skills"},
    {crc32c("Pistol Skills"), "Pistol Skills"},
    {crc32c("Pistols"), "Pistols"},
    {crc32c("Plasma Cannons"), "Plasma Cannons"},
    {crc32c("Plasma Pistols"), "Plasma Pistols"},
    {crc32c("Plasma Rifles"), "Plasma Rifles"},
    {crc32c("Poisionous"), "Poisionous"},
    {crc32c("Poisoned"), "Poisoned"},
    {crc32c("Poisonous"), "Poisonous"},
    {crc32c("Power Distribution"), "Power Distribution"},
    {crc32c("Power Suit"), "Power Suit"},
    {crc32c("Prepared Basic"), "Prepared Basic"},
    {crc32c("Prepared Intermediate"), "Prepared Intermediate"},
    {crc32c("Prepared Primary"), "Prepared Primary"},
    {crc32c("Prepared Simple"), "Prepared Simple"},
    {crc32c("Priceless"), "Priceless"},
    {crc32c("Protection Spell"), "Protection Spell"},
    {crc32c("Punching"), "Punching"},
    {crc32c("Quantity"), "Quantity"},
    {crc32c("Quickdraw"), "Quickdraw"},
    {crc32c("Quickness-Based Skills"), "Quickness-Based Skills"},
    {crc32c("Quickness"), "Quickness"},
    {crc32c("Race Car"), "Race Car"},
    {crc32c("Ranged-Combat Skills"), "Ranged-Combat Skills"},
    {crc32c("Range"), "Range"},
    {crc32c("Ratio"), "Ratio"},
    {crc32c("Reach"), "Reach"},
    {crc32c("Reaction-Based Skills"), "Reaction-Based Skills"},
    {crc32c("Reaction"), "Reaction"},
    {crc32c("Recall Group Spell"), "Recall Group Spell"},
    {crc32c("Recall"), "Recall"},
    {crc32c("Recall Spell"), "Recall Spell"},
    {crc32c("Refine Juju"), "Refine Juju"},
    {crc32c("Reflect"), "Reflect"},
    {crc32c("Remove Curse"), "Remove Curse"},
    {crc32c("Remove Curse Spell"), "Remove Curse Spell"},
    {crc32c("Research"), "Research"},
    {crc32c("Resilience Bonus"), "Resilience Bonus"},
    {crc32c("Resilience Penalty"), "Resilience Penalty"},
    {crc32c("Resilience Spell"), "Resilience Spell"},
    {crc32c("Restricted Item"), "Restricted Item"},
    {crc32c("Resurrect"), "Resurrect"},
    {crc32c("Resurrect Spell"), "Resurrect Spell"},
    {crc32c("Riding"), "Riding"},
    {crc32c("Rifle Skills"), "Rifle Skills"},
    {crc32c("Rifles"), "Rifles"},
    {crc32c("Ritual Conjuring"), "Ritual Conjuring"},
    {crc32c("Ritual Spellcasting"), "Ritual Spellcasting"},
    {crc32c("Rod Targeting"), "Rod Targeting"},
    {crc32c("Rotorcraft"), "Rotorcraft"},
    {crc32c("Rotor Drone"), "Rotor Drone"},
    {crc32c("Rot"), "Rot"},
    {crc32c("Running"), "Running"},
    {crc32c("Saurian Line Attacking"), "Saurian Line Attacking"},
    {crc32c("Sci-Fi Skills"), "Sci-Fi Skills"},
    {crc32c("Security"), "Security"},
    {crc32c("Seduction"), "Seduction"},
    {crc32c("Self Artificing"), "Self Artificing"},
    {crc32c("Sell Profit"), "Sell Profit"},
    {crc32c("Sensors"), "Sensors"},
    {crc32c("Shade"), "Shade"},
    {crc32c("Shadowrun Skills"), "Shadowrun Skills"},
    {crc32c("Shamanistic Skills"), "Shamanistic Skills"},
    {crc32c("Sharpness"), "Sharpness"},
    {crc32c("Shielding"), "Shielding"},
    {crc32c("Shields"), "Shields"},
    {crc32c("Ship, Powered"), "Ship, Powered"},
    {crc32c("Ship"), "Ship"},
    {crc32c("Shipwright"), "Shipwright"},
    {crc32c("Short Blades"), "Short Blades"},
    {crc32c("Short Cleaves"), "Short Cleaves"},
    {crc32c("Short Crushing"), "Short Crushing"},
    {crc32c("Short Flails"), "Short Flails"},
    {crc32c("Short Piercing"), "Short Piercing"},
    {crc32c("Short Staves"), "Short Staves"},
    {crc32c("Shotguns"), "Shotguns"},
    {crc32c("Shot Pistols"), "Shot Pistols"},
    {crc32c("Skiing"), "Skiing"},
    {crc32c("Skill-Based Skills"), "Skill-Based Skills"},
    {crc32c("Skill Points"), "Skill Points"},
    {crc32c("Skills"), "Skills"},
    {crc32c("Sled"), "Sled"},
    {crc32c("Sleep Other"), "Sleep Other"},
    {crc32c("Sleep Other Spell"), "Sleep Other Spell"},
    {crc32c("Sleepy"), "Sleepy"},
    {crc32c("Slings"), "Slings"},
    {crc32c("Smelting"), "Smelting"},
    {crc32c("SMGs"), "SMGs"},
    {crc32c("Snowmobile"), "Snowmobile"},
    {crc32c("Social Skills"), "Social Skills"},
    {crc32c("Soundproof"), "Soundproof"},
    {crc32c("Spacecraft"), "Spacecraft"},
    {crc32c("Spark Spell"), "Spark Spell"},
    {crc32c("Specialized Skills"), "Specialized Skills"},
    {crc32c("Specialty Skills"), "Specialty Skills"},
    {crc32c("Spellcasting"), "Spellcasting"},
    {crc32c("Spellcraft"), "Spellcraft"},
    {crc32c("Spellcraft, Star"), "Spellcraft, Star"},
    {crc32c("Spell Preparation"), "Spell Preparation"},
    {crc32c("Spell Targeting"), "Spell Targeting"},
    {crc32c("Sprinting"), "Sprinting"},
    {crc32c("Staff Slings"), "Staff Slings"},
    {crc32c("Staff Targeting"), "Staff Targeting"},
    {crc32c("Star Magical Skills"), "Star Magical Skills"},
    {crc32c("Star Martial Arts"), "Star Martial Arts"},
    {crc32c("Status Points"), "Status Points"},
    {crc32c("Stealth"), "Stealth"},
    {crc32c("Strength-Based Skills"), "Strength-Based Skills"},
    {crc32c("Strength Required"), "Strength Required"},
    {crc32c("Strength Spell"), "Strength Spell"},
    {crc32c("Strength"), "Strength"},
    {crc32c("Str Req"), "Str Req"},
    {crc32c("Style"), "Style"},
    {crc32c("Summon Creature Spell"), "Summon Creature Spell"},
    {crc32c("Surgery"), "Surgery"},
    {crc32c("Survival"), "Survival"},
    {crc32c("Swimming"), "Swimming"},
    {crc32c("Swordsmithing"), "Swordsmithing"},
    {crc32c("Tactics"), "Tactics"},
    {crc32c("Talismongery"), "Talismongery"},
    {crc32c("TBAAction"), "TBAAction"},
    {crc32c("TBAAffection"), "TBAAffection"},
    {crc32c("TBAAttack"), "TBAAttack"},
    {crc32c("TBADefense"), "TBADefense"},
    {crc32c("TBAGold"), "TBAGold"},
    {crc32c("TBAMOB"), "TBAMOB"},
    {crc32c("TBAObject"), "TBAObject"},
    {crc32c("TBAPopper"), "TBAPopper"},
    {crc32c("TBARoom"), "TBARoom"},
    {crc32c("TBAScriptNArg"), "TBAScriptNArg"},
    {crc32c("TBAScript"), "TBAScript"},
    {crc32c("TBAScriptType"), "TBAScriptType"},
    {crc32c("TBAZone"), "TBAZone"},
    {crc32c("Teamster"), "Teamster"},
    {crc32c("Technical Skills"), "Technical Skills"},
    {crc32c("Teleport Spell"), "Teleport Spell"},
    {crc32c("Teleport"), "Teleport"},
    {crc32c("Temporary"), "Temporary"},
    {crc32c("Thickness"), "Thickness"},
    {crc32c("Thirsty"), "Thirsty"},
    {crc32c("Throwing, Aero"), "Throwing, Aero"},
    {crc32c("Throwing, Non-Aero"), "Throwing, Non-Aero"},
    {crc32c("Tired"), "Tired"},
    {crc32c("Tracked Drone"), "Tracked Drone"},
    {crc32c("Tracked Vehicle"), "Tracked Vehicle"},
    {crc32c("Tracking"), "Tracking"},
    {crc32c("Tractor"), "Tractor"},
    {crc32c("Translate Spell"), "Translate Spell"},
    {crc32c("Translucent"), "Translucent"},
    {crc32c("Transparent"), "Transparent"},
    {crc32c("Treatment Spell"), "Treatment Spell"},
    {crc32c("Treatment"), "Treatment"},
    {crc32c("Trike"), "Trike"},
    {crc32c("Two-Handed Blades"), "Two-Handed Blades"},
    {crc32c("Two-Handed Cleaves"), "Two-Handed Cleaves"},
    {crc32c("Two-Handed Crushing"), "Two-Handed Crushing"},
    {crc32c("Two-Handed Flails"), "Two-Handed Flails"},
    {crc32c("Two-Handed Piercing"), "Two-Handed Piercing"},
    {crc32c("Two-Handed Staves"), "Two-Handed Staves"},
    {crc32c("Underwater Combat"), "Underwater Combat"},
    {crc32c("Vectored Thrust Drone"), "Vectored Thrust Drone"},
    {crc32c("Vectored Thrust"), "Vectored Thrust"},
    {crc32c("Vehicle"), "Vehicle"},
    {crc32c("Velocity"), "Velocity"},
    {crc32c("Verbal Interface"), "Verbal Interface"},
    {crc32c("Vortex"), "Vortex"},
    {crc32c("Wand Targeting"), "Wand Targeting"},
    {crc32c("Warm"), "Warm"},
    {crc32c("WaterDepth"), "WaterDepth"},
    {crc32c("Water Weaving"), "Water Weaving"},
    {crc32c("Weaken Subject Spell"), "Weaken Subject Spell"},
    {crc32c("WeaponDamage"), "WeaponDamage"},
    {crc32c("WeaponForce"), "WeaponForce"},
    {crc32c("WeaponReach"), "WeaponReach"},
    {crc32c("WeaponSeverity"), "WeaponSeverity"},
    {crc32c("Weapon System Skills"), "Weapon System Skills"},
    {crc32c("WeaponType"), "WeaponType"},
    {crc32c("Wearable on Back"), "Wearable on Back"},
    {crc32c("Wearable on Chest"), "Wearable on Chest"},
    {crc32c("Wearable on Collar"), "Wearable on Collar"},
    {crc32c("Wearable on Face"), "Wearable on Face"},
    {crc32c("Wearable on Head"), "Wearable on Head"},
    {crc32c("Wearable on Left Arm"), "Wearable on Left Arm"},
    {crc32c("Wearable on Left Finger"), "Wearable on Left Finger"},
    {crc32c("Wearable on Left Foot"), "Wearable on Left Foot"},
    {crc32c("Wearable on Left Hand"), "Wearable on Left Hand"},
    {crc32c("Wearable on Left Hip"), "Wearable on Left Hip"},
    {crc32c("Wearable on Left Leg"), "Wearable on Left Leg"},
    {crc32c("Wearable on Left Shoulder"), "Wearable on Left Shoulder"},
    {crc32c("Wearable on Left Wrist"), "Wearable on Left Wrist"},
    {crc32c("Wearable on Neck"), "Wearable on Neck"},
    {crc32c("Wearable on Right Arm"), "Wearable on Right Arm"},
    {crc32c("Wearable on Right Finger"), "Wearable on Right Finger"},
    {crc32c("Wearable on Right Foot"), "Wearable on Right Foot"},
    {crc32c("Wearable on Right Hand"), "Wearable on Right Hand"},
    {crc32c("Wearable on Right Hip"), "Wearable on Right Hip"},
    {crc32c("Wearable on Right Leg"), "Wearable on Right Leg"},
    {crc32c("Wearable on Right Shoulder"), "Wearable on Right Shoulder"},
    {crc32c("Wearable on Right Wrist"), "Wearable on Right Wrist"},
    {crc32c("Wearable on Shield"), "Wearable on Shield"},
    {crc32c("Wearable on Waist"), "Wearable on Waist"},
    {crc32c("Wheeled Drone"), "Wheeled Drone"},
    {crc32c("Whips"), "Whips"},
    {crc32c("Width"), "Width"},
    {crc32c("Willpower-Based Skills"), "Willpower-Based Skills"},
    {crc32c("Willpower"), "Willpower"},
    {crc32c("Woodworking"), "Woodworking"},
    {crc32c("Youth Bonus"), "Youth Bonus"},
    {crc32c("Youth Penalty"), "Youth Penalty"},
    {crc32c("Zap"), "Zap"},
    {crc32c("Zero-G Combat"), "Zero-G Combat"},
    {crc32c("Zero-G Ops"), "Zero-G Ops"},
};

void save_skill_names_to(FILE* fl) {
  fprintf(fl, "%lu\n", skill_names.size());
  for (auto skn : skill_names) {
    if (skn.second.length() > 255) {
      fprintf(stderr, CRED "Error: Skill name too long: '%s'\n", skn.second.c_str());
      fprintf(fl, "%.8X:Undefined\n", skn.first);
    } else {
      fprintf(fl, "%.8X:%s\n", skn.first, skn.second.c_str());
    }
  }
}

void load_skill_names_from(FILE* fl) {
  int32_t size;
  fscanf(fl, "%d\n", &size);
  char buf[256];
  skill_names.reserve(size);
  for (int sk = 0; sk < size; ++sk) {
    uint32_t hash;
    fscanf(fl, "%X:%255[^\n]\n", &hash, buf);
    skill_names.emplace_back(std::make_pair(hash, buf));
  }
  std::sort(skill_names.begin(), skill_names.end());
}

static int defaults_init = 0;
static void init_defaults() {
  if (defaults_init)
    return;
  defaults_init = 1;

  std::sort(skill_names.begin(), skill_names.end());

  // Skill Definition: Acrobatics
  defaults[crc32c("Acrobatics")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Acrobatics"));
  skcat["Athletic Skills"].push_back(crc32c("Acrobatics"));
  skcat["Medieval Skills"].push_back(crc32c("Acrobatics"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Acrobatics"));
  skcat["Modern Skills"].push_back(crc32c("Acrobatics"));
  skcat["Shadowrun Skills"].push_back(crc32c("Acrobatics"));
  skcat["Skills"].push_back(crc32c("Acrobatics"));

  // Skill Definition: Acting
  defaults[crc32c("Acting")] = 3;
  skcat["Charisma-Based Skills"].push_back(crc32c("Acting"));
  skcat["Social Skills"].push_back(crc32c("Acting"));
  skcat["Medieval Skills"].push_back(crc32c("Acting"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Acting"));
  skcat["Modern Skills"].push_back(crc32c("Acting"));
  skcat["Shadowrun Skills"].push_back(crc32c("Acting"));
  skcat["Skills"].push_back(crc32c("Acting"));

  // Skill Definition: Activate Juju
  defaults[crc32c("Activate Juju")] = 7;
  skcat["Skill-Based Skills"].push_back(crc32c("Activate Juju"));
  skcat["Shamanistic Skills"].push_back(crc32c("Activate Juju"));
  skcat["Medieval Skills"].push_back(crc32c("Activate Juju"));
  skcat["Skills"].push_back(crc32c("Activate Juju"));

  // Skill Definition: Air Pistols
  defaults[crc32c("Air Pistols")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Air Pistols"));
  skcat["Pistol Skills"].push_back(crc32c("Air Pistols"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Air Pistols"));
  skcat["Modern Skills"].push_back(crc32c("Air Pistols"));
  skcat["Shadowrun Skills"].push_back(crc32c("Air Pistols"));
  skcat["Skills"].push_back(crc32c("Air Pistols"));
  add_wts(crc32c("Air Pistols"));

  // Skill Definition: Air Rifles
  defaults[crc32c("Air Rifles")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Air Rifles"));
  skcat["Rifle Skills"].push_back(crc32c("Air Rifles"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Air Rifles"));
  skcat["Modern Skills"].push_back(crc32c("Air Rifles"));
  skcat["Shadowrun Skills"].push_back(crc32c("Air Rifles"));
  skcat["Skills"].push_back(crc32c("Air Rifles"));
  add_wts(crc32c("Air Rifles"));

  // Skill Definition: Air Weaving
  defaults[crc32c("Air Weaving")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Air Weaving"));
  skcat["Magical Skills"].push_back(crc32c("Air Weaving"));
  skcat["Medieval Skills"].push_back(crc32c("Air Weaving"));
  skcat["FP Skills"].push_back(crc32c("Air Weaving"));
  skcat["Skills"].push_back(crc32c("Air Weaving"));

  // Skill Definition: Alchemy
  defaults[crc32c("Alchemy")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Alchemy"));
  skcat["Magical Skills"].push_back(crc32c("Alchemy"));
  skcat["Medieval Skills"].push_back(crc32c("Alchemy"));
  skcat["Shadowrun Skills"].push_back(crc32c("Alchemy"));
  skcat["Skills"].push_back(crc32c("Alchemy"));

  // Skill Definition: Amphibious Combat
  defaults[crc32c("Amphibious Combat")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Amphibious Combat"));
  skcat["Combat Environment Skills"].push_back(crc32c("Amphibious Combat"));
  skcat["Medieval Skills"].push_back(crc32c("Amphibious Combat"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Amphibious Combat"));
  skcat["Modern Skills"].push_back(crc32c("Amphibious Combat"));
  skcat["Shadowrun Skills"].push_back(crc32c("Amphibious Combat"));
  skcat["Skills"].push_back(crc32c("Amphibious Combat"));

  // Skill Definition: Archery
  defaults[crc32c("Archery")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Archery"));
  skcat["Ranged-Combat Skills"].push_back(crc32c("Archery"));
  skcat["Medieval Skills"].push_back(crc32c("Archery"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Archery"));
  skcat["Modern Skills"].push_back(crc32c("Archery"));
  skcat["Shadowrun Skills"].push_back(crc32c("Archery"));
  skcat["Skills"].push_back(crc32c("Archery"));
  add_wts(crc32c("Archery"));

  // Skill Definition: Arctic Waraxe Martial Art
  defaults[crc32c("Arctic Waraxe Martial Art")] = 2;
  skcat["Strength-Based Skills"].push_back(crc32c("Arctic Waraxe Martial Art"));
  skcat["Lost Martial Arts"].push_back(crc32c("Arctic Waraxe Martial Art"));
  skcat["Medieval Skills"].push_back(crc32c("Arctic Waraxe Martial Art"));
  skcat["Skills"].push_back(crc32c("Arctic Waraxe Martial Art"));

  // Skill Definition: Armoring
  defaults[crc32c("Armoring")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Armoring"));
  skcat["Creation Skills"].push_back(crc32c("Armoring"));
  skcat["Medieval Skills"].push_back(crc32c("Armoring"));
  skcat["Skills"].push_back(crc32c("Armoring"));

  // Skill Definition: Art
  defaults[crc32c("Art")] = 3;
  skcat["Charisma-Based Skills"].push_back(crc32c("Art"));
  skcat["Social Skills"].push_back(crc32c("Art"));
  skcat["Medieval Skills"].push_back(crc32c("Art"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Art"));
  skcat["Modern Skills"].push_back(crc32c("Art"));
  skcat["Shadowrun Skills"].push_back(crc32c("Art"));
  skcat["Skills"].push_back(crc32c("Art"));

  // Skill Definition: Artificing
  defaults[crc32c("Artificing")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Artificing"));
  skcat["Magical Skills"].push_back(crc32c("Artificing"));
  skcat["Medieval Skills"].push_back(crc32c("Artificing"));
  skcat["Skills"].push_back(crc32c("Artificing"));

  // Skill Definition: Artificing, Star
  defaults[crc32c("Artificing, Star")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Artificing, Star"));
  skcat["Star Magical Skills"].push_back(crc32c("Artificing, Star"));
  skcat["Medieval Skills"].push_back(crc32c("Artificing, Star"));
  skcat["Skills"].push_back(crc32c("Artificing, Star"));

  // Skill Definition: Assault Cannons
  defaults[crc32c("Assault Cannons")] = 2;
  skcat["Strength-Based Skills"].push_back(crc32c("Assault Cannons"));
  skcat["Heavy Firearm Skills"].push_back(crc32c("Assault Cannons"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Assault Cannons"));
  skcat["Modern Skills"].push_back(crc32c("Assault Cannons"));
  skcat["Shadowrun Skills"].push_back(crc32c("Assault Cannons"));
  skcat["Skills"].push_back(crc32c("Assault Cannons"));

  // Skill Definition: Assault Rifles
  defaults[crc32c("Assault Rifles")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Assault Rifles"));
  skcat["Rifle Skills"].push_back(crc32c("Assault Rifles"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Assault Rifles"));
  skcat["Modern Skills"].push_back(crc32c("Assault Rifles"));
  skcat["Shadowrun Skills"].push_back(crc32c("Assault Rifles"));
  skcat["Skills"].push_back(crc32c("Assault Rifles"));
  add_wts(crc32c("Assault Rifles"));

  // Skill Definition: Ballista
  defaults[crc32c("Ballista")] = 6;
  skcat["Reaction-Based Skills"].push_back(crc32c("Ballista"));
  skcat["Medieval Skills"].push_back(crc32c("Ballista"));
  skcat["Skills"].push_back(crc32c("Ballista"));

  // Skill Definition: Bike
  defaults[crc32c("Bike")] = 6;
  skcat["Reaction-Based Skills"].push_back(crc32c("Bike"));
  skcat["Piloting Skills"].push_back(crc32c("Bike"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Bike"));
  skcat["Modern Skills"].push_back(crc32c("Bike"));
  skcat["Shadowrun Skills"].push_back(crc32c("Bike"));
  skcat["Skills"].push_back(crc32c("Bike"));

  // Skill Definition: Blacksmithing
  defaults[crc32c("Blacksmithing")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Blacksmithing"));
  skcat["Creation Skills"].push_back(crc32c("Blacksmithing"));
  skcat["Medieval Skills"].push_back(crc32c("Blacksmithing"));
  skcat["Skills"].push_back(crc32c("Blacksmithing"));

  // Skill Definition: Blowgun
  defaults[crc32c("Blowgun")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Blowgun"));
  skcat["Ranged-Combat Skills"].push_back(crc32c("Blowgun"));
  skcat["Medieval Skills"].push_back(crc32c("Blowgun"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Blowgun"));
  skcat["Modern Skills"].push_back(crc32c("Blowgun"));
  skcat["Shadowrun Skills"].push_back(crc32c("Blowgun"));
  skcat["Skills"].push_back(crc32c("Blowgun"));
  add_wts(crc32c("Blowgun"));

  // Skill Definition: Boat, Powered
  defaults[crc32c("Boat, Powered")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Boat, Powered"));
  skcat["Piloting Skills"].push_back(crc32c("Boat, Powered"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Boat, Powered"));
  skcat["Modern Skills"].push_back(crc32c("Boat, Powered"));
  skcat["Shadowrun Skills"].push_back(crc32c("Boat, Powered"));
  skcat["Skills"].push_back(crc32c("Boat, Powered"));

  // Skill Definition: Boat, Row
  defaults[crc32c("Boat, Row")] = 2;
  skcat["Strength-Based Skills"].push_back(crc32c("Boat, Row"));
  skcat["Piloting Skills"].push_back(crc32c("Boat, Row"));
  skcat["Medieval Skills"].push_back(crc32c("Boat, Row"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Boat, Row"));
  skcat["Modern Skills"].push_back(crc32c("Boat, Row"));
  skcat["Shadowrun Skills"].push_back(crc32c("Boat, Row"));
  skcat["Skills"].push_back(crc32c("Boat, Row"));

  // Skill Definition: Boat, Sail
  defaults[crc32c("Boat, Sail")] = 6;
  skcat["Reaction-Based Skills"].push_back(crc32c("Boat, Sail"));
  skcat["Piloting Skills"].push_back(crc32c("Boat, Sail"));
  skcat["Medieval Skills"].push_back(crc32c("Boat, Sail"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Boat, Sail"));
  skcat["Modern Skills"].push_back(crc32c("Boat, Sail"));
  skcat["Shadowrun Skills"].push_back(crc32c("Boat, Sail"));
  skcat["Skills"].push_back(crc32c("Boat, Sail"));

  // Skill Definition: Body Checking
  defaults[crc32c("Body Checking")] = 0;
  skcat["Body-Based Skills"].push_back(crc32c("Body Checking"));
  skcat["Hand-to-Hand Combat Skills"].push_back(crc32c("Body Checking"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Body Checking"));
  skcat["Modern Skills"].push_back(crc32c("Body Checking"));
  skcat["Shadowrun Skills"].push_back(crc32c("Body Checking"));
  skcat["Medieval Skills"].push_back(crc32c("Body Checking"));
  skcat["Skills"].push_back(crc32c("Body Checking"));
  add_wts(crc32c("Body Checking"));

  // Skill Definition: Bowyer
  defaults[crc32c("Bowyer")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Bowyer"));
  skcat["Creation Skills"].push_back(crc32c("Bowyer"));
  skcat["Medieval Skills"].push_back(crc32c("Bowyer"));
  skcat["Skills"].push_back(crc32c("Bowyer"));

  // Skill Definition: Breath Weapon
  defaults[crc32c("Breath Weapon")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Breath Weapon"));
  skcat["Medieval Skills"].push_back(crc32c("Breath Weapon"));
  skcat["Shadowrun Skills"].push_back(crc32c("Breath Weapon"));
  skcat["Skills"].push_back(crc32c("Breath Weapon"));

  // Skill Definition: Buggy
  defaults[crc32c("Buggy")] = 6;
  skcat["Reaction-Based Skills"].push_back(crc32c("Buggy"));
  skcat["Piloting Skills"].push_back(crc32c("Buggy"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Buggy"));
  skcat["Modern Skills"].push_back(crc32c("Buggy"));
  skcat["Shadowrun Skills"].push_back(crc32c("Buggy"));
  skcat["Skills"].push_back(crc32c("Buggy"));

  // Skill Definition: Cannon
  defaults[crc32c("Cannon")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Cannon"));
  skcat["Heavy Firearm Skills"].push_back(crc32c("Cannon"));
  skcat["Medieval Skills"].push_back(crc32c("Cannon"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Cannon"));
  skcat["Modern Skills"].push_back(crc32c("Cannon"));
  skcat["Shadowrun Skills"].push_back(crc32c("Cannon"));
  skcat["Skills"].push_back(crc32c("Cannon"));

  // Skill Definition: Car
  defaults[crc32c("Car")] = 6;
  skcat["Reaction-Based Skills"].push_back(crc32c("Car"));
  skcat["Piloting Skills"].push_back(crc32c("Car"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Car"));
  skcat["Modern Skills"].push_back(crc32c("Car"));
  skcat["Shadowrun Skills"].push_back(crc32c("Car"));
  skcat["Skills"].push_back(crc32c("Car"));

  // Skill Definition: Carpentry
  defaults[crc32c("Carpentry")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Carpentry"));
  skcat["Creation Skills"].push_back(crc32c("Carpentry"));
  skcat["Medieval Skills"].push_back(crc32c("Carpentry"));
  skcat["Modern Skills"].push_back(crc32c("Carpentry"));
  skcat["Skills"].push_back(crc32c("Carpentry"));

  // Skill Definition: Carromeleg - Tier I
  defaults[crc32c("Carromeleg - Tier I")] = 0;
  skcat["Body-Based Skills"].push_back(crc32c("Carromeleg - Tier I"));
  skcat["Elven Martial Arts Skills"].push_back(crc32c("Carromeleg - Tier I"));
  skcat["Medieval Skills"].push_back(crc32c("Carromeleg - Tier I"));
  skcat["Shadowrun Skills"].push_back(crc32c("Carromeleg - Tier I"));
  skcat["Skills"].push_back(crc32c("Carromeleg - Tier I"));

  // Skill Definition: Carromeleg - Tier II
  defaults[crc32c("Carromeleg - Tier II")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Carromeleg - Tier II"));
  skcat["Elven Martial Arts Skills"].push_back(crc32c("Carromeleg - Tier II"));
  skcat["Medieval Skills"].push_back(crc32c("Carromeleg - Tier II"));
  skcat["Shadowrun Skills"].push_back(crc32c("Carromeleg - Tier II"));
  skcat["Skills"].push_back(crc32c("Carromeleg - Tier II"));

  // Skill Definition: Carromeleg - Tier III
  defaults[crc32c("Carromeleg - Tier III")] = 2;
  skcat["Strength-Based Skills"].push_back(crc32c("Carromeleg - Tier III"));
  skcat["Elven Martial Arts Skills"].push_back(crc32c("Carromeleg - Tier III"));
  skcat["Medieval Skills"].push_back(crc32c("Carromeleg - Tier III"));
  skcat["Shadowrun Skills"].push_back(crc32c("Carromeleg - Tier III"));
  skcat["Skills"].push_back(crc32c("Carromeleg - Tier III"));

  // Skill Definition: Carromeleg - Tier IV
  defaults[crc32c("Carromeleg - Tier IV")] = 3;
  skcat["Charisma-Based Skills"].push_back(crc32c("Carromeleg - Tier IV"));
  skcat["Elven Martial Arts Skills"].push_back(crc32c("Carromeleg - Tier IV"));
  skcat["Medieval Skills"].push_back(crc32c("Carromeleg - Tier IV"));
  skcat["Shadowrun Skills"].push_back(crc32c("Carromeleg - Tier IV"));
  skcat["Skills"].push_back(crc32c("Carromeleg - Tier IV"));

  // Skill Definition: Carromeleg - Tier V
  defaults[crc32c("Carromeleg - Tier V")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Carromeleg - Tier V"));
  skcat["Elven Martial Arts Skills"].push_back(crc32c("Carromeleg - Tier V"));
  skcat["Medieval Skills"].push_back(crc32c("Carromeleg - Tier V"));
  skcat["Shadowrun Skills"].push_back(crc32c("Carromeleg - Tier V"));
  skcat["Skills"].push_back(crc32c("Carromeleg - Tier V"));

  // Skill Definition: Carromeleg - Tier VI
  defaults[crc32c("Carromeleg - Tier VI")] = 5;
  skcat["Willpower-Based Skills"].push_back(crc32c("Carromeleg - Tier VI"));
  skcat["Elven Martial Arts Skills"].push_back(crc32c("Carromeleg - Tier VI"));
  skcat["Medieval Skills"].push_back(crc32c("Carromeleg - Tier VI"));
  skcat["Shadowrun Skills"].push_back(crc32c("Carromeleg - Tier VI"));
  skcat["Skills"].push_back(crc32c("Carromeleg - Tier VI"));

  // Skill Definition: Cart
  defaults[crc32c("Cart")] = 6;
  skcat["Reaction-Based Skills"].push_back(crc32c("Cart"));
  skcat["Piloting Skills"].push_back(crc32c("Cart"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Cart"));
  skcat["Modern Skills"].push_back(crc32c("Cart"));
  skcat["Shadowrun Skills"].push_back(crc32c("Cart"));
  skcat["Skills"].push_back(crc32c("Cart"));

  // Skill Definition: Catching
  defaults[crc32c("Catching")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Catching"));
  skcat["Athletic Skills"].push_back(crc32c("Catching"));
  skcat["Medieval Skills"].push_back(crc32c("Catching"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Catching"));
  skcat["Modern Skills"].push_back(crc32c("Catching"));
  skcat["Shadowrun Skills"].push_back(crc32c("Catching"));
  skcat["Skills"].push_back(crc32c("Catching"));

  // Skill Definition: Centering
  defaults[crc32c("Centering")] = 5;
  skcat["Willpower-Based Skills"].push_back(crc32c("Centering"));
  skcat["Star Magical Skills"].push_back(crc32c("Centering"));
  skcat["Medieval Skills"].push_back(crc32c("Centering"));
  skcat["Skills"].push_back(crc32c("Centering"));

  // Skill Definition: Climbing
  defaults[crc32c("Climbing")] = 2;
  skcat["Strength-Based Skills"].push_back(crc32c("Climbing"));
  skcat["Athletic Skills"].push_back(crc32c("Climbing"));
  skcat["Medieval Skills"].push_back(crc32c("Climbing"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Climbing"));
  skcat["Modern Skills"].push_back(crc32c("Climbing"));
  skcat["Shadowrun Skills"].push_back(crc32c("Climbing"));
  skcat["Skills"].push_back(crc32c("Climbing"));

  // Skill Definition: Clothier
  defaults[crc32c("Clothier")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Clothier"));
  skcat["Creation Skills"].push_back(crc32c("Clothier"));
  skcat["Medieval Skills"].push_back(crc32c("Clothier"));
  skcat["Skills"].push_back(crc32c("Clothier"));

  // Skill Definition: Communications
  defaults[crc32c("Communications")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Communications"));
  skcat["Technical Skills"].push_back(crc32c("Communications"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Communications"));
  skcat["Modern Skills"].push_back(crc32c("Communications"));
  skcat["Shadowrun Skills"].push_back(crc32c("Communications"));
  skcat["Skills"].push_back(crc32c("Communications"));

  // Skill Definition: Computer
  defaults[crc32c("Computer")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Computer"));
  skcat["Technical Skills"].push_back(crc32c("Computer"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Computer"));
  skcat["Modern Skills"].push_back(crc32c("Computer"));
  skcat["Shadowrun Skills"].push_back(crc32c("Computer"));
  skcat["Skills"].push_back(crc32c("Computer"));

  // Skill Definition: Computer Build/Repair
  defaults[crc32c("Computer Build/Repair")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Computer Build/Repair"));
  skcat["Build/Repair Skills"].push_back(crc32c("Computer Build/Repair"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Computer Build/Repair"));
  skcat["Modern Skills"].push_back(crc32c("Computer Build/Repair"));
  skcat["Shadowrun Skills"].push_back(crc32c("Computer Build/Repair"));
  skcat["Skills"].push_back(crc32c("Computer Build/Repair"));

  // Skill Definition: Computer Targeting
  defaults[crc32c("Computer Targeting")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Computer Targeting"));
  skcat["Weapon System Skills"].push_back(crc32c("Computer Targeting"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Computer Targeting"));
  skcat["Modern Skills"].push_back(crc32c("Computer Targeting"));
  skcat["Shadowrun Skills"].push_back(crc32c("Computer Targeting"));
  skcat["Skills"].push_back(crc32c("Computer Targeting"));

  // Skill Definition: Conjuring
  defaults[crc32c("Conjuring")] = 3;
  skcat["Charisma-Based Skills"].push_back(crc32c("Conjuring"));
  skcat["Magical Skills"].push_back(crc32c("Conjuring"));
  skcat["Medieval Skills"].push_back(crc32c("Conjuring"));
  skcat["Shadowrun Skills"].push_back(crc32c("Conjuring"));
  skcat["Skills"].push_back(crc32c("Conjuring"));

  // Skill Definition: Crossbow
  defaults[crc32c("Crossbow")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Crossbow"));
  skcat["Ranged-Combat Skills"].push_back(crc32c("Crossbow"));
  skcat["Medieval Skills"].push_back(crc32c("Crossbow"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Crossbow"));
  skcat["Modern Skills"].push_back(crc32c("Crossbow"));
  skcat["Shadowrun Skills"].push_back(crc32c("Crossbow"));
  skcat["Skills"].push_back(crc32c("Crossbow"));
  add_wts(crc32c("Crossbow"));

  // Skill Definition: Cyber Data Manipulation
  defaults[crc32c("Cyber Data Manipulation")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Cyber Data Manipulation"));
  skcat["Cyber Skills"].push_back(crc32c("Cyber Data Manipulation"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Cyber Data Manipulation"));
  skcat["Shadowrun Skills"].push_back(crc32c("Cyber Data Manipulation"));
  skcat["Specialty Skills"].push_back(crc32c("Cyber Data Manipulation"));
  skcat["Skills"].push_back(crc32c("Cyber Data Manipulation"));

  // Skill Definition: Cyber Vehicle Operation
  defaults[crc32c("Cyber Vehicle Operation")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Cyber Vehicle Operation"));
  skcat["Cyber Skills"].push_back(crc32c("Cyber Vehicle Operation"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Cyber Vehicle Operation"));
  skcat["Shadowrun Skills"].push_back(crc32c("Cyber Vehicle Operation"));
  skcat["Specialty Skills"].push_back(crc32c("Cyber Vehicle Operation"));
  skcat["Skills"].push_back(crc32c("Cyber Vehicle Operation"));

  // Skill Definition: Cycling
  defaults[crc32c("Cycling")] = 6;
  skcat["Reaction-Based Skills"].push_back(crc32c("Cycling"));
  skcat["Piloting Skills"].push_back(crc32c("Cycling"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Cycling"));
  skcat["Modern Skills"].push_back(crc32c("Cycling"));
  skcat["Shadowrun Skills"].push_back(crc32c("Cycling"));
  skcat["Skills"].push_back(crc32c("Cycling"));

  // Skill Definition: Damage Control
  defaults[crc32c("Damage Control")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Damage Control"));
  skcat["Technical Skills"].push_back(crc32c("Damage Control"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Damage Control"));
  skcat["Modern Skills"].push_back(crc32c("Damage Control"));
  skcat["Shadowrun Skills"].push_back(crc32c("Damage Control"));
  skcat["Skills"].push_back(crc32c("Damage Control"));

  // Skill Definition: Damage Resistance
  defaults[crc32c("Damage Resistance")] = 0;
  skcat["Body-Based Skills"].push_back(crc32c("Damage Resistance"));
  skcat["Medieval Skills"].push_back(crc32c("Damage Resistance"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Damage Resistance"));
  skcat["Modern Skills"].push_back(crc32c("Damage Resistance"));
  skcat["Shadowrun Skills"].push_back(crc32c("Damage Resistance"));
  skcat["Skills"].push_back(crc32c("Damage Resistance"));

  // Skill Definition: Demolitions
  defaults[crc32c("Demolitions")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Demolitions"));
  skcat["Technical Skills"].push_back(crc32c("Demolitions"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Demolitions"));
  skcat["Modern Skills"].push_back(crc32c("Demolitions"));
  skcat["Shadowrun Skills"].push_back(crc32c("Demolitions"));
  skcat["Skills"].push_back(crc32c("Demolitions"));

  // Skill Definition: Diplomacy
  defaults[crc32c("Diplomacy")] = 3;
  skcat["Charisma-Based Skills"].push_back(crc32c("Diplomacy"));
  skcat["Social Skills"].push_back(crc32c("Diplomacy"));
  skcat["Medieval Skills"].push_back(crc32c("Diplomacy"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Diplomacy"));
  skcat["Modern Skills"].push_back(crc32c("Diplomacy"));
  skcat["Shadowrun Skills"].push_back(crc32c("Diplomacy"));
  skcat["Skills"].push_back(crc32c("Diplomacy"));

  // Skill Definition: Diving
  defaults[crc32c("Diving")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Diving"));
  skcat["Athletic Skills"].push_back(crc32c("Diving"));
  skcat["Medieval Skills"].push_back(crc32c("Diving"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Diving"));
  skcat["Modern Skills"].push_back(crc32c("Diving"));
  skcat["Shadowrun Skills"].push_back(crc32c("Diving"));
  skcat["Skills"].push_back(crc32c("Diving"));

  // Skill Definition: Dodge
  defaults[crc32c("Dodge")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Dodge"));
  skcat["Athletic Skills"].push_back(crc32c("Dodge"));
  skcat["Medieval Skills"].push_back(crc32c("Dodge"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Dodge"));
  skcat["Modern Skills"].push_back(crc32c("Dodge"));
  skcat["Shadowrun Skills"].push_back(crc32c("Dodge"));
  skcat["Skills"].push_back(crc32c("Dodge"));

  // Skill Definition: Earth Weaving
  defaults[crc32c("Earth Weaving")] = 2;
  skcat["Strength-Based Skills"].push_back(crc32c("Earth Weaving"));
  skcat["Magical Skills"].push_back(crc32c("Earth Weaving"));
  skcat["Medieval Skills"].push_back(crc32c("Earth Weaving"));
  skcat["FP Skills"].push_back(crc32c("Earth Weaving"));
  skcat["Skills"].push_back(crc32c("Earth Weaving"));

  // Skill Definition: Electrical Build/Repair
  defaults[crc32c("Electrical Build/Repair")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Electrical Build/Repair"));
  skcat["Build/Repair Skills"].push_back(crc32c("Electrical Build/Repair"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Electrical Build/Repair"));
  skcat["Modern Skills"].push_back(crc32c("Electrical Build/Repair"));
  skcat["Shadowrun Skills"].push_back(crc32c("Electrical Build/Repair"));
  skcat["Skills"].push_back(crc32c("Electrical Build/Repair"));

  // Skill Definition: Electronics
  defaults[crc32c("Electronics")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Electronics"));
  skcat["Technical Skills"].push_back(crc32c("Electronics"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Electronics"));
  skcat["Modern Skills"].push_back(crc32c("Electronics"));
  skcat["Shadowrun Skills"].push_back(crc32c("Electronics"));
  skcat["Skills"].push_back(crc32c("Electronics"));

  // Skill Definition: Enchanting
  defaults[crc32c("Enchanting")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Enchanting"));
  skcat["Magical Skills"].push_back(crc32c("Enchanting"));
  skcat["Medieval Skills"].push_back(crc32c("Enchanting"));
  skcat["Shadowrun Skills"].push_back(crc32c("Enchanting"));
  skcat["Skills"].push_back(crc32c("Enchanting"));

  // Skill Definition: Endurance
  defaults[crc32c("Endurance")] = 2;
  skcat["Strength-Based Skills"].push_back(crc32c("Endurance"));
  skcat["Medieval Skills"].push_back(crc32c("Endurance"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Endurance"));
  skcat["Modern Skills"].push_back(crc32c("Endurance"));
  skcat["Shadowrun Skills"].push_back(crc32c("Endurance"));
  skcat["Skills"].push_back(crc32c("Endurance"));

  // Skill Definition: Engineering
  defaults[crc32c("Engineering")] = 4;
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
  defaults[crc32c("Etiquette")] = 3;
  skcat["Charisma-Based Skills"].push_back(crc32c("Etiquette"));
  skcat["Social Skills"].push_back(crc32c("Etiquette"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Etiquette"));
  skcat["Medieval Skills"].push_back(crc32c("Etiquette"));
  skcat["Modern Skills"].push_back(crc32c("Etiquette"));
  skcat["Shadowrun Skills"].push_back(crc32c("Etiquette"));
  skcat["Skills"].push_back(crc32c("Etiquette"));

  // Skill Definition: Evasion
  defaults[crc32c("Evasion")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Evasion"));
  skcat["Athletic Skills"].push_back(crc32c("Evasion"));
  skcat["Medieval Skills"].push_back(crc32c("Evasion"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Evasion"));
  skcat["Modern Skills"].push_back(crc32c("Evasion"));
  skcat["Shadowrun Skills"].push_back(crc32c("Evasion"));
  skcat["Skills"].push_back(crc32c("Evasion"));

  // Skill Definition: Finance
  defaults[crc32c("Finance")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Finance"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Finance"));
  skcat["Modern Skills"].push_back(crc32c("Finance"));
  skcat["Shadowrun Skills"].push_back(crc32c("Finance"));
  skcat["Skills"].push_back(crc32c("Finance"));

  // Skill Definition: Find Juju
  defaults[crc32c("Find Juju")] = 5;
  skcat["Willpower-Based Skills"].push_back(crc32c("Find Juju"));
  skcat["Shamanistic Skills"].push_back(crc32c("Find Juju"));
  skcat["Medieval Skills"].push_back(crc32c("Find Juju"));
  skcat["Skills"].push_back(crc32c("Find Juju"));

  // Skill Definition: Fire Weaving
  defaults[crc32c("Fire Weaving")] = 3;
  skcat["Charisma-Based Skills"].push_back(crc32c("Fire Weaving"));
  skcat["Magical Skills"].push_back(crc32c("Fire Weaving"));
  skcat["Medieval Skills"].push_back(crc32c("Fire Weaving"));
  skcat["FP Skills"].push_back(crc32c("Fire Weaving"));
  skcat["Skills"].push_back(crc32c("Fire Weaving"));

  // Skill Definition: First Aid
  defaults[crc32c("First Aid")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("First Aid"));
  skcat["Technical Skills"].push_back(crc32c("First Aid"));
  skcat["Cyberpunk Skills"].push_back(crc32c("First Aid"));
  skcat["Modern Skills"].push_back(crc32c("First Aid"));
  skcat["Shadowrun Skills"].push_back(crc32c("First Aid"));
  skcat["Skills"].push_back(crc32c("First Aid"));

  // Skill Definition: Fixed-Wing Craft
  defaults[crc32c("Fixed-Wing Craft")] = 6;
  skcat["Reaction-Based Skills"].push_back(crc32c("Fixed-Wing Craft"));
  skcat["Piloting Skills"].push_back(crc32c("Fixed-Wing Craft"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Fixed-Wing Craft"));
  skcat["Modern Skills"].push_back(crc32c("Fixed-Wing Craft"));
  skcat["Shadowrun Skills"].push_back(crc32c("Fixed-Wing Craft"));
  skcat["Skills"].push_back(crc32c("Fixed-Wing Craft"));

  // Skill Definition: Fixed-Wing Drone
  defaults[crc32c("Fixed-Wing Drone")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Fixed-Wing Drone"));
  skcat["Drone Piloting Skills"].push_back(crc32c("Fixed-Wing Drone"));
  skcat["Shadowrun Skills"].push_back(crc32c("Fixed-Wing Drone"));
  skcat["Skills"].push_back(crc32c("Fixed-Wing Drone"));

  // Skill Definition: Fletcher
  defaults[crc32c("Fletcher")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Fletcher"));
  skcat["Creation Skills"].push_back(crc32c("Fletcher"));
  skcat["Medieval Skills"].push_back(crc32c("Fletcher"));
  skcat["Skills"].push_back(crc32c("Fletcher"));

  // Skill Definition: Flight
  defaults[crc32c("Flight")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Flight"));
  skcat["Athletic Skills"].push_back(crc32c("Flight"));
  skcat["Medieval Skills"].push_back(crc32c("Flight"));
  skcat["Shadowrun Skills"].push_back(crc32c("Flight"));
  skcat["Skills"].push_back(crc32c("Flight"));

  // Skill Definition: Focusing
  defaults[crc32c("Focusing")] = 2;
  skcat["Strength-Based Skills"].push_back(crc32c("Focusing"));
  skcat["Star Magical Skills"].push_back(crc32c("Focusing"));
  skcat["Medieval Skills"].push_back(crc32c("Focusing"));
  skcat["Skills"].push_back(crc32c("Focusing"));

  // Skill Definition: Grappling
  defaults[crc32c("Grappling")] = 2;
  skcat["Strength-Based Skills"].push_back(crc32c("Grappling"));
  skcat["Hand-to-Hand Combat Skills"].push_back(crc32c("Grappling"));
  skcat["Medieval Skills"].push_back(crc32c("Grappling"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Grappling"));
  skcat["Modern Skills"].push_back(crc32c("Grappling"));
  skcat["Shadowrun Skills"].push_back(crc32c("Grappling"));
  skcat["Skills"].push_back(crc32c("Grappling"));
  add_wts(crc32c("Grappling"));

  // Skill Definition: Gunnery
  defaults[crc32c("Gunnery")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Gunnery"));
  skcat["Weapon System Skills"].push_back(crc32c("Gunnery"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Gunnery"));
  skcat["Modern Skills"].push_back(crc32c("Gunnery"));
  skcat["Shadowrun Skills"].push_back(crc32c("Gunnery"));
  skcat["Skills"].push_back(crc32c("Gunnery"));

  // Skill Definition: Half-Track
  defaults[crc32c("Half-Track")] = 6;
  skcat["Reaction-Based Skills"].push_back(crc32c("Half-Track"));
  skcat["Piloting Skills"].push_back(crc32c("Half-Track"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Half-Track"));
  skcat["Modern Skills"].push_back(crc32c("Half-Track"));
  skcat["Shadowrun Skills"].push_back(crc32c("Half-Track"));
  skcat["Skills"].push_back(crc32c("Half-Track"));

  // Skill Definition: Healing
  defaults[crc32c("Healing")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Healing"));
  skcat["Nature Skills"].push_back(crc32c("Healing"));
  skcat["Medieval Skills"].push_back(crc32c("Healing"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Healing"));
  skcat["Modern Skills"].push_back(crc32c("Healing"));
  skcat["Shadowrun Skills"].push_back(crc32c("Healing"));
  skcat["Skills"].push_back(crc32c("Healing"));

  // Skill Definition: Heavy Lasers
  defaults[crc32c("Heavy Lasers")] = 2;
  skcat["Strength-Based Skills"].push_back(crc32c("Heavy Lasers"));
  skcat["Heavy Firearm Skills"].push_back(crc32c("Heavy Lasers"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Heavy Lasers"));
  skcat["Modern Skills"].push_back(crc32c("Heavy Lasers"));
  skcat["Shadowrun Skills"].push_back(crc32c("Heavy Lasers"));
  skcat["Skills"].push_back(crc32c("Heavy Lasers"));

  // Skill Definition: Heavy Projectors
  defaults[crc32c("Heavy Projectors")] = 2;
  skcat["Strength-Based Skills"].push_back(crc32c("Heavy Projectors"));
  skcat["Heavy Firearm Skills"].push_back(crc32c("Heavy Projectors"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Heavy Projectors"));
  skcat["Modern Skills"].push_back(crc32c("Heavy Projectors"));
  skcat["Shadowrun Skills"].push_back(crc32c("Heavy Projectors"));
  skcat["Skills"].push_back(crc32c("Heavy Projectors"));

  // Skill Definition: Heavy Rifles
  defaults[crc32c("Heavy Rifles")] = 2;
  skcat["Strength-Based Skills"].push_back(crc32c("Heavy Rifles"));
  skcat["Heavy Firearm Skills"].push_back(crc32c("Heavy Rifles"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Heavy Rifles"));
  skcat["Modern Skills"].push_back(crc32c("Heavy Rifles"));
  skcat["Shadowrun Skills"].push_back(crc32c("Heavy Rifles"));
  skcat["Skills"].push_back(crc32c("Heavy Rifles"));

  // Skill Definition: Helmsman, Star
  defaults[crc32c("Helmsman, Star")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Helmsman, Star"));
  skcat["Magical Skills"].push_back(crc32c("Helmsman, Star"));
  skcat["Medieval Skills"].push_back(crc32c("Helmsman, Star"));
  skcat["Skills"].push_back(crc32c("Helmsman, Star"));

  // Skill Definition: High-G Combat
  defaults[crc32c("High-G Combat")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("High-G Combat"));
  skcat["Combat Environment Skills"].push_back(crc32c("High-G Combat"));
  skcat["Medieval Skills"].push_back(crc32c("High-G Combat"));
  skcat["Cyberpunk Skills"].push_back(crc32c("High-G Combat"));
  skcat["Modern Skills"].push_back(crc32c("High-G Combat"));
  skcat["Shadowrun Skills"].push_back(crc32c("High-G Combat"));
  skcat["Skills"].push_back(crc32c("High-G Combat"));

  // Skill Definition: High-G Ops
  defaults[crc32c("High-G Ops")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("High-G Ops"));
  skcat["Specialized Skills"].push_back(crc32c("High-G Ops"));
  skcat["Cyberpunk Skills"].push_back(crc32c("High-G Ops"));
  skcat["Shadowrun Skills"].push_back(crc32c("High-G Ops"));
  skcat["Skills"].push_back(crc32c("High-G Ops"));

  // Skill Definition: Hovercraft
  defaults[crc32c("Hovercraft")] = 6;
  skcat["Reaction-Based Skills"].push_back(crc32c("Hovercraft"));
  skcat["Piloting Skills"].push_back(crc32c("Hovercraft"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Hovercraft"));
  skcat["Modern Skills"].push_back(crc32c("Hovercraft"));
  skcat["Shadowrun Skills"].push_back(crc32c("Hovercraft"));
  skcat["Skills"].push_back(crc32c("Hovercraft"));

  // Skill Definition: Hover Drone
  defaults[crc32c("Hover Drone")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Hover Drone"));
  skcat["Drone Piloting Skills"].push_back(crc32c("Hover Drone"));
  skcat["Shadowrun Skills"].push_back(crc32c("Hover Drone"));
  skcat["Skills"].push_back(crc32c("Hover Drone"));

  // Skill Definition: Hurling
  defaults[crc32c("Hurling")] = 2;
  skcat["Strength-Based Skills"].push_back(crc32c("Hurling"));
  skcat["Ranged-Combat Skills"].push_back(crc32c("Hurling"));
  skcat["Medieval Skills"].push_back(crc32c("Hurling"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Hurling"));
  skcat["Modern Skills"].push_back(crc32c("Hurling"));
  skcat["Shadowrun Skills"].push_back(crc32c("Hurling"));
  skcat["Skills"].push_back(crc32c("Hurling"));
  add_wts(crc32c("Hurling"));

  // Skill Definition: Identify Juju
  defaults[crc32c("Identify Juju")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Identify Juju"));
  skcat["Shamanistic Skills"].push_back(crc32c("Identify Juju"));
  skcat["Medieval Skills"].push_back(crc32c("Identify Juju"));
  skcat["Skills"].push_back(crc32c("Identify Juju"));

  // Skill Definition: Interrogation
  defaults[crc32c("Interrogation")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Interrogation"));
  skcat["Social Skills"].push_back(crc32c("Interrogation"));
  skcat["Medieval Skills"].push_back(crc32c("Interrogation"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Interrogation"));
  skcat["Modern Skills"].push_back(crc32c("Interrogation"));
  skcat["Shadowrun Skills"].push_back(crc32c("Interrogation"));
  skcat["Skills"].push_back(crc32c("Interrogation"));

  // Skill Definition: Intimidation
  defaults[crc32c("Intimidation")] = 5;
  skcat["Willpower-Based Skills"].push_back(crc32c("Intimidation"));
  skcat["Social Skills"].push_back(crc32c("Intimidation"));
  skcat["Medieval Skills"].push_back(crc32c("Intimidation"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Intimidation"));
  skcat["Modern Skills"].push_back(crc32c("Intimidation"));
  skcat["Shadowrun Skills"].push_back(crc32c("Intimidation"));
  skcat["Skills"].push_back(crc32c("Intimidation"));

  // Skill Definition: Jumping
  defaults[crc32c("Jumping")] = 2;
  skcat["Strength-Based Skills"].push_back(crc32c("Jumping"));
  skcat["Athletic Skills"].push_back(crc32c("Jumping"));
  skcat["Medieval Skills"].push_back(crc32c("Jumping"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Jumping"));
  skcat["Modern Skills"].push_back(crc32c("Jumping"));
  skcat["Shadowrun Skills"].push_back(crc32c("Jumping"));
  skcat["Skills"].push_back(crc32c("Jumping"));

  // Skill Definition: Kicking
  defaults[crc32c("Kicking")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Kicking"));
  skcat["Hand-to-Hand Combat Skills"].push_back(crc32c("Kicking"));
  skcat["Medieval Skills"].push_back(crc32c("Kicking"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Kicking"));
  skcat["Modern Skills"].push_back(crc32c("Kicking"));
  skcat["Shadowrun Skills"].push_back(crc32c("Kicking"));
  skcat["Skills"].push_back(crc32c("Kicking"));
  add_wts(crc32c("Kicking"));

  // Skill Definition: Knowledge
  defaults[crc32c("Knowledge")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Knowledge"));
  skcat["Specialized Skills"].push_back(crc32c("Knowledge"));
  skcat["Medieval Skills"].push_back(crc32c("Knowledge"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Knowledge"));
  skcat["Modern Skills"].push_back(crc32c("Knowledge"));
  skcat["Shadowrun Skills"].push_back(crc32c("Knowledge"));
  skcat["Skills"].push_back(crc32c("Knowledge"));

  // Skill Definition: Laser Pistols
  defaults[crc32c("Laser Pistols")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Laser Pistols"));
  skcat["Pistol Skills"].push_back(crc32c("Laser Pistols"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Laser Pistols"));
  skcat["Modern Skills"].push_back(crc32c("Laser Pistols"));
  skcat["Shadowrun Skills"].push_back(crc32c("Laser Pistols"));
  skcat["Skills"].push_back(crc32c("Laser Pistols"));
  add_wts(crc32c("Laser Pistols"));

  // Skill Definition: Laser Rifles
  defaults[crc32c("Laser Rifles")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Laser Rifles"));
  skcat["Rifle Skills"].push_back(crc32c("Laser Rifles"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Laser Rifles"));
  skcat["Modern Skills"].push_back(crc32c("Laser Rifles"));
  skcat["Shadowrun Skills"].push_back(crc32c("Laser Rifles"));
  skcat["Skills"].push_back(crc32c("Laser Rifles"));
  add_wts(crc32c("Laser Rifles"));

  // Skill Definition: Lasso
  defaults[crc32c("Lasso")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Lasso"));
  skcat["Ranged-Combat Skills"].push_back(crc32c("Lasso"));
  skcat["Medieval Skills"].push_back(crc32c("Lasso"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Lasso"));
  skcat["Modern Skills"].push_back(crc32c("Lasso"));
  skcat["Shadowrun Skills"].push_back(crc32c("Lasso"));
  skcat["Skills"].push_back(crc32c("Lasso"));
  add_wts(crc32c("Lasso"));

  // Skill Definition: Launch Weapons
  defaults[crc32c("Launch Weapons")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Launch Weapons"));
  skcat["Weapon System Skills"].push_back(crc32c("Launch Weapons"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Launch Weapons"));
  skcat["Modern Skills"].push_back(crc32c("Launch Weapons"));
  skcat["Shadowrun Skills"].push_back(crc32c("Launch Weapons"));
  skcat["Skills"].push_back(crc32c("Launch Weapons"));

  // Skill Definition: Leadership
  defaults[crc32c("Leadership")] = 3;
  skcat["Charisma-Based Skills"].push_back(crc32c("Leadership"));
  skcat["Social Skills"].push_back(crc32c("Leadership"));
  skcat["Medieval Skills"].push_back(crc32c("Leadership"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Leadership"));
  skcat["Modern Skills"].push_back(crc32c("Leadership"));
  skcat["Shadowrun Skills"].push_back(crc32c("Leadership"));
  skcat["Skills"].push_back(crc32c("Leadership"));

  // Skill Definition: Leatherworking
  defaults[crc32c("Leatherworking")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Leatherworking"));
  skcat["Creation Skills"].push_back(crc32c("Leatherworking"));
  skcat["Medieval Skills"].push_back(crc32c("Leatherworking"));
  skcat["Skills"].push_back(crc32c("Leatherworking"));

  // Skill Definition: Lifting
  defaults[crc32c("Lifting")] = 2;
  skcat["Strength-Based Skills"].push_back(crc32c("Lifting"));
  skcat["Athletic Skills"].push_back(crc32c("Lifting"));
  skcat["Medieval Skills"].push_back(crc32c("Lifting"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Lifting"));
  skcat["Modern Skills"].push_back(crc32c("Lifting"));
  skcat["Shadowrun Skills"].push_back(crc32c("Lifting"));
  skcat["Skills"].push_back(crc32c("Lifting"));

  // Skill Definition: Long Blades
  defaults[crc32c("Long Blades")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Long Blades"));
  skcat["Melee-Combat Skills"].push_back(crc32c("Long Blades"));
  skcat["Medieval Skills"].push_back(crc32c("Long Blades"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Long Blades"));
  skcat["Modern Skills"].push_back(crc32c("Long Blades"));
  skcat["Shadowrun Skills"].push_back(crc32c("Long Blades"));
  skcat["Skills"].push_back(crc32c("Long Blades"));
  add_wts(crc32c("Long Blades"));

  // Skill Definition: Long Cleaves
  defaults[crc32c("Long Cleaves")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Long Cleaves"));
  skcat["Melee-Combat Skills"].push_back(crc32c("Long Cleaves"));
  skcat["Medieval Skills"].push_back(crc32c("Long Cleaves"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Long Cleaves"));
  skcat["Modern Skills"].push_back(crc32c("Long Cleaves"));
  skcat["Shadowrun Skills"].push_back(crc32c("Long Cleaves"));
  skcat["Skills"].push_back(crc32c("Long Cleaves"));
  add_wts(crc32c("Long Cleaves"));

  // Skill Definition: Long Crushing
  defaults[crc32c("Long Crushing")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Long Crushing"));
  skcat["Melee-Combat Skills"].push_back(crc32c("Long Crushing"));
  skcat["Medieval Skills"].push_back(crc32c("Long Crushing"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Long Crushing"));
  skcat["Modern Skills"].push_back(crc32c("Long Crushing"));
  skcat["Shadowrun Skills"].push_back(crc32c("Long Crushing"));
  skcat["Skills"].push_back(crc32c("Long Crushing"));
  add_wts(crc32c("Long Crushing"));

  // Skill Definition: Long Flails
  defaults[crc32c("Long Flails")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Long Flails"));
  skcat["Melee-Combat Skills"].push_back(crc32c("Long Flails"));
  skcat["Medieval Skills"].push_back(crc32c("Long Flails"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Long Flails"));
  skcat["Modern Skills"].push_back(crc32c("Long Flails"));
  skcat["Shadowrun Skills"].push_back(crc32c("Long Flails"));
  skcat["Skills"].push_back(crc32c("Long Flails"));
  add_wts(crc32c("Long Flails"));

  // Skill Definition: Long Piercing
  defaults[crc32c("Long Piercing")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Long Piercing"));
  skcat["Melee-Combat Skills"].push_back(crc32c("Long Piercing"));
  skcat["Medieval Skills"].push_back(crc32c("Long Piercing"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Long Piercing"));
  skcat["Modern Skills"].push_back(crc32c("Long Piercing"));
  skcat["Shadowrun Skills"].push_back(crc32c("Long Piercing"));
  skcat["Skills"].push_back(crc32c("Long Piercing"));
  add_wts(crc32c("Long Piercing"));

  // Skill Definition: Long Staves
  defaults[crc32c("Long Staves")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Long Staves"));
  skcat["Melee-Combat Skills"].push_back(crc32c("Long Staves"));
  skcat["Medieval Skills"].push_back(crc32c("Long Staves"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Long Staves"));
  skcat["Modern Skills"].push_back(crc32c("Long Staves"));
  skcat["Shadowrun Skills"].push_back(crc32c("Long Staves"));
  skcat["Skills"].push_back(crc32c("Long Staves"));
  add_wts(crc32c("Long Staves"));

  // Skill Definition: Low-G Combat
  defaults[crc32c("Low-G Combat")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Low-G Combat"));
  skcat["Combat Environment Skills"].push_back(crc32c("Low-G Combat"));
  skcat["Medieval Skills"].push_back(crc32c("Low-G Combat"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Low-G Combat"));
  skcat["Modern Skills"].push_back(crc32c("Low-G Combat"));
  skcat["Shadowrun Skills"].push_back(crc32c("Low-G Combat"));
  skcat["Skills"].push_back(crc32c("Low-G Combat"));

  // Skill Definition: Low-G Ops
  defaults[crc32c("Low-G Ops")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Low-G Ops"));
  skcat["Specialized Skills"].push_back(crc32c("Low-G Ops"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Low-G Ops"));
  skcat["Shadowrun Skills"].push_back(crc32c("Low-G Ops"));
  skcat["Skills"].push_back(crc32c("Low-G Ops"));

  // Skill Definition: Lumberjack
  defaults[crc32c("Lumberjack")] = 2;
  skcat["Strength-Based Skills"].push_back(crc32c("Lumberjack"));
  skcat["Creation Skills"].push_back(crc32c("Lumberjack"));
  skcat["Medieval Skills"].push_back(crc32c("Lumberjack"));
  skcat["Modern Skills"].push_back(crc32c("Lumberjack"));
  skcat["Skills"].push_back(crc32c("Lumberjack"));

  // Skill Definition: Machine Guns
  defaults[crc32c("Machine Guns")] = 2;
  skcat["Strength-Based Skills"].push_back(crc32c("Machine Guns"));
  skcat["Heavy Firearm Skills"].push_back(crc32c("Machine Guns"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Machine Guns"));
  skcat["Modern Skills"].push_back(crc32c("Machine Guns"));
  skcat["Shadowrun Skills"].push_back(crc32c("Machine Guns"));
  skcat["Skills"].push_back(crc32c("Machine Guns"));

  // Skill Definition: Machine Pistols
  defaults[crc32c("Machine Pistols")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Machine Pistols"));
  skcat["Pistol Skills"].push_back(crc32c("Machine Pistols"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Machine Pistols"));
  skcat["Modern Skills"].push_back(crc32c("Machine Pistols"));
  skcat["Shadowrun Skills"].push_back(crc32c("Machine Pistols"));
  skcat["Skills"].push_back(crc32c("Machine Pistols"));
  add_wts(crc32c("Machine Pistols"));

  // Skill Definition: Magic Manipulation
  defaults[crc32c("Magic Manipulation")] = 3;
  skcat["Charisma-Based Skills"].push_back(crc32c("Magic Manipulation"));
  skcat["Magical Skills"].push_back(crc32c("Magic Manipulation"));
  skcat["Medieval Skills"].push_back(crc32c("Magic Manipulation"));
  skcat["Skills"].push_back(crc32c("Magic Manipulation"));

  // Skill Definition: Martial Arts, Elven
  defaults[crc32c("Martial Arts, Elven")] = 5;
  skcat["Willpower-Based Skills"].push_back(crc32c("Martial Arts, Elven"));
  skcat["Martial Arts Skills"].push_back(crc32c("Martial Arts, Elven"));
  skcat["Medieval Skills"].push_back(crc32c("Martial Arts, Elven"));
  skcat["Skills"].push_back(crc32c("Martial Arts, Elven"));

  // Skill Definition: Masonry
  defaults[crc32c("Masonry")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Masonry"));
  skcat["Creation Skills"].push_back(crc32c("Masonry"));
  skcat["Medieval Skills"].push_back(crc32c("Masonry"));
  skcat["Skills"].push_back(crc32c("Masonry"));

  // Skill Definition: Mechanical Build/Repair
  defaults[crc32c("Mechanical Build/Repair")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Mechanical Build/Repair"));
  skcat["Build/Repair Skills"].push_back(crc32c("Mechanical Build/Repair"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Mechanical Build/Repair"));
  skcat["Modern Skills"].push_back(crc32c("Mechanical Build/Repair"));
  skcat["Shadowrun Skills"].push_back(crc32c("Mechanical Build/Repair"));
  skcat["Skills"].push_back(crc32c("Mechanical Build/Repair"));

  // Skill Definition: Mechanics
  defaults[crc32c("Mechanics")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Mechanics"));
  skcat["Technical Skills"].push_back(crc32c("Mechanics"));
  skcat["Medieval Skills"].push_back(crc32c("Mechanics"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Mechanics"));
  skcat["Modern Skills"].push_back(crc32c("Mechanics"));
  skcat["Shadowrun Skills"].push_back(crc32c("Mechanics"));
  skcat["Skills"].push_back(crc32c("Mechanics"));

  // Skill Definition: Metalworking
  defaults[crc32c("Metalworking")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Metalworking"));
  skcat["Creation Skills"].push_back(crc32c("Metalworking"));
  skcat["Medieval Skills"].push_back(crc32c("Metalworking"));
  skcat["Skills"].push_back(crc32c("Metalworking"));

  // Skill Definition: Mindcasting
  defaults[crc32c("Mindcasting")] = 5;
  skcat["Willpower-Based Skills"].push_back(crc32c("Mindcasting"));
  skcat["Magical Skills"].push_back(crc32c("Mindcasting"));
  skcat["Medieval Skills"].push_back(crc32c("Mindcasting"));
  skcat["Shadowrun Skills"].push_back(crc32c("Mindcasting"));
  skcat["Skills"].push_back(crc32c("Mindcasting"));

  // Skill Definition: Mounted Air Pistols
  defaults[crc32c("Mounted Air Pistols")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Mounted Air Pistols"));
  skcat["Mounted Pistol Skills"].push_back(crc32c("Mounted Air Pistols"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Mounted Air Pistols"));
  skcat["Modern Skills"].push_back(crc32c("Mounted Air Pistols"));
  skcat["Shadowrun Skills"].push_back(crc32c("Mounted Air Pistols"));
  skcat["Skills"].push_back(crc32c("Mounted Air Pistols"));
  add_wts(crc32c("Mounted Air Pistols"));

  // Skill Definition: Mounted Archery
  defaults[crc32c("Mounted Archery")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Mounted Archery"));
  skcat["Mounted Ranged-Combat Skills"].push_back(crc32c("Mounted Archery"));
  skcat["Medieval Skills"].push_back(crc32c("Mounted Archery"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Mounted Archery"));
  skcat["Modern Skills"].push_back(crc32c("Mounted Archery"));
  skcat["Shadowrun Skills"].push_back(crc32c("Mounted Archery"));
  skcat["Skills"].push_back(crc32c("Mounted Archery"));
  add_wts(crc32c("Mounted Archery"));

  // Skill Definition: Mounted Blades
  defaults[crc32c("Mounted Blades")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Mounted Blades"));
  skcat["Mounted Melee-Combat Skills"].push_back(crc32c("Mounted Blades"));
  skcat["Medieval Skills"].push_back(crc32c("Mounted Blades"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Mounted Blades"));
  skcat["Modern Skills"].push_back(crc32c("Mounted Blades"));
  skcat["Shadowrun Skills"].push_back(crc32c("Mounted Blades"));
  skcat["Skills"].push_back(crc32c("Mounted Blades"));
  add_wts(crc32c("Mounted Blades"));

  // Skill Definition: Mounted Blowgun
  defaults[crc32c("Mounted Blowgun")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Mounted Blowgun"));
  skcat["Mounted Ranged-Combat Skills"].push_back(crc32c("Mounted Blowgun"));
  skcat["Medieval Skills"].push_back(crc32c("Mounted Blowgun"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Mounted Blowgun"));
  skcat["Modern Skills"].push_back(crc32c("Mounted Blowgun"));
  skcat["Shadowrun Skills"].push_back(crc32c("Mounted Blowgun"));
  skcat["Skills"].push_back(crc32c("Mounted Blowgun"));
  add_wts(crc32c("Mounted Blowgun"));

  // Skill Definition: Mounted Cleaves
  defaults[crc32c("Mounted Cleaves")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Mounted Cleaves"));
  skcat["Mounted Melee-Combat Skills"].push_back(crc32c("Mounted Cleaves"));
  skcat["Medieval Skills"].push_back(crc32c("Mounted Cleaves"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Mounted Cleaves"));
  skcat["Modern Skills"].push_back(crc32c("Mounted Cleaves"));
  skcat["Shadowrun Skills"].push_back(crc32c("Mounted Cleaves"));
  skcat["Skills"].push_back(crc32c("Mounted Cleaves"));
  add_wts(crc32c("Mounted Cleaves"));

  // Skill Definition: Mounted Crossbow
  defaults[crc32c("Mounted Crossbow")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Mounted Crossbow"));
  skcat["Mounted Ranged-Combat Skills"].push_back(crc32c("Mounted Crossbow"));
  skcat["Medieval Skills"].push_back(crc32c("Mounted Crossbow"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Mounted Crossbow"));
  skcat["Modern Skills"].push_back(crc32c("Mounted Crossbow"));
  skcat["Shadowrun Skills"].push_back(crc32c("Mounted Crossbow"));
  skcat["Skills"].push_back(crc32c("Mounted Crossbow"));
  add_wts(crc32c("Mounted Crossbow"));

  // Skill Definition: Mounted Crushing
  defaults[crc32c("Mounted Crushing")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Mounted Crushing"));
  skcat["Mounted Melee-Combat Skills"].push_back(crc32c("Mounted Crushing"));
  skcat["Medieval Skills"].push_back(crc32c("Mounted Crushing"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Mounted Crushing"));
  skcat["Modern Skills"].push_back(crc32c("Mounted Crushing"));
  skcat["Shadowrun Skills"].push_back(crc32c("Mounted Crushing"));
  skcat["Skills"].push_back(crc32c("Mounted Crushing"));
  add_wts(crc32c("Mounted Crushing"));

  // Skill Definition: Mounted Flails
  defaults[crc32c("Mounted Flails")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Mounted Flails"));
  skcat["Mounted Melee-Combat Skills"].push_back(crc32c("Mounted Flails"));
  skcat["Medieval Skills"].push_back(crc32c("Mounted Flails"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Mounted Flails"));
  skcat["Modern Skills"].push_back(crc32c("Mounted Flails"));
  skcat["Shadowrun Skills"].push_back(crc32c("Mounted Flails"));
  skcat["Skills"].push_back(crc32c("Mounted Flails"));
  add_wts(crc32c("Mounted Flails"));

  // Skill Definition: Mounted Hurling
  defaults[crc32c("Mounted Hurling")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Mounted Hurling"));
  skcat["Mounted Ranged-Combat Skills"].push_back(crc32c("Mounted Hurling"));
  skcat["Medieval Skills"].push_back(crc32c("Mounted Hurling"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Mounted Hurling"));
  skcat["Modern Skills"].push_back(crc32c("Mounted Hurling"));
  skcat["Shadowrun Skills"].push_back(crc32c("Mounted Hurling"));
  skcat["Skills"].push_back(crc32c("Mounted Hurling"));
  add_wts(crc32c("Mounted Hurling"));

  // Skill Definition: Mounted Laser Pistols
  defaults[crc32c("Mounted Laser Pistols")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Mounted Laser Pistols"));
  skcat["Mounted Pistol Skills"].push_back(crc32c("Mounted Laser Pistols"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Mounted Laser Pistols"));
  skcat["Modern Skills"].push_back(crc32c("Mounted Laser Pistols"));
  skcat["Shadowrun Skills"].push_back(crc32c("Mounted Laser Pistols"));
  skcat["Skills"].push_back(crc32c("Mounted Laser Pistols"));
  add_wts(crc32c("Mounted Laser Pistols"));

  // Skill Definition: Mounted Machine Pistols
  defaults[crc32c("Mounted Machine Pistols")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Mounted Machine Pistols"));
  skcat["Mounted Pistol Skills"].push_back(crc32c("Mounted Machine Pistols"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Mounted Machine Pistols"));
  skcat["Modern Skills"].push_back(crc32c("Mounted Machine Pistols"));
  skcat["Shadowrun Skills"].push_back(crc32c("Mounted Machine Pistols"));
  skcat["Skills"].push_back(crc32c("Mounted Machine Pistols"));
  add_wts(crc32c("Mounted Machine Pistols"));

  // Skill Definition: Mounted Nets
  defaults[crc32c("Mounted Nets")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Mounted Nets"));
  skcat["Mounted Ranged-Combat Skills"].push_back(crc32c("Mounted Nets"));
  skcat["Medieval Skills"].push_back(crc32c("Mounted Nets"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Mounted Nets"));
  skcat["Modern Skills"].push_back(crc32c("Mounted Nets"));
  skcat["Shadowrun Skills"].push_back(crc32c("Mounted Nets"));
  skcat["Skills"].push_back(crc32c("Mounted Nets"));
  add_wts(crc32c("Mounted Nets"));

  // Skill Definition: Mounted Piercing
  defaults[crc32c("Mounted Piercing")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Mounted Piercing"));
  skcat["Mounted Melee-Combat Skills"].push_back(crc32c("Mounted Piercing"));
  skcat["Medieval Skills"].push_back(crc32c("Mounted Piercing"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Mounted Piercing"));
  skcat["Modern Skills"].push_back(crc32c("Mounted Piercing"));
  skcat["Shadowrun Skills"].push_back(crc32c("Mounted Piercing"));
  skcat["Skills"].push_back(crc32c("Mounted Piercing"));
  add_wts(crc32c("Mounted Piercing"));

  // Skill Definition: Mounted Pistols
  defaults[crc32c("Mounted Pistols")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Mounted Pistols"));
  skcat["Mounted Pistol Skills"].push_back(crc32c("Mounted Pistols"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Mounted Pistols"));
  skcat["Modern Skills"].push_back(crc32c("Mounted Pistols"));
  skcat["Shadowrun Skills"].push_back(crc32c("Mounted Pistols"));
  skcat["Skills"].push_back(crc32c("Mounted Pistols"));
  add_wts(crc32c("Mounted Pistols"));

  // Skill Definition: Mounted Plasma Pistols
  defaults[crc32c("Mounted Plasma Pistols")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Mounted Plasma Pistols"));
  skcat["Mounted Pistol Skills"].push_back(crc32c("Mounted Plasma Pistols"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Mounted Plasma Pistols"));
  skcat["Modern Skills"].push_back(crc32c("Mounted Plasma Pistols"));
  skcat["Shadowrun Skills"].push_back(crc32c("Mounted Plasma Pistols"));
  skcat["Skills"].push_back(crc32c("Mounted Plasma Pistols"));
  add_wts(crc32c("Mounted Plasma Pistols"));

  // Skill Definition: Mounted Shot Pistols
  defaults[crc32c("Mounted Shot Pistols")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Mounted Shot Pistols"));
  skcat["Mounted Pistol Skills"].push_back(crc32c("Mounted Shot Pistols"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Mounted Shot Pistols"));
  skcat["Modern Skills"].push_back(crc32c("Mounted Shot Pistols"));
  skcat["Shadowrun Skills"].push_back(crc32c("Mounted Shot Pistols"));
  skcat["Skills"].push_back(crc32c("Mounted Shot Pistols"));
  add_wts(crc32c("Mounted Shot Pistols"));

  // Skill Definition: Mounted Slings
  defaults[crc32c("Mounted Slings")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Mounted Slings"));
  skcat["Mounted Ranged-Combat Skills"].push_back(crc32c("Mounted Slings"));
  skcat["Medieval Skills"].push_back(crc32c("Mounted Slings"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Mounted Slings"));
  skcat["Modern Skills"].push_back(crc32c("Mounted Slings"));
  skcat["Shadowrun Skills"].push_back(crc32c("Mounted Slings"));
  skcat["Skills"].push_back(crc32c("Mounted Slings"));
  add_wts(crc32c("Mounted Slings"));

  // Skill Definition: Mounted SMGs
  defaults[crc32c("Mounted SMGs")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Mounted SMGs"));
  skcat["Mounted Pistol Skills"].push_back(crc32c("Mounted SMGs"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Mounted SMGs"));
  skcat["Modern Skills"].push_back(crc32c("Mounted SMGs"));
  skcat["Shadowrun Skills"].push_back(crc32c("Mounted SMGs"));
  skcat["Skills"].push_back(crc32c("Mounted SMGs"));
  add_wts(crc32c("Mounted SMGs"));

  // Skill Definition: Mounted Throwing, Aero
  defaults[crc32c("Mounted Throwing, Aero")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Mounted Throwing, Aero"));
  skcat["Mounted Ranged-Combat Skills"].push_back(crc32c("Mounted Throwing, Aero"));
  skcat["Medieval Skills"].push_back(crc32c("Mounted Throwing, Aero"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Mounted Throwing, Aero"));
  skcat["Modern Skills"].push_back(crc32c("Mounted Throwing, Aero"));
  skcat["Shadowrun Skills"].push_back(crc32c("Mounted Throwing, Aero"));
  skcat["Skills"].push_back(crc32c("Mounted Throwing, Aero"));
  add_wts(crc32c("Mounted Throwing, Aero"));

  // Skill Definition: Mounted Throwing, Non-Aero
  defaults[crc32c("Mounted Throwing, Non-Aero")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Mounted Throwing, Non-Aero"));
  skcat["Mounted Ranged-Combat Skills"].push_back(crc32c("Mounted Throwing, Non-Aero"));
  skcat["Medieval Skills"].push_back(crc32c("Mounted Throwing, Non-Aero"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Mounted Throwing, Non-Aero"));
  skcat["Modern Skills"].push_back(crc32c("Mounted Throwing, Non-Aero"));
  skcat["Shadowrun Skills"].push_back(crc32c("Mounted Throwing, Non-Aero"));
  skcat["Skills"].push_back(crc32c("Mounted Throwing, Non-Aero"));
  add_wts(crc32c("Mounted Throwing, Non-Aero"));

  // Skill Definition: Mounted Whips
  defaults[crc32c("Mounted Whips")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Mounted Whips"));
  skcat["Mounted Ranged-Combat Skills"].push_back(crc32c("Mounted Whips"));
  skcat["Medieval Skills"].push_back(crc32c("Mounted Whips"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Mounted Whips"));
  skcat["Modern Skills"].push_back(crc32c("Mounted Whips"));
  skcat["Shadowrun Skills"].push_back(crc32c("Mounted Whips"));
  skcat["Skills"].push_back(crc32c("Mounted Whips"));
  add_wts(crc32c("Mounted Whips"));

  // Skill Definition: Navigation
  defaults[crc32c("Navigation")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Navigation"));
  skcat["Nature Skills"].push_back(crc32c("Navigation"));
  skcat["Medieval Skills"].push_back(crc32c("Navigation"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Navigation"));
  skcat["Modern Skills"].push_back(crc32c("Navigation"));
  skcat["Shadowrun Skills"].push_back(crc32c("Navigation"));
  skcat["Skills"].push_back(crc32c("Navigation"));

  // Skill Definition: Negotiation
  defaults[crc32c("Negotiation")] = 3;
  skcat["Charisma-Based Skills"].push_back(crc32c("Negotiation"));
  skcat["Social Skills"].push_back(crc32c("Negotiation"));
  skcat["Medieval Skills"].push_back(crc32c("Negotiation"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Negotiation"));
  skcat["Modern Skills"].push_back(crc32c("Negotiation"));
  skcat["Shadowrun Skills"].push_back(crc32c("Negotiation"));
  skcat["Skills"].push_back(crc32c("Negotiation"));

  // Skill Definition: Nets
  defaults[crc32c("Nets")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Nets"));
  skcat["Ranged-Combat Skills"].push_back(crc32c("Nets"));
  skcat["Medieval Skills"].push_back(crc32c("Nets"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Nets"));
  skcat["Modern Skills"].push_back(crc32c("Nets"));
  skcat["Shadowrun Skills"].push_back(crc32c("Nets"));
  skcat["Skills"].push_back(crc32c("Nets"));
  add_wts(crc32c("Nets"));

  // Skill Definition: Neural Interface
  defaults[crc32c("Neural Interface")] = 5;
  skcat["Willpower-Based Skills"].push_back(crc32c("Neural Interface"));
  skcat["Technical Skills"].push_back(crc32c("Neural Interface"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Neural Interface"));
  skcat["Shadowrun Skills"].push_back(crc32c("Neural Interface"));
  skcat["Skills"].push_back(crc32c("Neural Interface"));

  // Skill Definition: Offhand Air Pistols
  defaults[crc32c("Offhand Air Pistols")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Offhand Air Pistols"));
  skcat["Pistol Skills"].push_back(crc32c("Offhand Air Pistols"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Offhand Air Pistols"));
  skcat["Modern Skills"].push_back(crc32c("Offhand Air Pistols"));
  skcat["Shadowrun Skills"].push_back(crc32c("Offhand Air Pistols"));
  skcat["Skills"].push_back(crc32c("Offhand Air Pistols"));
  add_wts(crc32c("Offhand Air Pistols"));

  // Skill Definition: Offhand Blades
  defaults[crc32c("Offhand Blades")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Offhand Blades"));
  skcat["Melee-Combat Skills"].push_back(crc32c("Offhand Blades"));
  skcat["Medieval Skills"].push_back(crc32c("Offhand Blades"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Offhand Blades"));
  skcat["Modern Skills"].push_back(crc32c("Offhand Blades"));
  skcat["Shadowrun Skills"].push_back(crc32c("Offhand Blades"));
  skcat["Skills"].push_back(crc32c("Offhand Blades"));
  add_wts(crc32c("Offhand Blades"));

  // Skill Definition: Offhand Cleaves
  defaults[crc32c("Offhand Cleaves")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Offhand Cleaves"));
  skcat["Melee-Combat Skills"].push_back(crc32c("Offhand Cleaves"));
  skcat["Medieval Skills"].push_back(crc32c("Offhand Cleaves"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Offhand Cleaves"));
  skcat["Modern Skills"].push_back(crc32c("Offhand Cleaves"));
  skcat["Shadowrun Skills"].push_back(crc32c("Offhand Cleaves"));
  skcat["Skills"].push_back(crc32c("Offhand Cleaves"));
  add_wts(crc32c("Offhand Cleaves"));

  // Skill Definition: Offhand Crossbow
  defaults[crc32c("Offhand Crossbow")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Offhand Crossbow"));
  skcat["Ranged-Combat Skills"].push_back(crc32c("Offhand Crossbow"));
  skcat["Medieval Skills"].push_back(crc32c("Offhand Crossbow"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Offhand Crossbow"));
  skcat["Modern Skills"].push_back(crc32c("Offhand Crossbow"));
  skcat["Shadowrun Skills"].push_back(crc32c("Offhand Crossbow"));
  skcat["Skills"].push_back(crc32c("Offhand Crossbow"));
  add_wts(crc32c("Offhand Crossbow"));

  // Skill Definition: Offhand Crushing
  defaults[crc32c("Offhand Crushing")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Offhand Crushing"));
  skcat["Melee-Combat Skills"].push_back(crc32c("Offhand Crushing"));
  skcat["Medieval Skills"].push_back(crc32c("Offhand Crushing"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Offhand Crushing"));
  skcat["Modern Skills"].push_back(crc32c("Offhand Crushing"));
  skcat["Shadowrun Skills"].push_back(crc32c("Offhand Crushing"));
  skcat["Skills"].push_back(crc32c("Offhand Crushing"));
  add_wts(crc32c("Offhand Crushing"));

  // Skill Definition: Offhand Flails
  defaults[crc32c("Offhand Flails")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Offhand Flails"));
  skcat["Melee-Combat Skills"].push_back(crc32c("Offhand Flails"));
  skcat["Medieval Skills"].push_back(crc32c("Offhand Flails"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Offhand Flails"));
  skcat["Modern Skills"].push_back(crc32c("Offhand Flails"));
  skcat["Shadowrun Skills"].push_back(crc32c("Offhand Flails"));
  skcat["Skills"].push_back(crc32c("Offhand Flails"));
  add_wts(crc32c("Offhand Flails"));

  // Skill Definition: Offhand Hurling
  defaults[crc32c("Offhand Hurling")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Offhand Hurling"));
  skcat["Ranged-Combat Skills"].push_back(crc32c("Offhand Hurling"));
  skcat["Medieval Skills"].push_back(crc32c("Offhand Hurling"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Offhand Hurling"));
  skcat["Modern Skills"].push_back(crc32c("Offhand Hurling"));
  skcat["Shadowrun Skills"].push_back(crc32c("Offhand Hurling"));
  skcat["Skills"].push_back(crc32c("Offhand Hurling"));
  add_wts(crc32c("Offhand Hurling"));

  // Skill Definition: Offhand Laser Pistols
  defaults[crc32c("Offhand Laser Pistols")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Offhand Laser Pistols"));
  skcat["Pistol Skills"].push_back(crc32c("Offhand Laser Pistols"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Offhand Laser Pistols"));
  skcat["Modern Skills"].push_back(crc32c("Offhand Laser Pistols"));
  skcat["Shadowrun Skills"].push_back(crc32c("Offhand Laser Pistols"));
  skcat["Skills"].push_back(crc32c("Offhand Laser Pistols"));
  add_wts(crc32c("Offhand Laser Pistols"));

  // Skill Definition: Offhand Machine Pistols
  defaults[crc32c("Offhand Machine Pistols")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Offhand Machine Pistols"));
  skcat["Pistol Skills"].push_back(crc32c("Offhand Machine Pistols"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Offhand Machine Pistols"));
  skcat["Modern Skills"].push_back(crc32c("Offhand Machine Pistols"));
  skcat["Shadowrun Skills"].push_back(crc32c("Offhand Machine Pistols"));
  skcat["Skills"].push_back(crc32c("Offhand Machine Pistols"));
  add_wts(crc32c("Offhand Machine Pistols"));

  // Skill Definition: Offhand Piercing
  defaults[crc32c("Offhand Piercing")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Offhand Piercing"));
  skcat["Melee-Combat Skills"].push_back(crc32c("Offhand Piercing"));
  skcat["Medieval Skills"].push_back(crc32c("Offhand Piercing"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Offhand Piercing"));
  skcat["Modern Skills"].push_back(crc32c("Offhand Piercing"));
  skcat["Shadowrun Skills"].push_back(crc32c("Offhand Piercing"));
  skcat["Skills"].push_back(crc32c("Offhand Piercing"));
  add_wts(crc32c("Offhand Piercing"));

  // Skill Definition: Offhand Pistols
  defaults[crc32c("Offhand Pistols")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Offhand Pistols"));
  skcat["Pistol Skills"].push_back(crc32c("Offhand Pistols"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Offhand Pistols"));
  skcat["Modern Skills"].push_back(crc32c("Offhand Pistols"));
  skcat["Shadowrun Skills"].push_back(crc32c("Offhand Pistols"));
  skcat["Skills"].push_back(crc32c("Offhand Pistols"));
  add_wts(crc32c("Offhand Pistols"));

  // Skill Definition: Offhand Plasma Pistols
  defaults[crc32c("Offhand Plasma Pistols")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Offhand Plasma Pistols"));
  skcat["Pistol Skills"].push_back(crc32c("Offhand Plasma Pistols"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Offhand Plasma Pistols"));
  skcat["Modern Skills"].push_back(crc32c("Offhand Plasma Pistols"));
  skcat["Shadowrun Skills"].push_back(crc32c("Offhand Plasma Pistols"));
  skcat["Skills"].push_back(crc32c("Offhand Plasma Pistols"));
  add_wts(crc32c("Offhand Plasma Pistols"));

  // Skill Definition: Offhand Punching
  defaults[crc32c("Offhand Punching")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Offhand Punching"));
  skcat["Medieval Skills"].push_back(crc32c("Offhand Punching"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Offhand Punching"));
  skcat["Modern Skills"].push_back(crc32c("Offhand Punching"));
  skcat["Shadowrun Skills"].push_back(crc32c("Offhand Punching"));
  skcat["Skills"].push_back(crc32c("Offhand Punching"));

  // Skill Definition: Offhand Shot Pistols
  defaults[crc32c("Offhand Shot Pistols")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Offhand Shot Pistols"));
  skcat["Pistol Skills"].push_back(crc32c("Offhand Shot Pistols"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Offhand Shot Pistols"));
  skcat["Modern Skills"].push_back(crc32c("Offhand Shot Pistols"));
  skcat["Shadowrun Skills"].push_back(crc32c("Offhand Shot Pistols"));
  skcat["Skills"].push_back(crc32c("Offhand Shot Pistols"));
  add_wts(crc32c("Offhand Shot Pistols"));

  // Skill Definition: Offhand SMGs
  defaults[crc32c("Offhand SMGs")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Offhand SMGs"));
  skcat["Pistol Skills"].push_back(crc32c("Offhand SMGs"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Offhand SMGs"));
  skcat["Modern Skills"].push_back(crc32c("Offhand SMGs"));
  skcat["Shadowrun Skills"].push_back(crc32c("Offhand SMGs"));
  skcat["Skills"].push_back(crc32c("Offhand SMGs"));
  add_wts(crc32c("Offhand SMGs"));

  // Skill Definition: Offhand Staves
  defaults[crc32c("Offhand Staves")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Offhand Staves"));
  skcat["Melee-Combat Skills"].push_back(crc32c("Offhand Staves"));
  skcat["Medieval Skills"].push_back(crc32c("Offhand Staves"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Offhand Staves"));
  skcat["Modern Skills"].push_back(crc32c("Offhand Staves"));
  skcat["Shadowrun Skills"].push_back(crc32c("Offhand Staves"));
  skcat["Skills"].push_back(crc32c("Offhand Staves"));
  add_wts(crc32c("Offhand Staves"));

  // Skill Definition: Offhand Throwing, Aero
  defaults[crc32c("Offhand Throwing, Aero")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Offhand Throwing, Aero"));
  skcat["Ranged-Combat Skills"].push_back(crc32c("Offhand Throwing, Aero"));
  skcat["Medieval Skills"].push_back(crc32c("Offhand Throwing, Aero"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Offhand Throwing, Aero"));
  skcat["Modern Skills"].push_back(crc32c("Offhand Throwing, Aero"));
  skcat["Shadowrun Skills"].push_back(crc32c("Offhand Throwing, Aero"));
  skcat["Skills"].push_back(crc32c("Offhand Throwing, Aero"));
  add_wts(crc32c("Offhand Throwing, Aero"));

  // Skill Definition: Offhand Throwing, Non-Aero
  defaults[crc32c("Offhand Throwing, Non-Aero")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Offhand Throwing, Non-Aero"));
  skcat["Ranged-Combat Skills"].push_back(crc32c("Offhand Throwing, Non-Aero"));
  skcat["Medieval Skills"].push_back(crc32c("Offhand Throwing, Non-Aero"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Offhand Throwing, Non-Aero"));
  skcat["Modern Skills"].push_back(crc32c("Offhand Throwing, Non-Aero"));
  skcat["Shadowrun Skills"].push_back(crc32c("Offhand Throwing, Non-Aero"));
  skcat["Skills"].push_back(crc32c("Offhand Throwing, Non-Aero"));
  add_wts(crc32c("Offhand Throwing, Non-Aero"));

  // Skill Definition: Offhand Whips
  defaults[crc32c("Offhand Whips")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Offhand Whips"));
  skcat["Ranged-Combat Skills"].push_back(crc32c("Offhand Whips"));
  skcat["Medieval Skills"].push_back(crc32c("Offhand Whips"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Offhand Whips"));
  skcat["Modern Skills"].push_back(crc32c("Offhand Whips"));
  skcat["Shadowrun Skills"].push_back(crc32c("Offhand Whips"));
  skcat["Skills"].push_back(crc32c("Offhand Whips"));
  add_wts(crc32c("Offhand Whips"));

  // Skill Definition: Perception
  defaults[crc32c("Perception")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Perception"));
  skcat["Specialized Skills"].push_back(crc32c("Perception"));
  skcat["Medieval Skills"].push_back(crc32c("Perception"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Perception"));
  skcat["Modern Skills"].push_back(crc32c("Perception"));
  skcat["Shadowrun Skills"].push_back(crc32c("Perception"));
  skcat["Expert Skills"].push_back(crc32c("Perception"));
  skcat["Skills"].push_back(crc32c("Perception"));

  // Skill Definition: Performance
  defaults[crc32c("Performance")] = 3;
  skcat["Charisma-Based Skills"].push_back(crc32c("Performance"));
  skcat["Social Skills"].push_back(crc32c("Performance"));
  skcat["Medieval Skills"].push_back(crc32c("Performance"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Performance"));
  skcat["Modern Skills"].push_back(crc32c("Performance"));
  skcat["Shadowrun Skills"].push_back(crc32c("Performance"));
  skcat["Skills"].push_back(crc32c("Performance"));

  // Skill Definition: Pistols
  defaults[crc32c("Pistols")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Pistols"));
  skcat["Pistol Skills"].push_back(crc32c("Pistols"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Pistols"));
  skcat["Modern Skills"].push_back(crc32c("Pistols"));
  skcat["Shadowrun Skills"].push_back(crc32c("Pistols"));
  skcat["Skills"].push_back(crc32c("Pistols"));
  add_wts(crc32c("Pistols"));

  // Skill Definition: Plasma Cannons
  defaults[crc32c("Plasma Cannons")] = 2;
  skcat["Strength-Based Skills"].push_back(crc32c("Plasma Cannons"));
  skcat["Heavy Firearm Skills"].push_back(crc32c("Plasma Cannons"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Plasma Cannons"));
  skcat["Modern Skills"].push_back(crc32c("Plasma Cannons"));
  skcat["Shadowrun Skills"].push_back(crc32c("Plasma Cannons"));
  skcat["Skills"].push_back(crc32c("Plasma Cannons"));

  // Skill Definition: Plasma Pistols
  defaults[crc32c("Plasma Pistols")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Plasma Pistols"));
  skcat["Pistol Skills"].push_back(crc32c("Plasma Pistols"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Plasma Pistols"));
  skcat["Modern Skills"].push_back(crc32c("Plasma Pistols"));
  skcat["Shadowrun Skills"].push_back(crc32c("Plasma Pistols"));
  skcat["Skills"].push_back(crc32c("Plasma Pistols"));
  add_wts(crc32c("Plasma Pistols"));

  // Skill Definition: Plasma Rifles
  defaults[crc32c("Plasma Rifles")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Plasma Rifles"));
  skcat["Rifle Skills"].push_back(crc32c("Plasma Rifles"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Plasma Rifles"));
  skcat["Modern Skills"].push_back(crc32c("Plasma Rifles"));
  skcat["Shadowrun Skills"].push_back(crc32c("Plasma Rifles"));
  skcat["Skills"].push_back(crc32c("Plasma Rifles"));
  add_wts(crc32c("Plasma Rifles"));

  // Skill Definition: Power Distribution
  defaults[crc32c("Power Distribution")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Power Distribution"));
  skcat["Technical Skills"].push_back(crc32c("Power Distribution"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Power Distribution"));
  skcat["Modern Skills"].push_back(crc32c("Power Distribution"));
  skcat["Shadowrun Skills"].push_back(crc32c("Power Distribution"));
  skcat["Skills"].push_back(crc32c("Power Distribution"));

  // Skill Definition: Power Suit
  defaults[crc32c("Power Suit")] = 6;
  skcat["Reaction-Based Skills"].push_back(crc32c("Power Suit"));
  skcat["Piloting Skills"].push_back(crc32c("Power Suit"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Power Suit"));
  skcat["Shadowrun Skills"].push_back(crc32c("Power Suit"));
  skcat["Skills"].push_back(crc32c("Power Suit"));

  // Skill Definition: Prepared Simple
  defaults[crc32c("Prepared Simple")] = 5;
  skcat["Willpower-Based Skills"].push_back(crc32c("Prepared Simple"));
  skcat["Star Magical Skills"].push_back(crc32c("Prepared Simple"));
  skcat["Medieval Skills"].push_back(crc32c("Prepared Simple"));
  skcat["Skills"].push_back(crc32c("Prepared Simple"));

  // Skill Definition: Punching
  defaults[crc32c("Punching")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Punching"));
  skcat["Hand-to-Hand Combat Skills"].push_back(crc32c("Punching"));
  skcat["Medieval Skills"].push_back(crc32c("Punching"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Punching"));
  skcat["Modern Skills"].push_back(crc32c("Punching"));
  skcat["Shadowrun Skills"].push_back(crc32c("Punching"));
  skcat["Skills"].push_back(crc32c("Punching"));
  add_wts(crc32c("Punching"));

  // Skill Definition: Quickdraw
  defaults[crc32c("Quickdraw")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Quickdraw"));
  skcat["Specialized Skills"].push_back(crc32c("Quickdraw"));
  skcat["Medieval Skills"].push_back(crc32c("Quickdraw"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Quickdraw"));
  skcat["Modern Skills"].push_back(crc32c("Quickdraw"));
  skcat["Shadowrun Skills"].push_back(crc32c("Quickdraw"));
  skcat["Skills"].push_back(crc32c("Quickdraw"));

  // Skill Definition: Race Car
  defaults[crc32c("Race Car")] = 6;
  skcat["Reaction-Based Skills"].push_back(crc32c("Race Car"));
  skcat["Piloting Skills"].push_back(crc32c("Race Car"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Race Car"));
  skcat["Modern Skills"].push_back(crc32c("Race Car"));
  skcat["Shadowrun Skills"].push_back(crc32c("Race Car"));
  skcat["Skills"].push_back(crc32c("Race Car"));

  // Skill Definition: Refine Juju
  defaults[crc32c("Refine Juju")] = 3;
  skcat["Charisma-Based Skills"].push_back(crc32c("Refine Juju"));
  skcat["Shamanistic Skills"].push_back(crc32c("Refine Juju"));
  skcat["Medieval Skills"].push_back(crc32c("Refine Juju"));
  skcat["Skills"].push_back(crc32c("Refine Juju"));

  // Skill Definition: Research
  defaults[crc32c("Research")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Research"));
  skcat["Specialized Skills"].push_back(crc32c("Research"));
  skcat["Medieval Skills"].push_back(crc32c("Research"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Research"));
  skcat["Modern Skills"].push_back(crc32c("Research"));
  skcat["Shadowrun Skills"].push_back(crc32c("Research"));
  skcat["Specialty Skills"].push_back(crc32c("Research"));
  skcat["Skills"].push_back(crc32c("Research"));

  // Skill Definition: Riding
  defaults[crc32c("Riding")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Riding"));
  skcat["Athletic Skills"].push_back(crc32c("Riding"));
  skcat["Medieval Skills"].push_back(crc32c("Riding"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Riding"));
  skcat["Modern Skills"].push_back(crc32c("Riding"));
  skcat["Shadowrun Skills"].push_back(crc32c("Riding"));
  skcat["Skills"].push_back(crc32c("Riding"));

  // Skill Definition: Rifles
  defaults[crc32c("Rifles")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Rifles"));
  skcat["Rifle Skills"].push_back(crc32c("Rifles"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Rifles"));
  skcat["Modern Skills"].push_back(crc32c("Rifles"));
  skcat["Shadowrun Skills"].push_back(crc32c("Rifles"));
  skcat["Skills"].push_back(crc32c("Rifles"));
  add_wts(crc32c("Rifles"));

  // Skill Definition: Ritual Conjuring
  defaults[crc32c("Ritual Conjuring")] = 5;
  skcat["Willpower-Based Skills"].push_back(crc32c("Ritual Conjuring"));
  skcat["Magical Skills"].push_back(crc32c("Ritual Conjuring"));
  skcat["Medieval Skills"].push_back(crc32c("Ritual Conjuring"));
  skcat["Shadowrun Skills"].push_back(crc32c("Ritual Conjuring"));
  skcat["Skills"].push_back(crc32c("Ritual Conjuring"));

  // Skill Definition: Ritual Spellcasting
  defaults[crc32c("Ritual Spellcasting")] = 5;
  skcat["Willpower-Based Skills"].push_back(crc32c("Ritual Spellcasting"));
  skcat["Magical Skills"].push_back(crc32c("Ritual Spellcasting"));
  skcat["Medieval Skills"].push_back(crc32c("Ritual Spellcasting"));
  skcat["Shadowrun Skills"].push_back(crc32c("Ritual Spellcasting"));
  skcat["Skills"].push_back(crc32c("Ritual Spellcasting"));

  // Skill Definition: Rod Targeting
  defaults[crc32c("Rod Targeting")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Rod Targeting"));
  skcat["Magical Skills"].push_back(crc32c("Rod Targeting"));
  skcat["Medieval Skills"].push_back(crc32c("Rod Targeting"));
  skcat["Skills"].push_back(crc32c("Rod Targeting"));

  // Skill Definition: Rotorcraft
  defaults[crc32c("Rotorcraft")] = 6;
  skcat["Reaction-Based Skills"].push_back(crc32c("Rotorcraft"));
  skcat["Piloting Skills"].push_back(crc32c("Rotorcraft"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Rotorcraft"));
  skcat["Modern Skills"].push_back(crc32c("Rotorcraft"));
  skcat["Shadowrun Skills"].push_back(crc32c("Rotorcraft"));
  skcat["Skills"].push_back(crc32c("Rotorcraft"));

  // Skill Definition: Rotor Drone
  defaults[crc32c("Rotor Drone")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Rotor Drone"));
  skcat["Drone Piloting Skills"].push_back(crc32c("Rotor Drone"));
  skcat["Shadowrun Skills"].push_back(crc32c("Rotor Drone"));
  skcat["Skills"].push_back(crc32c("Rotor Drone"));

  // Skill Definition: Running
  defaults[crc32c("Running")] = 2;
  skcat["Strength-Based Skills"].push_back(crc32c("Running"));
  skcat["Athletic Skills"].push_back(crc32c("Running"));
  skcat["Medieval Skills"].push_back(crc32c("Running"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Running"));
  skcat["Modern Skills"].push_back(crc32c("Running"));
  skcat["Shadowrun Skills"].push_back(crc32c("Running"));
  skcat["Skills"].push_back(crc32c("Running"));

  // Skill Definition: Saurian Line Attacking
  defaults[crc32c("Saurian Line Attacking")] = 2;
  skcat["Strength-Based Skills"].push_back(crc32c("Saurian Line Attacking"));
  skcat["Star Martial Arts"].push_back(crc32c("Saurian Line Attacking"));
  skcat["Medieval Skills"].push_back(crc32c("Saurian Line Attacking"));
  skcat["Skills"].push_back(crc32c("Saurian Line Attacking"));

  // Skill Definition: Security
  defaults[crc32c("Security")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Security"));
  skcat["Technical Skills"].push_back(crc32c("Security"));
  skcat["Medieval Skills"].push_back(crc32c("Security"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Security"));
  skcat["Modern Skills"].push_back(crc32c("Security"));
  skcat["Shadowrun Skills"].push_back(crc32c("Security"));
  skcat["Skills"].push_back(crc32c("Security"));

  // Skill Definition: Seduction
  defaults[crc32c("Seduction")] = 3;
  skcat["Charisma-Based Skills"].push_back(crc32c("Seduction"));
  skcat["Social Skills"].push_back(crc32c("Seduction"));
  skcat["Medieval Skills"].push_back(crc32c("Seduction"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Seduction"));
  skcat["Modern Skills"].push_back(crc32c("Seduction"));
  skcat["Shadowrun Skills"].push_back(crc32c("Seduction"));
  skcat["Skills"].push_back(crc32c("Seduction"));

  // Skill Definition: Self Artificing
  defaults[crc32c("Self Artificing")] = 3;
  skcat["Charisma-Based Skills"].push_back(crc32c("Self Artificing"));
  skcat["Magical Skills"].push_back(crc32c("Self Artificing"));
  skcat["Medieval Skills"].push_back(crc32c("Self Artificing"));
  skcat["Skills"].push_back(crc32c("Self Artificing"));

  // Skill Definition: Sensors
  defaults[crc32c("Sensors")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Sensors"));
  skcat["Technical Skills"].push_back(crc32c("Sensors"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Sensors"));
  skcat["Modern Skills"].push_back(crc32c("Sensors"));
  skcat["Shadowrun Skills"].push_back(crc32c("Sensors"));
  skcat["Skills"].push_back(crc32c("Sensors"));

  // Skill Definition: Shields
  defaults[crc32c("Shields")] = 2;
  skcat["Strength-Based Skills"].push_back(crc32c("Shields"));
  skcat["Melee-Combat Skills"].push_back(crc32c("Shields"));
  skcat["Medieval Skills"].push_back(crc32c("Shields"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Shields"));
  skcat["Modern Skills"].push_back(crc32c("Shields"));
  skcat["Shadowrun Skills"].push_back(crc32c("Shields"));
  skcat["Skills"].push_back(crc32c("Shields"));
  add_wts(crc32c("Shields"));

  // Skill Definition: Ship
  defaults[crc32c("Ship")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Ship"));
  skcat["Piloting Skills"].push_back(crc32c("Ship"));
  skcat["Medieval Skills"].push_back(crc32c("Ship"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Ship"));
  skcat["Modern Skills"].push_back(crc32c("Ship"));
  skcat["Shadowrun Skills"].push_back(crc32c("Ship"));
  skcat["Skills"].push_back(crc32c("Ship"));

  // Skill Definition: Ship, Powered
  defaults[crc32c("Ship, Powered")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Ship, Powered"));
  skcat["Piloting Skills"].push_back(crc32c("Ship, Powered"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Ship, Powered"));
  skcat["Modern Skills"].push_back(crc32c("Ship, Powered"));
  skcat["Shadowrun Skills"].push_back(crc32c("Ship, Powered"));
  skcat["Skills"].push_back(crc32c("Ship, Powered"));

  // Skill Definition: Shipwright
  defaults[crc32c("Shipwright")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Shipwright"));
  skcat["Creation Skills"].push_back(crc32c("Shipwright"));
  skcat["Medieval Skills"].push_back(crc32c("Shipwright"));
  skcat["Skills"].push_back(crc32c("Shipwright"));

  // Skill Definition: Short Blades
  defaults[crc32c("Short Blades")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Short Blades"));
  skcat["Melee-Combat Skills"].push_back(crc32c("Short Blades"));
  skcat["Medieval Skills"].push_back(crc32c("Short Blades"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Short Blades"));
  skcat["Modern Skills"].push_back(crc32c("Short Blades"));
  skcat["Shadowrun Skills"].push_back(crc32c("Short Blades"));
  skcat["Skills"].push_back(crc32c("Short Blades"));
  add_wts(crc32c("Short Blades"));

  // Skill Definition: Short Cleaves
  defaults[crc32c("Short Cleaves")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Short Cleaves"));
  skcat["Melee-Combat Skills"].push_back(crc32c("Short Cleaves"));
  skcat["Medieval Skills"].push_back(crc32c("Short Cleaves"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Short Cleaves"));
  skcat["Modern Skills"].push_back(crc32c("Short Cleaves"));
  skcat["Shadowrun Skills"].push_back(crc32c("Short Cleaves"));
  skcat["Skills"].push_back(crc32c("Short Cleaves"));
  add_wts(crc32c("Short Cleaves"));

  // Skill Definition: Short Crushing
  defaults[crc32c("Short Crushing")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Short Crushing"));
  skcat["Melee-Combat Skills"].push_back(crc32c("Short Crushing"));
  skcat["Medieval Skills"].push_back(crc32c("Short Crushing"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Short Crushing"));
  skcat["Modern Skills"].push_back(crc32c("Short Crushing"));
  skcat["Shadowrun Skills"].push_back(crc32c("Short Crushing"));
  skcat["Skills"].push_back(crc32c("Short Crushing"));
  add_wts(crc32c("Short Crushing"));

  // Skill Definition: Short Flails
  defaults[crc32c("Short Flails")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Short Flails"));
  skcat["Melee-Combat Skills"].push_back(crc32c("Short Flails"));
  skcat["Medieval Skills"].push_back(crc32c("Short Flails"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Short Flails"));
  skcat["Modern Skills"].push_back(crc32c("Short Flails"));
  skcat["Shadowrun Skills"].push_back(crc32c("Short Flails"));
  skcat["Skills"].push_back(crc32c("Short Flails"));
  add_wts(crc32c("Short Flails"));

  // Skill Definition: Short Piercing
  defaults[crc32c("Short Piercing")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Short Piercing"));
  skcat["Melee-Combat Skills"].push_back(crc32c("Short Piercing"));
  skcat["Medieval Skills"].push_back(crc32c("Short Piercing"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Short Piercing"));
  skcat["Modern Skills"].push_back(crc32c("Short Piercing"));
  skcat["Shadowrun Skills"].push_back(crc32c("Short Piercing"));
  skcat["Skills"].push_back(crc32c("Short Piercing"));
  add_wts(crc32c("Short Piercing"));

  // Skill Definition: Short Staves
  defaults[crc32c("Short Staves")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Short Staves"));
  skcat["Melee-Combat Skills"].push_back(crc32c("Short Staves"));
  skcat["Medieval Skills"].push_back(crc32c("Short Staves"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Short Staves"));
  skcat["Modern Skills"].push_back(crc32c("Short Staves"));
  skcat["Shadowrun Skills"].push_back(crc32c("Short Staves"));
  skcat["Skills"].push_back(crc32c("Short Staves"));
  add_wts(crc32c("Short Staves"));

  // Skill Definition: Shotguns
  defaults[crc32c("Shotguns")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Shotguns"));
  skcat["Rifle Skills"].push_back(crc32c("Shotguns"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Shotguns"));
  skcat["Modern Skills"].push_back(crc32c("Shotguns"));
  skcat["Shadowrun Skills"].push_back(crc32c("Shotguns"));
  skcat["Skills"].push_back(crc32c("Shotguns"));
  add_wts(crc32c("Shotguns"));

  // Skill Definition: Shot Pistols
  defaults[crc32c("Shot Pistols")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Shot Pistols"));
  skcat["Pistol Skills"].push_back(crc32c("Shot Pistols"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Shot Pistols"));
  skcat["Modern Skills"].push_back(crc32c("Shot Pistols"));
  skcat["Shadowrun Skills"].push_back(crc32c("Shot Pistols"));
  skcat["Skills"].push_back(crc32c("Shot Pistols"));
  add_wts(crc32c("Shot Pistols"));

  // Skill Definition: Skiing
  defaults[crc32c("Skiing")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Skiing"));
  skcat["Athletic Skills"].push_back(crc32c("Skiing"));
  skcat["Medieval Skills"].push_back(crc32c("Skiing"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Skiing"));
  skcat["Modern Skills"].push_back(crc32c("Skiing"));
  skcat["Shadowrun Skills"].push_back(crc32c("Skiing"));
  skcat["Skills"].push_back(crc32c("Skiing"));

  // Skill Definition: Sled
  defaults[crc32c("Sled")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Sled"));
  skcat["Piloting Skills"].push_back(crc32c("Sled"));
  skcat["Medieval Skills"].push_back(crc32c("Sled"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Sled"));
  skcat["Modern Skills"].push_back(crc32c("Sled"));
  skcat["Shadowrun Skills"].push_back(crc32c("Sled"));
  skcat["Skills"].push_back(crc32c("Sled"));

  // Skill Definition: Slings
  defaults[crc32c("Slings")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Slings"));
  skcat["Ranged-Combat Skills"].push_back(crc32c("Slings"));
  skcat["Medieval Skills"].push_back(crc32c("Slings"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Slings"));
  skcat["Modern Skills"].push_back(crc32c("Slings"));
  skcat["Shadowrun Skills"].push_back(crc32c("Slings"));
  skcat["Skills"].push_back(crc32c("Slings"));
  add_wts(crc32c("Slings"));

  // Skill Definition: Smelting
  defaults[crc32c("Smelting")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Smelting"));
  skcat["Creation Skills"].push_back(crc32c("Smelting"));
  skcat["Medieval Skills"].push_back(crc32c("Smelting"));
  skcat["Skills"].push_back(crc32c("Smelting"));

  // Skill Definition: SMGs
  defaults[crc32c("SMGs")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("SMGs"));
  skcat["Pistol Skills"].push_back(crc32c("SMGs"));
  skcat["Cyberpunk Skills"].push_back(crc32c("SMGs"));
  skcat["Modern Skills"].push_back(crc32c("SMGs"));
  skcat["Shadowrun Skills"].push_back(crc32c("SMGs"));
  skcat["Skills"].push_back(crc32c("SMGs"));
  add_wts(crc32c("SMGs"));

  // Skill Definition: Snowmobile
  defaults[crc32c("Snowmobile")] = 6;
  skcat["Reaction-Based Skills"].push_back(crc32c("Snowmobile"));
  skcat["Piloting Skills"].push_back(crc32c("Snowmobile"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Snowmobile"));
  skcat["Modern Skills"].push_back(crc32c("Snowmobile"));
  skcat["Shadowrun Skills"].push_back(crc32c("Snowmobile"));
  skcat["Skills"].push_back(crc32c("Snowmobile"));

  // Skill Definition: Spacecraft
  defaults[crc32c("Spacecraft")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Spacecraft"));
  skcat["Piloting Skills"].push_back(crc32c("Spacecraft"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Spacecraft"));
  skcat["Modern Skills"].push_back(crc32c("Spacecraft"));
  skcat["Shadowrun Skills"].push_back(crc32c("Spacecraft"));
  skcat["Skills"].push_back(crc32c("Spacecraft"));

  // Skill Definition: Spellcasting
  defaults[crc32c("Spellcasting")] = 3;
  skcat["Charisma-Based Skills"].push_back(crc32c("Spellcasting"));
  skcat["Magical Skills"].push_back(crc32c("Spellcasting"));
  skcat["Medieval Skills"].push_back(crc32c("Spellcasting"));
  skcat["Shadowrun Skills"].push_back(crc32c("Spellcasting"));
  skcat["Skills"].push_back(crc32c("Spellcasting"));

  // Skill Definition: Spellcraft
  defaults[crc32c("Spellcraft")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Spellcraft"));
  skcat["Magical Skills"].push_back(crc32c("Spellcraft"));
  skcat["Medieval Skills"].push_back(crc32c("Spellcraft"));
  skcat["Shadowrun Skills"].push_back(crc32c("Spellcraft"));
  skcat["Skills"].push_back(crc32c("Spellcraft"));

  // Skill Definition: Spellcraft, Star
  defaults[crc32c("Spellcraft, Star")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Spellcraft, Star"));
  skcat["Star Magical Skills"].push_back(crc32c("Spellcraft, Star"));
  skcat["Medieval Skills"].push_back(crc32c("Spellcraft, Star"));
  skcat["Skills"].push_back(crc32c("Spellcraft, Star"));

  // Skill Definition: Spell Preparation
  defaults[crc32c("Spell Preparation")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Spell Preparation"));
  skcat["Star Magical Skills"].push_back(crc32c("Spell Preparation"));
  skcat["Medieval Skills"].push_back(crc32c("Spell Preparation"));
  skcat["Skills"].push_back(crc32c("Spell Preparation"));

  // Skill Definition: Spell Targeting
  defaults[crc32c("Spell Targeting")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Spell Targeting"));
  skcat["Magical Skills"].push_back(crc32c("Spell Targeting"));
  skcat["Medieval Skills"].push_back(crc32c("Spell Targeting"));
  skcat["Shadowrun Skills"].push_back(crc32c("Spell Targeting"));
  skcat["Skills"].push_back(crc32c("Spell Targeting"));

  // Skill Definition: Sprinting
  defaults[crc32c("Sprinting")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Sprinting"));
  skcat["Athletic Skills"].push_back(crc32c("Sprinting"));
  skcat["Medieval Skills"].push_back(crc32c("Sprinting"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Sprinting"));
  skcat["Modern Skills"].push_back(crc32c("Sprinting"));
  skcat["Shadowrun Skills"].push_back(crc32c("Sprinting"));
  skcat["Skills"].push_back(crc32c("Sprinting"));

  // Skill Definition: Staff Slings
  defaults[crc32c("Staff Slings")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Staff Slings"));
  skcat["Ranged-Combat Skills"].push_back(crc32c("Staff Slings"));
  skcat["Medieval Skills"].push_back(crc32c("Staff Slings"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Staff Slings"));
  skcat["Modern Skills"].push_back(crc32c("Staff Slings"));
  skcat["Shadowrun Skills"].push_back(crc32c("Staff Slings"));
  skcat["Skills"].push_back(crc32c("Staff Slings"));
  add_wts(crc32c("Staff Slings"));

  // Skill Definition: Staff Targeting
  defaults[crc32c("Staff Targeting")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Staff Targeting"));
  skcat["Magical Skills"].push_back(crc32c("Staff Targeting"));
  skcat["Medieval Skills"].push_back(crc32c("Staff Targeting"));
  skcat["Skills"].push_back(crc32c("Staff Targeting"));

  // Skill Definition: Stealth
  defaults[crc32c("Stealth")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Stealth"));
  skcat["Athletic Skills"].push_back(crc32c("Stealth"));
  skcat["Medieval Skills"].push_back(crc32c("Stealth"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Stealth"));
  skcat["Modern Skills"].push_back(crc32c("Stealth"));
  skcat["Shadowrun Skills"].push_back(crc32c("Stealth"));
  skcat["Skills"].push_back(crc32c("Stealth"));

  // Skill Definition: Style
  defaults[crc32c("Style")] = 3;
  skcat["Charisma-Based Skills"].push_back(crc32c("Style"));
  skcat["Social Skills"].push_back(crc32c("Style"));
  skcat["Medieval Skills"].push_back(crc32c("Style"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Style"));
  skcat["Modern Skills"].push_back(crc32c("Style"));
  skcat["Shadowrun Skills"].push_back(crc32c("Style"));
  skcat["Skills"].push_back(crc32c("Style"));

  // Skill Definition: Surgery
  defaults[crc32c("Surgery")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Surgery"));
  skcat["Technical Skills"].push_back(crc32c("Surgery"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Surgery"));
  skcat["Modern Skills"].push_back(crc32c("Surgery"));
  skcat["Shadowrun Skills"].push_back(crc32c("Surgery"));
  skcat["Skills"].push_back(crc32c("Surgery"));

  // Skill Definition: Survival
  defaults[crc32c("Survival")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Survival"));
  skcat["Nature Skills"].push_back(crc32c("Survival"));
  skcat["Medieval Skills"].push_back(crc32c("Survival"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Survival"));
  skcat["Modern Skills"].push_back(crc32c("Survival"));
  skcat["Shadowrun Skills"].push_back(crc32c("Survival"));
  skcat["Skills"].push_back(crc32c("Survival"));

  // Skill Definition: Swimming
  defaults[crc32c("Swimming")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Swimming"));
  skcat["Athletic Skills"].push_back(crc32c("Swimming"));
  skcat["Medieval Skills"].push_back(crc32c("Swimming"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Swimming"));
  skcat["Modern Skills"].push_back(crc32c("Swimming"));
  skcat["Shadowrun Skills"].push_back(crc32c("Swimming"));
  skcat["Skills"].push_back(crc32c("Swimming"));

  // Skill Definition: Swordsmithing
  defaults[crc32c("Swordsmithing")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Swordsmithing"));
  skcat["Creation Skills"].push_back(crc32c("Swordsmithing"));
  skcat["Medieval Skills"].push_back(crc32c("Swordsmithing"));
  skcat["Skills"].push_back(crc32c("Swordsmithing"));

  // Skill Definition: Tactics
  defaults[crc32c("Tactics")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Tactics"));
  skcat["Specialized Skills"].push_back(crc32c("Tactics"));
  skcat["Medieval Skills"].push_back(crc32c("Tactics"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Tactics"));
  skcat["Modern Skills"].push_back(crc32c("Tactics"));
  skcat["Shadowrun Skills"].push_back(crc32c("Tactics"));
  skcat["Skills"].push_back(crc32c("Tactics"));

  // Skill Definition: Talismongery
  defaults[crc32c("Talismongery")] = 5;
  skcat["Willpower-Based Skills"].push_back(crc32c("Talismongery"));
  skcat["Magical Skills"].push_back(crc32c("Talismongery"));
  skcat["Medieval Skills"].push_back(crc32c("Talismongery"));
  skcat["Shadowrun Skills"].push_back(crc32c("Talismongery"));
  skcat["Skills"].push_back(crc32c("Talismongery"));

  // Skill Definition: Teamster
  defaults[crc32c("Teamster")] = 3;
  skcat["Charisma-Based Skills"].push_back(crc32c("Teamster"));
  skcat["Piloting Skills"].push_back(crc32c("Teamster"));
  skcat["Medieval Skills"].push_back(crc32c("Teamster"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Teamster"));
  skcat["Modern Skills"].push_back(crc32c("Teamster"));
  skcat["Shadowrun Skills"].push_back(crc32c("Teamster"));
  skcat["Skills"].push_back(crc32c("Teamster"));

  // Skill Definition: Throwing, Aero
  defaults[crc32c("Throwing, Aero")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Throwing, Aero"));
  skcat["Ranged-Combat Skills"].push_back(crc32c("Throwing, Aero"));
  skcat["Medieval Skills"].push_back(crc32c("Throwing, Aero"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Throwing, Aero"));
  skcat["Modern Skills"].push_back(crc32c("Throwing, Aero"));
  skcat["Shadowrun Skills"].push_back(crc32c("Throwing, Aero"));
  skcat["Skills"].push_back(crc32c("Throwing, Aero"));
  add_wts(crc32c("Throwing, Aero"));

  // Skill Definition: Throwing, Non-Aero
  defaults[crc32c("Throwing, Non-Aero")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Throwing, Non-Aero"));
  skcat["Ranged-Combat Skills"].push_back(crc32c("Throwing, Non-Aero"));
  skcat["Medieval Skills"].push_back(crc32c("Throwing, Non-Aero"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Throwing, Non-Aero"));
  skcat["Modern Skills"].push_back(crc32c("Throwing, Non-Aero"));
  skcat["Shadowrun Skills"].push_back(crc32c("Throwing, Non-Aero"));
  skcat["Skills"].push_back(crc32c("Throwing, Non-Aero"));
  add_wts(crc32c("Throwing, Non-Aero"));

  // Skill Definition: Tracked Drone
  defaults[crc32c("Tracked Drone")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Tracked Drone"));
  skcat["Drone Piloting Skills"].push_back(crc32c("Tracked Drone"));
  skcat["Shadowrun Skills"].push_back(crc32c("Tracked Drone"));
  skcat["Skills"].push_back(crc32c("Tracked Drone"));

  // Skill Definition: Tracked Vehicle
  defaults[crc32c("Tracked Vehicle")] = 6;
  skcat["Reaction-Based Skills"].push_back(crc32c("Tracked Vehicle"));
  skcat["Piloting Skills"].push_back(crc32c("Tracked Vehicle"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Tracked Vehicle"));
  skcat["Modern Skills"].push_back(crc32c("Tracked Vehicle"));
  skcat["Shadowrun Skills"].push_back(crc32c("Tracked Vehicle"));
  skcat["Skills"].push_back(crc32c("Tracked Vehicle"));

  // Skill Definition: Tracking
  defaults[crc32c("Tracking")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Tracking"));
  skcat["Specialized Skills"].push_back(crc32c("Tracking"));
  skcat["Medieval Skills"].push_back(crc32c("Tracking"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Tracking"));
  skcat["Modern Skills"].push_back(crc32c("Tracking"));
  skcat["Shadowrun Skills"].push_back(crc32c("Tracking"));
  skcat["Skills"].push_back(crc32c("Tracking"));

  // Skill Definition: Tractor
  defaults[crc32c("Tractor")] = 6;
  skcat["Reaction-Based Skills"].push_back(crc32c("Tractor"));
  skcat["Piloting Skills"].push_back(crc32c("Tractor"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Tractor"));
  skcat["Modern Skills"].push_back(crc32c("Tractor"));
  skcat["Shadowrun Skills"].push_back(crc32c("Tractor"));
  skcat["Skills"].push_back(crc32c("Tractor"));

  // Skill Definition: Treatment
  defaults[crc32c("Treatment")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Treatment"));
  skcat["Technical Skills"].push_back(crc32c("Treatment"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Treatment"));
  skcat["Modern Skills"].push_back(crc32c("Treatment"));
  skcat["Shadowrun Skills"].push_back(crc32c("Treatment"));
  skcat["Skills"].push_back(crc32c("Treatment"));

  // Skill Definition: Trike
  defaults[crc32c("Trike")] = 6;
  skcat["Reaction-Based Skills"].push_back(crc32c("Trike"));
  skcat["Piloting Skills"].push_back(crc32c("Trike"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Trike"));
  skcat["Modern Skills"].push_back(crc32c("Trike"));
  skcat["Shadowrun Skills"].push_back(crc32c("Trike"));
  skcat["Skills"].push_back(crc32c("Trike"));

  // Skill Definition: Two-Handed Blades
  defaults[crc32c("Two-Handed Blades")] = 2;
  skcat["Strength-Based Skills"].push_back(crc32c("Two-Handed Blades"));
  skcat["Melee-Combat Skills"].push_back(crc32c("Two-Handed Blades"));
  skcat["Medieval Skills"].push_back(crc32c("Two-Handed Blades"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Two-Handed Blades"));
  skcat["Modern Skills"].push_back(crc32c("Two-Handed Blades"));
  skcat["Shadowrun Skills"].push_back(crc32c("Two-Handed Blades"));
  skcat["Skills"].push_back(crc32c("Two-Handed Blades"));
  add_wts(crc32c("Two-Handed Blades"));

  // Skill Definition: Two-Handed Cleaves
  defaults[crc32c("Two-Handed Cleaves")] = 2;
  skcat["Strength-Based Skills"].push_back(crc32c("Two-Handed Cleaves"));
  skcat["Melee-Combat Skills"].push_back(crc32c("Two-Handed Cleaves"));
  skcat["Medieval Skills"].push_back(crc32c("Two-Handed Cleaves"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Two-Handed Cleaves"));
  skcat["Modern Skills"].push_back(crc32c("Two-Handed Cleaves"));
  skcat["Shadowrun Skills"].push_back(crc32c("Two-Handed Cleaves"));
  skcat["Skills"].push_back(crc32c("Two-Handed Cleaves"));
  add_wts(crc32c("Two-Handed Cleaves"));

  // Skill Definition: Two-Handed Crushing
  defaults[crc32c("Two-Handed Crushing")] = 2;
  skcat["Strength-Based Skills"].push_back(crc32c("Two-Handed Crushing"));
  skcat["Melee-Combat Skills"].push_back(crc32c("Two-Handed Crushing"));
  skcat["Medieval Skills"].push_back(crc32c("Two-Handed Crushing"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Two-Handed Crushing"));
  skcat["Modern Skills"].push_back(crc32c("Two-Handed Crushing"));
  skcat["Shadowrun Skills"].push_back(crc32c("Two-Handed Crushing"));
  skcat["Skills"].push_back(crc32c("Two-Handed Crushing"));
  add_wts(crc32c("Two-Handed Crushing"));

  // Skill Definition: Two-Handed Flails
  defaults[crc32c("Two-Handed Flails")] = 2;
  skcat["Strength-Based Skills"].push_back(crc32c("Two-Handed Flails"));
  skcat["Melee-Combat Skills"].push_back(crc32c("Two-Handed Flails"));
  skcat["Medieval Skills"].push_back(crc32c("Two-Handed Flails"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Two-Handed Flails"));
  skcat["Modern Skills"].push_back(crc32c("Two-Handed Flails"));
  skcat["Shadowrun Skills"].push_back(crc32c("Two-Handed Flails"));
  skcat["Skills"].push_back(crc32c("Two-Handed Flails"));
  add_wts(crc32c("Two-Handed Flails"));

  // Skill Definition: Two-Handed Piercing
  defaults[crc32c("Two-Handed Piercing")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Two-Handed Piercing"));
  skcat["Melee-Combat Skills"].push_back(crc32c("Two-Handed Piercing"));
  skcat["Medieval Skills"].push_back(crc32c("Two-Handed Piercing"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Two-Handed Piercing"));
  skcat["Modern Skills"].push_back(crc32c("Two-Handed Piercing"));
  skcat["Shadowrun Skills"].push_back(crc32c("Two-Handed Piercing"));
  skcat["Skills"].push_back(crc32c("Two-Handed Piercing"));
  add_wts(crc32c("Two-Handed Piercing"));

  // Skill Definition: Two-Handed Staves
  defaults[crc32c("Two-Handed Staves")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Two-Handed Staves"));
  skcat["Melee-Combat Skills"].push_back(crc32c("Two-Handed Staves"));
  skcat["Medieval Skills"].push_back(crc32c("Two-Handed Staves"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Two-Handed Staves"));
  skcat["Modern Skills"].push_back(crc32c("Two-Handed Staves"));
  skcat["Shadowrun Skills"].push_back(crc32c("Two-Handed Staves"));
  skcat["Skills"].push_back(crc32c("Two-Handed Staves"));
  add_wts(crc32c("Two-Handed Staves"));

  // Skill Definition: Underwater Combat
  defaults[crc32c("Underwater Combat")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Underwater Combat"));
  skcat["Combat Environment Skills"].push_back(crc32c("Underwater Combat"));
  skcat["Medieval Skills"].push_back(crc32c("Underwater Combat"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Underwater Combat"));
  skcat["Modern Skills"].push_back(crc32c("Underwater Combat"));
  skcat["Shadowrun Skills"].push_back(crc32c("Underwater Combat"));
  skcat["Skills"].push_back(crc32c("Underwater Combat"));

  // Skill Definition: Vectored Thrust
  defaults[crc32c("Vectored Thrust")] = 6;
  skcat["Reaction-Based Skills"].push_back(crc32c("Vectored Thrust"));
  skcat["Piloting Skills"].push_back(crc32c("Vectored Thrust"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Vectored Thrust"));
  skcat["Modern Skills"].push_back(crc32c("Vectored Thrust"));
  skcat["Shadowrun Skills"].push_back(crc32c("Vectored Thrust"));
  skcat["Skills"].push_back(crc32c("Vectored Thrust"));

  // Skill Definition: Vectored Thrust Drone
  defaults[crc32c("Vectored Thrust Drone")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Vectored Thrust Drone"));
  skcat["Drone Piloting Skills"].push_back(crc32c("Vectored Thrust Drone"));
  skcat["Shadowrun Skills"].push_back(crc32c("Vectored Thrust Drone"));
  skcat["Skills"].push_back(crc32c("Vectored Thrust Drone"));

  // Skill Definition: Verbal Interface
  defaults[crc32c("Verbal Interface")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Verbal Interface"));
  skcat["Technical Skills"].push_back(crc32c("Verbal Interface"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Verbal Interface"));
  skcat["Shadowrun Skills"].push_back(crc32c("Verbal Interface"));
  skcat["Sci-Fi Skills"].push_back(crc32c("Verbal Interface"));
  skcat["Skills"].push_back(crc32c("Verbal Interface"));

  // Skill Definition: Wand Targeting
  defaults[crc32c("Wand Targeting")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Wand Targeting"));
  skcat["Magical Skills"].push_back(crc32c("Wand Targeting"));
  skcat["Medieval Skills"].push_back(crc32c("Wand Targeting"));
  skcat["Skills"].push_back(crc32c("Wand Targeting"));

  // Skill Definition: Water Weaving
  defaults[crc32c("Water Weaving")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Water Weaving"));
  skcat["Magical Skills"].push_back(crc32c("Water Weaving"));
  skcat["Medieval Skills"].push_back(crc32c("Water Weaving"));
  skcat["FP Skills"].push_back(crc32c("Water Weaving"));
  skcat["Skills"].push_back(crc32c("Water Weaving"));

  // Skill Definition: Wheeled Drone
  defaults[crc32c("Wheeled Drone")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Wheeled Drone"));
  skcat["Drone Piloting Skills"].push_back(crc32c("Wheeled Drone"));
  skcat["Shadowrun Skills"].push_back(crc32c("Wheeled Drone"));
  skcat["Skills"].push_back(crc32c("Wheeled Drone"));

  // Skill Definition: Whips
  defaults[crc32c("Whips")] = 1;
  skcat["Quickness-Based Skills"].push_back(crc32c("Whips"));
  skcat["Ranged-Combat Skills"].push_back(crc32c("Whips"));
  skcat["Medieval Skills"].push_back(crc32c("Whips"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Whips"));
  skcat["Modern Skills"].push_back(crc32c("Whips"));
  skcat["Shadowrun Skills"].push_back(crc32c("Whips"));
  skcat["Skills"].push_back(crc32c("Whips"));
  add_wts(crc32c("Whips"));

  // Skill Definition: Woodworking
  defaults[crc32c("Woodworking")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Woodworking"));
  skcat["Creation Skills"].push_back(crc32c("Woodworking"));
  skcat["Medieval Skills"].push_back(crc32c("Woodworking"));
  skcat["Skills"].push_back(crc32c("Woodworking"));

  // Skill Definition: Zero-G Combat
  defaults[crc32c("Zero-G Combat")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Zero-G Combat"));
  skcat["Combat Environment Skills"].push_back(crc32c("Zero-G Combat"));
  skcat["Medieval Skills"].push_back(crc32c("Zero-G Combat"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Zero-G Combat"));
  skcat["Modern Skills"].push_back(crc32c("Zero-G Combat"));
  skcat["Shadowrun Skills"].push_back(crc32c("Zero-G Combat"));
  skcat["Skills"].push_back(crc32c("Zero-G Combat"));

  // Skill Definition: Zero-G Ops
  defaults[crc32c("Zero-G Ops")] = 4;
  skcat["Intelligence-Based Skills"].push_back(crc32c("Zero-G Ops"));
  skcat["Specialized Skills"].push_back(crc32c("Zero-G Ops"));
  skcat["Cyberpunk Skills"].push_back(crc32c("Zero-G Ops"));
  skcat["Modern Skills"].push_back(crc32c("Zero-G Ops"));
  skcat["Shadowrun Skills"].push_back(crc32c("Zero-G Ops"));
  skcat["Skills"].push_back(crc32c("Zero-G Ops"));
}

std::string SkillName(uint32_t sktok) {
  std::string name = "Undefined";
  for (auto n : skill_names) {
    if (n.first == sktok) {
      name = n.second;
    }
  }
  return name;
}

int is_skill(uint32_t stok) {
  if (!defaults_init)
    init_defaults();
  return (defaults.count(stok) != 0);
}

uint32_t get_weapon_skill(int wtype) {
  if (!defaults_init)
    init_defaults();
  if (!weaponskills.count(wtype)) {
    fprintf(stderr, "Warning: No Skill Type %d!\n", wtype);
    return crc32c("None");
  }
  return weaponskills[wtype];
}

int get_weapon_type(std::string wskill) {
  if (!defaults_init)
    init_defaults();
  if (!weapontypes.count(crc32c(wskill))) {
    fprintf(stderr, "Warning: No Skill Named '%s'!\n", wskill.c_str());
    return 0;
  }
  return weapontypes[crc32c(wskill)];
}

uint32_t get_skill(std::string sk) {
  while (sk.length() > 0 && isspace(sk.back()))
    sk.pop_back();

  auto stok = crc32c(sk);
  if (defaults.count(stok))
    return stok;
  if (sk.length() < 2)
    return crc32c("None");

  for (auto itr : defaults) {
    std::string lc = SkillName(itr.first);
    std::transform(lc.begin(), lc.end(), lc.begin(), ::tolower);
    if (!strncmp(sk.c_str(), lc.c_str(), sk.length())) {
      return itr.first;
    }
  }
  return crc32c("None");
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

int get_linked(uint32_t stok) {
  if (defaults.count(stok))
    return defaults[stok];
  return 4; // Default to Int for knowledges
}

int get_linked(std::string sk) {
  while (sk.length() > 0 && isspace(sk.back()))
    sk.pop_back();
  return get_linked(crc32c(sk));
}

std::vector<uint32_t> get_skills(std::string cat) {
  std::vector<uint32_t> ret;

  if (!defaults_init)
    init_defaults();

  while (cat.length() > 0 && isspace(cat.back()))
    cat.pop_back();

  if (cat == "Categories") {
    for (auto ind : skcat) {
      ret.push_back(crc32c(ind.first));
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

void Object::SetSkill(uint32_t stok, int v) {
  if (v > 1000000000)
    v = 1000000000;
  else if (v < -1000000000)
    v = -1000000000;

  auto itn = hash_locate(skill_names, stok);
  if (itn == skill_names.end() || itn->first != stok) {
    fprintf(stderr, CRED "Error: bogus skill hash (x%X)\n" CNRM, stok);
    skill_names.emplace(itn, std::make_pair(stok, "Unknown"));
  }
  auto itr = hash_locate(skills, stok);
  if (itr == skills.end() || itr->first != stok) {
    if (v > 0) {
      skills.emplace(itr, std::make_pair(stok, v));
    }
  } else if (v <= 0) {
    skills.erase(itr);
  } else {
    itr->second = v;
  }
}

void Object::SetSkill(const std::string& s, int v) {
  if (v > 1000000000)
    v = 1000000000;
  else if (v < -1000000000)
    v = -1000000000;

  auto stok = crc32c(s);
  auto itn = hash_locate(skill_names, stok);
  if (itn == skill_names.end() || itn->first != stok) {
    skill_names.emplace(itn, std::make_pair(stok, s));
  }
  auto itr = hash_locate(skills, stok);
  if (itr == skills.end() || itr->first != stok) {
    if (v > 0) {
      skills.emplace(itr, std::make_pair(stok, v));
    }
  } else if (v <= 0) {
    skills.erase(itr);
  } else {
    itr->second = v;
  }
}

int Object::HasSkill(uint32_t stok) const {
  auto itr = hash_find(skills, stok);
  return (itr != skills.end());
}

int Object::Skill(uint32_t stok, int* tnum) const {
  if (stok == crc32c("Body"))
    return ModAttribute(0);
  if (stok == crc32c("Quickness"))
    return ModAttribute(1);
  if (stok == crc32c("Strength"))
    return ModAttribute(2);
  if (stok == crc32c("Charisma"))
    return ModAttribute(3);
  if (stok == crc32c("Intelligence"))
    return ModAttribute(4);
  if (stok == crc32c("Willpower"))
    return ModAttribute(5);
  if (stok == crc32c("Reaction"))
    return ModAttribute(6);

  if (!defaults_init)
    init_defaults();

  auto itr = hash_find(skills, stok);
  if (itr != skills.end())
    return itr->second;
  if (tnum) {
    (*tnum) += 4;
    return ModAttribute(defaults[stok]);
  }
  return 0;
}

int Object::Roll(uint32_t s1, const Object* p2, uint32_t s2, int bias, std::string* res) const {
  int succ = 0;

  int t1 = p2->Skill(s2) - bias;
  int t2 = Skill(s1) + bias;

  if (res)
    (*res) += "(";
  succ = Roll(s1, t1, res);
  if (s2 != crc32c("None")) {
    if (res)
      (*res) += " - ";
    succ -= p2->Roll(s2, t2, res);
  }
  if (res)
    (*res) += ")";
  return succ;
}

int Object::Roll(uint32_t s1, int targ, std::string* res) const {
  if (phys >= 10 || stun >= 10 || att[0].cur == 0 || att[1].cur == 0 || att[2].cur == 0 ||
      att[3].cur == 0 || att[4].cur == 0 || att[5].cur == 0) {
    if (res)
      (*res) += "N/A";
    return 0;
  }
  targ += WoundPenalty();
  return RollNoWounds(s1, targ, res);
}

int Object::RollNoWounds(uint32_t s1, int targ, std::string* res) const {
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
  return Roll(crc32c("Reaction"), 6 - ModAttribute(5));
}
