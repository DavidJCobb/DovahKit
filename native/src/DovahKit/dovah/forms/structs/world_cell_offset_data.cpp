#include "./world_cell_offset_data.h"
#include "../_common_cpp.h"

namespace dovah::loaded_forms::structs {
   void world_cell_offset_data::load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc) {
      if (subrecord.signature() == subrecord_signature) {
         const auto* file = intfc.current_file;
         auto&       data = this->offsets_by_file[file];

         const size_t size = subrecord.size() / sizeof(uint32_t);
         data.relative_offsets.resize(size);
         for (size_t i = 0; i < size; ++i)
            subrecord.unchecked_read(data.relative_offsets[i]);
         return;
      }
   }
}