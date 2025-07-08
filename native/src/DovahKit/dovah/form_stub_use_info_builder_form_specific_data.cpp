#include "./form_stub_use_info_builder_form_specific_data.h"
#include "./form_stub_use_info_builder.h"
#include "./forms/structs/navmesh_info_map/navmesh_info.h"

namespace dovah {
   void form_stub_use_info_builder_form_specific_data::commit(form_stub_use_info_builder& dst) {
      //
      // Form types:
      //
      if (auto& opt = this->by_form_type.default_object_manager; opt.has_value()) {
         auto& data = opt.value();
         for (auto& pair : data.default_objects) {
            auto id = pair.second;
            if (!id)
               continue;
            dst.add_outbound_reference(id);
         }
      }
      if (auto& opt = this->by_form_type.navmesh_info_map; opt.has_value()) {
         auto& data = opt.value();
         for (auto& pair : data.navmesh_info.infos) {
            auto& list = pair.second;
            for (auto id : list)
               dst.add_outbound_reference(id);
         }
         for (auto id : data.precomputed_paths.navmeshes)
            dst.add_outbound_reference(id);
      }
      //
      // Form structs:
      //
      if (auto& opt = this->by_form_struct.world_large_ref_data; opt.has_value()) {
         auto& data = opt.value();
         for (auto& pair : data.cells_to_refs) {
            for (auto id : pair.second) {
               if (!id)
                  continue;
               dst.add_outbound_reference(id);
            }
         }
      }
      // Done.
   }
   void form_stub_use_info_builder_form_specific_data::clear() {
      this->by_form_type   = {};
      this->by_form_struct = {};
   }
}