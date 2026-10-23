#pragma once
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
         std::thread thread;
         struct {
            struct {
               int32_t x = 0;
               int32_t y = 0;
            } center;
            struct {
               uint32_t start = 0;
               uint32_t end   = 0xFFFFFFFF;
            } radii;
         } ring;
         struct {
            size_t maximum = 0;
            size_t current = 0;
         } progress;
         
         static void _thread_handler(make_parenthood_bidirectional_worker* instance) {
            instance->_execute();
         }
         void _execute();

      public:
         make_parenthood_bidirectional_worker(const std::vector<queued_ref>&);

         void set_ring(const std::pair<int32_t, int32_t>& center, uint32_t start_radius, uint32_t end_radius);
         void start() noexcept;
         void wait_for() noexcept;

         // fast path for if there isn't enough work to divide across threads
         void run_on_this_thread();
               
         inline bool is_active() const noexcept { return this->thread.get_id() != std::thread::id(); }
         float assess_load_progress() const noexcept;
   };
}