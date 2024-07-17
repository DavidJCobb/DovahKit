#include "./leveled_list_preview.h"
#include <ctime>
#include <cstdlib>
#include "../forms/components/leveled_list.h"
#include "../forms/_component_access.h"

namespace dovah {
   bool leveled_list_preview::_check_chance_none(const _component_type& subject) const {
      uint8_t percentage = subject.chance_none.percentage;
      if (subject.chance_none.global) {
         // TODO
      }
      if (percentage > 0)
         if (this->rng() % 100 < percentage)
            return false;
      return true;
   }
   /*static*/ bool leveled_list_preview::_is_leveled_list(dovah::form_stub& stub) {
      switch (stub.form_type) {
         case dovah::form_type::leveled_character:
         case dovah::form_type::leveled_item:
         case dovah::form_type::leveled_spell:
            return true;
      }
      return false;
   }
   int32_t leveled_list_preview::_special_loot_level(int16_t input_level) const {
      std::uniform_real_distribution<> range(-0.5F, 0.5F);

      const auto& sl = this->game_settings.special_loot;

      float min = ((float)this->player_level * sl.min.player_level.mult) + sl.min.player_level.base + sl.min.zone_level.base + ((float)input_level * sl.min.zone_level.mult);
      float max = ((float)this->player_level * sl.max.player_level.mult) + sl.max.player_level.base + sl.max.zone_level.base + ((float)input_level * sl.max.zone_level.mult);

      float diff = max - min;
      return min + diff * pow(range(this->rng), sl.weighting);
   }

   leveled_list_preview::entry leveled_list_preview::_generate_single_entry(
      bool process_nested_lists,
      const _component_type& subject,
      int16_t level,
      int16_t count
   ) {
      if (!this->_check_chance_none(subject))
         return {};

      entry out;
      if (this->use_special_loot_formula) {
         level = this->_special_loot_level(level);
      }

      int min_level = level;
      int max_level = level;
      {
         bool cumulative = subject.flags & _component_type::flag::calculate_from_all_levels_below_player;
         if (
            (cumulative && (mode == selection_mode::vary_levels_only_when_cumulative || mode == selection_mode::always_vary_levels))
            ||
            (!cumulative && mode == selection_mode::always_vary_levels)
         ) {
            uint32_t level_difference_max = this->game_settings.max_level_difference;
            if (level_difference_max) {
               min_level = level - level_difference_max;
            } else {
               min_level = -1;
            }
         }
      }

      bool   has_exceeded_cap     = false;
      bool   within_desired_range = false;
      size_t eligible_range_start = -1;
      size_t eligible_range_size  = 0;
      size_t range_basis_level    = 0;
      for (size_t i = 0; i < subject.entries.size(); ++i) {
         auto& entry = subject.entries[i];
         if (entry.level > max_level) {
            if (has_exceeded_cap || mode != selection_mode::prefer_first_above_cap) {
               break;
            }
            //
            // We're using the selection mode wherein we prefer the first list item we 
            // see that exceeds our upper bound (i.e. Very Hard difficulty); and we are 
            // *just now seeing* the first list item to exceed that bound.
            //
            if (min_level == max_level) {
               //
               // If we wanted an exact level match, then clear the range of previously 
               // seen list items (so that we disregard any exact matches we actually 
               // did find), and start a new range at our current list item.
               //
               eligible_range_start = -1;
               eligible_range_size  = 0;
               range_basis_level    = 0;
            }
            max_level = entry.level;
            //
            // Oh, and make sure we don't do all this twice.
            //
            has_exceeded_cap = true;
            //
            // Then, fall through.
            //
         }

         if (
            (entry.level <= range_basis_level) // Condition A
            ||
            (range_basis_level && within_desired_range) // Condition B
         ) {
            //
            // Expand the range of eligible list items to include the current list item.
            //
            ++eligible_range_size;
         } else {
            //
            // If we haven't reached the minimum usable level yet (!B), then every time 
            // we see a level higher than any we've seen before (!A), we should clear the 
            // range of eligible list items, and start a new range.
            //
            if (entry.level >= min_level)
               within_desired_range = true;
            eligible_range_start = i;
            eligible_range_size  = 1;
            range_basis_level    = entry.level;
         }
      }

      if (eligible_range_start == -1 || eligible_range_size == 0)
         return {};

      size_t i = (rng() % eligible_range_size) + eligible_range_start;
      assert(i < subject.entries.size());
      const auto& entry = subject.entries[i];
         
      out.form  = entry.form.get_form_stub();
      out.count = entry.count;
      if (entry.item_extra_data.has_value()) {
         auto& src = entry.item_extra_data.value();
         out.extra = {
            .health = src.health,
            .owner  = src.ownership.get_owner()
         };
      }
      if (process_nested_lists && out.form) {
         if (_is_leveled_list(*out.form)) {
            auto  loaded = out.form->load();
            auto* nested = loaded_forms::component_access::get_leveled_list(loaded);
            if (nested) {
               auto inner = this->_generate_single_entry(process_nested_lists, *nested, level, count);
               out.form  = inner.form;
               out.count = (out.count * inner.count) & 0xFFFF;
               if (inner.extra.has_value())
                  out.extra = inner.extra;
            }
         }
      }
      return out;
   }
   void leveled_list_preview::_generate_into_list(const _component_type& subject, int16_t level, int16_t count, std::vector<entry>& out) {
      if (count < 0)
         return;

      if (!(subject.flags & _component_type::flag::calculate_from_all_levels_below_player)) {
         uint32_t highest = 0;
         for (auto& entry : subject.entries)
            if (entry.level > highest)
               highest = entry.level;
         if (highest < level)
            level = highest;
      }

      auto _generate_one_from_this = [this, &subject, level, count, &out]() {
         auto generated = this->_generate_single_entry(false, subject, level, count);
         if (generated.count <= 0 || !generated.form)
            return;
         if (_is_leveled_list(*generated.form)) {
            auto  loaded = generated.form->load();
            auto* nested = loaded_forms::component_access::get_leveled_list(loaded);
            if (nested)
               this->_generate_into_list(*nested, level, generated.count, out);
         } else {
            // TODO: validate form type of `generated.form` and skip if not valid?
            out.push_back(generated);
         }
      };

      if (subject.flags & _component_type::flag::calculate_for_each_item_in_count) {
         //
         // Each result object should be generated independently.
         //
         if (count == 0)
            return;
         for (size_t i = 0; i < count; ++i) {
            _generate_one_from_this();
         }
      } else {
         //
         // Generate a single result object with `count` many instances. If the chosen 
         // leveled list entry(s) is a nested leveled list, multiply the count of its 
         // result by this stack frame's `count` argument.
         //
         if (subject.flags & _component_type::flag::use_all) {
            //
            // Individually generate every single item in the leveled list as a result,
            // if "chance none" passes.
            // 
            // We're not using `_generate_one_from_this` in this branch, because we're 
            // not generating a single item nor multiple items one at a time; as such, 
            // we need to run the "Chance None" roll ourselves.
            //
            if (_check_chance_none(subject)) {
               for (auto& entry : subject.entries) {
                  auto* stub = entry.form.get_form_stub();
                  if (!stub || !entry.count)
                     continue;
                  if (_is_leveled_list(*stub)) {
                     auto  loaded = stub->load();
                     auto* nested = loaded_forms::component_access::get_leveled_list(loaded);
                     if (nested)
                        this->_generate_into_list(*nested, level, entry.count, out);
                     continue;
                  }
                  out.push_back({
                     .form  = stub,
                     .count = (uint32_t)entry.count,
                  });
                  if (entry.item_extra_data.has_value()) {
                     auto& src = entry.item_extra_data.value();
                     auto& dst = out.back();
                     dst.extra = {
                        .health = src.health,
                        .owner  = src.ownership.get_owner()
                     };
                  }
               }
            }
         } else {
            //
            // Normal, flagless leveled list behavior: pick a single entry to generate.
            //
            _generate_one_from_this();
         }
         //
         // In the "calculate for each item in count" branch, we generate items one at 
         // a time, `count` many times. In this branch, however, we've generated all of 
         // the items at once, so now we need to multiply their counts by `count`.
         //
         for (auto& item : out) {
            item.count *= count;
         }
      }
   }

   void leveled_list_preview::prepare_game_settings(const file_load_order& lo, const _component_type& leveled_list) {
      const char* ldn = leveled_list.get_level_difference_setting_name();
      this->game_settings.max_level_difference = 0;

      for (const auto& gmst_base : dovah::game_settings) {
         auto _pull_value = [this, &lo, &gmst_base](const char* desired_name, float& dst) -> bool {
            if (gmst_base.name != desired_name)
               return false;

            dovah::loaded_game_setting loaded;

            float value = gmst_base.default_value.f;
            if (lo.get_loaded_setting_by_name(gmst_base.name, loaded))
               value = loaded.value.f;
            dst = value;
            return true;
         };

         if (_pull_value("fSpecialLootMinPCLevelBase", this->game_settings.special_loot.min.player_level.base))
            continue;
         if (_pull_value("fSpecialLootMinPCLevelMult", this->game_settings.special_loot.min.player_level.mult))
            continue;
         if (_pull_value("fSpecialLootMinZoneLevelBase", this->game_settings.special_loot.min.zone_level.base))
            continue;
         if (_pull_value("fSpecialLootMinZoneLevelMult", this->game_settings.special_loot.min.zone_level.mult))
            continue;

         if (_pull_value("fSpecialLootMaxPCLevelBase", this->game_settings.special_loot.max.player_level.base))
            continue;
         if (_pull_value("fSpecialLootMaxPCLevelMult", this->game_settings.special_loot.max.player_level.mult))
            continue;
         if (_pull_value("fSpecialLootMaxZoneLevelBase", this->game_settings.special_loot.max.zone_level.base))
            continue;
         if (_pull_value("fSpecialLootMaxZoneLevelMult", this->game_settings.special_loot.max.zone_level.mult))
            continue;

         if (_pull_value("fSpecialLootWeighting", this->game_settings.special_loot.weighting))
            continue;

         if (ldn) {
            if (gmst_base.name == ldn) {
               dovah::loaded_game_setting loaded;

               int32_t value = gmst_base.default_value.i;
               if (lo.get_loaded_setting_by_name(ldn, loaded)) {
                  value = loaded.value.i;
               }
               this->game_settings.max_level_difference = value;
            }
         }
      }
   }

   std::vector<leveled_list_preview::entry> leveled_list_preview::generate(const _component_type& leveled_list) {
      std::vector<entry> out;

      if (this->input_level < 0)
         this->input_level = 0;
      if (this->input_count < 0)
         return out;

      this->rng.seed(std::time(nullptr));

      this->_generate_into_list(leveled_list, this->input_level, this->input_count, out);
      return out;
   }
}