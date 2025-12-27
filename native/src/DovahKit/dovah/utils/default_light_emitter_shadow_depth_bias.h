#pragma once
namespace dovah {
   class form_stub;
}

namespace dovah::utils {
   extern float default_light_emitter_shadow_depth_bias(
      float base_form_radius,
      float ref_radius, // ExtraRadius
      float ref_spotlight_end_distance_cap // if not a spotlight, use the ref's ExtraRadius
   );

   extern float default_light_emitter_shadow_depth_bias(form_stub& ref, bool prefer_working_copy = false);
}