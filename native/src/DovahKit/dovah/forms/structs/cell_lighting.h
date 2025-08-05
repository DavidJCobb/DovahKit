#pragma once
#include "../_common.h"
#include "./color_dword.h"
#include "./directional_ambient_lighting_colors.h"

namespace dovah::loaded_forms::structs {
   struct cell_lighting {
      public:
         struct inherit_flag {
            inherit_flag() = delete;
            enum type : uint32_t {
               ambient              = 1 <<  0, // also directional ambient colors
               directional          = 1 <<  1,
               fog_color            = 1 <<  2,
               fog_distance_near    = 1 <<  3,
               fog_distance_far     = 1 <<  4,
               directional_rotation = 1 <<  5,
               directional_fade     = 1 <<  6,
               fog_clip_distance    = 1 <<  7,
               fog_power            = 1 <<  8,
               fog_max              = 1 <<  9,
               light_fade_distances = 1 << 10,
            };
         };
         using inherit_flags_t = std::underlying_type_t<inherit_flag::type>;

         static constexpr const inherit_flags_t default_inherit_flags = (
            inherit_flag::ambient |
            inherit_flag::directional |
            inherit_flag::fog_color |
            inherit_flag::fog_distance_near |
            inherit_flag::fog_distance_far |
            inherit_flag::fog_clip_distance |
            inherit_flag::fog_power |
            inherit_flag::fog_max |
            inherit_flag::light_fade_distances
         );

      public:
         struct {
            color_t base;
            structs::directional_ambient_lighting_colors directional;
         } ambient;
         struct {
            color_t color;
            float   fade = 1;
            struct {
               int32_t xy = 0;
               int32_t z  = 0;
            } rotation;
         } directional;
         struct {
            struct {
               color_t near;
               color_t far;
            } colors;
            float clip_distance = 0;
            float far   = 0;
            float max   = 1;
            float near  = 0;
            float power = 1;
         } fog;
         struct {
            float start = 0;
            float end = 0;
         } light_fade_distance;
         inherit_flags_t inherit_flags = default_inherit_flags; // indicates which fields are inherited from a lighting template

      public:
         void load(tes_subrecord_reader&, load_order_interfaces::form_load& intfc);
         void save(tes_subrecord_writer&, load_order_interfaces::form_save& intfc);
   };
}