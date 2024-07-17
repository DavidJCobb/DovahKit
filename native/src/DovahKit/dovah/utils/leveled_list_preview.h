#pragma once
#include <cstdint>
#include <optional>
#include <random>

namespace dovah {
   namespace loaded_forms::components {
      class leveled_list;
   }
   class file_load_order;
   class form_stub;
}

namespace dovah {
   class leveled_list_preview {
      protected:
         using _component_type = loaded_forms::components::leveled_list;

      public:
         enum class selection_mode {
            use_default_behavior,             // Medium, Hard
            vary_levels_only_when_cumulative, // Easy
            always_vary_levels,               // None
            prefer_first_above_cap,           // Very Hard
         };

         struct entry_extra {
            float      health = 1;
            form_stub* owner  = nullptr;
         };
         struct entry {
            form_stub* form   = nullptr;
            uint32_t   count  = 0;
            std::optional<entry_extra> extra;
         };

      public:
         selection_mode mode = selection_mode::use_default_behavior;
         int16_t player_level = 1; // always the player's level
         int16_t input_level  = 1; // generally either the player's level, or max(player level, encounter zone level)
         int16_t input_count  = 1;
         bool    use_special_loot_formula = false;
      protected:
         mutable std::mt19937 rng;
         struct {
            int32_t max_level_difference = 0;
            struct {
               struct {
                  struct {
                     float base = 0;
                     float mult = 1;
                  } player_level;
                  struct {
                     float base = 0;
                     float mult = 1;
                  } zone_level;
               } min;
               struct {
                  struct {
                     float base = 0;
                     float mult = 1;
                  } player_level;
                  struct {
                     float base = 0;
                     float mult = 1;
                  } zone_level;
               } max;
               float weighting = 1;
            } special_loot;
         } game_settings; // pre-cache before we run the algorithm

         bool _check_chance_none(const _component_type&) const;
         static bool _is_leveled_list(dovah::form_stub& stub);
         int32_t _special_loot_level(int16_t input_level) const;

         entry _generate_single_entry(
            bool process_nested_lists,
            const _component_type& subject,
            int16_t level,
            int16_t count
         );
         void _generate_into_list(const _component_type& subject, int16_t level, int16_t count, std::vector<entry>& out);

      public:
         void prepare_game_settings(const file_load_order&, const _component_type&);
         std::vector<entry> generate(const _component_type& leveled_list);
   };
}