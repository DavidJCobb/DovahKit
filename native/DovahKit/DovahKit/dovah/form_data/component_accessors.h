#pragma once
#include <cstdint>
namespace dovah {
   namespace form_data::components {
      class bounds;
   }
   class base_managed_form_data;
   class base_unmanaged_form_data;
   enum class form_type : uint8_t;
}

namespace dovah::form_data::component_accessors {
   extern const components::bounds* bounds(const base_managed_form_data&);
   extern components::bounds* bounds(base_managed_form_data&);
   extern const components::bounds* bounds(const base_unmanaged_form_data&);
   extern components::bounds* bounds(base_unmanaged_form_data&);
}