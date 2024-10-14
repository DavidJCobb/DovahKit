#include "NiParticles.h"
#include "../reader.h"

namespace nifDK::block_types {
   void NiParticles::parse(file_reader& reader) {
      NiGeometry::parse(reader);
      if (reader.user_version<2>() >= 100) {
         reader.read(this->vertex_description.emplace());
      }
   }
}