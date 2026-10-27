#pragma once
#include <cstdint>
namespace dovah {
   enum class form_type : uint8_t;
}

namespace dovah {
   template<typename T>
   struct form_type_for_form_data_class {
      static constexpr const form_type value = {};
   };

   template<typename T>
   constexpr const form_type form_type_for_form_data_class_v = form_type_for_form_data_class<T>::value;
}
