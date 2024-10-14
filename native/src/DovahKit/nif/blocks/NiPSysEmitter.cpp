#include "NiPSysEmitter.h"
#include "../reader.h"

namespace nifDK::block_types {
   void NiPSysEmitter::parse(file_reader& reader) {
      NiPSysModifier::parse(reader);
      reader.read(this->speed.base);
      reader.read(this->speed.variance);
      reader.read(this->declination.base);
      reader.read(this->declination.variance);
      reader.read(this->planar_angle.base);
      reader.read(this->planar_angle.variance);
      reader.read(this->initial_color);
      reader.read(this->radius.base);
      reader.read(this->radius.variance);
      reader.read(this->lifespan.base);
      reader.read(this->lifespan.variance);
   }
}