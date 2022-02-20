#include "BSLightingShaderPropertyFloatController.h"
#include "../reader.h"

namespace nifDK::block_types {
   void BSLightingShaderPropertyFloatController::parse(file_reader& reader) {
      NiFloatInterpController::parse(reader);
      //
      reader.read(this->field);
   }
}