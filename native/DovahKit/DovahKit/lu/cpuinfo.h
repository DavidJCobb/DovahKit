#pragma once
#include "./singleton.h"

namespace lu {
   class cpuinfo : public singleton<cpuinfo> {
      protected:
         cpuinfo();

      public:
         struct {
            bool sse_1;
            bool sse_2;
            bool sse_3;
            bool ssse_3;
            bool sse_4_1;
            bool sse_4_2;
            bool sse_4a;
            bool sse_5;
            bool avx;
         } extension_support;
   };
}
