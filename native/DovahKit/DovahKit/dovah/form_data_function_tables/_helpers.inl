#pragma once
#include <array>
#include <tuple>
#include "dovah/form_data/all_types.h"
#include "dovah/form_data/form_type_for_form_data_class.h"
#include "dovah/form_type_info.h"

namespace dovah::form_data_function_tables::impl {
   template<form_type V>
   constexpr const size_t index_of_form_type = []() {
      size_t result = (size_t)-1;
      size_t i = 0;
      all_form_data_types<managed_form_params>::for_each([&result, &i]<typename Current>() {
         if constexpr (form_type_for_form_data_class_v<Current> == V) {
            result = i;
         }
         ++i;
      });
      return result;
   }();

   template<form_type V>
   using form_data_class_for_form_type = all_form_data_types<managed_form_params>::nth_type<index_of_form_type<V>>;

   template<typename FunctionType>
   using table_type = std::array<FunctionType, std::tuple_size_v<decltype(form_types)>>;
}