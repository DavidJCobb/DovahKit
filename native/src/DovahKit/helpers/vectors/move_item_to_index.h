#pragma once
#include "./move_item_within.h"

namespace cobb::vectors {
   //
   // Move `list[from]` such that the element located there will end up at 
   // `list[to]`.
   //
   template<is_std_vector Vector>
   constexpr bool move_item_to_index(Vector& list, size_t from, size_t to) {
      if (from == to)
         return false;
      return move_item_within(list, from, (std::make_signed_t<size_t>)to - from);
   }

   //
   // Move `list[from]` such that the element located there will end up 
   // immediately after the element presently at `list[after]`.
   //
   template<is_std_vector Vector>
   constexpr bool move_item_after_index(Vector& list, size_t from, size_t after) {
      return move_item_to_index(
         list,
         from,
         after + ((from >= after) ? 1 : 0)
      );
   }
   
   template<is_std_vector Vector>
   constexpr bool move_item_before_index(Vector& list, size_t from, size_t before) {
      return move_item_to_index(
         list,
         from,
         before - ((from < before) ? 1 : 0)
      );
   }
}