#pragma once
#include "./any.h"
#include "../../streams/bitstream_position.h"

namespace cobb::bitstreams::exceptions {
   class read_exception : public any {
      public:
         using position_type = cobb::streams::bitstream_position;
      public:
         read_exception(const position_type& p) : position(p) {}

         const position_type position;
   };
}