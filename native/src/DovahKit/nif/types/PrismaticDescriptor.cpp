#include "PrismaticDescriptor.h"
#include "../reader.h"

namespace nifDK {
   void PrismaticDescriptor::read(file_reader& reader) {
      bool is_old_havok = reader.version() < file_version::from_parts<20, 2, 0, 7>;

      if (is_old_havok) {
         reader.read(this->transform_a[3]);
         reader.read(this->transform_a[1]);
         reader.read(this->transform_a[2]);
         reader.read(this->transform_a[0]);
         reader.read(this->transform_b[0]);
         reader.read(this->transform_b[3]);
         reader.read(this->transform_b[1]);
         reader.read(this->transform_b[2]);
      } else {
         reader.read(this->transform_a);
         reader.read(this->transform_b);
      }
      reader.read(this->distance.minimum);
      reader.read(this->distance.maximum);
      reader.read(this->friction);
      reader.read(this->motor);
   }
}