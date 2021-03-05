#pragma once
#include "_templates.h"

namespace dovah::loaded_forms::components::extra {
   class location_ref_type : public formID_extra_data<'XLRT', extra_data_type::location_ref_type> {
      // The form should be an LCRT.
      public:
         /*static*/ void generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib, extra_data_use_info_state& state) {
            record.get_current_subrecord().read(state.by_name.location_ref_type);
         }
   };
}