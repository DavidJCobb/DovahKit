#include "./wav.h"
#include <xaudio2.h>

namespace {
   static constexpr const tWAVEFORMATEX dummy_format = {
      .wFormatTag = 1,
      .nChannels = 1,
      .nSamplesPerSec = 0,
      .nAvgBytesPerSec = 0,
      .nBlockAlign = 4,
      .wBitsPerSample = 0,
      .cbSize = 0,
   };
}

namespace dovahkit::subsystems::audio::sound_definitions {
   wav::wav(std::unique_ptr<dovah::bsa_archived_file>&& f) {
      this->_file = std::move(f);
         
      auto& file = *this->_file;
      auto* data = file.data();
      auto  size = file.size();
      if (!data_is_likely_wav(data, size)) {
         //
         // TODO: these constructors should be able to throw, instead
         //
         this->_format = std::make_unique<tWAVEFORMATEX>(dummy_format);
         return;
      }

      bool swap_four_cc_endian = false;
      {
         uint32_t signature = *(uint32_t*)data;
         if (signature == std::byteswap('RIFF')) {
            swap_four_cc_endian = true;
         }
      }
      uint32_t riff_size = *(uint32_t*)((const uint8_t*)data + 4);

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
               if (chunk_size < 14)
                  break;
               this->_format = std::make_unique<tWAVEFORMATEX>();
               if (chunk_size >= 18) {
                  *this->_format = *(const WAVEFORMATEX*)chunk_data;
               } else if (chunk_size >= 16) {
                  const auto* src = (const PCMWAVEFORMAT*)chunk_data;
                  *(WAVEFORMAT*)this->_format.get() = src->wf;
                  this->_format->wBitsPerSample = src->wBitsPerSample;
                  this->_format->cbSize         = 0;
               } else if (chunk_size >= 14) {
                  *(WAVEFORMAT*)this->_format.get() = *(const WAVEFORMAT*)chunk_data;
                  this->_format->wBitsPerSample = 8;
                  this->_format->cbSize         = 0;
               } else {
                  break;
               }
               break;
            case 'data':
               this->_audio.size = chunk_size;
               this->_audio.data = chunk_data;
               break;
            default:
               break;
         }
         pos += chunk_size;
      }
   }

   /*static*/ bool wav::data_is_likely_wav(const void* data, size_t size) {
      if (size < 4)
         return false;

      bool swap_four_cc_endian = false;

      //
      //  - FourCC: 'R' 'I' 'F' 'F'
      //
      {
         uint32_t signature = *(uint32_t*)data;
         if (signature == 'RIFF') {
            ;
         } else if (signature == std::byteswap('RIFF')) {
            swap_four_cc_endian = true;
         } else {
            return false;
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
         if (riff_type != 'WAVE')
            return false;
         if (riff_size > size - 8)
            return false;
      }

      return true;
   }

   /*virtual*/ const tWAVEFORMATEX& wav::get_format() const /*override*/ {
      return *this->_format;
   }
   /*virtual*/ XAUDIO2_BUFFER wav::get_audio_buffer_info() const /*override*/ {
      return XAUDIO2_BUFFER{
         .Flags      = XAUDIO2_END_OF_STREAM,
         .AudioBytes = this->_audio.size,
         .pAudioData = (const BYTE*)this->_audio.data,
         .PlayBegin  = 0,
         .PlayLength = 0,
         .LoopBegin  = 0,
         .LoopLength = 0,
         .LoopCount  = 0,
         .pContext   = nullptr
      };
   }
   /*virtual*/ float wav::estimated_length() const /*override*/ {
      return (float)(this->_audio.size * 8) / this->_format->nAvgBytesPerSec * this->_format->nSamplesPerSec;
   }
}