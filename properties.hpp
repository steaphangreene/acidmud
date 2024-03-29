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

#ifndef PROPERTIES_HPP
#define PROPERTIES_HPP

#include <algorithm>
#include <array>
#include <cassert>
#include <string>

#include "utils.hpp"

constexpr auto prop_names = std::to_array({
    u8"Accomplishment",
    u8"Accuracy Bonus",
    u8"Accuracy Penalty",
    u8"Acrobatics",
    u8"Acting",
    u8"Activate Juju",
    u8"Air Pistols",
    u8"Air Rifles",
    u8"Air Weaving",
    u8"Alchemy",
    u8"Alcohol",
    u8"Amphibious Combat",
    u8"Archery",
    u8"Arctic Waraxe Martial Art",
    u8"ArmorB",
    u8"ArmorI",
    u8"Armoring",
    u8"ArmorP",
    u8"ArmorT",
    u8"Art",
    u8"Artificing",
    u8"Artificing, Star",
    u8"Assault Cannons",
    u8"Assault Rifles",
    u8"Attribute Points",
    u8"Ballista",
    u8"Ballistic",
    u8"Bang",
    u8"Bike",
    u8"Blacksmithing",
    u8"Blessed",
    u8"Blind Spell",
    u8"Blowgun",
    u8"Boat, Powered",
    u8"Boat, Row",
    u8"Boat, Sail",
    u8"Body",
    u8"Body Bonus",
    u8"Body Checking",
    u8"Body Penalty",
    u8"Bored",
    u8"Bowyer",
    u8"Breath Weapon",
    u8"Brightness",
    u8"Buggy",
    u8"Bulk",
    u8"Burn",
    u8"Buy All",
    u8"Buy Armor",
    u8"Buy Boat",
    u8"Buy Container",
    u8"Buy Food",
    u8"Buy Liquid Container",
    u8"Buy Potion",
    u8"Buy Scroll",
    u8"Buy Staff",
    u8"Buy Wand",
    u8"Buy Weapon",
    u8"Buy Worn",
    u8"Cannon",
    u8"Capacity",
    u8"Car",
    u8"Carpentry",
    u8"Carromeleg - Tier I",
    u8"Carromeleg - Tier II",
    u8"Carromeleg - Tier III",
    u8"Carromeleg - Tier IV",
    u8"Carromeleg - Tier V",
    u8"Carromeleg - Tier VI",
    u8"Cart",
    u8"Catching",
    u8"Centering",
    u8"Charisma",
    u8"Charisma Bonus",
    u8"Charisma Penalty",
    u8"Chill",
    u8"Clear Weather Spell",
    u8"Climbing",
    u8"Closeable",
    u8"Clothier",
    u8"Communications",
    u8"Computer",
    u8"Computer Build/Repair",
    u8"Computer Targeting",
    u8"Concuss",
    u8"Conjuring",
    u8"Container",
    u8"Copy Book Spell",
    u8"Coverage",
    u8"Create Food",
    u8"Create Food Spell",
    u8"Create Water Spell",
    u8"Create Zombie Spell",
    u8"Crossbow",
    u8"Cure Blindness Spell",
    u8"Cure Poison",
    u8"Cure Poison Spell",
    u8"Cursed",
    u8"Cyber Data Manipulation",
    u8"Cyber Vehicle Operation",
    u8"Cycling",
    u8"Damage Bonus",
    u8"Damage Control",
    u8"Damage Penalty",
    u8"Damage Resistance",
    u8"Darkness Spell",
    u8"Dark Vision",
    u8"Dark Vision Spell",
    u8"Day Length",
    u8"Day Time",
    u8"Day Worker",
    u8"Defense Range",
    u8"Def Range",
    u8"Dehydrate Effect",
    u8"Demolitions",
    u8"Detect Cursed Items Spell",
    u8"Detect Poison Spell",
    u8"Diplomacy",
    u8"Dishonor",
    u8"Distance",
    u8"Distract Spell",
    u8"Diving",
    u8"Dodge",
    u8"Drink",
    u8"Durability",
    u8"DynamicInit",
    u8"DynamicMojo",
    u8"DynamicPhase",
    u8"Earthquake Spell",
    u8"Earth Weaving",
    u8"Electrical Build/Repair",
    u8"Electronics",
    u8"Enchanting",
    u8"Encumbrance Bonus",
    u8"Encumbrance Penalty",
    u8"Endurance",
    u8"Energize Spell",
    u8"Engineering",
    u8"Enterable",
    u8"Etiquette",
    u8"Evasion",
    u8"Evasion Bonus",
    u8"Evasion Penalty",
    u8"Faith",
    u8"Faith Remaining",
    u8"Finance",
    u8"Find Juju",
    u8"Fireball Spell",
    u8"Fire Burst Spell",
    u8"Fire Dart Spell",
    u8"Fire Weaving",
    u8"First Aid",
    u8"Fixed-Wing Craft",
    u8"Fixed-Wing Drone",
    u8"Flash",
    u8"Fletcher",
    u8"Flexibility",
    u8"Flight",
    u8"Float Spell",
    u8"Fly Spell",
    u8"Focusing",
    u8"Food",
    u8"Force Arrow Spell",
    u8"Force Sword",
    u8"Force Sword Spell",
    u8"Fun",
    u8"Grappling",
    u8"Group Resilience Spell",
    u8"Gunnery",
    u8"Half-Track",
    u8"Hardness",
    u8"Harm Spell",
    u8"Hazardous",
    u8"Heal Effect",
    u8"Heal Group Spell",
    u8"Healing",
    u8"Heal Spell",
    u8"Heat Vision",
    u8"Heat Vision Spell",
    u8"Heavy Lasers",
    u8"Heavy Projectors",
    u8"Heavy Rifles",
    u8"Helmsman, Star",
    u8"Hidden",
    u8"High-G Combat",
    u8"High-G Ops",
    u8"Hits",
    u8"Hit Weight",
    u8"Honor",
    u8"Hovercraft",
    u8"Hover Drone",
    u8"Hungry",
    u8"Hurling",
    u8"Identify",
    u8"Identify Character Spell",
    u8"Identify Juju",
    u8"Identify Person Spell",
    u8"Identify Spell",
    u8"Incomplete",
    u8"Influence Spell",
    u8"Ingestible",
    u8"Injure Spell",
    u8"Insulate",
    u8"Intelligence",
    u8"Intelligence Bonus",
    u8"Intelligence Penalty",
    u8"Interrogation",
    u8"Intimidation",
    u8"Invisibility Spell",
    u8"Invisible",
    u8"Irradiate",
    u8"Jumping",
    u8"Key",
    u8"Kicking",
    u8"Knowledge",
    u8"Laser Pistols",
    u8"Laser Rifles",
    u8"Lasso",
    u8"Last Object ID",
    u8"Launch Weapons",
    u8"Leadership",
    u8"Leatherworking",
    u8"Leverage",
    u8"Lifting",
    u8"Lightable",
    u8"Lightning Bolt Spell",
    u8"Light Source",
    u8"Light Spell",
    u8"Liquid",
    u8"Liquid Container",
    u8"Liquid Source",
    u8"Locate Object Spell",
    u8"Lock",
    u8"Lockable",
    u8"Locked",
    u8"Long Blades",
    u8"Long Cleaves",
    u8"Long Crushing",
    u8"Long Flails",
    u8"Long Piercing",
    u8"Long Staves",
    u8"Lost Martial Arts",
    u8"Low-G Combat",
    u8"Low-G Ops",
    u8"Luck Spell",
    u8"Lumberjack",
    u8"Machine Guns",
    u8"Machine Pistols",
    u8"Made of Leather",
    u8"Made of Copper",
    u8"Made of Gold",
    u8"Made of Iron",
    u8"Made of Platinum",
    u8"Made of Silver",
    u8"Made of Steel",
    u8"Made of Tin",
    u8"Made of Wood",
    u8"Magical",
    u8"Magical Charges",
    u8"Magical Scroll",
    u8"Magical Spell",
    u8"Magical Staff",
    u8"Magical Wand",
    u8"Magic Dead",
    u8"Magic Force Bonus",
    u8"Magic Force Penalty",
    u8"Magic Manipulation",
    u8"Magic Resistance",
    u8"Magic Vulnerability",
    u8"Martial Arts, Elven",
    u8"Masonry",
    u8"Mature Trees",
    u8"MaxBody",
    u8"MaxCharisma",
    u8"Max Gap",
    u8"MaxIntelligence",
    u8"MaxQuickness",
    u8"MaxStrength",
    u8"MaxWillpower",
    u8"Mechanical Build/Repair",
    u8"Mechanics",
    u8"Metalworking",
    u8"Mindcasting",
    u8"Min Gap",
    u8"Misfortune Spell",
    u8"Money",
    u8"Mounted Air Pistols",
    u8"Mounted Archery",
    u8"Mounted Blades",
    u8"Mounted Blowgun",
    u8"Mounted Cleaves",
    u8"Mounted Crossbow",
    u8"Mounted Crushing",
    u8"Mounted Flails",
    u8"Mounted Hurling",
    u8"Mounted Laser Pistols",
    u8"Mounted Machine Pistols",
    u8"Mounted Nets",
    u8"Mounted Piercing",
    u8"Mounted Pistols",
    u8"Mounted Plasma Pistols",
    u8"Mounted Shot Pistols",
    u8"Mounted Slings",
    u8"Mounted SMGs",
    u8"Mounted Throwing, Aero",
    u8"Mounted Throwing, Non-Aero",
    u8"Mounted Whips",
    u8"Muffle",
    u8"Multiple",
    u8"NaturalWeapon",
    u8"Navigation",
    u8"Needy",
    u8"Negotiation",
    u8"Nets",
    u8"Neural Interface",
    u8"Night Worker",
    u8"Noise Source",
    u8"None",
    u8"Object ID",
    u8"Obvious",
    u8"Offhand Air Pistols",
    u8"Offhand Blades",
    u8"Offhand Cleaves",
    u8"Offhand Crossbow",
    u8"Offhand Crushing",
    u8"Offhand Flails",
    u8"Offhand Hurling",
    u8"Offhand Laser Pistols",
    u8"Offhand Machine Pistols",
    u8"Offhand Piercing",
    u8"Offhand Pistols",
    u8"Offhand Plasma Pistols",
    u8"Offhand Punching",
    u8"Offhand Shot Pistols",
    u8"Offhand SMGs",
    u8"Offhand Staves",
    u8"Offhand Throwing, Aero",
    u8"Offhand Throwing, Non-Aero",
    u8"Offhand Whips",
    u8"Open",
    u8"Padding",
    u8"Peaceful",
    u8"Pen. Dist",
    u8"Pen. Ratio",
    u8"Pen. Width",
    u8"Perception",
    u8"Performance",
    u8"Perishable",
    u8"Personality",
    u8"Personal Shield Spell",
    u8"Pickable",
    u8"Pistols",
    u8"Plasma Cannons",
    u8"Plasma Pistols",
    u8"Plasma Rifles",
    u8"Poisionous",
    u8"Poisoned",
    u8"Poisonous",
    u8"Power Distribution",
    u8"Power Suit",
    u8"Prepared Basic",
    u8"Prepared Intermediate",
    u8"Prepared Primary",
    u8"Prepared Simple",
    u8"Priceless",
    u8"Protection Spell",
    u8"Punching",
    u8"Pure Bronze",
    u8"Pure Charcoal",
    u8"Pure Copper",
    u8"Pure Gold",
    u8"Pure Iron",
    u8"Pure Platinum",
    u8"Pure Silver",
    u8"Pure Steel",
    u8"Pure Tin",
    u8"Pure Wood",
    u8"Quantity",
    u8"Quickdraw",
    u8"Quickness",
    u8"Quickness Bonus",
    u8"Quickness Penalty",
    u8"Race Car",
    u8"Range",
    u8"Ratio",
    u8"Raw Copper",
    u8"Raw Gold",
    u8"Raw Iron",
    u8"Raw Platinum",
    u8"Raw Silver",
    u8"Raw Steel",
    u8"Raw Tin",
    u8"Raw Wood",
    u8"Reach",
    u8"Reaction",
    u8"Recall",
    u8"Recall Group Spell",
    u8"Recall Spell",
    u8"Refine Juju",
    u8"Reflect",
    u8"Remove Curse",
    u8"Remove Curse Spell",
    u8"Research",
    u8"Resilience",
    u8"Resilience Bonus",
    u8"Resilience Penalty",
    u8"Resilience Spell",
    u8"Rest",
    u8"Restricted Item",
    u8"Resurrect",
    u8"Resurrect Spell",
    u8"Riding",
    u8"Rifles",
    u8"Ritual Conjuring",
    u8"Ritual Spellcasting",
    u8"Rod Targeting",
    u8"Rot",
    u8"Rotorcraft",
    u8"Rotor Drone",
    u8"Running",
    u8"Saurian Line Attacking",
    u8"Security",
    u8"Seduction",
    u8"Self Artificing",
    u8"Sell Profit",
    u8"Sensors",
    u8"Shade",
    u8"Sharpness",
    u8"Shielding",
    u8"Shields",
    u8"Ship",
    u8"Ship, Powered",
    u8"Shipwright",
    u8"Short Blades",
    u8"Short Cleaves",
    u8"Short Crushing",
    u8"Short Flails",
    u8"Short Piercing",
    u8"Short Staves",
    u8"Shotguns",
    u8"Shot Pistols",
    u8"Skiing",
    u8"Skill Points",
    u8"Skills",
    u8"Sled",
    u8"Sleep Other",
    u8"Sleep Other Spell",
    u8"Sleepy",
    u8"Slings",
    u8"Smelting",
    u8"SMGs",
    u8"Snowmobile",
    u8"Soundproof",
    u8"Spacecraft",
    u8"Spark Spell",
    u8"Spellcasting",
    u8"Spellcraft",
    u8"Spellcraft, Star",
    u8"Spell Preparation",
    u8"Spell Targeting",
    u8"Sprinting",
    u8"Staff Slings",
    u8"Staff Targeting",
    u8"Star Martial Arts",
    u8"Status Points",
    u8"Stealth",
    u8"Strength",
    u8"Strength Bonus",
    u8"Strength Penalty",
    u8"Strength Required",
    u8"Strength Spell",
    u8"Str Req",
    u8"Stuff",
    u8"Style",
    u8"Summon Creature Spell",
    u8"Surgery",
    u8"Survival",
    u8"Swimming",
    u8"Swordsmithing",
    u8"Tactics",
    u8"Talismongery",
    u8"TBAAction",
    u8"TBAAffection",
    u8"TBAAttack",
    u8"TBADefense",
    u8"TBAGold",
    u8"TBAMOB",
    u8"TBAObject",
    u8"TBAPopper",
    u8"TBARoom",
    u8"TBAScript",
    u8"TBAScriptNArg",
    u8"TBAScriptType",
    u8"TBAZone",
    u8"Teamster",
    u8"Teleport",
    u8"Teleport Spell",
    u8"Temporary",
    u8"Thickness",
    u8"Thirsty",
    u8"Throwing, Aero",
    u8"Throwing, Non-Aero",
    u8"Tired",
    u8"Tracked Drone",
    u8"Tracked Vehicle",
    u8"Tracking",
    u8"Tractor",
    u8"Translate Spell",
    u8"Translucent",
    u8"Transparent",
    u8"Treatment",
    u8"Treatment Spell",
    u8"Trike",
    u8"Two-Handed Blades",
    u8"Two-Handed Cleaves",
    u8"Two-Handed Crushing",
    u8"Two-Handed Flails",
    u8"Two-Handed Piercing",
    u8"Two-Handed Staves",
    u8"Underwater Combat",
    u8"Vectored Thrust",
    u8"Vectored Thrust Drone",
    u8"Vehicle",
    u8"Velocity",
    u8"Verbal Interface",
    u8"Vortex",
    u8"Wand Targeting",
    u8"Warm",
    u8"WaterDepth",
    u8"Water Weaving",
    u8"Weaken Subject Spell",
    u8"WeaponDamage",
    u8"WeaponForce",
    u8"WeaponReach",
    u8"WeaponSeverity",
    u8"WeaponSkill",
    u8"Weapon2Skill",
    u8"WeaponType",
    u8"Wearable on Back",
    u8"Wearable on Chest",
    u8"Wearable on Collar",
    u8"Wearable on Face",
    u8"Wearable on Head",
    u8"Wearable on Left Arm",
    u8"Wearable on Left Finger",
    u8"Wearable on Left Foot",
    u8"Wearable on Left Hand",
    u8"Wearable on Left Hip",
    u8"Wearable on Left Leg",
    u8"Wearable on Left Shoulder",
    u8"Wearable on Left Wrist",
    u8"Wearable on Neck",
    u8"Wearable on Right Arm",
    u8"Wearable on Right Finger",
    u8"Wearable on Right Foot",
    u8"Wearable on Right Hand",
    u8"Wearable on Right Hip",
    u8"Wearable on Right Leg",
    u8"Wearable on Right Shoulder",
    u8"Wearable on Right Wrist",
    u8"Wearable on Shield",
    u8"Wearable on Waist",
    u8"Wheeled Drone",
    u8"Whips",
    u8"Width",
    u8"Willpower",
    u8"Willpower Bonus",
    u8"Willpower Penalty",
    u8"Woodworking",
    u8"Youth Bonus",
    u8"Youth Penalty",
    u8"Zap",
    u8"Zero-G Combat",
    u8"Zero-G Ops",
});
template <size_t L>
constexpr std::array<uint32_t, L> names2crcs(std::array<const char8_t*, L> in) {
  std::array<uint32_t, L> ret;
  std::transform(in.begin(), in.end(), ret.begin(), [](auto&& x) {
    return crc32c_c(x, 0xFFFFFFFFU, 0) ^ 0xFFFFFFFFU;
  });
  std::sort(ret.begin(), ret.end());
  return ret;
};
constexpr auto skill_crcs = names2crcs(prop_names);
static_assert(crc32c(prop_names[0]) == crc32c(u8"Accomplishment"));
static_assert(skill_crcs[0] == crc32c(u8"Warm"));

consteval bool same_string(const char8_t* s1, const char8_t* s2, size_t pos) {
  return (s1[pos] == '\0' && s2[pos] == '\0') ? true
      : (s1[pos] != s2[pos])                  ? false
                                              : same_string(s1, s2, pos + 1);
}

consteval bool is_in_prop_list(const char8_t* pr, size_t pos, size_t max) {
  return (pos >= max || pos >= prop_names.size()) ? false
      : (same_string(pr, prop_names[pos], 0))     ? true
                                                  : is_in_prop_list(pr, pos + 1, max);
}

consteval bool is_in_prop_list(const char8_t* pr) {
  return is_in_prop_list(pr, 0, 128) || is_in_prop_list(pr, 128, 256) ||
      is_in_prop_list(pr, 256, 384) || is_in_prop_list(pr, 384, 512) ||
      is_in_prop_list(pr, 512, 640) || is_in_prop_list(pr, 640, 768) ||
      is_in_prop_list(pr, 768, 896) || is_in_prop_list(pr, 896, 1024);
}
static_assert(prop_names.size() <= 1024); // Must add more steps to above function if this fails.

// This fails ugly if you use a string not in the list ...but at least it fails. :)
// Hopefully later C++ revs will provide better ways to cleanly assert in consteval.
consteval uint32_t prhash(const char8_t* pr) {
  uint32_t ret = crc32c(pr);
  assert(is_in_prop_list(pr));
  return ret;
}
static_assert(prhash(u8"Warm") == 0x0112DB37U);
static_assert(prhash(u8"Day Time") == 0x0BDB09F5U);
static_assert(prhash(u8"Object ID") != 0);
static_assert(prhash(u8"Last Object ID") != 0);

#endif // PROPERTIES_HPP
