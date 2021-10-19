#pragma once

namespace cobb {
   #if defined(__GNUC__)
      [[noreturn]] inline __attribute__((always_inline)) void unreachable() {
         __builtin_unreachable();
      }
   #elif defined(_MSC_VER)
      [[noreturn]] __forceinline void unreachable() {
         __assume(false);
      }
   #else
      inline void unreachable() {}
   #endif

}