#pragma once
#include "_templates.h"

namespace dovah::loaded_forms::components::extra {
   class horse : public formID_extra_data<'XHOR', extra_data_type::horse> { // usually appears on ACHR, not other REFRs
      // The form should be an ACHR.
      public:
         static void generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib, extra_data_use_info_state& state) {
            record.get_current_subrecord().read(state.by_name.horse);
         }
   };
}