#pragma once
#include "../../concepts/instantiation_of/std_array.h"
#include "../concepts/serializable_scalar.h"
#include "../concepts/serializable_struct.h"
#include "./value_kind.h"

namespace lu::bytestreams::impl {
   #pragma region Implementation details
      template<typename T, typename Stream>
      struct innermost_kind_of_trait {
         static constexpr const value_kind value = value_kind::not_serializable;
      };

      template<typename Stream, concepts::serializable_struct<Stream> T>
      struct innermost_kind_of_trait<T, Stream> {
         static constexpr const value_kind value = value_kind::serializable_struct;
      };

      template<typename Stream, concepts::serializable_scalar<Stream> T>
      struct innermost_kind_of_trait<T, Stream> {
         static constexpr const value_kind value = value_kind::serializable_scalar;
      };

      // Drill down into arrays (but not if e.g. a serializable struct subclasses `std::array`).
      template<typename T, typename Stream>
         requires (lu::concepts::instantiation_of_std_array<T> && !concepts::serializable_struct<T, Stream>)
      struct innermost_kind_of_trait<T, Stream> : public innermost_kind_of_trait<typename T::value_type, Stream> {};
   #pragma endregion

   template<typename T, typename Stream>
   constexpr const value_kind innermost_kind_of = innermost_kind_of_trait<T, Stream>::value;
}
