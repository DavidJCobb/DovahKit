#pragma once

namespace lu {
   template<typename T>
   struct templated_call_operator {
      template<typename... Params>
      using type = decltype(&T::template operator()<Params...>);

      template<auto... Params>
      using nttp_type = decltype(&T::template operator()<Params...>);
   };

   template<typename T, typename... Params>
   using templated_call_operator_t = typename templated_call_operator<T>::template type<Params...>;
}
