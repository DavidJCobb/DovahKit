#pragma once
#include <cstdint>
#include "../_common.h"

namespace dovah {
   namespace loaded_forms::components {
      class harvestable { // TESProduceForm
         public:
            static constexpr const uint32_t subrecord_signature_ingredient  = 'PFIG';
            static constexpr const uint32_t subrecord_signature_sound       = 'SNAM';
            static constexpr const uint32_t subrecord_signature_percentages = 'PFPC';

         public:
            form_reference_t ingredient;    // PFIG
            form_reference_t harvest_sound; // SNAM; form type SNDR // shadows ACTI/SNAM, so that superclass field will never load
            struct {
               uint8_t spring = 100;
               uint8_t summer = 100;
               uint8_t autumn = 100;
               uint8_t winter = 100;
            } chance_by_season;
            
            void load(tes_record_reader&, load_order_interfaces::form_load& intfc);
            void save(tes_record_writer&, load_order_interfaces::form_save& intfc);
            void clone_from(const harvestable& original, loaded_forms::Form& owner_of_clone) noexcept;
            void sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) noexcept;
            void clear(loaded_forms::Form& my_owner);
         
            struct use_info_state {
               form_id_t ingredient    = {};
               form_id_t harvest_sound = {};
               
               void read(tes_record_reader&);
               void commit(form_stub_use_info_builder&);
            };
      };
   }
}