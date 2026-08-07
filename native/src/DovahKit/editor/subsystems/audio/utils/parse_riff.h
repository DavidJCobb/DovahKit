#pragma once
#include <bit> // std::byteswap
#include <cstdint>
#include <type_traits>

namespace dovahkit::subsystems::audio::utils {
   enum class parse_riff_result {
      proceed,
      stop,
   };

   template<typename ChunkLambda>
      requires std::is_invocable_r_v<parse_riff_result, ChunkLambda, uint32_t, const void*, size_t>
   bool parse_riff(
      const void* data,
      size_t size,
      uint32_t desired_riff_type,
      ChunkLambda&& lambda
   ) {
      if (size < 0xC)
         return false;

      bool swap_four_cc_endian = false;

      //
      //  - FourCC: 'R' 'I' 'F' 'F'
      //
      {
         uint32_t signature = *(uint32_t*)data;
         if (signature == 'RIFF') {
         } else if (signature == std::byteswap('RIFF')) {
            swap_four_cc_endian = true;
         } else {
            return false;
         }
      }

      //
      //  - Size of the chunked data
      //  - FourCC identifying file type
      //
      {
         uint32_t riff_size = *(uint32_t*)((const uint8_t*)data + 4);
         uint32_t riff_type = *(uint32_t*)((const uint8_t*)data + 8);
         if (swap_four_cc_endian)
            riff_type = std::byteswap(riff_type);
         if (riff_type != desired_riff_type)
            return false;
         if (riff_size > size - 8)
            return false;
         size = riff_size + 8; // ignore any extra data at the end of the file
      }

      uint32_t pos = 0xC;
      //
      // Each chunk header consists of a FourCC followed by a chunk body size 
      // (i.e. it does not include the size of the chunk header).
      //
      while (pos + 8 < size) {
         uint32_t chunk_type = *(uint32_t*)((const uint8_t*)data + pos);
         uint32_t chunk_size = *(uint32_t*)((const uint8_t*)data + pos + 4);
         if (swap_four_cc_endian) {
            chunk_type = std::byteswap(chunk_type);
         }
         pos += 8;
         if (pos + chunk_size < pos) { // overflow
            break;
         }

         const auto* chunk_data = ((const uint8_t*)data + pos);
         auto result = lambda(chunk_type, chunk_data, chunk_size);
         if (result == parse_riff_result::stop)
            break;
         pos += chunk_size;
      }
      return true;
   }
}