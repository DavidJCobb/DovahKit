#pragma once
#include <cstdint>
#include "../extra_data.h"
#include "../../../form_stub_use_info_builder.h"

namespace dovah::loaded_forms::components {
   extern basic_extra_data* create_extra_data_by_type(extra_data_type);
   extern basic_extra_data* create_extra_data_for_subrecord(tes_subrecord_reader&);
   extern extra_data_load_result generate_extra_data_use_info(tes_record_reader&, form_stub_use_info_builder&, extra_data_use_info_state&); // call when a subrecord is already open
   extern extra_data_type get_extra_data_type_for_subrecord(uint32_t signature);
}