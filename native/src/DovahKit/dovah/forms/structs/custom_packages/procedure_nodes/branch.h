#pragma once
#include <cstdint>
#include <memory>
#include <vector>
#include "../../../_common.h"
#include "../../../../data/packages/procedure_tree_branch_type.h"

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
         packages::procedure_tree_branch_type branch_type = packages::procedure_tree_branch_type::sequence;
         std::vector<std::unique_ptr<procedure_node>> children;
         uint32_t flags = 0;
         
         // these functions should be called just after the first subrecord constituting this struct's 
         // data has been opened. when the functions exit, the subrecord after this struct's data will 
         // have just been opened.
         size_t load(tes_record_reader&, load_order_interfaces::form_load&); // returns the declared child count
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);

         void save(tes_record_writer&, load_order_interfaces::form_save&);
   };
}