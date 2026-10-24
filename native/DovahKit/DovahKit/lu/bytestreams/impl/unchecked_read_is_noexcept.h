#pragma once
#include "./innermost_kind_of.h"
#include "../concepts/checked_reads_can_be_implemented_using_unchecked_reads.h"
#include "../concepts/serializable_struct.h"

namespace lu::bytestreams::impl {
   template<typename T, typename Stream>
   struct unchecked_read_is_noexcept_trait;

   template<typename T, typename Stream>
      requires (innermost_kind_of<T, Stream> == value_kind::serializable_scalar)
   struct unchecked_read_is_noexcept_trait<T, Stream> : public std::true_type {};

   template<typename T, typename Stream>
      requires (
         innermost_kind_of<T, Stream> == value_kind::serializable_struct
      && (!lu::concepts::instantiation_of_std_array<T> || concepts::serializable_struct<T, Stream>)
      )
   struct unchecked_read_is_noexcept_trait<T, Stream> {
      static constexpr const bool value = requires {
         requires concepts::serializable_struct<T, Stream>;
         requires []() -> bool {
            if constexpr (concepts::checked_reads_can_be_implemented_using_unchecked_reads<T, Stream>) {
               return std::is_nothrow_invocable_v<decltype(&T::unchecked_stream), T&, Stream&>;
            } else {
               return std::is_nothrow_invocable_v<decltype(&T::stream), T&, Stream&>;
            }
         }();
      };
   };

   template<typename T, typename Stream>
      requires (
         innermost_kind_of<T, Stream> == value_kind::serializable_struct
      && (lu::concepts::instantiation_of_std_array<T> && !concepts::serializable_struct<T, Stream>)
      )
   struct unchecked_read_is_noexcept_trait<T, Stream> : public unchecked_read_is_noexcept_trait<typename T::value_type, Stream> {};

   template<typename T, typename Stream>
   concept unchecked_read_is_noexcept = unchecked_read_is_noexcept_trait<T, Stream>::value;
}
