#include "client_thread_script_borrow_handle.h"
#include "../coordinator.h"

namespace dovahscript::core {
   client_thread_script_borrow_handle::client_thread_script_borrow_handle() {
   }
   client_thread_script_borrow_handle::~client_thread_script_borrow_handle() {
      this->clear();
   }

   client_thread_script_borrow_handle& client_thread_script_borrow_handle::operator=(const client_thread_script_borrow_handle& other) noexcept {
      if (!other.valid) {
         this->clear();
         return;
      }
      if (this->valid == other.valid)
         return;
      this->set_valid();
   }
   client_thread_script_borrow_handle& client_thread_script_borrow_handle::operator=(client_thread_script_borrow_handle&& other) noexcept {
      this->clear();
      this->valid = other.valid;
      other.valid = false;
   }
   client_thread_script_borrow_handle::client_thread_script_borrow_handle(const client_thread_script_borrow_handle& other) {
      *this = other;
   }
   client_thread_script_borrow_handle::client_thread_script_borrow_handle(client_thread_script_borrow_handle&& other) {
      *this = other;
   }

   bool client_thread_script_borrow_handle::is_ready() const noexcept {
      if (!this->valid)
         return false;
      auto& coordinator_s = subsystems::coordinator::get();
      return coordinator_s.worker_thread_state == subsystems::coordinator::thread_wait_state::waiting;
   }
   void client_thread_script_borrow_handle::clear() {
      if (!this->valid)
         return;
      this->valid = false;
      auto& coordinator_s = subsystems::coordinator::get();
      --coordinator_s.outstanding_client_thread_script_borrow_requests;
   }

   void client_thread_script_borrow_handle::set_valid() {
      if (this->valid)
         return;
      this->valid = true;
      auto& coordinator_s = subsystems::coordinator::get();
      ++coordinator_s.outstanding_client_thread_script_borrow_requests;
   }
}