#include "./xwma_file_info.h"
#include <bit>
#include <xaudio2.h>
#include "../utils/parse_riff.h"

namespace dovahkit::subsystems::audio::impl {
   xwma_file_info::xwma_file_info(const void* data, size_t size) {
      bool is_xwma_riff = utils::parse_riff(
         data,
         size,
         'XWMA',
         [this](uint32_t chunk_type, const void* chunk_data, size_t chunk_size) -> utils::parse_riff_result {
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
            return utils::parse_riff_result::proceed;
         }
      );
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
   float xwma_file_info::estimated_length() const {
      if (!this->valid())
         return 0;
      if (this->dpds.data && this->dpds.size) {
         const auto bytes_per_sample = (this->format->nChannels * this->format->wBitsPerSample) / 8;
         const auto bytecount        = this->dpds.data[(this->dpds.size / 4) - 1];
         return ((float)bytecount / bytes_per_sample) / this->format->nSamplesPerSec;
      }
      return (float)this->audio.size / this->format->nAvgBytesPerSec;
   }
}