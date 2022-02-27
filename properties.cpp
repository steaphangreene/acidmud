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

#include <vector>

#include "color.hpp"
#include "properties.hpp"

// Generate initial skills definitions pair vector automatically from known skills list.
template <size_t L>
constexpr std::array<std::pair<uint32_t, std::string>, L> names2entry(
    std::array<const char*, L> in) {
  std::array<std::pair<uint32_t, std::string>, L> ret;
  std::transform(in.begin(), in.end(), ret.begin(), [](auto&& x) {
    return std::make_pair(crc32c_c(x, 0xFFFFFFFFU, 0) ^ 0xFFFFFFFFU, x);
  });
  std::sort(ret.begin(), ret.end());
  return ret;
};
static const auto skill_defs_array = names2entry(skill_names);
static std::vector<std::pair<uint32_t, std::string>> skill_defs(
    skill_defs_array.begin(),
    skill_defs_array.end());

void save_skill_names_to(FILE* fl) {
  std::sort(skill_defs.begin(), skill_defs.end());
  skill_defs.erase(std::unique(skill_defs.begin(), skill_defs.end()), skill_defs.end());
  fprintf(fl, "%lu\n", skill_defs.size());
  for (auto skn : skill_defs) {
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
  skill_defs.reserve(skill_defs.size() + size);
  for (int sk = 0; sk < size; ++sk) {
    uint32_t hash;
    fscanf(fl, "%X:%255[^\n]\n", &hash, buf);
    skill_defs.emplace_back(std::make_pair(hash, buf));
  }
  std::sort(skill_defs.begin(), skill_defs.end());
  skill_defs.erase(std::unique(skill_defs.begin(), skill_defs.end()), skill_defs.end());
}

void purge_invalid_skill_names() {
  for (auto& sk : skill_defs) {
    sk.first = crc32c(sk.second);
  }
  std::sort(skill_defs.begin(), skill_defs.end());
  skill_defs.erase(std::unique(skill_defs.begin(), skill_defs.end()), skill_defs.end());
}

void confirm_skill_hash(uint32_t stok) {
  auto itn = skill_defs.begin();
  for (; itn != skill_defs.end() && itn->first != stok; ++itn) {
  }
  if (itn == skill_defs.end()) {
    fprintf(stderr, CRED "Error: bogus skill hash (x%X)\n" CNRM, stok);
    skill_defs.emplace_back(std::make_pair(stok, "Unknown"));
  }
}
void insert_skill_hash(uint32_t stok, const std::string& s) {
  auto itn = skill_defs.begin();
  for (; itn != skill_defs.end() && itn->first != stok; ++itn) {
  }
  if (itn == skill_defs.end()) {
    skill_defs.emplace_back(std::make_pair(stok, s));
  }
}

std::string SkillName(uint32_t sktok) {
  std::string name = "Undefined";
  for (auto n : skill_defs) {
    if (n.first == sktok) {
      name = n.second;
    }
  }
  return name;
}
