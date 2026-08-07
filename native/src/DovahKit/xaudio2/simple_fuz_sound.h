#pragma once
#include <memory>
#include <QObject>
#include "dovah/files/bsa/bsa_archived_file.h"
#include "dovah/files/fuz/file_info.h"
#include "./xwma_file_info.h"
class IXAudio2SourceVoice;
namespace dovahkit::xaudio2 {
   class engine_and_thread;
}

namespace dovahkit::xaudio2 {
   class simple_fuz_sound_definition {
      protected:
         std::unique_ptr<dovah::bsa_archived_file> _file;
         dovah::fuz::file_info fuz;
         xwma_file_info        xwma;

      public:
         simple_fuz_sound_definition(std::unique_ptr<dovah::bsa_archived_file>&&);

         constexpr const xwma_file_info& audio_info() const noexcept { return this->xwma; }
         constexpr const dovah::fuz::file_info& fuz_info() const noexcept { return this->fuz; }
   };

   namespace impl {
      class simple_fuz_sound_instance_callbacks;

      class simple_fuz_sound_callback_passkey {
         friend simple_fuz_sound_instance_callbacks;
         private:
            constexpr simple_fuz_sound_callback_passkey() {}
      };
   }

   class simple_fuz_sound_instance : public QObject {
      Q_OBJECT;
      protected:
         std::shared_ptr<simple_fuz_sound_definition> _definition;
         engine_and_thread&   _owner;
         IXAudio2SourceVoice* _voice = nullptr;
         impl::simple_fuz_sound_instance_callbacks* _callbacks = nullptr;

         bool _is_at_start = true;
         bool _is_playback_queued = false;
         bool _is_playing = false;

      public:
         simple_fuz_sound_instance(engine_and_thread&, const std::shared_ptr<simple_fuz_sound_definition>&);
         ~simple_fuz_sound_instance();

      public:
         void queue_playback();
         void play();
         void pause();
         void stop();

         constexpr bool is_playing() const noexcept { return this->_is_playing; }
         constexpr bool is_at_start() const noexcept { return this->_is_at_start; }

      signals:
         void finished();
         void paused();
         void stopped();

      public: // passkeyed
         void on_playback_finished(const impl::simple_fuz_sound_callback_passkey&);
   };
}