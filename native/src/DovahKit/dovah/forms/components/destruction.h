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

         static constexpr int max_stage_count = std::numeric_limits<uint8_t>::max();

         struct Stage {
            uint16_t         healthPercent;
            uint8_t          damageStage;
            stage_flags_t    flags;
            uint32_t         selfDamageRate; // the object inflicts this much damage on itself per second (i.e. health decay)
            form_reference_t explosion;
            form_reference_t debris;
            uint32_t         debrisCount;
            model_ts         replacementModel;
         };
         //
         uint32_t     health;
         data_flags_t flags;
         std::vector<Stage> stages;
         //
         void load(tes_subrecord_reader&, load_order_interfaces::form_load& intfc);
         static void generate_use_info(tes_subrecord_reader&, form_stub_use_info_builder&);
         void save(tes_record_writer&, load_order_interfaces::form_save& intfc);
         void clone_from(const destruction_stage_data& original, loaded_forms::Form& owner_of_clone) noexcept;
         void sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) noexcept;
         void clear(loaded_forms::Form& my_owner);
      };
   }
}