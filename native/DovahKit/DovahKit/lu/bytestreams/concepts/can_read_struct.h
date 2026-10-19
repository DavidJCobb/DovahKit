#pragma once
#include <concepts>
#include "./checked_reads_can_be_implemented_using_unchecked_reads.h"

namespace lu::bytestreams::concepts {
   template<typename T, typename Stream>
   concept can_read_struct =
      checked_reads_can_be_implemented_using_unchecked_reads<T, Stream> ||
      requires(Stream& reader, T& dst) {
         { dst.stream(reader) } -> std::convertible_to<bool>;
      }
   ;
}
