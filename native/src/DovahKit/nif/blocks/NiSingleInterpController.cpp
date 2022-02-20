#include "NiSingleInterpController.h"
#include "NiInterpolator.h"
#include "../reader.h"

namespace nifDK::block_types {
   void NiSingleInterpController::parse(file_reader& reader) {
      NiInterpController::parse(reader);
      //
      reader.read_ref(this->interpolator);
   }
}