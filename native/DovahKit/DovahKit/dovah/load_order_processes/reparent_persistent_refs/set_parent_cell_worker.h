#pragma once
#include <cstdint>
#include <thread>
#include <vector>
#include "./queued_ref.h"
namespace dovah::load_order_processes::reparent_persistent_refs {
   class world_cell_map;
}

namespace dovah::load_order_processes::reparent_persistent_refs {
   class set_parent_cell_worker {
      protected:
         world_cell_map&          cells;
         std::vector<queued_ref>& refs;
         std::thread thread;
         struct {
            size_t start = 0;
            size_t end   = 0;
         } range;
         struct {
            uint32_t maximum = 0;
            uint32_t current = 0;
         } progress;
         
         static void _thread_handler(set_parent_cell_worker* instance) {
            instance->_execute();
         }
         void _execute();

      public:
         set_parent_cell_worker(
            world_cell_map&,
            std::vector<queued_ref>&
         );

         void set_ref_indices_to_process(size_t start, size_t end);
         void start() noexcept;
         void wait_for() noexcept;
               
         inline bool is_active() const noexcept { return this->thread.get_id() != std::thread::id(); }
         float assess_load_progress() const noexcept;
   };
}