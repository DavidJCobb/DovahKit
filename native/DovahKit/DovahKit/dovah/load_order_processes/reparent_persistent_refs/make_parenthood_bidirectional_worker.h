#pragma once
#include <cstdint>
#include <thread>
#include <vector>
#include "./queued_ref.h"
namespace dovah::load_order_processes::reparent_persistent_refs {
   class world_cell_map;
}

namespace dovah::load_order_processes::reparent_persistent_refs {
   class make_parenthood_bidirectional_worker {
      protected:
         const std::vector<queued_ref>& all_refs;
         const size_t quartet_index = 0;
         const bool   x_pos; // which quadrant should this worker handle? (true if positive on this axis)
         const bool   y_pos; // which quadrant should this worker handle? (true if positive on this axis)
         std::thread  thread;
         struct {
            size_t maximum = 0;
            size_t current = 0;
         } progress;
         
         static void _thread_handler(make_parenthood_bidirectional_worker* instance) {
            instance->_execute();
         }
         void _execute();

      public:
         make_parenthood_bidirectional_worker(
            const std::vector<queued_ref>&,
            size_t quartet_index,
            bool   x_pos,
            bool   y_pos
         );

         void start() noexcept;
         void wait_for() noexcept;
               
         inline bool is_active() const noexcept { return this->thread.get_id() != std::thread::id(); }
         float assess_load_progress() const noexcept;
   };
}