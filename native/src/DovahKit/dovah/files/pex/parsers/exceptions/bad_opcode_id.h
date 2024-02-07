#pragma once
#include "./base_read_exception.h"

namespace dovah::pex::exceptions {
   class bad_opcode_id : public base_read_exception {
      public:
         const uint8_t opcode;
         
         template<typename Stream>
         explicit bad_opcode_id(const Stream& stream, uint8_t op) : base_read_exception(stream.get_position()), opcode(op) {}
   };
}
