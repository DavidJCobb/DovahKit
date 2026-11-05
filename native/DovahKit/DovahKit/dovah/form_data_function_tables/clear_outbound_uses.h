#pragma once
#include <cstdint>
namespace dovah {
   class base_managed_form_data;
   enum class form_type : uint8_t;
}

namespace dovah::form_data_function_tables::clear_outbound_uses {
   using function_type = void(*)(base_managed_form_data&);

   extern function_type lookup(form_type);
}