#pragma once
#include "./function_debug_info.h"

namespace dovah::pex {
   template<typename Stream>
   constexpr void function_debug_info::read(Stream& stream) {
      stream.read(
         object,
         state,
         function_name,
         type
      );
      stream.read_length_prefixed_vector<2>(line_numbers);
   }
   template<typename Stream>
   /*static*/ constexpr void function_debug_info::skip(Stream& stream) {
      tabled_string::skip(stream);
      tabled_string::skip(stream);
      tabled_string::skip(stream);
      stream.skip_bytes(1);
      stream.template skip_length_prefixed_vector<2, typename decltype(line_numbers)::value_type>();
   }
}
