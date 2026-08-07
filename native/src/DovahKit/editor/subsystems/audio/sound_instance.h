#pragma once
#include <chrono>
#include <memory>
#include <QObject>
namespace dovahkit::subsystems::audio {
   class core;
   class sound_instance;
   namespace impl {
      class sound_instance_callbacks;

      class sound_instance_callback_passkey {
         friend sound_instance;
         friend sound_instance_callbacks;
         private:
            constexpr sound_instance_callback_passkey() {}
      };

      class sound_instance_voice_passkey {
         friend sound_instance;
         friend core;
         private:
            constexpr sound_instance_voice_passkey() {}
      };
   }
   class sound_definition;
}
class IXAudio2SourceVoice;

namespace dovahkit::subsystems::audio {
   struct sound_instance_params {
      float max_frequency_ratio = 2.0F;
   };

   class sound_instance : public QObject {
      Q_OBJECT;
      public:
         using clock_type      = std::chrono::steady_clock;
         using time_point_type = clock_type::time_point;
         using duration_type   = clock_type::duration;

      protected:
         std::shared_ptr<sound_definition> _definition;
         IXAudio2SourceVoice*              _voice     = nullptr;
         impl::sound_instance_callbacks*   _callbacks = nullptr;

         bool _is_at_start        = true;
         bool _is_playback_queued = false;
         bool _is_playing         = false;
         struct {
            time_point_type time;
            duration_type   played_prior;
         } _last_time_point;

      public:
         sound_instance(QObject* parent, const std::shared_ptr<sound_definition>&, const sound_instance_params& = {});
         ~sound_instance();

      public:
         void queue_playback();
         void play();
         void pause();
         void stop();

         void play_from(duration_type);
         void play_from_ms(size_t milliseconds);
         void play_from_s(double seconds);

         float get_volume() const;
         void set_volume(float);

         constexpr bool is_playing() const noexcept { return this->_is_playing; }
         constexpr bool is_at_start() const noexcept { return this->_is_at_start; }

         duration_type position() const;

         constexpr IXAudio2SourceVoice* _get_raw_interface(impl::sound_instance_voice_passkey) const noexcept {
            return this->_voice;
         }

      signals:
         void finished();
         void paused();
         void stopped();

      public: // passkeyed
         void on_playback_finished(const impl::sound_instance_callback_passkey&);

      protected:
         void _queue_playback_from(duration_type);

         void _reset_last_time_point();
         void _update_last_time_point(bool has_been_playing);
   };
}