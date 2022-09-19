#pragma once

namespace cobb {
   template<typename T, typename U> inline void edit_bit(T& target, U mask, bool change) {
      //
      // T and U should sorta be the same, but if T is an int and U is an enum 
      // based on the same int, then they count as different types.
      //
      //target ^= (-change ^ target) & mask;
      if (change)
         target |= mask;
      else
         target &= ~mask;
   }

   template<int bytecount> struct bytecount_to_int;
   template<> struct bytecount_to_int<1> { using type = uint8_t; };
   template<> struct bytecount_to_int<2> { using type = uint16_t; };
   template<> struct bytecount_to_int<4> { using type = uint32_t; };
   template<> struct bytecount_to_int<8> { using type = uint64_t; };
   template<int bytecount> using bytecount_to_int_t = typename bytecount_to_int<bytecount>::type;
};