#pragma once
#include <stdexcept>

namespace dovah::pex::exceptions {
   class base_read_exception : public std::runtime_error {
      public:
         const uint32_t offset;
         
         explicit base_read_exception(uint32_t o) : runtime_error(""), offset(o) {}
   };
}
