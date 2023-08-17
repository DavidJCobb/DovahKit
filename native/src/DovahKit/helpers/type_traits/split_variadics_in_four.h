#pragma once
#include <utility>
#include "../index_sequences/incremented_by.h"
#include "../tuples/pick_types_by_indices.h"
#include "../tuples/unpack_types_into.h"

namespace cobb {
   // Split a variadic list of types in four, and wrap each quarter in `PassToTemplate`. 
   // If the list isn't exactly divisible by four, biases toward the last quarter.
   template<template<typename...> typename PassToTemplate, typename... Types>
   class split_variadics_in_four {
      protected:
         static constexpr const size_t divisor = 4;

      private:
         split_variadics_in_four() = delete;
         ~split_variadics_in_four() = delete;

         static constexpr const size_t part = sizeof...(Types) / divisor;

         using seq_a = std::make_index_sequence<part>;
         using seq_b = index_sequences::incremented_by<seq_a, part>;
         using seq_c = index_sequences::incremented_by<seq_b, part>;
         using seq_d = index_sequences::incremented_by<std::make_index_sequence<sizeof...(Types) - (part * 3)>, (part * 3)>;

         using all_as_tuple = std::tuple<Types...>;

         using tuple_a = tuples::pick_types_by_indices<all_as_tuple, seq_a>;
         using tuple_b = tuples::pick_types_by_indices<all_as_tuple, seq_b>;
         using tuple_c = tuples::pick_types_by_indices<all_as_tuple, seq_c>;
         using tuple_d = tuples::pick_types_by_indices<all_as_tuple, seq_d>;

      public:
         using result_a = tuples::unpack_types_into<tuple_a, PassToTemplate>;
         using result_b = tuples::unpack_types_into<tuple_b, PassToTemplate>;
         using result_c = tuples::unpack_types_into<tuple_c, PassToTemplate>;
         using result_d = tuples::unpack_types_into<tuple_d, PassToTemplate>;
   };
   
   template<template<typename...> typename PassToTemplate, typename... Types>
   class split_variadics_in_four_strict : public split_variadics_in_four<PassToTemplate, Types...> {
      static_assert(sizeof...(Types) % divisor == 0);
   };
}