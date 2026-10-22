#pragma once
#include <cstdint>
namespace dovah {
   namespace load_order_processes::use_info_build {
      class use_info_builder;
   }
   namespace tes {
      class record_view;
   }
   enum class form_type : uint8_t;
}

namespace dovah::form_data_function_tables::build_outbound_use_info {
   using function_type = void(*)(tes::record_view&, load_order_processes::use_info_build::use_info_builder&);
   
   extern function_type lookup(form_type);
}