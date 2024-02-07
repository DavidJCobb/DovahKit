#pragma once
#include <cstdint>
#include <string>
#include "./tabled_string.h"

namespace dovah::pex {
   //
   // Flags like "Hidden" and "Conditional" don't have fixed bits. Instead, they're 
   // encoded into the script's "user_flags" section and given bits there. Then, each 
   // object that can have these flags will have a flags-mask whose bits should match 
   // the bits given to user-flags in the header.
   //
   struct user_flag_definition {
      tabled_string name;
      uint8_t       bit_index = 0xFF;

      constexpr uint32_t to_mask() const noexcept {
         return (uint32_t)1 << this->bit_index;
      }

      template<typename Stream>
      constexpr void read(Stream&);
      //
      template<typename Stream>
      static constexpr void skip(Stream&);
   };
}

#include "./user_flag_definition.inl"