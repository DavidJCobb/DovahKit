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
   //
   /*static*/ void navmesh_door_portal::generate_use_info(tes_record_reader& record, form_stub* stub) {
      auto& subrecord = record.get_current_subrecord();
      form_id_t formID;
      if (subrecord.read(formID) && formID)
         stub->add_outbound_reference(formID);
   }
   basic_extra_data* navmesh_door_portal::clone(form_stub& clone_owner) const noexcept {
      auto* clone = new navmesh_door_portal;
      clone->navmesh.set(&clone_owner, this->navmesh);
      clone->triangle = this->triangle;
      clone->pad06    = this->pad06;
      return clone;
   }
   void navmesh_door_portal::sever_outbound_references_to(form_stub& target, form_stub& my_owner) {
      if (this->navmesh == target.formID)
         this->navmesh.set(&my_owner, nullptr);
   }
}