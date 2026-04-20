#pragma once
#include "./property_definition.h"
#include <tuple>
#include "helpers/tuples/all_types_match_functor.h"
#include "helpers/type_traits/is_std_tuple.h"

namespace dovahscript::api_helpers::subobject_property_helpers {
   template<typename T>
   concept is_property_tuple_type = requires {
      requires cobb::is_std_tuple<T>;
      #ifndef __INTELLISENSE__ // 04/20/2026: IntelliSense hates `all_types_match_functor`
         requires cobb::tuples::all_types_match_functor<T, []<typename U>() {
            return impl::is_property_definition_specialization<U>;
         }>;
      #endif
   };
}