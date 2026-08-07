#include "./wav.h"
#include <xaudio2.h>
#include "../exceptions/invalid_sound_file_data.h"
#include "../utils/is_valid_wav_riff.h"
#include "../utils/parse_riff.h"

namespace dovahkit::subsystems::audio::sound_definitions {
   wav::wav(std::unique_ptr<dovah::bsa_archived_file>&& f) {
      this->_file = std::move(f);
         
      auto& file = *this->_file;
      auto* data = file.data();
      auto  size = file.size();
      if (!utils::is_valid_wav_riff(data, size)) {
         throw exceptions::invalid_sound_file_data{};
      }

      utils::parse_riff(data, size, 'WAVE', [this](uint32_t chunk_type, const void* chunk_data, size_t chunk_size) -> utils::parse_riff_result {
         switch (chunk_type) {
            case 'fmt ':
               if (chunk_size < 14)
                  break;
               this->_format = {};
               if (chunk_size >= 18) {
                  this->_format = *(const impl::wave_format_ex*)chunk_data;
               } else if (chunk_size >= 16) {
                  this->_format = *(const impl::pcm_wave_format*)chunk_data;
               } else if (chunk_size >= 14) {
                  this->_format = *(const impl::wave_format*)chunk_data;
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
         return utils::parse_riff_result::proceed;
      });
   }

   /*virtual*/ const impl::wave_format_ex& wav::get_format() const /*override*/ {
      return this->_format;
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
      return (float)this->_audio.size / this->_format.nAvgBytesPerSec;
   }
   /*virtual*/ size_t wav::estimated_sample_count() const /*override*/ {
      const auto bytes_per_sample = (this->_format.nChannels * this->_format.wBitsPerSample) / 8;
      return this->_audio.size / bytes_per_sample;
   }
}