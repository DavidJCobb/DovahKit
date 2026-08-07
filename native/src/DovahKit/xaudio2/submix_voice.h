#pragma once
#include <cstdint>
#include <vector>
#include "./voice.h"
namespace dovahkit::xaudio2 {
   class engine_and_thread;
}
class IXAudio2SubmixVoice;

namespace dovahkit::xaudio2 {
   struct submix_voice_params {
      struct {
         uint32_t channel_count = 1;
         uint32_t sample_rate   = 44100;
      } input;
      uint32_t processing_stage = 0;
      bool     allow_filter_effect = false;
   };

   class submix_voice : public voice {
      protected:
         IXAudio2SubmixVoice*& _typed_voice();

      public:
         submix_voice(engine_and_thread&, const submix_voice_params&);

         constexpr IXAudio2SubmixVoice* get_raw_interface() {
            return (IXAudio2SubmixVoice*)this->_voice;
         }
   };
}