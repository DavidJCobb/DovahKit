#pragma once
#include "../_common.h"
#include "./conditions.h"

namespace dovah::loaded_forms::components {
   class magic_effect_list {
      public:
         static constexpr const uint32_t subrecord_signature_effect  = 'EFID'; // Effect ID
         static constexpr const uint32_t subrecord_signature_details = 'EFIT'; // Effect Item

         struct item {
            form_reference_t effect;           // EFID // MGEF
            float            magnitude = 0.0F; // EFIT+0x00
            uint32_t         area      = 0;    // EFIT+0x04
            uint32_t         duration  = 0;    // EFIT+0x08
            condition_list   conditions;       // CTDA
         };

      public:
         std::vector<item> items;
      
         void load(tes_record_reader&, load_order_interfaces::form_load& intfc);
         void save(tes_record_writer&, load_order_interfaces::form_save& intfc);
         void clear(loaded_forms::Form& my_containing_form) noexcept;
         void clone_from(const magic_effect_list& original, loaded_forms::Form& owner_of_clone) noexcept;
         void sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_containing_form) noexcept;

         struct use_info_state {
            std::vector<form_id_t> effect_forms;
            //
            void read(tes_record_reader&);
            void commit(form_stub_use_info_builder&);
         };
   };
}