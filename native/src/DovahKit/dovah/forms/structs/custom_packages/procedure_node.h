#pragma once
#include <memory>
#include <variant>
#include "../../_common.h"
#include "../../components/conditions.h"
#include "./procedure_nodes/branch.h"
#include "./procedure_nodes/procedure.h"
#include "./procedure_nodes/unknown.h"

namespace dovah::loaded_forms::structs::custom_packages {
   class procedure_node {
      public:
         static constexpr const uint32_t subrecord_typename  = 'ANAM';

      public:
         components::condition_list conditions; // CITC+CTDA[]
         std::variant<
            procedure_node_data::unknown,
            procedure_node_data::procedure,
            procedure_node_data::branch
         > data;

      public:
         // These functions only act on the serialized data for THIS NODE:
         size_t load(tes_record_reader&, load_order_interfaces::form_load&); // returns declared child count
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);
         void save(tes_record_writer&, load_order_interfaces::form_save&);

         // These functions are recursive, acting on all child and descendant nodes:
         std::unique_ptr<procedure_node> clone(loaded_forms::Form& owner_of_clone) const noexcept;
         void sever_outbound_references_to(form_stub&, loaded_forms::Form& my_containing_form) noexcept;
         void clear(loaded_forms::Form& my_containing_form);

   };
}