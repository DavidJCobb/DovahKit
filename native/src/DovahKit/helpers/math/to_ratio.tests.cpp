#include "./to_ratio.h"

constexpr auto test = []() {
   int64_t  n = 0;
   uint64_t d = 0;
   cobb::to_ratio(1.3333333333F, n, d);
   return d;
}();