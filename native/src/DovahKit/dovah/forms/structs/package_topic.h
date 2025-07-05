#pragma once
#include <variant>
#include "../_common.h"

namespace dovah::loaded_forms::structs {
   struct package_topic {
      public:
         static constexpr const uint32_t subrecord_legacy = 'TPIC';
         static constexpr const uint32_t subrecord_modern = 'PDTO';

      public:
         std::variant<
            uint32_t,        // topic subtype signature
            form_reference_t // topic
         > data;

      public:
         void load(tes_subrecord_reader&, load_order_interfaces::form_load&);
         static void generate_use_info(tes_subrecord_reader&, form_stub_use_info_builder&);
         void save(tes_record_writer&, load_order_interfaces::form_save&);
         void clone_from(const package_topic&, loaded_forms::Form& my_containing_form) noexcept;
         void sever_outbound_references_to(form_stub&, loaded_forms::Form& my_containing_form) noexcept;
         void clear(loaded_forms::Form& my_containing_form);
   };
}