#pragma once
#include "_templates.h"

namespace dovah::loaded_forms::components::extra {
   class leveled_item_base : public formID_extra_data<'LVLI', extra_data_type::leveled_item_base> {
      // The form should be an LVLI.
      public:
         /*static*/ void generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib, extra_data_use_info_state& state) {
            record.get_current_subrecord().read(state.by_name.leveled_item_base);
         }
   };
}