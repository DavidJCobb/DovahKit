#pragma once
#include <memory>
#include <vector>
#include "dovah/data/packages/procedure_type.h"
#include "dovah/forms/structs/custom_packages/package_flag_overrides.h"
namespace dovah::loaded_forms {
   namespace structs::custom_packages::procedure_node_data {
      class procedure;
   }
   class Form;
}
namespace ui::types::packages {
   class procedure_node;
}

namespace ui::types::packages::procedure_tree_typed_data {
   class procedure {
      public:
         using backend_type = dovah::loaded_forms::structs::custom_packages::procedure_node_data::procedure;

         using procedure_type = dovah::packages::procedure_type;

         struct flag {
            enum type : uint32_t {
               success_completes_package = 1 << 0,
            };
         };

      public:
         procedure_type type = procedure_type::invalid;
         uint32_t flags = 0; // FNAM
         std::optional<dovah::loaded_forms::structs::custom_packages::package_flag_overrides> flag_overrides;
         std::vector<uint8_t> parameter_unique_ids; // PKC2[]

         void importData(const backend_type&);
         void exportData(backend_type&, dovah::loaded_forms::Form& dst_owner) const;
   };
}