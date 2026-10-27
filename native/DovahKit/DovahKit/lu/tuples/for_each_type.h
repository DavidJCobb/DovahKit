#pragma once
#include <tuple>
#include "../foreach_lambda_result.h"
#include "../type_aliases/tuple_like_to_tuple_type.h"
#include "./elements_usable_as_lambda_template_params.h"

namespace lu::tuples {
   namespace impl::_for_each_type {
      template<typename TupleOrTupleLike, typename Lambda>
      struct invoke_helper;

      template<typename Lambda, typename... Elements>
      struct invoke_helper<std::tuple<Elements...>, Lambda> {
         using usability = elements_usable_as_lambda_template_params<std::tuple<Elements...>, Lambda>;

         static constexpr const bool all_invocable    = usability::all_usable;
         static constexpr const bool all_with_result  = usability::template all_usable_r<foreach_lambda_result>;
         static constexpr const bool none_with_result = usability::template none_usable_r<foreach_lambda_result>;

         template<typename E> requires invocable_with_result<E>
         static constexpr bool invoke_with_result(Lambda&& f, foreach_lambda_result& result) {
            result = f.template operator()<E>();
            return result != foreach_lambda_result::stop;
         }

         static constexpr void execute(Lambda&& f) requires (all_invocable) {
            if constexpr (all_with_result) {
               foreach_lambda_result result = foreach_lambda_result::keep_going;
               (invoke_with_result<Elements> && ...);
            } else {
               (f.template operator()<Elements>(), ...);
            }
         }
      };

      template<typename TupleOrTupleLike, typename Lambda>
      concept lambda_is_valid_for_tuple = requires {
         // Require that the tuple or tuple-like type... actually be that.
         typename tuple_like_to_tuple_type<TupleOrTupleLike>;

         // Require that the lambda have a non-static `operator()` member template taking no arguments.
         typename elements_usable_as_lambda_template_params<tuple_like_to_tuple_type<TupleOrTupleLike>, Lambda>;

         // Require that `lambda.template operator()<E>()` be a valid expression for each element type `E` 
         // in the tuple or tuple-like type.
         requires invoke_helper<tuple_like_to_tuple_type<TupleOrTupleLike>, Lambda>::all_invocable;

         // Require that for each element type `E` in the tuple or tuple-like type, invoking the lambda as 
         // described above will either always or never return a `foreach_lambda_result`.
         requires (
            invoke_helper<tuple_like_to_tuple_type<TupleOrTupleLike>, Lambda>::all_with_result
         || invoke_helper<tuple_like_to_tuple_type<TupleOrTupleLike>, Lambda>::none_with_result
         );
      };
   }

   template<typename TupleOrTupleLike, typename Lambda>
      requires impl::_for_each_type::lambda_is_valid_for_tuple<TupleOrTupleLike, std::decay_t<Lambda>>
   constexpr void for_each_type(Lambda&& lambda) {
      impl::_for_each_type::invoke_helper<tuple_like_to_tuple_type<TupleOrTupleLike>, std::decay_t<Lambda>>::execute(std::forward<Lambda>(lambda));
   }
}
