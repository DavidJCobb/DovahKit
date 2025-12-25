#include "./attach_ref.h"
#include "../../../../_common_cpp.h"
#include "../../use_info_state.h"

namespace dovah::loaded_forms::components::extra_data_types {
   /*static*/ void attach_ref::generate_use_info(tes_file_reading::record& record, form_stub_use_info_builder& uib, extra_data_use_info_state& uis) {
      record.get_current_subrecord().read(uis.form_ids.by_name.attach_ref);
   }
}