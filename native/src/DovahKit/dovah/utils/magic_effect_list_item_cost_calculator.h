#pragma once
#include <cstdint>
#include "../data/magic_casting_type.h"
#include "../data/magic_delivery_type.h"
#include "../forms/components/magic_effect_list.h"

namespace dovah {
   class file_load_order;
   class form_stub;
}

namespace dovah {
   class magic_effect_list_item_cost_calculator {
      public:
         struct {
            form_stub* form; // EFID // MGEF
            struct {
               bool  no_area      = false;
               bool  no_duration  = false;
               bool  no_magnitude = false;
               bool  flag_24      = false;
               float base_cost    = 0.0F;
               float charge_time  = 0.0F;
               magic_casting_type  casting_type  = magic_casting_type::fire_and_forget;
               magic_delivery_type delivery_type = magic_delivery_type::self;
            } form_info;
            float      magnitude = 0.0F; // EFIT+0x00
            uint32_t   area      = 0;    // EFIT+0x04
            uint32_t   duration  = 0;    // EFIT+0x08
         } effect;
         struct {
            float fMagicAreaBaseCostMult    = 0.0F;
            float fMagicDurMagBaseCostMult  = 0.1F;
            float fMagicCostScale           = 1.1F;
            float fMagicRangeTargetCostMult = 1.0F;
         } game_settings;

      public:
         void prepare_game_settings(const file_load_order&);
         void prepare_effect_item(const loaded_forms::components::magic_effect_list::item&);
         void prepare_effect_form(form_stub*);

         float calculate() const;
   };
}