#pragma once
#include <cstdint>
#include "../_common.h"

namespace dovah {
   namespace loaded_forms::components {
      class enchantable { // TESEnchantableForm
         public:
            static constexpr const uint32_t subrecord_signature_effect        = 'EITM'; // Effect ITeM
            static constexpr const uint32_t subrecord_signature_charge        = 'EAMT'; // Enchantment AMounT
            static constexpr const uint32_t subrecord_signature_effect_legacy = 'ENAM'; // changed to EITM since Fallout 3
            static constexpr const uint32_t subrecord_signature_charge_legacy = 'ANAM'; // changed to EAMT since Fallout 3

         public:
            form_reference_t effect;
            uint16_t charge = 0;

            void load(tes_subrecord_reader&, load_order_interfaces::form_load& intfc);
            void save(tes_record_writer&, load_order_interfaces::form_save& intfc);
            void clone_from(const enchantable& original, loaded_forms::Form& owner_of_clone) noexcept;
            void sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) noexcept;
            void clear(loaded_forms::Form& my_owner);
         
            struct use_info_state {
               form_id_t effect;
               
               void read(tes_subrecord_reader&);
               void commit(form_stub_use_info_builder&);
            };
      };
   }
}