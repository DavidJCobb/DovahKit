#pragma once
#include <atomic>
#include <thread>
#include "file_part_loader.h"

namespace dovah::tes_file_reading {
   class file_threaded_part_loader_base : public file_part_loader {
      using lo_interface_t = load_order_interfaces::file_load;
      public:
         //
         // Subclasses should shadow at least (recommended_thread_count) with the desired number of 
         // threads of that type. The (heavy_duty_thread_count) is an optional thread count that can 
         // be used if hardware thread limits allow.
         //
         static constexpr int recommended_thread_count = 0;
         static constexpr int heavy_duty_thread_count  = 0;
         //
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
         file_threaded_part_loader_base(file_loader& owner, lo_interface_t& intfc) : file_part_loader(owner, intfc) {}
         //
         void  start();
         void  wait_for();
         float assess_progress() const noexcept;
         inline bool is_running() const noexcept { return this->running; }
         //
         inline const std::thread& get_thread_object() const noexcept { return this->thread; }
   };

   template<class C> struct _threaded_loader_recommended_thread_count {
      static constexpr int value = C::recommended_thread_count;
      static_assert(value != 0, "You forgot to specify a recommended thread count for a (file_threaded_part_loader_base) subclass that you're using!");
   };
   template<class C> struct _threaded_loader_heavy_duty_thread_count {
      static constexpr int value = C::heavy_duty_thread_count ? C::heavy_duty_thread_count : C::recommended_thread_count;
      static_assert(value != 0, "You forgot to specify a recommended thread count for a (file_threaded_part_loader_base) subclass that you're using!");
   };
   template<class C> constexpr int threaded_loader_recommended_thread_count = _threaded_loader_recommended_thread_count<C>::value;
   template<class C> constexpr int threaded_loader_heavy_duty_thread_count  = _threaded_loader_heavy_duty_thread_count<C>::value;
}