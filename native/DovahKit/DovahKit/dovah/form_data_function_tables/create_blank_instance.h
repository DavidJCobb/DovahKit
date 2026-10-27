#pragma once
#include <cstdint>
namespace dovah {
   class base_managed_form_data;
   class form_stub;
   enum class form_type : uint8_t;
}

namespace dovah::form_data_function_tables::create_blank_instance {
   using function_type = base_managed_form_data&(*)(form_stub&);

   extern function_type lookup(form_type);

   template<form_type V>
   struct entry;
}