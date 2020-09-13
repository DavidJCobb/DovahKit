#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "../_common.h"
#include "model.h"

namespace dovah {
   class form_stub;
   namespace loaded_forms::components {
      struct destruction_stage_data {
         struct data_flag {
            data_flag() = delete;
            enum type : uint8_t {
               vats_enabled = 0x01,
            };
         };
         struct stage_flag {
            stage_flag() = delete;
            enum type : uint8_t {
               cap_damage     = 0x01, // "When the object is at this stage, prevents you from skipping multiple stages by inflicting massive damage."
               disable_object = 0x02,
               destroy_object = 0x04,
               ignore_external_damage = 0x08, // "Prevents the object from taking damage from anything other than Self Damage Per Second."
            };
         };
         using data_flags_t  = std::underlying_type_t<data_flag::type>;
         using stage_flags_t = std::underlying_type_t<stage_flag::type>;

         struct Stage {
            uint16_t      healthPercent;
            uint8_t       damageStage;
            stage_flags_t flags;
            uint32_t      selfDamageRate; // the object inflicts this much damage on itself per second (i.e. health decay)
            form_id_t     explosionID;
            form_id_t     debrisID;
            uint32_t      debrisCount;
            model         replacementModel;
         };
         //
         uint32_t     health;
         data_flags_t flags;
         std::vector<Stage> stages;
         //
         void load(tes_subrecord_reader&);
         static void generateUseInfo(tes_subrecord_reader&, form_stub*);
      };
   }
}