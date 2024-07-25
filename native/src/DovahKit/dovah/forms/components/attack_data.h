#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "../_common.h"
#include "model.h"

namespace dovah {
   class form_stub;
   namespace loaded_forms::components {
      class attack_data { // BGSAttackDataForm
         public:
            static constexpr const uint32_t subrecord_signature_race  = 'ATKR';
            static constexpr const uint32_t subrecord_signature_data  = 'ATKD';
            static constexpr const uint32_t subrecord_signature_event = 'ATKE'; // NOTE: Form loaders don't check for this. It's blindly swallowed after ATKD.

         public:
            struct flag {
               enum type : uint32_t {
                  ignore_weapon   = 0x00000001,
                  bash_attack     = 0x00000002,
                  power_attack    = 0x00000004,
                  left_attack     = 0x00000008,
                  rotating_attack = 0x00000010,
                  //
                  override_data   = 0x80000000,
               };
            };
            using flags_t = std::underlying_type_t<flag::type>;

            struct attack {
               float   damage_mult   = 1.0F; // ATKD+0x00
               float   attack_chance = 1.0F; // ATKD+0x04
               form_reference_t attack_spell; // ATKD+0x08
               flags_t flags         = 0; // ATKD+0x0C
               float   attack_angle  = 0.0F; // ATKD+0x10
               float   strike_angle  = 0.0F; // ATKD+0x14 // defaults to GMST:fCombatHitConeAngle
               float   stagger       = 0.0F; // ATKD+0x18
               form_reference_t keyword; // ATKD+0x1C
               float   knockdown     = 0.0F; // ATKD+0x20
               float   recovery_time = 0.0F; // ATKD+0x24
               float   stamina_mult  = 1.0F; // ATKD+0x28
               std::string event; // ATKE
            };

            form_reference_t race; // ATKR
            std::vector<attack> attacks;
            //
            void load(tes_record_reader&, load_order_interfaces::form_load& intfc);
            void save(tes_record_writer&, load_order_interfaces::form_save& intfc);
            void clone_from(const attack_data& original, loaded_forms::Form& owner_of_clone) noexcept;
            void sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) noexcept;
            void clear(loaded_forms::Form& my_owner);
         
            struct use_info_state {
               form_id_t race;
               std::vector<form_id_t> attack_forms;
               //
               void read(tes_record_reader&);
               void commit(form_stub_use_info_builder&);
            };
      };
   }
}