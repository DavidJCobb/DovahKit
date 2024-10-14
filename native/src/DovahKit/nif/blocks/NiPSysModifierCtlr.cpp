#include "NiPSysModifierCtlr.h"
#include "../reader.h"

namespace nifDK::block_types {
   void NiPSysModifierCtlr::parse(file_reader& reader) {
      NiSingleInterpController::parse(reader);
      reader.read_indexed_string(this->modifier_name);
   }
}