#pragma once
#include <utility>
#include "../index_sequences/incremented_by.h"
#include "../tuples/pick_types_by_indices.h"
#include "../tuples/unpack_types_into.h"

namespace cobb {
   // Split a variadic list of types in half, and wrap each half in `PassToTemplate`. 
   // If the list isn't exactly divisible by two, biases toward the second half.
   template<template<typename...> typename PassToTemplate, typename... Types>
   class split_variadics_in_half {
      protected:
         static constexpr const size_t divisor = 2;

      private:
         split_variadics_in_half_loose() = delete;
         ~split_variadics_in_half_loose() = delete;

         static constexpr const size_t half = sizeof...(Types) / divisor;

         using seq_a = std::make_index_sequence<half>;
         using seq_b = index_sequences::incremented_by<std::make_index_sequence<sizeof...(Types) - half>, half>;

         using all_as_tuple = std::tuple<Types...>;

         using tuple_a = tuples::pick_types_by_indices<all_as_tuple, seq_a>;
         using tuple_b = tuples::pick_types_by_indices<all_as_tuple, seq_b>;

      public:
         using result_a = tuples::unpack_types_into<tuple_a, PassToTemplate>;
         using result_b = tuples::unpack_types_into<tuple_b, PassToTemplate>;
   };

   template<template<typename...> typename PassToTemplate, typename... Types>
   class split_variadics_in_half_strict : public split_variadics_in_half<PassToTemplate, Types...> {
      static_assert(sizeof...(Types) % divisor == 0);
   };
}