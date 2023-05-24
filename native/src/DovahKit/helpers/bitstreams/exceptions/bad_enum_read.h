#pragma once
#include "./read_exception.h"

namespace cobb::bitstreams::exceptions {
   class bad_enum_read : public read_exception {
      public:
         bad_enum_read(const position_type& p, uintmax_t us) : read_exception(p), underlying_seen(us) {}

         const uintmax_t underlying_seen = 0;
   };
}
