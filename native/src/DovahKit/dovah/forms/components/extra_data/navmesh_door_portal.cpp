#include "navmesh_door_portal.h"
#include "../../_common_cpp.h"

namespace dovah::loaded_forms::components::extra {
   extra_data_load_result navmesh_door_portal::load(tes_subrecord_reader& subrecord) {
      if (subrecord.signature() != signature)
         return load_result::unrecognized;
      subrecord.read(this->navmesh);
      subrecord.read(this->triangle);
      subrecord.read(this->pad06);
      return load_result::succeeded;
   }
   void navmesh_door_portal::save(tes_record_writer& record) {
      auto& subrecord = record.open_next_subrecord(signature);
      subrecord.write(this->navmesh);
      subrecord.write(this->triangle);
      subrecord.write(this->pad06);
      subrecord.close();
   }
}