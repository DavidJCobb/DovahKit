#include "./procedure_tree.h"
#include "../../_common_cpp.h"
#include <array>
#include <utility>

#include "../../../notices/form_load_warnings/by_form_type/package/procedure_tree_has_orphaned_nodes.h"

namespace {
   namespace specific_load_warnings {
      using namespace dovah::notices::form_load_warnings::by_type::package;
   }
}

namespace dovah::loaded_forms::structs::custom_packages {
   void procedure_tree::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      struct loaded_node {
         std::unique_ptr<procedure_node> ptr;
         size_t child_count = 0;
      };
      std::vector<loaded_node> nodes;

      while (record.get_current_subrecord().signature() == procedure_node::subrecord_typename) {
         auto  node_ptr = std::make_unique<procedure_node>();
         auto* node     = node_ptr.get();

         auto& entry = nodes.emplace_back();
         entry.ptr         = std::move(node_ptr);
         entry.child_count = node->load(record, intfc);
      }

      if (nodes.empty())
         return;
      
      // Returns index to continue from.
      auto _acquire = [&nodes](this auto&& recurse, procedure_node& parent, size_t parent_index, size_t child_count) -> size_t {
         if (child_count == 0) {
            return parent_index + 1;
         }
         assert(std::holds_alternative<procedure_node_data::branch>(parent.data));
         auto& branch_data = std::get<procedure_node_data::branch>(parent.data);

         size_t child_index = parent_index + 1;
         for (size_t i = 0; i < child_count; ++i) {
            auto&  child_info  = nodes[child_index];
            auto&  child_ptr   = child_info.ptr;
            auto*  child       = child_ptr.get();

            branch_data.children.push_back(std::move(child_ptr));

            if (auto* casted = std::get_if<procedure_node_data::branch>(&child->data)) {
               child_index = recurse(*child, child_index, child_info.child_count);
            } else {
               ++child_index;
            }
         }
         return child_index;
      };
      
      this->root = std::move(nodes[0].ptr);
      size_t next = _acquire(
         *this->root,
         0,
         nodes[0].child_count
      );
      while (next < nodes.size()) {
         auto& orphan_info = nodes[next];
         auto* orphan      = orphan_info.ptr.get();
         this->orphans.push_back(std::move(orphan_info.ptr));
         next = _acquire(
            *orphan,
            next,
            orphan_info.child_count
         );
      }

      if (this->orphans.size()) {
         specific_load_warnings::procedure_tree_has_orphaned_nodes notice(
            intfc.target_stub,
            this->orphans.size()
         );
         intfc.log_load_warning(notice);
      }
   }
   /*static*/ void procedure_tree::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      while (record.get_current_subrecord().signature() == procedure_node::subrecord_typename) {
         procedure_node::generate_use_info(record, uib);
      }
   }
   void procedure_tree::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->for_each_node([&record, &intfc](procedure_node& node) {
         node.save(record, intfc);
      });
   }
   void procedure_tree::clone_from(const procedure_tree& src, Form& my_owner) noexcept {
      this->clear(my_owner);
      if (src.root) {
         auto clone_ptr = src.root->clone(my_owner);
         this->root = std::move(clone_ptr);
      }
      for (auto& node_ptr : src.orphans) {
         auto clone_ptr = node_ptr->clone(my_owner);
         this->orphans.push_back(std::move(clone_ptr));
      }
   }
   void procedure_tree::sever_outbound_references_to(form_stub& other, loaded_forms::Form& my_owner) noexcept {
      if (this->root)
         this->root->sever_outbound_references_to(other, my_owner);
      for (auto& node_ptr : this->orphans)
         node_ptr->sever_outbound_references_to(other, my_owner);
   }
   void procedure_tree::clear(loaded_forms::Form& my_owner) {
      if (this->root) {
         this->root->clear(my_owner);
         this->root.reset();
      }

      for (auto& node_ptr : this->orphans)
         node_ptr->clear(my_owner);
      this->orphans.clear();
   }
}