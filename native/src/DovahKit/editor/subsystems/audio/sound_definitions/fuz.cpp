#include "./fuz.h"
#include <xaudio2.h>
#include "../exceptions/invalid_sound_file_data.h"
#include "../impl/wave_format_ex.h"
#include "../utils/is_valid_fuz_file.h"
#include "../utils/is_valid_xwma_riff.h"

namespace dovahkit::subsystems::audio::sound_definitions {
   fuz::fuz(std::unique_ptr<dovah::bsa_archived_file>&& f) {
      this->_file = std::move(f);
         
      auto& file = *this->_file;
      auto* data = file.data();
      auto  size = file.size();

      this->_fuz_info = dovah::fuz::file_info{ data, size };
      if (!this->_fuz_info.buffer.data) {
         throw exceptions::invalid_sound_file_data{};
      }

      if (size > this->_fuz_info.buffer.size + dovah::fuz::header_size) {
         auto*    riff_data = (const void*)((const uint8_t*)data + dovah::fuz::header_size + this->_fuz_info.buffer.size);
         uint32_t riff_size = size - this->_fuz_info.buffer.size - dovah::fuz::header_size;
         if (!utils::is_valid_xwma_riff(riff_data, riff_size)) {
            throw exceptions::invalid_sound_file_data{};
         }
         this->_xwma_info = impl::xwma_file_info{ riff_data, riff_size };
      }
   }

   /*virtual*/ const impl::wave_format_ex& fuz::get_format() const /*override*/ {
      return *(const impl::wave_format_ex*)this->_xwma_info.format;
   }
   /*virtual*/ XAUDIO2_BUFFER fuz::get_audio_buffer_info() const /*override*/ {
      return this->_xwma_info.describe_buffer();
   }
   /*virtual*/ XAUDIO2_BUFFER_WMA fuz::get_xwma_info() const /*override*/ {
      return this->_xwma_info.describe_wma();
   }

   /*virtual*/ float fuz::estimated_length() const /*override*/ {
      return this->_xwma_info.estimated_length();
   }

   /*virtual*/ size_t fuz::estimated_sample_count() const /*override*/ {
      const auto* format = this->_xwma_info.format;
      if (!format)
         return 0;
      const auto bytes_per_sample = (format->nChannels * format->wBitsPerSample) / 8;
      if (this->_xwma_info.dpds.size) {
         const auto bytecount = this->_xwma_info.dpds.data[(this->_xwma_info.dpds.size / 4) - 1];
         return bytecount / bytes_per_sample;
      }
      return this->_xwma_info.audio.size / bytes_per_sample;
   }
}