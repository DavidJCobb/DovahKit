#pragma once
#include <array>
#include <QColor>

namespace ui::types::imagespace_modifier {
   struct computed_keyframe {
      public:
         using  color = QColor;
         struct mult_add {
            float mult = 1;
            float add  = 0;
         };

      public:
         struct {
            struct {
               float radius = 0;
            } basic;
            struct {
               float strength = 0;
            } motion;
            struct {
               float strength = 0;
               float ramp_up  = 0;
               float start    = 0;
               struct {
                  float start = 0;
                  float value = 0;
               } ramp_down;
            } radial;
         } blurs;
         struct {
            mult_add saturation;
            mult_add brightness;
            mult_add contrast;
            mult_add unused; // base: DNAM+0xA8,0xAC; interp subrecord indices: 0x14, 0x54
         } cinematic;
         struct {
            color fade;
            color tint;
         } colors;
         struct {
            float strength = 0;
            float distance = 0;
            float range    = 0;
         } depth_of_field;
         struct {
            float strength = 0;
         } double_vision;
         struct {
            struct {
               mult_add blur_radius;
               mult_add scale;
               mult_add threshold;
            } bloom;
            mult_add eye_adapt_speed;
            struct {
               mult_add min;
               mult_add max;
            } target_luminescence;
            mult_add sky_scale;
            mult_add sunlight_scale;
         } hdr;
         std::array<mult_add, 8> unknown; // base: DNAM+0x48,0x4C ... DNAM+0x88,0x8C; interp subrecord indices are mult:[0x08, 0x10] and add:[0x48, 0x50]
   };
}