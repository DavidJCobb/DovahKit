#pragma once
#include "_templates.h"

namespace dovah::loaded_forms::components::extra {
   class teleport_name : public formID_extra_data<'XTNM', extra_data_type::teleport_name> {
      // The form should be a MESG.
      public:
         /*static*/ void generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib, extra_data_use_info_state& state) {
            record.get_current_subrecord().read(state.by_name.teleport_name);
         }
   };
}