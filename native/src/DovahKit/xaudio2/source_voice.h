#pragma once
#include <functional>
#include "./voice.h"
namespace dovahkit::xaudio2 {
   class engine_and_thread;
}
class  IXAudio2SourceVoice;
struct tWAVEFORMATEX;

namespace dovahkit::xaudio2 {
   struct source_voice_params {
      bool allow_filter_effect          = true;
      bool allow_sample_rate_conversion = true;
      bool allow_pitch                  = true;
      IXAudio2VoiceCallback* callbacks = nullptr;
      float max_frequency_ratio = 2.0F;
   };

   class source_voice : public voice {
      protected:
         IXAudio2SourceVoice*& _typed_voice();
         IXAudio2SourceVoice*& _typed_voice() const;

      public:
         source_voice(engine_and_thread&, const tWAVEFORMATEX&, const source_voice_params& = {});

         constexpr IXAudio2SourceVoice* get_raw_interface() {
            return (IXAudio2SourceVoice*)this->_voice;
         }

         float get_frequency_ratio() const;
         void set_frequency_ratio(float); // not instantaneous
   };
}