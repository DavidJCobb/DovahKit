#pragma once
#include <type_traits>
#include "lu/passkey.h" // for blank-instance constructor
#include "dovah/form_data/helpers/type_aliases.h"
#include "dovah/form_type.h"
#include "../base_managed_form_data.h"
#include "../base_unmanaged_form_data.h"
#include "../params.h"
namespace dovah {
   namespace load_order_interfaces {
      class form_load;
   }
   namespace load_order_processes::use_info_build {
      class use_info_builder;
   }
   namespace tes {
      class record_view;
   }
}

namespace dovah::form_data {
   template<form_data_params Params>
   using _base = std::conditional_t<
      (Params::management_mode == use_info::management_mode::managed),
      base_managed_form_data,
      base_unmanaged_form_data
   >;
}

namespace dovah::form_data_function_tables::build_outbound_use_info {
   template<form_type>
   struct entry;
}
namespace dovah::form_data_function_tables::create_blank_instance {
   template<form_type>
   struct entry;
}