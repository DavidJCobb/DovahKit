#pragma once
#include "lu/type_containers/type_array.h"
#include "./params.h"
#pragma region Form data types
   #pragma region S
      #include "./types/s/shout.h"
   #pragma endregion
#pragma endregion

namespace dovah {
   template<form_data_params FormDataParams>
   using all_form_data_types = lu::type_containers::type_array<
      #pragma region S
         form_data::shout<FormDataParams>//,
      #pragma endregion
   >;
}