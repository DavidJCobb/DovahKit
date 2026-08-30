#pragma once
namespace dovah {
   namespace loaded_forms {
      class ObjectReference;
   }
   class form_stub;
}

namespace dovah::utils::refs {
   // Returns NaN on failure.
   extern float get_light_radius(form_stub&);
   extern float get_light_radius(const loaded_forms::ObjectReference&);

   extern void set_light_radius(form_stub&, float, bool delete_no_op_extra_data = true);
   extern void set_light_radius(loaded_forms::ObjectReference&, float, bool delete_no_op_extra_data = true);
}