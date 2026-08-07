#include "./engine_and_thread.h"
#include <algorithm> // std::swap
#include <cassert>
#include <windows.h>
#include <xaudio2.h>
#include "./operation_set_handle.h"

namespace dovahkit::xaudio2 {
   engine_and_thread::engine_and_thread() {
      {
         assert(this->_com_initialized == false);
         auto result = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED); // Qt forcibly uses COINIT_APARTMENTTHREADED via OleInitialize
         switch (result) {
            case S_OK:
            case S_FALSE:
               this->_com_initialized = true;
               break;
            case RPC_E_CHANGED_MODE:
               break;
         }
      }
      if (this->_com_initialized) {
         //
         // Per docs, these API codes don't write the target pointer if they fail, so 
         // while they *do* return HRESULTs, we shouldn't have to check those unless 
         // we want detailed error information (which these functions are not spec'd 
         // to provide).
         //
         XAudio2Create(&this->x.core, 0, XAUDIO2_DEFAULT_PROCESSOR);
         if (this->x.core) {
            this->x.core->CreateMasteringVoice(&this->x.mastering_voice);
         }
      }
   }
   engine_and_thread::~engine_and_thread() {
      this->_teardown();
   }

   engine_and_thread::engine_and_thread(engine_and_thread&& src) noexcept {
      this->_teardown();
      std::swap(this->x, src.x);
   }

   engine_and_thread& engine_and_thread::operator=(engine_and_thread&& src) noexcept {
      this->_teardown();
      std::swap(this->x, src.x);
      return *this;
   }

   void engine_and_thread::_teardown() {
      if (auto*& p = this->x.mastering_voice) {
         p->DestroyVoice();
         p = nullptr;
      }
      if (auto*& p = this->x.core) {
         p->Release();
         p = nullptr;
      }

      if (this->_com_initialized) {
         CoUninitialize();
         this->_com_initialized = false;
      }
   }

   operation_set_handle engine_and_thread::begin_operation_set() {
      auto id = this->_next_operation_set_id.fetch_add(1);
      return operation_set_handle{id};
   }
   void engine_and_thread::commit_operation_set(operation_set_handle& handle) {
      assert(!!this->x.core);
      this->x.core->CommitChanges(handle.id);
   }

   IXAudio2SourceVoice* engine_and_thread::create_raw_source_voice(const WAVEFORMATEX& format, IXAudio2VoiceCallback* callbacks) {
      IXAudio2SourceVoice* out = nullptr;
      if (auto* intfc = this->x.core) {
         intfc->CreateSourceVoice(&out, &format, 0, 1.0F, callbacks);
      }
      return out;
   }
}