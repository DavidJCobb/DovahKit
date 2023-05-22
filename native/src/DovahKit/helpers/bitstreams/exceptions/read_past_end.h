#pragma once
#include "./read_exception.h"

namespace cobb::bitstreams::exceptions {
   class read_past_end : public read_exception {
      public:
         read_past_end(const position_type& p, size_t br) : read_exception(p), bits_requested(br) {}

         const size_t bits_requested = 0;
   };
}
