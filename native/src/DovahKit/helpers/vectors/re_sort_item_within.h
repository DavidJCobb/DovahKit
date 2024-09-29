#pragma once
#include <algorithm>
#include <vector>
#include "../type_traits/is_std_vector.h"
#include "./move_item_within.h"

namespace cobb::vectors {
   namespace impl::re_sort_item_within {
      template<is_std_vector Vector>
      constexpr void default_on_before_handler(typename Vector::iterator from, typename Vector::iterator to) {}
   }

   //
   // If you want to insert an element into an already-sorted vector without breaking 
   // the sort, you use std::upper_bound to find the insertion point. But what do you  
   // do if you later edit the element, and need to re-sort it within the vector? You 
   // can't use std::upper_bound again, because that requires the entire vector to be 
   // sorted, and you have a single element (potentially) out of order. Sure, you can 
   // erase the element from the vector and then get the std::upper_bound, but if you 
   // do that, then you'll be shifting all of the vector elements twice (once for the 
   // removal and again for the reinsertion).
   // 
   // What if you could just... split the std::upper_bound call in half? Take all the 
   // elements to the left of your to-be-re-sorted element and get their upper bound, 
   // and then do the same with all the elements to the right, and combine both calls 
   // to get a unified result?
   // 
   // Thus, this function. Maybe there's a better, more mathematically correct way to 
   // do this, but I'm essay-brained, not math-brained,  and Google is a rotting husk 
   // of its former self, so this'll have to do.
   //
   template<is_std_vector Vector, typename Comparator, typename OnBeforeMoveHandler>
   constexpr void re_sort_item_within(
      Vector& list,
      typename Vector::iterator it,
      Comparator          comparator,
      OnBeforeMoveHandler handler
   ) {
      const auto& item  = *it;
      auto        left  = it;
      auto        right = list.end();
      if (it != list.begin()) {
         left = std::upper_bound(
            list.begin(),
            it,
            item,
            comparator
         );
      }
      if (it + 1 != list.end()) {
         right = std::upper_bound(
            it + 1,
            list.end(),
            item,
            comparator
         );
      }
      if (left == it) { // no appropriate place on the lefthand side
         //
         // Move to the spot we found on the righthand side. Note that the entire 
         // righthand side is displaced by 1 since we're moving in the direction 
         // of the upper bound (or something like that; it's easier to explain 
         // through visual examples like those commented in the unit tests below).
         //
         auto right_dst = right - 1;
         if (right_dst == it) // no-op move
            return;
         handler(it, right_dst);
         std::rotate(it, it + 1, right);
         return;
      }
      //
      // Move to the spot we found on the lefthand side.
      //
      handler(it, left);
      std::rotate(left, it, it + 1);
   }

   template<is_std_vector Vector>
   constexpr void re_sort_item_within(Vector& list, typename Vector::iterator it) {
      re_sort_item_within(list, it, std::less{}, &impl::re_sort_item_within::default_on_before_handler<Vector>);
   }

   template<is_std_vector Vector, typename Comparator>
   constexpr void re_sort_item_within(
      Vector& list,
      typename Vector::iterator it,
      Comparator&& comparator
   ) {
      re_sort_item_within(list, it, comparator, &impl::re_sort_item_within::default_on_before_handler<Vector>);
   }

   #pragma region Correctness checks
      static_assert([]() -> bool {
         std::vector<int> list = { 0, 1, 2, 3, 4 };
         size_t i = 2;
         /*
         0 1 2 3 4      test list prior to re-sorting
             x	 	      upper bound left
               x  	   upper bound right
             x          destination (lefthand upper bound; no-op move)
         */
         re_sort_item_within(list, list.begin() + i, [](int a, int b) -> bool { return a < b; });

         for (size_t i = 1; i < list.size(); ++i)
            if (list[i] <= list[i - 1])
               return false;
         return true;
      }(), "No-op.");

      static_assert([]() -> bool {
         std::vector<int> list = { 0, 2, 1, 3, 4 };
         size_t i = 2;
         /*
         0 2 1 3 4      test list prior to re-sorting
           x  		      upper bound left
               x  	   upper bound right
           x            destination (lefthand upper bound)
         */
         re_sort_item_within(list, list.begin() + i);

         for (size_t i = 1; i < list.size(); ++i)
            if (list[i] <= list[i - 1])
               return false;
         return true;
      }(), "Re-sort leftward by 1.");

      static_assert([]() -> bool {
         std::vector<int> list = { 1, 2, 0, 3, 4 };
         size_t i = 2;
         /*
         1 2 0 3 4      test list prior to re-sorting
         x    		      upper bound left
               x  	   upper bound right
         x              destination (lefthand upper bound)
         */
         re_sort_item_within(list, list.begin() + i);

         for (size_t i = 1; i < list.size(); ++i)
            if (list[i] <= list[i - 1])
               return false;
         return true;
      }(), "Re-sort leftward to start.");

      static_assert([]() -> bool {
         std::vector<int> list = { 0, 1, 3, 2, 4 };
         size_t i = 2;
         /*
         0 1 3 2 4      test list prior to re-sorting
             x		      upper bound left
                 x	   upper bound right
               x        destination (righthand upper bound minus 1)
         */
         re_sort_item_within(list, list.begin() + i);

         for (size_t i = 1; i < list.size(); ++i)
            if (list[i] <= list[i - 1])
               return false;
         return true;
      }(), "Re-sort rightward by 1.");

      static_assert([]() -> bool {
         std::vector<int> list = { 0, 1, 4, 2, 3 };
         size_t i = 2;
         /*
         0 1 3 2 4      test list prior to re-sorting
             x		      upper bound left
                   x	   upper bound right
                 x      destination (righthand upper bound minus 1)
         */
         re_sort_item_within(list, list.begin() + i);

         for (size_t i = 1; i < list.size(); ++i)
            if (list[i] <= list[i - 1])
               return false;
         return true;
      }(), "Re-sort rightward to end.");
   #pragma endregion
}
