#include "./engine_and_thread.h"
#include <algorithm> // std::swap
#include <cassert>
#include <windows.h>
#include <xaudio2.h>

namespace dovahkit::subsystems::audio::impl {
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
         XAudio2Create(&this->interfaces.core, 0, XAUDIO2_DEFAULT_PROCESSOR);
         if (this->interfaces.core) {
            this->interfaces.core->CreateMasteringVoice(&this->interfaces.mastering_voice);
         }
      }
   }
   engine_and_thread::~engine_and_thread() {
      this->_teardown();
   }

   engine_and_thread::engine_and_thread(engine_and_thread&& src) noexcept {
      this->_teardown();
      std::swap(this->interfaces, src.interfaces);
   }

   engine_and_thread& engine_and_thread::operator=(engine_and_thread&& src) noexcept {
      this->_teardown();
      std::swap(this->interfaces, src.interfaces);
      return *this;
   }

   void engine_and_thread::_teardown() {
      if (auto*& p = this->interfaces.mastering_voice) {
         p->DestroyVoice();
         p = nullptr;
      }
      if (auto*& p = this->interfaces.core) {
         p->Release();
         p = nullptr;
      }

      if (this->_com_initialized) {
         CoUninitialize();
         this->_com_initialized = false;
      }
   }
}