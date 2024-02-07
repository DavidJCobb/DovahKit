#pragma once
#include "./variable_declaration.h"

namespace dovah::pex {
   template<typename Stream>
   constexpr void variable_declaration::read(Stream& stream) {
      stream.read(
         name,
         type
      );
   }
   template<typename Stream>
   /*static*/ constexpr void variable_declaration::skip(Stream& stream) {
      tabled_string::skip(stream);
      tabled_string::skip(stream);
   }
}
