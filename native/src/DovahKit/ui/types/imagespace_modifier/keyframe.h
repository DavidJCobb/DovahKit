#pragma once
#include <array>
#include <QColor>
#include "helpers/optionals/finite_float.h"

namespace ui::types::imagespace_modifier {
   struct keyframe {
      public:
         using  optional_color = std::optional<QColor>;
         using  optional_float = cobb::optionals::finite_float;
         struct mult_add {
            optional_float mult;
            optional_float add;
         };

      public:
         float timestamp = 0.0F;
         //
         struct {
            struct {
               optional_float radius;
            } basic;
            struct {
               optional_float strength;
            } motion;
            struct {
               optional_float strength;
               optional_float ramp_up;
               optional_float start;
               struct {
                  optional_float start;
                  optional_float value;
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
            optional_color fade;
            optional_color tint;
         } colors;
         struct {
            optional_float strength;
            optional_float distance;
            optional_float range;
         } depth_of_field;
         struct {
            optional_float strength;
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
         std::array<mult_add, 9> unknown; // base: DNAM+0x48,0x4C ... DNAM+0x88,0x8C; interp subrecord indices are mult:[0x08, 0x10] and add:[0x48, 0x50]
   };
}