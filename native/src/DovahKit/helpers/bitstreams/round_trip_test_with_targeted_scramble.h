#pragma once
#include <type_traits>
#include "./round_trip_test_scramble.h"


namespace cobb::bitstreams {
   template<typename T, auto... MemberPointers> requires (std::is_member_object_pointer_v<decltype(MemberPointers)> && ...)
   constexpr const bool round_trip_test_with_targeted_scramble() {
      if constexpr (
         sizeof(T) == 0
         || std::is_empty_v<T> // guard against a bug in MSVC's bit-cast intrinsic affecting empty structs, which are technically sizeof 1
      ) {
         return true;
      } else {
         T src = {};

         writer w;
         w.stream(w.header());
         w.stream(src);

         T dst = {};
         if constexpr (sizeof...(MemberPointers) > 0) {
            //
            // Scramble those members that we have been asked to scramble prior to the 
            // read, so that totally failed or no-op reads don't automatically compare 
            // equal.
            //
            (round_trip_test_scramble(dst.*MemberPointers), ...);
         }

         reader r;
         r.set_buffer(w.data(), w.get_bytespan());
         r.stream(dst);

         return src == dst;
      }
   }
}
