#include "cell_region_list.h"
#include "../../_common_cpp.h"

namespace dovah::loaded_forms::components::extra {
   extra_data_load_result cell_region_list::load(tes_subrecord_reader& subrecord) {
      if (subrecord.signature() != signature)
         return load_result::unrecognized;
      auto size = subrecord.size() / 4;
      this->regions.resize(size);
      for (size_t i = 0; i < size; ++i)
         subrecord.read(this->regions[i]);
      return load_result::succeeded;
   }
   void cell_region_list::save(tes_record_writer& record) {
      auto& subrecord = record.open_next_subrecord(signature);
      for (auto id : this->regions)
         subrecord.write(id);
      subrecord.close();
   }
}