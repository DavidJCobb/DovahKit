#include "./persistent_cell_children.h"
#include "dovah/active_load_order.h"
#include "dovah/callback_sets/data_load.h"
#include "dovah/form_stub.h"
#include "dovah/form_stub_collection.h"
#include "dovah/form_type.h"
#include "dovah/tes/file.h"
#include "dovah/tes/reading/file_view.h"
#include "dovah/utils/form_types/is_cell_child.h"
#include "../impl/fail_if_form_fatally_misplaced.h"
#include "../make_stub_from_record.h"

#include "dovah/exceptions/file_parse_errors/unexpected_nested_group.h"
#include "dovah/notices/file_parse_warnings/record_found_in_wrong_group.h"

namespace dovah::load_order_processes::build_form_stubs_from_file::threads {
   void persistent_cell_children::execute_on_thread() {
      auto& alo         = this->_config.active_load_order;
      auto& callbacks   = this->_config.callbacks;
      auto& source_file = this->_config.source_file;

      tes::file_view view;
      view.set_target_file(&source_file);

      // Skipped: top-level group, type 0, label WRLD    (overarching group for worldspaces)
      // Skipped: nested    group, type 1                (overarching group for child cells)
      const auto& all_children_group     = view.groups[0]; // "Cell Children"
      const auto& children_of_type_group = view.groups[1]; // "Cell Persistent Children" or "Cell Temporary Children"

      const size_t size = this->queue.size();
      this->_progress.maximum = (uint32_t)size;
      for (uint32_t i = 0; i < size; i++) {
         this->_progress.current = i;
         auto& desired = this->queue[i];
         view.set_position(desired.pos);
         assert(view.next_record_or_group() == tes::file_view::chunk_type::group); // opens `sub_block_group`
         assert(view.get_current_group()    == &all_children_group);

         tes::file_view::chunk_type ot;
         while (ot = view.next_record_or_group(), ot != tes::file_view::chunk_type::none) {
            if (!all_children_group.signature) {
               //
               // We've reached the end of this queued-for-parse group.
               //
               break;
            }

            if (ot == tes::file_view::chunk_type::group) {
               auto& current_group = *view.get_current_group();
               if (&current_group == &all_children_group) {
                  //
                  // We've reached the end of this queued-for-parse group.
                  //
                  break;
               }
               auto& opened_from = *view.parent_of_group(current_group);

               bool valid = false;
               if (&opened_from == &all_children_group) [[likely]] {
                  switch (current_group.type) {
                     case tes::group_type::cell_persistent_children:
                     case tes::group_type::cell_temporary_children:
                        valid = true;
                        break;
                  }
               }
               if (valid) {
                  if (opened_from.label != current_group.label) [[unlikely]] {
                     //
                     // The outer "children" group and the inner "children" group specify different 
                     // parent cell IDs from one another. We can't be sure which cell any child forms 
                     // are actually meant to go in, so treat this as a hard error.
                     //
                     valid = false;
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
               //
               // This record should be a cell child form.
               //
               impl::fail_if_form_fatally_misplaced<form_type::cell>(source_file, record, *stub);
               const bool is_misplaced_form_type = !utils::form_types::is_cell_child(stub->form_type);
               if (callbacks.on_warning) {
                  const tes::file_view::opened_group* misplaced_in = nullptr;
                  if (children_of_type_group.signature) {
                     if (is_misplaced_form_type) {
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
                           desired.cell,
                           record.file_offset_of_header(),
                           record.header(),
                           stub
                        )
                     );
                  }
               }
               if (!is_misplaced_form_type) {
                  this->_set_stub_parent(*stub, desired.cell);
               }
               this->_finish_stub(std::move(stub), record);
            }

            // else unreachable
         }
      }
   }
}