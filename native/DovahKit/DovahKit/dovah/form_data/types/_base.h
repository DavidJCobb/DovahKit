#pragma once
#include <type_traits>
#include "../base_managed_form_data.h"
#include "../base_unmanaged_form_data.h"
#include "../params.h"

namespace dovah::form_data {
   template<form_data_params Params>
   using _base = std::conditional_t<
      (Params::management_mode == use_info::management_mode::managed),
      base_managed_form_data,
      base_unmanaged_form_data
   >;
}
namespace dovah::form_data_function_tables::create_blank_instance {
   template<form_type V>
   struct entry;
}