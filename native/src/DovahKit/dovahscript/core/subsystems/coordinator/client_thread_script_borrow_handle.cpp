#include "client_thread_script_borrow_handle.h"
#include "../coordinator.h"

namespace dovahscript::core {
   client_thread_script_borrow_handle::client_thread_script_borrow_handle() {
   }
   client_thread_script_borrow_handle::~client_thread_script_borrow_handle() {
      this->release();
   }

   client_thread_script_borrow_handle& client_thread_script_borrow_handle::operator=(const client_thread_script_borrow_handle& other) noexcept {
      if (!other.active) {
         this->release();
         return *this;
      }
      if (this->active == other.active)
         return *this;
      this->request();
      return *this;
   }
   client_thread_script_borrow_handle& client_thread_script_borrow_handle::operator=(client_thread_script_borrow_handle&& other) noexcept {
      this->release();
      this->active = other.active;
      other.active = false;
      return *this;
   }
   client_thread_script_borrow_handle::client_thread_script_borrow_handle(const client_thread_script_borrow_handle& other) {
      *this = other;
   }
   client_thread_script_borrow_handle::client_thread_script_borrow_handle(client_thread_script_borrow_handle&& other) {
      *this = other;
   }

   [[nodiscard]] bool client_thread_script_borrow_handle::is_ready() const noexcept {
      if (!this->active)
         return false;
      auto& coordinator_s = subsystems::coordinator::get();
      return coordinator_s.worker_thread_state == subsystems::coordinator::thread_wait_state::waiting;
   }

   void client_thread_script_borrow_handle::request() {
      if (this->active)
         return;
      this->active = true;
      auto& coordinator_s = subsystems::coordinator::get();
      ++coordinator_s.outstanding_client_thread_script_borrow_requests;
   }
   void client_thread_script_borrow_handle::release() {
      if (!this->active)
         return;
      this->active = false;
      auto& coordinator_s = subsystems::coordinator::get();
      --coordinator_s.outstanding_client_thread_script_borrow_requests;
   }
}