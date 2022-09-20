#include "BSMultiBound.h"
#include "../reader.h"

namespace nifDK::block_types {
   void BSMultiBound::parse(file_reader& reader) {
      reader.read_ref(this->data);
   }

   void BSMultiBoundDataOBB::parse(file_reader& reader) {
      reader.read(this->center);
      reader.read(this->sizes);
      reader.read(this->rotation);
   }

   void BSMultiBoundDataSphere::parse(file_reader& reader) {
      reader.read(this->center);
      reader.read(this->radius);
   }
}