#pragma once
#include <tuple>
#include "../concepts/supports_std_tuple_element.h"
#include "../concepts/supports_std_tuple_size.h"
#include "../type_traits/is_std_tuple.h"

namespace lu {
   namespace impl {
      template<typename TupleLike>
      struct _tuple_like_to_tuple_type {
         template<typename IS>
         struct helper;
         template<auto... I>
         struct helper<std::index_sequence<I...>> {
            using type = std::tuple<std::tuple_element_t<I, TupleLike>...>;
         };

         using type = typename helper<std::make_index_sequence<std::tuple_size_v<TupleLike>>>::type;
      };

      template<typename Tuple> requires is_std_tuple_v<Tuple>
      struct _tuple_like_to_tuple_type<Tuple> {
         using type = Tuple;
      };
   }

   template<typename TupleOrTupleLike>
      requires (concepts::supports_std_tuple_size<TupleOrTupleLike> && concepts::supports_std_tuple_element<TupleOrTupleLike>)
   using tuple_like_to_tuple_type = impl::_tuple_like_to_tuple_type<TupleOrTupleLike>::type;
}