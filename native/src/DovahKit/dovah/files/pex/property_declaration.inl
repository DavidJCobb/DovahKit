#pragma once
#include "./property_declaration.h"

namespace dovah::pex {
   template<typename Stream>
   constexpr void property_declaration::read(Stream& stream) {
      stream.read(
         name,
         type,
         docstring,
         flags,
         property_flags
      );
      if (property_flags & 4)
         stream.read(autovar_name);
      else {
         if (property_flags & 1)
            stream.read(getter);
         if (property_flags & 2)
            stream.read(setter);
      }
   }
   template<typename Stream>
   /*static*/ constexpr void property_declaration::skip(Stream& stream) {
      tabled_string::skip(stream);
      tabled_string::skip(stream);
      tabled_string::skip(stream);
      stream.skip_bytes(sizeof(flags));

      decltype(property_flags) f = 0;
      stream.read(f);
      if (f & 4)
         tabled_string::skip(stream);
      else {
         if (f & 1)
            compiled_function::skip(stream);
         if (f & 2)
            compiled_function::skip(stream);
      }
   }
}
