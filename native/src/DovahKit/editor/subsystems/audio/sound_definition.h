#pragma once
struct tWAVEFORMATEX;
struct XAUDIO2_BUFFER;
struct XAUDIO2_BUFFER_WMA;

namespace dovahkit::subsystems::audio {
   class sound_definition {
      public:
         virtual ~sound_definition() {}
         virtual const tWAVEFORMATEX& get_format() const = 0;
         virtual XAUDIO2_BUFFER get_audio_buffer_info() const = 0;
         virtual XAUDIO2_BUFFER_WMA get_xwma_info() const;
   };
}