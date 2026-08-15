#include "./collision_layer.h"
#include "dovah/files/tes_file_reading/elements.h"
#include "dovah/forms/CollisionLayer.h"

namespace {
   using loaded_form_type = dovah::loaded_forms::CollisionLayer;
}

namespace dovahkit::subsystems::form_info_cache::cached_data::by_form {
   void collision_layer::skim_subrecord(dovah::tes_file_reading::subrecord& subrecord) {
      switch (subrecord.signature()) {
         case 'BNAM':
            subrecord.read(this->unique_id);
            break;
         case 'GNAM':
            {
               uint32_t flags = 0;
               if (subrecord.read(flags)) {
                  this->flags.sensor  = flags & loaded_form_type::layer_flag::sensor;
                  this->flags.trigger = flags & loaded_form_type::layer_flag::trigger_volume;
               }
            }
            break;
      }
   }
   bool collision_layer::update(const loaded_form_type& src) {
      const auto prior = *this;

      bool changed = false;
      if (src.unique_id != this->unique_id) {
         this->unique_id = src.unique_id;
         changed = true;
      }

      {
         bool src_flag = src.layer_flags & loaded_form_type::layer_flag::sensor;
         if (src_flag != this->flags.sensor) {
            this->flags.sensor = src_flag;
            changed = true;
         }
      }
      {
         bool src_flag = src.layer_flags & loaded_form_type::layer_flag::trigger_volume;
         if (src_flag != this->flags.trigger) {
            this->flags.trigger = src_flag;
            changed = true;
         }
      }

      return changed;
   }
}