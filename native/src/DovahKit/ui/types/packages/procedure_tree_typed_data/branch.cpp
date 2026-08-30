#include "./branch.h"
#include "dovah/forms/structs/custom_packages/procedure_nodes/branch.h"
#include "dovah/forms/structs/custom_packages/procedure_node.h"
#include "../procedure_node.h"

namespace ui::types::packages::procedure_tree_typed_data {
   void branch::importData(const backend_type& src) {
      this->flags = src.flags;
      this->type  = src.branch_type;
      if (src.can_have_flag_overrides()) {
         this->flag_overrides = src.flag_overrides;
      } else {
         this->flag_overrides.reset();
      }

      const auto& src_list = src.children;
      auto&       dst_list = this->children;
      const auto  size     = src_list.size();
      dst_list.clear();
      dst_list.reserve(size);
      for (const auto& src_ptr : src_list) {
         auto& dst_ptr = dst_list.emplace_back(std::make_unique<procedure_node>());
         dst_ptr->importData(*src_ptr);
      }
   }
   void branch::exportData(backend_type& dst, dovah::loaded_forms::Form& dst_owner) const {
      dst.branch_type = this->type;
      dst.flags       = this->flags;
      if (this->can_have_flag_overrides()) {
         dst.flag_overrides = this->flag_overrides;
      } else {
         dst.flag_overrides.reset();
      }
      
      const auto& src_list = this->children;
      auto&       dst_list = dst.children;
      const auto  size     = src_list.size();
      dst_list.clear();
      dst_list.reserve(size);
      for (const auto& src_ptr : src_list) {
         auto& dst_ptr = dst_list.emplace_back(std::make_unique<procedure_node::backend_type>());
         src_ptr->exportData(*dst_ptr, dst_owner);
      }
   }
}