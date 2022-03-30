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

#include <string>
#include <vector>

// Replace with C++20 std::ranges, when widely available
#include <range/v3/algorithm.hpp>
namespace rng = ranges;

#include "color.hpp"
#include "log.hpp"
#include "mind.hpp"
#include "object.hpp"
#include "properties.hpp"
#include "utils.hpp"

struct Project {
  std::vector<uint32_t> tags_;

  std::u8string short_;
  std::u8string desc_;
  std::u8string long_template_;
  skill_pair prop_ = {prhash(u8"None"), 0};

  uint32_t material_;
  uint32_t amount_ = 1000;
  uint32_t max_qty_ = 1;
  uint32_t weight_ = 1000;
  uint32_t volume_ = 100;
  uint32_t size_ = 100;
  uint32_t value_ = 10;
};

static std::vector<const Project> projects = {
    {{crc32c(u8"collier")},
     u8"a piece of charcoal",
     u8"a piece of high-quality charcoal",
     u8"Created in {} by {}, Master Collier.",
     {prhash(u8"Pure Charcoal"), 100},
     prhash(u8"Raw Wood"),
     200,
     100,
     1000,
     100,
     100,
     10},
    {{crc32c(u8"collier")},
     u8"a piece of charcoal",
     u8"a piece of high-quality charcoal",
     u8"Created in {} by {}, Master Collier.",
     {prhash(u8"Pure Charcoal"), 100},
     prhash(u8"Pure Wood"),
     100,
     100,
     1000,
     100,
     100,
     10},
    {{crc32c(u8"miller")},
     u8"a beam of wood",
     u8"a large piece of finished lumber",
     u8"Created in {} by {}, Master Wood Miller.",
     {prhash(u8"Pure Wood"), 100000},
     prhash(u8"Raw Wood"),
     200000,
     1,
     1000,
     100,
     100,
     10},
    {{crc32c(u8"woodworker")},
     u8"an axe handle",
     u8"a high-quality wooden axe handle",
     u8"Created in {} by {}, Master Woodworker.",
     {prhash(u8"Made of Wood"), 200},
     prhash(u8"Pure Wood"),
     1000,
     1,
     1000,
     100,
     100,
     220},
};

void Mind::StartNewProject() {
  std::vector<std::pair<const Project*, Object*>> materials;
  auto wares = body->Room()->PickObjects(u8"everything", LOC_INTERNAL);
  for (auto item : wares) {
    if (item->ActTarg(act_t::SPECIAL_OWNER) == body->Room()) {
      for (const auto& proj : projects) {
        bool relevant = true;
        for (auto tag : proj.tags_) {
          if (!body->HasTag(tag)) {
            relevant = false;
          }
        }
        if (relevant && item->Skill(proj.material_) >= proj.amount_) {
          materials.emplace_back(std::make_pair(&proj, item));
          break;
        }
      }
      /*
            if (body->HasTag(crc32c(u8"smelter"))) {
              if (item->Skill(prhash(u8"Pure Charcoal")) || item->Skill(prhash(u8"Raw Copper")) ||
                  item->Skill(prhash(u8"Raw Silver")) || item->Skill(prhash(u8"Raw Gold")) ||
                  item->Skill(prhash(u8"Raw Platinum")) || item->Skill(prhash(u8"Raw Tin"))) {
                materials.push_back(item);
              }
            } else if (body->HasTag(crc32c(u8"metalworker"))) {
              if (item->Skill(prhash(u8"Pure Charcoal")) || item->Skill(prhash(u8"Pure Iron")) ||
                  item->Skill(prhash(u8"Pure Copper")) || item->Skill(prhash(u8"Pure Tin"))) {
                materials.push_back(item);
              }
            } else if (body->HasTag(crc32c(u8"coinmaker"))) {
              if (item->Skill(prhash(u8"Pure Charcoal")) || item->Skill(prhash(u8"Pure Copper")) ||
                  item->Skill(prhash(u8"Pure Silver")) || item->Skill(prhash(u8"Pure Gold")) ||
                  item->Skill(prhash(u8"Pure Platinum"))) {
                materials.push_back(item);
              }
            }
      */
    }
  }
  /*
    rng::sort(materials, [](const Object* a, const Object* b) {
      size_t rat_a = a->Skill(prhash(u8"Raw Wood"));
      rat_a += a->Skill(prhash(u8"Pure Wood"));
      size_t rat_b = b->Skill(prhash(u8"Raw Wood"));
      rat_b += b->Skill(prhash(u8"Raw Wood"));
      rat_a *= 1000UL;
      rat_b *= 1000UL;
      rat_a /= a->Value();
      rat_b /= b->Value();
      return rat_a > rat_b;
    });
  */
  if (materials.size() > 0) {
    const Project* proj = materials.front().first;
    Object* mat = materials.front().second;
    if (mat->Quantity() > 1) {
      mat = mat->Split(1);
    }
    mat->SetSkill(prhash(u8"Incomplete"), 1000);
    mat->SetShortDesc(u8"a piece of work");
    mat->AddAct(act_t::SPECIAL_OWNER, body);
    body->AddAct(act_t::WORK, mat);

    Object* goal = new Object(mat);
    goal->SetDescs(
        proj->short_,
        u8"",
        proj->desc_,
        fmt::format(proj->long_template_, goal->Zone()->ShortDesc(), body->Name()));
    if (proj->prop_.first != prhash(u8"None")) {
      goal->SetSkill(proj->prop_.first, proj->prop_.second);
    }
    goal->SetWeight(proj->weight_);
    goal->SetSize(proj->size_);
    goal->SetVolume(proj->volume_);
    goal->SetValue(proj->value_);
    if (proj->max_qty_ > 1 && proj->amount_ > 0 &&
        mat->Skill(proj->material_) >= proj->amount_ * 2) {
      goal->SetSkill(
          u8"Quantity", std::min(proj->max_qty_, mat->Skill(proj->material_) / proj->amount_));
    }

    body->Parent()->SendOut(
        ALL, 0, u8";s grabs {} and begins working on it.\n", u8"", body, nullptr, mat->ShortDesc());
  }
}

void Mind::ContinueWorkOn(Object* project) {
  auto todo = project->Skill(prhash(u8"Incomplete")) - body->ModAttribute(5);
  if (todo > 0) {
    project->SetSkill(prhash(u8"Incomplete"), todo);
  } else {
    project->ClearSkill(prhash(u8"Incomplete"));
    if (project->Contents().size() > 0) {
      Object* old_proj = project;
      project = project->Contents().front();
      project->Travel(project->Room());
      delete old_proj;
    }
    project->AddAct(act_t::SPECIAL_OWNER, body->Room());
    body->Parent()->SendOut(
        ALL,
        0,
        u8";s compeletes {} and places it with the rest of the wares.\n",
        u8"",
        body,
        nullptr,
        project->ShortDesc());
  }
}