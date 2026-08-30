#pragma once
#include <memory>
#include <vector>
#include "../../_common.h"
#include "./procedure_node.h"

namespace dovah::loaded_forms::structs::custom_packages {
   class procedure_tree {
      public:
         std::unique_ptr<procedure_node> root;
         std::vector<std::unique_ptr<procedure_node>> orphans; // extra top-level nodes

      public:
         template<typename Functor>
         void for_each_node(Functor&& functor) {
            auto traverse = [&functor](this auto&& recurse, procedure_node& node) -> void {
               functor(node);
               if (auto* casted = std::get_if<procedure_node_data::branch>(&node.data)) {
                  for (auto& child_ptr : casted->children)
                     recurse(*child_ptr);
               }
            };
            if (this->root)
               traverse(*this->root);
            for (auto& node_ptr : this->orphans)
               traverse(*node_ptr);
         }
         
         template<typename Functor, typename TrailerFunctor>
         void for_each_node_with_trailer(Functor&& functor, TrailerFunctor&& trailer) {
            auto traverse = [&functor, &trailer](this auto&& recurse, procedure_node& node) -> void {
               functor(node);
               if (auto* casted = std::get_if<procedure_node_data::branch>(&node.data)) {
                  for (auto& child_ptr : casted->children)
                     recurse(*child_ptr);
               }
               trailer(node);
            };
            if (this->root)
               traverse(*this->root);
            for (auto& node_ptr : this->orphans)
               traverse(*node_ptr);
         }

      public:
         void load(tes_record_reader&, load_order_interfaces::form_load&);
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);
         void save(tes_record_writer&, load_order_interfaces::form_save&);
         void clone_from(const procedure_tree& src, Form& my_owner) noexcept;
         void sever_outbound_references_to(form_stub&, loaded_forms::Form& my_containing_form) noexcept;
         void clear(loaded_forms::Form& my_containing_form);

   };
}