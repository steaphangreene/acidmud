#include "tags.hpp"
#include "log.hpp"
#include "properties.hpp"
#include "utils.hpp"

static std::map<const Object*, std::map<uint32_t, ObjectTag>> npctagdefs;
static std::map<const Object*, std::map<uint32_t, ObjectTag>> weapontagdefs;
static std::map<const Object*, std::map<uint32_t, ObjectTag>> armortagdefs;
static std::map<const Object*, std::map<uint32_t, ObjectTag>> itemtagdefs;
static std::map<const Object*, std::map<uint32_t, ObjectTag>> roomtagdefs;
static std::map<const Object*, std::map<uint32_t, ObjectTag>> decortagdefs;

static std::map<uint32_t, std::u8string> tag_dictionary;
static uint32_t add_to_dictionary(const std::u8string_view& tag) {
  if (tag_dictionary.contains(crc32c(tag))) {
    if (tag_dictionary.at(crc32c(tag)) != tag) {
      logerr(
          u8"ERROR: Duplicate crc32c in tags system ('{}' and '{})!\n",
          tag_dictionary.at(crc32c(tag)),
          tag);
    }
  } else {
    tag_dictionary[crc32c(tag)] = tag;
  }
  return crc32c(tag);
}

static bool intparam(std::u8string_view& line, const std::u8string_view& lab, int& min, int& max) {
  if (process(line, lab)) {
    skipspace(line);
    min = nextnum(line);
    skipspace(line);
    if (line.length() > 0 && nextchar(line) == '-') {
      skipspace(line);
      max = nextnum(line);
      skipspace(line);
    } else {
      max = min;
    }
    return true;
  } else {
    return false;
  }
}

static bool wearparam(std::u8string_view& line, std::vector<std::vector<act_t>>& defs) {
  if (process(line, u8"wear:")) {
    defs.push_back(std::vector<act_t>());
    while (line.length() > 0) {
      std::u8string wearstr(getuntil(line, ','));
      std::transform(wearstr.begin(), wearstr.end(), wearstr.begin(), ascii_toupper);
      std::u8string_view wear = wearstr;
      if (wear.starts_with(u8"SHIE")) {
        defs.back().push_back(act_t::WEAR_SHIELD);
      } else if (wear.starts_with(u8"BACK")) {
        defs.back().push_back(act_t::WEAR_BACK);
      } else if (wear.starts_with(u8"CHES")) {
        defs.back().push_back(act_t::WEAR_CHEST);
      } else if (wear.starts_with(u8"HEAD")) {
        defs.back().push_back(act_t::WEAR_HEAD);
      } else if (wear.starts_with(u8"FACE")) {
        defs.back().push_back(act_t::WEAR_FACE);
      } else if (wear.starts_with(u8"NECK")) {
        defs.back().push_back(act_t::WEAR_NECK);
      } else if (wear.starts_with(u8"COLL")) {
        defs.back().push_back(act_t::WEAR_COLLAR);
      } else if (wear.starts_with(u8"WAIS")) {
        defs.back().push_back(act_t::WEAR_WAIST);
      } else if (
          process(wear, u8"LEFT_") || process(wear, u8"LEFT.") || process(wear, u8"LEFT ") ||
          process(wear, u8"LEFT") || process(wear, u8"L_") || process(wear, u8"L.") ||
          process(wear, u8"L ") || process(wear, u8"L")) {
        if (wear.starts_with(u8"ARM")) {
          defs.back().push_back(act_t::WEAR_LARM);
        } else if (wear.starts_with(u8"ARM")) {
          defs.back().push_back(act_t::WEAR_LARM);
        } else if (wear.starts_with(u8"FIN")) {
          defs.back().push_back(act_t::WEAR_LFINGER);
        } else if (wear.starts_with(u8"FOO")) {
          defs.back().push_back(act_t::WEAR_LFOOT);
        } else if (wear.starts_with(u8"HAN")) {
          defs.back().push_back(act_t::WEAR_LHAND);
        } else if (wear.starts_with(u8"LEG")) {
          defs.back().push_back(act_t::WEAR_LLEG);
        } else if (wear.starts_with(u8"SHO")) {
          defs.back().push_back(act_t::WEAR_LSHOULDER);
        } else if (wear.starts_with(u8"WRI")) {
          defs.back().push_back(act_t::WEAR_LWRIST);
        } else if (wear.starts_with(u8"HIP")) {
          defs.back().push_back(act_t::WEAR_LHIP);
        } else {
          loger(u8"ERROR: Unknown left wear location: '{}'\n", line);
        }
      } else if (
          process(wear, u8"RIGHT_") || process(wear, u8"RIGHT.") || process(wear, u8"RIGHT ") ||
          process(wear, u8"RIGHT") || process(wear, u8"R_") || process(wear, u8"R.") ||
          process(wear, u8"R ") || process(wear, u8"R")) {
        if (wear.starts_with(u8"ARM")) {
          defs.back().push_back(act_t::WEAR_RARM);
        } else if (wear.starts_with(u8"ARM")) {
          defs.back().push_back(act_t::WEAR_RARM);
        } else if (wear.starts_with(u8"FIN")) {
          defs.back().push_back(act_t::WEAR_RFINGER);
        } else if (wear.starts_with(u8"FOO")) {
          defs.back().push_back(act_t::WEAR_RFOOT);
        } else if (wear.starts_with(u8"HAN")) {
          defs.back().push_back(act_t::WEAR_RHAND);
        } else if (wear.starts_with(u8"LEG")) {
          defs.back().push_back(act_t::WEAR_RLEG);
        } else if (wear.starts_with(u8"SHO")) {
          defs.back().push_back(act_t::WEAR_RSHOULDER);
        } else if (wear.starts_with(u8"WRI")) {
          defs.back().push_back(act_t::WEAR_RWRIST);
        } else if (wear.starts_with(u8"HIP")) {
          defs.back().push_back(act_t::WEAR_RHIP);
        } else {
          loger(u8"ERROR: Unknown right wear location: '{}'\n", line);
        }
      } else {
        loger(u8"ERROR: Unknown wear location: '{}'\n", line);
      }
    }
    return true;
  } else {
    return false;
  }
}

static std::u8string desc_merge(std::u8string_view d1, std::u8string_view d2) {
  if (d1.length() == 0) {
    return std::u8string(d2);
  } else if (d2.length() == 0) {
    return std::u8string(d1);
  } else {
    process(d2, u8"a ");
    process(d2, u8"an ");
    process(d2, u8"A ");
    process(d2, u8"An ");
    bool add_definite = (process(d2, u8"the ") || process(d2, u8"The "));
    if (add_definite) {
      add_definite &=
          (process(d1, u8"a ") || process(d1, u8"an ") || process(d1, u8"A ") ||
           process(d1, u8"An "));
    }
    if (add_definite) {
      return fmt::format(u8"the {} {}", d1, d2);
    } else {
      return fmt::format(u8"{} {}", d1, d2);
    }
  }
}

static std::vector<ObjectTag> finalize_tags(
    int max_items,
    const std::vector<uint32_t>& tags,
    const std::map<uint32_t, ObjectTag>& tagdefs) {
  std::vector<ObjectTag> ret;

  // Merge Given Tags into Single Set of Adjectives, and List of Nouns
  std::map<tag_t, ObjectTag> defs;
  for (auto tag : tags) {
    if (tagdefs.contains(tag)) {
      auto& def = tagdefs.at(tag);
      if (def.type_ == tag_t::NOUN) {
        ret.emplace_back(def);
      } else if (defs.contains(def.type_)) {
        defs.at(def.type_) = def;
      } else {
        defs.try_emplace(def.type_, def);
      }
    } else {
      loger(u8"ERROR: Use of undefined tag: '{}'.  Skipping.\n", tag);
    }
  }

  // Merge Set of Adjectives into a Single Adjectives Definition
  std::vector<ObjectTag> adjectives;
  for (auto tag : defs) {
    if (adjectives.size() == 0) {
      adjectives.emplace_back(tag.second);
    } else {
      adjectives.back() += tag.second;
    }
  }

  // Order Nouns by Highest Priority First
  std::reverse(ret.begin(), ret.end());

  // Limit Total Number of Items, If Requested
  if (max_items > 0) {
    if (ret.size() > max_items) {
      ret.erase(ret.begin() + max_items, ret.end());
    }
  }

  // Apply List of Adjectives To Each Noun
  if (adjectives.size() > 0) {
    for (auto& noun : ret) {
      noun += adjectives.front();
    }
  }

  // Profit
  return ret;
}

void ObjectTag::operator+=(const ObjectTag& in) {
  if (type_ > in.type_) { // Preserve Adjective Order
    auto tmp = in;
    tmp += (*this);
    (*this) = tmp;
    return;
  }
  short_desc_ = desc_merge(short_desc_, in.short_desc_);
  desc_ = desc_merge(desc_, in.desc_);
  if (in.long_desc_ != u8"") {
    long_desc_ += '\n';
    long_desc_ += in.long_desc_;
  }

  props_.insert(props_.end(), in.props_.begin(), in.props_.end());

  if (genders_.size() == 0 || (in.genders_.size() > 0 && genders_.size() > in.genders_.size())) {
    genders_ = in.genders_;
  }

  for (auto i : {0, 1, 2, 3, 4, 5}) {
    min_.v[i] += in.min_.v[i];
    max_.v[i] += in.max_.v[i];
  }
  min_gold_ += in.min_gold_;
  max_gold_ += in.max_gold_;

  if (type_ == tag_t::NONE) {
    tags_.clear();
  }
  if (in.type_ != tag_t::NONE) {
    tags_.insert(tags_.end(), in.tags_.begin(), in.tags_.end());
  }
  dtags_.insert(dtags_.end(), in.dtags_.begin(), in.dtags_.end());
  ntags_.insert(ntags_.end(), in.ntags_.begin(), in.ntags_.end());
  wtags_.insert(wtags_.end(), in.wtags_.begin(), in.wtags_.end());
  atags_.insert(atags_.end(), in.atags_.begin(), in.atags_.end());
  itags_.insert(itags_.end(), in.itags_.begin(), in.itags_.end());

  type_ = std::max(type_, in.type_);

  wmin_.reach += in.wmin_.reach;
  wmax_.reach += in.wmax_.reach;
  wmin_.force += in.wmin_.force;
  wmax_.force += in.wmax_.force;
  wmin_.severity += in.wmin_.severity;
  wmax_.severity += in.wmax_.severity;

  amin_.bulk += in.amin_.bulk;
  amax_.bulk += in.amax_.bulk;
  amin_.impact += in.amin_.impact;
  amax_.impact += in.amax_.impact;
  amin_.thread += in.amin_.thread;
  amax_.thread += in.amax_.thread;
  amin_.planar += in.amin_.planar;
  amax_.planar += in.amax_.planar;

  omin_.weight += in.omin_.weight;
  omax_.weight += in.omax_.weight;
  omin_.size += in.omin_.size;
  omax_.size += in.omax_.size;
  omin_.volume += in.omin_.volume;
  omax_.volume += in.omax_.volume;
  omin_.value += in.omin_.value;
  omax_.value += in.omax_.value;

  if (wtype_ == 0) {
    wtype_ = in.wtype_;
  } else if (in.wtype_ == 0) {
  } else if (wtype_ == in.wtype_) {
  } else if (two_handed(in.wtype_)) { // Convert Long to Two-Handed
    wtype_ += get_weapon_type(u8"Two-Handed Blades") - get_weapon_type(u8"Long Blades");
  } else if (two_handed(wtype_)) { // Convert Long to Two-Handed
    wtype_ = in.wtype_ + get_weapon_type(u8"Two-Handed Blades") - get_weapon_type(u8"Long Blades");
  } else {
    // logey(u8"Warning: No idea how to combine weapon types {} and {}.\n", wtype_, in.wtype_);
    wtype_ = in.wtype_;
  }

  // FIXME: Make this sort armor correctly
  if (loc_.empty()) {
    loc_ = in.loc_;
  }
}

ObjectTag::ObjectTag(const std::u8string_view& tagdef) {
  std::u8string_view def = tagdef;
  LoadFrom(def);
}

ObjectTag::ObjectTag(std::u8string_view& tagdef) {
  LoadFrom(tagdef);
}

bool ObjectTag::LoadFrom(std::u8string_view& def) {
  skipspace(def);
  tags_.push_back(add_to_dictionary(getuntil(def, '\n')));
  skipspace(def);
  while (def.length() > 0 && !def.starts_with(u8"tag:")) {
    auto line = getuntil(def, '\n');
    skipspace(line); // Ignore indentation, blank lines, etc.
    if (line.length() == 0 || line.front() == '#') {
      // Comment or blank link - skip it.
    } else if (process(line, u8"type:")) {
      std::u8string typestr(getuntil(line, '\n'));
      std::transform(typestr.begin(), typestr.end(), typestr.begin(), ascii_toupper);
      if (typestr == u8"NONE") {
        type_ = tag_t::NONE;
      } else if (typestr == u8"OPINION") {
        type_ = tag_t::OPINION;
      } else if (typestr == u8"SIZE") {
        type_ = tag_t::SIZE;
      } else if (typestr == u8"QUALITY") {
        type_ = tag_t::QUALITY;
      } else if (typestr == u8"AGE") {
        type_ = tag_t::AGE;
      } else if (typestr == u8"SHAPE") {
        type_ = tag_t::SHAPE;
      } else if (typestr == u8"COLOR") {
        type_ = tag_t::COLOR;
      } else if (typestr == u8"ORIGIN") {
        type_ = tag_t::ORIGIN;
      } else if (typestr == u8"MATERIAL") {
        type_ = tag_t::MATERIAL;
      } else if (typestr == u8"TYPE") {
        type_ = tag_t::TYPE;
      } else if (typestr == u8"PURPOSE") {
        type_ = tag_t::PURPOSE;
      } else if (typestr == u8"NOUN") {
        type_ = tag_t::NOUN;
      } else {
        loger(u8"ERROR: bad tag type: '{}'\n", typestr);
      }
    } else if (process(line, u8"short:")) {
      short_desc_ = std::u8string(getuntil(line, '\n'));
    } else if (process(line, u8"desc:")) {
      desc_ = std::u8string(getuntil(line, '\n'));
    } else if (process(line, u8"long:")) {
      long_desc_ = std::u8string(getuntil(line, '\n'));
    } else if (process(line, u8"genders:")) {
      while (line.length() > 0) {
        char8_t gen = nextchar(line);
        if (ascii_toupper(gen) == 'N') {
          genders_.push_back(gender_t::NONE);
        } else if (ascii_toupper(gen) == 'F') {
          genders_.push_back(gender_t::FEMALE);
        } else if (ascii_toupper(gen) == 'M') {
          genders_.push_back(gender_t::MALE);
        } else if (ascii_toupper(gen) == 'O') {
          genders_.push_back(gender_t::NEITHER);
        }
      }
    } else if (intparam(line, u8"b:", min_.v[0], max_.v[0])) {
    } else if (intparam(line, u8"q:", min_.v[1], max_.v[1])) {
    } else if (intparam(line, u8"s:", min_.v[2], max_.v[2])) {
    } else if (intparam(line, u8"c:", min_.v[3], max_.v[3])) {
    } else if (intparam(line, u8"i:", min_.v[4], max_.v[4])) {
    } else if (intparam(line, u8"w:", min_.v[5], max_.v[5])) {
    } else if (intparam(line, u8"gold:", min_gold_, max_gold_)) {
    } else if (process(line, u8"includes:")) {
      tags_.push_back(add_to_dictionary(getuntil(line, '\n')));
    } else if (process(line, u8"wtag:")) {
      wtags_.push_back(add_to_dictionary(getuntil(line, '\n')));
    } else if (process(line, u8"atag:")) {
      atags_.push_back(add_to_dictionary(getuntil(line, '\n')));
    } else if (process(line, u8"itag:")) {
      itags_.push_back(add_to_dictionary(getuntil(line, '\n')));
    } else if (process(line, u8"dtag:")) {
      dtags_.push_back(add_to_dictionary(getuntil(line, '\n')));
    } else if (process(line, u8"ntag:")) {
      ntags_.push_back(add_to_dictionary(getuntil(line, '\n')));
    } else if (process(line, u8"skill:")) {
      wtype_ = get_weapon_type(getuntil(line, '\n'));
    } else if (process(line, u8"prop:")) {
      auto sname = getuntil(line, ':');
      int32_t sval = getnum(line);
      props_.push_back({crc32c(sname), sval});
    } else if (wearparam(line, loc_)) {
    } else if (intparam(line, u8"weight:", omin_.weight, omax_.weight)) {
    } else if (intparam(line, u8"size:", omin_.size, omax_.size)) {
    } else if (intparam(line, u8"volume:", omin_.volume, omax_.volume)) {
    } else if (intparam(line, u8"value:", omin_.value, omax_.value)) {
    } else if (intparam(line, u8"reach:", wmin_.reach, wmax_.reach)) {
    } else if (intparam(line, u8"force:", wmin_.force, wmax_.force)) {
    } else if (intparam(line, u8"severity:", wmin_.severity, wmax_.severity)) {
    } else if (intparam(line, u8"bulk:", amin_.bulk, amax_.bulk)) {
    } else if (intparam(line, u8"impact:", amin_.impact, amax_.impact)) {
    } else if (intparam(line, u8"thread:", amin_.thread, amax_.thread)) {
    } else if (intparam(line, u8"planar:", amin_.planar, amax_.planar)) {
    } else {
      loger(u8"ERROR: bad item tag file entry: '{}'\n", line);
      return false;
    }
  }
  if (type_ == tag_t::NONE) {
    tags_.clear();
  }
  return true;
}

bool Object::LoadTags() {
  if (npctagdefs.contains(this)) {
    // Already Loaded
    return true;
  }

  // Make sure these all exist, even if they remain empty.
  npctagdefs[this];
  weapontagdefs[this];
  armortagdefs[this];
  itemtagdefs[this];
  roomtagdefs[this];
  decortagdefs[this];

  auto datasets = PickObjects(u8"all world data: defined tags", LOC_INTERNAL | LOC_NINJA);
  if (datasets.size() == 0) {
    logey(u8"Warning: No Saved Tags For World: '{}'!\n", ShortDesc());
    return false;
  }
  for (auto data : datasets) {
    if (!LoadTagsFrom(data->LongDesc(), false)) {
      loger(u8"ERROR: Failed Loading Saved Tags Into World: '{}'!\n", ShortDesc());
      return false;
    }
  }
  return true;
}

bool Object::LoadTagsFrom(const std::u8string_view& tagdefs, bool save) {
  // Make sure these all exist, even if they remain empty.
  npctagdefs[this];
  weapontagdefs[this];
  armortagdefs[this];
  itemtagdefs[this];
  roomtagdefs[this];
  decortagdefs[this];

  std::u8string_view defs = tagdefs;

  skipspace(defs);
  while (process(defs, u8"tag:")) {
    if (process(defs, u8"npc:")) {
      std::u8string_view tag(defs);
      tag = getuntil(tag, '\n');
      if (!npctagdefs[this].try_emplace(add_to_dictionary(tag), defs).second) {
        loger(u8"Duplicate NPC tag '{}' insertion into {} rejected.\n", tag, ShortDesc());
        return false;
      }
    } else if (process(defs, u8"weapon:")) {
      std::u8string_view tag(defs);
      tag = getuntil(tag, '\n');
      if (!weapontagdefs[this].try_emplace(add_to_dictionary(tag), defs).second) {
        loger(u8"Duplicate Weapon tag '{}' insertion into {} rejected.\n", tag, ShortDesc());
        return false;
      }
    } else if (process(defs, u8"armor:")) {
      std::u8string_view tag(defs);
      tag = getuntil(tag, '\n');
      if (!armortagdefs[this].try_emplace(add_to_dictionary(tag), defs).second) {
        loger(u8"Duplicate Armor tag '{}' insertion into {} rejected.\n", tag, ShortDesc());
        return false;
      }
    } else if (process(defs, u8"item:")) {
      std::u8string_view tag(defs);
      tag = getuntil(tag, '\n');
      if (!itemtagdefs[this].try_emplace(add_to_dictionary(tag), defs).second) {
        loger(u8"Duplicate Item tag '{}' insertion into {} rejected.\n", tag, ShortDesc());
        return false;
      }
    } else if (process(defs, u8"room:")) {
      std::u8string_view tag(defs);
      tag = getuntil(tag, '\n');
      if (!roomtagdefs[this].try_emplace(add_to_dictionary(tag), defs).second) {
        loger(u8"Duplicate Room tag '{}' insertion into {} rejected.\n", tag, ShortDesc());
        return false;
      }
    } else if (process(defs, u8"decor:")) {
      std::u8string_view tag(defs);
      tag = getuntil(tag, '\n');
      if (!decortagdefs[this].try_emplace(add_to_dictionary(tag), defs).second) {
        loger(u8"Duplicate Decor tag '{}' insertion into {} rejected.\n", tag, ShortDesc());
        return false;
      }
    } else {
      loger(u8"Bad tag type '{}'.\n", getuntil(defs, '\n'));
      return false;
    }
    skipspace(defs);
  }

  if (defs.length() > 0) {
    loger(u8"Bad content at end of .tags file: '{}'.\n", getuntil(defs, '\n'));
    return false;
  }

  if (save) {
    // Load Worked!  So, Now Safe These For The Future.
    Object* worldtags = new Object(this);
    worldtags->SetShortDesc(u8"World Data: Defined Tags");
    worldtags->SetLongDesc(tagdefs);
  }

  return true;
}

std::u8string get_tags_string(Object* world, const DArr64<1, uint64_t>& tags) {
  if (!world->LoadTags()) {
    logey(u8"Warning: Asked to inspect tags in a world with no tags defined.\n");
  }

  std::u8string ret = u8"";
  for (const auto& t : tags) {
    if (tag_dictionary.contains(t)) {
      ret += fmt::format(u8"{},", tag_dictionary.at(t));
    } else {
      ret += fmt::format(u8"{:08X},", t);
    }
  }
  if (ret.length() > 0) {
    ret.pop_back();
  }
  return ret;
}

static ObjectTag base_npc(
    u8"npc\n"
    u8"type:none\n"
    u8"genders:FM\n"
    u8"b:1-7\n"
    u8"q:1-7\n"
    u8"s:1-7\n"
    u8"c:1-7\n"
    u8"i:1-7\n"
    u8"w:1-7\n");

static ObjectTag base_room(
    u8"room\n"
    u8"type:none\n");

ObjectTag Object::BuildNPC(const std::u8string_view& tagstr) {
  if (!World()->LoadTags()) {
    loger(u8"ERROR: Asked to load NPC in a world with no tags defined.\n");
  }

  // Merge Given NPC Tags into new NPC Def
  std::vector<uint32_t> tags;
  for (const auto& rtag : completed) {
    if (roomtagdefs.at(World()).contains(rtag)) {
      for (const auto& ntag : roomtagdefs.at(World()).at(rtag).ntags_) {
        if (npctagdefs.at(World()).contains(ntag)) {
          tags.push_back(ntag);
        } else {
          loger(
              u8"ERROR: Use of undefined NPC tag: '{}' from room tag '{}'.  Skipping.\n",
              tag_dictionary[ntag],
              tag_dictionary[rtag]);
        }
      }
    } else {
      loger(
          u8"ERROR: Use of undefined Room tag: '{}' in room '{}'.  Skipping.\n",
          tag_dictionary[rtag],
          ShortDesc());
    }
  }
  auto start = tagstr.cbegin();
  auto end = std::find(start, tagstr.cend(), ',');
  while (start != tagstr.cend()) {
    std::u8string ntag(tagstr.substr(start - tagstr.cbegin(), end - start));
    if (npctagdefs.at(World()).contains(crc32c(ntag))) {
      tags.push_back(crc32c(ntag));
    } else {
      loger(u8"ERROR: Use of undefined NPC tag: '{}'.  Skipping.\n", ntag);
    }
    start = end;
    if (start != tagstr.cend()) {
      ++start;
      end = std::find(start, tagstr.cend(), ',');
    }
  }

  ObjectTag npcdef = base_npc;
  auto npcdefs = finalize_tags(1, tags, npctagdefs.at(World()));
  if (npcdefs.size() > 0) {
    npcdef += npcdefs.front();
  }
  return npcdef;
}

Object* Object::MakeNPC(std::mt19937& gen, const ObjectTag& npcdef_in) {
  ObjectTag npcdef = npcdef_in;

  npcdef.weapons_ = finalize_tags(1, npcdef.wtags_, weapontagdefs.at(World()));
  npcdef.armor_ = finalize_tags(0, npcdef.atags_, armortagdefs.at(World()));
  npcdef.items_ = finalize_tags(0, npcdef.itags_, itemtagdefs.at(World()));

  Object* npc = new Object(this);
  for (auto t : npcdef.tags_) {
    npc->AddTag(t);
  }
  npc->GenerateNPC(npcdef, gen);
  return npc;
}

Object* Object::AddNPC(std::mt19937& gen, const std::u8string_view& tagstr) {
  return MakeNPC(gen, BuildNPC(tagstr));
}

ObjectTag Object::BuildRoom(const std::u8string_view& tagstr) {
  if (!World()->LoadTags()) {
    loger(u8"ERROR: Asked to load Room in a world with no tags defined.\n");
  }

  std::vector<uint32_t> tags;
  auto start = tagstr.cbegin();
  auto end = std::find(start, tagstr.cend(), ',');
  while (start != tagstr.cend()) {
    std::u8string rtag(tagstr.substr(start - tagstr.cbegin(), end - start));
    if (roomtagdefs.at(World()).contains(crc32c(rtag))) {
      tags.push_back(crc32c(rtag));
    } else {
      loger(u8"ERROR: Use of undefined Room tag: '{}'.  Skipping.\n", rtag);
    }
    start = end;
    if (start != tagstr.cend()) {
      ++start;
      end = std::find(start, tagstr.cend(), ',');
    }
  }

  ObjectTag roomdef = base_room;
  auto roomdefs = finalize_tags(1, tags, roomtagdefs.at(World()));
  if (roomdefs.size() > 0) {
    roomdef += roomdefs.front();
  }
  return roomdef;
}

Object* Object::MakeRoom(std::mt19937& gen, const ObjectTag& roomdef_in) {
  ObjectTag roomdef = roomdef_in;

  roomdef.decors_ = finalize_tags(1, roomdef.dtags_, decortagdefs.at(World()));

  Object* room = new Object(this);
  for (auto t : roomdef.tags_) {
    room->AddTag(t);
  }
  room->GenerateRoom(roomdef, gen);
  return room;
}

Object* Object::AddRoom(std::mt19937& gen, const std::u8string_view& tagstr) {
  return MakeRoom(gen, BuildRoom(tagstr));
}

void Object::SetTags(const std::u8string_view& tags_in) {
  std::u8string_view tags = tags_in;
  completed.clear();
  std::u8string_view tag = getuntil(tags, ',');
  while (tag.length() > 0 || tags.length() > 0) {
    if (tag.length() > 0) {
      if (!tag_dictionary.contains(crc32c(tag))) {
        loger(u8"ERROR: Applied an undefined tag: '{}'\n", tag);
      }
      completed.push_back(crc32c(tag));
    }
    tag = getuntil(tags, ',');
  }
}

void Object::AddTag(uint64_t tag) {
  if (HasTag(tag)) {
    return;
  }
  completed.push_back(tag);
}

bool Object::HasTag(uint64_t tag) const {
  for (auto comp : completed) {
    if (comp == tag) {
      return true;
    }
  }
  return false;
}

static int rint1(auto& gen, int min, int max) { // Flat
  return std::uniform_int_distribution<int>(min, max)(gen);
}

static int rint3(auto& gen, int min, int max) { // Bell-Curve-Ish
  auto rando = std::uniform_int_distribution<int>(min, max);
  return (rando(gen) + rando(gen) + rando(gen) + 2) / 3;
}

void Object::GenerateRoom(const ObjectTag& type, std::mt19937& gen) {
  for (auto sk : type.props_) {
    SetSkill(sk.first, sk.second);
  }

  SetShortDesc(type.short_desc_);
  SetDesc(type.desc_);
  SetLongDesc(type.long_desc_);

  for (auto dc : type.decors_) {
    Object* obj = new Object(this);
    for (auto sk : dc.props_) {
      obj->SetSkill(sk.first, sk.second);
    }
    obj->SetShortDesc(dc.short_desc_);
    obj->SetDesc(dc.desc_);
    obj->SetLongDesc(dc.long_desc_);
    obj->SetWeight(rint3(gen, dc.omin_.weight, dc.omax_.weight));
    obj->SetSize(rint3(gen, dc.omin_.size, dc.omax_.size));
    obj->SetVolume(rint3(gen, dc.omin_.volume, dc.omax_.volume));
    obj->SetValue(rint3(gen, dc.omin_.value, dc.omax_.value));

    for (auto t : dc.tags_) {
      obj->AddTag(t);
    }

    if (obj->HasTag(crc32c(u8"fountain"))) { // These Need Water!
      Object* water = new Object(obj);
      water->SetShortDesc(u8"water");
      water->SetDesc(u8"clean, drinkable, water");
      water->SetSkill(prhash(u8"Liquid"), 1);
      water->SetSkill(prhash(u8"Ingestible"), 1);
      water->SetSkill(prhash(u8"Drink"), 10000);
      water->SetSkill(prhash(u8"Quantity"), std::max(10, obj->Skill(prhash(u8"Liquid Container"))));
      obj->Activate();
    }
  }
}
