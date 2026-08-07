#pragma once
#include <atomic>
#include <cstdint>
#include "./operation_set_handle.h"
class  IXAudio2;
class  IXAudio2MasteringVoice;
class  IXAudio2SourceVoice;
class  IXAudio2VoiceCallback;
struct tWAVEFORMATEX;

namespace dovahkit::xaudio2 {
   class engine_and_thread {
      protected:
         struct {
            IXAudio2* core = nullptr; // COM; refcounted
            IXAudio2MasteringVoice* mastering_voice = nullptr;
         } x;
         std::atomic<uint32_t> _next_operation_set_id = 1;
         bool _com_initialized = false;

      public:
         engine_and_thread();
         ~engine_and_thread();
         engine_and_thread(const engine_and_thread&) = delete;
         engine_and_thread& operator=(const engine_and_thread&) = delete;
         engine_and_thread(engine_and_thread&&) noexcept;
         engine_and_thread& operator=(engine_and_thread&&) noexcept;

      protected:
         void _teardown();

      public:
         constexpr IXAudio2* get_raw_interface() noexcept {
            return this->x.core;
         }
         constexpr IXAudio2MasteringVoice* get_raw_mastering_voice_interface() noexcept {
            return this->x.mastering_voice;
         }

         operation_set_handle begin_operation_set();
         void commit_operation_set(operation_set_handle&);

         IXAudio2SourceVoice* create_raw_source_voice(const tWAVEFORMATEX& format, IXAudio2VoiceCallback* callbacks);
   };
}
