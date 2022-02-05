#pragma once
#include <cstdint>

namespace nifDK {
   class file_reader;

   struct NiColor {
      uint8_t r;
      uint8_t g;
      uint8_t b;

      void read(file_reader&);
      void unchecked_read(file_reader&);
   };
   struct NiColorA {
      uint8_t r;
      uint8_t g;
      uint8_t b;
      uint8_t a;

      void read(file_reader&);
      void unchecked_read(file_reader&);
   };
}
