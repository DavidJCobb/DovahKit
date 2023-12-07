#include "./property_value.h"
#include <stdexcept>

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

   extern property_value property_value_from_type(property_type v) {
      switch (v) {
         case property_type::object:  return property_object_value{};
         case property_type::string:  return std::string{};
         case property_type::integer: return int32_t{0};
         case property_type::float32: return float{0};
         case property_type::boolean: return false;

         case property_type::array_of_object:  return std::vector<property_object_value>{};
         case property_type::array_of_string:  return std::vector<std::string>{};
         case property_type::array_of_integer: return std::vector<int32_t>{};
         case property_type::array_of_float32: return std::vector<float>{};
         case property_type::array_of_boolean: return std::vector<bool>{};
      }
      throw std::runtime_error("invalid Papyrus property type; cannot instantiate a value");
   }
}