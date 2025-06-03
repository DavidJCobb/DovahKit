#include "./magic_effect_list_item_cost_calculator.h"
#include "../forms/MagicEffect.h"

namespace dovah {
   void magic_effect_list_item_cost_calculator::prepare_game_settings(const file_load_order& lo) {
      constexpr size_t desired = 4;
      size_t found = 0;
      for (const auto& gmst_base : dovah::game_settings) {
         if (found == desired)
            break;

         auto _pull_value = [this, &lo, &gmst_base, &found](const char* desired_name, float& dst) -> bool {
            if (gmst_base.name != desired_name)
               return false;

            dovah::loaded_game_setting loaded;

            float value = gmst_base.default_value.f;
            if (lo.get_loaded_setting_by_name(gmst_base.name, loaded))
               value = loaded.value.f;
            dst = value;
            ++found;
            return true;
         };

         if (_pull_value("fMagicAreaBaseCostMult", this->game_settings.fMagicAreaBaseCostMult))
            continue;
         if (_pull_value("fMagicDurMagBaseCostMult", this->game_settings.fMagicDurMagBaseCostMult))
            continue;
         if (_pull_value("fMagicCostScale", this->game_settings.fMagicCostScale))
            continue;
         if (_pull_value("fMagicRangeTargetCostMult", this->game_settings.fMagicRangeTargetCostMult))
            continue;
      }
   }
   void magic_effect_list_item_cost_calculator::prepare_effect_item(const loaded_forms::components::magic_effect_list::item& src) {
      this->effect.area      = src.area;
      this->effect.magnitude = src.magnitude;
      this->effect.duration  = src.duration;
      this->prepare_effect_form(src.effect.get_form_stub());
   }
   void magic_effect_list_item_cost_calculator::prepare_effect_form(form_stub* mgef) {
      this->effect.form      = mgef;
      this->effect.form_info = {};
      if (!mgef)
         return;
      auto loaded = mgef->load().ptr_cast<loaded_forms::MagicEffect>();
      if (!loaded)
         return;
      this->effect.form_info.base_cost     = loaded->base_cost;
      this->effect.form_info.casting_type  = loaded->casting_type;
      this->effect.form_info.delivery_type = loaded->delivery_type;
      if (loaded->flags & loaded_forms::MagicEffect::effect_flag::no_area)
         this->effect.form_info.no_area = true;
      if (loaded->flags & loaded_forms::MagicEffect::effect_flag::no_duration)
         this->effect.form_info.no_duration = true;
      if (loaded->flags & loaded_forms::MagicEffect::effect_flag::no_magnitude)
         this->effect.form_info.no_magnitude = true;
      if (loaded->flags & loaded_forms::MagicEffect::effect_flag::unknown_24)
         this->effect.form_info.flag_24 = true;
   }

   float magic_effect_list_item_cost_calculator::calculate() const {
      float calc_area = 0.0F;
      float calc_mag  = 0.0F;
      if (!this->effect.form_info.no_area)
         calc_area = this->effect.area * this->game_settings.fMagicAreaBaseCostMult;
      if (!this->effect.form_info.no_magnitude)
         calc_mag  = this->effect.magnitude;
      calc_area = std::max(1.0F, calc_area);
      calc_mag  = std::max(1.0F, calc_mag);

      float duration_scale = 1.0F;
      {
         auto duration = this->effect.duration;
         if (this->effect.form_info.no_duration) {
            duration = 0;
         }
         if (this->effect.form_info.casting_type != magic_casting_type::concentration && duration >= 1) {
            duration_scale = (float)duration * this->game_settings.fMagicDurMagBaseCostMult;
         }
      }
      float scaled_magnitude = std::pow(duration_scale * calc_mag, this->game_settings.fMagicCostScale);

      float result = calc_area * this->effect.form_info.base_cost * scaled_magnitude;
      if (this->effect.form_info.delivery_type == magic_delivery_type::aimed) {
         result *= this->game_settings.fMagicRangeTargetCostMult;
      }
      return result;
   }
}