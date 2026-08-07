#pragma once
#include <memory>
#include "dovah/files/bsa/bsa_archived_file.h"
#include "dovah/files/fuz/file_info.h"
#include "xaudio2/xwma_file_info.h"
#include "../sound_definition.h"

namespace dovahkit::subsystems::audio::sound_definitions {
   class fuz : public sound_definition {
      protected:
         std::unique_ptr<dovah::bsa_archived_file> _file;
         dovah::fuz::file_info   _fuz_info;
         xaudio2::xwma_file_info _xwma_info;

      public:
         fuz(std::unique_ptr<dovah::bsa_archived_file>&&);

         virtual const tWAVEFORMATEX& get_format() const override;
         virtual XAUDIO2_BUFFER       get_audio_buffer_info() const override;
         virtual XAUDIO2_BUFFER_WMA   get_xwma_info() const override;

         constexpr const xaudio2::xwma_file_info& audio_info() const noexcept { return this->_xwma_info; }
         constexpr const dovah::fuz::file_info& fuz_info() const noexcept { return this->_fuz_info; }
   };
}