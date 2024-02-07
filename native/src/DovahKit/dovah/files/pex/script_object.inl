#pragma once
#include "./script_object.h"

namespace dovah::pex {
   template<typename Stream>
   constexpr void script_object::read(Stream& stream) {
      uint32_t size_after_name = 0;
      stream.read(
         name,
         size_after_name,
         superclass,
         docstring,
         user_flags,
         auto_state_name
      );
      stream.read_length_prefixed_vector<2>(variables);
      stream.read_length_prefixed_vector<2>(properties);
      stream.read_length_prefixed_vector<2>(states);
   }
   template<typename Stream>
   /*static*/ constexpr void script_object::skip(Stream& stream) {
      tabled_string::skip(stream);
      stream.skip_bytes(4);
      tabled_string::skip(stream);
      tabled_string::skip(stream);
      stream.skip_bytes(sizeof(user_flags));
      tabled_string::skip(stream);
      stream.template skip_length_prefixed_vector<2, typename decltype(variables)::value_type>();
      stream.template skip_length_prefixed_vector<2, typename decltype(properties)::value_type>();
      stream.template skip_length_prefixed_vector<2, typename decltype(states)::value_type>();
   }
}
