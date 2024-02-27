#pragma once
#include "dovah/files/tes_file_reading/basic_reader.h"
#include <thread>
#include <vector>

namespace dovah {
   class form_stub;
   namespace load_order_interfaces {
      class form_load;
   }
   namespace tes_file_reading {
      class record;
   }
}

namespace dovahkit::subsystems::form_info_cache {
   class threaded_builder : dovah::tes_file_reading::basic_reader {
      public:
         using handler_t = void(*)(dovah::form_stub&, dovah::tes_file_reading::record&, dovah::load_order_interfaces::form_load&);
      protected:
         struct _handler_set {
            handler_t handler = nullptr;
            std::vector<dovah::form_stub*> stubs;
         };

         std::vector<_handler_set> queue;
         std::thread thread;
         struct {
            uint32_t maximum = 0;
            uint32_t current = 0;
         } progress;
         
         static void _thread_handler(threaded_builder* instance) {
            instance->_execute();
         }
         void _execute();

      public:
         void add_to_queue(handler_t, dovah::form_stub* stub) noexcept;
         void start() noexcept;
         void wait_for() noexcept;
         
         inline bool is_active() const noexcept { return this->thread.get_id() != std::thread::id(); }
         float assess_load_progress() const noexcept;
   };
}