#pragma once
#include <tuple>
#include <type_traits>
#include <utility>

namespace cobb::tuples {
   namespace impl {
      template<typename T>
      struct for_each_nttp_value;

      template<auto... I>
      struct for_each_nttp_value<std::index_sequence<I...>> {
         template<const auto& Tuple, auto Functor, typename... Args>
         static void execute(Args&&... args) {
            (
               (Functor.template operator()<std::get<I>(Tuple)>(std::forward<Args>(args)...)),
               ...
            );
         }
      };
   }

   template<const auto& Tuple, auto Functor, typename... Args>
   void for_each_nttp_value(Args&&... args) {
      using tuple_type    = std::decay_t<decltype(Tuple)>;
      using sequence_type = std::make_index_sequence<std::tuple_size_v<tuple_type>>;

      impl::for_each_nttp_value<sequence_type>::template execute<Tuple, Functor>(std::forward<Args>(args)...);
   }
}