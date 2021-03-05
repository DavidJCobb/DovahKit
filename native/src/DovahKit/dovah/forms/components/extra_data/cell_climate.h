#pragma once
#include "_templates.h"

namespace dovah::loaded_forms::components::extra {
   class cell_climate : public formID_extra_data<'XCCM', extra_data_type::cell_climate> {
      // The form should be a REGN?
      public:
         /*static*/ void generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib, extra_data_use_info_state& state) {
            record.get_current_subrecord().read(state.by_name.cell_climate);
         }
   };
}