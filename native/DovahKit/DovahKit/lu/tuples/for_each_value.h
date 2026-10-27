#pragma once
#include <concepts>
#include <tuple>
#include <type_traits>
#include <utility>
#include "../concepts/lambda_invocable_with_nttp.h"
#include "../concepts/supports_tuple_interface.h"
#include "../foreach_lambda_result.h"
#include "../type_aliases/tuple_like_to_tuple_type.h"
#include "./elements_usable_as_lambda_args.h"
#include "./elements_usable_as_lambda_nttps.h"

namespace lu::tuples {
   #pragma region for_each_value(tuple, lambda)
      namespace impl::for_each_value {
         template<typename Tuple, typename Lambda>
         struct info_for_tuple_type {
            using usability = elements_usable_as_lambda_args<Tuple, Lambda>;

            static constexpr const bool all_invocable = usability::all_usable;
            static constexpr const bool all_return    = usability::template all_usable_r<foreach_lambda_result>;
            static constexpr const bool none_return   = usability::template none_usable_r<foreach_lambda_result>;
         };

         template<typename TupleLike, typename IS = std::make_index_sequence<std::tuple_size_v<std::decay_t<TupleLike>>>>
         struct invoke_helper_for_arg_tuple;
         template<typename TupleLike, auto... I>
         struct invoke_helper_for_arg_tuple<TupleLike, std::index_sequence<I...>> {
            template<size_t Index, typename Lambda> requires info_for_tuple_type<std::decay_t<TupleLike>, Lambda>::all_return
            static constexpr bool invoke_with_result(Lambda&& lambda, TupleLike&& tuple, foreach_lambda_result& result) {
               result = lambda(std::get<Index>(tuple));
               return result != foreach_lambda_result::stop;
            }

            template<typename Lambda>
            static constexpr void invoke(Lambda&& lambda, TupleLike&& tuple) {
               if constexpr (info_for_tuple_type<std::decay_t<TupleLike>, Lambda>::all_return) {
                  foreach_lambda_result result = foreach_lambda_result::keep_going;
                  (invoke_with_result<I>(std::forward<Lambda>(lambda), std::forward<TupleLike>(tuple), result) && ...);
               } else {
                  (lambda(std::get<I>(tuple)), ...);
               }
            }
         };

         template<typename Lambda, typename TupleOrTupleLike>
         concept lambda_valid_for_argument_tuple_like = requires {
            // Require that the tuple or tuple-like type... actually be that.
            requires concepts::supports_tuple_interface<std::decay_t<TupleOrTupleLike>>;

            // Require that `lambda(e)` be a valid expression for each element value `e` in an instance of 
            // the tuple or tuple-like type.
            typename info_for_tuple_type<std::decay_t<TupleOrTupleLike>, Lambda>;
            requires info_for_tuple_type<std::decay_t<TupleOrTupleLike>, Lambda>::all_invocable;

            // Require that for each element type `E` in the tuple or tuple-like type, invoking the lambda 
            // as described above will either always or never return a `foreach_lambda_result`.
            requires (info_for_tuple_type<std::decay_t<TupleOrTupleLike>, Lambda>::all_return || info_for_tuple_type<std::decay_t<TupleOrTupleLike>, Lambda>::none_return);
         };
      }

      template<typename TupleOrTupleLike, typename Lambda>
         requires impl::for_each_value::lambda_valid_for_argument_tuple_like<Lambda, TupleOrTupleLike>
      constexpr void for_each_value(TupleOrTupleLike&& tuple, Lambda&& lambda) {
         impl::for_each_value::invoke_helper_for_arg_tuple<TupleOrTupleLike>::template invoke(std::forward<Lambda>(lambda), std::forward<TupleOrTupleLike>(tuple));
      }
   #pragma endregion

   #pragma region for_each_value<tuple>(lambda)
      namespace impl::for_each_value {
         template<const auto& Tuple, typename Lambda>
         struct info_for_tuple_nttp {
            using usability = elements_usable_as_lambda_nttps<Tuple, Lambda>;

            static constexpr const bool all_invocable = usability::all_usable;
            static constexpr const bool all_return    = usability::template all_usable_r<foreach_lambda_result>;
            static constexpr const bool none_return   = usability::template none_usable_r<foreach_lambda_result>;
         };

         template<const auto& TupleLike, typename IS = std::make_index_sequence<std::tuple_size_v<std::decay_t<decltype(TupleLike)>>>>
         struct invoke_helper_for_nttp_tuple;
         template<const auto& TupleLike, auto... I>
         struct invoke_helper_for_nttp_tuple<TupleLike, std::index_sequence<I...>> {
            using tuple_type = std::decay_t<decltype(TupleLike)>;

            template<size_t Index, typename Lambda> requires info_for_tuple_nttp<TupleLike, Lambda>::all_return
            static constexpr bool invoke_with_result(Lambda&& lambda, foreach_lambda_result& result) {
               result = lambda.template operator()<std::get<Index>(TupleLike)>();
               return result != foreach_lambda_result::stop;
            }

            template<typename Lambda>
            static constexpr void invoke(Lambda&& lambda) {
               if constexpr (info_for_tuple_nttp<TupleLike, Lambda>::all_return) {
                  foreach_lambda_result result = foreach_lambda_result::keep_going;
                  (invoke_with_result<I>(std::forward<Lambda>(lambda), result) && ...);
               } else {
                  (lambda.template operator()<std::get<I>(TupleLike)>(), ...);
               }
            }
         };

         template<typename Lambda, const auto& TupleOrTupleLike>
         concept lambda_valid_for_nttp_tuple_like = requires {
            // Require that the tuple or tuple-like type... actually be that.
            requires concepts::supports_tuple_interface<std::decay_t<decltype(TupleOrTupleLike)>>;

            // Require that `lambda(e)` be a valid expression for each element value `e` in an instance of 
            // the tuple or tuple-like type.
            typename info_for_tuple_nttp<TupleOrTupleLike, Lambda>;
            requires info_for_tuple_nttp<TupleOrTupleLike, Lambda>::all_invocable;

            // Require that for each element type `E` in the tuple or tuple-like type, invoking the lambda 
            // as described above will either always or never return a `foreach_lambda_result`.
            requires (info_for_tuple_nttp<TupleOrTupleLike, Lambda>::all_return || info_for_tuple_nttp<TupleOrTupleLike, Lambda>::none_return);
         };
      }

      template<const auto& TupleLike, typename Lambda>
         requires impl::for_each_value::lambda_valid_for_nttp_tuple_like<Lambda, TupleLike>
      constexpr void for_each_value(Lambda&& lambda) {
         impl::for_each_value::invoke_helper_for_nttp_tuple<TupleLike>::template invoke(std::forward<Lambda>(lambda));
      }
   #pragma endregion
}
