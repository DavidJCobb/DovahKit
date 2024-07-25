#pragma once
#include "../_common.h"

namespace dovah::loaded_forms::components {
   class spell_list {
      public:
         static constexpr const uint32_t subrecord_signature_count = 'SPCT';
         static constexpr const uint32_t subrecord_signature_entry = 'SPLO';

      public:
         std::vector<form_reference_t> forms;
      
         void load(tes_subrecord_reader&, load_order_interfaces::form_load& intfc);
         static void generate_use_info(tes_subrecord_reader&, form_stub_use_info_builder&);
         void save(tes_record_writer&, load_order_interfaces::form_save& intfc);
         //
         void clear(loaded_forms::Form& my_containing_form) noexcept;
         void clone_from(const spell_list& original, loaded_forms::Form& owner_of_clone) noexcept;
         void sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_containing_form) noexcept;
   };
}