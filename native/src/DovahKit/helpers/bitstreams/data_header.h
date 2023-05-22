#pragma once
#include <cstdint>
#include "./exceptions/missing_data_header.h"
#include "./bitstream.h"

namespace cobb::bitstreams {
   struct data_header {
      static constexpr const uint32_t current_version = 0;

      uint32_t version = current_version;

      // templates without includes, so we can avoid things breaking on circular includes
      template<bitstream Stream> requires std::is_same_v<Stream, reader>
      constexpr void stream(Stream& s) {
         if (s.bytes_remaining() < 4) {
            throw exceptions::missing_data_header{};
         }
         s.unchecked_stream(this->version);
      }
      //
      template<bitstream Stream> requires std::is_same_v<Stream, writer>
      constexpr void stream(Stream& s) const {
         s.unchecked_stream(this->version);
      }
   };
}