#pragma once
#include <cstdint>

namespace dovah::conditions {
   struct event_function {
      event_function() = delete;
      enum type : uint16_t {
         GetIsID,
         IsInList,
         GetValue,
         HasKeyword,
         GetItemValue,
      };
   };

   constexpr bool event_function_uses_form(uint16_t id) noexcept {
      switch (id) {
         case event_function::GetIsID:
         case event_function::IsInList:
         case event_function::HasKeyword:
            return true;
      }
      return false;
   }
}