#pragma once
#include <functional>
#include <tuple>
#include "../../tuples/contains_type.h"
#include "../../tuples/prepend_types.h"
#include "../../tuples/unpack_types_into_template.h"

namespace lu::bytestreams::impl {
   template<typename V, typename... Modifiers>
   class modifier_wrapper : public std::reference_wrapper<V> {
      public:
         using modifiers = std::tuple<Modifiers...>;
         using type      = std::reference_wrapper<V>::type;
   };

   template<typename T>
   concept is_modifier_wrapper = requires {
      typename T::modifiers;
      typename T::type;
      requires std::is_same_v<
         T,
         tuples::unpack_types_into_template<
            tuples::prepend_types<typename T::modifiers, typename T::type>,
            modifier_wrapper
         >
      >;
   };

   template<typename W, typename Modifier>
   concept has_modifier = requires {
      requires is_modifier_wrapper<W>;
      requires tuples::contains_type<typename W::modifiers, Modifier>;
   };
}