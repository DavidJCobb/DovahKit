#pragma once
#include <atomic>
#include <thread>
#include "file_part_loader.h"

namespace dovah::tes_file_reading {
   class file_threaded_part_loader_base : public file_part_loader {
      protected:
         virtual void exec() = 0; // thread-local loading behavior should be defined in an override
         //
         std::thread thread;
         struct {
            //
            // These values are non-atomic (i.e. not thread-safe) and should only be used for cosmetic 
            // tasks, like displaying progress in the UI, not actual important tasks that need a 100% 
            // accurate result. They power the (assess_progress) function, and should be updated by 
            // the (exec) overload.
            //
            uint32_t current = 0;
            uint32_t maximum = 0;
         } progress;
         //
      private:
         static void _thread_handler(file_threaded_part_loader_base* instance);
         //
         std::atomic<bool> running = false; // no, (std::thread::joinable) is not the same thing; it returns (true) until (std::thread::join) is manually called
         //
      public:
         file_threaded_part_loader_base(file_loader& owner) : file_part_loader(owner) {}
         //
         void  start();
         void  wait_for();
         float assess_progress() const noexcept;
         inline bool is_running() const noexcept { return this->running; }
         //
         inline const std::thread& get_thread_object() const noexcept { return this->thread; }
   };
}