#include "./fuz.h"
#include <xaudio2.h>

namespace dovahkit::subsystems::audio::sound_definitions {
   fuz::fuz(std::unique_ptr<dovah::bsa_archived_file>&& f) {
      this->_file = std::move(f);
         
      auto& file = *this->_file;
      auto* data = file.data();
      auto  size = file.size();

      this->_fuz_info = dovah::fuz::file_info{ data, size };
      // Read RIFF header.
      if (size > this->_fuz_info.buffer.size + dovah::fuz::header_size) {
         auto*    riff_data = (const void*)((const uint8_t*)data + dovah::fuz::header_size + this->_fuz_info.buffer.size);
         uint32_t riff_size = size - this->_fuz_info.buffer.size - dovah::fuz::header_size;

         this->_xwma_info = dovahkit::xaudio2::xwma_file_info{ riff_data, riff_size };
      }
   }

   /*virtual*/ const tWAVEFORMATEX& fuz::get_format() const /*override*/ {
      return *this->_xwma_info.format;
   }
   /*virtual*/ XAUDIO2_BUFFER fuz::get_audio_buffer_info() const /*override*/ {
      return this->_xwma_info.describe_buffer();
   }
   /*virtual*/ XAUDIO2_BUFFER_WMA fuz::get_xwma_info() const /*override*/ {
      return this->_xwma_info.describe_wma();
   }
}