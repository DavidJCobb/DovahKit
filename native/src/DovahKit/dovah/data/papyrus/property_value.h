#pragma once
#include <cstdint>
#include <string>
#include <variant>
#include <vector>

namespace dovah {
   class form_stub;
   namespace loaded_forms::components::papyrus {
      struct property_object_value;
   }
}

namespace dovah::papyrus {
   struct property_object_value {
      public:
         static constexpr const uint16_t no_alias = 0xFFFF;

      public:
         constexpr property_object_value() {}
         property_object_value(const loaded_forms::components::papyrus::property_object_value&);

         form_stub* form     = nullptr;
         uint16_t   alias_id = no_alias;
   };

   using property_value = std::variant<
      std::monostate,
      //
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
}