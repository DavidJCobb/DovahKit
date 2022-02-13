#pragma once
#include <stdexcept>

namespace vulkanDK::dds {
   class load_exception : public std::runtime_error {
      public:
         load_exception() : std::runtime_error("Invalid DDS data.") {};
         load_exception(const char* m) : std::runtime_error(m) {};
   };
}