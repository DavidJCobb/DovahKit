#include "./exterior_cells_and_children.h"
#include "dovah/active_load_order.h"
#include "dovah/callback_sets/data_load.h"
#include "dovah/form_stub.h"
#include "dovah/form_stub_collection.h"
#include "dovah/form_type.h"
#include "dovah/tes/file.h"
#include "dovah/tes/reading/file_view.h"
#include "dovah/utils/form_types/is_cell_child.h"
#include "../impl/fail_if_form_fatally_misplaced.h"
#include "../impl/get_parent_stub_for_group.h"
#include "../make_stub_from_record.h"

#include "dovah/exceptions/file_parse_errors/child_form_group_follows_wrong_parent.h"
#include "dovah/exceptions/file_parse_errors/unexpected_nested_group.h"
#include "dovah/notices/file_parse_warnings/record_found_in_wrong_group.h"

namespace dovah::load_order_processes::build_form_stubs_from_file::threads {
   void exterior_cells_and_children::execute_on_thread() {
      auto& alo         = this->_config.active_load_order;
      auto& callbacks   = this->_config.callbacks;
      auto& source_file = this->_config.source_file;

      tes::file_view view;
      view.set_target_file(&source_file);

      // Skipped: top-level group, type 0, label WRLD    (overarching group for worldspaces)
      // Skipped: nested    group, type 1                (overarching group for child cells)
      // Skipped: nested    group, type 4, label <block> (block of exterior cells)
      const auto& sub_block_group        = view.groups[0]; // exterior cells by sub-block; CELL records will be in here
      const auto& all_children_group     = view.groups[1]; // "Cell Children"
      const auto& children_of_type_group = view.groups[2]; // "Cell Persistent Children" or "Cell Temporary Children"

      const size_t size = this->queue.size();
      this->_progress.maximum = (uint32_t)size;
      for (uint32_t i = 0; i < size; i++) {
         this->_progress.current = i;
         auto& queue_item = this->queue[i];
         view.set_position(queue_item.pos);
         assert(view.next_record_or_group() == tes::file_view::chunk_type::group); // opens `sub_block_group`
         assert(view.get_current_group()    == &sub_block_group);

         form_stub* cell_stub_for_group   = nullptr;
         form_stub* last_seen_cell_record = nullptr;

         tes::file_view::chunk_type ot;
         while (ot = view.next_record_or_group(), ot != tes::file_view::chunk_type::none) {
            if (!sub_block_group.signature) {
               //
               // We've reached the end of this queued-for-parse group.
               //
               break;
            }

            if (ot == tes::file_view::chunk_type::group) {
               auto& current_group = *view.get_current_group();
               if (&current_group == &sub_block_group) {
                  //
                  // We've reached the end of this queued-for-parse group.
                  //
                  break;
               }
               auto& opened_from = *view.parent_of_group(current_group);

               bool valid = false;
               if (&opened_from == &sub_block_group) {
                  valid = current_group.type == tes::group_type::cell_children;
               } else if (&opened_from == &all_children_group) {
                  switch (current_group.type) {
                     case tes::group_type::cell_persistent_children:
                     case tes::group_type::cell_temporary_children:
                        valid = true;
                        break;
                  }
               }
               if (valid) {
                  if (current_group.type == tes::group_type::cell_children) {
                     cell_stub_for_group = &impl::get_parent_stub_for_group<form_type::cell>(this->_config, current_group.position, current_group);
                     if (cell_stub_for_group != last_seen_cell_record) [[unlikely]] {
                        //
                        // The outer "children" group specifies a parent cell ID that doesn't correspond 
                        // to the immediately preceding CELL record. There's no guarantee that the parent 
                        // cell it specifies has already had its record parsed and indexed as a form stub, 
                        // so we can't reliably load the child forms therein. Treat this as a hard error.
                        //
                        throw exceptions::file_parse_errors::child_form_group_follows_wrong_parent(
                           source_file,
                           current_group.position,
                           current_group,
                           last_seen_cell_record
                        );
                     }
                  } else {
                     if (opened_from.label != current_group.label) [[unlikely]] {
                        //
                        // The outer "children" group and the inner "children" group specify different 
                        // parent cell IDs from one another. We can't be sure which cell any child forms 
                        // are actually meant to go in, so treat this as a hard error.
                        //
                        valid = false;
                     }
                  }
               }
               if (!valid) {
                  throw exceptions::file_parse_errors::unexpected_nested_group(
                     source_file,
                     opened_from.position,
                     opened_from,
                     current_group.position,
                     current_group
                  );
               }
               continue;
            }

            if (ot == tes::file_view::chunk_type::record) {
               auto& record = view.subviews.record;
               auto* stub   = make_stub_from_record(source_file, record.file_offset_of_header(), record.header());
               const bool is_cell_child = utils::form_types::is_cell_child(stub->form_type);
               if (all_children_group.signature) {
                  //
                  // This record should be a cell child form.
                  //
                  impl::fail_if_form_fatally_misplaced<form_type::cell>(source_file, record, *stub);
                  assert(!!cell_stub_for_group);
                  if (callbacks.on_warning) {
                     const tes::file_view::opened_group* misplaced_in = nullptr;
                     if (children_of_type_group.signature) {
                        if (!is_cell_child) [[unlikely]] {
                           misplaced_in = &children_of_type_group;
                        } else {
                           const bool is_persistent = record.header().flags.has_all_of<tes::record_flag::persistent>();
                           const bool in_persistent = children_of_type_group.type == tes::group_type::cell_persistent_children;
                           if (is_persistent != in_persistent) {
                              misplaced_in = &children_of_type_group;
                           }
                        }
                     } else {
                        //
                        // Cell child records should not be placed directly in the "Cell Children" group; they 
                        // should go in the nested "Cell Persistent Children" or "Cell Temporary Children" groups.
                        //
                        misplaced_in = &all_children_group;
                     }
                     if (misplaced_in) {
                        (callbacks.on_warning)(
                           notices::file_parse_warnings::record_found_in_wrong_group(
                              source_file.path().filename().string(),
                              (*misplaced_in).position,
                              (*misplaced_in),
                              cell_stub_for_group,
                              record.file_offset_of_header(),
                              record.header(),
                              stub
                           )
                        );
                     }
                  }
                  if (is_cell_child) {
                     this->_set_stub_parent(*stub, cell_stub_for_group);
                  }
               } else {
                  //
                  // This record should be a cell.
                  //
                  if (stub->form_type != dovah::form_type::cell) {
                     impl::fail_if_form_fatally_misplaced<form_type::worldspace>(source_file, record, *stub);
                     last_seen_cell_record = nullptr;
                  } else {
                     //
                     // TODO: Warn if cell record ID doesn't match block/sub-block numbers.
                     //
                     last_seen_cell_record = stub;
                     this->_set_stub_parent(*stub, queue_item.worldspace);
                  }
               }
               this->_finish_stub(std::move(stub), record);
            }

            // else unreachable
         }
      }
   }
}