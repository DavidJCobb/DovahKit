#pragma once
#include <cstdint>
#include <string>
#include <variant>
#include <vector>
#include "./property_object_value.h"
#include "./property_type.h"

namespace dovah::loaded_forms::components::papyrus {
   using property_value = std::variant<
      property_object_value,
      std::string,
      int32_t,
      float,
      bool,
      //
      std::vector<property_object_value>,
      std::vector<std::string>,
      std::vector<int32_t>,
      std::vector<float>,
      std::vector<bool>
   >;

   extern property_value property_value_from_type(property_type);

   constexpr property_type property_type_for(const property_value& v) {
      if (std::holds_alternative<property_object_value>(v))
         return property_type::object;
      if (std::holds_alternative<std::string>(v))
         return property_type::string;
      if (std::holds_alternative<int32_t>(v))
         return property_type::integer;
      if (std::holds_alternative<float>(v))
         return property_type::float32;
      if (std::holds_alternative<bool>(v))
         return property_type::boolean;

      if (std::holds_alternative<std::vector<property_object_value>>(v))
         return property_type::array_of_object;
      if (std::holds_alternative<std::vector<std::string>>(v))
         return property_type::array_of_string;
      if (std::holds_alternative<std::vector<int32_t>>(v))
         return property_type::array_of_integer;
      if (std::holds_alternative<std::vector<float>>(v))
         return property_type::array_of_float32;
      if (std::holds_alternative<std::vector<bool>>(v))
         return property_type::array_of_boolean;

      std::unreachable();
   }
}