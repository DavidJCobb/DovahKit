#include "./xwma.h"
#include <xaudio2.h>
#include "../impl/wave_format_ex.h"

namespace dovahkit::subsystems::audio::sound_definitions {
   xwma::xwma(std::unique_ptr<dovah::bsa_archived_file>&& f) {
      this->_file = std::move(f);
         
      auto& file = *this->_file;
      auto* data = file.data();
      auto  size = file.size();
      this->_xwma_info = impl::xwma_file_info{ data, size };
   }

   /*static*/ bool xwma::data_is_likely_xwma(const void* data, size_t size) {
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
         if (riff_type != 'XWMA')
            return false;
         if (riff_size > size - 8)
            return false;
      }

      return true;
   }

   /*virtual*/ const impl::wave_format_ex& xwma::get_format() const /*override*/ {
      return *(const impl::wave_format_ex*)this->_xwma_info.format;
   }
   /*virtual*/ XAUDIO2_BUFFER xwma::get_audio_buffer_info() const /*override*/ {
      return this->_xwma_info.describe_buffer();
   }
   /*virtual*/ XAUDIO2_BUFFER_WMA xwma::get_xwma_info() const /*override*/ {
      return this->_xwma_info.describe_wma();
   }

   /*virtual*/ float xwma::estimated_length() const /*override*/ {
      return this->_xwma_info.estimated_length();
   }

   /*virtual*/ size_t xwma::estimated_sample_count() const /*override*/ {
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