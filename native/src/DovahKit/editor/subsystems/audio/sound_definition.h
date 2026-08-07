#pragma once
struct tWAVEFORMATEX;
struct XAUDIO2_BUFFER;
struct XAUDIO2_BUFFER_WMA;
namespace dovahkit::subsystems::audio::impl {
   struct wave_format_ex;
}

namespace dovahkit::subsystems::audio {
   class sound_definition {
      public:
         virtual ~sound_definition() {}
         virtual const impl::wave_format_ex& get_format() const = 0;
         virtual XAUDIO2_BUFFER get_audio_buffer_info() const = 0;
         virtual XAUDIO2_BUFFER_WMA get_xwma_info() const; // default implementation returns empty/none

         virtual float estimated_length() const = 0;

         virtual size_t estimated_sample_count() const = 0;
         virtual size_t estimated_samples_at_time_point(double) const;
   };
}