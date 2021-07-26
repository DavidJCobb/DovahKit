#pragma once

namespace dovahscript::core {
   namespace subsystems {
      class coordinator;
   }
   
   class client_thread_script_borrow_handle {
      friend class subsystems::coordinator;
      protected:
         bool valid = false;
      public:
         client_thread_script_borrow_handle();
         ~client_thread_script_borrow_handle();

         client_thread_script_borrow_handle& operator=(const client_thread_script_borrow_handle&) noexcept;
         client_thread_script_borrow_handle& operator=(client_thread_script_borrow_handle&&) noexcept;
         client_thread_script_borrow_handle(const client_thread_script_borrow_handle&);
         client_thread_script_borrow_handle(client_thread_script_borrow_handle&&);

         bool is_ready() const noexcept;
         inline bool is_valid() const noexcept { return this->valid; }

         void clear();

      protected:
         void set_valid();
   };
}