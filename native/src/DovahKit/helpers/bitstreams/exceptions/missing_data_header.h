#pragma once
#include "./read_exception.h"

namespace cobb::bitstreams::exceptions {
   class missing_data_header : public read_exception {
      using read_exception::read_exception;
   };
}
