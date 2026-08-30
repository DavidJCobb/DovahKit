#include "./light_radius.h"
#include <limits>
#include "../../form_stubs/helpers/get_base_form.h"
#include "../../form_stub.h"
#include "../../forms/components/extra_data/types/r/radius.h"
#include "../../forms/Light.h"
#include "../../forms/ObjectReference.h"

namespace {
   constexpr const auto no_radius = std::numeric_limits<float>::quiet_NaN();

   using extra_radius = dovah::loaded_forms::components::extra_data_types::radius;
}

namespace dovah::utils::refs {
   extern float get_light_radius(form_stub& ref) {
      if (!form_type_is_reference(ref.form_type))
         return no_radius;
      auto loaded_ref = ref.load().ptr_cast<loaded_forms::ObjectReference>();
      if (loaded_ref)
         return get_light_radius(*loaded_ref);
      return no_radius;
   }
   extern float get_light_radius(const loaded_forms::ObjectReference& loaded_ref) {
      float radius = no_radius;
      {
         auto* base = loaded_ref.base_form.get_form_stub();
         if (!base || base->form_type != form_type::light)
            return no_radius;
         auto loaded_base = base->load().ptr_cast<loaded_forms::Light>();
         if (!loaded_base)
            return no_radius;
         radius = loaded_base->radius;
      }
      if (auto* extra = loaded_ref.extra_data.get<extra_radius>()) {
         radius += extra->value;
      }
      return radius;
   }

   extern void set_light_radius(form_stub& ref, float radius, bool delete_no_op_extra_data) {
      if (!form_type_is_reference(ref.form_type))
         return;
      auto loaded_ref = ref.load().ptr_cast<loaded_forms::ObjectReference>();
      if (loaded_ref) {
         set_light_radius(*loaded_ref, radius, delete_no_op_extra_data);
         ref.set_edited(true);
      }
   }
   extern void set_light_radius(loaded_forms::ObjectReference& loaded_ref, float radius, bool delete_no_op_extra_data) {
      constexpr const float epsilon = 0.00001F;

      float radius_mod;
      {
         auto* base = loaded_ref.base_form.get_form_stub();
         if (!base || base->form_type != dovah::form_type::light)
            return;
         auto loaded_base = base->load().ptr_cast<loaded_forms::Light>();
         if (!loaded_base)
            return;
         radius_mod = radius - loaded_base->radius;
      }
      if (radius_mod > -epsilon && radius_mod < epsilon) {
         if (delete_no_op_extra_data) {
            loaded_ref.extra_data.remove<extra_radius>(loaded_ref);
            return;
         }
         radius_mod = 0.0F;
      }
      loaded_ref.extra_data.get_or_create<extra_radius>()->value = radius_mod;
   }
}