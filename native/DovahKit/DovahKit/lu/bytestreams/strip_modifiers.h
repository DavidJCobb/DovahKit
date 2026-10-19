#pragma once
#include "./impl/modifier_wrapper.h"

namespace lu::bytestreams {
   namespace impl {
      template<typename T>
      struct strip_modifiers_template {
         using type = T;
      };

      template<is_modifier_wrapper T>
      struct strip_modifiers_template<T> {
         using type = strip_modifiers_template<typename T::type>::type;
      };
   }

   template<typename T>
   using strip_modifiers = impl::strip_modifiers_template<T>::type;
}