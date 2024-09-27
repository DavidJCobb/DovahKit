#include "./form_stub_use_info_builder_form_specific_data.h"
#include "./form_stub_use_info_builder.h"

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