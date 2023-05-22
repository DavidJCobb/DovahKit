#pragma once
#include <cstdint>
#include "./exceptions/missing_data_header.h"

namespace cobb::bitstreams {
   class reader;
   class writer;

   struct data_header {
      static constexpr const uint32_t current_version = 0;

      uint32_t version = current_version;

      // templates without includes, so we can avoid things breaking on circular includes
      template<typename Stream> requires std::is_same_v<Stream, reader>
      constexpr void stream(Stream& s) {
         if (s.bytes_remaining() < 4) {
            throw exceptions::missing_data_header{ s.get_position() };
         }
         s.unchecked_stream(this->version);
      }
      //
      template<typename Stream> requires std::is_same_v<Stream, writer>
      constexpr void stream(Stream& s) const {
         s.stream(this->version);
      }
   };
}