#include "./dovahkit_audio_subsystem.h"
#include <xaudio2.h>
#include "./sound_category.h"
#include "./sound_instance.h"

namespace dovahkit::subsystems::audio {
   core::core() {
      for (auto*& voice : this->_sound_category_submixes.list) {
         voice = _make_sound_category_submix();
      }
   }
   core::~core() {
      emit this->onBeforeTeardown();
      for (auto*& voice : this->_sound_category_submixes.list) {
         if (!voice)
            continue;
         voice->DestroyVoice();
         voice = nullptr;
      }
   }

   void core::set_sound_instance_category(sound_instance& inst, sound_category c) {
      auto* submix = this->get_sound_category_submix(c);
      if (!submix) {
         submix = this->_sound_category_submixes.uncategorized;
         if (!submix)
            return;
      }
      auto* source = inst._get_raw_interface({});
      if (!source)
         return;

      XAUDIO2_SEND_DESCRIPTOR desc = {
         .Flags        = 0,
         .pOutputVoice = submix,
      };
      XAUDIO2_VOICE_SENDS sends = {
         .SendCount = 1,
         .pSends    = &desc,
      };
      source->SetOutputVoices(&sends);
   }

   void core::set_category_volume(sound_category c, float v) {
      auto* voice = this->get_sound_category_submix(c);
      if (voice)
         voice->SetVolume(v);
   }
   void core::set_master_volume(float v) {
      auto* voice = this->_engine_and_thread.get_raw_mastering_voice_interface();
      if (!voice)
         return;
      voice->SetVolume(v);
   }

   const IXAudio2SubmixVoice* core::get_sound_category_submix(sound_category c) const {
      switch (c) {
         case sound_category::uncategorized:
            return this->_sound_category_submixes.uncategorized;
         case sound_category::dialogue_preview:
            return this->_sound_category_submixes.dialogue_preview;
      }
      return nullptr;
   }
   IXAudio2SubmixVoice* core::get_sound_category_submix(sound_category c) {
      return const_cast<IXAudio2SubmixVoice*>(std::as_const(*this).get_sound_category_submix(c));
   }

   IXAudio2SubmixVoice* core::_make_sound_category_submix() {
      constexpr const bool     allow_filter_effect = false;
      constexpr const uint32_t channel_count       = 1;
      constexpr const uint32_t processing_stage    = 0;
      constexpr const uint32_t sample_rate         = 44100;

      constexpr const uint32_t flags = []() {
         uint32_t v = 0;
         if (allow_filter_effect)
            v |= XAUDIO2_VOICE_USEFILTER;
         return v;
      }();

      IXAudio2SubmixVoice* result = nullptr;
      this->_engine_and_thread.get_raw_interface()->CreateSubmixVoice(&result, channel_count, sample_rate, flags, processing_stage, nullptr, nullptr);
      return result;
   }
}