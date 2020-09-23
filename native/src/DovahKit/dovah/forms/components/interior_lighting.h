#pragma once
#include "../_common.h"
#include "../structs/color_dword.h"

namespace dovah::loaded_forms::components {
   struct interior_lighting {
      struct inherit_flag {
         inherit_flag() = delete;
         enum type : uint32_t {
            ambient              = 0x0001, // also directional ambient colors
            directional          = 0x0002,
            fog_color            = 0x0004,
            fog_distance_near    = 0x0008,
            fog_distance_far     = 0x0010,
            directional_rotation = 0x0020,
            directional_fade     = 0x0040,
            fog_clip_distance    = 0x0080,
            fog_power            = 0x0100,
            fog_max              = 0x0200,
            light_fade_distances = 0x0400,
         };
      };
      //
      color_t ambient;
      color_t directional;
      color_t fog_color_near;
      float   fog_distance_near;
      float   fog_distance_far;
      struct {
         uint32_t xy;
         uint32_t z;
      } rotation;
      float directional_fade  = 1.0F;
      float fog_distance_clip;
      float fog_power;
      struct { // unused for BGSLightingTemplate
         color_t x_pos;
         color_t x_neg;
         color_t y_pos;
         color_t y_neg;
         color_t z_pos;
         color_t z_neg;
      } directional_ambient_colors;
      color_t specular; // unused
      float   fresnel = 1.0F; // unused
      color_t fog_color_far;
      float   fog_max = 1.0F;
      struct {
         float start;
         float end;
      } light_fade_distance;
      uint32_t inherit_flags = 0x79F; // what fields are inherited from the lighting template?
      //
      void load(tes_subrecord_reader&);
      static void generateUseInfo(tes_subrecord_reader&, form_stub*);
      void save(tes_subrecord_writer&);
   };
}