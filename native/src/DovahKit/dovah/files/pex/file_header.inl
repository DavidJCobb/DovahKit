#pragma once
#include "./file_header.h"

namespace dovah::pex {
   template<typename Stream>
   constexpr void file_header::read(Stream& stream) {
      stream.read(
         version.major,
         version.minor,
         game,
         compile_time
      );
      stream.read_length_prefixed_string<2>(source_file);
      stream.read_length_prefixed_string<2>(author.username);
      stream.read_length_prefixed_string<2>(author.computer);
   }
   template<typename Stream>
   /*static*/ constexpr void file_header::skip(Stream& stream) {
      stream.skip_bytes(
         sizeof(decltype(version)::major) +
         sizeof(decltype(version)::minor) +
         sizeof(game) +
         sizeof(compile_time)
      );
      stream.template skip_length_prefixed_string<2>(); // source file
      stream.template skip_length_prefixed_string<2>(); // author username
      stream.template skip_length_prefixed_string<2>(); // author computer
   }
}
