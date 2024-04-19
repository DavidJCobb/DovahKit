#pragma once
#include <type_traits>
#include "./dynamic_exact_cast.h"

namespace cobb {
   template<typename Desired, typename Operand> requires (!std::is_pointer_v<Operand>)
   constexpr Desired dynamic_fast_cast(Operand& value) {
      if constexpr (std::is_final_v<std::remove_pointer_t<std::remove_reference_t<Operand>>>) {
         return cobb::dynamic_exact_cast<Desired>(value);
      } else {
         return dynamic_cast<Desired>(value);
      }
   }
   
   template<typename Desired, typename Operand> requires (std::is_pointer_v<Desired>)
   constexpr Desired dynamic_fast_cast(Operand* value) {
      if constexpr (std::is_final_v<Operand>) {
         return cobb::dynamic_exact_cast<Desired>(value);
      } else {
         return dynamic_cast<Desired>(value);
      }
   }
}
