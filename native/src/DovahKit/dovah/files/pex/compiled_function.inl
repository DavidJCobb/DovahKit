#pragma once
#include "./compiled_function.h"

namespace dovah::pex {
   template<typename Stream>
   constexpr void compiled_function::read(Stream& stream) {
      stream.read(
         return_type,
         docstring,
         flags,
         function_flags
      );
      stream.read_length_prefixed_vector<2>(arguments);
      stream.read_length_prefixed_vector<2>(locals);
      stream.read_length_prefixed_vector<2>(opcodes);
   }
   template<typename Stream>
   /*static*/ constexpr void compiled_function::skip(Stream& stream) {
      tabled_string::skip(stream);
      tabled_string::skip(stream);
      stream.skip_bytes(sizeof(flags));
      stream.skip_bytes(sizeof(function_flags));
      stream.template skip_length_prefixed_vector<2, typename decltype(arguments)::value_type>();
      stream.template skip_length_prefixed_vector<2, typename decltype(locals)::value_type>();
      stream.template skip_length_prefixed_vector<2, typename decltype(opcodes)::value_type>();
   }

   template<typename Stream>
   constexpr void compiled_named_function::read(Stream& stream) {
      stream.read(name);
      compiled_function::read(stream);
   }
   template<typename Stream>
   /*static*/ constexpr void compiled_named_function::skip(Stream& stream) {
      tabled_string::skip(stream);
      compiled_function::skip(stream);
   }
}
