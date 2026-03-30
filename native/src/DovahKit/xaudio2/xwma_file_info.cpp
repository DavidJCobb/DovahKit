#include "./xwma_file_info.h"
#include <bit>
#include <xaudio2.h>

namespace dovahkit::xaudio2 {
   xwma_file_info::xwma_file_info(const void* data, size_t size) {
      bool swap_four_cc_endian = false;

      //
      //  - FourCC: 'R' 'I' 'F' 'F'
      //
      if (size < 4)
         return;
      else {
         uint32_t signature = *(uint32_t*)data;
         if (signature == 'RIFF') {
         } else if (signature == std::byteswap('RIFF')) {
            swap_four_cc_endian = true;
         } else {
            return;
         }
      }

      //
      //  - Size of the chunked data
      //  - FourCC: 'X' 'W' 'M' 'A'
      //
      {
         uint32_t riff_size = *(uint32_t*)((const uint8_t*)data + 4);
         uint32_t riff_type = *(uint32_t*)((const uint8_t*)data + 8);
         if (swap_four_cc_endian)
            riff_type = std::byteswap(riff_type);
         if (riff_type != 'XWMA')
            return;
         if (riff_size > size - 8)
            return;
         size = riff_size + 8;
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
         switch (chunk_type) {
            case 'fmt ':
               if (chunk_size < 18)
                  break;
               this->format = (const WAVEFORMATEX*)chunk_data;
               break;
            case 'dpds':
               if (chunk_size % 4)
                  break;
               this->dpds.size = chunk_size;
               this->dpds.data = (const uint32_t*)chunk_data;
               break;
            case 'data':
               this->audio.size = chunk_size;
               this->audio.data = chunk_data;
               break;
            default:
               break;
         }
         pos += chunk_size;
      }
   }

   XAUDIO2_BUFFER xwma_file_info::describe_buffer() const {
      return XAUDIO2_BUFFER{
         .Flags      = XAUDIO2_END_OF_STREAM,
         .AudioBytes = this->audio.size,
         .pAudioData = (const BYTE*)this->audio.data,
         .PlayBegin  = 0,
         .PlayLength = 0,
         .LoopBegin  = 0,
         .LoopLength = 0,
         .LoopCount  = 0,
         .pContext   = nullptr
      };
   }
   XAUDIO2_BUFFER_WMA xwma_file_info::describe_wma() const {
      return XAUDIO2_BUFFER_WMA{
         .pDecodedPacketCumulativeBytes = this->dpds.data,
         .PacketCount                   = this->dpds.size / 4,
      };
   }
}