/*

This file is provided under the Creative Commons 0 License.
License: <https://creativecommons.org/publicdomain/zero/1.0/legalcode>
Summary: <https://creativecommons.org/publicdomain/zero/1.0/>

One-line summary: This file is public domain or the closest legal equivalent.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/
#pragma once
#include <vector>
#include "../type_traits/is_std_vector.h"

namespace cobb::vectors {
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
   // You can find a good explanation of how the use of std::rotate here works 
   // at this page:
   // https://www.fluentcpp.com/2018/04/20/ways-reordering-collection-stl/
   //
   template<bool Clamp, is_std_vector Vector>
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
         std::rotate(from, from, to);
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
         std::rotate(to, from, from);
      }
      return true;
   }
}
