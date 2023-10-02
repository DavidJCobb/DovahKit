#pragma once
#include <array>
#include <bit>
#include <type_traits>

namespace cobb::bitstreams {
   //
   // Function to write patterned bits into a given value, if bit-casts to 
   // that value are possible. Used by the round-trip tests.
   //
   template<typename T> requires (requires {
      requires std::is_trivially_copyable_v<T>;
      requires !std::is_empty_v<T>;
      requires sizeof(T) > 0;
   })
   constexpr void round_trip_test_scramble(T& dst) {
      using byte_array = std::array<uint8_t, sizeof(T)>;
      if constexpr (std::is_trivially_copyable_v<byte_array> && sizeof(byte_array) == sizeof(T)) { // std::bit_cast possible?
         byte_array bytes = {};
         for (uint8_t& byte : bytes)
            byte = uint8_t(0b01010101);
         dst = std::bit_cast<T>(bytes);
      }
   }
}
