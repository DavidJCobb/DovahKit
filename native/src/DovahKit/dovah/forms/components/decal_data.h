#pragma once
#include <cstdint>
#include <string>
#include "../_common.h"
#include "../structs/color_dword.h"

namespace dovah {
   namespace loaded_forms::components {
      struct decal_data {
         struct flag {
            enum type : uint8_t {
               parallax       = 0x01,
               alpha_blending = 0x02,
               alpha_testing  = 0x04,
               no_subtextures = 0x08,
            };
         };
         using flags_t = std::underlying_type_t<flag::type>;

         struct {
            float min =  8.0F;
            float max = 32.0F;
         } width;
         struct {
            float min =  8.0F;
            float max = 32.0F;
         } height;
         float depth     = 32.0F;
         float shininess =  4.0F;
         struct {
            float   scale  = 1.0F;
            uint8_t passes = 4; // cannot exceed 30
         } parallax;
         flags_t flags = 0;
         color_t color;
         
         void load(tes_subrecord_reader&, load_order_interfaces::form_load& intfc);
         void save(tes_subrecord_writer&, load_order_interfaces::form_save& intfc);
      };
   }
}