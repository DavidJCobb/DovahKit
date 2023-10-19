#include "./raycast_requirement.h"
#include "./raycast_result.h"

#include "dovah/form_stub.h"
#include "dovah/form_types.h"

namespace dovahkit::subsystems::worldinput {
   bool raycast_requirement::is_satisfied_by(const raycast_result& res) const {
      if (this->empty())
         return true;

      if (res.target_info.edit_gizmo.mode != gizmo_mode::none) {
         if (this->targets.edit_gizmo_mode != res.target_info.edit_gizmo.mode)
            return false;
         if (this->targets.edit_gizmo_axis != res.target_info.edit_gizmo.axis)
            return false;
         return true;
      }
      if (res.target_info.form) {
         auto ft = res.target_info.form->formType;

         if (ft == dovah::form_type::land) {
            return this->targets.landscapes;
         }
         if (dovah::form_type_info::form_type_is_reference(ft)) {
            switch (this->target_options.selected) {
               using enum optional_yn;
               case unspecified:
                  break;
               case yes:
                  if (res.target_info.is_selected == false)
                     return false;
                  break;
               case no:
                  if (res.target_info.is_selected == true)
                     return false;
                  break;
            }
            return this->targets.object_references;
         }

         return false;
      }
      if (res.empty()) {
         return this->targets.nothing;
      }

      return false;
   }
}