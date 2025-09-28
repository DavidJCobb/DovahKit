#pragma once
#include <variant>
#include <vector>
#include "./procedure_tree_typed_data/branch.h"
#include "./procedure_tree_typed_data/procedure.h"
#include "../conditions/condition.h"
namespace dovah::loaded_forms::structs::custom_packages {
   class procedure_node;
}

namespace ui::types::packages {
   class procedure_node {
      public:
         using backend_type = dovah::loaded_forms::structs::custom_packages::procedure_node;

      public:
         procedure_node* parent_node = nullptr; // unowned
         std::vector<conditions::condition> conditions;
         std::variant<
            std::monostate,
            procedure_tree_typed_data::procedure,
            procedure_tree_typed_data::branch
         > data;

      public:
         void importData(const backend_type&);
         void exportData(backend_type&, dovah::loaded_forms::Form& dst_owner) const;

         bool contains(const procedure_node&) const noexcept;
         size_t index_of(const procedure_node&) const noexcept;
   };
}