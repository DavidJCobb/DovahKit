#include "./forms_of_type.h"
#include <cassert>
#include "dovah/active_load_order.h"
#include "dovah/form_type.h"
#include "dovah/form_type_info.h"
#include "dovah/tes/file.h"
#include "dovah/tes/reading/file_view.h"
#include "../extract_high_value_subrecords_for_stub.h"
#include "../make_stub_from_record.h"

#include "dovah/exceptions/file_parse_errors/unexpected_nested_group_in_simple_top_group.h"
#include "dovah/notices/file_parse_warnings/record_found_in_wrong_top_level_group.h"

namespace dovah::load_order_processes::build_form_stubs_from_file::threads {
   void forms_of_type::execute_on_thread() {
      auto& alo         = this->_config.active_load_order;
      auto& callbacks   = this->_config.callbacks;
      auto& source_file = this->_config.source_file;

      tes::file_view view;
      view.set_target_file(&source_file);

      const size_t size = this->queue.size();
      this->_progress.maximum = (uint32_t)size;
      for (uint32_t i = 0; i < size; i++) {
         this->_progress.current = i;
         auto& desired = this->queue[i];
         view.set_position(desired.pos);
         assert(view.next_record_or_group() == tes::file_view::chunk_type::group); // TODO: error instead

         const form_type group_form_type = form_type_info::signature_to_form_type(desired.signature);
         assert(group_form_type != form_type::none);

         tes::file_view::chunk_type ot;
         uint32_t    lastSignature  = 0; // shortcut to reduce the number of form type lookups we need
         form_type   lastFormType   = dovah::form_type::none;
         while (ot = view.next_record_or_group(), ot != tes::file_view::chunk_type::none) {
            if (!view.groups[0].signature) {
               //
               // We've reached the end of the desired group.
               //
               break;
            }
            if (ot == tes::file_view::chunk_type::group) {
               throw exceptions::file_parse_errors::unexpected_nested_group_in_simple_top_group(
                  source_file,
                  desired.pos,
                  desired.signature,
                  view.groups[1].position,
                  view.groups[1]
               );
            }
            if (ot == tes::file_view::chunk_type::record) {
               auto& record = view.subviews.record;
               if (record.signature() != lastSignature) {
                  lastSignature = record.signature();
                  lastFormType  = form_type_info::signature_to_form_type(lastSignature);
               }
               form_type formType = lastFormType;
               if (formType == dovah::form_type::none) // probably shouldn't happen; invalid signatures should cause errors
                  continue;
               
               auto* stub = make_stub_from_record(source_file, record.file_offset_of_header(), record.header());
               if (stub->form_type != group_form_type) {
                  if (callbacks.on_warning) {
                     (callbacks.on_warning)(
                        notices::file_parse_warnings::record_found_in_wrong_top_level_group(
                           source_file.path().filename().string(),
                           group_form_type,
                           stub->form_type,
                           record.formID(),
                           stub->form_id
                        )
                     );
                  }
               }
               try {
                  alo.get_form_stub_collection()._take_newly_parsed_form_stub(
                     callbacks,
                     source_file,
                     record.file_offset_of_header(),
                     record.header(),
                     stub
                  );
               } catch (...) {
                  delete stub;
                  throw;
               }
               if (!stub)
                  continue;
               extract_high_value_subrecords_for_stub(source_file, *stub, record);
            }
         }
      }
   }
}