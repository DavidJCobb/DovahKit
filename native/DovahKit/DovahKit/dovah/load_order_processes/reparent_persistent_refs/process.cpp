#include "./process.h"
#include "lu/arrays/make_filled.h"
#include "dovah/form_stub.h"
#include "dovah/form_stub_addenda.h"
#include "dovah/form_stub_collection.h"
#include "dovah/form_type.h"
#include "dovah/use_info/entry_flag_to_mask.h"
#include "dovah/use_info/entry_flags/base.h"
#include "dovah/utils/form_types/is_ref.h"
#include "./queued_ref.h"
#include "./world_cell_map.h"
#include "./set_parent_cell_worker.h"
#include "./make_parenthood_bidirectional_worker.h"

namespace dovah::load_order_processes::reparent_persistent_refs {
   void process::execute(form_stub_collection& fsc) {
      fsc.for_each_form_of_type(dovah::form_type::worldspace, [this](dovah::form_stub& world) {
         auto* addenda = world.addenda;
         if (!addenda)
            return;
         auto* p_cell = addenda->persistent_cell;
         if (!p_cell)
            return;
         
         std::vector<queued_ref> refs;
         world_cell_map cells;
         cells.set_world(world);
         //
         // We need to bidirectionally sever the references from the persistent cell children 
         // to the persistent cell, and then bidirectionally create references from the 
         // persistent cell children to their true parent cells. However, making these changes 
         // as a single operation is impossible to multi-thread and, in general, very slow. 
         // Instead, let's break those operations apart.
         //
         // We'll start by both queuing multi-threaded changes to the cell children, and 
         // one-way-severing the inbound references to the persistent cell.
         //
         refs.reserve(p_cell->inbound.size());
         {
            decltype(p_cell->inbound) entries_to_keep;
            auto& src = p_cell->inbound;
            auto& dst = entries_to_keep;
            //
            // Here, we want to do two things:
            // 
            //  - Gather up a list of all refs to reparent.
            // 
            //  - Sever the inbound connections from  the references to the persistent cell.
            //
            // 99% of the time, we will be removing every single element from the persistent 
            // cell's inbound uses. I can't even think of any circumstance in which any form 
            // would actually refer to the persistent cell for any reason other than being a 
            // child REFR of that cell. The default case is removal.
            // 
            // That in turn  means that the fastest way to filter the inbound  use map is to 
            // create a new map, copy the few (usually no) elements we intend to preserve to 
            // that new map, and then swap the two maps. Extracting and reparenting nodes is 
            // slightly slower in practice, and  using the "erase" function on the container 
            // is the slowest approach of all (which makes sense; it would only be faster if 
            // the default code path was to retain, not remove, elements).
            //
            for (auto& pair : src) {
               auto& entry = pair.second;
               if (entry.flags & use_info::entry_flag_to_mask(use_info::entry_flags::base::parent)) {
                  auto* child = entry.other;
                  if (child && utils::form_types::is_ref(child->form_type)) {
                     refs.emplace_back(*child);
                     if (--entry.refcount == 0)
                        continue;
                  }
               }
               dst[pair.first] = entry;
            }
            std::swap(src, dst);
         }
         if (refs.empty()) {
            //
            // No persistent refs to process.
            //
            return;
         }
         //
         // So now, the persistent cell is no longer tracking that it's used by the refs, but 
         // the refs still track themselves as using the persistent cell. Let's change that.
         //
         {
            auto workers = lu::arrays::make_filled<set_parent_cell_worker, 4>(cells, refs);
            if (refs.size() > workers.size()) {
               size_t per_thread = refs.size() / workers.size();
               size_t start      = 0;
               for (size_t i = 0; i < workers.size() - 1; ++i) {
                  workers[i].set_ref_indices_to_process(start, start + per_thread);
                  start += per_thread;
               }
               workers.back().set_ref_indices_to_process(start, refs.size());
               for (auto& worker : workers)
                  worker.start();
               for (auto& worker : workers)
                  worker.wait_for();
            } else {
               auto& worker = workers[0];
               worker.set_ref_indices_to_process(0, refs.size());
               worker.start();
               worker.wait_for();
            }
         }
         //
         // Now, the refs track themselves as using the cells we want to re-parent them to, but 
         // those cells don't track themselves as being used by the refs. We need to make these 
         // new uses bidirectional.
         //
         {
            //
            // So normally, we wouldn't be able to multi-thread updating the used cells, because 
            // we can't be sure that multiple threads acting on unrelated refs wouldn't end up 
            // trying to touch the same cell at the same time. However, there's one property that 
            // we can take advantage of: we know each ref's cell grid coordinates.
            // 
            // We can divide the worldspace into rings, and task each worker to a ring.
            //
            auto workers = lu::arrays::make_filled<make_parenthood_bidirectional_worker, 8>(refs);

            const size_t max_radius = [&cells]() {
               auto x_span = cells.grid_extents.x.max - cells.grid_extents.x.min / 2;
               auto y_span = cells.grid_extents.y.max - cells.grid_extents.y.min / 2;
               if (x_span > y_span)
                  return x_span;
               return y_span;
            }();
            const size_t radius_per_worker = max_radius / workers.size();

            if (radius_per_worker <= 1) {
               auto& worker = workers[0];
               worker.run_on_this_thread();
            } else {
               const auto center       = cells.grid_extents.center();
               size_t     inner_radius = 0;
               for (size_t i = 0; i < workers.size(); ++i) {
                  auto& worker = workers[i];
                  worker.set_ring(center, inner_radius, inner_radius + radius_per_worker);
                  inner_radius += radius_per_worker;
               }
               for (auto& worker : workers)
                  worker.start();
               for (auto& worker : workers)
                  worker.wait_for();
            }
         }
         //
         // Done with this worldspace!
         //
      });
      //
      // Done with all worldspaces
      //
   }
}