#pragma once
#include <array>
#include <cstdint>
#include <string_view>
#include "./parameter_typeinfo.h"

namespace dovah::conditions {
   constexpr const uint16_t no_function_id = 0xFFFF;

   struct function_info {
      uint16_t         id = no_function_id;
      std::string_view name;
      bool             valid           = true;
      bool             sse_only        = false;
      bool             uses_event_data = false;
      std::array<const parameter_typeinfo*, 2> argument_types = { &parameter_types::None, &parameter_types::None };
      
      constexpr function_info() : valid(false) {};
      constexpr function_info(uint16_t id, const char* name)
         :
         id(id),
         name(name)
      {};
      constexpr function_info(uint16_t id, const char* name, const parameter_typeinfo& a)
         :
         id(id),
         name(name),
         argument_types{ &a, &parameter_types::None }
      {};
      constexpr function_info(uint16_t id, const char* name, const parameter_typeinfo& a, const parameter_typeinfo& b)
         :
         id(id),
         name(name),
         argument_types{ &a, &b }
      {};

      static constexpr function_info make_event_data_function(uint16_t id, const char* name) {
         function_info out(id, name);
         out.uses_event_data = true;
         return out;
      }

      constexpr function_info& mark_as_sse_only() noexcept {
         this->sse_only = true;
         return *this;
      }
      
      constexpr size_t argument_count() const noexcept {
         for (int i = this->argument_types.size() - 1; i >= 0; --i) {
            auto* type = this->argument_types[i];
            if (type && type != &parameter_types::None)
               return i + 1;
         }
         return 0;
      }
   };
}
