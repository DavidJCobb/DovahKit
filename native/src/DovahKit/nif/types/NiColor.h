#pragma once

namespace nifDK {
   class file_reader;

   struct NiColor {
      float r;
      float g;
      float b;

      void read(file_reader&);
      void unchecked_read(file_reader&);
   };
   struct NiColorA {
      float r;
      float g;
      float b;
      float a;

      void read(file_reader&);
      void unchecked_read(file_reader&);
   };
}
