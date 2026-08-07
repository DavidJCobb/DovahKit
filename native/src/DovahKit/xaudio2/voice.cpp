#include "./voice.h"
#include <cassert>
#include <limits>
#include <xaudio2.h>
#include "./engine_and_thread.h"

namespace dovahkit::xaudio2 {
   void voice::_set_nth_effect_parameters(uint32_t n, const void* data, uint32_t size) {
      if (n >= this->get_effect_count())
         return;
      this->_voice->SetEffectParameters(n, data, size, 0);
   }
   
   #pragma region Constructors and destructor
      voice::voice() {
      }
      voice::voice(voice&& src) noexcept {
         this->_voice   = src._voice;
         this->_details = src._details;
         src._voice   = nullptr;
         src._details = {};
      }

      voice::~voice() {
         if (this->_voice) {
            // NOTE: Fails if any voice is sending input to this voice.
            this->_voice->DestroyVoice();
         }
      }
   #pragma endregion

   float voice::get_volume() const {
      float v;
      this->_voice->GetVolume(&v);
      return v;
   }
   void voice::set_volume(float v) {
      this->_voice->SetVolume(v);
   }

   std::vector<float> voice::get_all_channel_volumes() const {
      std::vector<float> out;
      if (this->_voice) {
         out.resize(this->_details.channel_count);
         this->_voice->GetChannelVolumes(this->_details.channel_count, out.data());
      }
      return out;
   }
   void voice::set_all_channel_volumes(const std::vector<float>& src) {
      const auto src_size = src.size();
      if (src_size == this->_details.channel_count) {
         this->_voice->SetChannelVolumes(src_size, src.data());
      } else {
         std::vector<float> list;
         if (src_size < this->_details.channel_count) {
            list = this->get_all_channel_volumes();
            for (size_t i = 0; i < src_size; ++i) {
               list[i] = src[i];
            }
         } else {
            list.resize(this->_details.channel_count);
            for (size_t i = 0; i < this->_details.channel_count; ++i) {
               list[i] = src[i];
            }
         }
         this->_voice->SetChannelVolumes(list.size(), list.data());
      }
   }

   void voice::clear_effect_chain() {
      XAUDIO2_EFFECT_CHAIN chain = {
         .EffectCount        = 0,
         .pEffectDescriptors = nullptr,
      };
      this->_voice->SetEffectChain(&chain);
      this->_details.effect_count = 0;
   }
   void voice::replace_effect_chain(const std::vector<XAUDIO2_EFFECT_DESCRIPTOR>& src) {
      assert(src.size() <= (std::numeric_limits<uint32_t>::max)());
      XAUDIO2_EFFECT_CHAIN chain = {
         .EffectCount        = (uint32_t)src.size(),
         .pEffectDescriptors = const_cast<XAUDIO2_EFFECT_DESCRIPTOR*>(src.data()),
      };
      this->_voice->SetEffectChain(&chain);
      this->_details.effect_count = src.size();
   }

   void voice::set_filter_parameters(const XAUDIO2_FILTER_PARAMETERS& params) {
      this->_voice->SetFilterParameters(&params);
   }

   void voice::clear_destination_voices() {
      XAUDIO2_VOICE_SENDS sends = {
         .SendCount = 0,
         .pSends    = nullptr,
      };
      this->_voice->SetOutputVoices(&sends);
   }
   void voice::set_destination_voices(const std::vector<XAUDIO2_SEND_DESCRIPTOR>& src) {
      assert(src.size() <= (std::numeric_limits<uint32_t>::max)());
      XAUDIO2_VOICE_SENDS sends = {
         .SendCount = (uint32_t)src.size(),
         .pSends    = const_cast<XAUDIO2_SEND_DESCRIPTOR*>(src.data()),
      };
      this->_voice->SetOutputVoices(&sends);
   }
   void voice::set_mastering_voice_as_sole_destination() {
      XAUDIO2_VOICE_SENDS sends = {
         .SendCount = 1,
         .pSends    = nullptr,
      };
      this->_voice->SetOutputVoices(&sends);
   }
}