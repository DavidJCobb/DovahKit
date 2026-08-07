#pragma once
#include <memory>
#include "dovah/files/bsa/bsa_archived_file.h"
#include "../impl/xwma_file_info.h"
#include "../sound_definition.h"

namespace dovahkit::subsystems::audio::sound_definitions {
   class xwma : public sound_definition {
      protected:
         std::unique_ptr<dovah::bsa_archived_file> _file;
         impl::xwma_file_info _xwma_info;

      public:
         xwma(std::unique_ptr<dovah::bsa_archived_file>&&);

         static bool data_is_likely_xwma(const void* data, size_t size);

         virtual const tWAVEFORMATEX& get_format() const override;
         virtual XAUDIO2_BUFFER       get_audio_buffer_info() const override;
         virtual XAUDIO2_BUFFER_WMA   get_xwma_info() const override;
         virtual float estimated_length() const override;

         constexpr const impl::xwma_file_info& audio_info() const noexcept { return this->_xwma_info; }
   };
}