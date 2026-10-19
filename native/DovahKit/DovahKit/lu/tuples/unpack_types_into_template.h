#pragma once
#include "../type_aliases/tuple_like_to_tuple_type.h"

namespace lu::tuples {
   namespace impl {
      template<typename Tuple, template<typename...> typename DstTemplate>
      struct _unpack_types_into_template;

      template<template<typename...> typename DstTemplate, typename... Elements>
      struct _unpack_types_into_template<std::tuple<Elements...>, DstTemplate> {
         using type = DstTemplate<Elements...>;
      };
   }

   template<typename TupleOrTupleLike, template<typename...> typename DstTemplate>
   using unpack_types_into_template = impl::_unpack_types_into_template<tuple_like_to_tuple_type<TupleOrTupleLike>, DstTemplate>::type;
}
