#pragma once
#include "helpers/function_traits.h"
#include "./fields/prepare_for_insertion.h"

namespace dovahscript::api_helpers::native_lists::impl {
   template<typename Spec>
   struct preparation_list_type {
      struct type {};
   };
   template<typename Spec> requires fields::prepare_for_insertion::present<Spec>
   struct preparation_list_type<Spec> {
      using type = typename cobb::function_traits<decltype(&Spec::prepare_for_insertion)>::return_type;
   };
   template<typename Spec>
   using preparation_list_type_t = typename preparation_list_type<Spec>::type;

   template<typename Spec>
   struct preparation_value_type {
      struct type {};
   };
   template<typename Spec> requires fields::prepare_for_insertion::present<Spec>
   struct preparation_value_type<Spec> {
      using type = typename preparation_list_type<Spec>::value_type;
   };
   template<typename Spec>
   using preparation_value_type_t = typename preparation_value_type<Spec>::type;
}