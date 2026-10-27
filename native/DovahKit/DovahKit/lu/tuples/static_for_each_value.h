#pragma once
#include <tuple>
#include <type_traits>
#include <utility>
#include "../concepts/lambda_invocable_with_nttp.h"
#include "../concepts/supports_tuple_interface.h"
#include "../foreach_lambda_result.h"

namespace lu::tuples {
   namespace impl::static_for_each_value {
      template<typename T>
      struct helper;

      template<auto... I>
      struct helper<std::index_sequence<I...>> {
         //
         // 04/26/2026: It is tempting to have `helper` take the tuple, lambda, and argument types as 
         // parameters, so that each of its members don't have to take them. It would even be legal to 
         // do something like
         // 
         //    template<const auto& Tuple, auto Lambda, typename IS = std::make_index_sequence<blah blah blah>>
         //    struct helper;
         // 
         //    template<const auto& Tuple, auto Lambda, auto... I>
         //    struct helper<Tuple, Lambda, std::index_sequence<I...> {
         //       // ...
         //    };
         // 
         // Unfortunately, IntelliSense/EDG chokes, blows up, and dies when I attempt any of the above 
         // improvements, so we're stuck with this clunkiness.
         //

         template<const auto& Tuple, typename Lambda, typename... Args>
         static constexpr const bool invocable = (concepts::lambda_invocable_with_nttp<Lambda, std::get<I>(Tuple), Args...> && ...);

         template<const auto& Tuple, typename Lambda, typename... Args>
         static constexpr const bool always_returns_value = (concepts::lambda_invocable_with_nttp_r<Lambda, std::get<I>(Tuple), foreach_lambda_result, Args...> && ...);

         template<const auto& Tuple, typename Lambda, typename... Args>
         static constexpr const bool never_returns_value = (!concepts::lambda_invocable_with_nttp_r<Lambda, std::get<I>(Tuple), foreach_lambda_result, Args...> && ...);

         template<const auto& Tuple, typename Lambda, typename... Args>
         static constexpr const bool valid = invocable<Tuple, Lambda, Args...> && (always_returns_value<Tuple, Lambda, Args...> || never_returns_value<Tuple, Lambda, Args...>);

         template<auto Lambda, const auto& Value, typename... Args>
         static constexpr bool invoke_with_result(foreach_lambda_result& result, Args&&... args) {
            result = Lambda.template operator()<Value>(std::forward<Args>(args)...);
            return result != foreach_lambda_result::stop;
         }

         template<const auto& Tuple, auto Lambda, typename... Args>
         static constexpr void execute(Args&&... args) {
            using lambda_type = std::decay_t<decltype(Lambda)>;
            if constexpr (always_returns_value<Tuple, lambda_type, Args...>) {
               foreach_lambda_result result = foreach_lambda_result::keep_going;
               //
               // We need a separate function, `invoke_with_result`, because an equivalent fold expression without one 
               // would take the form `(((result = func()), (result != stop)) && ...)`. The inner comma in the to-be-
               // folded expression confuses IntelliSense/EDG as of this writing (04/26/2026).
               //
               (invoke_with_result<Lambda, std::get<I>(Tuple)>(result, std::forward<Args>(args)...) && ...);
            } else {
               (Lambda.template operator()<std::get<I>(Tuple)>(std::forward<Args>(args)...), ...);
            }
         }
      };

      template<const auto& Tuple>
      using sequence = std::make_index_sequence<std::tuple_size_v<std::decay_t<decltype(Tuple)>>>;
      template<const auto& Tuple>
      using helper_type = helper<sequence<Tuple>>;
   }

   template<const auto& Tuple, auto Lambda, typename... Args>
      requires (
         concepts::supports_tuple_interface<std::decay_t<decltype(Tuple)>>
         #ifndef __INTELLISENSE__ // 04/26/2026: IntelliSense/EDG chokes on this, puts red squiggles on random pieces of text in this file, and generally pukes itself to death
            && impl::static_for_each_value::helper_type<Tuple>::template valid<Tuple, std::decay_t<decltype(Lambda)>, Args...>
         #endif
      )
   constexpr void static_for_each_value(Args&&... args) {
      impl::static_for_each_value::helper_type<Tuple>::template execute<Tuple, Lambda>(std::forward<Args>(args)...);
   }
}
