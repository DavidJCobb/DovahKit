#pragma once
#include <cstdint>

namespace dovah::utils::files::tes {
   constexpr bool signature_is_suspicious(uint32_t signature) noexcept {
      for (int i = 0; i < 4; ++i) {
         uint8_t c = (signature >> (0x08 * i)) & 0xFF;
         if (c != '_' && !(c >= '0' && c <= '9') && !(c >= 'A' && c <= 'Z'))
            return true;
      }
      return false;
   }
}