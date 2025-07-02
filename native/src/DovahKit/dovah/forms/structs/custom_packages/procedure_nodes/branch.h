#pragma once
#include <cstdint>
#include <memory>
#include <vector>
#include "../../../_common.h"

namespace dovah::loaded_forms::structs::custom_packages {
   class procedure_node;
}

namespace dovah::loaded_forms::structs::custom_packages::procedure_node_data {
   class branch {
      public:
         static constexpr const uint32_t subrecord_branch  = 'PRCB';

         static constexpr const size_t max_children = std::numeric_limits<uint32_t>::max();

         struct flag {
            enum type : uint32_t {
               repeat_when_complete = 1 << 0,
            };
         };

      public:
         std::vector<std::unique_ptr<procedure_node>> children;
         uint32_t flags = 0;
         
         // When we load PRCB, we resize our children vector to match the child count, 
         // but leave it empty. Procedure nodes are serialized list, you see, so our 
         // caller has to reorganize the children accordingly.
         //
         void load(tes_record_reader&, load_order_interfaces::form_load&);
         void save(tes_record_writer&, load_order_interfaces::form_save&);
   };
}