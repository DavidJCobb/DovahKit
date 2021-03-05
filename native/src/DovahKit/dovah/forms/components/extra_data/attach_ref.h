#pragma once
#include "_templates.h"
#include "_use_info.h"

namespace dovah::loaded_forms::components::extra {
   class attach_ref : public formID_extra_data<'XATR', extra_data_type::attach_ref> {
      // The form should be a GLOB.
      public:
         static void generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib, extra_data_use_info_state& state) {
            record.get_current_subrecord().read(state.by_name.attach_ref);
         }
   };
}