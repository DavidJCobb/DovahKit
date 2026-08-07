#include "./submix_voice.h"
#include <cassert>
#include <xaudio2.h>
#include "./engine_and_thread.h"

namespace dovahkit::xaudio2 {
   IXAudio2SubmixVoice*& submix_voice::_typed_voice() {
      return *(IXAudio2SubmixVoice**)&this->_voice;
   }

   submix_voice::submix_voice(engine_and_thread& owner, const submix_voice_params& params) {
      auto* intfc = owner.get_raw_interface();
      assert(!!intfc);
      HRESULT result;
      {
         uint32_t channel_count = params.input.channel_count;
         if (channel_count > XAUDIO2_MAX_AUDIO_CHANNELS)
            channel_count = XAUDIO2_MAX_AUDIO_CHANNELS;

         uint32_t sample_rate = params.input.sample_rate;
         sample_rate -= (sample_rate % XAUDIO2_QUANTUM_DENOMINATOR);
         if (sample_rate < XAUDIO2_MIN_SAMPLE_RATE)
            sample_rate = XAUDIO2_MIN_SAMPLE_RATE;
         else if (sample_rate > XAUDIO2_MAX_SAMPLE_RATE)
            sample_rate = XAUDIO2_MAX_SAMPLE_RATE;

         uint32_t flags = 0;
         if (params.allow_filter_effect)
            flags |= XAUDIO2_VOICE_USEFILTER;

         result = intfc->CreateSubmixVoice(&_typed_voice(), channel_count, sample_rate, flags, params.processing_stage, nullptr, nullptr);
         if (result == S_OK) {
            this->_details.channel_count = channel_count;
         }
      }
      if (result != S_OK) {
         //
         // TODO: throw an exception
         //
      }
   }
}