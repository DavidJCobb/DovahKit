#pragma once
#include <cstdint>

namespace nifDK {
   class file_reader;

   struct hkWorldObjCinfoProperty {
      uint32_t data = 0;
      uint32_t size = 0;
      uint32_t capacity_and_flags = 0;

      void read(file_reader&);
      void unchecked_read(file_reader&);
   };
}