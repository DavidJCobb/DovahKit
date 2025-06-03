#include "./effect_item_handler.h"
#include <format>
#include "helpers/string/strieq_ascii.h"
#include "dovah/data/actor_values.h"
#include "dovah/forms/Global.h"
#include "dovah/forms/MagicEffect.h"

namespace dovah::text_replacers {
   std::optional<std::string> effect_item_handler::try_token_substitution(std::string_view tag, std::string_view subtag, std::string_view parameter) {
      if (!this->magic_effect)
         return "";

      if (cobb::strieq_ascii(tag, "global")) {
         form_stub* global = nullptr;
         auto&      lo     = this->magic_effect->get_owning_load_order();
         lo.for_each_form_of_type(dovah::form_type::global, [&global, parameter](dovah::form_stub* subj) -> bool {
            if (cobb::strieq_ascii(subj->editorID, parameter)) {
               global = subj;
               return true;
            }
            return false;
         });
         if (global) {
            auto gl = global->load().ptr_cast<loaded_forms::Global>();
            if (gl) {
               return std::format("<b>{}</b>", (int)gl->value);
            }
         }
         return "";
      }

      auto& loaded = this->loaded_magic_effect;
      if (!loaded) {
         loaded = this->magic_effect->load().ptr_cast<loaded_forms::MagicEffect>();
      }

      if (cobb::strieq_ascii(tag, "area")) {
         if (!loaded)
            return "";
         if (loaded->flags & loaded_forms::MagicEffect::effect_flag::no_area)
            return "";
         return std::format("<b>{}</b>", this->area);
      }
      if (cobb::strieq_ascii(tag, "dur")) {
         if (!loaded)
            return "";
         if (loaded->flags & loaded_forms::MagicEffect::effect_flag::no_duration)
            return "";
         return std::format("<b>{}</b>", this->duration);
      }
      if (cobb::strieq_ascii(tag, "mag")) {
         if (!loaded)
            return "";
         if (loaded->flags & loaded_forms::MagicEffect::effect_flag::no_magnitude)
            return "";

         float magnitude = this->magnitude;
         if (!this->simulate_active_effect_list) {
            //
            // The "Mod Spell Magnitude" perk entry point would be checked here, in-game.
            //
            ;
         }
         auto  avIndex   = loaded->associated_items.actor_value_indices[0];
         if (avIndex >= 0 && avIndex < all_actor_value_info.size()) {
            const auto& av_info = all_actor_value_info[avIndex];
            if (this->simulate_active_effect_list) {
               bool flip;
               {
                  bool recover = loaded->flags & loaded_forms::MagicEffect::effect_flag::recover;
                  bool inverts = av_info.flags & actor_value_info::flag::inverted;
                  flip = recover != inverts;
               }
               if (flip)
                  magnitude = -magnitude;
            }
            if (av_info.flags & actor_value_info::flag::displayed_effect_magnitude_times_one_hundred)
               magnitude *= 100.0F;
            if (this->simulate_active_effect_list) {
               //
               // The game would here check if the effect is Hostile and if the AV has 
               // flag `hostile_effects_scale_with_difficulty`, and if so, it'd scale 
               // the displayed value based on the current difficulty.
               //
               ;
            }
         }
         return std::format("<b>{}</b>", (int)magnitude);
      }

      //
      // Unrecognized tokens are bolded, but their parameters are omitted.
      //
      return std::format("<b>{}</b>", subtag);
   }
}