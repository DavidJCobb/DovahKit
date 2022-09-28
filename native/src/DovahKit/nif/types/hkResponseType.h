#pragma once
#include <cstdint>

namespace nifDK {
   enum class hkResponseType : uint8_t {
      invalid,
      simple_contact,
      reporting,
      none,
   };
}