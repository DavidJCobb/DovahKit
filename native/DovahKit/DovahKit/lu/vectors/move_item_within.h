#pragma once
#include <algorithm>
#include <vector>
#include "../concepts/instantiation_of/std_vector.h"

namespace lu::vectors {
   //
   // Moves an item within the vector. Returns `true` if the move is successful, 
   // or `false` if it isn't.
   // 
   // If `Clamp` is true, then the move will fail if the item is already as far 
   // as it can go in the direction you want to move it in; otherwise, the item 
   // will always be moved, though if the distance you request isn't possible, 
   // the item will only move as far as is possible. If `Clamp` is false, then 
   // the move will fail if the item can't move the full distance you've asked 
   // for.
   // 
   // See also:
   // https://stackoverflow.com/a/65203019
   //
   template<bool Clamp = false, concepts::instantiation_of_std_vector Vector>
   constexpr bool move_item_within(Vector& list, size_t i, int by) {
      if (by == 0)
         return true;
      auto from = list.begin() + i;
      if (by > 0) {
         {
            const auto size = list.size();
            if constexpr (Clamp) {
               if (i == size - 1)
                  return false;
               if (i + by >= size) {
                  by = size - 1 - i;
               }
            } else {
               if (i + by >= size)
                  return false;
            }
         }
         if (by == 1) {
            std::swap(list[i], list[i + 1]);
            return true;
         }
         auto to = from + by;
         std::rotate(from, from + 1, to + 1);
      } else {
         if constexpr (Clamp) {
            if (i == 0)
               return false;
            if (i < -by)
               by = -(int)i;
         } else {
            if (i < -by)
               return false;
         }
         if (by == -1) {
            std::swap(list[i], list[i - 1]);
            return true;
         }
         auto to = from + by;
         std::rotate(to, from, from + 1);
      }
      return true;
   }
}
