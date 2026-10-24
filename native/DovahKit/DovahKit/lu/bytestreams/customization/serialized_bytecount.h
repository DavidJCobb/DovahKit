#pragma once
#include <array>
#include "../../utils/integral_wrapper.h"
#include "../concepts/serializable_scalar.h"

namespace lu::bytestreams {
   #pragma region serialized_bytecount
      template<typename Stream, typename T>
      struct serialized_bytecount {
         // must be defined but empty for the `serialized_bytecount_is_statically_known` concept 
         // to work (i.e. not error) in the empty case
      };

      template<typename Stream, concepts::serializable_scalar<Stream> T>
         requires (utils::integral_wrapper_type<T>)
      struct serialized_bytecount<Stream, T> {
         static constexpr const size_t value = serialized_bytecount<Stream, utils::integral_wrapper_underlying_type<T>>::value;
      };

      template<typename Stream, concepts::serializable_scalar<Stream> T>
         requires (!utils::integral_wrapper_type<T>)
      struct serialized_bytecount<Stream, T> {
         static constexpr const size_t value = sizeof(T);
      };

      #pragma region containers of the above cases
         // std::array
         template<typename Stream, typename T, size_t Size>
            requires requires {
               { serialized_bytecount<Stream, T>::value };
            }
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