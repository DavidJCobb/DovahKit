#pragma once
#include "../_common.h"
#include "../structs/color_dword.h"

namespace dovah::loaded_forms::structs {
   class directional_ambient_lighting_colors {
      public:
         static constexpr const uint32_t subrecord = 'DALC';

         struct axis_colors {
            color_t positive = { 255, 255, 255, 0 };
            color_t negative = { 255, 255, 255, 0 };
         };

      public:
         axis_colors x;
         axis_colors y;
         axis_colors z;
         color_t     specular = { 0, 0, 0, 0 };
         float       fresnel = 1.0F;
      
      public:
         void load(tes_subrecord_reader&, load_order_interfaces::form_load& intfc);
         void save(tes_subrecord_writer&, load_order_interfaces::form_save& intfc);
   };
}