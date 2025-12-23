#pragma once
#include <stdexcept>

namespace dovah::exceptions {
   class invalid_load_order : public std::runtime_error {
      public:
         invalid_load_order() : std::runtime_error("The load order is invalid.") {}
      protected:
         invalid_load_order(const char* c) : std::runtime_error(c) {}
   };
}