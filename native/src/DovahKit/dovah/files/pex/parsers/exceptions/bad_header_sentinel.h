#pragma once
#include "./base_read_exception.h"

namespace dovah::pex::exceptions {
   class bad_header_sentinel : public base_read_exception {
      public:
         template<typename Stream>
         explicit bad_header_sentinel(const Stream& stream) : base_read_exception(stream.get_position()) {}
   };
}
