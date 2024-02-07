#pragma once
#include "./global_variable_declaration.h"

namespace dovah::pex {
   template<typename Stream>
   constexpr void global_variable_declaration::read(Stream& stream) {
      stream.read(
         name,
         type,
         flags,
         initial_value
      );
   }
   template<typename Stream>
   /*static*/ constexpr void global_variable_declaration::skip(Stream& stream) {
      tabled_string::skip(stream);
      tabled_string::skip(stream);
      stream.skip_bytes(sizeof(flags));
      value::skip(stream);
   }
}
