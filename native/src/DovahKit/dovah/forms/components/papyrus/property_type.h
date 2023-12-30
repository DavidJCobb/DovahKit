#pragma once
#include <cstdint>
#include <utility> // std::unreachable

namespace dovah::loaded_forms::components::papyrus {
   enum class property_type : uint8_t {
      none = 0,
      //
      object  = 1, // i.e. Form or any derived type
      string  = 2,
      integer = 3,
      float32 = 4,
      boolean = 5,
      //
      array_of_object  = 11,
      array_of_string  = 12,
      array_of_integer = 13,
      array_of_float32 = 14,
      array_of_boolean = 15,
   };

   constexpr bool property_type_is_array(property_type t) {
      switch (t) {
         using enum property_type;
         case array_of_object:
         case array_of_string:
         case array_of_integer:
         case array_of_float32:
         case array_of_boolean:
            return true;
      }
      return false;
   }
   constexpr property_type scalar_property_type_for(property_type t) {
      if (!property_type_is_array(t))
         return t;
      switch (t) {
         using enum property_type;
         case array_of_object:  return object;
         case array_of_string:  return string;
         case array_of_integer: return integer;
         case array_of_float32: return float32;
         case array_of_boolean: return boolean;
      }
      std::unreachable();
   }
   constexpr property_type array_property_type_for(property_type t) {
      if (property_type_is_array(t))
         return t;
      switch (t) {
         using enum property_type;
         case object:  return array_of_object;
         case string:  return array_of_string;
         case integer: return array_of_integer;
         case float32: return array_of_float32;
         case boolean: return array_of_boolean;
      }
      std::unreachable();
   }
}