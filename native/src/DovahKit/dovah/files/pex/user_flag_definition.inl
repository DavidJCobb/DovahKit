#pragma once
#include "./user_flag_definition.h"

namespace dovah::pex {
   template<typename Stream>
   constexpr void user_flag_definition::read(Stream& stream) {
      stream.read(
         name,
         bit_index
      );
   }
   template<typename Stream>
   /*static*/ constexpr void user_flag_definition::skip(Stream& stream) {
      tabled_string::skip(stream);
      stream.skip_bytes(1);
   }
}
