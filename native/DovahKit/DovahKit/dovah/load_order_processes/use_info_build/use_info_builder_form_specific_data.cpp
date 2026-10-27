#include "./use_info_builder_form_specific_data.h"
#include "./use_info_builder.h"

namespace dovah::load_order_processes::use_info_build {
   void use_info_builder_form_specific_data::commit(use_info_builder& dst) {
      //
      // Form types:
      //
      if (auto& opt = this->by_form_type.default_object_manager; opt.has_value()) {
         auto& data = opt.value();
         for (auto& pair : data.default_objects) {
            auto id = pair.second;
            if (!id)
               continue;
            dst.add_outbound_use(id);
         }
      }
      if (auto& opt = this->by_form_type.navmesh_info_map; opt.has_value()) {
         auto& data = opt.value();
         /*// TODO: Put this back in place once NAVI is implemented
         for (auto& pair : data.navmesh_info.infos) {
            auto& list = pair.second;
            for (auto id : list)
               dst.add_outbound_reference(id);
         }
         for (auto id : data.precomputed_paths.navmeshes)
            dst.add_outbound_reference(id);
         //*/
         for (auto id : data.deleted_navmeshes)
            dst.add_outbound_use(id);
         /*// TODO: Put this back in place once NAVI is implemented
         for (auto id : data.road_markers.navmeshes)
            dst.add_outbound_reference(id);
         //*/
      }
      //
      // Form structs:
      //
      /*// TODO: Put this back in place once WRLD is implemented
      if (auto& opt = this->by_form_struct.large_ref_index; opt.has_value()) {
         auto& data = opt.value();
         for (auto& pair : data.cells_to_refs) {
            for (auto id : pair.second) {
               if (!id)
                  continue;
               dst.add_outbound_reference(id);
            }
         }
      }
      //*/
      //
      // Done.
      //
   }
   void use_info_builder_form_specific_data::clear() {
      this->by_form_type   = {};
      this->by_form_struct = {};
   }
}