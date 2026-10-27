#pragma once
#include <tuple>

namespace lu::tuples {
   namespace impl {
      template<typename Tuple, template<typename> typename TransformTrait>
      struct _map_types;

      template<template<typename> typename TransformTrait, typename... Elements>
      struct _map_types<std::tuple<Elements...>, TransformTrait> {
         using type = std::tuple<typename TransformTrait<Elements>::type...>;
      };
   }
   template<typename Tuple, template<typename> typename TransformTrait>
   using map_types = impl::_map_types<Tuple, TransformTrait>::type;
}
