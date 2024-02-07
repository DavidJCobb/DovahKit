#pragma once
#include "./base_read_exception.h"

namespace dovah::pex::exceptions {
   class unexpected_eof : public base_read_exception {
      public:
         template<typename Stream>
         explicit unexpected_eof(const Stream& stream) : base_read_exception(stream.get_position()) {}
   };
}
