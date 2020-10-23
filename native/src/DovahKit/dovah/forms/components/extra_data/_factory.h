#pragma once
#include "../extra_data.h"

namespace dovah::loaded_forms::components {
   basic_extra_data* create_extra_data_by_type(extra_data_type);
   basic_extra_data* create_extra_data_for_subrecord(tes_subrecord_reader&);
   extra_data_load_result generate_extra_data_use_info(tes_record_reader&, form_stub_use_info_builder&); // call when a subrecord is already open
}