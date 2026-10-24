#pragma once
#include "dovah/callback_sets/data_load.h"
#include "dovah/form_stub.h"
#include "dovah/form_stub_addenda.h"
#include "dovah/form_type.h"
#include "dovah/tes/file.h"
#include "dovah/utils/form_stubs/get_parent_form.h"
#include "dovah/utils/form_stubs/is_interior_cell.h"

#include "dovah/notices/file_parse_warnings/esl_defined_interior_cell_is_overridden.h"
#include "dovah/notices/file_parse_warnings/partial_info_override_has_different_parent.h"

namespace dovah::load_order_processes::build_form_stubs_from_file {
   inline void fold_overriding_stub_into_overridden_stub(
      const callback_sets::data_load& callbacks,
      const tes::file&                overriding_file,
      const bool                      overriding_file_is_active,
      form_stub&   overriding_stub,
      form_stub&   overridden_stub,
      size_t       override_record_offset,
      bare_form_id override_record_id
   ) {
      form_stub* new_parent = utils::form_stubs::get_parent_form(overriding_stub); // set by caller before calling us

      if (overriding_stub.form_type == form_type::topic_info) {
         //
         // If the stub is a topic info and it's being re-parented by an override, then we need to 
         // update the form stub addenda for its old parent.
         //
         auto* old_parent = utils::form_stubs::get_parent_form(overridden_stub);
         if (old_parent && old_parent != new_parent) {
            if (auto* addenda = old_parent->addenda) {
               addenda->ordered_children._remove_child_on_load({}, overriding_file_is_active, overridden_stub);
            }
            if (callbacks.on_warning) {
               if (overriding_stub.test_record_flags((uint32_t)1 << (size_t)tes::record_flag::partial)) {
                  //
                  // A partial INFO override that re-parents the info? Oh, the game won't handle this 
                  // sanely at all. Refer to <topic infos' placement in topics' info lists.txt> in our 
                  // internal documentation for details on the chaos.
                  //
                  (callbacks.on_warning)(
                     notices::file_parse_warnings::partial_info_override_has_different_parent(
                        overriding_file.path().filename().string(),
                        override_record_id,
                        overridden_stub,
                        new_parent,
                        old_parent
                     )
                  );
               }
            }
         }
      } else if (overriding_stub.form_type == form_type::cell) {
         if (overriding_stub.test_record_flags((uint32_t)1 << (size_t)tes::record_flag::persistent)) {
            auto* old_parent = utils::form_stubs::get_parent_form(overridden_stub);
            if (old_parent && old_parent != new_parent) { // Re-parenting cells isn't supported and isn't a sane operation, but no harm in covering it here
               auto* addenda = old_parent->addenda;
               if (addenda && addenda->persistent_cell == &overridden_stub)
                  addenda->persistent_cell = nullptr;
            }
         }
         if (callbacks.on_warning) {
            if (utils::form_stubs::is_interior_cell(overridden_stub)) {
               auto* overridden_file_ptr = overridden_stub.get_file_at_index(0);
               if (overridden_file_ptr && overriding_file.is_light()) {
                  (callbacks.on_warning)(
                     notices::file_parse_warnings::esl_defined_interior_cell_is_overridden(
                        overriding_file.path().filename().string(),
                        override_record_id,
                        overridden_stub
                     )
                  );
               }
            }
         }
      }
      assert(!overriding_stub.has_multiple_source_files()); // the input stub should've been read by ONE file
      overridden_stub._add_source_file({}, const_cast<tes::file&>(overriding_file), override_record_offset, overriding_stub.file.flags);
      overridden_stub._set_parent_form_one_way({}, new_parent);
   }
}