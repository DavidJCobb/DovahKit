#include "./default_light_emitter_shadow_depth_bias.h"
#include <algorithm> // std::min, std::max
#include <cmath> // std::log1p
#include "../forms/components/extra_data/types/l/light.h"
#include "../forms/components/extra_data/types/r/radius.h"
#include "../forms/Light.h"
#include "../forms/ObjectReference.h"
#include "../form_stub.h"

namespace {
   namespace extra_data_types {
      using namespace dovah::loaded_forms::components::extra_data_types;
   }
}

namespace dovah::utils {
   extern float default_light_emitter_shadow_depth_bias(
      float base_form_radius,
      float ref_radius,
      float ref_spotlight_end_distance_cap
   ) {
      float spherical_end = base_form_radius + ref_radius;
      float spotlight_end = ref_spotlight_end_distance_cap;

      float bias = std::min(spherical_end, spotlight_end);
      bias *=   11.5978F;
      bias -= 2770.84F;
      bias = std::max(bias, 1.0F);
      bias = std::log1p(bias);
      bias *=    8.06318F;
      bias -=   42.7284F;
      return std::max(bias, 2.0F);
   }

   extern float default_light_emitter_shadow_depth_bias(form_stub& ref, bool prefer_working_copy) {
      loaded_form_ptr<loaded_forms::ObjectReference> loaded_refr_ptr;
      loaded_forms::ObjectReference* loaded_refr = nullptr;
      if (prefer_working_copy) {
         loaded_refr = ref.get_working_or_stable_copy(loaded_refr_ptr);
      } else {
         loaded_refr_ptr = ref.load().ptr_cast<loaded_forms::ObjectReference>();
         loaded_refr     = loaded_refr_ptr.unwrap();
      }
      if (!loaded_refr)
         return 0;

      float base_form_radius = 0;
      if (auto* base = loaded_refr->base_form.get_form_stub()) {
         if (base->form_type == dovah::form_type::light) {
            auto loaded_base = base->load().ptr_cast<loaded_forms::Light>();
            if (loaded_base)
               base_form_radius = loaded_base->radius;
         }
      }

      float ref_radius = 0;
      if (auto* extra = loaded_refr->extra_data.get<extra_data_types::radius>())
         ref_radius = extra->value;

      float end_distance = ref_radius;
      if (auto* extra = loaded_refr->extra_data.get<extra_data_types::light>()) {
         end_distance = extra->end_distance_cap;
         if (end_distance == 0)
            end_distance = ref_radius;
      }

      return default_light_emitter_shadow_depth_bias(base_form_radius, ref_radius, end_distance);
   }
}