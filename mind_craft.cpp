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

// Replace with C++20 std::ranges, when widely available
#include <range/v3/algorithm.hpp>
namespace rng = ranges;

#include "color.hpp"
#include "log.hpp"
#include "mind.hpp"
#include "object.hpp"
#include "properties.hpp"
#include "utils.hpp"

void Mind::ContinueWorkOn(Object* project) {
  auto todo = project->Skill(prhash(u8"Incomplete")) - body->ModAttribute(5);
  if (todo > 0) {
    project->SetSkill(prhash(u8"Incomplete"), todo);
  } else {
    project->ClearSkill(prhash(u8"Incomplete"));
    if (body->HasTag(crc32c(u8"collier"))) {
      project->SetShortDesc(u8"a piece of charcoal");
      project->SetDesc(u8"a piece of high-quality charcoal");
      project->SetLongDesc(fmt::format(
          u8"Created in {} by {}, Master Collier.", project->Zone()->ShortDesc(), body->Name()));
      size_t wood = project->Skill(prhash(u8"Raw Wood"));
      wood += project->Skill(prhash(u8"Pure Wood"));
      project->SetSkill(prhash(u8"Pure Charcoal"), wood / 2);
      project->ClearSkill(prhash(u8"Pure Wood"));
      project->ClearSkill(prhash(u8"Raw Wood"));
      project->SetValue(project->Value() * 10);
    } else if (body->HasTag(crc32c(u8"miller"))) {
      project->SetShortDesc(u8"a beam of wood");
      project->SetDesc(u8"a large piece of finished lumber");
      project->SetLongDesc(fmt::format(
          u8"Created in {} by {}, Master Wood Miller.",
          project->Zone()->ShortDesc(),
          body->Name()));
      size_t wood = project->Skill(prhash(u8"Raw Wood"));
      project->SetSkill(prhash(u8"Pure Wood"), wood / 2);
      project->ClearSkill(prhash(u8"Raw Wood"));
      project->SetValue(project->Value() * 10);
    } else if (body->HasTag(crc32c(u8"woodworker"))) {
      project->SetShortDesc(u8"an axe handle");
      project->SetDesc(u8"a high-quality wooden axe handle");
      project->SetLongDesc(fmt::format(
          u8"Created in {} by {}, Master Woodworker.", project->Zone()->ShortDesc(), body->Name()));
      size_t wood = project->Skill(prhash(u8"Pure Wood"));
      project->SetSkill(prhash(u8"Made of Wood"), wood / 2);
      project->ClearSkill(prhash(u8"Pure Wood"));
      project->SetValue(project->Value() * 10);
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

void Mind::StartNewProject() {
  std::vector<Object*> materials;
  auto wares = body->Room()->PickObjects(u8"everything", LOC_INTERNAL);
  for (auto item : wares) {
    if (item->ActTarg(act_t::SPECIAL_OWNER) == body->Room()) {
      if (body->HasTag(crc32c(u8"collier"))) {
        if (item->Skill(prhash(u8"Raw Wood")) || item->Skill(prhash(u8"Pure Wood"))) {
          materials.push_back(item);
        }
      } else if (body->HasTag(crc32c(u8"miller"))) {
        if (item->Skill(prhash(u8"Raw Wood"))) {
          materials.push_back(item);
        }
      } else if (body->HasTag(crc32c(u8"woodworker"))) {
        if (item->Skill(prhash(u8"Pure Wood"))) {
          materials.push_back(item);
        }
      } else if (body->HasTag(crc32c(u8"smelter"))) {
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
    }
  }
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
  if (materials.size() > 0) {
    Object* mat = materials.front();
    if (mat->Quantity() > 1) {
      mat = mat->Split(1);
    }
    mat->SetSkill(prhash(u8"Incomplete"), 1000);
    mat->SetShortDesc(u8"a piece of work");
    mat->AddAct(act_t::SPECIAL_OWNER, body);
    body->AddAct(act_t::WORK, mat);
    body->Parent()->SendOut(
        ALL, 0, u8";s grabs {} and begins working on it.\n", u8"", body, nullptr, mat->ShortDesc());
  }
}
