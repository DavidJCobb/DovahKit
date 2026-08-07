#include "./source_voice.h"
#include <cassert>
#include <xaudio2.h>
#include "./engine_and_thread.h"

namespace dovahkit::xaudio2 {
   IXAudio2SourceVoice*& source_voice::_typed_voice() {
      return *(IXAudio2SourceVoice**)&this->_voice;
   }
   IXAudio2SourceVoice*& source_voice::_typed_voice() const {
      return *(IXAudio2SourceVoice**)&this->_voice;
   }

   source_voice::source_voice(engine_and_thread& owner, const tWAVEFORMATEX& format, const source_voice_params& params) {
      assert(format.nChannels <= XAUDIO2_MAX_AUDIO_CHANNELS);
      assert(format.nSamplesPerSec >= XAUDIO2_MIN_SAMPLE_RATE);
      assert(format.nSamplesPerSec <= XAUDIO2_MAX_SAMPLE_RATE);

      auto* intfc = owner.get_raw_interface();
      assert(!!intfc);
      HRESULT result;
      {
         uint32_t flags = 0;
         if (params.allow_filter_effect)
            flags |= XAUDIO2_VOICE_USEFILTER;
         if (!params.allow_pitch)
            flags |= XAUDIO2_VOICE_NOPITCH;
         if (!params.allow_sample_rate_conversion)
            flags |= (XAUDIO2_VOICE_NOPITCH | XAUDIO2_VOICE_NOSRC);

         float freq = params.max_frequency_ratio;
         if (freq < XAUDIO2_MIN_FREQ_RATIO)
            freq = XAUDIO2_MIN_FREQ_RATIO;
         else if (freq > XAUDIO2_MAX_FREQ_RATIO)
            freq = XAUDIO2_MAX_FREQ_RATIO;

         result = intfc->CreateSourceVoice(&_typed_voice(), &format, flags, freq, params.callbacks);
      }
      if (result == S_OK) {
         this->_details.channel_count = format.nChannels;
      } else {
         // ... TODO: throw?
      }
   }

   float source_voice::get_frequency_ratio() const {
      float v;
      _typed_voice()->GetFrequencyRatio(&v);
      return v;
   }
   void source_voice::set_frequency_ratio(float v) {
      _typed_voice()->SetFrequencyRatio(v);
   }
}