#include "./compute_classed_stat_points.h"
#include <algorithm>
#include <string_view>
#include "../forms/Class.h"
#include "../forms/Race.h"

namespace dovah {
   extern classed_stat_points compute_classed_stat_points(
      const file_load_order& lo,
      const dovah::loaded_forms::Class* cls,
      const dovah::loaded_forms::Race* race,
      size_t level
   ) {
      //
      // There are some game setting values we need to fetch before we can begin.
      //
      uint32_t skill_points_base         =  15;
      uint32_t skill_points_per_level_up =   8;
      uint32_t max_points_per_skill      = 100;
      uint32_t attr_points_per_level_up  =  10;
      {
         struct desired_gmst {
            uint32_t&        dst;
            std::string_view name;
            bool             found = false;
         };
         auto desired_settings = std::array{
            desired_gmst{
               skill_points_base,
               "iAVDSkillStart"
            },
            desired_gmst{
               skill_points_per_level_up,
               "iAVDSkiiAVDSkillsLevelUpllStart"
            },
            desired_gmst{
               max_points_per_skill,
               "iAVDAutoCalcSkillMax"
            },
            desired_gmst{
               attr_points_per_level_up,
               "iAVDhmsLevelUp"
            },
         };

         bool all_found = true;
         {
            dovah::loaded_game_setting l_gmst;
            for (auto& item : desired_settings) {
               if (lo.get_loaded_setting_by_name(item.name.data(), l_gmst)) {
                  auto value = l_gmst.definition->default_value.i;
                  if (value >= 0) {
                     item.found = true;
                     item.dst = l_gmst.definition->default_value.i;
                     continue;
                  }
               }
               all_found = false;
            }
         }
         if (!all_found) {
            for (const auto& gmst_base : dovah::game_settings) {
               size_t found_count = 0;
               for (auto& item : desired_settings) {
                  if (item.found) {
                     ++found_count;
                     continue;
                  }
                  if (_strnicmp(gmst_base.name, item.name.data(), item.name.size()) != 0)
                     continue;
                  ++found_count;
                  item.found = true;
                  item.dst   = gmst_base.default_value.i;
               }
               if (found_count == desired_settings.size()) {
                  break;
               }
            }
         }
      }

      //
      // We have the game settings we need. Let's get some work done now.
      //

      classed_stat_points result;
      //
      // First: the base value of each skill and attribute.
      //
      if (skill_points_base > 0) {
         for (auto& dst : result.skill_points)
            dst += skill_points_base;
      }
      if (race) {
         for (auto& skill_opt : race->stats.skill_boosts) {
            if (!skill_opt.has_value())
               continue;
            auto& skill = skill_opt.value();
            auto  index = (size_t)skill.skill;
            if (index < result.skill_points.size())
               result.skill_points[index] += skill.boost;
         }
         result.attribute_points.h = race->stats.attribute_base.health;
         result.attribute_points.m = race->stats.attribute_base.magicka;
         result.attribute_points.s = race->stats.attribute_base.stamina;
      }
      //
      // Let's real quick handle the level.
      //
      if (level == 0)
         return result;
      //
      // We only care about points from level-ups; Level 1 gains you nothing:
      //
      --level;
      // 
      // Now compute the auto-calc portion of each skill's value.
      //
      {
         struct _item {
            skill    skill;
            uint32_t weight = 0;
         };

         size_t total_points = level * skill_points_per_level_up;
         size_t total_weight = 0;
         std::array<_item, skill_count> skills;
         for (size_t i = 0; i < skill_count; ++i) {
            skills[i].skill = (skill)i;
            if (cls) {
               auto weight = cls->skill_weights[i];
               skills[i].weight = weight;
               total_weight += weight;
            }
         }
         std::stable_sort(skills.begin(), skills.end(), [](const _item& a, const _item& b) { return a.weight < b.weight; });

         float    points_per_weight = total_points / total_weight;
         uint32_t wholes_per_weight = points_per_weight;
         
         uint32_t points_lost_to_cap        = 0;
         uint32_t points_lost_to_truncation = total_points - (wholes_per_weight * total_weight);
         uint32_t num_skills_maxed          = 0;

         // Returns points lost due to the cap.
         auto _increase_stat = [max_points_per_skill, &num_skills_maxed](classed_stat_points::value_type& dst, classed_stat_points::value_type by) -> uint32_t {
            if (!by)
               return 0;
            dst += by;
            if (dst >= max_points_per_skill) {
               ++num_skills_maxed;
               if (dst > max_points_per_skill) {
                  uint32_t lost = dst - max_points_per_skill;
                  dst = max_points_per_skill;
                  return lost;
               }
            }
            return 0;
         };

         if (cls && wholes_per_weight > 0) {
            for (auto& item : skills) {
               if (item.weight == 0)
                  continue;
               auto&    dst    = result.skill_points[(size_t)item.skill];
               uint32_t points = wholes_per_weight * item.weight;
               points_lost_to_cap += _increase_stat(dst, points);
            }
         }
         //
         // If any points were lost to skill caps, then split them evenly among all skills 
         // that aren't yet maxed out.
         //
         if (num_skills_maxed > 0 && points_lost_to_cap > 0) {
            bool failed = true;
            while (points_lost_to_cap > 0 && num_skills_maxed < skill_count) {
               for (auto& item : skills) {
                  auto& dst = result.skill_points[(size_t)item.skill];
                  if (dst >= max_points_per_skill)
                     continue;

                  uint32_t points_per = points_lost_to_cap / (skill_count - num_skills_maxed);
                  if (!points_per)
                     break;
                  points_lost_to_cap -= points_per;
                  points_lost_to_cap += _increase_stat(dst, points_per);
                  failed = false;
               }
               if (failed)
                  //
                  // Failsafe, so we don't end up in an infinite loop in the event that the 
                  // number of points to distribute doesn't divide evenly (such that the 
                  // `points_per` variable gets stuck at zero).
                  //
                  break;
            }
         }
         if (points_lost_to_cap > 0) {
            points_lost_to_truncation += points_lost_to_cap;
         }
         //
         // If any points remain, distribute them one at a time, preferring skills that come 
         // earlier in the game's hardcoded skill enum (i.e. skills with lower actor value 
         // indices). Do this until we've distributed all points or until all skills are at 
         // the maximum.
         // 
         // We still respect per-skill weightings here. This loop is meant to handle skill 
         // points that failed to divide evenly somewhere -- skill points that were lost to 
         // truncation in the first go-around, or skill points that were lost to the skill 
         // cap and then subsequently failed to be divided evenly amongst all skills (i.e. 
         // the "points-per variable being zero" case in the previous loop). As such, we 
         // can respect skill weights by decreasing the weight values as we distribute the 
         // skills: if a skill has weight 2, it gets at most 2 points; if a skill has weight 
         // 1, it gets at most 1 point; and so on.
         // 
         // Of course, we also still respect skill maximums here, so if *every* skill gets 
         // maxed out, or if a skill isn't maxed but also has weight zero, then we'll just 
         // end up losing points in the end. C'est la vie: if there's nowhere to put them, 
         // then there's nowhere to put them.
         //
         while (points_lost_to_truncation > 0) {
            bool failed = true;
            for (auto& item : skills) {
               auto& dst = result.skill_points[(size_t)item.skill];
               if (item.weight <= 0)
                  continue;
               --item.weight;
               points_lost_to_truncation -= 1;
               points_lost_to_truncation += _increase_stat(dst, 1);
               failed = false;
            }
            if (failed)
               //
               // Failsafe, so we don't end up in an infinite loop.
               //
               break;
         }
         //
         // Done with skills.
         //
      }
      //
      // Attributes next. It's the same basic algorithm as skills, except that attributes 
      // aren't capped, so there's a few steps we can strip out entirely.
      //
      {
         struct _item {
            uint32_t id;
            uint32_t weight = 0;
         };
         constexpr const size_t attribute_count = 3;

         size_t total_points = level * attr_points_per_level_up;
         size_t total_weight = 0;
         std::array<_item, attribute_count> attributes;
         for (size_t i = 0; i < attribute_count; ++i) {
            attributes[i].id = i;
            if (cls) {
               auto weight = cls->attribute_weights.list[i];
               attributes[i].weight = weight;
               total_weight += weight;
            }
         }
         std::stable_sort(attributes.begin(), attributes.end(), [](const _item& a, const _item& b) { return b.weight < a.weight; });

         float    points_per_weight = total_points / total_weight;
         uint32_t wholes_per_weight = points_per_weight;
         
         uint32_t points_lost_to_truncation = total_points - (wholes_per_weight * total_weight);
         uint32_t num_skills_maxed          = 0;

         if (cls && wholes_per_weight > 0) {
            for (auto& item : attributes) {
               if (item.weight == 0)
                  continue;
               auto&    dst    = result.attribute_points.list[item.id];
               uint32_t points = wholes_per_weight * item.weight;
               dst += points;
            }
         }
         //
         // If any points remain, distribute them one at a time. Again, this is the same 
         // basic algorithm as with skills, handling points that failed to divide in 
         // somewhere while still respecting weights.
         //
         while (points_lost_to_truncation > 0) {
            bool failed = true;
            for (auto& item : attributes) {
               auto& dst = result.attribute_points.list[item.id];
               if (item.weight <= 0)
                  continue;
               --item.weight;
               --points_lost_to_truncation;
               ++dst;
               failed = false;
               if (points_lost_to_truncation == 0)
                  //
                  // All points distributed.
                  //
                  break;
            }
            if (failed)
               //
               // Failsafe, so we don't end up in an infinite loop if all attributes have 
               // (or reach) a weight of zero.
               //
               break;
         }
         //
         // Done with attributes.
         //
      }
      //
      // Done.
      //
      return result;
   }
}