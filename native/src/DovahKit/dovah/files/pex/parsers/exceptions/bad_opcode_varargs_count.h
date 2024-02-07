#pragma once
#include "./base_read_exception.h"

namespace dovah::pex::exceptions {
   class bad_opcode_varargs_count : public base_read_exception {
      public:
         const uint8_t varargs_count_type;
         
         template<typename Stream>
         explicit bad_opcode_varargs_count(const Stream& stream, uint8_t type) : base_read_exception(stream.get_position()), varargs_count_type(type) {}
   };
}
