#pragma once
#include <cstdint>
#include "./exceptions/missing_data_header.h"

namespace cobb::bitstreams {
   class reader;
   class writer;

   struct data_header {
      static constexpr const uint32_t current_version = 0;

      struct {
         uint32_t bitstream = current_version;
         uint32_t data      = 0;
      } versions;

      // templates without includes, so we can avoid things breaking on circular includes
      template<typename Stream> requires std::is_same_v<Stream, reader>
      constexpr void stream(Stream& s) {
         if (s.bytes_remaining() < 8) {
            throw exceptions::missing_data_header{ s.get_position() };
         }
         s.unchecked_stream(this->versions.bitstream);
         s.unchecked_stream(this->versions.data);
      }
      //
      template<typename Stream> requires std::is_same_v<Stream, writer>
      constexpr void stream(Stream& s) const {
         s.stream(this->versions.bitstream);
         s.stream(this->versions.data);
      }
   };
}