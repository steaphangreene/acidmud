#include <algorithm>
#include <string>
#include <vector>

#include "color.hpp"
#include "utils.hpp"

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
    {crc32c("Resilience"), "Resilience"},
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

void init_skill_names() {
  std::sort(skill_names.begin(), skill_names.end());
}

void confirm_skill_hash(uint32_t stok) {
  auto itn = hash_locate(skill_names, stok);
  if (itn == skill_names.end() || itn->first != stok) {
    fprintf(stderr, CRED "Error: bogus skill hash (x%X)\n" CNRM, stok);
    skill_names.emplace(itn, std::make_pair(stok, "Unknown"));
  }
}
void insert_skill_hash(uint32_t stok, const std::string& s) {
  auto itn = hash_locate(skill_names, stok);
  if (itn == skill_names.end() || itn->first != stok) {
    skill_names.emplace(itn, std::make_pair(stok, s));
  }
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
