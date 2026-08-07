#pragma once
class IXAudio2;
class IXAudio2MasteringVoice;

namespace dovahkit::subsystems::audio::impl {
   class engine_and_thread {
      public:
         struct {
            IXAudio2* core = nullptr; // COM; refcounted
            IXAudio2MasteringVoice* mastering_voice = nullptr;
         } interfaces;
      protected:
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
   };
}
