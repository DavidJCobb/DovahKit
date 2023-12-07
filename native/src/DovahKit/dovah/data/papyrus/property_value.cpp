#include "./property_value.h"
#include "dovah/forms/components/papyrus/property_object_value.h"

namespace dovah::papyrus {
   property_object_value::property_object_value(const loaded_forms::components::papyrus::property_object_value& src) {
      this->form     = src.form.get_form_stub();
      this->alias_id = src.alias_id;
   }
}