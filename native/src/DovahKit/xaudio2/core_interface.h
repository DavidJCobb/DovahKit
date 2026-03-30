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
   class core_interface {
      protected:
         struct {
            IXAudio2* core = nullptr; // COM; refcounted
            IXAudio2MasteringVoice* mastering_voice = nullptr;
         } x;
         std::atomic<uint32_t> _next_operation_set_id = 1;
         bool _com_initialized = false;

      public:
         core_interface();
         ~core_interface();
         core_interface(const core_interface&) = delete;
         core_interface& operator=(const core_interface&) = delete;
         core_interface(core_interface&&) noexcept;
         core_interface& operator=(core_interface&&) noexcept;

      protected:
         void _teardown();

      public:
         operation_set_handle begin_operation_set();
         void commit_operation_set(operation_set_handle&);

         IXAudio2SourceVoice* create_raw_source_voice(const tWAVEFORMATEX& format, IXAudio2VoiceCallback* callbacks);
   };
}
