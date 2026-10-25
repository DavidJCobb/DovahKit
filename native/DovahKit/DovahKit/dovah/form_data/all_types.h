#pragma once
#include "lu/type_containers/type_array.h"
#include "./params.h"
#pragma region Form data types
   #pragma region F
      //#include "./types/f/foo.h"
   #pragma endregion
#pragma endregion

namespace dovah {
   template<form_data_params FormDataParams>
   using all_form_data_types = lu::type_containers::type_array<
      // foo<FormDataParams>,
   >;
}