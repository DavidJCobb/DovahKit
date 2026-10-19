#pragma once
#include <type_traits>

namespace lu {
   namespace impl {
      template<typename T>
      struct _enum_type_to_underlying {
         using type = T;
      };

      template<typename T>
         requires std::is_enum_v<T>
      struct _enum_type_to_underlying<T> {
         using type = std::underlying_type_t<T>;
      };
   }

   template<typename T>
   using enum_type_to_underlying = impl::_enum_type_to_underlying<T>::type;
}