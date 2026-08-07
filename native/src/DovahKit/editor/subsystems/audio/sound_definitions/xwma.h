#pragma once
#include <memory>
#include <string_view>
#include "dovah/files/bsa/bsa_archived_file.h"
#include "../impl/xwma_file_info.h"
#include "../sound_definition.h"
#include "../utils/is_valid_xwma_riff.h"

namespace dovahkit::subsystems::audio::sound_definitions {
   class xwma final : public sound_definition {
      public:
         static constexpr const std::string_view primary_extension = "xwm";
         static constexpr const auto is_valid_data = &utils::is_valid_xwma_riff;

      protected:
         std::unique_ptr<dovah::bsa_archived_file> _file;
         impl::xwma_file_info _xwma_info;

      public:
         xwma(std::unique_ptr<dovah::bsa_archived_file>&&);

         virtual const impl::wave_format_ex& get_format() const override;
         virtual XAUDIO2_BUFFER       get_audio_buffer_info() const override;
         virtual XAUDIO2_BUFFER_WMA   get_xwma_info() const override;

         virtual float estimated_length() const override;

         virtual size_t estimated_sample_count() const override;

         constexpr const impl::xwma_file_info& audio_info() const noexcept { return this->_xwma_info; }
   };
}