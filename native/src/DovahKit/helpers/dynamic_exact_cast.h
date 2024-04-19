#pragma once
#include <typeinfo>
#include <type_traits>

namespace cobb {
   namespace impl::_dynamic_exact_cast {
      template<typename Desired, typename Operand> concept cannot_cast_away_constness = (std::is_const_v<Desired> || !std::is_const_v<Operand>);
   }

   template<typename Desired, typename Operand> requires (
      !std::is_pointer_v<Operand> &&
      !std::is_pointer_v<Desired> &&
      std::is_polymorphic_v<Operand> &&
      std::is_polymorphic_v<Desired> &&
      impl::_dynamic_exact_cast::cannot_cast_away_constness<Desired, Operand>
   )
   constexpr Desired& dynamic_exact_cast(Operand& value) {
      if (typeid(value) == typeid(std::remove_pointer_t<Desired>)) {
         return static_cast<Desired&>(value);
      }
      throw std::bad_cast{}; // references cannot be null
   }

   template<typename Desired, typename Operand> requires (
      std::is_pointer_v<Desired> &&
      std::is_polymorphic_v<Operand> &&
      std::is_polymorphic_v<std::remove_pointer_t<Desired>> &&
      impl::_dynamic_exact_cast::cannot_cast_away_constness<Desired, Operand>
   )
   constexpr Desired dynamic_exact_cast(Operand* value) {
      if (typeid(*value) == typeid(std::remove_pointer_t<Desired>)) {
         return static_cast<Desired>(value);
      }
      return nullptr;
   }
}
