#include "./forms_of_type.h"
#include "dovah/callback_sets/data_load.h"
#include "dovah/form_stub.h"
#include "dovah/form_type.h"
#include "dovah/form_type_info.h"
#include "dovah/tes/file.h"
#include "dovah/tes/reading/file_view.h"
#include "../make_stub_from_record.h"
#include "../process.h"
#include "./impl/fail_if_form_fatally_misplaced.h"

#include "dovah/exceptions/file_parse_errors/unexpected_nested_group.h"
#include "dovah/notices/file_parse_warnings/record_found_in_wrong_group.h"

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

         const auto&     group_of_interest = view.groups[0];
         const form_type group_form_type   = form_type_info::signature_to_form_type(desired.signature);
         assert(group_form_type != form_type::none);

         tes::file_view::chunk_type ot;
         while (ot = view.next_record_or_group(), ot != tes::file_view::chunk_type::none) {
            if (!group_of_interest.signature) {
               //
               // We've reached the end of the desired group.
               //
               break;
            }
            if (ot == tes::file_view::chunk_type::group) {
               throw exceptions::file_parse_errors::unexpected_nested_group(
                  source_file,
                  group_of_interest.position,
                  group_of_interest,
                  view.groups[1].position,
                  view.groups[1]
               );
            }
            if (ot == tes::file_view::chunk_type::record) {
               auto& record = view.subviews.record;
               auto* stub   = make_stub_from_record(source_file, record.file_offset_of_header(), record.header());
               if (stub->form_type != group_form_type) {
                  impl::fail_if_form_fatally_misplaced(this->_config, record, *stub);
                  if (callbacks.on_warning) {
                     (callbacks.on_warning)(
                        notices::file_parse_warnings::record_found_in_wrong_group(
                           source_file.path().filename().string(),
                           group_of_interest.position,
                           group_of_interest,
                           nullptr,
                           record.file_offset_of_header(),
                           record.header(),
                           stub
                        )
                     );
                  }
               }
               this->_finish_stub(std::move(stub), record);
            }
         }
      }
   }
}