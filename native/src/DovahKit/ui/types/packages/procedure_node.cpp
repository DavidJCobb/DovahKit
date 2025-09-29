#include "./procedure_node.h"
#include <memory>
#include "helpers/vectors/move_item_within.h"
#include "dovah/forms/structs/custom_packages/procedure_node.h"
namespace {
   namespace typed_node_data {
      using namespace dovah::loaded_forms::structs::custom_packages::procedure_node_data;
   }
}

namespace ui::types::packages {
   void procedure_node::importData(const backend_type& src) {
      {
         auto& src_list = src.conditions;
         auto& dst_list = this->conditions;
         const size_t size = src_list.size();
         dst_list.clear();
         dst_list.reserve(size);
         for (const auto& src_item : src_list)
            dst_list.emplace_back(src_item);
      }
      {
         auto& src_variant = src.data;
         auto& dst_variant = this->data;
         if (auto* src_casted = std::get_if<typed_node_data::branch>(&src_variant)) {
            auto& dst_casted = dst_variant.emplace<procedure_tree_typed_data::branch>();
            dst_casted.flags = src_casted->flags;
            dst_casted.type  = src_casted->branch_type;

            auto& src_list = src_casted->children;
            auto& dst_list = dst_casted.children;
            const size_t size = src_list.size();
            dst_list.clear();
            dst_list.reserve(size);
            for (const auto& src_ptr : src_list) {
               auto& dst_ptr = dst_list.emplace_back(std::make_unique<procedure_node>());
               dst_ptr->parent_node = this;
               dst_ptr->importData(*src_ptr);
            }
         } else if (auto* src_casted = std::get_if<typed_node_data::procedure>(&src_variant)) {
            auto& dst_casted = dst_variant.emplace<procedure_tree_typed_data::procedure>();
            dst_casted.flags = src_casted->flags;
            dst_casted.type  = src_casted->type;
            dst_casted.flag_overrides       = src_casted->flag_overrides;
            dst_casted.parameter_unique_ids = src_casted->parameter_unique_ids;
         }
      }
   }
   void procedure_node::exportData(backend_type& dst, dovah::loaded_forms::Form& dst_owner) const {
      dst.clear(dst_owner);
      {
         auto& src_list = this->conditions;
         auto& dst_list = dst.conditions;
         const size_t size = src_list.size();
         dst_list.reserve(size);
         dst_list.append_all_of(dst_owner, src_list);
      }
      {
         auto& src_variant = this->data;
         auto& dst_variant = dst.data;
         if (auto* src_casted = std::get_if<procedure_tree_typed_data::branch>(&src_variant)) {
            auto& dst_casted = dst_variant.emplace<typed_node_data::branch>();
            dst_casted.branch_type = src_casted->type;
            dst_casted.flags       = src_casted->flags;

            auto& src_list = src_casted->children;
            auto& dst_list = dst_casted.children;
            const size_t size = src_list.size();
            dst_list.clear();
            dst_list.reserve(size);
            for (const auto& src_ptr : src_list) {
               auto& dst_ptr = dst_list.emplace_back(std::make_unique<procedure_node::backend_type>());
               src_ptr->exportData(*dst_ptr, dst_owner);
            }
         } else if (auto* src_casted = std::get_if<procedure_tree_typed_data::procedure>(&src_variant)) {
            auto& dst_casted = dst_variant.emplace<typed_node_data::procedure>();
            dst_casted.flags = src_casted->flags;
            dst_casted.type  = src_casted->type;
            dst_casted.flag_overrides       = src_casted->flag_overrides;
            dst_casted.parameter_unique_ids = src_casted->parameter_unique_ids;
         }
      }
   }

   bool procedure_node::contains(const procedure_node& descendant) const noexcept {
      for (const auto* parent = descendant.parent_node; parent; parent = parent->parent_node)
         if (parent == this)
            return true;
      return false;
   }
   size_t procedure_node::index_of(const procedure_node& child) const noexcept {
      if (child.parent_node != this)
         return (size_t)-1;
      auto* casted = std::get_if<procedure_tree_typed_data::branch>(&this->data);
      if (!casted)
         return (size_t)-1;
      for (size_t i = 0; i < casted->children.size(); ++i)
         if (casted->children[i].get() == &child)
            return i;
      return (size_t)-1;
   }

   void procedure_node::append_child(std::unique_ptr<procedure_node>&& node_ptr) {
      assert(std::holds_alternative<procedure_tree_typed_data::branch>(this->data));
      if (node_ptr->parent_node == this)
         return;
      assert(node_ptr->parent_node == nullptr && "Don't std::move a parent's children directly! Call take_child to prep them for move!");
      auto& list    = std::get<procedure_tree_typed_data::branch>(this->data).children;
      auto& dst_ptr = list.emplace_back(); // ensure that if we can't realloc, we throw before making changes
      node_ptr->parent_node = this;
      dst_ptr = std::move(node_ptr);
   }
   void procedure_node::insert_child(std::unique_ptr<procedure_node>&& node_ptr, size_t at_index) {
      auto& node = *node_ptr;
      auto& list = std::get<procedure_tree_typed_data::branch>(this->data).children;
      if (node.parent_node == this) {
         auto i = this->index_of(node);
         if (at_index == i)
            return;
         cobb::vectors::move_item_within(
            list,
            i,
            (ptrdiff_t)at_index - i
         );
      } else {
         //
         // We could get rid of the need for this assertion by making the child list private...
         //
         assert(node.parent_node == nullptr && "Don't std::move a parent's children directly! Call take_child to prep them for move!");
         list.reserve(list.size() + 1); // ensure that if we can't realloc, we throw before making changes
         node.parent_node = this;
         list.insert(list.begin() + at_index, std::move(node_ptr));
      }
   }
   std::unique_ptr<procedure_node> procedure_node::take_child(size_t i) {
      assert(std::holds_alternative<procedure_tree_typed_data::branch>(this->data));
      auto& list = std::get<procedure_tree_typed_data::branch>(this->data).children;
      assert(i < list.size());
      auto node_ptr = std::move(list[i]);
      node_ptr->parent_node = nullptr;
      list.erase(list.begin() + i);
      return node_ptr;
   }
}