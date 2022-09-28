#pragma once
#include <cstdint>
#include <vector>
#include "./ConstraintData.h"

namespace nifDK {
   class file_reader;

   static_assert(false, "NifSkope's nif.xml description handles nested constraints, but the way we've set things up does not.");
   static_assert(false, "Try reverse-engineering these structures in the game itself, and see whether we can get a better ");
   static_assert(false, "understanding of how they're laid out. Then, go back to the drawing board for these.");

   struct MalleableDescriptor : public ConstraintData {
      float tau; // not in FO3 or Skyrim
      float damping;
      float strength;

      void read(file_reader&);
   };
}