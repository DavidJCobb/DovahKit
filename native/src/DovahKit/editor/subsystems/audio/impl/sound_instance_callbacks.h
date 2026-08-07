#pragma once
#include <xaudio2.h>
namespace dovahkit::subsystems::audio {
   class sound_instance;
}

namespace dovahkit::subsystems::audio::impl {
   class sound_instance_callbacks : public IXAudio2VoiceCallback {
      protected:
         sound_instance& owner;

      public:
         sound_instance_callbacks(sound_instance& owner) : owner(owner) {}
         ~sound_instance_callbacks() {}

         virtual void OnStreamEnd() override;

         virtual void OnVoiceProcessingPassEnd() override {}
         virtual void OnVoiceProcessingPassStart(UINT32 SamplesRequired) override {}
         virtual void OnBufferEnd(void* pBufferContext) override {}
         virtual void OnBufferStart(void* pBufferContext) override {}
         virtual void OnLoopEnd(void* pBufferContext) override {}
         virtual void OnVoiceError(void* pBufferContext, HRESULT Error) override {}
   };
}