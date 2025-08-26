#pragma once
#include <memory>
#include <vector>
namespace dovah::loaded_forms {
   namespace structs::custom_packages {
      class procedure_tree;
   }
   class Form;
}
namespace ui::types::packages {
   class procedure_node;
}

namespace ui::types::packages {
   class procedure_tree {
      public:
         using backend_type = dovah::loaded_forms::structs::custom_packages::procedure_tree;

      public:
         std::unique_ptr<procedure_node> root;
         std::vector<std::unique_ptr<procedure_node>> orphans; // extra top-level nodes

      public:
         void importData(const backend_type&);
         void exportData(backend_type&, dovah::loaded_forms::Form& dst_owner) const;
   };
}