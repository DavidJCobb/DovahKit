#pragma once
#include <variant>
#include "../../_common.h"
#include "../../components/conditions.h"
#include "./procedure_nodes/branch.h"
#include "./procedure_nodes/procedure.h"
#include "dovah/data/packages/procedure_node_type.h"

namespace dovah::loaded_forms::structs::custom_packages {
   class procedure_node {
      public:
         static constexpr const uint32_t subrecord_typename  = 'ANAM';

         using procedure_node_type = packages::procedure_node_type;

      public:
         components::condition_list conditions; // CITC+CTDA[]
         std::variant<
            procedure_node_data::procedure, // "Procedure"
            procedure_node_data::branch,    // "Random"
            procedure_node_data::branch,    // "Sequence"
            procedure_node_data::branch,    // "Simultaneous"
            procedure_node_data::branch     // "Stacked"
         > data;

         static_assert(
            false,
            "TODO: This sucks. We should have a variant with just two members (`procedure` and `branch`), and "
            "have an enum on `branch` that indicates the branch type. (Currently the variant as a whole is "
            "synched with the `procedure_node_type` enum.)"
         );

      public:
         constexpr packages::procedure_node_type get_type() const noexcept { return (packages::procedure_node_type) this->data.index(); }
         constexpr size_t child_count() const noexcept {
            switch (this->data.index()) {
               case 1: return std::get<1>(this->data).children.size();
               case 2: return std::get<2>(this->data).children.size();
               case 3: return std::get<3>(this->data).children.size();
               case 4: return std::get<4>(this->data).children.size();
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