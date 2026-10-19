#pragma once
#include <array>
#include <type_traits>
#include "../../utils/integral_wrapper.h"
#include "../concepts/serializable_scalar.h"

namespace lu::bytestreams {
   #pragma region serialized_bytecount
      template<typename Stream, typename T>
      struct serialized_bytecount;

      template<typename Stream, typename T>
         requires (concepts::serializable_scalar<T> && !utils::integral_wrapper_type<T>)
      struct serialized_bytecount<Stream, T> {
         static constexpr const size_t value = sizeof(T);
      };

      template<typename Stream, utils::integral_wrapper_type T>
      struct serialized_bytecount<Stream, T> : public serialized_bytecount<Stream, utils::integral_wrapper_underlying_type<T>> {};

      #pragma region containers of the above cases
         template<typename Stream, typename T, size_t Size>
         struct serialized_bytecount<Stream, std::array<T, Size>> {
            static constexpr const size_t value = serialized_bytecount<Stream, T>::value * Size;
         };
      #pragma endregion
   #pragma endregion

   template<typename Stream, typename T>
   concept serialized_bytecount_is_statically_known = requires {
      { serialized_bytecount<Stream, T>::value };
   };
}