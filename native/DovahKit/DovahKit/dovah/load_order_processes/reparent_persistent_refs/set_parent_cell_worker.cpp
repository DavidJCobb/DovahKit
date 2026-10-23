#include "./set_parent_cell_worker.h"
#include <cassert>
#include "dovah/form_stub.h"
#include "dovah/form_stub_addenda.h"
#include "dovah/load_order_interfaces/form_load.h"
#include "dovah/termination_handler.h"
#include "dovah/tes/reading/record_view.h"
#include "dovah/tes/reading/subrecord_view.h"
#include "./world_cell_map.h"

namespace dovah::load_order_processes::reparent_persistent_refs {
   void set_parent_cell_worker::_execute() {
      termination_handler::update_this_thread();
      for (size_t i = this->range.start; i < this->range.end; ++i) {
         auto& item = this->refs[i];
         auto& ref  = *item.ref;
         ref.for_each_source_record([this, &item](tes::record_view& record, dovah::load_order_interfaces::form_load& intfc) {
            if (!intfc.is_winning_record)
               return;
            float x = 0.0;
            float y = 0.0;
            while (auto& subrecord = record.next_subrecord()) {
               if (subrecord.signature() != 'DATA')
                  continue;
               if (!subrecord.is_in_bounds(8))
                  break;
               subrecord.unchecked_read(x);
               subrecord.unchecked_read(y);
               break;
            }
            form_stub* cell = this->cells.cell_for_position(x, y);
            if (cell) {
               assert(cell->addenda->grid_position.has_value());
               item.ref->_set_parent_form_one_way({}, *cell);
               item.cell          = cell;
               item.grid_position = cell->addenda->grid_position.value();
            } else {
               ++this->orphan_ref_count;
            }
         });
         ++this->progress.current;
      }
   }

   set_parent_cell_worker::set_parent_cell_worker(
      world_cell_map&          cells,
      std::vector<queued_ref>& refs
   ) : cells(cells), refs(refs) {
   }

   void set_parent_cell_worker::set_ref_indices_to_process(size_t start, size_t end) {
      this->range.start = start;
      this->range.end   = end;
   }
   void set_parent_cell_worker::start() noexcept {
      assert(!this->is_active());
      this->progress.current = 0;
      this->progress.maximum = this->range.end - this->range.start;
      this->thread = std::thread(&_thread_handler, this);
   }
   void set_parent_cell_worker::wait_for() noexcept {
      if (this->is_active())
         this->thread.join();
   }
               
   float set_parent_cell_worker::assess_load_progress() const noexcept {
      if (!this->progress.maximum)
         return 0.0F;
      return (float)this->progress.current / (float)this->progress.maximum;
   }
}