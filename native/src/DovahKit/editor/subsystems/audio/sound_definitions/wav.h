#pragma once
#include <memory>
#include <string_view>
#include "dovah/files/bsa/bsa_archived_file.h"
#include "../sound_definition.h"

namespace dovahkit::subsystems::audio::sound_definitions {
   class wav : public sound_definition {
      public:
         static constexpr const std::string_view primary_extension = "wav";

      protected:
         std::unique_ptr<dovah::bsa_archived_file> _file;
         std::unique_ptr<tWAVEFORMATEX> _format;
         struct {
            const void* data = nullptr;
            uint32_t    size = 0;
         } _audio;

      public:
         wav(std::unique_ptr<dovah::bsa_archived_file>&&);

         static bool data_is_likely_wav(const void* data, size_t size);

         virtual const tWAVEFORMATEX& get_format() const override;
         virtual XAUDIO2_BUFFER       get_audio_buffer_info() const override;
         virtual float estimated_length() const override;
         uint32_t sample_count() const;
   };
}