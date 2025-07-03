#pragma once
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
         constexpr size_t child_count() const noexcept {
            if (std::holds_alternative<procedure_node_data::branch>(this->data)) {
               return std::get<procedure_node_data::branch>(this->data).children.size();
            }
            return 0;
         }

         void load(tes_record_reader&, load_order_interfaces::form_load&);
         static void generate_use_info(tes_record_reader&, std::vector<form_id_t>& out);
         void save(tes_record_writer&, load_order_interfaces::form_save&);
         procedure_node* clone(loaded_forms::Form& owner_of_clone) const noexcept;
         void sever_outbound_references_to(form_stub&, loaded_forms::Form& my_containing_form) noexcept;
         void clear(loaded_forms::Form& my_containing_form);

   };
}