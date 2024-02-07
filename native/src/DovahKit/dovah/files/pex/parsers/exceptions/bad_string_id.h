#pragma once
#include "./base_read_exception.h"

namespace dovah::pex::exceptions {
   class bad_string_id : public base_read_exception {
      public:
         const uint16_t id;
         
         template<typename Stream>
         explicit bad_string_id(const Stream& stream, uint16_t n) : base_read_exception(stream.get_position()), id(n) {}
   };
}
