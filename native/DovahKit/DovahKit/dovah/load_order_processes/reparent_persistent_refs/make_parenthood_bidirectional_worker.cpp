#include "./make_parenthood_bidirectional_worker.h"
#include <bit>
#include <cstdint>
#include "dovah/_impl/form_stubs/make_one_way_use_bidirectional.h"
#include "dovah/termination_handler.h"
#include "./config/bidi_workers_per_quartet.h"
#include "./queued_ref.h"

namespace dovah::load_order_processes::reparent_persistent_refs {
   make_parenthood_bidirectional_worker::make_parenthood_bidirectional_worker(
      const std::vector<queued_ref>& all_refs,
      size_t quartet_index,
      bool   x_pos,
      bool   y_pos
   ) :
      all_refs(all_refs),
      quartet_index(quartet_index),
      x_pos(x_pos),
      y_pos(y_pos)
   {}

   void make_parenthood_bidirectional_worker::_execute() {
      termination_handler::update_this_thread();
      constexpr bool use_sign_masking = false;
      constexpr auto sign_bit_mask    = std::bit_cast<int32_t, uint32_t>(uint32_t(1 << 31));
      const uint32_t x_mask = this->x_pos ? 0 : sign_bit_mask;
      const uint32_t y_mask = this->y_pos ? 0 : sign_bit_mask;
      //
      const auto& list = this->all_refs;
      size_t start;
      size_t end;
      if constexpr (config::bidi_workers_per_quartet == 1) {
         start = 0;
         end   = list.size();
      } else {
         const size_t refs_per_quartet = list.size() / config::bidi_workers_per_quartet;
         start = this->quartet_index * refs_per_quartet;
         end   = (this->quartet_index == config::bidi_workers_per_quartet - 1) ? list.size() : start + refs_per_quartet;
      }
      //
      for (size_t i = start; i < end; ++i) {
         auto& item = list[i];
         ++this->progress.current;
         if (!item.cell)
            continue;
         if constexpr (use_sign_masking) {
            if ((item.grid_position.x & sign_bit_mask) != x_mask)
               continue;
            if ((item.grid_position.y & sign_bit_mask) != y_mask)
               continue;
         } else {
            if ((item.grid_position.x >= 0) != this->x_pos)
               continue;
            if ((item.grid_position.y >= 0) != this->y_pos)
               continue;
         }

         if (item.cell) {
            static_assert(false, "TODO: Wait... This isn't thread-safe. except by happenstance.");
            impl::_form_stubs::make_one_way_use_bidirectional({}, *item.ref, *item.cell);

            //
            // That isn't thread-safe except by happenstance. When we build the ref list, we go cell-by-cell, so 
            // USUALLY cells won't end up being handled by the same worker. But if a cell's refs end up split 
            // across two quartet workers, then we'd run into problems, wouldn't we??
            //

         }
      }
   }

   void make_parenthood_bidirectional_worker::start() noexcept {
      assert(!this->is_active());
      this->progress.current = 0;
      {
         const auto& list = this->all_refs;
         this->progress.maximum = list.size();
         if (this->quartet_index != config::bidi_workers_per_quartet - 1) {
            const size_t refs_per_quartet = list.size() / config::bidi_workers_per_quartet;
            this->progress.maximum = (this->quartet_index * refs_per_quartet) + refs_per_quartet;
         }
      }
      this->thread = std::thread(&_thread_handler, this);
   }
   void make_parenthood_bidirectional_worker::wait_for() noexcept {
      if (this->is_active())
         this->thread.join();
   }
               
   float make_parenthood_bidirectional_worker::assess_load_progress() const noexcept {
      if (!this->progress.maximum)
         return 0.0F;
      return (float)this->progress.current / (float)this->progress.maximum;
   }
}