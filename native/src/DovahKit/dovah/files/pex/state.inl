#pragma once
#include "./state.h"

namespace dovah::pex {
   template<typename Stream>
   constexpr void state::read(Stream& stream) {
      stream.read(
         name
      );
      stream.read_length_prefixed_vector<2>(functions);
   }
   template<typename Stream>
   /*static*/ constexpr void state::skip(Stream& stream) {
      tabled_string::skip(stream);
      stream.template skip_length_prefixed_vector<2, typename decltype(functions)::value_type>();
   }
}
