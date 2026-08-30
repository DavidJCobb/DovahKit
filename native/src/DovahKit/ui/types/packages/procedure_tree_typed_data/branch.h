#pragma once
#include <memory>
#include <vector>
#include "dovah/data/packages/procedure_tree_branch_type.h"
#include "dovah/forms/structs/custom_packages/package_flag_overrides.h"
namespace dovah::loaded_forms {
   namespace structs::custom_packages::procedure_node_data {
      class branch;
   }
   class Form;
}
namespace ui::types::packages {
   class procedure_node;
}

namespace ui::types::packages::procedure_tree_typed_data {
   class branch {
      public:
         using backend_type = dovah::loaded_forms::structs::custom_packages::procedure_node_data::branch;

         using branch_type = dovah::packages::procedure_tree_branch_type;

         static constexpr const size_t max_children = std::numeric_limits<uint32_t>::max();

         struct flag {
            enum type : uint32_t {
               repeat_when_complete = 1 << 0,
            };
         };

      public:
         branch_type type = branch_type::sequence;
         std::vector<std::unique_ptr<procedure_node>> children;
         uint32_t flags = 0;
         std::optional<dovah::loaded_forms::structs::custom_packages::package_flag_overrides> flag_overrides;

         constexpr bool can_have_flag_overrides() const noexcept {
            return this->type == branch_type::simultaneous;
         }

         void importData(const backend_type&);
         void exportData(backend_type&, dovah::loaded_forms::Form& dst_owner) const;
   };
}