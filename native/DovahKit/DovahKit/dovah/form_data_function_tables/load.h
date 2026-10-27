#pragma once
#include <cstdint>
namespace dovah {
   namespace load_order_interfaces {
      class form_load;
   }
   namespace tes {
      class record_view;
   }
   class base_managed_form_data;
   enum class form_type : uint8_t;
}

namespace dovah::form_data_function_tables::load {
   using function_type = void(*)(base_managed_form_data&, tes::record_view&, load_order_interfaces::form_load&);

   extern function_type lookup(form_type);
}