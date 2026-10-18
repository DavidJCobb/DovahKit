#pragma once
#include <cstdint>
#include <unordered_map>
#include <vector>
#include "../_common.h"
#include "./world_bounds.h"
namespace dovah::tes_file_reading {
   class file_loader;
}

namespace dovah::loaded_forms::structs {
   struct world_cell_offset_data {
      public:
         using load_order_file = dovah::tes_file_reading::file_loader;

         static constexpr const uint32_t subrecord_signature = 'OFST';

         struct offset_collection {
            std::vector<uint32_t> relative_offsets;
            world_bounds bounds;
         };

      public:
         std::unordered_map<const load_order_file*, offset_collection> offsets_by_file;

      public:
         void load(tes_subrecord_reader&, load_order_interfaces::form_load&);
   };
}