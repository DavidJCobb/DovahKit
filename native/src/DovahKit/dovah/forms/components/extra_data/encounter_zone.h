#pragma once
#include "_templates.h"

namespace dovah::loaded_forms::components::extra {
   class encounter_zone : public formID_extra_data<'XEZN', extra_data_type::encounter_zone> {
      // The form should be a ECZN.
      public:
         /*static*/ void generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib, extra_data_use_info_state& state) {
            record.get_current_subrecord().read(state.by_name.encounter_zone);
         }
   };
}