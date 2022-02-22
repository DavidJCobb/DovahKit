#include "NiTimeController.h"
#include "../reader.h"

#include "NiObjectNET.h"

namespace nifDK::block_types {
   void NiTimeController::parse(file_reader& reader) {
      reader.read_ref(this->next);
      {
         uint16_t flags = 0;
         reader.read(flags);
         this->config.anim_type  = (animation_type)(flags & 1);
         this->config.cycle_type = (cycle_type)((flags >> 1) & 0b11);
         this->config.active                = (flags & (1 << 3)) != 0;
         this->config.play_backwards        = (flags & (1 << 4)) != 0;
         this->config.is_manager_controlled = (flags & (1 << 5)) != 0;
         this->config.unknown               = (flags & (1 << 6)) != 0;
      }
      reader.read(this->frequency);
      reader.read(this->phase);
      reader.read(this->start_time);
      reader.read(this->end_time);
      if (reader.version() >= file_version::from_parts<3, 3, 0, 13>) {
         reader.read_ref(this->target);
      }
      if (reader.version() <= file_version::from_parts<3, 1, 0, 0>) {
         reader.read(this->legacy_data.unknown);
      }
   }
}