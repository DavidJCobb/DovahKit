#include "./simple_fuz_sound.h"
#include <xaudio2.h>
#include "./core_interface.h"

namespace dovahkit::xaudio2 {
   #pragma region simple_fuz_sound_definition
      simple_fuz_sound_definition::simple_fuz_sound_definition(std::unique_ptr<dovah::bsa_archived_file>&& f) {
         this->_file = std::move(f);
         
         auto& file = *this->_file;
         auto* data = file.data();
         auto  size = file.size();

         this->fuz = dovah::fuz::file_info{ data, size };
         // Read RIFF header.
         if (size > this->fuz.buffer.size + dovah::fuz::header_size) {
            auto*    riff_data = (const void*)((const uint8_t*)data + dovah::fuz::header_size + this->fuz.buffer.size);
            uint32_t riff_size = size - this->fuz.buffer.size - dovah::fuz::header_size;

            this->xwma = dovahkit::xaudio2::xwma_file_info{ riff_data, riff_size };
         }
      }
   #pragma endregion

   namespace impl {
      class simple_fuz_sound_instance_callbacks : public IXAudio2VoiceCallback {
         protected:
            simple_fuz_sound_instance& owner;

         public:
            simple_fuz_sound_instance_callbacks(simple_fuz_sound_instance& owner) : owner(owner) {}
            ~simple_fuz_sound_instance_callbacks() {}

            void OnStreamEnd() {
               this->owner.on_playback_finished({});
            }

            void OnVoiceProcessingPassEnd() {}
            void OnVoiceProcessingPassStart(UINT32 SamplesRequired) {}
            void OnBufferEnd(void* pBufferContext) {}
            void OnBufferStart(void* pBufferContext) {}
            void OnLoopEnd(void* pBufferContext) {}
            void OnVoiceError(void* pBufferContext, HRESULT Error) {}
      };
   }

   #pragma region simple_fuz_sound_instance
      simple_fuz_sound_instance::simple_fuz_sound_instance(core_interface& o, const std::shared_ptr<simple_fuz_sound_definition>& dfn) : _owner(o) {
         this->_definition = dfn;
         if (this->_definition) {
            auto& xwma = this->_definition->audio_info();
            if (xwma.format) {
               this->_callbacks = new impl::simple_fuz_sound_instance_callbacks(*this);
               this->_voice     = this->_owner.create_raw_source_voice(*xwma.format, this->_callbacks);
            }
         }
      }
      simple_fuz_sound_instance::~simple_fuz_sound_instance() {
         if (auto*& p = this->_voice) {
            p->DestroyVoice();
            p = nullptr;
         }
         if (auto*& p = this->_callbacks) {
            delete p;
            p = nullptr;
         }
      }

      void simple_fuz_sound_instance::queue_playback() {
         if (!this->_voice)
            return;
         auto& xwma = this->_definition->audio_info();
         if (!xwma.audio.data || !xwma.audio.size)
            return;
         auto buffer_info = xwma.describe_buffer();
         buffer_info.pContext = this;
         if (xwma.dpds.data) {
            auto buffer_xwma = xwma.describe_wma();
            this->_voice->SubmitSourceBuffer(&buffer_info, &buffer_xwma);
         } else {
            this->_voice->SubmitSourceBuffer(&buffer_info);
         }
         this->_is_playback_queued = true;
      }
      void simple_fuz_sound_instance::play() {
         if (this->_is_playing)
            return;
         if (!this->_voice)
            return;
         if (!this->_is_playback_queued) {
            this->queue_playback();
         }
         this->_voice->Start(0, 0);
         this->_is_playing  = true;
         this->_is_at_start = false;
      }
      void simple_fuz_sound_instance::pause() {
         if (!this->_voice)
            return;
         this->_voice->Stop(0, 0);
         this->_is_playing = false;
         emit this->paused();
      }
      void simple_fuz_sound_instance::stop() {
         if (!this->_voice)
            return;
         this->_voice->Stop(0, 0);
         this->_voice->FlushSourceBuffers();
         this->_is_playing         = false;
         this->_is_playback_queued = false;
         this->_is_at_start        = true;
         emit this->stopped();
      }

      void simple_fuz_sound_instance::on_playback_finished(const impl::simple_fuz_sound_callback_passkey&) {
         this->_is_playing         = false;
         this->_is_playback_queued = false;
         this->_is_at_start        = true;
         //
         // We don't want to `emit this->finished()`, because we're running from a 
         // performance-sensitive XAudio2 callback. Instead, we want to queue the 
         // signal to fire on the next spin of Qt's event loop, by co-opting the 
         // mechanism that's normally used to dispatch signals across threads.
         //
         if (auto* meta = this->metaObject())
            meta->invokeMethod(this, &simple_fuz_sound_instance::finished, Qt::ConnectionType::QueuedConnection);
      }
   #pragma endregion
}