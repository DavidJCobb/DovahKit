#pragma once
#include "./impl/modifier_wrapper.h"

namespace lu::bytestreams {
   template<typename T>
      requires (!impl::is_modifier_wrapper<T>)
   constexpr T& unwrap_modifiers(T& v) {
      return v;
   }

   template<impl::is_modifier_wrapper T>
   constexpr auto& unwrap_modifiers(T v) {
      return unwrap_modifiers(v.get());
   }
}