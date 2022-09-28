#include "HavokFilter.h"
#include "../reader.h"

namespace nifDK {
   void HavokFilter::read(file_reader& reader) {
      if (reader.version() == file_version::from_parts<20, 2, 0, 7>) {
         reader.require_size(4);
      } else {
         reader.require_size(3);
      }
      this->unchecked_read(reader);
   }
   void HavokFilter::unchecked_read(file_reader& reader) {
      if (reader.version() == file_version::from_parts<20, 2, 0, 7>) {
         uint8_t byte;
         reader.unchecked_read(byte);
         this->collision_layer = (dovah::collision_layer)byte;
      }

      uint8_t byte;
      reader.unchecked_read(byte);
      this->flags.linked            = (byte & (1 << 7));
      this->flags.disable_collision = (byte & (1 << 6));
      this->flags.scaled            = (byte & (1 << 5));
      this->body_part = (dovah::body_part)(byte & 0b00001111);

      reader.unchecked_read(this->group);
   }
}