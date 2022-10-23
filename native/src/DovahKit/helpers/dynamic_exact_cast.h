#pragma once
#include <typeinfo>
#include <type_traits>

namespace cobb {
   namespace impl::_dynamic_exact_cast {
      template<typename Desired, typename Operand> concept cannot_cast_away_constness = (std::is_const_v<Desired> || !std::is_const_v<Operand>);
      template<typename Desired, typename Operand> concept must_be_consistent_about_pointers = (std::is_pointer_v<Desired> == std::is_pointer_v<Operand>);

      template<typename Type> concept cannot_be_bare = (std::is_pointer_v<Type> || std::is_reference_v<Type>);
      template<typename Type> constexpr const bool must_be_polymorphic = [](){
         if constexpr (std::is_pointer_v<Type>) {
            return std::is_polymorphic_v<std::remove_pointer_t<Type>>;
         } else {
            return std::is_polymorphic_v<std::remove_reference_t<Type>>;
         }
      }();
   }

   template<typename Desired, typename Operand> requires (
      impl::_dynamic_exact_cast::cannot_cast_away_constness<Desired, Operand> &&
      impl::_dynamic_exact_cast::must_be_consistent_about_pointers<Desired, Operand> &&
      impl::_dynamic_exact_cast::cannot_be_bare<Desired> &&
      impl::_dynamic_exact_cast::cannot_be_bare<Operand> &&
      impl::_dynamic_exact_cast::must_be_polymorphic<Desired> &&
      impl::_dynamic_exact_cast::must_be_polymorphic<Operand>
   )
   constexpr Desired dynamic_exact_cast(Operand& value) {
      if (typeid(value) == typeid(std::remove_pointer_t<Desired>)) {
         return static_cast<Desired>(value);
      }
      if constexpr (std::is_pointer_v<Desired>) {
         return nullptr;
      } else {
         //
         // References cannot be null.
         //
         throw std::bad_cast{};
      }
   }
}
