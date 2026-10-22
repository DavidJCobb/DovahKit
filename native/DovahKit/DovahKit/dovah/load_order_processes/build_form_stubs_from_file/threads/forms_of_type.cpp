#include "./forms_of_type.h"
#include <cassert>
#include "dovah/exceptions/file_parse_errors/unexpected_nested_group_in_simple_top_group.h"
#include "dovah/form_type.h"
#include "dovah/form_type_info.h"
#include "dovah/tes/file.h"
#include "dovah/tes/reading/file_view.h"
#include "../extract_high_value_subrecords_for_stub.h"
#include "../make_stub_from_record.h"

namespace dovah::load_order_processes::build_form_stubs_from_file::threads {
   void forms_of_type::execute_on_thread() {
      assert(this->active_load_order);
      assert(this->source_file);

      auto& alo = *this->active_load_order;

      tes::file_view view;
      view.set_target_file(this->source_file);

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
                  *this->source_file,
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
               //
               auto* stub = make_stub_from_record(*this->source_file, record.file_offset_of_header(), record.header());
               if (stub->form_type != group_form_type) {
                  notices::file_load_warnings::record_found_in_wrong_top_level_group notice;
                  notice.source_file = this->source_file->path().filename().string();
                  notice.top_level_group_label = desired.signature;
                  notice.record = {
                     .local_id  = record.formID(),
                     .global_id = stub->form_id,
                     .signature = record.signature(),
                  };
                  //
                  this->get_file_loader().get_load_interface(*this).log_warning(notice);
               }
               if (!this->commit_stub(stub)) {
                  continue;
               }
               extract_high_value_subrecords_for_stub(*this->source_file, *stub, record);
            }
         }
      }
   }
}