#pragma once
#include <tuple>
#include "./for_each_value.h"

namespace cobb::tuples {
   template<typename Tuple, typename Functor, typename Result>
   extern constexpr Result reduce(Tuple&& tuple, Functor&& functor, Result initial = Result{}) {
      Result r = initial;
      for_each_value(tuple, [&r, &functor]<typename T>(const T& current) {
         r = functor(r, current);
      });
      return r;
   }

   template<typename Result, typename Tuple, typename Functor>
   extern constexpr Result reduce(Tuple&& tuple, Functor&& functor) {
      Result r = {};
      for_each_value(tuple, [&r, &functor]<typename T>(const T& current) {
         r = functor(r, current);
      });
      return r;
   }
}
