#pragma once
#include <cstdint>
namespace dovah {
   class base_managed_form_data;
   class form_stub;
   enum class form_type : uint8_t;
}

namespace dovah::form_data::function_tables::initialize {
   using function_type = void(*)(base_managed_form_data&);

   extern function_type lookup(form_type);

   //
   // If newly-created forms of some type need fields initialized based on run-time state, 
   // such as the values of GMSTs or Skyrim INI settings, then specialize this type and 
   // define a static `execute` function which takes a managed T& as its argument.
   //
   template<form_type V>
   struct entry;
}