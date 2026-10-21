#pragma once
#include <bit>
#include <type_traits>
#include "lu/bytestreams/base_bytereader.h"
#include "lu/integral_character_code.h"
#include "lu/bytestreams/customization/type_has_opposite_endianness.h"

namespace dovah::bsa {
   class bytereader : public lu::bytestreams::base_bytereader {
   };
}

namespace lu::bytestreams {
   template<typename Reader>
      requires std::is_base_of_v<dovah::bsa::bytereader, Reader>
   struct type_has_opposite_endianness<Reader, lu::four_cc> :
      public std::bool_constant<(std::endian::native != std::endian::big)>
   {};
}