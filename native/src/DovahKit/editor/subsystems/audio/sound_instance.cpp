#include "./sound_instance.h"
#include "./dovahkit_audio_subsystem.h"
#include "./impl/sound_instance_callbacks.h"
#include "./sound_definition.h"

namespace dovahkit::subsystems::audio {
   sound_instance::sound_instance(QObject* parent, const std::shared_ptr<sound_definition>& dfn, const sound_instance_params& params) : QObject(parent) {
      this->_definition = dfn;
      this->_callbacks  = new impl::sound_instance_callbacks(*this);

      auto& subsystem = core::get_or_create();
      if (auto* intfc = subsystem.get_engine().interfaces.core) {
         intfc->CreateSourceVoice(&this->_voice, (WAVEFORMATEX*)&this->_definition->get_format(), 0, params.max_frequency_ratio, this->_callbacks);
      }
      QObject::connect(&subsystem, &core::onBeforeTeardown, this, [this]() {
         if (auto*& p = this->_voice) {
            p->DestroyVoice();
            p = nullptr;
         }
      });
   }
   sound_instance::~sound_instance() {
      if (auto*& p = this->_voice) {
         p->DestroyVoice();
         p = nullptr;
      }
      if (auto*& p = this->_callbacks) {
         delete p;
         p = nullptr;
      }
   }

   void sound_instance::queue_playback() {
      this->_queue_playback_from(duration_type::zero());
   }
   void sound_instance::play() {
      if (this->_is_playing)
         return;
      if (!this->_voice)
         return;
      const bool was_at_start = this->_is_at_start;
      if (!this->_is_playback_queued) {
         this->queue_playback();
      }
      this->_voice->Start(0, 0);
      this->_is_playing  = true;
      this->_is_at_start = false;
      if (was_at_start) {
         this->_reset_last_time_point();
      } else {
         this->_update_last_time_point(false);
      }
   }
   void sound_instance::pause() {
      if (!this->_voice)
         return;
      if (!this->_is_playing)
         return;
      this->_voice->Stop(0, 0);
      this->_is_playing = false;
      this->_update_last_time_point(true);
      emit this->paused();
   }
   void sound_instance::stop() {
      if (!this->_voice)
         return;
      this->_voice->Stop(0, 0);
      this->_voice->FlushSourceBuffers();
      this->_is_playing         = false;
      this->_is_playback_queued = false;
      this->_is_at_start        = true;
      this->_reset_last_time_point();
      emit this->stopped();
   }

   void sound_instance::play_from(duration_type offset) {
      if (!this->_voice)
         return;
      //
      // Stop the sound without emitting signals.
      //
      this->_voice->Stop(0, 0);
      this->_voice->FlushSourceBuffers();
      this->_is_playing         = false;
      this->_is_playback_queued = false;
      this->_is_at_start        = true;
      //
      // Now play from the desired offset.
      //
      this->_reset_last_time_point();
      this->_queue_playback_from(offset);
      this->_is_playing  = true;
      this->_is_at_start = offset == duration_type::zero();
      this->_last_time_point.played_prior = offset;
      this->_voice->Start(0, 0);
   }
   void sound_instance::play_from_ms(size_t milliseconds) {
      std::chrono::milliseconds ms(milliseconds);
      this->play_from(duration_cast<duration_type>(ms));
   }
   void sound_instance::play_from_s(double seconds) {
      this->play_from_ms(seconds * 1000);
   }

   float sound_instance::get_volume() const {
      if (!this->_voice)
         return 1;
      float v = 1;
      this->_voice->GetVolume(&v);
      return v;
   }
   void sound_instance::set_volume(float v) {
      if (!this->_voice)
         return;
      this->_voice->SetVolume(v);
   }

   sound_instance::duration_type sound_instance::position() const {
      if (this->_is_playing) {
         const auto now = clock_type::now();
         return this->_last_time_point.played_prior + (now - this->_last_time_point.time);
      }
      return this->_last_time_point.played_prior;
   }

   void sound_instance::on_playback_finished(const impl::sound_instance_callback_passkey&) {
      this->_is_playing         = false;
      this->_is_playback_queued = false;
      this->_is_at_start        = true;
      this->_reset_last_time_point();
      //
      // We don't want to `emit this->finished()`, because we're running from a 
      // performance-sensitive XAudio2 callback. Instead, we want to queue the 
      // signal to fire on the next spin of Qt's event loop, by co-opting the 
      // mechanism that's normally used to dispatch signals across threads.
      //
      if (auto* meta = this->metaObject())
         meta->invokeMethod(this, &sound_instance::finished, Qt::ConnectionType::QueuedConnection);
   }

   void sound_instance::_queue_playback_from(duration_type from) {
      if (!this->_voice || !this->_definition)
         return;
      auto buffer_info = this->_definition->get_audio_buffer_info();
      auto xwma_info   = this->_definition->get_xwma_info();
      buffer_info.pContext = this;

      if (from != duration_type::zero()) {
         const auto seconds = (double)std::chrono::duration_cast<std::chrono::milliseconds>(from).count() / 1000;

         auto sample_count   = this->_definition->estimated_sample_count();
         auto samples_before = this->_definition->estimated_samples_at_time_point(seconds);
         buffer_info.PlayBegin  = samples_before;
         buffer_info.PlayLength = sample_count - samples_before;
      }

      if (xwma_info.PacketCount) {
         this->_voice->SubmitSourceBuffer(&buffer_info, &xwma_info);
      } else {
         this->_voice->SubmitSourceBuffer(&buffer_info);
      }

      this->_is_playback_queued = true;
   }

   void sound_instance::_reset_last_time_point() {
      this->_last_time_point.time         = clock_type::now();
      this->_last_time_point.played_prior = {};
   }
   void sound_instance::_update_last_time_point(bool has_been_playing) {
      const auto now = clock_type::now();
      if (has_been_playing) {
         this->_last_time_point.played_prior += (now - this->_last_time_point.time);
      }
      this->_last_time_point.time = now;
   }
}