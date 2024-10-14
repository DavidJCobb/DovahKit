#include "NiPSysEmitterCtlr.h"
#include "NiInterpolator.h"
#include "NiPSysEmitterCtlrData.h"
#include "../reader.h"

namespace nifDK::block_types {
   void NiPSysEmitterCtlr::parse(file_reader& reader) {
      NiPSysModifierCtlr::parse(reader);
      if (reader.version() >= file_version::from_parts<10, 2, 0, 0>) {
         reader.read_ref(this->visibility_interpolator);
      }
      if (reader.version() <= file_version::from_parts<10, 1, 0, 103>) {
         reader.read_ref(this->deprecated_data);
      }
   }
}