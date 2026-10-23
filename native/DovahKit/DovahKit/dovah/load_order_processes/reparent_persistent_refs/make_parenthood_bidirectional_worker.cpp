#include "./make_parenthood_bidirectional_worker.h"
#include "dovah/_impl/form_stubs/make_one_way_use_bidirectional.h"
#include "dovah/termination_handler.h"
#include "./queued_ref.h"

namespace dovah::load_order_processes::reparent_persistent_refs {
   make_parenthood_bidirectional_worker::make_parenthood_bidirectional_worker(const std::vector<queued_ref>& all_refs) :
      all_refs(all_refs)
   {}

   void make_parenthood_bidirectional_worker::_execute() {
      termination_handler::update_this_thread();

      const uint64_t start_distance_sq = (int64_t)this->ring.radii.start * this->ring.radii.start;
      const uint64_t end_distance_sq   = (int64_t)this->ring.radii.end   * this->ring.radii.end;

      const size_t size = this->all_refs.size();
      for (size_t i = 0; i < size; ++i) {
         auto& item = this->all_refs[i];
         ++this->progress.current;
         if (!item.cell)
            continue;

         uint64_t distance_sq;
         {
            auto x = item.grid_position.x - this->ring.center.x;
            auto y = item.grid_position.y - this->ring.center.y;
            distance_sq = (uint64_t)(((int64_t)x * x) + ((int64_t)y * y));
         }
         if (distance_sq < start_distance_sq)
            continue;
         if (distance_sq >= end_distance_sq)
            continue;

         impl::_form_stubs::make_one_way_use_bidirectional({}, *item.ref, *item.cell);
      }
   }

   void make_parenthood_bidirectional_worker::set_ring(const std::pair<int32_t, int32_t>& center, uint32_t start_radius, uint32_t end_radius) {
      this->ring = {
         .center = { center.first, center.second },
         .radii  = { start_radius, end_radius },
      };
   }
   void make_parenthood_bidirectional_worker::start() noexcept {
      assert(!this->is_active());
      this->progress.current = 0;
      this->progress.maximum = this->all_refs.size();
      this->thread = std::thread(&_thread_handler, this);
   }
   void make_parenthood_bidirectional_worker::wait_for() noexcept {
      if (this->is_active())
         this->thread.join();
   }

   void make_parenthood_bidirectional_worker::run_on_this_thread() {
      const size_t size = this->all_refs.size();
      for (size_t i = 0; i < size; ++i) {
         auto& item = this->all_refs[i];
         if (!item.cell)
            continue;
         impl::_form_stubs::make_one_way_use_bidirectional({}, *item.ref, *item.cell);
      }
   }
               
   float make_parenthood_bidirectional_worker::assess_load_progress() const noexcept {
      if (!this->progress.maximum)
         return 0.0F;
      return (float)this->progress.current / (float)this->progress.maximum;
   }
}