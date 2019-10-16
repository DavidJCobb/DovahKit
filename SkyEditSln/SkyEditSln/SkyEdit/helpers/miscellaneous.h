#pragma once

namespace cobb {
   template<typename T, typename U> inline void edit_bit(T& target, U mask, bool change) {
      // T and U should sorta be the same, but if T is an int and U is an enum 
      // based on the same int, then they count as different types.
      target ^= (-change ^ target) & mask;
      /*//
      if (v)
         this->refcount |= kRefcountFlag_Edited;
      else
         this->refcount &= ~kRefcountFlag_Edited;
      //*/
   }
};