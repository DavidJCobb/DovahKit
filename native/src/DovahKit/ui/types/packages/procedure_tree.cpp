#include "./procedure_tree.h"
#include "./procedure_node.h"
#include "dovah/forms/structs/custom_packages/procedure_tree.h"

namespace ui::types::packages {
   void procedure_tree::importData(const backend_type& src) {
      if (src.root) {
         if (!this->root)
            this->root = std::make_unique<procedure_node>();
         this->root->importData(*src.root);
      } else {
         this->root.reset();
      }

      const auto& src_list = src.orphans;
      auto&       dst_list = this->orphans;
      const auto  size     = src_list.size();
      dst_list.clear();
      dst_list.reserve(size);
      for (const auto& src_ptr : src_list) {
         auto& dst_ptr = dst_list.emplace_back(std::make_unique<procedure_node>());
         dst_ptr->importData(*src_ptr);
      }
   }
   void procedure_tree::exportData(backend_type& dst, dovah::loaded_forms::Form& dst_owner) const {
      dst.clear(dst_owner);

      if (this->root) {
         if (!dst.root)
            dst.root = std::make_unique<procedure_node::backend_type>();
         this->root->exportData(*dst.root, dst_owner);
      }
      
      const auto& src_list = this->orphans;
      auto&       dst_list = dst.orphans;
      const auto  size     = src_list.size();
      dst_list.reserve(size);
      for (const auto& src_ptr : src_list) {
         auto& dst_ptr = dst_list.emplace_back(std::make_unique<procedure_node::backend_type>());
         src_ptr->exportData(*dst_ptr, dst_owner);
      }
   }
}