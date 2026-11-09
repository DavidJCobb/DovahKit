#pragma once
#include "lu/type_containers/type_array.h"
#include "./params.h"
#pragma region Form data types
   #pragma region F
      #include "./types/f/form_list.h"
   #pragma endregion
   #pragma region S
      #include "./types/s/shout.h"
   #pragma endregion
#pragma endregion

namespace dovah {
   template<form_data_params FormDataParams>
   using all_form_data_types = lu::type_containers::type_array<
      #pragma region F
         form_data::form_list<FormDataParams>,
      #pragma endregion
      #pragma region S
         form_data::shout<FormDataParams>//,
      #pragma endregion
   >;
}