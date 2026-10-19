#pragma once
#include <type_traits>
#include "./can_read_struct.h"
#include "./checked_reads_can_be_implemented_using_unchecked_reads.h"

namespace lu::bytestreams::concepts {
   template<typename T, typename Stream>
   concept unchecked_read_is_noexcept = requires {
      requires can_read_struct<T, Stream>;
      requires []() -> bool {
         if constexpr (checked_reads_can_be_implemented_using_unchecked_reads<T, Stream>) {
            return std::is_nothrow_invocable_v<decltype(&T::unchecked_stream), T&, Stream&>;
         } else {
            return std::is_nothrow_invocable_v<decltype(&T::stream), T&, Stream&>;
         }
      }();
   };
}