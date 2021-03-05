#pragma once
#include "_templates.h"

namespace dovah::loaded_forms::components::extra {
   class emittance_source : public formID_extra_data<'XEMI', extra_data_type::emittance_source> {
      // The form should be a LIGH or REGN.
      public:
         /*static*/ void generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib, extra_data_use_info_state& state) {
            record.get_current_subrecord().read(state.by_name.emittance_source);
         }
   };
}