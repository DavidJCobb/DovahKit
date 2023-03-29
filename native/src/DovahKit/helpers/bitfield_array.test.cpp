#include "./bitfield_array.h"

namespace cobb::tests::bitfield_array {
   constexpr bool bitcount_9 = []() {
      {  // Basic case: first element has all bits set
         cobb::bitfield_array<unsigned int, 2, 9> arr = {};
         arr[0] = 0b111111111;
         if (arr.data()[0] != 0b11111111) throw;
         if (arr.data()[1] != 0b10000000) throw;
         if (arr[0] != 0b111111111) throw;
         if (arr[1] != 0) throw;
      }
      {  // Advanced case: three elements; middle has all bits set; others are zero
         cobb::bitfield_array<unsigned int, 3, 9> arr = {};
         arr[1] = 0b111111111;
         if (arr[0] != 0) throw;
         if (arr[1] != 0b111111111) throw;
         if (arr[2] != 0) throw;
      }
      return true;
   }();
   constexpr auto bitcount_3 = []() {
      cobb::bitfield_array<unsigned int, 8, 3> arr = {};
      for (int i = 0; i < arr.size(); ++i) {
         arr[i] = i;
      }
      if (arr.data()[0] != 0b00000101) throw;
      if (arr.data()[1] != 0b00111001) throw;
      if (arr.data()[2] != 0b01110111) throw;
      if (arr[0] != 0) throw;
      if (arr[1] != 1) throw;
      if (arr[2] != 2) throw;
      if (arr[3] != 3) throw;
      if (arr[4] != 4) throw;
      if (arr[5] != 5) throw;
      if (arr[6] != 6) throw;
      if (arr[7] != 7) throw;
      return arr;
   }();
}