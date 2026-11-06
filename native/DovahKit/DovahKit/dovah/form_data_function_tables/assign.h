#pragma once
#include <cstdint>
namespace dovah {
   class base_managed_form_data;
   enum class form_type : uint8_t;
}

namespace dovah::form_data_function_tables::assign {
   namespace managed_to_managed {
      using function_type = void(*)(base_managed_form_data&, const base_managed_form_data&);

      extern function_type lookup(form_type);
   }
   
   // TODO: managed_to_unmanaged
   //       (as of 11/5/2026, blocked by needing to make `assign` support unmanaged l-string uses)

   // TODO: unmanaged_to_managed
}