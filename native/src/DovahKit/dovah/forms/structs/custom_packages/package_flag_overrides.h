#pragma once
#include "../../_common.h"
#include "../../../data/packages/preferred_movement_speed.h"

namespace dovah::loaded_forms::structs::custom_packages {
   class package_flag_overrides {
      public:
         static constexpr const uint32_t subrecord_legacy = 'PFOR';
         static constexpr const uint32_t subrecord_modern = 'PFO2';

         using preferred_movement_speed = packages::preferred_movement_speed;

      public:
         struct {
            uint32_t set   = 0; // PFO*+0x00
            uint32_t clear = 0; // PFO*+0x04
         } general;
         struct {
            uint16_t set   = 0; // PFO*+0x08
            uint16_t clear = 0; // PFO*+0x0A
         } interrupt;
         preferred_movement_speed preferred_speed = preferred_movement_speed::run; // PFO2+0x0C

      public:
         void load(tes_subrecord_reader&, load_order_interfaces::form_load&);
         void save(tes_subrecord_writer&, load_order_interfaces::form_save&);

         constexpr bool empty() const noexcept {
            if (this->general.set)
               return false;
            if (this->general.clear)
               return false;
            if (this->interrupt.set)
               return false;
            if (this->interrupt.clear)
               return false;
            return this->preferred_speed != preferred_movement_speed::run;
         }
   };
}