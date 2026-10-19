#pragma once
#include <type_traits>

namespace lu::bytestreams {
   template<typename Reader, typename ValueType>
   struct type_has_opposite_endianness : public std::false_type {};
}